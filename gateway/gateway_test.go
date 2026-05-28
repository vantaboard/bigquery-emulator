package gateway

import (
	"context"
	"errors"
	"net"
	"os"
	"os/exec"
	"path/filepath"
	"runtime"
	"testing"
	"time"

	"github.com/vantaboard/bigquery-emulator/gateway/engine"
)

// emulatorBinaryPath returns a path to a built emulator_main binary, or
// "" if one cannot be located. `task emulator:build-engine-bazel`
// (the canonical engine build) stages the binary into ./bin via
// `task emulator:build-all`. We honor an explicit override via the
// BIGQUERY_EMULATOR_BIN env var so CI can pin a path, then fall back
// to ./bin/emulator_main.
func emulatorBinaryPath(t *testing.T) string {
	t.Helper()
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

// repoRoot walks up from the working directory until it finds a go.mod
// (the repository root). The Go test harness sets WD to the package
// directory; the C++ binary lives in the repo root.
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

// freeTCPPort opens a loopback listener, captures the kernel-assigned
// port number, and immediately closes the listener so the caller can
// hand the port to a subprocess. There is a (tiny) race window between
// Close and the subprocess binding; in practice the kernel does not
// re-hand out the same port that quickly under TIME_WAIT, so this is
// the same pattern net/http/httptest uses for ephemeral ports.
func freeTCPPort(t *testing.T) int {
	t.Helper()
	lis, err := net.Listen("tcp", "127.0.0.1:0")
	if err != nil {
		t.Fatalf("listen: %v", err)
	}
	port := lis.Addr().(*net.TCPAddr).Port
	_ = lis.Close()
	return port
}

// TestGatewayConnectsToEmulatorMain exercises the full
// engine-readiness path end-to-end:
//
//  1. Spawn the real C++ emulator_main subprocess on a free loopback port.
//  2. Dial it with engine.Dial and run WaitForReady.
//  3. Assert grpc.health.v1.Health.Check returned SERVING (proves the
//     C++ ServerBuilder + EnableDefaultHealthCheckService wiring from
//     plan grpc-cpp-server is reachable from Go).
//
// The test is skipped when emulator_main is not on disk; CI runs it
// after the C++ build target completes (see .github/workflows/ci.yml).
//
// We deliberately call into engine.Client directly instead of
// Gateway.Run; Gateway.Run blocks on srv.ListenAndServe and is
// architecturally the wrong shape for a unit test. The readiness
// behavior under test (Dial + health probe) is identical between the
// two call sites, and gateway.go now goes through engine.Client.
func TestGatewayConnectsToEmulatorMain(t *testing.T) {
	if runtime.GOOS == "windows" {
		t.Skip("emulator_main is a POSIX subprocess; Windows path not yet wired")
	}
	bin := emulatorBinaryPath(t)
	if bin == "" {
		t.Skip("emulator_main binary not found; run `task emulator:build-engine` (or set BIGQUERY_EMULATOR_BIN) to enable this integration test")
	}

	port := freeTCPPort(t)
	addr := net.JoinHostPort("127.0.0.1", itoa(port))

	cmd := exec.Command(bin, "--host_port", addr)
	cmd.Stdout = os.Stderr
	cmd.Stderr = os.Stderr
	if err := cmd.Start(); err != nil {
		t.Fatalf("spawn %s: %v", bin, err)
	}
	t.Cleanup(func() {
		if cmd.Process == nil {
			return
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
	})

	client, err := engine.Dial(addr)
	if err != nil {
		t.Fatalf("Dial(%s): %v", addr, err)
	}
	defer func() { _ = client.Close() }()

	ctx, cancel := context.WithTimeout(context.Background(), engineReadyTimeout)
	defer cancel()
	if err := client.WaitForReady(ctx); err != nil {
		t.Fatalf("WaitForReady: %v", err)
	}
}

// itoa avoids pulling in strconv just for one int->string convert in
// the test. Inlined to keep the test file dependency-light.
func itoa(n int) string {
	if n == 0 {
		return "0"
	}
	buf := make([]byte, 0, 6)
	neg := n < 0
	if neg {
		n = -n
	}
	for n > 0 {
		buf = append(buf, byte('0'+n%10))
		n /= 10
	}
	for i, j := 0, len(buf)-1; i < j; i, j = i+1, j-1 {
		buf[i], buf[j] = buf[j], buf[i]
	}
	if neg {
		return "-" + string(buf)
	}
	return string(buf)
}
