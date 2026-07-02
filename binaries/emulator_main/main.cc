// emulator_main is the C++ entry point for the BigQuery emulator's engine.
//
// It is structurally analogous to cloud-spanner-emulator's emulator_main:
// it owns a gRPC server that fronts GoogleSQL (analyzer + DuckDB engine)
// for the Go gateway to call into. The Go gateway spawns this binary on
// startup; the binary blocks until the gateway terminates it.
//
// Runtime shape: the `Engine` is a `LocalCoordinatorEngine` that owns a
// `RouteClassifier` plus one executor per route (DuckDB / semantic /
// control-op / unsupported); see
// `docs/ENGINE_POLICY.md` and
// `docs/ENGINE_POLICY.md`. The storage backend is the DuckDB
// Parquet/Arrow store under `--data_dir`. The `--host_port` flag
// selects the gRPC listen address (default `localhost:9060`);
// `--data_dir` selects the persistent catalog root (default
// `$HOME/.bigquery-emulator`, see `DefaultDataDir`).

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <string>
#include <string_view>

#include "absl/status/statusor.h"
#include "backend/engine/coordinator/local_coordinator_engine.h"
#include "backend/engine/coordinator/routine_rehydrate.h"
#include "backend/engine/coordinator/view_rehydrate.h"
#include "backend/engine/engine.h"
#include "backend/storage/duckdb/duckdb_storage.h"
#include "backend/storage/storage.h"
#include "binaries/emulator_main/version.h"
#include "frontend/server/server.h"

namespace {

// Default `--data_dir` when not explicitly provided. Mirrors what
// cloud-spanner-emulator does for its persistent-store directory: a
// stable, user-scoped path under $HOME. Resolved lazily because $HOME
// is process-environment state.
std::string DefaultDataDir() {
  const char* home = std::getenv("HOME");
  if (home == nullptr || *home == '\0') {
    // No HOME (e.g. running inside a stripped container). Fall back to
    // a working-directory-relative path so the binary still starts;
    // callers should set --data_dir explicitly when this matters.
    return "./.bigquery-emulator";
  }
  std::string out(home);
  if (out.empty() || out.back() != '/') out.push_back('/');
  out.append(".bigquery-emulator");
  return out;
}

struct Flags {
  std::string host_port = "localhost:9060";
  // Persistent-store root used by the DuckDB storage backend. Default
  // is `$HOME/.bigquery-emulator`; users override via `--data_dir=PATH`.
  std::string data_dir;
  bool data_dir_explicit = false;
  bool help = false;
  // `--version` short-circuits before any catalog / storage init in
  // `main`, so the flag works in stripped containers where the
  // default `$HOME/.bigquery-emulator` directory is unwritable or
  // the listening port is already taken. Mirrors the Go gateway's
  // `--version` posture (see `binaries/gateway_main/main.go`).
  bool version = false;
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
               "  --data_dir=PATH         persistent-store root used by\n"
               "                          the DuckDB storage backend.\n"
               "                          default: $HOME/.bigquery-emulator\n"
               "\n"
               "  --version               print engine version (semver +\n"
               "                          git commit + build date) and exit\n"
               "\n"
               "  --help, -h              print this message and exit\n",
               argv0);
}

// Writes the multi-line version block to `out`. Pulled out into a
// named helper so the layout stays in lock-step with the Go
// gateway's `printVersion` (see `binaries/gateway_main/main.go`):
// one title line plus indented `key: value` rows. The trailing
// `clang:` row reports the compiler that built this TU; that is
// stable per Bazel cc_toolchain (see `.cursor/rules/bazel-process-
// hygiene.mdc`'s pin to system clang-18) so it is genuinely
// informative to operators diagnosing build-environment drift.
void PrintVersion(std::FILE* out) {
  std::fprintf(out,
               "bigquery-emulator-engine version %s\n",
               bigquery_emulator::binaries::emulator_main::kVersion);
  std::fprintf(out,
               "  commit:  %s\n",
               bigquery_emulator::binaries::emulator_main::kCommit);
  std::fprintf(out,
               "  built:   %s\n",
               bigquery_emulator::binaries::emulator_main::kBuildDate);
#if defined(__clang_version__)
  std::fprintf(out, "  clang:   %s\n", __clang_version__);
#elif defined(__VERSION__)
  std::fprintf(out, "  cc:      %s\n", __VERSION__);
#endif
}

// Parses `--key=value` and `--key value` shapes for one expected flag.
// Returns true if `argv[*i]` matched `key`, writes the value into
// `*value`, and advances `*i` past the value when it lives in
// `argv[*i+1]`. Returns false on mismatch.
bool MatchStringFlag(
    int argc, char** argv, int* i, const char* key, std::string* value) {
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

absl::StatusOr<Flags> ParseFlags(int argc, char** argv) {
  Flags flags;
  for (int i = 1; i < argc; ++i) {
    const std::string_view arg = argv[i];
    if (arg == "--help" || arg == "-h") {
      flags.help = true;
      continue;
    }
    if (arg == "--version") {
      flags.version = true;
      continue;
    }
    std::string value;
    if (MatchStringFlag(argc, argv, &i, "host_port", &value)) {
      flags.host_port = value;
      continue;
    }
    if (MatchStringFlag(argc, argv, &i, "data_dir", &value)) {
      flags.data_dir = value;
      flags.data_dir_explicit = true;
      continue;
    }
    return absl::InvalidArgumentError(std::string("unknown flag: ") +
                                      std::string(arg));
  }
  // Fill in the default data_dir lazily so the user's explicit
  // --data_dir always wins. We defer the lookup to here (rather than
  // the Flags initializer) because $HOME is process state, not a
  // constant.
  if (!flags.data_dir_explicit) {
    flags.data_dir = DefaultDataDir();
  }
  return flags;
}

}  // namespace

int main(int argc, char** argv) {
  auto parsed = ParseFlags(argc, argv);
  if (!parsed.ok()) {
    std::fprintf(stderr,
                 "[emulator_main] %s\n",
                 std::string(parsed.status().message()).c_str());
    PrintUsage(stderr, argv[0]);
    return EXIT_FAILURE;
  }
  Flags flags = std::move(parsed).value();

  if (flags.help) {
    PrintUsage(stdout, argv[0]);
    return EXIT_SUCCESS;
  }

  // Short-circuit before storage / engine init: `--version` must
  // succeed in stripped containers where the default DuckDB data dir
  // is unwritable, and operators invoking it from `docker run --rm
  // ... --version` should never trip the gRPC server's port bind.
  if (flags.version) {
    PrintVersion(stdout);
    return EXIT_SUCCESS;
  }

  if (setenv("BIGQUERY_EMULATOR_DATA_DIR", flags.data_dir.c_str(), 0) != 0) {
    std::fprintf(stderr,
                 "[emulator_main] warning: failed to set "
                 "BIGQUERY_EMULATOR_DATA_DIR\n");
  }

  auto storage_or =
      bigquery_emulator::backend::storage::duckdb::DuckDBStorage::Open(
          flags.data_dir);
  if (!storage_or.ok()) {
    std::fprintf(stderr,
                 "[emulator_main] failed to create storage: %s\n",
                 std::string(storage_or.status().message()).c_str());
    return EXIT_FAILURE;
  }
  std::unique_ptr<bigquery_emulator::backend::storage::Storage> storage =
      std::move(storage_or).value();

  absl::Status rehydrate = bigquery_emulator::backend::engine::coordinator::
      RehydrateRoutinesFromStorage(storage.get());
  if (!rehydrate.ok()) {
    std::fprintf(stderr,
                 "[emulator_main] failed to rehydrate routines: %s\n",
                 std::string(rehydrate.message()).c_str());
    return EXIT_FAILURE;
  }

  rehydrate = bigquery_emulator::backend::engine::coordinator::
      RehydrateViewsFromStorage(storage.get());
  if (!rehydrate.ok()) {
    std::fprintf(stderr,
                 "[emulator_main] failed to rehydrate views: %s\n",
                 std::string(rehydrate.message()).c_str());
    return EXIT_FAILURE;
  }

  std::unique_ptr<bigquery_emulator::backend::engine::Engine> engine(
      new bigquery_emulator::backend::engine::coordinator::
          LocalCoordinatorEngine(storage.get()));

  std::fprintf(stderr,
               "[emulator_main] starting engine=local_coordinator "
               "storage=duckdb data_dir=%s host_port=%s\n",
               flags.data_dir.c_str(),
               flags.host_port.c_str());

  bigquery_emulator::frontend::Server::Options options;
  options.server_address = flags.host_port;
  options.storage = storage.get();
  options.engine = engine.get();

  auto server = bigquery_emulator::frontend::Server::Create(options);
  if (!server) {
    std::fprintf(stderr, "[emulator_main] failed to start engine\n");
    return EXIT_FAILURE;
  }

  std::fprintf(stderr,
               "[emulator_main] BigQuery emulator engine listening on %s:%d\n",
               server->host().c_str(),
               server->port());

  server->WaitForShutdown();
  return EXIT_SUCCESS;
}
