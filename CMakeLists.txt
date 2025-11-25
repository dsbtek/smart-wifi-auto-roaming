cmake_minimum_required(VERSION 3.16)
project(ZHR VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/ui)

find_package(Qt6 REQUIRED COMPONENTS Widgets Sql Core Gui Network)
find_package(Qt6 COMPONENTS Charts QUIET)

set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/NetworkScanner.cpp
    src/ConnectionManager.cpp
    src/Database.cpp
    src/ConfigManager.cpp
    src/RoamingManager.cpp
    src/SpeedTester.cpp
)

set(HEADERS
    src/MainWindow.h
    src/NetworkScanner.h
    src/ConnectionManager.h
    src/Database.h
    src/ConfigManager.h
    src/RoamingManager.h
    src/SpeedTester.h
)

set(UI_FILES
    ui/MainWindow.ui
)

set(RESOURCES
    src/resources.qrc
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${UI_FILES} ${RESOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets Qt6::Sql Qt6::Core Qt6::Gui Qt6::Network)
if(Qt6Charts_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Charts)
    target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_QTCHARTS)
endif()

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)

# Install desktop file
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/ZHR.desktop
        DESTINATION share/applications)

# Install icon
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/resources/icons/wifi.svg
        DESTINATION share/icons/hicolor/scalable/apps
        RENAME zhr.svg)
