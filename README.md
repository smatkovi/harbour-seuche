# Seuche

Kooperatives Seuchen-Brettspiel für Sailfish OS (später auch Android), 2–4 Personen
gegen das Spiel. Regelwerk und Spielplan folgen dem bekannten Vorbild; Name,
Grafik, Rollen- und Ereignisbezeichnungen sowie alle Texte sind eigenständig.

## Stand

| Teil | Stand |
|---|---|
| Regelwerk, Spielplan, Datenmodell | `spec/` — fertig |
| Regelkern (`src/core`) | Aufbau, Aktionen, Ziehen, Epidemie, Infektion, Ausbrüche, Sieg/Niederlage — fertig und getestet |
| App (Silica/QML) | spielbar: Spielplan, Aktionen je Stadt, Handkarten, Ereigniskarten, Protokoll |
| LAN/Online | offen |
| Erweiterungsmodule | vorgesehen, nicht implementiert (`spec/regeln.md`, §10) |

Computergegner sind **nicht** geplant: Das Spiel ist kooperativ, alle Sitze werden
an einem Gerät gesteuert. Beim Start wird gewählt, wie viele Personen mitspielen
(2–4); die Rollen werden zufällig gezogen.

## Aufbau

```
spec/      Regelwerk (regeln.md), Spielplan (karte.md), Datenmodell (datenmodell.md)
src/core/  Regelkern, reines C++17 ohne Qt
src/       SeucheEngine — die QML-Brücke, und main.cpp
sailfish/  Silica-Oberfläche, Symbole, Desktop-Datei
rpm/       Paketbeschreibung
tests/     drei Testprogramme ohne Framework
tools/     Bauen, Testen, Symbol, Veröffentlichen
```

`spec/karte.md` wird aus `src/core/Map.cpp` erzeugt (`tools/genmap.py`), nicht von
Hand pflegen.

## Bauen und testen

Auf dem Telefon steht kein C++-Compiler; gebaut wird auf dem Arch-Rechner:

    tools/test.sh            # Regelkern bauen und beide Kerntests laufen lassen

Das RPM und der Test der QML-Brücke brauchen das Sailfish-SDK im Container
`sfossdk52` auf demselben Rechner:

    # Paket
    mb2 -t SailfishOS-5.2.0.15-aarch64 build
    # alle drei Tests
    sb2 -t SailfishOS-5.2.0.15-i486 cmake .. -DSEUCHE_BUILD_TESTS=ON && make && ctest

## Veröffentlichen

RPMs erscheinen als Release dieses Repos:

    tools/release.sh 0.1.0 pfad/zum/harbour-seuche-0.1.0-1.aarch64.rpm

Das Skript legt das Tag an, erzeugt das Release und hängt das RPM an; eine Kopie
landet zusätzlich in `~/ps/rpms/seuche/`.
