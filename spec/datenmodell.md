# Seuche — Datenmodell

C++17, Namensraum `seuche`, in `src/core/`. Aufbau wie beim
Tarock-Kern: reine Datentypen ohne Qt, damit derselbe Code unter Sailfish, Android
und in den Tests läuft.

## Identitäten

Alles hängt an **einer** Nummerierung, die sich nie ändern darf, weil sie zugleich
Spielplan, Kartendeck, Speicherformat und Netzwerkprotokoll ist:

```
Stadt 0..47    nach Farbe gruppiert: 0–11 blau, 12–23 gelb, 24–35 schwarz, 36–47 rot
               Farbe = id / 12, "fünf Karten einer Farbe" ist damit ein Bereichstest
               Städte aus Erweiterungen hängen hinter 47 an
Spielerkarte   0..47    Stadtkarte (gleiche Nummer wie die Stadt)
               64..95   Ereigniskarte (Grundspiel 64–68, Rest für Erweiterungen)
               96..111  Epidemiekarte (Grundspiel nur 96; der virulente Stamm
                        gibt jeder Epidemiekarte eine eigene Nummer)
Farbe 0..4     blau, gelb, schwarz, rot, violett (Mutation, ohne eigene Städte)
Rolle 0..6     Grundspiel; Erweiterungsrollen ab 7
Infektionskarte = Stadtnummer, ein eigener Typ ist unnötig
```

Die Lücken zwischen den Bereichen sind Absicht: Ein zugeschaltetes Modul soll nie
etwas umnummerieren, sonst wären alte Spielstände und laufende Netzpartien hin.

## Dateien

| Datei | Inhalt |
|---|---|
| `Map.h/.cpp` | `City`, `Colour`, `CityInfo` (Name, Schlüssel, Koordinaten, Einwohner), Nachbarschaft, `mapIsSane()` |
| `Card.h/.cpp` | `PlayerCard`, `Event`, Handkarten-Hilfen (`countColour`, `removeCard`) |
| `Role.h/.cpp` | die 7 Rollen, `cardsForCure(role)` |
| `State.h/.cpp` | `Game` — der gesamte Zustand, plus Konstanten und Abfragen |
| `Action.h` | `Action` (eine Zugaktion) und `EventPlay` (eine Ereigniskarte) |
| `Reason.h/.cpp` | Ablehnungsgründe mit deutschem Text |
| `Setup.h/.cpp` | `newGame()`, Deckbau, Epidemien einmischen |
| `Rules.h/.cpp` | die Engine: Aktionsprüfung, Zugphasen, Epidemie, Ausbruchskette |

## Zustand

```cpp
struct Game {
    std::array<std::array<uint8_t, 5>, 48> cubes;   // Würfel je Stadt und Farbe
    CitySet stations;                               // bitset<48>
    std::array<uint8_t, 5> supply;                  // Vorrat, je 24 (violett 12)
    std::array<CureState, 5> cures;                 // None / Cured / Eradicated
    uint8_t colours;                                // 4, mit Mutation 5

    std::vector<Player> players;                    // Rolle, Stadt, Hand, Planerkarte
    uint8_t atTurn, actionsLeft, drawsLeft, discardingPlayer;

    PlayerCards playerDeck, playerDiscard;          // back() ist oben
    InfectionCards infectionDeck, infectionDiscard, removedFromGame;

    uint8_t outbreaks, infectionRateIndex, infectionsLeft;
    bool quietNight;
    Difficulty difficulty; Modules modules; Phase phase; Outcome outcome;
};
```

Entwurfsentscheidungen:

- **`back()` ist die Deckoberseite.** Ziehen ist `pop_back()`; „unterste Karte" bei
  der Epidemie ist `front()`. Ein Deque wäre bequemer, aber der Vektor
  serialisiert sich direkt.
- **Würfel je Stadt *und* Farbe.** Durch Ausbrüche liegen fremdfarbige Würfel in
  Städten, das lässt sich nicht auf eine Zahl pro Stadt verkürzen.
- **Vorrat getrennt geführt.** Die Niederlage „Würfel alle" ist eine Prüfung beim
  Setzen, kein Nachzählen über den ganzen Plan.
- **Phasen statt eines Zustandsautomaten mit Rückrufen.** `Phase` macht jeden
  Zwischenzustand speicherbar — auch „X muss abwerfen" mitten im Ziehen —, was für
  Speicherstände und den Netzwerkmodus nötig ist.
- **Rollen fragen, nicht Sitzplätze prüfen.** `seatOf(Role)` und `shielded(City)`
  kapseln die zwei Regeln, die von außen in die Würfelplatzierung hineinwirken
  (Quarantäne, Ärztin).
- **Ereignisse außerhalb des Aktionsstroms.** `EventPlay` ist ein eigener Typ, weil
  Ereigniskarten keine Aktion kosten und auch im Zug anderer gespielt werden.
- **Module als Flags, nicht als Ableitungen.** `Modules` sitzt im Zustand, die
  Farbzahl ist eine Laufzeitgröße, die Kartenbereiche haben Luft. Nichts davon ist
  implementiert — es kostet aber jetzt nichts und später viel (siehe `regeln.md`, §10).

## Netzwerk

Offen ist alles außer der Reihenfolge der beiden Decks (siehe `regeln.md`, §9).
Damit reicht: Der Host mischt und schickt `Game` ohne `playerDeck`/`infectionDeck`,
Clients schicken `Action` bzw. `EventPlay`. Kein Betrugsschutz nötig, weil niemand
gegen die anderen spielt — anders als bei Schnapsen oder Tarock.

Genau eine Erweiterung bricht das: der **Bioterrorist** zieht verdeckt gegen das
Team. Solange das Modul aus ist, bleibt der einfache Weg gültig; eingeschaltet
braucht er einen eigenen Protokollpfad mit Sichtfiltern je Empfänger.

## Stand

`tests/test_model.cpp` prüft Kartendaten, Aufbau für 2–4 Personen, Kartenzahlen,
Würfelbuchhaltung und eine Epidemie je Deckabschnitt:

```
cities 48, edges 93, average degree 3.88
  degree 1: 1   2: 4   3: 12   4: 16   5: 13   6: 2
all checks passed
```

Der Test prüft den Graphen auch gegen Stichproben aus dem Vorbild (Atlanta als
Start mit Chicago/Washington/Miami, Santiago als einzige Sackgasse, Istanbul und
Hongkong mit Grad 6), damit ein Tippfehler in der Kantenliste auffällt.

`tests/test_rules.cpp` prüft die Engine an von Hand gestellten Positionen —
Bewegung, Stationen, Heilen und Ausrotten, Behandeln, Ausbruch samt Kettenreaktion,
Quarantäne, Epidemie, Handlimit, alle fünf Ereignisse, alle Rollenfähigkeiten und
die vier Spielenden — und spielt danach 200 Partien mit zufälligen legalen Zügen
durch, wobei nach **jedem** Schritt Würfelbilanz, Stationszahl und Handlimit
geprüft werden:

```
random games: 0 won, 200 lost (120 outbreaks, 80 cubes, 0 cards)
all rules checks passed
```

Dass Zufallszüge nie gewinnen, ist erwartbar — der Durchlauf sucht keine Strategie,
sondern Zustandsfehler.

Die QML-Brücke `src/SeucheEngine.*` legt genau diesen Zustand für die Oberfläche
aus: Städte mit Würfeln, Figuren und Stationen, Hände, Heilmittel, dazu die
legalen Aktionen mit fertigem deutschem Text. `tests/test_qmlbridge.cpp` wertet
jede davon in einer echten `QQmlEngine` aus und spielt eine ganze Partie über die
Aufrufe der Seiten durch — eine Eigenschaft, die in QML als `undefined` ankommt,
fällt dort auf und nicht erst als leere Seite auf dem Gerät.

Computergegner sind nicht vorgesehen; gespielt wird kooperativ mit 2–4 Sitzen an
einem Gerät. Nächster Schritt: LAN/Online, danach die Erweiterungsmodule.
