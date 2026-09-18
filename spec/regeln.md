# Seuche — Regelwerk

Kooperatives Brettspiel für 2–4 Personen. Alle spielen gemeinsam gegen das Spiel:
Vier Seuchen brechen weltweit aus, das Team gewinnt nur, wenn es für alle vier ein
Heilmittel findet, bevor die Lage kippt.

Das Regelwerk ist mechanisch deckungsgleich mit der Vorlage (Grundspiel, zweite
Ausgabe), ebenso der Spielplan: gleiche Knoten, gleiche Farbzuordnung, gleiche
Kanten, erste Forschungsstation im selben Knoten (siehe `karte.md`). Eigenständig
sind Titel, Rollen- und Ereignisnamen, Grafik sowie alle Texte. Erweiterungen
siehe §10 — in Version 1 ist keine davon aktiv.

---

## 1. Material

| Element | Anzahl |
|---|---|
| Städte | 48, je 12 pro Seuchenfarbe |
| Seuchenwürfel | 24 pro Farbe (blau, gelb, schwarz, rot) |
| Forschungsstationen | 6 |
| Spielerkarten | 48 Stadtkarten + 5 Ereigniskarten + 4/5/6 Epidemiekarten |
| Infektionskarten | 48 (eine pro Stadt) |
| Rollen | 7, je Partie eine pro Person |
| Ausbruchsleiste | 0–8 |
| Infektionsrate | 2, 2, 2, 3, 3, 4, 4 |

Die Farbe einer Stadt ist fest und bestimmt, welche Seuche dort ausbricht — nicht,
welche Würfel dort liegen dürfen. Durch Ausbrüche landen auch fremdfarbige Würfel
in einer Stadt.

**Startstadt:** Atlanta (Nr. 2), Sitz des Instituts. Dort steht zu Beginn die erste
Forschungsstation, dort starten alle Figuren.

---

## 2. Aufbau

1. Forschungsstation nach Atlanta, alle Figuren nach Atlanta.
2. Ausbruchsmarker auf 0, Infektionsrate auf das erste Feld (2).
3. Infektionsdeck mischen. Drei Karten aufdecken, je **3** Würfel der Stadtfarbe in
   diese Städte. Drei weitere: je **2** Würfel. Drei weitere: je **1** Würfel.
   Alle neun Karten auf den Infektionsablagestapel.
4. Jede Person zieht eine Rolle (verdeckt gemischt, nie zwei gleiche). In der App
   dürfen die Rollen bis zum ersten Zug frei getauscht werden — jede Rolle bleibt
   dabei einmalig, eine vergebene steht nicht mehr zur Wahl.
5. Spielerdeck aus 48 Stadt- + 5 Ereigniskarten mischen und austeilen:

   | Personen | Handkarten zu Beginn |
   |---|---|
   | 2 | 4 |
   | 3 | 3 |
   | 4 | 2 |

6. Epidemiekarten einmischen: Restdeck in *n* möglichst gleich große Stapel teilen
   (*n* = Anzahl Epidemiekarten), in jeden Stapel eine Epidemiekarte mischen,
   Stapel wieder aufeinander legen. Dadurch kommt in jedem Deckabschnitt genau eine
   Epidemie, aber an unbekannter Stelle.

   | Schwierigkeit | Epidemiekarten |
   |---|---|
   | Einführung | 4 |
   | Normal | 5 |
   | Heroisch | 6 |

7. Beginn: Wer die höchste einzelne Stadtbevölkerung auf der Hand hat. (Umsetzung:
   die Stadt mit der höchsten hinterlegten Einwohnerzahl; bei Gleichstand der
   niedrigere Sitzplatz.)

---

## 3. Spielzug

Ein Zug besteht aus drei Phasen in dieser Reihenfolge:

1. **Bis zu 4 Aktionen** ausführen (Wiederholungen erlaubt, Verzicht erlaubt).
2. **2 Spielerkarten ziehen.**
3. **Städte infizieren** — so viele Karten wie die aktuelle Infektionsrate.

Danach ist die nächste Person am Zug.

### 3.1 Aktionen

**Bewegung**

- **Fahrt/Fähre** — in eine per Linie verbundene Nachbarstadt.
- **Direktflug** — Stadtkarte abwerfen und in genau diese Stadt ziehen.
- **Charterflug** — die Karte der Stadt abwerfen, **in der man steht**, und in eine
  beliebige Stadt ziehen.
- **Shuttleflug** — von einer Stadt mit Forschungsstation in eine andere Stadt mit
  Forschungsstation.

**Sonstige**

- **Forschungsstation bauen** — Karte der eigenen Stadt abwerfen. Sind alle 6
  Stationen im Spiel, wird eine bestehende Station versetzt.
- **Seuche behandeln** — einen Würfel aus der eigenen Stadt entfernen. Ist die
  Seuche dieser Farbe bereits **geheilt**, werden stattdessen **alle** Würfel dieser
  Farbe aus der Stadt entfernt.
- **Wissen austauschen** — nur mit einer Person in derselben Stadt und nur mit der
  Karte **dieser** Stadt: geben oder nehmen. Beide müssen einverstanden sein; die
  Aktion zählt für die Person, die am Zug ist.
- **Heilmittel entdecken** — in einer Stadt mit Forschungsstation **5 Stadtkarten
  derselben Farbe** abwerfen. Die Seuche gilt als geheilt. Sind zu diesem Zeitpunkt
  keine Würfel dieser Farbe mehr auf dem Plan, gilt sie sofort als **ausgerottet**.

### 3.2 Karten ziehen

Zwei Karten vom Spielerdeck. Ist das Deck leer und muss gezogen werden, ist das
Spiel **sofort verloren**.

**Epidemiekarte** (wird sofort abgehandelt, dann abgelegt):

1. **Steigern** — Infektionsrate ein Feld weiter.
2. **Infizieren** — die **unterste** Karte des Infektionsdecks aufdecken, **3**
   Würfel der Stadtfarbe dorthin (bzw. so viele, bis 3 erreicht sind; liegt dort
   schon ein Würfel, kommt es zum Ausbruch). Karte auf den Ablagestapel.
3. **Verdichten** — Infektionsablagestapel mischen und **oben** auf das
   Infektionsdeck legen.

Werden zwei Epidemien in einem Zug gezogen, wird die erste vollständig abgehandelt,
bevor die zweite gezogen wird.

**Handlimit:** 7 Karten. Wer darüber liegt, wirft sofort ab oder spielt
Ereigniskarten, bis 7 erreicht sind.

### 3.3 Infektionsphase

So viele Infektionskarten aufdecken, wie die Infektionsrate angibt. In jede
aufgedeckte Stadt kommt **1** Würfel ihrer Farbe; Karte auf den Ablagestapel.

- Ist die Seuche dieser Farbe **ausgerottet**, passiert nichts.
- Liegen in der Stadt bereits **3** Würfel dieser Farbe, kommt es zum **Ausbruch**.

---

## 4. Ausbruch

Ein Ausbruch tritt ein, sobald in einer Stadt ein **vierter** Würfel einer Farbe
läge.

1. Ausbruchsmarker ein Feld weiter.
2. In **jede** verbundene Nachbarstadt **1** Würfel dieser Farbe.
3. Löst das dort einen weiteren Ausbruch aus, wird dieser danach abgehandelt —
   **jede Stadt bricht pro Kettenreaktion aber nur einmal aus**.

Der vierte Würfel selbst wird nicht platziert; er bleibt im Vorrat.

Reicht der Würfelvorrat einer Farbe nicht aus, um alle nötigen Würfel zu setzen,
ist das Spiel **sofort verloren**.

---

## 5. Heilen und Ausrotten

- **Geheilt:** Heilmittel entdeckt. Behandeln entfernt ab jetzt alle Würfel dieser
  Farbe aus einer Stadt auf einmal. Neue Würfel kommen weiterhin.
- **Ausgerottet:** geheilt **und** kein Würfel dieser Farbe mehr auf dem Plan.
  Infektionskarten dieser Farbe bewirken nichts mehr, auch bei Epidemien nicht.
  Der Zustand wird geprüft, sobald der letzte Würfel entfernt wird oder das
  Heilmittel entdeckt wird, und kann nicht zurückfallen.

---

## 6. Spielende

**Gewonnen**, sobald alle vier Heilmittel entdeckt sind — sofort, unabhängig von
Würfeln auf dem Plan. Es muss nicht ausgerottet werden.

**Verloren** bei einem von drei Ereignissen:

1. Der Ausbruchsmarker erreicht **8**.
2. Ein Würfelvorrat reicht nicht für eine nötige Platzierung.
3. Das Spielerdeck ist leer und es muss gezogen werden.

---

## 7. Rollen

Jede Rolle hat eine Dauerfähigkeit. Fähigkeiten, die Aktionen ersetzen, kosten
weiterhin eine Aktion, sofern nicht anders angegeben.

| Rolle | Fähigkeit |
|---|---|
| **Ärztin** | *Behandeln* entfernt immer **alle** Würfel einer Farbe aus der Stadt, auch ohne Heilmittel. Betritt sie eine Stadt (oder wird sie bewegt), werden Würfel **geheilter** Seuchen dort ohne Aktion entfernt; sie verhindert auch, dass dort neue Würfel geheilter Farben platziert werden. |
| **Forscherin** | Beim *Wissen austauschen* darf sie **jede** Stadtkarte abgeben, nicht nur die der aktuellen Stadt. Nehmen von ihr geht ebenfalls mit jeder Karte. |
| **Wissenschaftler** | Braucht nur **4** gleichfarbige Stadtkarten für ein Heilmittel. |
| **Logistikerin** | Darf als Aktion eine **fremde** Figur bewegen, als wäre es die eigene (Karten kommen dabei aus der eigenen Hand), und darf eine Figur auf das Feld einer beliebigen anderen Figur ziehen. Bewegungen fremder Figuren brauchen deren Einverständnis. |
| **Bautechniker** | Baut eine Forschungsstation **ohne** Karte abzuwerfen. Einmal pro Zug darf er von einer Station aus in eine beliebige Stadt ziehen, indem er eine beliebige Stadtkarte abwirft. |
| **Quarantänebeauftragte** | In ihrer Stadt und allen Nachbarstädten werden **keine** Würfel platziert und es gibt dort **keine** Ausbrüche — egal aus welcher Quelle. Sie verhindert keine Ausbrüche, die woanders starten und nur Würfel weitertragen würden; solche Würfel kommen schlicht nicht an. |
| **Krisenplaner** | Darf als Aktion eine **Ereigniskarte vom Ablagestapel** nehmen und auf der Rolle ablegen (nur eine gleichzeitig; zählt nicht zum Handlimit). Wird sie gespielt, verlässt sie das Spiel. |

---

## 8. Ereigniskarten

Ereigniskarten zählen zum Handlimit, kosten **keine** Aktion und dürfen jederzeit
gespielt werden — auch im Zug einer anderen Person, auch mitten in einer
Kartenzieh- oder Infektionsphase, aber nie mitten in der Abhandlung einer einzelnen
Karte oder einer Ausbruchskette.

| Karte | Wirkung |
|---|---|
| **Ruhige Nacht** | Die nächste Infektionsphase entfällt vollständig. |
| **Prognose** | Die obersten **6** Karten des Infektionsdecks ansehen, in beliebiger Reihenfolge zurücklegen. |
| **Sonderbudget** | Eine Forschungsstation in eine beliebige Stadt setzen, ohne Karte abzuwerfen. |
| **Lufttransport** | Eine beliebige Figur in eine beliebige Stadt setzen (Einverständnis der betroffenen Person nötig). |
| **Zähe Bevölkerung** | Eine Karte aus dem **Infektionsablagestapel** aus dem Spiel nehmen. Nicht während der Abhandlung einer Epidemie zwischen *Infizieren* und *Verdichten* gespielt — dort gilt die übliche Fensterregel. |

---

## 9. Offene Informationen

Offen für alle: Anzahl und Farbe aller Würfel, Ablagestapel (beide), Ausbruchs- und
Ratenstand, Restkartenzahl beider Decks, alle Rollen. Handkarten sind offen,
Diskussion ist Teil des Spiels — Absprachen sind ausdrücklich erlaubt.

Verdeckt: Reihenfolge beider Decks.

Daraus folgt für den Netzwerkmodus: Es gibt **keine** geheime Information zwischen
Mitspielenden. Der gesamte Zustand außer der Deckreihenfolge darf an alle Clients
gespiegelt werden; nur das Mischen bleibt beim Host.


---

## 10. Erweiterungen

Ja, die lassen sich als **Module** zuschalten. Das Datenmodell hat die Plätze dafür
schon (`Modules` in `src/core/State.h`, reservierte Karten-, Rollen- und Farbnummern), damit
ein später eingeschaltetes Modul keine Nummern verschiebt und alte Spielstände
lesbar bleiben. Implementiert ist noch keines.

Die PC-Fassung bietet die Tischerweiterungen als Zusatzinhalte an; welche davon dort
genau enthalten sind, müsste ich nachsehen — als Modul umsetzbar sind alle drei.

### Erste Erweiterung („On the Brink")

| Modul | Was es ändert | Was das Modell dafür braucht |
|---|---|---|
| Virulenter Stamm | Epidemiekarten bekommen einzelne Zusatzeffekte | eigene Kartennummer je Epidemiekarte — bereits vorgesehen (`Epidemic`) |
| Mutation | fünfte, violette Seuche mit 12 Würfeln, ohne eigene Städte | fünfter Farbplatz — bereits vorgesehen (`Colour::Purple`, `kColourSlots`) |
| Bioterrorist | eine Person spielt **verdeckt gegen** das Team | **einziges Modul mit geheimer Information**: bricht die Annahme, dass der ganze Zustand gespiegelt werden darf; braucht einen eigenen Protokollpfad |
| Quarantänemarker | Städte lassen sich sperren | ein Byte Zusatzzustand je Stadt |
| 5. Person, Legendär | fünfter Platz, 7 Epidemien | `kMaxPlayers = 5`, `Difficulty::Legendary` — vorhanden |
| 8 weitere Rollen, 4 Ereignisse | — | Rollen ab Nummer 7, Ereignisse ab 5 |

### Zweite Erweiterung („In the Lab")

| Modul | Was es ändert | Was das Modell dafür braucht |
|---|---|---|
| Labor | Heilen wird ein mehrstufiges Verfahren (Proben sammeln, sequenzieren, testen) | Laborzustand je Farbe und Probenbestand je Person |
| Weltweite Panik | Städte geraten in Panik und fallen, Bewegung wird eingeschränkt | Statusbyte je Stadt |
| Teamspiel | zwei Zweierteams, teils gegeneinander | Teamzuordnung je Platz, dazu teilweise verdeckte Hände |
| weitere Rollen und Ereignisse | — | siehe oben |

### Dritte Erweiterung („State of Emergency")

| Modul | Was es ändert | Was das Modell dafür braucht |
|---|---|---|
| Hinterland | **neue Städte** samt Kanten | Städte hängen hinter Nr. 47 an; `kCities` wird zur Laufzeitgröße, Kartendeck wächst mit |
| Notfallereignisse | Ereignisse, die gegen das Team wirken | eigener Kartenbereich |
| Superkeim | zusätzliche Seuchenstufe | weiterer Farbplatz |
| Quarantänen | wie oben | — |

### Reihenfolge

Zuerst das Grundspiel vollständig (Regelkern und Oberfläche — beides steht).
Danach sind
**Virulenter Stamm** und **Mutation** die kleinsten Schritte, weil sie nur an
Epidemie und Farbzahl hängen. **Labor** und **Weltweite Panik** sind je ein eigenes
Regelpaket. **Bioterrorist** kommt zuletzt, weil er den Netzwerkteil umbaut —
solange er aus ist, bleibt der Zustand für alle offen und der LAN-Modus einfach.
