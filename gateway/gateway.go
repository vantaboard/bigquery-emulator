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
)

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

	// CopyEngineStdout / CopyEngineStderr forward the engine subprocess's
	// streams to the gateway's own streams.
	CopyEngineStdout bool
	CopyEngineStderr bool

	// LogRequests prints each REST request and response.
	LogRequests bool
}

// Gateway is the top-level BigQuery emulator gateway.
type Gateway struct {
	opts       Options
	engine     *exec.Cmd
	engineDone chan struct{}
}

// New constructs a Gateway. Run actually starts it.
func New(opts Options) *Gateway {
	return &Gateway{opts: opts}
}

// Run starts the engine subprocess (if configured) and the HTTP server,
// then blocks until either terminates or a signal arrives.
func (g *Gateway) Run() error {
	if err := g.startEngine(); err != nil {
		return fmt.Errorf("start engine: %w", err)
	}

	srv := &http.Server{
		Addr:              g.opts.HTTPAddress,
		Handler:           NewServer(g.opts),
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

	// TODO(phase 2): replace this sleep with a real grpc.health.v1 probe
	// against the engine's gRPC port once the engine actually serves it.
	if err := waitForEngine(g.opts.EngineAddress, 10*time.Second); err != nil {
		return err
	}
	return nil
}

func (g *Gateway) stopEngine() {
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

// waitForEngine polls until the engine's gRPC port is reachable. Currently
// a placeholder that just gives the subprocess a moment to start; Phase 2
// will replace this with a proper grpc.health.v1 probe.
func waitForEngine(address string, timeout time.Duration) error {
	_ = address
	_ = timeout
	time.Sleep(200 * time.Millisecond)
	return nil
}
