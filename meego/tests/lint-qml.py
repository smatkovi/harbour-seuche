#!/usr/bin/env python3
"""Sucht in der N9-Oberfläche nach dem, was QtQuick 1.1 und Qt 4.7 nicht können.

Der eigentliche Prüfer (tests/checkqml.cpp) kommt an den Seiten nicht vorbei,
weil das Plugin von com.nokia.meego einen Bildschirm braucht. Diese Liste
fängt dafür die Fälle ab, die auf dem Telefon sonst erst als leere Seite oder
als Fehler in der Konsole auffielen — alle schon einmal dagewesen:

  readonly property   gibt es erst in Qt 5 ("Readonly not yet supported")
  Canvas              QtQuick 2; die Linien zeichnet LinkItem
  qsTr                Qt 4.7 schiebt den Quelltext durch Latin-1, und die
                      Texte hier sind voller Umlaute
  let / const / =>    ES6; die JavaScript-Maschine von Qt 4.7 kennt nur ES5
  Silica-Reste        Übernahmen aus sailfish/, die hier nichts finden
"""
import re
import sys
from pathlib import Path

PATTERNS = [
    (re.compile(r'\breadonly\s+property\b'), "readonly property — gibt es in QtQuick 1.1 nicht"),
    (re.compile(r'^\s*Canvas\s*\{'), "Canvas — QtQuick 2; hier zeichnet LinkItem"),
    (re.compile(r'\bqsTr\s*\('), "qsTr — Qt 4.7 liest den Quelltext als Latin-1"),
    (re.compile(r'(?<![\w.])let\s+[A-Za-z_$]'), "let — ES6, Qt 4.7 kann nur ES5"),
    (re.compile(r'(?<![\w.])const\s+[A-Za-z_$]'), "const — ES6, Qt 4.7 kann nur ES5"),
    (re.compile(r'=>'), "Pfeilfunktion — ES6, Qt 4.7 kann nur ES5"),
    (re.compile(r'`'), "Schablonenzeichenkette — ES6, Qt 4.7 kann nur ES5"),
    (re.compile(r'import\s+Sailfish\.Silica'), "Sailfish.Silica — Rest aus der Sailfish-Fassung"),
    (re.compile(r'\b(SilicaFlickable|SilicaListView|PullDownMenu|ViewPlaceholder|'
                r'VerticalScrollDecorator|IconButton|ContextMenu|ComboBox|RemorsePopup|'
                r'allowedOrientations)\b'), "Silica-Baustein — hier nicht vorhanden"),
    (re.compile(r'import\s+QtQuick\s+2\.'), "QtQuick 2 — das Gerät hat QtQuick 1.1"),
]

def main():
    if len(sys.argv) < 2:
        print("usage: lint-qml.py <qml-dir>", file=sys.stderr)
        return 2
    root = Path(sys.argv[1])
    bad = 0
    files = sorted(root.glob("*.qml"))
    for path in files:
        for number, line in enumerate(path.read_text(encoding="utf-8").splitlines(), 1):
            # Kommentare beschreiben diese Fallen absichtlich beim Namen.
            stripped = line.strip()
            if stripped.startswith("//") or stripped.startswith("*"):
                continue
            for pattern, why in PATTERNS:
                if pattern.search(line):
                    print(f"FEHLER {path.name}:{number}: {why}")
                    print(f"       {stripped}")
                    bad += 1
    print(f"{len(files)} QML-Dateien abgesucht, {bad} Fundstellen")
    return 1 if bad else 0

if __name__ == "__main__":
    sys.exit(main())
