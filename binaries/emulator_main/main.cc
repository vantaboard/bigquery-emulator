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

// emulator_main is the C++ entry point for the BigQuery emulator's engine.
//
// It is structurally analogous to cloud-spanner-emulator's emulator_main:
// it owns a gRPC server that fronts GoogleSQL (analyzer + reference impl)
// for the Go gateway to call into. The Go gateway spawns this binary on
// startup; the binary blocks until the gateway terminates it.
//
// Phase 0 status: this file currently parses --host_port and prints what
// it would do. Wiring up the actual gRPC server, GoogleSQL Analyzer, and
// the in-memory backend is Phase 2/3 of ROADMAP.md. See
// frontend/server/server.h for the planned shape.

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include "frontend/server/server.h"

namespace {

// Tiny custom flag parser to keep this file dependency-free for the
// scaffold commit. Phase 1 will replace this with absl::flags, matching
// the Spanner emulator's binary.
std::string ParseHostPort(int argc, char** argv) {
  for (int i = 1; i < argc - 1; ++i) {
    if (std::strcmp(argv[i], "--host_port") == 0) {
      return std::string(argv[i + 1]);
    }
  }
  return "localhost:9060";
}

}  // namespace

int main(int argc, char** argv) {
  const std::string host_port = ParseHostPort(argc, argv);

  bigquery_emulator::frontend::Server::Options options;
  options.server_address = host_port;

  auto server = bigquery_emulator::frontend::Server::Create(options);
  if (!server) {
    std::fprintf(stderr, "[emulator_main] failed to start engine\n");
    return EXIT_FAILURE;
  }

  std::fprintf(stderr, "[emulator_main] BigQuery emulator engine listening at %s\n",
               host_port.c_str());

  server->WaitForShutdown();
  return EXIT_SUCCESS;
}
