package engine

import (
	"context"
	"net"
	"testing"
	"time"

	"google.golang.org/grpc"
	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/health"
	healthpb "google.golang.org/grpc/health/grpc_health_v1"
	"google.golang.org/grpc/status"
)

// startFakeEngine spins up an in-process gRPC server with the standard
// grpc.health.v1 service registered. It mirrors what the real C++
// emulator_main does in frontend/server/server.cc (EnableDefault
// HealthCheckService + SetServingStatus), but stays in-process so the
// Go tests do not need a built C++ binary.
//
// Returns the bound "host:port" address and a stop func that callers
// must defer. The returned *health.Server is exposed so individual
// tests can flip the serving status mid-flight to exercise the
// retry-until-SERVING branch of Client.WaitForReady.
func startFakeEngine(t *testing.T, opts ...func(*health.Server)) (string, *health.Server, func()) {
	t.Helper()

	lis, err := net.Listen("tcp", "127.0.0.1:0")
	if err != nil {
		t.Fatalf("listen: %v", err)
	}

	hs := health.NewServer()
	for _, o := range opts {
		o(hs)
	}

	srv := grpc.NewServer()
	healthpb.RegisterHealthServer(srv, hs)
	go func() {
		_ = srv.Serve(lis)
	}()
	stop := func() {
		srv.Stop()
	}
	return lis.Addr().String(), hs, stop
}

// TestDialEmptyAddress pins the trivial misuse case: an empty address
// should error before we even open a socket.
func TestDialEmptyAddress(t *testing.T) {
	if _, err := Dial(""); err == nil {
		t.Fatal("Dial(\"\") returned nil error")
	}
}

// TestDialPopulatesClients verifies Dial wires up the three logical
// clients consumers care about (Catalog, Query, Health) on top of a
// single shared connection. Nothing is RPC'd; we only assert the
// constructors ran.
func TestDialPopulatesClients(t *testing.T) {
	addr, _, stop := startFakeEngine(t)
	defer stop()

	c, err := Dial(addr)
	if err != nil {
		t.Fatalf("Dial: %v", err)
	}
	defer func() { _ = c.Close() }()

	if c.Catalog == nil {
		t.Fatal("Catalog client is nil")
	}
	if c.Query == nil {
		t.Fatal("Query client is nil")
	}
	if c.Health == nil {
		t.Fatal("Health client is nil")
	}
}

// TestWaitForReadyServing confirms the happy path: a fresh
// health.NewServer marks "" as SERVING out of the box, so WaitForReady
// should return on the first probe.
func TestWaitForReadyServing(t *testing.T) {
	addr, _, stop := startFakeEngine(t)
	defer stop()

	c, err := Dial(addr)
	if err != nil {
		t.Fatalf("Dial: %v", err)
	}
	defer func() { _ = c.Close() }()

	ctx, cancel := context.WithTimeout(context.Background(), 5*time.Second)
	defer cancel()
	if err := c.WaitForReady(ctx); err != nil {
		t.Fatalf("WaitForReady: %v", err)
	}
}

// TestWaitForReadyFlipsToServing simulates a slow-starting engine: the
// fake reports NOT_SERVING for the first ~150ms, then SERVING. This
// covers the retry branch that the old `time.Sleep(200 * time.Millisecond)`
// stub never actually exercised.
func TestWaitForReadyFlipsToServing(t *testing.T) {
	addr, hs, stop := startFakeEngine(t, func(s *health.Server) {
		s.SetServingStatus("", healthpb.HealthCheckResponse_NOT_SERVING)
	})
	defer stop()

	c, err := Dial(addr)
	if err != nil {
		t.Fatalf("Dial: %v", err)
	}
	defer func() { _ = c.Close() }()

	go func() {
		time.Sleep(150 * time.Millisecond)
		hs.SetServingStatus("", healthpb.HealthCheckResponse_SERVING)
	}()

	ctx, cancel := context.WithTimeout(context.Background(), 5*time.Second)
	defer cancel()
	if err := c.WaitForReady(ctx); err != nil {
		t.Fatalf("WaitForReady: %v", err)
	}
}

// TestWaitForReadyHonorsTimeout verifies the loop exits on context
// timeout when the engine never comes up. We dial a port nothing is
// listening on, and the underlying RPCs will return Unavailable; the
// loop should keep retrying until ctx fires.
func TestWaitForReadyHonorsTimeout(t *testing.T) {
	// Reserve and immediately free a port: anything connecting after
	// the listener closes will see ECONNREFUSED, which grpc surfaces
	// as codes.Unavailable -- the canonical "engine not yet up" case.
	lis, err := net.Listen("tcp", "127.0.0.1:0")
	if err != nil {
		t.Fatalf("listen: %v", err)
	}
	addr := lis.Addr().String()
	_ = lis.Close()

	c, err := Dial(addr)
	if err != nil {
		t.Fatalf("Dial: %v", err)
	}
	defer func() { _ = c.Close() }()

	ctx, cancel := context.WithTimeout(context.Background(), 250*time.Millisecond)
	defer cancel()
	err = c.WaitForReady(ctx)
	if err == nil {
		t.Fatal("WaitForReady returned nil; want timeout")
	}
	if ctx.Err() == nil {
		t.Fatal("ctx not cancelled but WaitForReady errored anyway")
	}
}

// TestIsTransientHealthError documents the retry policy: only a small
// well-known set of grpc codes cause WaitForReady to keep polling.
func TestIsTransientHealthError(t *testing.T) {
	cases := []struct {
		code codes.Code
		want bool
	}{
		{codes.Unavailable, true},
		{codes.DeadlineExceeded, true},
		{codes.Canceled, true},
		{codes.ResourceExhausted, true},
		{codes.Unimplemented, false},
		{codes.InvalidArgument, false},
		{codes.NotFound, false},
		{codes.PermissionDenied, false},
	}
	for _, tc := range cases {
		err := status.Error(tc.code, "synthetic")
		if got := isTransientHealthError(err); got != tc.want {
			t.Fatalf("isTransientHealthError(%v) = %v, want %v", tc.code, got, tc.want)
		}
	}
	if isTransientHealthError(nil) {
		t.Fatal("isTransientHealthError(nil) = true, want false")
	}
}
