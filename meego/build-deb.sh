#!/bin/sh
# Packt den ARM-Bau als Harmattan-.deb. Läuft auf dem Baurechner, nach
# "meego/build.sh arm":
#
#   meego/build-deb.sh              # -> build/meego/harbour-seuche_<version>_armel.deb
#   VERSION=0.5.1 meego/build-deb.sh
#
# Wie es auf dem N9 liegt:
#   /opt/harbour-seuche/bin/harbour-seuche        das Spiel
#   /opt/harbour-seuche/qml                       die Oberfläche samt world.png
#   /opt/harbour-seuche/icons
#   /usr/share/applications/harbour-seuche.desktop
#   /usr/share/icons/hicolor/80x80/apps/harbour-seuche.png
# Das .deb schreibt mkdeb.py (kein dpkg-deb nötig): Harmattans dpkg ist 1.15.x
# und will debian-binary, control.tar.gz, data.tar.gz, nur gzip, und keine
# angehängten Schrägstriche an den Mitgliedsnamen.
set -e

HERE=$(cd "$(dirname "$0")/.." && pwd)
PKG=$HERE/meego
OUT=$HERE/build/meego
BIN=$OUT/arm/harbour-seuche
XGCC=${XGCC:-/tmp/xgcc-harmattan}
VERSION=${VERSION:-$(sed -n 's/^Version: *//p' "$HERE/rpm/harbour-seuche.spec" | head -1)}

[ -x "$BIN" ] || { echo "ARM-Binärdatei fehlt: $BIN (meego/build.sh arm)" >&2; exit 1; }

STAGE=$OUT/stage
rm -rf "$STAGE"
mkdir -p "$STAGE/DEBIAN" "$STAGE/opt/harbour-seuche/bin" "$STAGE/opt/harbour-seuche/icons" \
         "$STAGE/usr/share/applications" "$STAGE/usr/share/themes/base/meegotouch/icons" \
         "$STAGE/usr/share/icons/hicolor/80x80/apps" \
         "$STAGE/usr/share/doc/harbour-seuche"

# --- Programm und Daten -----------------------------------------------------
cp "$BIN" "$STAGE/opt/harbour-seuche/bin/harbour-seuche"
"$XGCC/bin/arm-none-linux-gnueabi-strip" "$STAGE/opt/harbour-seuche/bin/harbour-seuche"
chmod 755 "$STAGE/opt/harbour-seuche/bin/harbour-seuche"
cp -a "$PKG/qml" "$STAGE/opt/harbour-seuche/qml"
# Der Landhintergrund liegt nur einmal im Baum, bei der Sailfish-Oberfläche.
cp "$HERE/sailfish/world.png" "$STAGE/opt/harbour-seuche/qml/world.png"
cp "$HERE/sailfish/icons/icon-256.png" "$STAGE/opt/harbour-seuche/icons/icon-256.png"

# --- Symbole: 80x80 für den Starter, 64x64 base64 für die Paketverwaltung ---
# Der Starter sucht in hicolor, nicht im meegotouch-Thema; beides zu setzen
# kostet nichts und deckt beide Wege ab.
# Das Icon traegt die Stock-Silhouette von Harmattan, erzeugt mit
# ~/ps/meego-icon-tool/squircle.py --fill. Ein rundes Icon faellt auf dem
# Startbildschirm zwischen den Stock-Icons sofort als fremd auf; die Ecken
# werden dabei mit der eigenen Grundfarbe des Icons gefuellt, damit nichts
# vom Inhalt weggeschnitten wird.
cp "$HERE/meego/icons/icon-80.png" \
    "$STAGE/usr/share/icons/hicolor/80x80/apps/harbour-seuche.png"
cp "$HERE/meego/icons/icon-80.png" \
    "$STAGE/usr/share/themes/base/meegotouch/icons/harbour-seuche-80.png"
cp "$HERE/meego/icons/icon-64.png" "$OUT/icon-64.png"

cp "$PKG/harbour-seuche.desktop" "$STAGE/usr/share/applications/harbour-seuche.desktop"
gzip -9nc "$PKG/changelog" > "$STAGE/usr/share/doc/harbour-seuche/changelog.gz"
find "$STAGE" -type f ! -path "*/bin/*" -exec chmod 644 {} +
find "$STAGE" -type d -exec chmod 755 {} +

# --- control aus control.in -------------------------------------------------
# XB-Maemo-Icon-26 ist das 64x64-PNG als base64, Folgezeilen um ein Leerzeichen
# eingerückt; ohne das zeigt die Anwendungsverwaltung kein Symbol.
VERSION="$VERSION" ICON="$OUT/icon-64.png" python3 - "$PKG/control.in" "$STAGE/DEBIAN/control" <<'PY'
import base64, os, sys, textwrap
src, dst = sys.argv[1], sys.argv[2]
with open(os.environ["ICON"], "rb") as f:
    b64 = base64.b64encode(f.read()).decode("ascii")
icon = "\n".join(" " + line for line in textwrap.wrap(b64, 76))
with open(src, "r", encoding="utf-8") as f:
    ctl = f.read()
ctl = ctl.replace("@VERSION@", os.environ["VERSION"]).replace("@ICON@", icon)
with open(dst, "w", encoding="utf-8") as f:
    f.write(ctl)
PY

DEB="$OUT/harbour-seuche_${VERSION}_armel.deb"
python3 "$PKG/mkdeb.py" "$STAGE" "$DEB"
python3 "$PKG/mkdeb.py" --info "$DEB" | head -40
echo "== $DEB"
