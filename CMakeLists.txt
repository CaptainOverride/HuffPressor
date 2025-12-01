cmake_minimum_required(VERSION 3.15)
project(HuffPressor)

# Use modern C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ==========================================
# Core Library
# ==========================================
add_library(HuffPressorCore STATIC
    src/core/compressor.cpp
    src/core/decompressor.cpp
    src/core/utils.cpp
    src/core/huffmanTree.cpp
    src/core/bitWriter.cpp
    src/core/bitReader.cpp
)

target_include_directories(HuffPressorCore
    PUBLIC ${CMAKE_SOURCE_DIR}/include
)

target_compile_options(HuffPressorCore
    PRIVATE -Wall -Wextra -pedantic -O2
)

# ==========================================
# CLI Application
# ==========================================
add_executable(HuffPressorCLI
    src/cli/main.cpp
)

target_link_libraries(HuffPressorCLI
    PRIVATE HuffPressorCore
)

target_compile_options(HuffPressorCLI
    PRIVATE -Wall -Wextra -pedantic -O2
)

# ==========================================
# GUI Application (Qt)
# ==========================================
find_package(Qt6 COMPONENTS Widgets REQUIRED)

add_executable(HuffPressor
    src/gui/main.cpp
    src/gui/mainwindow.cpp
    src/gui/mainwindow.h
)

target_link_libraries(HuffPressor
    PRIVATE Qt6::Widgets
    PRIVATE HuffPressorCore
)

target_compile_options(HuffPressor
    PRIVATE -Wall -Wextra -pedantic -O2
)

# Qt Configuration
set_property(TARGET HuffPressor PROPERTY CXX_STANDARD 20)
set_property(TARGET HuffPressor PROPERTY AUTOMOC ON)
set_property(TARGET HuffPressor PROPERTY AUTOUIC ON)
set_property(TARGET HuffPressor PROPERTY AUTORCC ON)

# Ensure the executable is named correctly on Windows (HuffPressor.exe)
set_target_properties(HuffPressor PROPERTIES OUTPUT_NAME "HuffPressor")
set_target_properties(HuffPressorCLI PROPERTIES OUTPUT_NAME "HuffPressorCLI")
