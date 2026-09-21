# Seuche

Kooperatives Seuchen-Brettspiel für Sailfish OS (später auch Android), 2–4 Personen
gegen das Spiel. Regelwerk und Spielplan folgen dem bekannten Vorbild; Name,
Grafik, Rollen- und Ereignisbezeichnungen sowie alle Texte sind eigenständig.

## Stand

| Teil | Stand |
|---|---|
| Regelwerk, Spielplan, Datenmodell | `spec/` — fertig |
| Regelkern (`src/core`) | Aufbau, Aktionen, Ziehen, Epidemie, Infektion, Ausbrüche, Sieg/Niederlage — fertig und getestet |
| App (Silica/QML) | spielbar: Spielplan mit Landkarte und Zoom (klein und formatfüllend), Aktionen je Stadt, Rollenwahl mit fester Farbe je Rolle, Handkarten, Ereigniskarten, Rückgängig, Protokoll |
| LAN (`spec/netz.md`) | 2–4 Geräte im WLAN: Gastgeber hält die Partie, Gäste spiegeln sie |
| Erweiterungsmodule | vorgesehen, nicht implementiert (`spec/regeln.md`, §10) |

Computergegner sind **nicht** geplant: Das Spiel ist kooperativ, alle Sitze werden
an einem Gerät gesteuert. Beim Start wird gewählt, wie viele Personen mitspielen
(2–4). Die Rollen werden zufällig gezogen und lassen sich anschließend auf der
Rollenseite frei tauschen, solange die Partie noch nicht begonnen hat; jede Rolle
hat ihre eigene Farbe und die Figur auf dem Spielplan trägt sie.

Ein Zug lässt sich zurücknehmen, solange noch keine Karte aufgedeckt wurde: nach
der vierten Aktion fragt die Spielseite, ob der Zug wirklich vorbei ist, und bis
dahin führt **Rückgängig** Schritt für Schritt zurück. Sobald die erste Karte
gezogen ist, ist der Zug endgültig — in einem kooperativen Spiel wäre alles
andere ein Blick in die Zukunft.

## Aufbau

```
spec/      Regelwerk (regeln.md), Spielplan (karte.md), Datenmodell (datenmodell.md),
           Netzwerkspiel (netz.md)
src/core/  Regelkern, reines C++17 ohne Qt
src/       SeucheEngine — die QML-Brücke, und main.cpp
src/net/   LAN-Transport (aus harbour-snapszer) und Drahtformat
sailfish/  Silica-Oberfläche, Symbole, Desktop-Datei
rpm/       Paketbeschreibung
tests/     fünf Testprogramme ohne Framework, darunter test_play.cpp, das
           tausende vollständige Partien durchspielt
tools/     Bauen (build.sh), Testen (test.sh), Symbol (make_icon.py),
           Landhintergrund (make_world.py), Veröffentlichen (release.sh)
```

`spec/karte.md` wird aus `src/core/Map.cpp` erzeugt (`tools/genmap.py`), nicht von
Hand pflegen. `sailfish/world.png` erzeugt `tools/make_world.py` aus den
Flugplatzdaten, die harbour-fgview mitbringt — Einzelheiten in `CREDITS.md`.

## Bauen und testen

Auf dem Telefon steht kein C++-Compiler; gebaut wird auf dem Arch-Rechner:

    tools/test.sh            # Regelkern bauen und beide Kerntests laufen lassen

Die Pakete und der Test der QML-Brücke brauchen das Sailfish-SDK im Container
`sfossdk52` auf demselben Rechner:

    tools/build.sh              # RPMs für aarch64 und armv7hl
    tools/build.sh aarch64      # nur eine Architektur

    # alle vier Tests, im Container gegen das i486-Ziel
    sb2 -t SailfishOS-5.2.0.15-i486 cmake .. -DSEUCHE_BUILD_TESTS=ON && make && ctest

## Veröffentlichen

RPMs erscheinen als Release dieses Repos, je Version eines für **aarch64** und
eines für **armv7hl**:

    tools/build.sh
    tools/release.sh 0.2.0 ~/ps/rpms/seuche/harbour-seuche-0.2.0-1.*.rpm

`release.sh` erzeugt das Release und hängt alle übergebenen Pakete an (ein
bestehendes Release wird ergänzt); Kopien liegen in `~/ps/rpms/seuche/`.

| Architektur | Geräte |
|---|---|
| `aarch64` | Sailfish 4.4 und neuer, 64-bit — z. B. Xperia 10 III/IV/V |
| `armv7hl` | ältere 32-bit-Geräte |
