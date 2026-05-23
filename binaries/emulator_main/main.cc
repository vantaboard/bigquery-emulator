// emulator_main is the C++ entry point for the BigQuery emulator's engine.
//
// It is structurally analogous to cloud-spanner-emulator's emulator_main:
// it owns a gRPC server that fronts GoogleSQL (analyzer + reference impl)
// for the Go gateway to call into. The Go gateway spawns this binary on
// startup; the binary blocks until the gateway terminates it.
//
// Phase 3c status: this binary parses the CLI surface (`--host_port`,
// `--engine`, `--storage`, `--profile`, `--help`), instantiates the
// chosen storage backend and engine through a factory, and hands them
// to the gRPC front door. Both engine implementations are currently
// scaffolds that return `UNIMPLEMENTED`; Phase 5 wires the real
// `googlesql::reference_impl` and DuckDB transpiler paths.

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <string>
#include <string_view>

#include "absl/status/statusor.h"
#include "backend/engine/duckdb/duckdb_engine.h"
#include "backend/engine/engine.h"
#include "backend/engine/reference_impl/reference_impl_engine.h"
#include "backend/storage/memory/in_memory_storage.h"
#include "backend/storage/storage.h"
#include "frontend/server/server.h"

namespace {

// Selectable engine implementations. Reference impl is the semantic
// source of truth (slow but correct); DuckDB is the fast transpiled
// path. Both scaffolds currently return UNIMPLEMENTED.
enum class EngineKind {
  kReferenceImpl,
  kDuckDB,
};

// Selectable storage backends. Memory is volatile and CI-friendly;
// DuckDB-backed persistent storage lands in a later plan and is
// rejected by the factory for now.
enum class StorageKind {
  kMemory,
  kDuckDB,
};

// Pre-baked combinations exposed via `--profile`. Mirrors the two
// callouts in ROADMAP's "Pluggable engine and storage" section: `ci`
// is `(reference_impl, memory)` (the default; maximizes conformance),
// `dev` is `(duckdb, duckdb)` (the published "production emulator"
// shape, persistent + fast).
enum class ProfileKind {
  kUnset,
  kCi,
  kDev,
};

struct Flags {
  std::string host_port = "localhost:9060";
  EngineKind engine = EngineKind::kReferenceImpl;
  StorageKind storage = StorageKind::kMemory;
  ProfileKind profile = ProfileKind::kUnset;
  // Set by --engine / --storage on the command line so the profile
  // shorthand never overrides an explicit flag.
  bool engine_explicit = false;
  bool storage_explicit = false;
  bool help = false;
};

void PrintUsage(std::FILE* out, const char* argv0) {
  std::fprintf(out,
               "Usage: %s [flags]\n"
               "\n"
               "The C++ engine half of the BigQuery emulator. The Go\n"
               "gateway spawns this binary on startup and talks to it\n"
               "over gRPC on --host_port.\n"
               "\n"
               "Flags:\n"
               "  --host_port=HOST:PORT   gRPC listen address\n"
               "                          (default: localhost:9060)\n"
               "\n"
               "  --engine=KIND           query engine implementation:\n"
               "                            reference_impl  GoogleSQL\n"
               "                                            reference impl\n"
               "                                            (slow, correct;\n"
               "                                            ROADMAP 5.A)\n"
               "                            duckdb          ZetaSQL ->\n"
               "                                            DuckDB SQL\n"
               "                                            transpiler\n"
               "                                            (fast OLAP;\n"
               "                                            ROADMAP 5.B)\n"
               "                          default: reference_impl\n"
               "\n"
               "  --storage=KIND          row storage backend:\n"
               "                            memory          volatile,\n"
               "                                            CI-friendly\n"
               "                            duckdb          Parquet/Arrow\n"
               "                                            on disk via\n"
               "                                            DuckDB (not\n"
               "                                            yet wired)\n"
               "                          default: memory\n"
               "\n"
               "  --profile=KIND          shorthand for the two common\n"
               "                          engine+storage combinations:\n"
               "                            ci              reference_impl\n"
               "                                            + memory\n"
               "                                            (default)\n"
               "                            dev             duckdb + duckdb\n"
               "                          Explicit --engine / --storage\n"
               "                          flags win over --profile.\n"
               "\n"
               "  --help, -h              print this message and exit\n",
               argv0);
}

// Parses `--key=value` and `--key value` shapes for one expected flag.
// Returns true if `argv[*i]` matched `key`, writes the value into
// `*value`, and advances `*i` past the value when it lives in
// `argv[*i+1]`. Returns false on mismatch.
bool MatchStringFlag(int argc, char** argv, int* i, const char* key,
                      std::string* value) {
  const std::string_view arg = argv[*i];
  const std::string with_eq = std::string("--") + key + "=";
  if (arg.substr(0, with_eq.size()) == with_eq) {
    *value = std::string(arg.substr(with_eq.size()));
    return true;
  }
  const std::string bare = std::string("--") + key;
  if (arg == bare) {
    if (*i + 1 >= argc) {
      std::fprintf(stderr, "[emulator_main] --%s requires a value\n", key);
      return false;
    }
    *value = argv[++(*i)];
    return true;
  }
  return false;
}

absl::StatusOr<EngineKind> ParseEngineKind(const std::string& value) {
  if (value == "reference_impl") return EngineKind::kReferenceImpl;
  if (value == "duckdb") return EngineKind::kDuckDB;
  return absl::InvalidArgumentError(
      "unknown --engine value (expected reference_impl|duckdb): " + value);
}

absl::StatusOr<StorageKind> ParseStorageKind(const std::string& value) {
  if (value == "memory") return StorageKind::kMemory;
  if (value == "duckdb") return StorageKind::kDuckDB;
  return absl::InvalidArgumentError(
      "unknown --storage value (expected memory|duckdb): " + value);
}

absl::StatusOr<ProfileKind> ParseProfileKind(const std::string& value) {
  if (value == "ci") return ProfileKind::kCi;
  if (value == "dev") return ProfileKind::kDev;
  return absl::InvalidArgumentError(
      "unknown --profile value (expected ci|dev): " + value);
}

const char* EngineName(EngineKind kind) {
  switch (kind) {
    case EngineKind::kReferenceImpl:
      return "reference_impl";
    case EngineKind::kDuckDB:
      return "duckdb";
  }
  return "?";
}

const char* StorageName(StorageKind kind) {
  switch (kind) {
    case StorageKind::kMemory:
      return "memory";
    case StorageKind::kDuckDB:
      return "duckdb";
  }
  return "?";
}

// Applies the --profile shorthand to `flags`, but never overrides
// engine/storage choices that were set explicitly on the command line.
// Returning OK is unconditional today; this signature leaves room for
// per-profile validation later.
absl::Status ApplyProfile(Flags* flags) {
  switch (flags->profile) {
    case ProfileKind::kUnset:
      return absl::OkStatus();
    case ProfileKind::kCi:
      if (!flags->engine_explicit) flags->engine = EngineKind::kReferenceImpl;
      if (!flags->storage_explicit) flags->storage = StorageKind::kMemory;
      return absl::OkStatus();
    case ProfileKind::kDev:
      if (!flags->engine_explicit) flags->engine = EngineKind::kDuckDB;
      if (!flags->storage_explicit) flags->storage = StorageKind::kDuckDB;
      return absl::OkStatus();
  }
  return absl::InternalError("ApplyProfile: unreachable profile kind");
}

absl::StatusOr<Flags> ParseFlags(int argc, char** argv) {
  Flags flags;
  for (int i = 1; i < argc; ++i) {
    const std::string_view arg = argv[i];
    if (arg == "--help" || arg == "-h") {
      flags.help = true;
      continue;
    }
    std::string value;
    if (MatchStringFlag(argc, argv, &i, "host_port", &value)) {
      flags.host_port = value;
      continue;
    }
    if (MatchStringFlag(argc, argv, &i, "engine", &value)) {
      auto kind = ParseEngineKind(value);
      if (!kind.ok()) return kind.status();
      flags.engine = *kind;
      flags.engine_explicit = true;
      continue;
    }
    if (MatchStringFlag(argc, argv, &i, "storage", &value)) {
      auto kind = ParseStorageKind(value);
      if (!kind.ok()) return kind.status();
      flags.storage = *kind;
      flags.storage_explicit = true;
      continue;
    }
    if (MatchStringFlag(argc, argv, &i, "profile", &value)) {
      auto kind = ParseProfileKind(value);
      if (!kind.ok()) return kind.status();
      flags.profile = *kind;
      continue;
    }
    return absl::InvalidArgumentError(
        std::string("unknown flag: ") + std::string(arg));
  }
  auto status = ApplyProfile(&flags);
  if (!status.ok()) return status;
  return flags;
}

// Storage factory. DuckDB storage is reserved for the next plan
// (`duckdb-storage-core_o0d1e2f3`); until then the binary refuses to
// start with --storage=duckdb so users get a clear error instead of a
// silent fallback.
absl::StatusOr<std::unique_ptr<
    bigquery_emulator::backend::storage::Storage>>
CreateStorage(StorageKind kind) {
  switch (kind) {
    case StorageKind::kMemory:
      return std::unique_ptr<bigquery_emulator::backend::storage::Storage>(
          new bigquery_emulator::backend::storage::memory::InMemoryStorage());
    case StorageKind::kDuckDB:
      return absl::UnimplementedError(
          "--storage=duckdb is not implemented yet "
          "(ROADMAP Phase 3: duckdb-storage-core)");
  }
  return absl::InternalError("CreateStorage: unreachable storage kind");
}

// Engine factory. Both implementations are Phase 3c scaffolds and
// return UNIMPLEMENTED on every Engine method; the real wiring lands
// in ROADMAP Phase 5.A / 5.B.
std::unique_ptr<bigquery_emulator::backend::engine::Engine> CreateEngine(
    EngineKind kind, bigquery_emulator::backend::storage::Storage* storage) {
  switch (kind) {
    case EngineKind::kReferenceImpl:
      return std::unique_ptr<bigquery_emulator::backend::engine::Engine>(
          new bigquery_emulator::backend::engine::reference_impl::
              ReferenceImplEngine(storage));
    case EngineKind::kDuckDB:
      return std::unique_ptr<bigquery_emulator::backend::engine::Engine>(
          new bigquery_emulator::backend::engine::duckdb::DuckDBEngine(
              storage));
  }
  return nullptr;
}

}  // namespace

int main(int argc, char** argv) {
  auto parsed = ParseFlags(argc, argv);
  if (!parsed.ok()) {
    std::fprintf(stderr, "[emulator_main] %s\n",
                 std::string(parsed.status().message()).c_str());
    PrintUsage(stderr, argv[0]);
    return EXIT_FAILURE;
  }
  Flags flags = std::move(parsed).value();

  if (flags.help) {
    PrintUsage(stdout, argv[0]);
    return EXIT_SUCCESS;
  }

  auto storage = CreateStorage(flags.storage);
  if (!storage.ok()) {
    std::fprintf(stderr, "[emulator_main] failed to create storage: %s\n",
                 std::string(storage.status().message()).c_str());
    return EXIT_FAILURE;
  }
  std::unique_ptr<bigquery_emulator::backend::storage::Storage> storage_owned =
      std::move(storage).value();

  std::unique_ptr<bigquery_emulator::backend::engine::Engine> engine =
      CreateEngine(flags.engine, storage_owned.get());
  if (engine == nullptr) {
    std::fprintf(stderr, "[emulator_main] failed to create engine\n");
    return EXIT_FAILURE;
  }

  std::fprintf(stderr,
               "[emulator_main] starting engine=%s storage=%s host_port=%s\n",
               EngineName(flags.engine), StorageName(flags.storage),
               flags.host_port.c_str());

  bigquery_emulator::frontend::Server::Options options;
  options.server_address = flags.host_port;

  auto server = bigquery_emulator::frontend::Server::Create(options);
  if (!server) {
    std::fprintf(stderr, "[emulator_main] failed to start engine\n");
    return EXIT_FAILURE;
  }

  std::fprintf(stderr,
               "[emulator_main] BigQuery emulator engine listening on %s:%d\n",
               server->host().c_str(), server->port());

  server->WaitForShutdown();
  return EXIT_SUCCESS;
}
