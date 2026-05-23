#include "frontend/server/server.h"

#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>

#include <csignal>
#include <cstdio>
#include <memory>
#include <string>
#include <utility>

#include "frontend/handlers/catalog.h"
#include "frontend/handlers/query.h"

namespace bigquery_emulator {
namespace frontend {

namespace {

// GrpcServer hosts the real `grpc::Server` for the emulator engine.
//
// It owns:
//   * a `CatalogService` and `QueryService` implementation of the
//     `bigquery_emulator.v1.*` gRPC services defined in
//     `proto/emulator.proto`, and
//   * the default `grpc.health.v1.Health` service that ServerBuilder
//     registers when EnableDefaultHealthCheckService(true) is called.
//
// Both Catalog and Query are Phase 2b stubs that return
// `UNIMPLEMENTED`; the wiring exists so the Go gateway and external
// probes (`grpc_health_probe`) can drive the engine over a real socket.
class GrpcServer final : public Server {
 public:
  GrpcServer(std::unique_ptr<::grpc::Server> server,
             std::unique_ptr<CatalogService> catalog,
             std::unique_ptr<QueryService> query,
             std::string host, int port)
      : server_(std::move(server)),
        catalog_(std::move(catalog)),
        query_(std::move(query)),
        host_(std::move(host)),
        port_(port) {}

  void WaitForShutdown() override { server_->Wait(); }

  std::string host() const override { return host_; }
  int port() const override { return port_; }

  void Stop() {
    if (server_ != nullptr) {
      server_->Shutdown();
    }
  }

 private:
  std::unique_ptr<::grpc::Server> server_;
  std::unique_ptr<CatalogService> catalog_;
  std::unique_ptr<QueryService> query_;
  std::string host_;
  int port_;
};

GrpcServer* g_server = nullptr;

void HandleSignal(int /*signo*/) {
  if (g_server != nullptr) g_server->Stop();
}

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
  // EnableDefaultHealthCheckService must be toggled before constructing
  // a ServerBuilder; the builder snapshots the global flag at
  // construction time.
  ::grpc::EnableDefaultHealthCheckService(true);

  auto catalog = std::make_unique<CatalogService>();
  auto query = std::make_unique<QueryService>();

  ::grpc::ServerBuilder builder;
  int bound_port = 0;
  builder.AddListeningPort(options.server_address,
                           ::grpc::InsecureServerCredentials(), &bound_port);
  builder.RegisterService(catalog.get());
  builder.RegisterService(query.get());

  std::unique_ptr<::grpc::Server> grpc_server = builder.BuildAndStart();
  if (grpc_server == nullptr || bound_port == 0) {
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
  }

  auto [host, port] = SplitHostPort(options.server_address);
  if (port == 0) {
    port = bound_port;
  }

  auto server = std::make_unique<GrpcServer>(
      std::move(grpc_server), std::move(catalog), std::move(query),
      std::move(host), port);

  g_server = server.get();
  std::signal(SIGINT, HandleSignal);
  std::signal(SIGTERM, HandleSignal);
  return server;
}

}  // namespace frontend
}  // namespace bigquery_emulator
