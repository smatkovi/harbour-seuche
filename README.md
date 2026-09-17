# Seuche

Kooperatives Seuchen-Brettspiel für Sailfish OS (später auch Android), 2–4 Personen
gegen das Spiel. Regelwerk und Spielplan folgen dem bekannten Vorbild; Name,
Grafik, Rollen- und Ereignisbezeichnungen sowie alle Texte sind eigenständig.

## Stand

| Teil | Stand |
|---|---|
| Regelwerk, Spielplan, Datenmodell | `spec/` — fertig |
| Regelkern (`src/core`) | Aufbau, Aktionen, Ziehen, Epidemie, Infektion, Ausbrüche, Sieg/Niederlage — fertig und getestet |
| Computergegner | offen |
| Oberfläche (QML) | offen |
| LAN/Online | offen |
| Erweiterungsmodule | vorgesehen, nicht implementiert (`spec/regeln.md`, §10) |

## Aufbau

```
spec/      Regelwerk (regeln.md), Spielplan (karte.md), Datenmodell (datenmodell.md)
src/core/  Regelkern, reines C++17 ohne Qt
tests/     zwei Testprogramme ohne Framework
tools/     Bauen, Testen, Veröffentlichen
```

`spec/karte.md` wird aus `src/core/Map.cpp` erzeugt (`tools/genmap.py`), nicht von
Hand pflegen.

## Bauen und testen

Auf dem Entwicklungsrechner steht kein C++-Compiler; gebaut wird auf dem
Arch-Rechner:

    tools/test.sh            # rsync, bauen, beide Testprogramme laufen lassen

## Veröffentlichen

RPMs erscheinen als Release dieses Repos:

    tools/release.sh 0.1.0 pfad/zum/harbour-seuche-0.1.0-1.aarch64.rpm

Das Skript legt das Tag an, erzeugt das Release und hängt das RPM an; eine Kopie
landet zusätzlich in `~/ps/rpms/seuche/`.
