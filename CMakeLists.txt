cmake_minimum_required(VERSION 3.15)
project(HuffPressor)

# Use modern C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Create a static library for the core logic
add_library(huffpressor_lib STATIC
    src/compressor.cpp
    src/decompressor.cpp
    src/utils.cpp
    src/huffmanTree.cpp
    src/bitWriter.cpp
    src/bitReader.cpp
)

# Include directories for the library
target_include_directories(huffpressor_lib
    PUBLIC ${CMAKE_SOURCE_DIR}/include
)

# Compiler options for the library
target_compile_options(huffpressor_lib
    PRIVATE -Wall -Wextra -pedantic -O2
)

# Set executable target (CLI tool)
add_executable(huffpressor_cli
    src/main.cpp
)

# Link the CLI tool against the core library
target_link_libraries(huffpressor_cli
    PRIVATE huffpressor_lib
)

# Compiler options for the CLI
target_compile_options(huffpressor_cli
    PRIVATE -Wall -Wextra -pedantic -O2
)

# ==========================================
# GUI Application (Qt)
# ==========================================

# Find Qt6 package
find_package(Qt6 COMPONENTS Widgets REQUIRED)

# Add GUI executable
add_executable(huffpressor_gui
    src/gui/main.cpp
    src/gui/mainwindow.cpp
    src/gui/mainwindow.h
)

# Link Qt libraries and core logic
target_link_libraries(huffpressor_gui
    PRIVATE Qt6::Widgets
    PRIVATE huffpressor_lib
)

# Enable C++20 and standard Qt settings
target_compile_options(huffpressor_gui
    PRIVATE -Wall -Wextra -pedantic -O2
)

# Handle Qt meta-object compilation (MOC)
set_property(TARGET huffpressor_gui PROPERTY CXX_STANDARD 20)
set_property(TARGET huffpressor_gui PROPERTY AUTOMOC ON)
set_property(TARGET huffpressor_gui PROPERTY AUTOUIC ON)
set_property(TARGET huffpressor_gui PROPERTY AUTORCC ON)
