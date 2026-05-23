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

#ifndef BIGQUERY_EMULATOR_FRONTEND_SERVER_SERVER_H_
#define BIGQUERY_EMULATOR_FRONTEND_SERVER_SERVER_H_

#include <memory>
#include <string>

namespace bigquery_emulator {
namespace frontend {

// Server is the BigQuery emulator's C++ engine front door.
//
// It hosts the gRPC services defined in proto/emulator.proto:
//
//   - Catalog: dataset/table/schema management (called by the Go gateway
//     when it sees REST CRUD against datasets and tables).
//   - Query:   SQL execution + dry-run (called by the Go gateway for
//     `bigquery.jobs.query` and `bigquery.jobs.insert` with a query
//     configuration).
//
// Internally these handlers will:
//
//   * Run statements through `googlesql::Analyzer::AnalyzeStatement` to
//     resolve names and types.
//   * Hand resolved ASTs to `googlesql::reference_impl::Algebrizer` and
//     `Evaluator` to produce result rows.
//   * Resolve catalog references against the in-memory backend (see
//     backend/storage/storage.h).
//
// Today this header just declares the API surface; Create returns a no-op
// server that blocks on WaitForShutdown until interrupted.
class Server {
 public:
  struct Options {
    // host:port the gRPC server should listen on, e.g. "localhost:9060".
    std::string server_address;
  };

  virtual ~Server() = default;

  // Creates and starts a new Server on options.server_address. Returns
  // null on failure.
  static std::unique_ptr<Server> Create(const Options& options);

  // Blocks until the server is told to stop (SIGINT/SIGTERM today).
  virtual void WaitForShutdown() = 0;

  // Accessors so emulator_main can log what we ended up bound to.
  virtual std::string host() const = 0;
  virtual int port() const = 0;
};

}  // namespace frontend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_FRONTEND_SERVER_SERVER_H_
