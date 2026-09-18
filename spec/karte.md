# Seuche — Spielplan

48 Städte, je 12 pro Farbe, 93 Verbindungen (Schnitt 3.88 pro Stadt).
Knoten, Farbzuordnung und Kanten entsprechen dem Vorbild; die erste
Forschungsstation steht im selben Knoten (**Atlanta**, Nr. 2), dort starten alle
Figuren. Eigen sind Namensschreibung, Koordinaten und Einwohnerzahlen (eigene
Schätzungen, sie entscheiden nur, wer beginnt).

Die Nummer ist zugleich die Kartennummer im Spielerdeck, im Speicherformat und im
Netzwerkprotokoll — sie darf sich nie ändern. Städte aus Erweiterungen hängen
hinten an.

Gradverteilung: 1×1, 4×2, 12×3, 16×4, 13×5, 2×6.

Hinter den Städten liegt in der App eine grobe Landmaske (`sailfish/world.png`,
erzeugt von `tools/make_world.py`). Sie ist Orientierung, keine Regel: verbunden
sind Städte genau dann, wenn diese Tabelle es sagt.

Erzeugt aus `src/core/Map.cpp` mit `tools/genmap.py`, nicht von Hand pflegen.

## Blau (Nordamerika, Europa)

| Nr. | Stadt | Schlüssel | Einw. | Grad | Verbindungen |
|---:|---|---|---:|---:|---|
| 0 | San Francisco | `sanfrancisco` | 5.9 | 4 | Chicago, Los Angeles, Tokio, Manila |
| 1 | Chicago | `chicago` | 9.5 | 5 | San Francisco, Atlanta, Montreal, Los Angeles, Mexiko-Stadt |
| 2 | Atlanta | `atlanta` | 6.1 | 3 | Chicago, Washington, Miami |
| 3 | Montreal | `montreal` | 4.3 | 3 | Chicago, New York, Washington |
| 4 | New York | `newyork` | 19.0 | 4 | Montreal, Washington, London, Madrid |
| 5 | Washington | `washington` | 6.3 | 4 | Atlanta, Montreal, New York, Miami |
| 6 | London | `london` | 9.5 | 4 | New York, Madrid, Paris, Essen |
| 7 | Madrid | `madrid` | 6.7 | 5 | New York, London, Paris, São Paulo, Algier |
| 8 | Paris | `paris` | 11.0 | 5 | London, Madrid, Essen, Mailand, Algier |
| 9 | Essen | `essen` | 5.1 | 4 | London, Paris, Mailand, Sankt Petersburg |
| 10 | Mailand | `mailand` | 4.3 | 3 | Paris, Essen, Istanbul |
| 11 | Sankt Petersburg | `stpetersburg` | 5.4 | 3 | Essen, Istanbul, Moskau |

## Gelb (Lateinamerika, Afrika)

| Nr. | Stadt | Schlüssel | Einw. | Grad | Verbindungen |
|---:|---|---|---:|---:|---|
| 12 | Los Angeles | `losangeles` | 13.2 | 4 | San Francisco, Chicago, Mexiko-Stadt, Sydney |
| 13 | Mexiko-Stadt | `mexikostadt` | 21.8 | 5 | Chicago, Los Angeles, Miami, Bogotá, Lima |
| 14 | Miami | `miami` | 6.2 | 4 | Atlanta, Washington, Mexiko-Stadt, Bogotá |
| 15 | Bogotá | `bogota` | 11.0 | 5 | Mexiko-Stadt, Miami, Lima, São Paulo, Buenos Aires |
| 16 | Lima | `lima` | 10.7 | 3 | Mexiko-Stadt, Bogotá, Santiago |
| 17 | Santiago | `santiago` | 7.1 | 1 | Lima |
| 18 | São Paulo | `saopaulo` | 22.4 | 4 | Madrid, Bogotá, Buenos Aires, Lagos |
| 19 | Buenos Aires | `buenosaires` | 15.4 | 2 | Bogotá, São Paulo |
| 20 | Lagos | `lagos` | 15.4 | 3 | São Paulo, Kinshasa, Khartum |
| 21 | Kinshasa | `kinshasa` | 17.1 | 3 | Lagos, Khartum, Johannesburg |
| 22 | Khartum | `khartum` | 6.3 | 4 | Lagos, Kinshasa, Johannesburg, Kairo |
| 23 | Johannesburg | `johannesburg` | 6.1 | 2 | Kinshasa, Khartum |

## Schwarz (Nordafrika, Osteuropa, Naher Osten, Südasien)

| Nr. | Stadt | Schlüssel | Einw. | Grad | Verbindungen |
|---:|---|---|---:|---:|---|
| 24 | Algier | `algier` | 3.4 | 4 | Madrid, Paris, Istanbul, Kairo |
| 25 | Istanbul | `istanbul` | 15.8 | 6 | Mailand, Sankt Petersburg, Algier, Moskau, Kairo, Bagdad |
| 26 | Moskau | `moskau` | 12.6 | 3 | Sankt Petersburg, Istanbul, Teheran |
| 27 | Kairo | `kairo` | 22.2 | 5 | Khartum, Algier, Istanbul, Bagdad, Riad |
| 28 | Bagdad | `bagdad` | 7.7 | 5 | Istanbul, Kairo, Teheran, Riad, Karatschi |
| 29 | Teheran | `teheran` | 9.6 | 4 | Moskau, Bagdad, Karatschi, Delhi |
| 30 | Riad | `riad` | 7.7 | 3 | Kairo, Bagdad, Karatschi |
| 31 | Karatschi | `karatschi` | 16.8 | 5 | Bagdad, Teheran, Riad, Delhi, Mumbai |
| 32 | Delhi | `delhi` | 32.9 | 5 | Teheran, Karatschi, Mumbai, Chennai, Kalkutta |
| 33 | Mumbai | `mumbai` | 21.3 | 3 | Karatschi, Delhi, Chennai |
| 34 | Chennai | `chennai` | 11.5 | 5 | Delhi, Mumbai, Kalkutta, Bangkok, Jakarta |
| 35 | Kalkutta | `kalkutta` | 15.1 | 4 | Delhi, Chennai, Hongkong, Bangkok |

## Rot (Ost- und Südostasien, Ozeanien)

| Nr. | Stadt | Schlüssel | Einw. | Grad | Verbindungen |
|---:|---|---|---:|---:|---|
| 36 | Peking | `peking` | 21.5 | 2 | Seoul, Schanghai |
| 37 | Seoul | `seoul` | 25.6 | 3 | Peking, Tokio, Schanghai |
| 38 | Tokio | `tokio` | 37.4 | 4 | San Francisco, Seoul, Schanghai, Osaka |
| 39 | Schanghai | `schanghai` | 27.1 | 5 | Peking, Seoul, Tokio, Hongkong, Taipeh |
| 40 | Hongkong | `hongkong` | 7.5 | 6 | Kalkutta, Schanghai, Taipeh, Bangkok, Ho-Chi-Minh-Stadt, Manila |
| 41 | Taipeh | `taipeh` | 7.0 | 4 | Schanghai, Hongkong, Osaka, Manila |
| 42 | Osaka | `osaka` | 19.1 | 2 | Tokio, Taipeh |
| 43 | Bangkok | `bangkok` | 10.5 | 5 | Chennai, Kalkutta, Hongkong, Ho-Chi-Minh-Stadt, Jakarta |
| 44 | Ho-Chi-Minh-Stadt | `hochiminh` | 9.3 | 4 | Hongkong, Bangkok, Manila, Jakarta |
| 45 | Manila | `manila` | 13.9 | 5 | San Francisco, Hongkong, Taipeh, Ho-Chi-Minh-Stadt, Sydney |
| 46 | Jakarta | `jakarta` | 10.6 | 4 | Chennai, Bangkok, Ho-Chi-Minh-Stadt, Sydney |
| 47 | Sydney | `sydney` | 5.3 | 3 | Los Angeles, Manila, Jakarta |
