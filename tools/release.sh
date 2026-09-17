#!/bin/sh
# Publish RPMs as a release of this repository.
#
#   tools/release.sh <version> <rpm> [<rpm> ...]
#
# gh is logged in on the Arch machine, so the files go there first. A copy also
# lands in ~/ps/rpms/seuche/, like the other apps keep theirs.
set -e
VERSION=$1
shift 2>/dev/null || true
if [ -z "$VERSION" ] || [ $# -eq 0 ]; then
    echo "usage: tools/release.sh <version> <rpm> [<rpm> ...]" >&2
    exit 2
fi
for FILE in "$@"; do
    [ -f "$FILE" ] || { echo "no such file: $FILE" >&2; exit 2; }
done

if [ -n "$BUILD_HOST" ]; then
    HOST=$BUILD_HOST
elif ssh -o BatchMode=yes -o ConnectTimeout=4 sebastian@192.168.1.21 true 2>/dev/null; then
    HOST=sebastian@192.168.1.21
else
    HOST=arch
fi
REPO=${REPO:-smatkovi/harbour-seuche}
TAG=v$VERSION
WORK=/tmp/seuche-release

mkdir -p "$HOME/ps/rpms/seuche"
cp "$@" "$HOME/ps/rpms/seuche/"

NOTES=$(mktemp)
cat > "$NOTES" <<NOTE
Seuche $VERSION

RPM für Sailfish OS. Regelstand siehe \`spec/regeln.md\`.
NOTE

ssh "$HOST" "rm -rf $WORK && mkdir -p $WORK"
scp "$@" "$NOTES" "$HOST:$WORK/"
rm -f "$NOTES"
NOTENAME=$(basename "$NOTES")

ssh "$HOST" "cd $WORK && \
    if gh release view $TAG --repo $REPO >/dev/null 2>&1; then \
        gh release upload $TAG *.rpm --repo $REPO --clobber; \
    else \
        gh release create $TAG *.rpm --repo $REPO --title 'Seuche $VERSION' --notes-file $NOTENAME; \
    fi"
echo "released $TAG to $REPO, copy in ~/ps/rpms/seuche/"
