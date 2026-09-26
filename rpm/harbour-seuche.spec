Name: harbour-seuche
Version: 0.7.0
Release: 1
Summary: Kooperatives Seuchen-Brettspiel
License: GPL-3.0-or-later
URL: https://github.com/smatkovi/harbour-seuche
Source0: %{name}-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-root

Requires:       sailfishsilica-qt5
BuildRequires:  pkgconfig(sailfishapp)
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5DBus)
BuildRequires:  pkgconfig(Qt5Network)
BuildRequires:  pkgconfig(Qt5Qml)
BuildRequires:  pkgconfig(Qt5Quick)

%description
Seuche is a co-operative board game for two to four players on one device: four
diseases break out around the world, and the table wins only by discovering a
cure for all four before eight outbreaks happen, a cube supply runs dry or the
player deck runs out.

Play every seat on one device, or open a table in the local network and
let two to four phones share it: the host keeps the game, the guests
mirror it. The rule core is plain C++ and covered by its own tests; the
expansion modules have room in the model but are not built yet.

%prep
%setup -q

%build
mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/usr/bin
install -m 755 build/harbour-seuche %{buildroot}/usr/bin/

mkdir -p %{buildroot}/usr/share/%{name}/qml
cp -a sailfish/*.qml sailfish/qmldir sailfish/world.png %{buildroot}/usr/share/%{name}/qml/

mkdir -p %{buildroot}/usr/share/applications
install -m 644 sailfish/desktop/%{name}.desktop %{buildroot}/usr/share/applications/%{name}.desktop

for size in 86 108 128 172 256; do
    mkdir -p %{buildroot}/usr/share/icons/hicolor/${size}x${size}/apps
    install -m 644 sailfish/icons/icon-${size}.png \
        %{buildroot}/usr/share/icons/hicolor/${size}x${size}/apps/%{name}.png
done

# cp -a keeps the group-writable bits of the working tree; the package wants
# plain 755 directories and 644 files (rpmlint non-standard-dir-perm).
find %{buildroot}/usr/share/%{name} -type d -exec chmod 755 {} \;
find %{buildroot}/usr/share/%{name} -type f -exec chmod 644 {} \;

mkdir -p %{buildroot}/usr/share/doc/%{name}
install -m 644 README.md %{buildroot}/usr/share/doc/%{name}/
install -m 644 CREDITS.md %{buildroot}/usr/share/doc/%{name}/
install -m 644 spec/regeln.md %{buildroot}/usr/share/doc/%{name}/
mkdir -p %{buildroot}/usr/share/licenses/%{name}
install -m 644 LICENSE %{buildroot}/usr/share/licenses/%{name}/

%files
%defattr(-,root,root,-)
/usr/bin/%{name}
/usr/share/%{name}
/usr/share/icons/hicolor/*/apps/%{name}.png
/usr/share/applications/%{name}.desktop
/usr/share/doc/%{name}
/usr/share/licenses/%{name}

%changelog
* Wed Sep 24 2026 smatkovi - 0.6.1-1
- Wer im Netz gespielt hat — oder es versucht hat und nicht hingekommen ist —
  bekommt seine eigene angefangene Partie beim Verlassen des Netzes sofort
  zurück, statt die App dafür neu starten zu müssen.
- "Angefangene Partie verwerfen" räumt auch eine ausgespielte Partie weg.
* Wed Sep 24 2026 smatkovi - 0.6.0-1
- Die angefangene Partie überlebt das Schließen der App. Sie wird nach jeder
  Änderung weggeschrieben — nicht erst beim Beenden, denn ein Programm, das
  der Aufgabenverwalter abschießt, bekommt kein aboutToQuit mehr zu sehen —
  und ist beim nächsten Start wieder da; auf der Startseite steht dann
  "Laufende Partie fortsetzen".
- Gespeichert wird auch die Reihenfolge der beiden verdeckten Stapel und der
  Stand des Zufallsgenerators. Ohne das erste würden beim Weiterspielen leere
  Karten ausgeteilt, ohne das zweite ließe sich die nächste Mischung durch
  Schließen und Öffnen der App neu würfeln.
- Eine beendete Partie wird nicht fortgesetzt, und eine Netzpartie gar nicht
  erst gespeichert: wer an welchem Sitz sitzt, hängt an Verbindungen, die es
  nach einem Neustart nicht mehr gibt. Der eigene Spielstand bleibt dabei
  unangetastet liegen.
- Neu im Menü der Startseite: "Angefangene Partie verwerfen".
* Fri Sep 18 2026 smatkovi - 0.4.0-1
- Die Karte hat einen Landhintergrund: eine grobe Weltkarte, aus den
  FlightGear-Flugplatzdaten gerechnet, statt abstrakter Farbflächen
- Längentreues Seitenverhältnis, damit die Lage der Städte stimmt
- Neue Seite „Karte groß“: der Spielplan formatfüllend, im Querformat lesbar
- Würfel rechts, Figuren links, Station als Quadrat — feste Plätze statt
  wechselnder Stapel über und unter der Stadt
- Städtenamen immer dort, wo Würfel liegen oder die Figur am Zug steht

* Fri Sep 18 2026 smatkovi - 0.3.0-1
- Rollen sind frei wählbar, bis der erste Zug geschehen ist; jede Rolle nur
  einmal, auch über mehrere Geräte — eine vergebene steht nicht mehr zur Wahl
- Übersichtlichere Karte: farbige Flächen je Seuchengebiet, hellere Linien,
  die drei Verbindungen über den Pazifik laufen als Stummel über den Rand
  statt quer durch das Bild
- Zoomen mit zwei Fingern oder Knöpfen, Sprung zur Figur am Zug, Städtenamen
  erst ab genügend Vergrößerung, größere Antippflächen

* Fri Sep 18 2026 smatkovi - 0.2.0-1
- Netzwerkspiel im WLAN: ein Gerät eröffnet, zwei bis vier spielen zusammen
- Tische werden per UDP gefunden, Adresse geht auch von Hand
- Jedes Gerät bewegt nur seinen Sitz; freie Sitze spielt der Gastgeber mit
- Fällt ein Gerät aus, übernimmt der Gastgeber dessen Sitz

* Fri Sep 18 2026 smatkovi - 0.1.0-1
- Erste Fassung: vollständiges Grundspiel für 2 bis 4 Sitze an einem Gerät
- Spielplan mit 48 Städten, Aktionen, Epidemien, Ausbrüchen, Ereigniskarten
- Alle sieben Rollen, drei Schwierigkeitsgrade
