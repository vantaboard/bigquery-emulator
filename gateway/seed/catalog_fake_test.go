package seed

import (
	"context"

	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
	"google.golang.org/grpc"
)

// fakeCatalogClient is a lightweight stub of enginepb.CatalogClient
// for the seed package's unit tests. The shape mirrors the one in
// gateway/handlers/catalog_fake_test.go so a reader who knows that
// file can read this one without context-switching, but the two are
// intentionally separate to keep test-only types from leaking across
// package boundaries.
type fakeCatalogClient struct {
	registerDatasetFn func(context.Context, *enginepb.RegisterDatasetRequest) (*enginepb.RegisterDatasetResponse, error)
	registerTableFn   func(context.Context, *enginepb.RegisterTableRequest) (*enginepb.RegisterTableResponse, error)
	insertRowsFn      func(context.Context, *enginepb.InsertRowsRequest) (*enginepb.InsertRowsResponse, error)

	lastRegisterDataset *enginepb.RegisterDatasetRequest
	lastRegisterTable   *enginepb.RegisterTableRequest
	lastInsertRows      *enginepb.InsertRowsRequest
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
	_ context.Context,
	_ *enginepb.DropDatasetRequest,
	_ ...grpc.CallOption,
) (*enginepb.DropDatasetResponse, error) {
	return &enginepb.DropDatasetResponse{}, nil
}

func (f *fakeCatalogClient) UndeleteDataset(
	_ context.Context,
	_ *enginepb.UndeleteDatasetRequest,
	_ ...grpc.CallOption,
) (*enginepb.UndeleteDatasetResponse, error) {
	return &enginepb.UndeleteDatasetResponse{}, nil
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
	_ context.Context,
	_ *enginepb.DropTableRequest,
	_ ...grpc.CallOption,
) (*enginepb.DropTableResponse, error) {
	return &enginepb.DropTableResponse{}, nil
}

func (f *fakeCatalogClient) DescribeTable(
	_ context.Context,
	_ *enginepb.DescribeTableRequest,
	_ ...grpc.CallOption,
) (*enginepb.DescribeTableResponse, error) {
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
	_ context.Context,
	_ *enginepb.ListRowsRequest,
	_ ...grpc.CallOption,
) (*enginepb.ListRowsResponse, error) {
	return &enginepb.ListRowsResponse{}, nil
}

func (f *fakeCatalogClient) ListDatasets(
	_ context.Context,
	_ *enginepb.ListDatasetsRequest,
	_ ...grpc.CallOption,
) (*enginepb.ListDatasetsResponse, error) {
	return &enginepb.ListDatasetsResponse{}, nil
}

func (f *fakeCatalogClient) ListTables(
	_ context.Context,
	_ *enginepb.ListTablesRequest,
	_ ...grpc.CallOption,
) (*enginepb.ListTablesResponse, error) {
	return &enginepb.ListTablesResponse{}, nil
}

func (f *fakeCatalogClient) ListRoutines(
	_ context.Context,
	_ *enginepb.ListRoutinesRequest,
	_ ...grpc.CallOption,
) (*enginepb.ListRoutinesResponse, error) {
	return &enginepb.ListRoutinesResponse{}, nil
}

func (f *fakeCatalogClient) GetRoutine(
	_ context.Context,
	_ *enginepb.GetRoutineRequest,
	_ ...grpc.CallOption,
) (*enginepb.GetRoutineResponse, error) {
	return &enginepb.GetRoutineResponse{}, nil
}

func (f *fakeCatalogClient) UpsertRoutine(
	_ context.Context,
	_ *enginepb.UpsertRoutineRequest,
	_ ...grpc.CallOption,
) (*enginepb.UpsertRoutineResponse, error) {
	return &enginepb.UpsertRoutineResponse{}, nil
}

func (f *fakeCatalogClient) DeleteRoutine(
	_ context.Context,
	_ *enginepb.DeleteRoutineRequest,
	_ ...grpc.CallOption,
) (*enginepb.DeleteRoutineResponse, error) {
	return &enginepb.DeleteRoutineResponse{}, nil
}

func (f *fakeCatalogClient) UpsertRowAccessPolicy(
	_ context.Context,
	_ *enginepb.UpsertRowAccessPolicyRequest,
	_ ...grpc.CallOption,
) (*enginepb.UpsertRowAccessPolicyResponse, error) {
	return &enginepb.UpsertRowAccessPolicyResponse{}, nil
}

func (f *fakeCatalogClient) DeleteRowAccessPolicy(
	_ context.Context,
	_ *enginepb.DeleteRowAccessPolicyRequest,
	_ ...grpc.CallOption,
) (*enginepb.DeleteRowAccessPolicyResponse, error) {
	return &enginepb.DeleteRowAccessPolicyResponse{}, nil
}

func (f *fakeCatalogClient) ListRowAccessPolicies(
	_ context.Context,
	_ *enginepb.ListRowAccessPoliciesRequest,
	_ ...grpc.CallOption,
) (*enginepb.ListRowAccessPoliciesResponse, error) {
	return &enginepb.ListRowAccessPoliciesResponse{}, nil
}

func (f *fakeCatalogClient) SetColumnGovernance(
	_ context.Context,
	_ *enginepb.SetColumnGovernanceRequest,
	_ ...grpc.CallOption,
) (*enginepb.SetColumnGovernanceResponse, error) {
	return &enginepb.SetColumnGovernanceResponse{}, nil
}
