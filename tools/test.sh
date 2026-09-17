#!/bin/sh
# Build the core on the Arch machine and run both test programs.
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
ssh "$HOST" "cd $DIR && \
    g++ -std=c++17 -Wall -Wextra -O1 -o test_model tests/test_model.cpp src/core/*.cpp && ./test_model && \
    g++ -std=c++17 -Wall -Wextra -O1 -o test_rules tests/test_rules.cpp src/core/*.cpp && ./test_rules"
