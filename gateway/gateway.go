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
	"log/slog"
	"net/http"
	"os"
	"os/exec"
	"os/signal"
	"syscall"
	"time"

	"github.com/vantaboard/bigquery-emulator/gateway/engine"
	"github.com/vantaboard/bigquery-emulator/gateway/grpcserver"
	"github.com/vantaboard/bigquery-emulator/gateway/handlers"
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
	// server, e.g. "localhost:9061". The Go gateway forwards SQL work
	// and the bqstorage shim's engine client to this address.
	EngineAddress string

	// StorageGRPCAddress is the host:port where the gateway registers
	// the public google.cloud.bigquery.storage.v1 BigQueryRead /
	// BigQueryWrite services, e.g. "localhost:9060". Client libraries
	// dial BIGQUERY_STORAGE_GRPC_ENDPOINT here.
	StorageGRPCAddress string

	// EngineBinary is the path to the C++ engine binary. If empty, the
	// gateway runs without an engine (useful early on while the engine
	// is still being scaffolded; queries will return Unimplemented).
	EngineBinary string

	// EngineArgs is the additional flag list passed to the engine
	// subprocess after `--host_port`. Use this to forward
	// `--data_dir` (and any future engine-level flags) from
	// gateway-level CLI flags through to `emulator_main` without
	// the gateway needing to know each flag's semantics.
	EngineArgs []string

	// CopyEngineStdout / CopyEngineStderr forward the engine subprocess's
	// streams to the gateway's own streams.
	CopyEngineStdout bool
	CopyEngineStderr bool

	// LogRequests prints each REST request and response.
	LogRequests bool

	// DefaultProjectID is the project clients are assumed to be acting
	// against when seeding or other gateway-level operations need a
	// fallback project. Mirrors `--project-id` on gateway_main.
	DefaultProjectID string

	// DefaultDatasetID is the server-level fallback dataset used to
	// resolve unqualified table names when a query/job does not carry
	// its own `defaultDataset`. Mirrors setting `default_dataset` on a
	// production BigQuery client/job. Empty means no fallback (bare
	// table names error, exactly like production with no default set).
	// Mirrors `--dataset` on gateway_main.
	DefaultDatasetID string

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

	// EnableSqlToolsAPI registers POST /api/emulator/sql/{format,parse,
	// tokenize,complete} for downstream UIs. Off by default.
	EnableSqlToolsAPI bool

	// SqlToolsAPIAllowRemote allows non-loopback callers when true.
	SqlToolsAPIAllowRemote bool

	// SqlToolsAPISeedToken requires matching header
	// X-BigQuery-Emulator-SqlTools-Token when non-empty.
	SqlToolsAPISeedToken string

	// DataDir is the persistent storage root the engine uses for
	// the DuckDB catalog + table data. Mirrors `--data-dir`; the
	// gateway passes it through via `--data_dir` in EngineArgs.
	DataDir string

	// InitialDataDir is an optional template directory the gateway
	// copies into DataDir on startup when DataDir does not yet
	// contain an initialized catalog (`catalog.duckdb` missing).
	// Mirrors `--initial-data-dir` on gateway_main.
	InitialDataDir string

	// Debug enables verbose request and lifecycle logging.
	Debug bool

	// Logger is the structured logger the gateway emits lifecycle and
	// request events to. When nil, the gateway logs to a discard
	// handler so callers that want silent embedding (unit tests, the
	// shallow-emulator harness) get zero output without having to
	// build their own no-op logger. Production binaries (see
	// binaries/gateway_main) wire a real *slog.Logger here so the
	// emulator's structured logs surface in stderr / stackdriver.
	Logger *slog.Logger
}

// Gateway is the top-level BigQuery emulator gateway.
type Gateway struct {
	opts       Options
	logger     *slog.Logger
	engine     *exec.Cmd
	engineDone chan struct{}

	// engineClient is the long-lived gRPC channel to the engine
	// subprocess. nil when EngineBinary is empty (gateway-only stub mode).
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

	// storageGRPC is the public BigQuery Storage listener (nil when
	// StorageGRPCAddress is empty).
	storageGRPC *grpcserver.Server
}

// New constructs a Gateway. Run actually starts it.
func New(opts Options) *Gateway {
	logger := opts.Logger
	if logger == nil {
		logger = slog.New(slog.DiscardHandler)
	}
	return &Gateway{opts: opts, logger: logger}
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
	ctx := context.Background()
	if g.preStartHook != nil {
		if err := g.preStartHook(g.opts); err != nil {
			return fmt.Errorf("pre-start hook: %w", err)
		}
	}

	if err := g.startEngine(ctx); err != nil {
		return fmt.Errorf("start engine: %w", err)
	}

	if g.postEngineHook != nil {
		if err := g.postEngineHook(g.opts, g.engineClient); err != nil {
			g.stopEngine()
			return fmt.Errorf("post-engine hook: %w", err)
		}
	}

	deps := handlers.BuildDependenciesWith(g.engineClient, handlers.DepsOptions{
		DataDir:          g.opts.DataDir,
		DefaultDatasetID: g.opts.DefaultDatasetID,
	})

	if err := g.startStorageGRPC(ctx, deps); err != nil {
		g.stopEngine()
		return fmt.Errorf("start storage grpc: %w", err)
	}

	srv := &http.Server{
		Addr:              g.opts.HTTPAddress,
		Handler:           NewServer(g.opts, deps, g.engineClient),
		ReadHeaderTimeout: 10 * time.Second,
	}

	errCh := make(chan error, 1)
	go func() {
		g.logStartupExpectations(ctx)
		err := srv.ListenAndServe()
		if err != nil && !errors.Is(err, http.ErrServerClosed) {
			errCh <- err
			return
		}
		errCh <- nil
	}()

	sigCh := make(chan os.Signal, 1)
	signal.Notify(sigCh, os.Interrupt, syscall.SIGTERM)

	return g.waitForShutdown(ctx, srv, errCh, sigCh)
}

// startEngine spawns the C++ engine subprocess if one is configured and
// waits for it to come up. It is a no-op when EngineBinary is empty.
func (g *Gateway) startEngine(ctx context.Context) error {
	if g.opts.EngineBinary == "" {
		return nil
	}

	args := []string{
		"--host_port", g.opts.EngineAddress,
	}
	args = append(args, g.opts.EngineArgs...)
	// #nosec G204 -- engine binary path is operator-supplied via
	// --engine_binary.
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
			g.logger.WarnContext(ctx, "engine subprocess exited",
				slog.Any("err", err))
		}
	}()

	if err := g.connectAndWaitForEngine(ctx); err != nil {
		return err
	}
	return nil
}

// connectAndWaitForEngine dials the engine's gRPC port and polls
// grpc.health.v1.Health.Check until it reports SERVING (or
// engineReadyTimeout fires). Replaces the earlier sleep-and-pray
// stub with a real readiness probe so the gateway's HTTP listener
// never accepts traffic before the engine is actually able to answer
// it.
//
// Stores the live *engine.Client on the receiver for the lifetime of
// the gateway; the connection is reused for every business RPC and torn
// down by stopEngine.
func (g *Gateway) connectAndWaitForEngine(ctx context.Context) error {
	client, err := engine.Dial(g.opts.EngineAddress)
	if err != nil {
		return fmt.Errorf("dial engine at %s: %w", g.opts.EngineAddress, err)
	}

	readyCtx, cancel := context.WithTimeout(ctx, engineReadyTimeout)
	defer cancel()

	if err := client.WaitForReady(readyCtx); err != nil {
		_ = client.Close()
		return fmt.Errorf("wait for engine ready at %s: %w", g.opts.EngineAddress, err)
	}
	g.engineClient = client
	g.logger.InfoContext(ctx, "engine grpc serving",
		slog.String("addr", g.opts.EngineAddress))
	return nil
}

func (g *Gateway) stopStorageGRPC() {
	if g.storageGRPC != nil {
		_ = g.storageGRPC.Close()
		g.storageGRPC = nil
	}
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
