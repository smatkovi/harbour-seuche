# Seuche — Netzwerkspiel

Zwei bis vier Geräte im selben WLAN. Ein Gerät eröffnet den Tisch, die anderen
treten bei. Umgesetzt in `src/net/` und in `SeucheEngine`.

## Warum es so einfach sein darf

Im kooperativen Spiel gibt es **keine geheime Information zwischen den
Mitspielenden**: Handkarten, Würfel, Stapelablagen, Rollen — alles liegt offen
(`regeln.md` §9). Verdeckt ist nur die **Reihenfolge** der beiden Nachziehstapel.

Daraus folgt der ganze Entwurf:

- Der Gastgeber hält die einzige echte Partie und ist das einzige Gerät, das sie
  verändert. Nur dort wird gemischt und gezogen.
- Gäste bekommen den vollständigen Zustand **außer** der Deckreihenfolge; die
  Decks kommen als blanke Karten in der richtigen Anzahl an, damit jede Zahl und
  jede Regelprüfung stimmt.
- Gäste rechnen ihre legalen Aktionen selbst aus (dafür braucht es keine
  Deckreihenfolge) und schicken die **gewählte Aktion**, nicht deren Platz in
  einer Liste. Der Gastgeber prüft sie erneut mit demselben Regelkern.
- Betrugsschutz ist unnötig — niemand spielt gegen die anderen. Der Gastgeber
  prüft trotzdem jede Nachricht, aber gegen Versehen, nicht gegen Absicht.

Das einzige Modul, das diesen Entwurf bricht, wäre der **Bioterrorist** aus der
ersten Erweiterung: dort zieht eine Person verdeckt gegen das Team. Der bräuchte
einen eigenen Pfad mit Sichtfiltern je Empfänger (`regeln.md` §10).

## Transport

`src/net/LanSession.*` stammt aus harbour-snapszer und ist spielunabhängig: TCP
für die Partie, UDP für die Suche, Zeilen-JSON als Nachrichtenformat, Ping und
Leerlauferkennung. Übernommen mit eigenen Kennungen, damit sich die beiden Spiele
im selben Netz nie gegenseitig finden:

| | Snapszer | Seuche |
|---|---|---|
| Spielport (TCP) | 45465 | **45475** |
| Suchport (UDP) | 45466 | **45476** |
| Suchwort | `SNAPSZER-DISCOVER` | `SEUCHE-DISCOVER 1` |
| Antwort | `SNAPSZER-HOST` | `SEUCHE-HOST 1 ` |

## Nachrichten

Gastgeber → Gäste:

| Nachricht | Inhalt |
|---|---|
| `welcome` | `seat` — welcher Sitz diesem Gerät gehört (−1 = Zuschauer) |
| `state` | `g` = ganzer Zustand (`src/net/Snapshot.*`), `journal`, `owners` |

Gäste → Gastgeber:

| Nachricht | Inhalt |
|---|---|
| `action` | `a` = die gewählte Aktion mit allen Feldern |
| `step` | `s` = `draw` oder `infect` |
| `discard` | `card` = abgeworfene Karte |
| `event` | `e` = Ereigniskarte samt Zielangaben |

Nach **jeder** Änderung schickt der Gastgeber einen vollständigen `state`. Das
sind ein paar Kilobyte und macht Teilaktualisierungen und deren Fehlerquellen
überflüssig.

## Sitze

Sitz 1 bleibt beim Gastgeber, jedes beitretende Gerät bekommt den nächsten freien.
Nicht vergebene Sitze spielt der Gastgeber mit — ein Tisch ist also immer
vollständig besetzt, egal wie viele Geräte da sind. Trennt sich ein Gerät, fällt
sein Sitz an den Gastgeber zurück.

Ein Gerät darf nur den Sitz bewegen, der ihm gehört; `mayAct` in der Oberfläche
schaltet Karte, Knöpfe und Aktionsliste entsprechend frei.

## Geprüft

`tests/test_lan.cpp` startet einen Gastgeber und zwei Gäste in einem Prozess über
echtes TCP auf der Loopback-Adresse und spielt eine ganze Partie über die
Leitung. Geprüft wird: gleicher Zustand auf allen drei Geräten (Fingerabdruck aus
Phase, Zug, Würfeln, Stationen, Händen), dass ein fremder Sitz sich nicht bewegen
lässt, dass immer genau ein Gerät am Zug ist, dass Protokoll und Ausgang überall
gleich ankommen, und dass ein verlorener Gast seinen Sitz zurückgibt.
