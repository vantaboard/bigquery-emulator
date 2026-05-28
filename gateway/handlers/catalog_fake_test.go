package handlers

import (
	"context"

	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
	"google.golang.org/grpc"
)

// fakeCatalogClient is a hand-rolled stub of [enginepb.CatalogClient]
// for unit tests. Each method either delegates to a per-test callback
// (when set) or returns a default success response, which keeps the
// happy-path tests terse while still letting the error-path tests
// inject a specific gRPC status code.
//
// We avoid pulling in a generated mock (gomock / mockgen) because the
// gateway has no other use for one yet and the interface is tiny.
type fakeCatalogClient struct {
	registerDatasetFn func(context.Context, *enginepb.RegisterDatasetRequest) (*enginepb.RegisterDatasetResponse, error)
	dropDatasetFn     func(context.Context, *enginepb.DropDatasetRequest) (*enginepb.DropDatasetResponse, error)
	registerTableFn   func(context.Context, *enginepb.RegisterTableRequest) (*enginepb.RegisterTableResponse, error)
	dropTableFn       func(context.Context, *enginepb.DropTableRequest) (*enginepb.DropTableResponse, error)
	describeTableFn   func(context.Context, *enginepb.DescribeTableRequest) (*enginepb.DescribeTableResponse, error)
	insertRowsFn      func(context.Context, *enginepb.InsertRowsRequest) (*enginepb.InsertRowsResponse, error)
	listRowsFn        func(context.Context, *enginepb.ListRowsRequest) (*enginepb.ListRowsResponse, error)

	// lastRegisterDataset captures the most recent request so tests
	// can assert on the values forwarded over the wire without having
	// to set a callback on every test.
	lastRegisterDataset *enginepb.RegisterDatasetRequest
	lastDropDataset     *enginepb.DropDatasetRequest
	lastRegisterTable   *enginepb.RegisterTableRequest
	lastDropTable       *enginepb.DropTableRequest
	lastDescribeTable   *enginepb.DescribeTableRequest
	lastInsertRows      *enginepb.InsertRowsRequest
	lastListRows        *enginepb.ListRowsRequest
}

func (f *fakeCatalogClient) RegisterDataset(
	ctx context.Context,
	in *enginepb.RegisterDatasetRequest,
	_ ...grpc.CallOption,
) (*enginepb.RegisterDatasetResponse, error) {
	f.lastRegisterDataset = in
	if f.registerDatasetFn != nil {
		return f.registerDatasetFn(ctx, in)
	}
	return &enginepb.RegisterDatasetResponse{}, nil
}

func (f *fakeCatalogClient) DropDataset(
	ctx context.Context,
	in *enginepb.DropDatasetRequest,
	_ ...grpc.CallOption,
) (*enginepb.DropDatasetResponse, error) {
	f.lastDropDataset = in
	if f.dropDatasetFn != nil {
		return f.dropDatasetFn(ctx, in)
	}
	return &enginepb.DropDatasetResponse{}, nil
}

func (f *fakeCatalogClient) RegisterTable(
	ctx context.Context,
	in *enginepb.RegisterTableRequest,
	_ ...grpc.CallOption,
) (*enginepb.RegisterTableResponse, error) {
	f.lastRegisterTable = in
	if f.registerTableFn != nil {
		return f.registerTableFn(ctx, in)
	}
	return &enginepb.RegisterTableResponse{}, nil
}

func (f *fakeCatalogClient) DropTable(
	ctx context.Context,
	in *enginepb.DropTableRequest,
	_ ...grpc.CallOption,
) (*enginepb.DropTableResponse, error) {
	f.lastDropTable = in
	if f.dropTableFn != nil {
		return f.dropTableFn(ctx, in)
	}
	return &enginepb.DropTableResponse{}, nil
}

func (f *fakeCatalogClient) DescribeTable(
	ctx context.Context,
	in *enginepb.DescribeTableRequest,
	_ ...grpc.CallOption,
) (*enginepb.DescribeTableResponse, error) {
	f.lastDescribeTable = in
	if f.describeTableFn != nil {
		return f.describeTableFn(ctx, in)
	}
	return &enginepb.DescribeTableResponse{}, nil
}

func (f *fakeCatalogClient) InsertRows(
	ctx context.Context,
	in *enginepb.InsertRowsRequest,
	_ ...grpc.CallOption,
) (*enginepb.InsertRowsResponse, error) {
	f.lastInsertRows = in
	if f.insertRowsFn != nil {
		return f.insertRowsFn(ctx, in)
	}
	return &enginepb.InsertRowsResponse{}, nil
}

func (f *fakeCatalogClient) ListRows(
	ctx context.Context,
	in *enginepb.ListRowsRequest,
	_ ...grpc.CallOption,
) (*enginepb.ListRowsResponse, error) {
	f.lastListRows = in
	if f.listRowsFn != nil {
		return f.listRowsFn(ctx, in)
	}
	return &enginepb.ListRowsResponse{}, nil
}
