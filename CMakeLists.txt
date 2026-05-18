# Cypherush - Copyright (c) 2026 semihturker0. All Rights Reserved.
# Proprietary software. See LICENSE for terms.
cmake_minimum_required(VERSION 3.20)

project(Cypherush
    VERSION 0.1.0
    DESCRIPTION "End-to-End Encrypted Messaging"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_package(Qt6 REQUIRED COMPONENTS Core Network Widgets)
find_library(CRYPTOPP_LIB NAMES cryptopp libcryptopp)

add_subdirectory(common)
add_subdirectory(server)
add_subdirectory(client)
