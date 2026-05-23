// Copyright 2026 BigQuery Emulator Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "frontend/server/server.h"

#include <condition_variable>
#include <csignal>
#include <memory>
#include <mutex>
#include <string>

namespace bigquery_emulator {
namespace frontend {

namespace {

// Phase 0 placeholder server. It does not actually listen on a port; it
// just blocks WaitForShutdown until SIGINT/SIGTERM. Once we add the gRPC
// dependency (Phase 2 of ROADMAP.md), this becomes a real
// grpc::Server-backed implementation that registers handlers from
// frontend/handlers/.
class StubServer final : public Server {
 public:
  explicit StubServer(const Options& options)
      : address_(options.server_address) {}

  void WaitForShutdown() override {
    std::unique_lock<std::mutex> lock(mu_);
    cv_.wait(lock, [this] { return shutdown_; });
  }

  std::string host() const override {
    auto colon = address_.rfind(':');
    return colon == std::string::npos ? address_ : address_.substr(0, colon);
  }

  int port() const override {
    auto colon = address_.rfind(':');
    if (colon == std::string::npos) return 0;
    try {
      return std::stoi(address_.substr(colon + 1));
    } catch (...) {
      return 0;
    }
  }

  void Stop() {
    std::lock_guard<std::mutex> lock(mu_);
    shutdown_ = true;
    cv_.notify_all();
  }

 private:
  const std::string address_;
  std::mutex mu_;
  std::condition_variable cv_;
  bool shutdown_ = false;
};

StubServer* g_server = nullptr;

void HandleSignal(int /*signo*/) {
  if (g_server != nullptr) g_server->Stop();
}

}  // namespace

std::unique_ptr<Server> Server::Create(const Options& options) {
  auto srv = std::make_unique<StubServer>(options);
  g_server = srv.get();
  std::signal(SIGINT, HandleSignal);
  std::signal(SIGTERM, HandleSignal);
  return srv;
}

}  // namespace frontend
}  // namespace bigquery_emulator
