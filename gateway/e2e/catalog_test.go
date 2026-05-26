//go:build integration

// Package e2e holds end-to-end tests that exercise the BigQuery
// emulator REST gateway against a real C++ engine subprocess
// (emulator_main). They are gated behind the `integration` build tag
// so `go test ./...` stays hermetic on machines without a CMake build,
// and are skipped when the engine binary cannot be located.
//
// Run via:
//
//	go test -tags=integration ./gateway/e2e/...
//
// The tests build a gateway HTTP handler in-process (using
// gateway.NewServer plus an engine.Client dialed at the engine's
// socket) and drive it through httptest. This keeps the runtime
// arrangement faithful to production (HTTP -> gRPC -> Storage) while
// letting the tests assert on the REST wire shape directly.
package e2e

import (
	"bytes"
	"context"
	"encoding/json"
	"errors"
	"io"
	"net"
	"net/http"
	"net/http/httptest"
	"os"
	"os/exec"
	"path/filepath"
	"runtime"
	"strconv"
	"testing"
	"time"

	"github.com/vantaboard/bigquery-emulator/gateway"
	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
	"github.com/vantaboard/bigquery-emulator/gateway/engine"
)

// engineReadyTimeout matches the gateway package's startup probe
// budget; see gateway/gateway.go.
const engineReadyTimeout = 30 * time.Second

// emulatorBinaryPath locates the C++ emulator_main binary by checking
// (in order) BIGQUERY_EMULATOR_BIN, ./bin/, ./build-out/. Returns ""
// when nothing is found so the test can Skip rather than fail.
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
	for _, rel := range []string{
		filepath.Join("bin", "emulator_main"),
		filepath.Join("build-out", "emulator_main"),
	} {
		candidate := filepath.Join(root, rel)
		if _, err := os.Stat(candidate); err == nil {
			return candidate
		}
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

// freeTCPPort returns an available loopback port. We close the
// listener immediately; the small race window with the subprocess's
// bind is the same pattern net/http/httptest uses.
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
// the emulator's own defaults apply (reference_impl engine + in-memory
// storage). Tests that need the canonical Phase 5i `--engine=duckdb
// --storage=duckdb --on_unknown_fn=fallback` configuration construct a
// fully-populated `emulatorFlags`.
type emulatorFlags struct {
	// engine maps to `--engine`. Empty leaves the flag off so the
	// emulator's default (`reference_impl`) applies.
	engine string
	// storage maps to `--storage`. Empty leaves the flag off so the
	// emulator's default (`memory`) applies.
	storage string
	// onUnknownFn maps to `--on_unknown_fn`. Empty leaves the flag off
	// so the emulator's default (`unimplemented`) applies.
	onUnknownFn string
	// dataDir maps to `--data_dir`. Empty defers to the emulator's
	// default; tests that want a hermetic data dir pass `t.TempDir()`.
	dataDir string
}

// startEmulator is the default-configuration helper that mirrors the
// pre-Phase-5i behavior: `--storage=memory` with no engine override.
// New tests that need a different engine/storage combination call
// `startEmulatorWithFlags` directly.
func startEmulator(t *testing.T) *emulatorEnv {
	t.Helper()
	return startEmulatorWithFlags(t, emulatorFlags{storage: "memory"})
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
	bin := emulatorBinaryPath()
	if bin == "" {
		t.Skip("emulator_main binary not found; run `task emulator:build-engine` " +
			"or set BIGQUERY_EMULATOR_BIN to enable the integration tests")
	}

	port := freeTCPPort(t)
	addr := net.JoinHostPort("127.0.0.1", strconv.Itoa(port))

	args := []string{"--host_port", addr}
	if flags.engine != "" {
		args = append(args, "--engine", flags.engine)
	}
	if flags.storage != "" {
		args = append(args, "--storage", flags.storage)
	}
	if flags.onUnknownFn != "" {
		args = append(args, "--on_unknown_fn", flags.onUnknownFn)
	}
	if flags.dataDir != "" {
		args = append(args, "--data_dir", flags.dataDir)
	}

	cmd := exec.Command(bin, args...)
	cmd.Stdout = os.Stderr
	cmd.Stderr = os.Stderr
	if err := cmd.Start(); err != nil {
		t.Fatalf("spawn %s: %v", bin, err)
	}

	client, err := engine.Dial(addr)
	if err != nil {
		_ = cmd.Process.Kill()
		t.Fatalf("Dial(%s): %v", addr, err)
	}
	ctx, cancel := context.WithTimeout(context.Background(), engineReadyTimeout)
	defer cancel()
	if err := client.WaitForReady(ctx); err != nil {
		_ = client.Close()
		_ = cmd.Process.Kill()
		t.Fatalf("WaitForReady: %v", err)
	}

	handler := gateway.NewServer(gateway.Options{}, client)
	httpServer := httptest.NewServer(handler)

	env := &emulatorEnv{
		httpServer: httpServer,
		client:     client,
		cmd:        cmd,
		engineAddr: addr,
	}
	t.Cleanup(env.tearDown)
	return env
}

// doJSON issues an HTTP request with a JSON body (when provided) and
// returns the response status, decoded body bytes, and any transport
// error. Body is closed before returning.
func doJSON(t *testing.T, method, url string, body []byte) (int, []byte) {
	t.Helper()
	var reqBody io.Reader
	if body != nil {
		reqBody = bytes.NewReader(body)
	}
	req, err := http.NewRequest(method, url, reqBody)
	if err != nil {
		t.Fatalf("NewRequest: %v", err)
	}
	if body != nil {
		req.Header.Set("Content-Type", "application/json")
	}
	resp, err := http.DefaultClient.Do(req)
	if err != nil {
		t.Fatalf("HTTP %s %s: %v", method, url, err)
	}
	defer resp.Body.Close()
	respBody, err := io.ReadAll(resp.Body)
	if err != nil {
		t.Fatalf("read body: %v", err)
	}
	return resp.StatusCode, respBody
}

// TestTabledataRoundTrip is the Phase 3j end-to-end story: create a
// dataset and a table over REST, stream two rows in with
// tabledata.insertAll, and read them back out with tabledata.list.
// Exercises the full gateway -> Catalog gRPC -> InMemoryStorage path.
func TestTabledataRoundTrip(t *testing.T) {
	env := startEmulator(t)

	const (
		projectID = "proj-e2e"
		datasetID = "ds_e2e"
		tableID   = "people"
	)
	base := env.URL() + "/bigquery/v2/projects/" + projectID

	// 1. Create the dataset.
	status, body := doJSON(t, http.MethodPost, base+"/datasets",
		[]byte(`{"datasetReference":{"projectId":"`+projectID+
			`","datasetId":"`+datasetID+`"},"location":"US"}`))
	if status != http.StatusOK {
		t.Fatalf("datasets.insert -> %d: %s", status, string(body))
	}

	// 2. Create the table with the expected schema.
	tableBody := `{
        "tableReference":{"projectId":"` + projectID +
		`","datasetId":"` + datasetID +
		`","tableId":"` + tableID + `"},
        "schema":{"fields":[
            {"name":"id","type":"INT64","mode":"REQUIRED"},
            {"name":"name","type":"STRING","mode":"NULLABLE"},
            {"name":"tags","type":"STRING","mode":"REPEATED"}
        ]}
    }`
	status, body = doJSON(t, http.MethodPost,
		base+"/datasets/"+datasetID+"/tables", []byte(tableBody))
	if status != http.StatusOK {
		t.Fatalf("tables.insert -> %d: %s", status, string(body))
	}

	// 3. Stream two rows via tabledata.insertAll.
	insertBody := `{
        "rows":[
            {"insertId":"a","json":{"id":1,"name":"alice","tags":["x","y"]}},
            {"insertId":"b","json":{"id":2,"name":"bob","tags":[]}}
        ]
    }`
	status, body = doJSON(t, http.MethodPost,
		base+"/datasets/"+datasetID+"/tables/"+tableID+"/insertAll",
		[]byte(insertBody))
	if status != http.StatusOK {
		t.Fatalf("tabledata.insertAll -> %d: %s", status, string(body))
	}
	var ins bqtypes.TableDataInsertAllResponse
	if err := json.Unmarshal(body, &ins); err != nil {
		t.Fatalf("decode insertAll response: %v (body=%s)", err, string(body))
	}
	if ins.Kind != "bigquery#tableDataInsertAllResponse" {
		t.Errorf("insertAll kind = %q", ins.Kind)
	}
	if len(ins.InsertErrors) != 0 {
		t.Errorf("insertAll insertErrors = %+v, want empty", ins.InsertErrors)
	}

	// 4. tabledata.list must round-trip the same two rows.
	status, body = doJSON(t, http.MethodGet,
		base+"/datasets/"+datasetID+"/tables/"+tableID+"/data", nil)
	if status != http.StatusOK {
		t.Fatalf("tabledata.list -> %d: %s", status, string(body))
	}
	var page bqtypes.TableDataList
	if err := json.Unmarshal(body, &page); err != nil {
		t.Fatalf("decode tabledata.list response: %v (body=%s)", err, string(body))
	}
	if page.Kind != "bigquery#tableDataList" {
		t.Errorf("list kind = %q", page.Kind)
	}
	if page.TotalRows != "2" {
		t.Errorf("totalRows = %q, want %q", page.TotalRows, "2")
	}
	if len(page.Rows) != 2 {
		t.Fatalf("rows = %d, want 2: %+v", len(page.Rows), page.Rows)
	}
	if page.PageToken != "" {
		t.Errorf("pageToken = %q, want empty when all rows fit on one page",
			page.PageToken)
	}

	got := []string{}
	for _, r := range page.Rows {
		if len(r.F) != 3 {
			t.Fatalf("row cells = %d, want 3: %+v", len(r.F), r.F)
		}
		idStr, _ := r.F[0].V.(string)
		nameStr, _ := r.F[1].V.(string)
		got = append(got, idStr+":"+nameStr)
	}
	wantPairs := map[string]bool{"1:alice": true, "2:bob": true}
	for _, g := range got {
		if !wantPairs[g] {
			t.Errorf("unexpected row %q (want one of %v)", g, wantPairs)
		}
		delete(wantPairs, g)
	}
	if len(wantPairs) != 0 {
		t.Errorf("missing rows: %v", wantPairs)
	}
}
