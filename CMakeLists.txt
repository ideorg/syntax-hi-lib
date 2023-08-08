cmake_minimum_required(VERSION 3.5)

project(SyntaxHighlighting VERSION 1.0 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6)
find_package(Qt6 REQUIRED COMPONENTS Widgets Network)

qt6_add_resources(QRC_SOURCES data/themes/theme-data.qrc)

file(GLOB libSRC
        "lib/*.cpp"
)

add_library(${PROJECT_NAME}
        ${libSRC}
        ${QRC_SOURCES}
)


target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets Qt6::Network)