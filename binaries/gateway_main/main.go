// gateway_main is the BigQuery emulator's REST gateway entry point.
//
// It is structurally analogous to cloud-spanner-emulator's gateway_main:
// the C++ engine (emulator_main) implements SQL semantics on top of
// GoogleSQL, and this Go binary fronts it with a BigQuery-shaped REST API.
// On startup the gateway spawns the engine as a subprocess and shuts it
// down on exit.
//
// # CLI surface
//
// The parser is broken out into cli.go and supports both the legacy
// underscore-separated flag names this repository started with
// (`--http_port`) and the hyphen-separated equivalents
// (`--http-port`) documented for gateway_main. Every
// new operator-facing flag (data dir, engine pass-through, seed API,
// seed YAML files) is registered there; this file only wires the
// parsed Config into the gateway runtime.
package main

import (
	"errors"
	"flag"
	"fmt"
	"io"
	"log" //nolint:depguard // process-launch + version-print error paths use stdlib log; gateway runtime emits structured slog via opts.Logger
	"log/slog"
	"os"
	"path"
	"path/filepath"
	"runtime"

	"github.com/vantaboard/bigquery-emulator/gateway"
	"github.com/vantaboard/bigquery-emulator/gateway/engine"
	"github.com/vantaboard/bigquery-emulator/gateway/seed"
	"github.com/vantaboard/bigquery-emulator/gateway/seedfile"
	"github.com/vantaboard/bigquery-emulator/gateway/storagetmpl"
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

// printVersion writes the multi-line version block to w. Pulled out
// into its own function so unit tests can drive it with a
// `bytes.Buffer` rather than fork a process. The format intentionally
// mirrors cloud-spanner-emulator's `gateway_main --version` shape (one
// title line, then indented `key: value` rows) so operators who know
// one emulator can read the other.
func printVersion(w io.Writer) {
	// Writes into io.Writer can in principle fail (e.g. broken
	// pipe when `gateway_main --version | head -1` closes early),
	// but there's no meaningful recovery here -- the process is
	// about to exit. Discard the errcheck warnings rather than
	// pad each Fprintf with a no-op handler.
	_, _ = fmt.Fprintf(w, "bigquery-emulator-gateway version %s\n", version)
	_, _ = fmt.Fprintf(w, "  commit:  %s\n", commit)
	_, _ = fmt.Fprintf(w, "  built:   %s\n", date)
	_, _ = fmt.Fprintf(w, "  go:      %s\n", runtime.Version())
	_, _ = fmt.Fprintf(w, "  os/arch: %s/%s\n", runtime.GOOS, runtime.GOARCH)
}

// resolveEngineBinary mirrors the resolution logic from
// cloud-spanner-emulator: accept an absolute path as-is, otherwise look in
// the gateway binary's directory and its parent. Returns "" if disabled.
func resolveEngineBinary(name string) string {
	if name == "" {
		return ""
	}
	if path.IsAbs(name) {
		return name
	}

	gwPath, err := os.Executable()
	if err != nil {
		log.Fatalf("could not resolve own executable path: %v", err)
	}
	gwDir := filepath.Dir(gwPath)

	candidate := filepath.Join(gwDir, name)
	if _, err := os.Stat(candidate); err == nil {
		return candidate
	}
	candidate = filepath.Join(filepath.Dir(gwDir), name)
	if _, err := os.Stat(candidate); err == nil {
		return candidate
	}
	log.Fatalf("could not locate engine binary %q in %q or its parent",
		name, gwDir)
	return ""
}

func main() {
	cfg, err := parseArgs(os.Args[1:], os.Stderr, os.LookupEnv)
	if err != nil {
		if errors.Is(err, flag.ErrHelp) {
			os.Exit(0)
		}
		log.Fatal(err)
	}
	if cfg.VersionRequested {
		printVersion(os.Stdout)
		return
	}
	if err := runGateway(cfg); err != nil {
		log.Fatal(err)
	}
}

func runGateway(cfg Config) error {
	log.SetFlags(log.Ldate | log.Ltime | log.Lshortfile)

	httpAddr, storageGRPCAddr, engineAddr, engineArgs := cfg.ToOptions(cfg.EngineBinary)

	logLevel := slog.LevelInfo
	if cfg.Debug {
		logLevel = slog.LevelDebug
	}
	gatewayLogger := slog.New(slog.NewTextHandler(os.Stderr, &slog.HandlerOptions{
		Level: logLevel,
	}))

	opts := gateway.Options{
		HTTPAddress:            httpAddr,
		StorageGRPCAddress:     storageGRPCAddr,
		EngineAddress:          engineAddr,
		EngineBinary:           resolveEngineBinary(cfg.EngineBinary),
		EngineArgs:             engineArgs,
		CopyEngineStdout:       cfg.CopyEngineStdout,
		CopyEngineStderr:       cfg.CopyEngineStderr,
		LogRequests:            cfg.LogRequests,
		DefaultProjectID:       cfg.DefaultProjectID,
		DefaultDatasetID:       cfg.DefaultDatasetID,
		DefaultDatasetLocation: cfg.DefaultDatasetLocation,
		EnableSeedAPI:          cfg.EnableSeedAPI,
		SeedAPIAllowRemote:     cfg.SeedAPIAllowRemote,
		SeedAPISeedToken:       cfg.SeedAPISeedToken,
		SeedFiles:              cfg.SeedFiles,
		DataDir:                cfg.DataDir,
		InitialDataDir:         cfg.InitialDataDir,
		Debug:                  cfg.Debug,
		Logger:                 gatewayLogger,
	}

	gw := gateway.New(opts).
		WithPreStartHook(func(o gateway.Options) error {
			return storagetmpl.MaybeMaterialize(o.InitialDataDir, o.DataDir)
		}).
		WithPostEngineHook(func(o gateway.Options, ec *engine.Client) error {
			if len(o.SeedFiles) == 0 || ec == nil {
				return nil
			}
			return seedfile.ApplyFiles(o.SeedFiles,
				seed.NewCatalogApplier(ec.Catalog),
				gateway.DefaultsFromOptions(o))
		})
	return gw.Run()
}
