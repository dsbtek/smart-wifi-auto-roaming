cmake_minimum_required(VERSION 3.16)
project(SmartAutoRoam VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Sql Core Gui)
find_package(Qt6 COMPONENTS Charts QUIET)

set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/NetworkScanner.cpp
    src/ConnectionManager.cpp
    src/Database.cpp
    src/ConfigManager.cpp
    src/RoamingManager.cpp
)

set(HEADERS
    src/MainWindow.h
    src/NetworkScanner.h
    src/ConnectionManager.h
    src/Database.h
    src/ConfigManager.h
    src/RoamingManager.h
)

qt6_wrap_ui(UI_HEADERS ui/MainWindow.ui)

qt_add_resources(RCC resources.qrc)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${UI_HEADERS} ${RCC})

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets Qt6::Sql Qt6::Core Qt6::Gui)
if(Qt6Charts_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Charts)
endif()

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
