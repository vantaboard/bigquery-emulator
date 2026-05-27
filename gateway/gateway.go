// Package gateway runs the BigQuery emulator's REST gateway and manages
// the lifecycle of the C++ engine subprocess.
//
// The flow mirrors cloud-spanner-emulator's gateway:
//
//  1. Optionally spawn the engine binary, wiring its stdout/stderr.
//  2. Wait for the engine's gRPC port to become reachable.
//  3. Start the HTTP server that serves the BigQuery REST API.
//  4. On SIGINT/SIGTERM, shut down both cleanly.
package gateway

import (
	"context"
	"errors"
	"fmt"
	"log"
	"net/http"
	"os"
	"os/exec"
	"os/signal"
	"syscall"
	"time"

	"github.com/vantaboard/bigquery-emulator/gateway/engine"
)

// engineReadyTimeout bounds how long Gateway.Run will wait for the engine
// subprocess's gRPC health service to report SERVING before giving up.
// 30s is generous: a debug build of the engine takes <1s to bind and
// flip to SERVING on a developer laptop, but CI cold-starts and
// container builds sometimes spend 5-10s in linker/loader before main()
// runs.
const engineReadyTimeout = 30 * time.Second

// Options configures the gateway.
type Options struct {
	// HTTPAddress is the host:port the REST gateway listens on, e.g.
	// "localhost:9050".
	HTTPAddress string

	// EngineAddress is the host:port of the internal C++ engine gRPC
	// server, e.g. "localhost:9060". The Go gateway forwards SQL work
	// to this address.
	EngineAddress string

	// EngineBinary is the path to the C++ engine binary. If empty, the
	// gateway runs without an engine (useful early on while the engine
	// is still being scaffolded; queries will return Unimplemented).
	EngineBinary string

	// EngineArgs is the additional flag list passed to the engine
	// subprocess after `--host_port`. Use this to forward
	// `--engine`, `--storage`, `--profile`, `--data_dir`, and
	// `--on_unknown_fn` from gateway-level CLI flags through to
	// `emulator_main` without the gateway needing to know each
	// flag's semantics.
	EngineArgs []string

	// CopyEngineStdout / CopyEngineStderr forward the engine subprocess's
	// streams to the gateway's own streams.
	CopyEngineStdout bool
	CopyEngineStderr bool

	// LogRequests prints each REST request and response.
	LogRequests bool

	// DefaultProjectID is the project clients are assumed to be acting
	// against when seeding or other gateway-level operations need a
	// fallback project. Mirrors `--project-id` in go-googlesql.
	DefaultProjectID string

	// DefaultDatasetLocation is the BigQuery location used as the
	// fallback when a dataset is created without an explicit location
	// (US, EU, regional). Mirrors `--default-dataset-location`.
	DefaultDatasetLocation string

	// EnableSeedAPI registers `POST /api/emulator/seed` and the
	// matching `GET .../operations/{operationId}` endpoints so a
	// caller can copy live production BigQuery metadata + rows into
	// this emulator. Default false (off) for local safety.
	EnableSeedAPI bool

	// SeedAPIAllowRemote allows non-loopback callers to hit the seed
	// API when true. When false (the default), seed routes refuse
	// any request whose RemoteAddr is not loopback.
	SeedAPIAllowRemote bool

	// SeedAPISeedToken, when non-empty, requires matching header
	// `X-BigQuery-Emulator-Seed-Token` on every seed API request.
	// Loaded from `BIGQUERY_EMULATOR_SEED_TOKEN` when the flag is
	// empty (see binaries/gateway_main).
	SeedAPISeedToken string

	// SeedFiles is the optional list of YAML seed-data file paths
	// the gateway applies after the engine reports SERVING but
	// before it starts accepting public traffic. See
	// gateway/seedfile for the schema.
	SeedFiles []string

	// DataDir is the persistent storage root the engine uses when
	// `--storage=duckdb` is selected. Mirrors `--data-dir`; the
	// gateway passes it through via `--data_dir` in EngineArgs.
	DataDir string

	// InitialDataDir is an optional template directory the gateway
	// copies into DataDir on startup when DataDir does not yet
	// contain an initialized catalog (`catalog.duckdb` missing).
	// Mirrors `--initial-data-dir` in go-googlesql.
	InitialDataDir string

	// Debug enables verbose request and lifecycle logging.
	Debug bool
}

// Gateway is the top-level BigQuery emulator gateway.
type Gateway struct {
	opts       Options
	engine     *exec.Cmd
	engineDone chan struct{}

	// engineClient is the long-lived gRPC channel to the engine
	// subprocess. nil when EngineBinary is empty (Phase 1 stub mode).
	engineClient *engine.Client

	// preStartHook runs once just before the engine subprocess is
	// spawned. Use it for filesystem prep that must complete before
	// the engine touches DataDir (e.g. materializing a template tree
	// into an empty data directory).
	preStartHook func(Options) error

	// postEngineHook runs once after the engine reports SERVING but
	// before the gateway begins serving HTTP traffic. Use it for
	// startup-time seeding from YAML files that needs the
	// CatalogClient to be reachable.
	postEngineHook func(Options, *engine.Client) error
}

// New constructs a Gateway. Run actually starts it.
func New(opts Options) *Gateway {
	return &Gateway{opts: opts}
}

// WithPreStartHook installs a callback executed once before the engine
// subprocess is spawned. The hook runs synchronously on the Run
// goroutine and a non-nil error aborts startup without touching the
// engine.
func (g *Gateway) WithPreStartHook(hook func(Options) error) *Gateway {
	g.preStartHook = hook
	return g
}

// WithPostEngineHook installs a callback executed once after the
// engine reports SERVING but before the HTTP gateway accepts traffic.
// The hook receives the long-lived *engine.Client so it can use the
// CatalogClient / QueryClient to mutate state (e.g. apply YAML seed
// files). A non-nil error from the hook tears down the engine and
// aborts Run.
func (g *Gateway) WithPostEngineHook(hook func(Options, *engine.Client) error) *Gateway {
	g.postEngineHook = hook
	return g
}

// Run starts the engine subprocess (if configured) and the HTTP server,
// then blocks until either terminates or a signal arrives.
func (g *Gateway) Run() error {
	if g.preStartHook != nil {
		if err := g.preStartHook(g.opts); err != nil {
			return fmt.Errorf("pre-start hook: %w", err)
		}
	}

	if err := g.startEngine(); err != nil {
		return fmt.Errorf("start engine: %w", err)
	}

	if g.postEngineHook != nil {
		if err := g.postEngineHook(g.opts, g.engineClient); err != nil {
			g.stopEngine()
			return fmt.Errorf("post-engine hook: %w", err)
		}
	}

	srv := &http.Server{
		Addr:              g.opts.HTTPAddress,
		Handler:           NewServer(g.opts, g.engineClient),
		ReadHeaderTimeout: 10 * time.Second,
	}

	errCh := make(chan error, 1)
	go func() {
		log.Printf("BigQuery emulator REST gateway listening at http://%s", g.opts.HTTPAddress)
		if g.opts.EngineBinary != "" {
			log.Printf("Engine gRPC expected at %s", g.opts.EngineAddress)
		} else {
			log.Printf("Engine subprocess disabled (--engine_binary=\"\"); query routes will return Unimplemented")
		}
		err := srv.ListenAndServe()
		if err != nil && !errors.Is(err, http.ErrServerClosed) {
			errCh <- err
			return
		}
		errCh <- nil
	}()

	sigCh := make(chan os.Signal, 1)
	signal.Notify(sigCh, os.Interrupt, syscall.SIGTERM)

	select {
	case err := <-errCh:
		g.stopEngine()
		return err
	case sig := <-sigCh:
		log.Printf("Received signal %s, shutting down", sig)
		ctx, cancel := context.WithTimeout(context.Background(), 5*time.Second)
		defer cancel()
		_ = srv.Shutdown(ctx)
		g.stopEngine()
		return nil
	}
}

// startEngine spawns the C++ engine subprocess if one is configured and
// waits for it to come up. It is a no-op when EngineBinary is empty.
func (g *Gateway) startEngine() error {
	if g.opts.EngineBinary == "" {
		return nil
	}

	args := []string{
		"--host_port", g.opts.EngineAddress,
	}
	args = append(args, g.opts.EngineArgs...)
	cmd := exec.Command(g.opts.EngineBinary, args...)
	if g.opts.CopyEngineStdout {
		cmd.Stdout = os.Stdout
	}
	if g.opts.CopyEngineStderr {
		cmd.Stderr = os.Stderr
	}
	if err := cmd.Start(); err != nil {
		return fmt.Errorf("start %s: %w", g.opts.EngineBinary, err)
	}
	g.engine = cmd
	g.engineDone = make(chan struct{})

	go func() {
		err := cmd.Wait()
		close(g.engineDone)
		if err != nil {
			log.Printf("Engine subprocess exited: %v", err)
		}
	}()

	if err := g.connectAndWaitForEngine(); err != nil {
		return err
	}
	return nil
}

// connectAndWaitForEngine dials the engine's gRPC port and polls
// grpc.health.v1.Health.Check until it reports SERVING (or
// engineReadyTimeout fires). Replaces the Phase 1 sleep-and-pray stub
// with a real readiness probe so the gateway's HTTP listener never
// accepts traffic before the engine is actually able to answer it.
//
// Stores the live *engine.Client on the receiver for the lifetime of
// the gateway; the connection is reused for every business RPC and torn
// down by stopEngine.
func (g *Gateway) connectAndWaitForEngine() error {
	client, err := engine.Dial(g.opts.EngineAddress)
	if err != nil {
		return fmt.Errorf("dial engine at %s: %w", g.opts.EngineAddress, err)
	}

	ctx, cancel := context.WithTimeout(context.Background(), engineReadyTimeout)
	defer cancel()

	if err := client.WaitForReady(ctx); err != nil {
		_ = client.Close()
		return fmt.Errorf("wait for engine ready at %s: %w", g.opts.EngineAddress, err)
	}
	g.engineClient = client
	log.Printf("Engine gRPC at %s reported SERVING", g.opts.EngineAddress)
	return nil
}

func (g *Gateway) stopEngine() {
	if g.engineClient != nil {
		_ = g.engineClient.Close()
		g.engineClient = nil
	}
	if g.engine == nil || g.engine.Process == nil {
		return
	}
	_ = g.engine.Process.Signal(os.Interrupt)
	select {
	case <-g.engineDone:
	case <-time.After(5 * time.Second):
		_ = g.engine.Process.Kill()
		<-g.engineDone
	}
}
