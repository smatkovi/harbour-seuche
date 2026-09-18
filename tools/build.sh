#!/bin/sh
# Build the RPMs in the Sailfish SDK container on the Arch machine.
#
#   tools/build.sh [aarch64|armv7hl] ...     (default: both)
#
# The packages land in ~/ps/rpms/seuche/ on this device, ready for
# tools/release.sh. BUILD_HOST, SDK_CONTAINER and SDK_TARGET override the
# defaults.
set -e
ROOT=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
ARCHES=${*:-"aarch64 armv7hl"}

if [ -n "$BUILD_HOST" ]; then
    HOST=$BUILD_HOST
elif ssh -o BatchMode=yes -o ConnectTimeout=4 sebastian@192.168.1.21 true 2>/dev/null; then
    HOST=sebastian@192.168.1.21
else
    HOST=arch
fi
CONTAINER=${SDK_CONTAINER:-sfossdk52}
TARGET=${SDK_TARGET:-SailfishOS-5.2.0.15}
VERSION=$(sed -n 's/^Version: *//p' "$ROOT/rpm/harbour-seuche.spec")

echo "harbour-seuche $VERSION for: $ARCHES"
ssh "$HOST" "mkdir -p ~/seuche-build/src ~/seuche-build/out"
rsync -a --delete --exclude .git "$ROOT/" "$HOST:seuche-build/src/"
ssh "$HOST" "cd ~/seuche-build/src && tar czf /tmp/seuche-src.tgz . && \
    docker cp /tmp/seuche-src.tgz $CONTAINER:/tmp/seuche-src.tgz"

mkdir -p "$HOME/ps/rpms/seuche"
for ARCH in $ARCHES; do
    RPM=harbour-seuche-$VERSION-1.$ARCH.rpm
    ssh "$HOST" "docker exec $CONTAINER bash -lc '\
        rm -rf ~/sbuild-$ARCH && mkdir -p ~/sbuild-$ARCH && cd ~/sbuild-$ARCH && \
        tar xzf /tmp/seuche-src.tgz && mb2 -t $TARGET-$ARCH build' | \
        grep -E '^Wrote:|error:|packages and'"
    ssh "$HOST" "docker cp $CONTAINER:/home/mersdk/sbuild-$ARCH/RPMS/$RPM ~/seuche-build/out/"
    rsync -a "$HOST:seuche-build/out/$RPM" "$HOME/ps/rpms/seuche/"
    echo "  -> ~/ps/rpms/seuche/$RPM"
done
