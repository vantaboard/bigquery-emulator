#include "frontend/server/server.h"

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>

#include <chrono>
#include <csignal>
#include <cstdio>
#include <memory>
#include <string>
#include <thread>
#include <utility>

#include "frontend/handlers/catalog.h"
#include "frontend/handlers/query.h"
#include "frontend/handlers/storage_read.h"
#include "frontend/handlers/storage_write.h"
#include "frontend/handlers/sqltools.h"

namespace bigquery_emulator {
namespace frontend {

namespace {

// Async-signal-safe: only written from HandleSignal, read from the
// shutdown watcher thread in WaitForShutdown.
volatile sig_atomic_t g_shutdown_requested = 0;

void HandleSignal(int /*signo*/) {
  g_shutdown_requested = 1;
}

// GrpcServer hosts the real `grpc::Server` for the emulator engine.
//
// It owns:
//   * a `CatalogService`, `QueryService`, `SqlToolsService`,
//     `StorageReadService`, and `StorageWriteService` implementation
//     of the `bigquery_emulator.v1.*` gRPC services defined in
//     `proto/emulator.proto`, `proto/storage_read.proto`, and
//     `proto/storage_write.proto`, and
//   * the default `grpc.health.v1.Health` service that ServerBuilder
//     registers when EnableDefaultHealthCheckService(true) is called.
//
// Catalog + Query are wired end-to-end against the storage backend
// and the DuckDB engine; StorageRead lights up `CreateReadSession` +
// `ReadRows`; StorageWrite lights up `CreateWriteStream` /
// `AppendRows` / `GetWriteStream` for the `_default`, `COMMITTED`,
// and `BUFFERED` stream types (`FlushRows` / `FinalizeWriteStream`
// drive BUFFERED visibility), while PENDING +
// `BatchCommitWriteStreams` return UNIMPLEMENTED per
// `docs/ENGINE_POLICY.md`.
class GrpcServer final : public Server {
 public:
  GrpcServer(std::unique_ptr<::grpc::Server> server,
             std::unique_ptr<CatalogService> catalog,
             std::unique_ptr<QueryService> query,
             std::unique_ptr<SqlToolsService> sqltools,
             std::unique_ptr<StorageReadService> storage_read,
             std::unique_ptr<StorageWriteService> storage_write,
             std::string host,
             int port)
      : server_(std::move(server)),
        catalog_(std::move(catalog)),
        query_(std::move(query)),
        sqltools_(std::move(sqltools)),
        storage_read_(std::move(storage_read)),
        storage_write_(std::move(storage_write)),
        host_(std::move(host)),
        port_(port) {}

  void WaitForShutdown() override {
    // gRPC's `Server::Wait()` holds an internal absl::Mutex for the
    // server's lifetime. Calling `Shutdown()` from a POSIX signal
    // handler (which conformance / the gateway trigger via SIGINT)
    // re-enters that mutex on the same thread and Abseil fatals with
    // "illegal recursion into Mutex code". Only set a flag here;
    // a helper thread calls `Stop()` so `Wait()` can unwind cleanly.
    std::thread shutdown_watcher([this]() {
      while (g_shutdown_requested == 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
      }
      Stop();
    });
    server_->Wait();
    shutdown_watcher.join();
  }

  std::string host() const override {
    return host_;
  }
  int port() const override {
    return port_;
  }

  void Stop() {
    if (server_ != nullptr) {
      server_->Shutdown();
    }
  }

 private:
  std::unique_ptr<::grpc::Server> server_{};
  std::unique_ptr<CatalogService> catalog_{};
  std::unique_ptr<QueryService> query_{};
  std::unique_ptr<SqlToolsService> sqltools_{};
  std::unique_ptr<StorageReadService> storage_read_{};
  std::unique_ptr<StorageWriteService> storage_write_{};
  std::string host_{};
  int port_ = 0;
};

// Splits "host:port" into (host, port). Returns an empty host string on
// malformed input; the caller treats that as "bind to all interfaces".
std::pair<std::string, int> SplitHostPort(const std::string& address) {
  const auto colon = address.rfind(':');
  if (colon == std::string::npos) {
    return {address, 0};
  }
  const std::string host = address.substr(0, colon);
  int port = 0;
  try {
    port = std::stoi(address.substr(colon + 1));
  } catch (...) {
    port = 0;
  }
  return {host, port};
}

}  // namespace

std::unique_ptr<Server> Server::Create(const Options& options) {
  if (options.storage == nullptr) {
    // cpp-lint:allow(banned-logging) -- pre-gRPC bootstrap diagnostic; Create
    // has no Status return today
    std::fprintf(stderr,
                 "[frontend::Server] Options.storage must be non-null; "
                 "Catalog RPCs have no backend to delegate to\n");
    return nullptr;
  }

  // EnableDefaultHealthCheckService must be toggled before constructing
  // a ServerBuilder; the builder snapshots the global flag at
  // construction time. The reflection plugin must be installed in the
  // same pre-builder window so `grpcurl -plaintext :PORT list` works
  // against the running engine (used by the storage-read smoke
  // verification scripts and any ad-hoc gRPC debugging session).
  ::grpc::EnableDefaultHealthCheckService(true);
  ::grpc::reflection::InitProtoReflectionServerBuilderPlugin();

  auto catalog = std::make_unique<CatalogService>(options.storage);
  auto query = std::make_unique<QueryService>(options.storage, options.engine);
  auto sqltools = std::make_unique<SqlToolsService>(options.storage);
  auto storage_read = std::make_unique<StorageReadService>(options.storage);
  auto storage_write = std::make_unique<StorageWriteService>(options.storage);

  ::grpc::ServerBuilder builder;
  int bound_port = 0;
  builder.AddListeningPort(
      options.server_address, ::grpc::InsecureServerCredentials(), &bound_port);
  builder.RegisterService(catalog.get());
  builder.RegisterService(query.get());
  builder.RegisterService(sqltools.get());
  builder.RegisterService(storage_read.get());
  builder.RegisterService(storage_write.get());

  std::unique_ptr<::grpc::Server> grpc_server = builder.BuildAndStart();
  if (grpc_server == nullptr || bound_port == 0) {
    // cpp-lint:allow(banned-logging) -- pre-gRPC bootstrap diagnostic; Create
    // has no Status return today
    std::fprintf(stderr,
                 "[frontend::Server] failed to bind gRPC server on %s\n",
                 options.server_address.c_str());
    return nullptr;
  }

  // Default health service starts unset; explicitly mark "" (the empty
  // service name `grpc.health.v1.Health.Check` uses for "the server as a
  // whole") plus our two real services as SERVING. `grpc_health_probe`
  // hits the empty-name entry by default; the per-service entries let
  // the gateway's per-service readiness checks succeed once they land
  // in plan grpc-gateway-client.
  if (auto* health_service = grpc_server->GetHealthCheckService()) {
    health_service->SetServingStatus("", true);
    health_service->SetServingStatus("bigquery_emulator.v1.Catalog", true);
    health_service->SetServingStatus("bigquery_emulator.v1.Query", true);
    health_service->SetServingStatus("bigquery_emulator.v1.SqlTools", true);
    health_service->SetServingStatus("bigquery_emulator.v1.StorageRead", true);
    health_service->SetServingStatus("bigquery_emulator.v1.StorageWrite", true);
  }

  auto [host, port] = SplitHostPort(options.server_address);
  if (port == 0) {
    port = bound_port;
  }

  auto server = std::make_unique<GrpcServer>(std::move(grpc_server),
                                             std::move(catalog),
                                             std::move(query),
                                             std::move(sqltools),
                                             std::move(storage_read),
                                             std::move(storage_write),
                                             std::move(host),
                                             port);

  g_shutdown_requested = 0;
  std::signal(SIGINT, HandleSignal);
  std::signal(SIGTERM, HandleSignal);
  return server;
}

}  // namespace frontend
}  // namespace bigquery_emulator
