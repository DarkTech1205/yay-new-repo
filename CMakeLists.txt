cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

project(HowToRuinGD VERSION 1.1.0)

file(GLOB SOURCES CONFIGURE_DEPENDS src/*.cpp)
add_library(${PROJECT_NAME} SHARED ${SOURCES})

# Pinned to Geode 6.0.0-beta.1 (mod.json "geode"). GEODE_SDK must point at a checkout of tag v6.0.0-beta.1.
# For reproducible bindings also set GEODE_BINDINGS_REPO_PATH to commit 78d5bb4eb0732dd636ae16d4dcf6e5ce84ec1221.
if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Set GEODE_SDK to a checkout of geode-sdk/geode at tag v6.0.0-beta.1")
endif()

set(SKIP_BUILDING_CODEGEN OFF CACHE BOOL "" FORCE)
add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

setup_geode_mod(${PROJECT_NAME})
