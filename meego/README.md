# Seuche für MeeGo Harmattan (Nokia N9)

Dasselbe Spiel wie die Sailfish-Fassung: derselbe Regelkern, dieselbe
`SeucheEngine`, dasselbe Netzwerkspiel. Nur die Oberfläche ist eigens
gebaut — Qt 4.7.4, QtQuick 1.1 und die `com.nokia.meego`-Komponenten.

## Was hier liegt

| Pfad | Wozu |
| --- | --- |
| `qml/` | Die N9-Oberfläche. Die Dateien heißen wie die unter `sailfish/`, damit sich beide nebeneinander lesen lassen. |
| `src/Theme.h` | `Theme` und `Style` als C++-Objekte: QtQuick 1.1 kennt kein `pragma Singleton`, und `com.nokia.meego` bringt kein `Theme` mit. |
| `src/LinkItem.{h,cpp}` | Die 93 Verbindungslinien des Spielplans. QtQuick 1.1 hat kein `Canvas`. |
| `main.cpp` | Einstiegspunkt: `QDeclarativeView`, die Engine und die beiden Objekte oben als Kontexteigenschaften. `SEUCHE_ROOT` und `SEUCHE_WINDOWED` sind Hilfen für Läufe am Schreibtisch. |
| `compat/` | Qt-4-Ersatz für die Qt-5-Klassen, die der geteilte Code benutzt: `QJsonDocument`/`QJsonObject` über QtScripts JSON, `QStandardPaths`, `QGuiApplication` und die `QStringLiteral`-Makros (`qt4compat.h`, zwangsweise eingebunden). Aus harbour-snapszer übernommen. |
| `toolchain.sh` | Baut GCC 14 für `arm-none-linux-gnueabi` gegen das MADDE-Sysroot. |
| `build.sh` | Baut die N9-Binärdatei (`arm`), einen x86-Bau für den Qt-Simulator (`x86`) oder prüft die QML-Dateien (`check`). |
| `build-deb.sh` | Packt `build/meego/arm` als `harbour-seuche_<version>_armel.deb` (`mkdeb.py`, ohne dpkg). |
| `tests/` | Der QML-Prüfer und die Liste der QtQuick-1.1-Fallen. |

## Warum ein neues GCC

Harmattans eigene Werkzeugkette (MADDE, GCC 4.4) übersetzt den C++17-Regelkern
nicht. `toolchain.sh` baut GCC 14.2 gegen das MADDE-Sysroot (glibc 2.10,
Qt 4.7.4); libstdc++ und libgcc werden statisch eingebunden, sodass das Gerät
nur sein eigenes Qt braucht. Harmattan ist armv7-a, NEON, hartes Gleitkomma,
benutzt aber weiterhin `/lib/ld-linux.so.3` als Lader — daher
`-Wl,--dynamic-linker=/lib/ld-linux.so.3` in `build.sh`. Die Werkzeugkette
braucht etwa eine halbe Stunde und liegt unter `/tmp`; ein Tarball davon liegt
in `~/ps/toolchains`.

## Was sich im geteilten Code geändert hat

Alles mit `#if QT_VERSION`, die Sailfish-Fassung verhält sich unverändert:

* Qt 4 kennt kein `connect()` über Zeiger auf Elementfunktionen. Alles, was ein
  Signal erreicht, ist ein erklärter Slot; die Lambdas in `connect()` sind
  benannte Slots geworden und holen sich ihre Gegenstelle über `sender()`.
* `QHostAddress::AnyIPv4`, `isLoopback()` und `toIPv4Address(bool*)` gibt es
  erst ab Qt 5; `QList` hat kein `crbegin()`.

Qt 4s Sockets sind hier nur IPv4, der N9 eröffnet einen Tisch also nicht über
IPv6. Suchen und Beitreten funktionieren in beide Richtungen.

## Kein qsTr

In `qml/` steht kein `qsTr()`. Qt 4.7 liest den Quelltext von `qsTr()` als
Latin-1, und die Texte hier sind voller Umlaute. Eine Übersetzung gibt es
ohnehin nicht, also stehen die Zeichenketten unmittelbar da; `main.cpp` setzt
für die C++-Seite `QTextCodec::setCodecForTr(UTF-8)`.

## Bauen und prüfen

    meego/build.sh check     # QML gegen Qt 4 übersetzen, plus die Fallenliste
    meego/build.sh arm       # -> build/meego/arm/harbour-seuche
    meego/build-deb.sh       # -> build/meego/harbour-seuche_<version>_armel.deb

`build.sh check` übersetzt jede QML-Datei mit einem Qt 4 und meldet unbekannte
Typen und Eigenschaften. Die Seiten selbst überspringt er: das Plugin von
`com.nokia.meego` legt beim Laden ein QWidget an und bricht ohne Bildschirm
ab — geprüft werden damit die eigenen Bausteine und `Board.qml`, wo die meiste
Rechnerei steckt. `tests/lint-qml.py` läuft danach über alle Dateien und sucht
nach dem, was QtQuick 1.1 und die JavaScript-Maschine von Qt 4.7 nicht können:
`readonly property`, `Canvas`, `qsTr`, `let`/`const`/Pfeilfunktionen, und
Silica-Reste aus der Vorlage.

## Auf dem N9 installieren

Entwicklermodus, dann im Terminal oder über SSH:

    devel-su dpkg -i harbour-seuche_0.5.0_armel.deb

## Stand

* [x] Regelkern, `SeucheEngine`, LAN-Schicht und Drahtformat übersetzen und
      binden gegen Qt 4.7.4. Die ARM-Binärdatei ist ELF 32-bit ARM EABI5 mit
      Lader `/lib/ld-linux.so.3` und braucht außer dem Qt 4 des Geräts nichts.
* [x] Oberfläche vollständig: Startseite, Rollenwahl, Spielplan klein und
      groß, Aktionen, Hände, Ereigniskarten, Prognose, Protokoll, Netzwerk,
      Rückgängig und die Frage nach der letzten Aktion.
* [x] `.deb` baut und ist richtig aufgebaut.
* [ ] **Noch nichts davon lief auf dem Gerät.** Der Baurechner hat keinen
      X-Server, also konnte auch der Qt-Simulator die Oberfläche nicht
      anzeigen. Geprüft sind: Übersetzen und Binden für ARM, das Übersetzen
      der QML-Bausteine und von `Board.qml` gegen Qt 4, die Fallenliste, und
      der Aufbau des Pakets. Ungeprüft sind Aussehen, Bedienung und alles,
      was erst beim Erzeugen der Objekte auffällt.
