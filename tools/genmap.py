#!/usr/bin/env python3
"""Regenerate spec/karte.md from src/core/Map.cpp.

    tools/genmap.py

The board data lives in the source, the document is only a readable view of it.
"""
import collections
import os
import re

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
COLOURS = ["Blau (Nordamerika, Europa)",
           "Gelb (Lateinamerika, Afrika)",
           "Schwarz (Nordafrika, Osteuropa, Naher Osten, Südasien)",
           "Rot (Ost- und Südostasien, Ozeanien)"]

src = open(os.path.join(ROOT, "src/core/Map.cpp"), encoding="utf-8").read()
info = re.findall(r'\{"([^"]+)",\s*"([^"]+)",\s*(-?[\d.]+)f,\s*(-?[\d.]+)f,\s*([\d\']+)\}', src)[:48]
edges = [(int(a), int(b)) for a, b in re.findall(r"\{\s*(\d+),\s*(\d+)\}", src)]
if len(info) != 48:
    raise SystemExit("expected 48 cities, found %d" % len(info))

adjacency = collections.defaultdict(list)
for a, b in edges:
    adjacency[a].append(b)
    adjacency[b].append(a)

degrees = collections.Counter(len(adjacency[i]) for i in range(48))
spread = ", ".join("%d×%d" % (degrees[d], d) for d in sorted(degrees))

out = [
    "# Seuche — Spielplan",
    "",
    "48 Städte, je 12 pro Farbe, %d Verbindungen (Schnitt %.2f pro Stadt)."
    % (len(edges), 2 * len(edges) / 48),
    "Knoten, Farbzuordnung und Kanten entsprechen dem Vorbild; die erste",
    "Forschungsstation steht im selben Knoten (**Atlanta**, Nr. 2), dort starten alle",
    "Figuren. Eigen sind Namensschreibung, Koordinaten und Einwohnerzahlen (eigene",
    "Schätzungen, sie entscheiden nur, wer beginnt).",
    "",
    "Die Nummer ist zugleich die Kartennummer im Spielerdeck, im Speicherformat und im",
    "Netzwerkprotokoll — sie darf sich nie ändern. Städte aus Erweiterungen hängen",
    "hinten an.",
    "",
    "Gradverteilung: %s." % spread,
    "",
    "Erzeugt aus `src/core/Map.cpp` mit `tools/genmap.py`, nicht von Hand pflegen.",
    "",
]
for colour in range(4):
    out += ["## %s" % COLOURS[colour], "",
            "| Nr. | Stadt | Schlüssel | Einw. | Grad | Verbindungen |",
            "|---:|---|---|---:|---:|---|"]
    for city in range(colour * 12, colour * 12 + 12):
        name, key, _lat, _lon, people = info[city]
        linked = ", ".join(info[n][0] for n in sorted(adjacency[city]))
        out.append("| %d | %s | `%s` | %.1f | %d | %s |"
                   % (city, name, key, int(people.replace("'", "")) / 1e6,
                      len(adjacency[city]), linked))
    out.append("")

path = os.path.join(ROOT, "spec/karte.md")
open(path, "w", encoding="utf-8").write("\n".join(out))
print("wrote %s (%d cities, %d edges)" % (path, len(info), len(edges)))
