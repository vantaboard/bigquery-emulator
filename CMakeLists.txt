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
# Engine front-door + binary
# ---------------------------------------------------------------------------

add_library(frontend_server STATIC
  frontend/server/server.cc
)
target_include_directories(frontend_server PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(frontend_server PUBLIC enginepb_cc)

add_executable(emulator_main
  binaries/emulator_main/main.cc
)
target_link_libraries(emulator_main PRIVATE frontend_server)
