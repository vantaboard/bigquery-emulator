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
	"io"
	"log"
	"os"
	"path"
	"path/filepath"
	"runtime"

	"github.com/vantaboard/bigquery-emulator/gateway"
)

// Version metadata. The defaults (`dev` / `none` / `unknown`) are what a
// plain `go build` produces; release builds replace them via
// `-X main.version=... -X main.commit=... -X main.date=...` ldflags
// (see `.goreleaser.yml` and `taskfiles/emulator.yml`'s `gateway:build`
// helper). Keep these as `var` (not `const`) so the linker can overwrite
// them — `const string` cannot be ldflag-injected.
var (
	version = "dev"
	commit  = "none"
	date    = "unknown"
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

	versionFlag = flag.Bool("version", false,
		"Print version information (semver + git commit + build date + "+
			"Go toolchain) and exit.")
)

// printVersion writes the multi-line version block to w. Pulled out
// into its own function so unit tests can drive it with a
// `bytes.Buffer` rather than fork a process. The format intentionally
// mirrors cloud-spanner-emulator's `gateway_main --version` shape (one
// title line, then indented `key: value` rows) so operators who know
// one emulator can read the other.
func printVersion(w io.Writer) {
	fmt.Fprintf(w, "bigquery-emulator-gateway version %s\n", version)
	fmt.Fprintf(w, "  commit:  %s\n", commit)
	fmt.Fprintf(w, "  built:   %s\n", date)
	fmt.Fprintf(w, "  go:      %s\n", runtime.Version())
	fmt.Fprintf(w, "  os/arch: %s/%s\n", runtime.GOOS, runtime.GOARCH)
}

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

	// Short-circuit before any side effect (engine subprocess lookup,
	// socket bind, log setup). `--version` must work in stripped
	// environments where the engine binary is missing and the gateway
	// has no permission to bind a port.
	if *versionFlag {
		printVersion(os.Stdout)
		return
	}

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
