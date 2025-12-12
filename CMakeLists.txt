cmake_minimum_required(VERSION 3.28.3)
project(towerDefense_cpp)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O3")
find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

include_directories(
    include/
    ${SFML_INCLUDE_DIRS}
)
set(SOURCES
    src/main.cpp
    src/Game.cpp
    src/Map.cpp
    src/Tower.cpp
    src/TowerTypes.cpp
    src/Enemy.cpp
    src/Projectile.cpp
    src/GameUI.cpp
)

set(HEADERS
    include/Game.h
    include/Map.h
    include/Tower.h
    include/TowerTypes.h
    include/Enemy.h
    include/Projectile.h
    include/GameUI.h
    include/ElementGraphique.h
)
add_executable(tower_defense ${SOURCES} ${HEADERS})

target_link_libraries(tower_defense
    sfml-graphics
    sfml-window
    sfml-system
)
