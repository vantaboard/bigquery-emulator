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

// gateway_main is the BigQuery emulator's REST gateway entry point.
//
// It is structurally analogous to cloud-spanner-emulator's gateway_main:
// the C++ engine (emulator_main) implements SQL semantics on top of
// GoogleSQL, and this Go binary fronts it with a BigQuery-shaped REST API.
// On startup the gateway spawns the engine as a subprocess and shuts it
// down on exit.
package main

import (
	"flag"
	"fmt"
	"log"
	"os"
	"path"
	"path/filepath"

	"github.com/brighten-tompkins/bigquery-emulator/gateway"
)

var (
	hostname = flag.String("hostname", "localhost",
		"Hostname for the emulator servers.")
	httpPort = flag.Int("http_port", 9050,
		"Port on which to run the BigQuery REST gateway.")
	grpcPort = flag.Int("grpc_port", 9060,
		"Port on which to run the internal engine gRPC server.")

	engineBinary = flag.String("engine_binary", "emulator_main",
		"Path to the C++ engine binary. Empty disables the subprocess "+
			"(useful while the engine is still being scaffolded).")

	copyEngineStdout = flag.Bool("copy_engine_stdout", false,
		"If true, the gateway copies the engine's stdout to its own.")
	copyEngineStderr = flag.Bool("copy_engine_stderr", true,
		"If true, the gateway copies the engine's stderr to its own.")
	logRequests = flag.Bool("log_requests", false,
		"If true, every REST request and response is logged.")
)

// resolveEngineBinary mirrors the resolution logic from
// cloud-spanner-emulator: accept an absolute path as-is, otherwise look in
// the gateway binary's directory and its parent. Returns "" if disabled.
func resolveEngineBinary() string {
	if *engineBinary == "" {
		return ""
	}
	if path.IsAbs(*engineBinary) {
		return *engineBinary
	}

	gwPath, err := os.Executable()
	if err != nil {
		log.Fatalf("could not resolve own executable path: %v", err)
	}
	gwDir := filepath.Dir(gwPath)

	candidate := filepath.Join(gwDir, *engineBinary)
	if _, err := os.Stat(candidate); err == nil {
		return candidate
	}
	candidate = filepath.Join(filepath.Dir(gwDir), *engineBinary)
	if _, err := os.Stat(candidate); err == nil {
		return candidate
	}
	log.Fatalf("could not locate engine binary %q in %q or its parent",
		*engineBinary, gwDir)
	return ""
}

func main() {
	flag.Parse()
	log.SetFlags(log.Ldate | log.Ltime | log.Lshortfile)

	gw := gateway.New(gateway.Options{
		HTTPAddress:      fmt.Sprintf("%s:%d", *hostname, *httpPort),
		EngineAddress:    fmt.Sprintf("%s:%d", *hostname, *grpcPort),
		EngineBinary:     resolveEngineBinary(),
		CopyEngineStdout: *copyEngineStdout,
		CopyEngineStderr: *copyEngineStderr,
		LogRequests:      *logRequests,
	})
	if err := gw.Run(); err != nil {
		log.Fatal(err)
	}
}
