package handlers

import (
	"context"
	"io"

	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
	"google.golang.org/grpc"
)

// fakeQueryClient is a hand-rolled stub of [enginepb.QueryClient] for
// unit tests. Like fakeCatalogClient, each method either delegates to
// a per-test callback (when set) or returns a default success value,
// which keeps the happy-path tests terse while letting error-path
// tests inject a specific gRPC status.
//
// ExecuteQuery dispatches to a per-test callback when set. The
// default returns nil/nil, which is fine for dry-run tests that
// never invoke it; tests that exercise the streaming path build a
// fakeQueryResultStream and return it from a custom executeQueryFn
// (see queries_test.go).
type fakeQueryClient struct {
	dryRunFn       func(context.Context, *enginepb.QueryRequest) (*enginepb.DryRunResponse, error)
	executeQueryFn func(context.Context, *enginepb.QueryRequest) (grpc.ServerStreamingClient[enginepb.QueryResultRow], error)

	// lastDryRun captures the most recent request so tests can assert
	// on the values forwarded over the wire.
	lastDryRun       *enginepb.QueryRequest
	lastExecuteQuery *enginepb.QueryRequest
}

func (f *fakeQueryClient) DryRun(
	ctx context.Context,
	in *enginepb.QueryRequest,
	_ ...grpc.CallOption,
) (*enginepb.DryRunResponse, error) {
	f.lastDryRun = in
	if f.dryRunFn != nil {
		return f.dryRunFn(ctx, in)
	}
	return &enginepb.DryRunResponse{}, nil
}

func (f *fakeQueryClient) ExecuteQuery(
	ctx context.Context,
	in *enginepb.QueryRequest,
	_ ...grpc.CallOption,
) (grpc.ServerStreamingClient[enginepb.QueryResultRow], error) {
	f.lastExecuteQuery = in
	if f.executeQueryFn != nil {
		return f.executeQueryFn(ctx, in)
	}
	return nil, nil
}

// fakeQueryResultStream is a hand-rolled
// grpc.ServerStreamingClient[enginepb.QueryResultRow] for unit tests.
// It returns the queued messages from Recv in order, then either
// io.EOF (clean termination) or the value of `tailErr` (used to
// inject an UNAVAILABLE / INTERNAL mid-stream and pin the gateway's
// gRPC->HTTP translation).
//
// grpc.ClientStream's auxiliary methods (Header, Trailer,
// CloseSend, Context, SendMsg, RecvMsg) are satisfied by the
// embedded nil interface -- the gateway's stream consumer never
// calls them, so a nil deref would only surface if a future change
// reached past Recv, which is exactly when we'd want a test to fail
// loudly.
type fakeQueryResultStream struct {
	grpc.ClientStream
	msgs    []*enginepb.QueryResultRow
	idx     int
	tailErr error
}

func (s *fakeQueryResultStream) Recv() (*enginepb.QueryResultRow, error) {
	if s.idx >= len(s.msgs) {
		if s.tailErr != nil {
			return nil, s.tailErr
		}
		return nil, io.EOF
	}
	m := s.msgs[s.idx]
	s.idx++
	return m, nil
}
