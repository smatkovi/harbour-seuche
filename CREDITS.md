# Herkunft der Daten

## Spielplan

Die 48 Städte, ihre Farben und ihre Verbindungen folgen dem Vorbild (siehe
`spec/karte.md`). Namensschreibung, Koordinaten und Einwohnerzahlen sind eigene
Angaben; die Einwohnerzahlen entscheiden nur, wer beginnt.

## Landhintergrund (`sailfish/world.png`)

Auf dem Gerät gibt es keine Küstenlinien. Der Hintergrund ist deshalb aus den
**Flugplatzdaten von FlightGear** gerechnet, die harbour-fgview mitbringt
(`/usr/share/harbour-fgview/airports`, rund 27 000 Punkte): rastern, aufblasen,
Löcher schließen, Kanten glätten — `tools/make_world.py`.

Das Ergebnis ist eine grobe Landmaske, kein Kartenwerk. Sie dient der
Orientierung; welche Städte verbunden sind, sagen allein die Linien.

## Rollen- und Ereignisnamen, Grafik

Eigene Bezeichnungen und eigenes Symbol (`tools/make_icon.py`).
