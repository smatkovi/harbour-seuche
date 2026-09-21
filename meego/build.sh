#!/bin/sh
# Baut die MeeGo-Harmattan-Ausgabe (Nokia N9) von Seuche. Läuft auf dem
# Baurechner (siehe meego/README.md), im dorthin gespiegelten Quellbaum:
#
#   meego/build.sh arm    N9-Binärdatei -> build/meego/arm/harbour-seuche
#   meego/build.sh x86    derselbe Code für x86_64 gegen das Qt 4.7.4 des
#                         Qt-Simulators, um ihn am Schreibtisch anzusehen
#   meego/build.sh check  übersetzt jede QML-Datei mit dem Qt 4.7 des Geräts
#                         und meldet unbekannte Typen und Eigenschaften
#
# Der ARM-Bau braucht die Kreuzwerkzeugkette aus meego/toolchain.sh (XGCC) und
# das MADDE-Sysroot (SYSROOT); moc kommt aus dem Qt des Simulators (SIMQT),
# demselben Qt 4.7.4 wie auf dem Gerät.
set -e

HERE=$(cd "$(dirname "$0")/.." && pwd)
MODE=${1:-arm}
XGCC=${XGCC:-/tmp/xgcc-harmattan}
SYSROOT=${SYSROOT:-$HOME/QtSDK/Madde/sysroots/harmattan_sysroot_10.2011.34-1_slim}
SIMQT=${SIMQT:-$HOME/QtSDK/Simulator/Qt/gcc}
DESKTOPQT=${DESKTOPQT:-$HOME/QtSDK/Desktop/Qt/4.8.1/gcc}
JOBS=${JOBS:-8}
OUT=$HERE/build/meego/$MODE
mkdir -p "$OUT"

# Der Regelkern ist reines C++17 und wird hier genauso gebaut wie für Sailfish.
CORE_SRC=$(ls "$HERE"/src/core/*.cpp | tr '\n' ' ')
APP_SRC="$HERE/src/SeucheEngine.cpp $HERE/src/net/LanSession.cpp $HERE/src/net/Snapshot.cpp"
MEEGO_SRC="$HERE/meego/main.cpp $HERE/meego/src/LinkItem.cpp"
MOC_HEADERS="$HERE/src/SeucheEngine.h $HERE/src/net/LanSession.h $HERE/meego/src/LinkItem.h $HERE/meego/src/Theme.h"

# -Wno-register: Qt 4.7 headers still use the keyword, which C++17 removed.
# -Wno-nonnull: qobject_cast's Qt 4 type check dereferences a null pointer in
# a branch that is never taken, and GCC 14 sees it.
QT4_FLAGS="-std=gnu++17 -O2 -Wall -Wno-register -Wno-deprecated-declarations -Wno-nonnull \
 -D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS -DQT_NO_DEBUG \
 -I$HERE/meego/compat -include $HERE/meego/compat/qt4compat.h \
 -I$HERE/src -I$HERE/meego/src -I$OUT"
QT4_MODULES="QtCore QtGui QtNetwork QtScript QtDeclarative"

case "$MODE" in
arm)
    CXX=$XGCC/bin/arm-none-linux-gnueabi-g++
    [ -x "$CXX" ] || { echo "Kreuzübersetzer fehlt: $CXX (meego/toolchain.sh)" >&2; exit 1; }
    MOC=$SIMQT/bin/moc
    QTINC=$SYSROOT/usr/include/qt4
    CXXFLAGS="--sysroot=$SYSROOT $QT4_FLAGS -I$QTINC"
    for m in $QT4_MODULES; do CXXFLAGS="$CXXFLAGS -I$QTINC/$m"; done
    # Hartes Gleitkomma, aber Harmattan behält den alten Lader ld-linux.so.3;
    # GCC würde nach dem armhf-Namen fragen, den der N9 nicht hat.
    # --exclude-libs hält das statische libstdc++/libgcc privat, damit das Qt
    # des Geräts an seiner eigenen (GCC-4.4-)Laufzeit hängen bleibt.
    LDFLAGS="--sysroot=$SYSROOT -static-libstdc++ -static-libgcc -Wl,-O1 -Wl,--as-needed \
 -Wl,--exclude-libs,ALL -Wl,--dynamic-linker=/lib/ld-linux.so.3"
    LIBS="-lQtDeclarative -lQtScript -lQtNetwork -lQtGui -lQtCore -lpthread"
    ;;
x86)
    CXX=${CXX:-g++}
    MOC=$SIMQT/bin/moc
    QTINC=$SIMQT/include
    CXXFLAGS="$QT4_FLAGS -I$QTINC"
    for m in $QT4_MODULES; do CXXFLAGS="$CXXFLAGS -I$QTINC/$m"; done
    LDFLAGS="-L$SIMQT/lib -Wl,-rpath,$SIMQT/lib"
    LIBS="-lQtDeclarative -lQtScript -lQtNetwork -lQtGui -lQtCore -lpthread"
    ;;
check)
    # Nur der QML-Prüfer, ohne Kreuzübersetzer und ohne Bildschirm.
    #
    # Gebaut gegen das gewöhnliche Desktop-Qt (DESKTOPQT): das Qt des
    # Simulators klinkt sich beim Start in den Simulator ein und bricht ohne
    # X-Server ab. Die com.nokia.meego-Komponenten liegen getrennt davon unter
    # Desktop/Qt/474/gcc/imports und werden von dort importiert.
    CXX=${CXX:-g++}
    # moc muss zu den Kopfdateien passen: hier 4.8.1, nicht das 4.7.4 des
    # Simulators, das der ARM- und der x86-Bau benutzen.
    MOC=$DESKTOPQT/bin/moc
    QTINC=$DESKTOPQT/include
    MEEGOIMPORTS=${MEEGOIMPORTS:-$HOME/QtSDK/Desktop/Qt/474/gcc/imports}
    CXXFLAGS="$QT4_FLAGS -I$QTINC"
    for m in $QT4_MODULES; do CXXFLAGS="$CXXFLAGS -I$QTINC/$m"; done
    "$MOC" $(echo "$CXXFLAGS" | tr ' ' '\n' | grep '^-[ID]' | tr '\n' ' ') \
        "$HERE/meego/src/LinkItem.h" -o "$OUT/moc_LinkItem.cpp"
    "$CXX" $CXXFLAGS -o "$OUT/checkqml" "$HERE/meego/tests/checkqml.cpp" \
        "$HERE/meego/src/LinkItem.cpp" "$OUT/moc_LinkItem.cpp" \
        -L"$DESKTOPQT/lib" -Wl,-rpath,"$DESKTOPQT/lib" \
        -lQtDeclarative -lQtScript -lQtNetwork -lQtGui -lQtCore
    # Das meego-Plugin zieht libQtDBus nach, das nur im lib-Verzeichnis liegt.
    LD_LIBRARY_PATH=$DESKTOPQT/lib "$OUT/checkqml" "$HERE/meego/qml" "$MEEGOIMPORTS" || exit 1
    python3 "$HERE/meego/tests/lint-qml.py" "$HERE/meego/qml"
    exit $?
    ;;
*)
    echo "usage: meego/build.sh [arm|x86|check]" >&2
    exit 2
    ;;
esac

echo "== moc"
for h in $MOC_HEADERS; do
    "$MOC" $(echo "$CXXFLAGS" | tr ' ' '\n' | grep '^-[ID]' | tr '\n' ' ') \
        "$h" -o "$OUT/moc_$(basename "$h" .h).cpp"
done
MOC_SRC=$(ls "$OUT"/moc_*.cpp | tr '\n' ' ')

echo "== compile"
# Alle auf einmal in den Hintergrund und einmal warten: es sind gut zwanzig
# Dateien, und `wait -n` gibt es in /bin/sh nicht.
OBJS=""
FAIL=$OUT/.failed
rm -f "$FAIL"
for f in $CORE_SRC $APP_SRC $MEEGO_SRC $MOC_SRC; do
    o="$OUT/$(echo "$f" | sed "s|^$HERE/||; s|/|_|g; s|\.cpp$|.o|")"
    ( "$CXX" $CXXFLAGS -c "$f" -o "$o" || echo "$f" >> "$FAIL" ) &
    OBJS="$OBJS $o"
done
wait
[ -s "$FAIL" ] && { echo "übersetzen fehlgeschlagen:"; cat "$FAIL"; exit 1; }

echo "== link"
"$CXX" $LDFLAGS -o "$OUT/harbour-seuche" $OBJS $LIBS
ls -la "$OUT/harbour-seuche"
