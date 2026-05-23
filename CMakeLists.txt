# Top-level CMake build for the BigQuery emulator's C++ engine.
#
# This file is the CMake fallback for the build (Bazel will eventually
# be the canonical one once GoogleSQL lands in Phase 4/5). For now it
# does enough to:
#
#   * Run `protoc` + `grpc_cpp_plugin` against proto/emulator.proto to
#     produce `emulator.pb.{h,cc}` and `emulator.grpc.pb.{h,cc}` under
#     the build directory.
#   * Bundle those generated files into a `enginepb_cc` static library
#     that transitively pulls in `protobuf::libprotobuf` and
#     `gRPC::grpc++`.
#   * Link the library into the existing `frontend_server` target so
#     `emulator_main` ends up with the gRPC runtime ready for the
#     Catalog / Query services to be wired in next phase.

cmake_minimum_required(VERSION 3.20)

project(bigquery_emulator CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

include_directories(${CMAKE_SOURCE_DIR})

# ---------------------------------------------------------------------------
# Proto / gRPC codegen
#
# We deliberately use whichever protoc / libprotobuf the host
# distribution ships (find_package(Protobuf) prefers the system copy)
# so the generated .pb.{h,cc} headers stay ABI-compatible with the
# libprotobuf we link. The mise-pinned `protoc` (35.0) is only used by
# the Go codegen (see buf.gen.yaml); mixing protoc 35 with the
# libprotobuf 3.x runtime that Ubuntu/Debian package today would mean
# generated code references symbols the runtime does not yet export.
# ---------------------------------------------------------------------------

# Force find_package(Protobuf) to ignore whatever protoc is first on the
# developer's PATH (mise pins a far newer one for the Go side) and use
# the one shipped with libprotobuf instead, so generated C++ stays ABI-
# compatible with the runtime we link against.
if(NOT Protobuf_PROTOC_EXECUTABLE)
  find_program(_system_protoc protoc
    HINTS /usr/bin /usr/local/bin
    NO_DEFAULT_PATH)
  if(_system_protoc)
    set(Protobuf_PROTOC_EXECUTABLE "${_system_protoc}" CACHE FILEPATH
      "Path to protoc used for C++ codegen")
  endif()
endif()

find_package(Protobuf REQUIRED)
find_package(gRPC CONFIG REQUIRED)
find_package(Threads REQUIRED)

# Abseil ships with the gRPC packages on every distro we support and
# is already a transitive runtime dep through gRPC::grpc++; we depend
# on it directly for absl::Status / absl::StatusOr in the
# backend/{schema,storage,engine} interfaces, so request the targets
# explicitly so the linker resolves them even on stripped-down
# installations.
find_package(absl CONFIG REQUIRED)

set(PROTO_SOURCE_DIR ${CMAKE_SOURCE_DIR}/proto)
set(PROTO_GEN_DIR ${CMAKE_BINARY_DIR}/generated/proto)
file(MAKE_DIRECTORY ${PROTO_GEN_DIR})

set(EMULATOR_PROTO ${PROTO_SOURCE_DIR}/emulator.proto)

# Locate the gRPC C++ plugin. CONFIG-mode gRPC exports it as a target;
# fall back to a pkg-config / PATH lookup if the export is unavailable
# (older distro packages).
if(TARGET gRPC::grpc_cpp_plugin)
  set(_grpc_cpp_plugin "$<TARGET_FILE:gRPC::grpc_cpp_plugin>")
else()
  find_program(_grpc_cpp_plugin grpc_cpp_plugin REQUIRED)
endif()

set(EMULATOR_GENERATED_SOURCES
  ${PROTO_GEN_DIR}/emulator.pb.cc
  ${PROTO_GEN_DIR}/emulator.pb.h
  ${PROTO_GEN_DIR}/emulator.grpc.pb.cc
  ${PROTO_GEN_DIR}/emulator.grpc.pb.h
)

add_custom_command(
  OUTPUT ${EMULATOR_GENERATED_SOURCES}
  COMMAND protobuf::protoc
  ARGS
    --cpp_out=${PROTO_GEN_DIR}
    --grpc_out=${PROTO_GEN_DIR}
    --plugin=protoc-gen-grpc=${_grpc_cpp_plugin}
    -I ${PROTO_SOURCE_DIR}
    ${EMULATOR_PROTO}
  DEPENDS ${EMULATOR_PROTO}
  COMMENT "protoc + grpc_cpp_plugin: proto/emulator.proto -> ${PROTO_GEN_DIR}"
  VERBATIM
)

add_library(enginepb_cc STATIC
  ${PROTO_GEN_DIR}/emulator.pb.cc
  ${PROTO_GEN_DIR}/emulator.grpc.pb.cc
)
target_include_directories(enginepb_cc
  PUBLIC
    ${PROTO_GEN_DIR}
)
target_link_libraries(enginepb_cc
  PUBLIC
    protobuf::libprotobuf
    gRPC::grpc++
    gRPC::grpc++_reflection
    Threads::Threads
)

# ---------------------------------------------------------------------------
# Backend interfaces (Phase 3a)
#
# Three libraries that mirror the directory layout under backend/:
#   * backend_schema:  proto FieldSchema <-> internal ColumnSchema
#                      conversions plus the engine-agnostic schema
#                      types every storage backend speaks.
#   * backend_storage: Storage abstract interface (CreateDataset,
#                      DropTable, AppendRows, ScanRows, ...) plus the
#                      `Value` cell representation. NO engine-specific
#                      types in any signature (see ROADMAP).
#   * backend_engine:  Engine abstract interface (Analyze, DryRun,
#                      ExecuteQuery). Concrete impls land in Phase 5.
#
# Phase 3b's `InMemoryStorage`, Phase 3c's CLI scaffold and Phase 5's
# engine implementations link against these libraries.
# ---------------------------------------------------------------------------

add_library(backend_schema STATIC
  backend/schema/schema.cc
)
target_include_directories(backend_schema PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(backend_schema
  PUBLIC
    enginepb_cc
    absl::status
    absl::statusor
    absl::strings
)

add_library(backend_storage STATIC
  backend/storage/storage.cc
)
target_include_directories(backend_storage PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(backend_storage
  PUBLIC
    backend_schema
    absl::status
    absl::statusor
    absl::strings
    absl::span
)

add_library(backend_engine STATIC
  backend/engine/engine.cc
)
target_include_directories(backend_engine PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(backend_engine
  PUBLIC
    backend_schema
    backend_storage
    absl::status
    absl::statusor
    absl::strings
)

# ---------------------------------------------------------------------------
# Storage implementations (Phase 3b+)
#
# Concrete Storage backends each get their own library so callers can
# link in only the ones they need (the CLI in Phase 3c picks per the
# `--storage` flag; the test binary just links the memory store).
# ---------------------------------------------------------------------------

add_library(backend_storage_memory STATIC
  backend/storage/memory/in_memory_storage.cc
)
target_include_directories(backend_storage_memory PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(backend_storage_memory
  PUBLIC
    backend_schema
    backend_storage
    absl::base
    absl::status
    absl::statusor
    absl::strings
    absl::synchronization
    absl::span
)

# ---------------------------------------------------------------------------
# Engine implementations (Phase 3c scaffolds, real wiring in Phase 5)
#
# Each concrete `Engine` lives in its own static library so the CLI can
# pick one at link time via `--engine`. Today both impls return
# UNIMPLEMENTED on every Engine method; the libraries exist so the
# binary, the factory, and (later) the conformance harness can already
# bind against the real types.
# ---------------------------------------------------------------------------

add_library(backend_engine_reference_impl STATIC
  backend/engine/reference_impl/reference_impl_engine.cc
)
target_include_directories(backend_engine_reference_impl
  PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(backend_engine_reference_impl
  PUBLIC
    backend_engine
    backend_storage
    absl::status
    absl::statusor
)

add_library(backend_engine_duckdb STATIC
  backend/engine/duckdb/duckdb_engine.cc
)
target_include_directories(backend_engine_duckdb
  PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(backend_engine_duckdb
  PUBLIC
    backend_engine
    backend_storage
    absl::status
    absl::statusor
)

# ---------------------------------------------------------------------------
# Engine front-door + binary
# ---------------------------------------------------------------------------

add_library(frontend_server STATIC
  frontend/handlers/catalog.cc
  frontend/handlers/query.cc
  frontend/server/server.cc
)
target_include_directories(frontend_server PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(frontend_server PUBLIC enginepb_cc backend_engine)

# emulator_main links every backend library it can construct at CLI
# parse time: the schema/storage interface stack, the in-memory storage
# impl (only `--storage` value wired today), and both engine scaffolds.
# Phase 5+ adds the DuckDB storage library here once it exists.
add_executable(emulator_main
  binaries/emulator_main/main.cc
)
target_link_libraries(emulator_main
  PRIVATE
    frontend_server
    backend_engine_reference_impl
    backend_engine_duckdb
    backend_storage_memory
    backend_storage
    backend_schema
)

# ---------------------------------------------------------------------------
# Unit tests
#
# `task ci:run` invokes `cmake --build build-out` (no target), which
# walks every target in this file — so the test executable is built
# alongside emulator_main and `ctest --test-dir build-out` picks it up
# without an explicit build step.
#
# GoogleTest is pulled in via FetchContent because no Linux distro
# packages it in a stable spot; the upstream tarball is small (~1MB)
# and the build is cached under build-out/_deps after the first cmake
# configure. Set BIGQUERY_EMULATOR_BUILD_TESTS=OFF at configure time to
# skip if you're cross-compiling or working offline.
# ---------------------------------------------------------------------------

option(BIGQUERY_EMULATOR_BUILD_TESTS
  "Build the C++ unit tests (requires GoogleTest, fetched via FetchContent)"
  ON)

if(BIGQUERY_EMULATOR_BUILD_TESTS)
  include(FetchContent)
  # Pin to a stable release so the build is reproducible. v1.15.2 is the
  # current LTS at the time of this commit; bump deliberately.
  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.tar.gz
    URL_HASH SHA256=7b42b4d6ed48810c5362c265a17faebe90dc2373c885e5216439d37927f02926
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
  )
  # GoogleTest's CMake defaults to building shared CRTs on MSVC; we
  # don't care on Linux but the option name is the same and forcing it
  # OFF keeps our static-link story consistent on every host.
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)

  enable_testing()

  add_executable(in_memory_storage_test
    backend/storage/memory/in_memory_storage_test.cc
  )
  target_link_libraries(in_memory_storage_test
    PRIVATE
      backend_storage_memory
      backend_storage
      backend_schema
      absl::status
      absl::statusor
      absl::strings
      GTest::gtest
      GTest::gmock
      GTest::gtest_main
  )
  # Register the whole binary as one ctest entry so `ctest -R
  # in_memory_storage` matches; the GoogleTest binary still reports
  # per-TEST results inside itself, which is what we want for CI logs.
  add_test(
    NAME in_memory_storage_test
    COMMAND in_memory_storage_test
  )
endif()
