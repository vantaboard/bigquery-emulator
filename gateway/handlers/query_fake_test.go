package handlers

import (
	"context"

	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
	"google.golang.org/grpc"
)

// fakeQueryClient is a hand-rolled stub of [enginepb.QueryClient] for
// unit tests. Like fakeCatalogClient, each method either delegates to
// a per-test callback (when set) or returns a default success value,
// which keeps the happy-path tests terse while letting error-path
// tests inject a specific gRPC status.
//
// ExecuteQuery is wired but not exercised by Phase 4c tests; it exists
// so the fake satisfies enginepb.QueryClient (a streaming RPC) without
// having to be retrofitted later. The default returns nil/nil so a
// caller that does not set ExecuteQueryFn would get a nil stream --
// fine for the dry-run tests since they never invoke it.
type fakeQueryClient struct {
	dryRunFn       func(context.Context, *enginepb.QueryRequest) (*enginepb.DryRunResponse, error)
	executeQueryFn func(context.Context, *enginepb.QueryRequest) (grpc.ServerStreamingClient[enginepb.QueryResultRow], error)

	// lastDryRun captures the most recent request so tests can assert
	// on the values forwarded over the wire.
	lastDryRun       *enginepb.QueryRequest
	lastExecuteQuery *enginepb.QueryRequest
}

func (f *fakeQueryClient) DryRun(ctx context.Context, in *enginepb.QueryRequest, _ ...grpc.CallOption) (*enginepb.DryRunResponse, error) {
	f.lastDryRun = in
	if f.dryRunFn != nil {
		return f.dryRunFn(ctx, in)
	}
	return &enginepb.DryRunResponse{}, nil
}

func (f *fakeQueryClient) ExecuteQuery(ctx context.Context, in *enginepb.QueryRequest, _ ...grpc.CallOption) (grpc.ServerStreamingClient[enginepb.QueryResultRow], error) {
	f.lastExecuteQuery = in
	if f.executeQueryFn != nil {
		return f.executeQueryFn(ctx, in)
	}
	return nil, nil
}
