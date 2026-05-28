package runner

import (
	"bytes"
	"context"
	"errors"
	"fmt"
	"io"
	"net"
	"net/http"
	"net/http/httptest"
	"os"
	"os/exec"
	"path/filepath"
	"strconv"
	"sync"
	"time"

	"github.com/vantaboard/bigquery-emulator/gateway"
	"github.com/vantaboard/bigquery-emulator/gateway/engine"
)

// engineReadyTimeout matches the value the production gateway uses;
// keeps the cold-start budget consistent across CI lanes.
const engineReadyTimeout = 30 * time.Second

// EmulatorEnv is one running emulator the runner can drive: an
// HTTP gateway (in-process) sitting in front of either a subprocess
// engine the runner spawned or an already-running engine the runner
// dialed via `--connect`.
//
// The struct intentionally mirrors `gateway/e2e/catalog_test.go::emulatorEnv`
// so a future plan could fold the two into a shared package. Today
// the e2e harness is in `package e2e` with the `integration` build
// tag, so the runner needs its own copy.
type EmulatorEnv struct {
	// BaseURL is the gateway's HTTP root. Callers concatenate
	// `/bigquery/v2/projects/...` onto it.
	BaseURL string

	httpServer *httptest.Server
	client     *engine.Client

	// cmd is set when the runner spawned `emulator_main` itself.
	// nil when --connect is in use; Close in that mode only tears
	// down the HTTP server + gRPC channel.
	cmd *exec.Cmd

	// dataDir is the temporary `--data_dir` the harness allocated
	// for this emulator. The teardown path removes it.
	dataDir string
}

// Close terminates the subprocess (if any), closes the gRPC channel,
// and shuts down the HTTP gateway. Safe to call more than once.
func (e *EmulatorEnv) Close() error {
	if e == nil {
		return nil
	}
	var firstErr error
	if e.httpServer != nil {
		e.httpServer.Close()
	}
	if e.client != nil {
		if err := e.client.Close(); err != nil && firstErr == nil {
			firstErr = err
		}
	}
	if e.cmd != nil && e.cmd.Process != nil {
		// Best-effort graceful shutdown: SIGINT first, then KILL
		// after a short budget. The C++ engine registers a SIGINT
		// handler that flushes the storage layer; KILL is the
		// belt-and-suspenders path for a wedged subprocess.
		_ = e.cmd.Process.Signal(os.Interrupt)
		done := make(chan struct{})
		go func() {
			_, _ = e.cmd.Process.Wait()
			close(done)
		}()
		select {
		case <-done:
		case <-time.After(5 * time.Second):
			_ = e.cmd.Process.Kill()
			<-done
		}
	}
	if e.dataDir != "" {
		if err := os.RemoveAll(e.dataDir); err != nil && firstErr == nil {
			firstErr = err
		}
	}
	return firstErr
}

// HarnessOptions configures how the runner spins up the emulator for
// one fixture x profile execution. Callers either set EngineBinary
// (the runner spawns its own emulator subprocess and tears it down
// after the fixture) or ConnectAddress (the runner dials an
// already-running engine on `host:port`).
type HarnessOptions struct {
	// EngineBinary is the path to `emulator_main`. Defaults to
	// `./bin/emulator_main` when empty. Mutually exclusive with
	// ConnectAddress.
	EngineBinary string

	// ConnectAddress is `host:port` for an already-running engine.
	// Empty means the harness spawns its own subprocess. Mutually
	// exclusive with EngineBinary.
	ConnectAddress string

	// EngineStdout / EngineStderr receive the engine subprocess's
	// streams. nil discards them; tests typically pass `os.Stderr`
	// to keep crash output visible.
	EngineStdout io.Writer
	EngineStderr io.Writer

	// DataDirRoot is the parent directory under which the harness
	// allocates per-emulator `--data_dir` paths for the DuckDB
	// profile. Empty defers to `os.TempDir()`.
	DataDirRoot string
}

// validate enforces the exclusivity contract between EngineBinary and
// ConnectAddress and resolves the EngineBinary default.
func (o *HarnessOptions) validate() error {
	if o.EngineBinary != "" && o.ConnectAddress != "" {
		return errors.New("HarnessOptions: --engine-binary and --connect are mutually exclusive")
	}
	if o.EngineBinary == "" && o.ConnectAddress == "" {
		o.EngineBinary = filepath.Join(".", "bin", "emulator_main")
	}
	return nil
}

// StartEmulator spins up an EmulatorEnv for the given profile. The
// returned env owns its subprocess (if any) and must be Closed by the
// caller -- the harness registers no global cleanup, so the caller
// (typically the runner loop) is responsible for orderly teardown.
//
// When ConnectAddress is set the profile only controls which fixtures
// run against the connected gateway: the harness does not push
// `--engine` / `--storage` over the wire, so the connected emulator
// must already be configured for the requested profile (CI wires
// this).
func StartEmulator(ctx context.Context, opts HarnessOptions, p Profile) (*EmulatorEnv, error) {
	if err := opts.validate(); err != nil {
		return nil, err
	}
	if opts.ConnectAddress != "" {
		return startConnected(ctx, opts)
	}
	return startSpawned(ctx, opts, p)
}

// startConnected dials an already-running engine on the configured
// address and wires an in-process HTTP gateway in front of it. The
// returned env's cmd field is nil; Close only releases the channel
// and the HTTP server.
func startConnected(ctx context.Context, opts HarnessOptions) (*EmulatorEnv, error) {
	client, err := engine.Dial(opts.ConnectAddress)
	if err != nil {
		return nil, fmt.Errorf("dial connect=%s: %w", opts.ConnectAddress, err)
	}
	if err := client.WaitForReady(ctx); err != nil {
		_ = client.Close()
		return nil, fmt.Errorf("connected engine not ready at %s: %w",
			opts.ConnectAddress, err)
	}
	handler := gateway.NewServer(gateway.Options{}, client)
	srv := httptest.NewServer(handler)
	return &EmulatorEnv{
		BaseURL:    srv.URL,
		httpServer: srv,
		client:     client,
	}, nil
}

// startSpawned launches a fresh `emulator_main` subprocess with the
// profile's flags, waits for its gRPC health service to flip to
// SERVING, and returns an env that owns the subprocess.
func startSpawned(ctx context.Context, opts HarnessOptions, p Profile) (*EmulatorEnv, error) {
	if _, err := os.Stat(opts.EngineBinary); err != nil {
		return nil, fmt.Errorf("engine binary not found at %s: %w "+
			"(build with `task emulator:build-engine:bazel` or pass "+
			"--connect HOST:PORT)", opts.EngineBinary, err)
	}
	port, err := freePort()
	if err != nil {
		return nil, fmt.Errorf("allocate engine port: %w", err)
	}
	addr := net.JoinHostPort("127.0.0.1", strconv.Itoa(port))
	args := append([]string{"--host_port", addr}, p.EmulatorMainArgs()...)
	// DuckDB storage always needs a persistent --data_dir; give each
	// spawn its own temp directory so concurrent profile runs do not
	// collide on the same catalog.
	root := opts.DataDirRoot
	if root == "" {
		root = os.TempDir()
	}
	dataDir, err := os.MkdirTemp(root, "bq-conformance-")
	if err != nil {
		return nil, fmt.Errorf("create data_dir: %w", err)
	}
	args = append(args, "--data_dir", dataDir)
	cmd := exec.Command(opts.EngineBinary, args...)
	cmd.Stdout = opts.EngineStdout
	cmd.Stderr = opts.EngineStderr
	if err := cmd.Start(); err != nil {
		if dataDir != "" {
			_ = os.RemoveAll(dataDir)
		}
		return nil, fmt.Errorf("spawn %s: %w", opts.EngineBinary, err)
	}

	// Once the process is alive, every error path needs to reap it.
	// `cleanup` runs on every failure below; success transfers
	// ownership to EmulatorEnv.
	var (
		client *engine.Client
		srv    *httptest.Server
		once   sync.Once
	)
	cleanup := func() {
		once.Do(func() {
			if srv != nil {
				srv.Close()
			}
			if client != nil {
				_ = client.Close()
			}
			_ = cmd.Process.Signal(os.Interrupt)
			done := make(chan struct{})
			go func() {
				_, _ = cmd.Process.Wait()
				close(done)
			}()
			select {
			case <-done:
			case <-time.After(5 * time.Second):
				_ = cmd.Process.Kill()
				<-done
			}
			if dataDir != "" {
				_ = os.RemoveAll(dataDir)
			}
		})
	}

	client, err = engine.Dial(addr)
	if err != nil {
		cleanup()
		return nil, fmt.Errorf("dial %s: %w", addr, err)
	}

	readyCtx, cancel := context.WithTimeout(ctx, engineReadyTimeout)
	defer cancel()
	if err := client.WaitForReady(readyCtx); err != nil {
		cleanup()
		return nil, fmt.Errorf("emulator at %s not ready: %w", addr, err)
	}

	srv = httptest.NewServer(gateway.NewServer(gateway.Options{}, client))
	return &EmulatorEnv{
		BaseURL:    srv.URL,
		httpServer: srv,
		client:     client,
		cmd:        cmd,
		dataDir:    dataDir,
	}, nil
}

// freePort returns an available loopback TCP port. Mirrors the
// pattern `net/http/httptest` uses: bind on :0, capture the port,
// close immediately. The race with the subprocess's bind is the same
// race the standard library accepts.
func freePort() (int, error) {
	lis, err := net.Listen("tcp", "127.0.0.1:0")
	if err != nil {
		return 0, err
	}
	port := lis.Addr().(*net.TCPAddr).Port
	if err := lis.Close(); err != nil {
		return 0, err
	}
	return port, nil
}

// doRequest is the runner's slimmed-down HTTP helper. It POSTs JSON
// bodies (or issues a GET) and returns the status + body. Errors are
// wrapped with the URL so the runner-internal error path is easy to
// debug.
func doRequest(ctx context.Context, method, url string, body []byte) (int, []byte, error) {
	var reader io.Reader
	if body != nil {
		reader = bytes.NewReader(body)
	}
	req, err := http.NewRequestWithContext(ctx, method, url, reader)
	if err != nil {
		return 0, nil, fmt.Errorf("build request %s %s: %w", method, url, err)
	}
	if body != nil {
		req.Header.Set("Content-Type", "application/json")
	}
	resp, err := http.DefaultClient.Do(req)
	if err != nil {
		return 0, nil, fmt.Errorf("http %s %s: %w", method, url, err)
	}
	defer func() { _ = resp.Body.Close() }()
	respBody, err := io.ReadAll(resp.Body)
	if err != nil {
		return resp.StatusCode, nil, fmt.Errorf("read body from %s %s: %w",
			method, url, err)
	}
	return resp.StatusCode, respBody, nil
}
