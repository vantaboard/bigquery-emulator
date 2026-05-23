# Top-level CMake build for the BigQuery emulator's C++ engine.
#
# Phase 0 status: this file builds the placeholder `emulator_main` binary
# without any GoogleSQL dependency, so the C++ side is at least
# compilable. Phase 2 of ROADMAP.md replaces this with a real build that
# links GoogleSQL (preferring Bazel; CMake is the fallback).

cmake_minimum_required(VERSION 3.20)

project(bigquery_emulator CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

include_directories(${CMAKE_SOURCE_DIR})

add_library(frontend_server STATIC
  frontend/server/server.cc
)
target_include_directories(frontend_server PUBLIC ${CMAKE_SOURCE_DIR})

add_executable(emulator_main
  binaries/emulator_main/main.cc
)
target_link_libraries(emulator_main PRIVATE frontend_server)
