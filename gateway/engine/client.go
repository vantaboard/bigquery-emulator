// Package engine is the Go-side gRPC client for the BigQuery emulator's
// C++ engine.
//
// The gateway and engine are two separate processes that talk over an
// in-process gRPC channel (see proto/emulator.proto and
// gateway/enginepb). This package wraps the dial / health-probe / close
// dance so the gateway lifecycle code in gateway.go and the per-request
// HTTP handlers in gateway/handlers can share one connection.
//
// Client mirrors the way cloud-spanner-emulator's gateway connects to
// emulator_main: a single insecure loopback channel, one shared
// connection per gateway process, health checked via grpc.health.v1
// before any business RPCs are dispatched.
package engine

import (
	"context"
	"errors"
	"fmt"
	"time"

	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
	"google.golang.org/grpc"
	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/credentials/insecure"
	healthpb "google.golang.org/grpc/health/grpc_health_v1"
	"google.golang.org/grpc/status"
)

// Client is a thin facade around the *grpc.ClientConn that the gateway
// uses to talk to the C++ engine. It owns the connection so callers can
// share one channel across all handlers and only have to Close once at
// shutdown.
//
// Catalog and Query are the two business-logic clients defined in
// proto/emulator.proto; StorageRead and StorageWrite are the internal
// storage contracts the public bqstorage shim adapts. Health is the
// standard grpc.health.v1 probe the engine wires up via
// grpc::EnableDefaultHealthCheckService (see frontend/server/server.cc).
type Client struct {
	conn *grpc.ClientConn

	Catalog      enginepb.CatalogClient
	Query        enginepb.QueryClient
	SQLTools     enginepb.SqlToolsClient
	StorageRead  enginepb.StorageReadClient
	StorageWrite enginepb.StorageWriteClient
	Health       healthpb.HealthClient
}

// Dial opens a gRPC channel to the engine listening at address (typically
// "host:port" on the loopback interface) and returns a Client that wraps
// it. The connection uses insecure credentials because the channel never
// leaves the local machine; the engine subprocess's listening port is a
// gateway-internal contract, not a public API.
//
// Dial does not wait for the engine to be ready. Call WaitForReady (or
// the gateway's own startup probe) before issuing business RPCs. The
// returned Client owns its connection; callers must Close it at
// shutdown.
func Dial(address string) (*Client, error) {
	if address == "" {
		return nil, errors.New("engine: empty engine address")
	}
	conn, err := grpc.NewClient(
		address,
		grpc.WithTransportCredentials(insecure.NewCredentials()),
	)
	if err != nil {
		return nil, fmt.Errorf("engine: dial %s: %w", address, err)
	}
	return &Client{
		conn:         conn,
		Catalog:      enginepb.NewCatalogClient(conn),
		Query:        enginepb.NewQueryClient(conn),
		SQLTools:     enginepb.NewSqlToolsClient(conn),
		StorageRead:  enginepb.NewStorageReadClient(conn),
		StorageWrite: enginepb.NewStorageWriteClient(conn),
		Health:       healthpb.NewHealthClient(conn),
	}, nil
}

// Close releases the underlying gRPC channel. It is safe to call on a
// nil receiver (gateway constructed without an engine subprocess). It
// is also idempotent; subsequent calls are no-ops because *grpc.ClientConn
// itself is idempotent on Close.
func (c *Client) Close() error {
	if c == nil || c.conn == nil {
		return nil
	}
	return c.conn.Close()
}

// healthRetryInterval is the gap between successive grpc.health.v1.Check
// probes inside WaitForReady. Tuned to keep the worst-case startup
// latency low (we expect the engine subprocess to bind its socket within
// a few hundred milliseconds) without burning CPU on tight retries.
const healthRetryInterval = 100 * time.Millisecond

// WaitForReady polls grpc.health.v1.Health.Check on the empty service
// name until it reports SERVING. A SERVING response means the engine has
// finished BuildAndStart and called SetServingStatus("", true) (see
// frontend/server/server.cc), which is the moment business RPCs become
// safe to issue.
//
// The loop is bounded by ctx; callers typically wrap a context.Background
// with a 30s timeout (see gateway.waitForEngine). Transient errors
// (Unavailable, DeadlineExceeded, Connection refused before the engine
// has started listening) are retried at healthRetryInterval; non-
// transient errors (for example Unimplemented, returned by an engine
// without the health service registered) are surfaced immediately so we
// fail fast instead of waiting out the timeout.
//
// Returns nil on SERVING, ctx.Err() on timeout/cancel, or a wrapped
// status error for non-retriable conditions.
func (c *Client) WaitForReady(ctx context.Context) error {
	if c == nil {
		return errors.New("engine: nil client")
	}
	req := &healthpb.HealthCheckRequest{Service: ""}
	for {
		// Each Check inherits the outer deadline so the loop cannot run
		// past it; the per-RPC deadline is the only timeout grpc-go
		// honors here.
		resp, err := c.Health.Check(ctx, req)
		switch {
		case err == nil && resp.GetStatus() == healthpb.HealthCheckResponse_SERVING:
			return nil
		case err == nil:
			// Engine reachable but not yet SERVING (NOT_SERVING /
			// SERVICE_UNKNOWN / UNKNOWN). Keep polling; the engine may
			// flip to SERVING once it finishes initialization.
		case isTransientHealthError(err):
			// Engine still starting up: socket not yet listening, RPC
			// queue not yet ready. Sleep and retry.
		default:
			return fmt.Errorf("engine: health check: %w", err)
		}

		select {
		case <-ctx.Done():
			return fmt.Errorf("engine: wait for ready: %w", ctx.Err())
		case <-time.After(healthRetryInterval):
		}
	}
}

// isTransientHealthError reports whether err looks like the engine
// simply has not finished booting yet, so the caller should retry. Any
// other error (Unimplemented, InvalidArgument, ...) is a real failure
// that should surface immediately.
func isTransientHealthError(err error) bool {
	if err == nil {
		return false
	}
	switch status.Code(err) {
	case codes.Unavailable, codes.DeadlineExceeded, codes.Canceled, codes.ResourceExhausted:
		return true
	default:
		return false
	}
}
