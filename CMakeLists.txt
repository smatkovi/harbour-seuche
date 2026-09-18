# Sailfish OS build of harbour-seuche (Qt 5.6, Silica). The rule core in
# src/core/ is plain C++17 without Qt and is also built on its own by
# tools/test.sh.
cmake_minimum_required(VERSION 3.6)
project(harbour-seuche LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt5 REQUIRED COMPONENTS Core Qml Quick)
find_package(PkgConfig REQUIRED)
pkg_check_modules(SAILFISHAPP REQUIRED sailfishapp)

# Globbed on purpose: the core grows file by file, and a new rule file must not
# need a change here. Re-run cmake after adding one in a working tree.
file(GLOB SEUCHE_CORE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/core/*.cpp)
list(SORT SEUCHE_CORE_SOURCES)

add_executable(harbour-seuche
    src/main.cpp
    src/SeucheEngine.cpp
    ${SEUCHE_CORE_SOURCES}
)

target_include_directories(harbour-seuche PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${SAILFISHAPP_INCLUDE_DIRS}
)

target_link_libraries(harbour-seuche
    Qt5::Core Qt5::Qml Qt5::Quick ${SAILFISHAPP_LIBRARIES}
)

# The QML is not compiled in: rpm/harbour-seuche.spec copies sailfish/*.qml and
# sailfish/qmldir into /usr/share/harbour-seuche/qml.

# The two core test programs, the same ones tools/test.sh builds by hand.
option(SEUCHE_BUILD_TESTS "Build the rule core tests" OFF)
if(SEUCHE_BUILD_TESTS)
    enable_testing()
    foreach(t test_model test_rules)
        add_executable(${t} tests/${t}.cpp ${SEUCHE_CORE_SOURCES})
        target_include_directories(${t} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
        add_test(NAME ${t} COMMAND ${t})
    endforeach()

    # The QML facade, checked from a real QQmlEngine: a property that reaches
    # QML as `undefined` is invisible on the device but fails here.
    add_executable(test_qmlbridge tests/test_qmlbridge.cpp src/SeucheEngine.cpp ${SEUCHE_CORE_SOURCES})
    target_include_directories(test_qmlbridge PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
    target_link_libraries(test_qmlbridge Qt5::Core Qt5::Qml)
    add_test(NAME test_qmlbridge COMMAND test_qmlbridge)
endif()
