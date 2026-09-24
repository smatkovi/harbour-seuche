#!/bin/sh
# Build the core on the Arch machine and run the test programs.
#
#   tools/test.sh
#
# BUILD_HOST overrides the probe; BUILD_DIR the remote scratch directory.
set -e
ROOT=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)

if [ -n "$BUILD_HOST" ]; then
    HOST=$BUILD_HOST
elif ssh -o BatchMode=yes -o ConnectTimeout=4 sebastian@192.168.1.21 true 2>/dev/null; then
    HOST=sebastian@192.168.1.21          # home LAN, much faster
else
    HOST=arch                            # cloudflared tunnel
fi
DIR=${BUILD_DIR:-/tmp/seuche-core}

rsync -a --delete --exclude '.git' "$ROOT/" "$HOST:$DIR/"

# The rule core on its own: no Qt, so a plain g++ is the whole recipe.
ssh "$HOST" "cd $DIR && \
    g++ -std=c++17 -Wall -Wextra -O1 -o test_model tests/test_model.cpp src/core/*.cpp && ./test_model && \
    g++ -std=c++17 -Wall -Wextra -O1 -o test_rules tests/test_rules.cpp src/core/*.cpp && ./test_rules && \
    g++ -std=c++17 -Wall -Wextra -O2 -o test_play tests/test_play.cpp src/core/*.cpp && ./test_play"

# The QML facade and the LAN game need Qt but not Silica, so they build against
# the build machine's own Qt 5 rather than inside the Sailfish SDK. moc is run
# by hand because there is no qmake or cmake project here -- CMakeLists.txt
# builds the same two programs inside the SDK.
ssh "$HOST" "cd $DIR && \
    moc-qt5 src/SeucheEngine.h -o moc_SeucheEngine.cpp && \
    moc-qt5 src/net/LanSession.h -o moc_LanSession.cpp && \
    FLAGS=\$(pkg-config --cflags --libs Qt5Core Qt5Gui Qt5Network Qt5Qml) && \
    for t in test_qmlbridge test_lan test_save; do \
        g++ -std=c++17 -fPIC -O1 -Isrc -o \$t tests/\$t.cpp src/SeucheEngine.cpp \
            src/core/*.cpp src/net/*.cpp moc_SeucheEngine.cpp moc_LanSession.cpp \$FLAGS && \
        QT_QPA_PLATFORM=minimal ./\$t || exit 1; \
    done"

# Syntax of every QML file. Silica cannot resolve here, so this catches typos
# and nothing else -- but a typo otherwise only shows up on the phone.
ssh "$HOST" "cd $DIR/sailfish && for f in *.qml; do \
        out=\$(qmllint-qt5 \"\$f\" 2>&1); \
        if [ -n \"\$out\" ]; then echo \"\$f: \$out\"; exit 1; fi; \
    done; echo 'qml syntax ok'"
