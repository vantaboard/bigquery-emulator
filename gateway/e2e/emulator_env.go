//go:build integration

package e2e

import (
	"context"
	"errors"
	"fmt"
	"net"
	"net/http/httptest"
	"os"
	"os/exec"
	"path/filepath"
	"runtime"
	"strconv"
	"strings"
	"testing"
	"time"

	"github.com/vantaboard/bigquery-emulator/gateway"
	"github.com/vantaboard/bigquery-emulator/gateway/engine"
	"github.com/vantaboard/bigquery-emulator/gateway/handlers"
)

// engineReadyTimeout matches the gateway package's startup probe
// budget; see gateway/gateway.go.
const engineReadyTimeout = 30 * time.Second

// emulatorBinaryPath locates the C++ emulator_main binary by checking
// (in order) BIGQUERY_EMULATOR_BIN, then ./bin/emulator_main staged
// by `task emulator:build-engine-bazel`. Returns "" when nothing is
// found so the test can Skip rather than fail.
func emulatorBinaryPath() string {
	if p := os.Getenv("BIGQUERY_EMULATOR_BIN"); p != "" {
		if _, err := os.Stat(p); err == nil {
			return p
		}
	}
	root, err := repoRoot()
	if err != nil {
		return ""
	}
	candidate := filepath.Join(root, "bin", "emulator_main")
	if _, err := os.Stat(candidate); err == nil {
		return candidate
	}
	return ""
}

// repoRoot walks upward from the current directory until it finds the
// go.mod that marks the repository root.
func repoRoot() (string, error) {
	wd, err := os.Getwd()
	if err != nil {
		return "", err
	}
	dir := wd
	for {
		if _, err := os.Stat(filepath.Join(dir, "go.mod")); err == nil {
			return dir, nil
		}
		parent := filepath.Dir(dir)
		if parent == dir {
			return "", errors.New("no go.mod found upstream of " + wd)
		}
		dir = parent
	}
}

// emulatorEnv bundles everything the test needs to drive the gateway:
// the in-process HTTP server (backed by gateway.NewServer + a real
// engine.Client) and the engine subprocess we spawned. tearDown
// shuts everything down cleanly so the test does not leak
// subprocesses across runs.
type emulatorEnv struct {
	httpServer *httptest.Server
	client     *engine.Client
	cmd        *exec.Cmd
	// engineAddr is the `host:port` the engine subprocess is listening
	// on for its gRPC surface (Catalog + Query + StorageRead). The
	// gateway's REST handlers reach it via `client`; tests that drive
	// the engine directly (plan 39 StorageRead integration) dial this
	// address to open their own channel.
	engineAddr string
	// dataDir is the `--data_dir` passed when spawning emulator_main.
	// The query port TestMain harness reuses it when restarting after
	// an engine crash (503 EOF / connection refused).
	dataDir string
}

func (e *emulatorEnv) URL() string { return e.httpServer.URL }

// EngineAddress returns the `host:port` for the engine subprocess's
// gRPC server. Plan 39's StorageRead E2E uses this to open a
// dedicated channel that mirrors the way real BigQuery clients dial
// the Storage Read endpoint (which is a separate gRPC service from
// the REST surface, so the gateway does not proxy it — see
// `docs/REST_API.md#storage-read-api`).
func (e *emulatorEnv) EngineAddress() string { return e.engineAddr }

func (e *emulatorEnv) tearDown() {
	e.httpServer.Close()
	if e.client != nil {
		_ = e.client.Close()
	}
	if e.cmd == nil || e.cmd.Process == nil {
		return
	}
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

// emulatorFlags bundles the subset of `emulator_main` flags the E2E
// tests need to control. Zero values mean "do not pass the flag" so
// the emulator's own defaults apply (DuckDB engine + DuckDB storage
// rooted at `$HOME/.bigquery-emulator`). Tests that want a hermetic
// data directory populate `dataDir`.
type emulatorFlags struct {
	// dataDir maps to `--data_dir`. Empty defers to the emulator's
	// default; tests that want a hermetic data dir pass `t.TempDir()`.
	dataDir string
}

// startEmulator is the default-configuration helper for tests that
// only need a fresh DuckDB-backed emulator with a per-test data
// directory.
func startEmulator(t *testing.T) *emulatorEnv {
	t.Helper()
	return startEmulatorWithFlags(t, emulatorFlags{dataDir: t.TempDir()})
}

// launchEmulator starts emulator_main and an in-process gateway. Used by
// TestMain for the query port and by startEmulatorWithFlags.
func launchEmulator(dataDir string) (*emulatorEnv, error) {
	if runtime.GOOS == "windows" {
		return nil, errors.New("emulator_main is a POSIX subprocess; Windows is not yet wired")
	}
	bin := emulatorBinaryPath()
	if bin == "" {
		return nil, errors.New("emulator_main binary not found; run `task emulator:build-engine` or set BIGQUERY_EMULATOR_BIN")
	}
	lis, err := net.Listen("tcp", "127.0.0.1:0")
	if err != nil {
		return nil, fmt.Errorf("listen: %w", err)
	}
	port := lis.Addr().(*net.TCPAddr).Port
	_ = lis.Close()
	addr := net.JoinHostPort("127.0.0.1", strconv.Itoa(port))

	args := []string{"--host_port", addr}
	if dataDir != "" {
		args = append(args, "--data_dir", dataDir)
	}
	cmd := exec.Command(bin, args...)
	cmd.Stdout = os.Stderr
	cmd.Stderr = os.Stderr
	if err := cmd.Start(); err != nil {
		return nil, fmt.Errorf("spawn %s: %w", bin, err)
	}
	client, err := engine.Dial(addr)
	if err != nil {
		_ = cmd.Process.Kill()
		return nil, fmt.Errorf("Dial(%s): %w", addr, err)
	}
	ctx, cancel := context.WithTimeout(context.Background(), engineReadyTimeout)
	defer cancel()
	if err := client.WaitForReady(ctx); err != nil {
		_ = client.Close()
		_ = cmd.Process.Kill()
		return nil, fmt.Errorf("WaitForReady: %w", err)
	}
	handler := gateway.NewServer(gateway.Options{}, handlers.BuildDependencies(client), client)
	httpServer := httptest.NewServer(handler)
	return &emulatorEnv{
		httpServer: httpServer,
		client:     client,
		cmd:        cmd,
		engineAddr: addr,
		dataDir:    dataDir,
	}, nil
}

// launchEmulatorForMain is launchEmulator with a dedicated data directory for
// the query_port_test port (TestMain has no *testing.T).
func launchEmulatorForMain() (*emulatorEnv, error) {
	dir, err := os.MkdirTemp("", "bq-emulator-query-*")
	if err != nil {
		return nil, err
	}
	return launchEmulator(dir)
}

// startEmulatorWithFlags launches the engine subprocess with the
// given flag overrides and wires up an in-process gateway HTTP server
// dialed at it. The test is responsible for tearing it down via the
// returned cleanup function (registered with `t.Cleanup`).
func startEmulatorWithFlags(t *testing.T, flags emulatorFlags) *emulatorEnv {
	t.Helper()
	if runtime.GOOS == "windows" {
		t.Skip("emulator_main is a POSIX subprocess; Windows is not yet wired")
	}
	env, err := launchEmulator(flags.dataDir)
	if err != nil {
		if strings.Contains(err.Error(), "not found") {
			t.Skip(err.Error())
		}
		t.Fatal(err)
	}
	t.Cleanup(env.tearDown)
	return env
}
