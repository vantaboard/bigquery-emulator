# Vendor DuckDB via the upstream prebuilt libduckdb-linux-* tarballs.
#
# This file is included from the top-level CMakeLists.txt whenever
# `BIGQUERY_EMULATOR_ENABLE_DUCKDB` is ON (the default). It exposes a
# single imported target, `duckdb::duckdb`, that downstream backend
# libraries (`backend_engine_duckdb`, the upcoming
# `backend_storage_duckdb`) link against.
#
# Strategy: download the prebuilt `libduckdb.so` + `duckdb.h` /
# `duckdb.hpp` zip from the GitHub release, hash-pin it, and drop it
# in verbatim. This is dramatically faster than building DuckDB from
# source (~40 MB network fetch vs. several minutes of compilation per
# clean build) and matches the CI matrix exactly — upstream publishes
# linux/amd64 + linux/arm64 binaries, and `.github/workflows/ci.yml`
# runs the same two architectures. See `third_party/duckdb/VERSION`
# for the rationale and the bump checklist.

include(FetchContent)

# Pin lives in third_party/duckdb/VERSION; keep the two in sync.
set(BIGQUERY_EMULATOR_DUCKDB_VERSION "v1.5.3")

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
  message(FATAL_ERROR
    "DuckDB vendoring: only Linux is supported by the prebuilt-tarball "
    "strategy today (got CMAKE_SYSTEM_NAME='${CMAKE_SYSTEM_NAME}'). Disable "
    "with -DBIGQUERY_EMULATOR_ENABLE_DUCKDB=OFF or extend "
    "third_party/duckdb/CMakeLists.txt with a darwin/windows variant.")
endif()

if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|amd64|AMD64)$")
  set(_duckdb_arch "amd64")
  set(_duckdb_sha256
    "0a926eba5bce0abc0010f4b9109133e4440cb74e97bd10fd2d0fc2a721621b05")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(aarch64|arm64)$")
  set(_duckdb_arch "arm64")
  set(_duckdb_sha256
    "162806d591c0431d031d9bdf43dbecc5f00755da01a2064df68f9a69a6f50a10")
else()
  message(FATAL_ERROR
    "DuckDB vendoring: unsupported Linux architecture "
    "'${CMAKE_SYSTEM_PROCESSOR}'. Upstream publishes prebuilt tarballs "
    "only for amd64 and arm64.")
endif()

set(_duckdb_url
  "https://github.com/duckdb/duckdb/releases/download/${BIGQUERY_EMULATOR_DUCKDB_VERSION}/libduckdb-linux-${_duckdb_arch}.zip")

FetchContent_Declare(
  libduckdb_prebuilt
  URL ${_duckdb_url}
  URL_HASH SHA256=${_duckdb_sha256}
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

# The prebuilt zip ships a flat directory with no CMakeLists.txt, so
# FetchContent_MakeAvailable here just downloads + extracts and exposes
# `libduckdb_prebuilt_SOURCE_DIR`. We then point an imported target at
# the extracted .so + headers so consumers see a normal CMake target.
FetchContent_MakeAvailable(libduckdb_prebuilt)

set(_duckdb_dir "${libduckdb_prebuilt_SOURCE_DIR}")

if(NOT EXISTS "${_duckdb_dir}/libduckdb.so" OR
   NOT EXISTS "${_duckdb_dir}/duckdb.h")
  message(FATAL_ERROR
    "DuckDB vendoring: prebuilt tarball at ${_duckdb_url} did not extract "
    "the expected layout (missing libduckdb.so or duckdb.h under "
    "${_duckdb_dir}). Re-check the SHA256 pin against upstream.")
endif()

# IMPORTED + GLOBAL so we can hang an `ALIAS` off of it; the alias is
# the public name we ask consumers to depend on. CMake automatically
# adds the directory containing IMPORTED_LOCATION to the BUILD_RPATH
# of any executable that links transitively against this target, so
# `./build-out/emulator_main` can locate libduckdb.so without
# LD_LIBRARY_PATH games for local dev. Install/release builds add
# their own RPATH via CMAKE_INSTALL_RPATH.
add_library(libduckdb SHARED IMPORTED GLOBAL)
set_target_properties(libduckdb PROPERTIES
  IMPORTED_LOCATION "${_duckdb_dir}/libduckdb.so"
  INTERFACE_INCLUDE_DIRECTORIES "${_duckdb_dir}")
add_library(duckdb::duckdb ALIAS libduckdb)

# Expose the install path so emulator_main's BUILD_RPATH can point at
# it without re-deriving the FetchContent layout.
set(BIGQUERY_EMULATOR_DUCKDB_LIB_DIR "${_duckdb_dir}"
  CACHE INTERNAL "Directory containing the vendored libduckdb.so")
