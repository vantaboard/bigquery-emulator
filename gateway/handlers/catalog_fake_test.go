package handlers

import (
	"context"

	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
	"google.golang.org/grpc"
	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"
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
	listDatasetsFn    func(context.Context, *enginepb.ListDatasetsRequest) (*enginepb.ListDatasetsResponse, error)
	registerTableFn   func(context.Context, *enginepb.RegisterTableRequest) (*enginepb.RegisterTableResponse, error)
	dropTableFn       func(context.Context, *enginepb.DropTableRequest) (*enginepb.DropTableResponse, error)
	listTablesFn      func(context.Context, *enginepb.ListTablesRequest) (*enginepb.ListTablesResponse, error)
	describeTableFn   func(context.Context, *enginepb.DescribeTableRequest) (*enginepb.DescribeTableResponse, error)
	insertRowsFn      func(context.Context, *enginepb.InsertRowsRequest) (*enginepb.InsertRowsResponse, error)
	listRowsFn        func(context.Context, *enginepb.ListRowsRequest) (*enginepb.ListRowsResponse, error)

	// registeredDatasets mirrors RegisterDataset/DropDataset when no
	// per-test callback overrides those methods, so ListDatasets and
	// catalogDatasetExists behave like the live engine in unit tests.
	registeredDatasets []*enginepb.DatasetRef

	// lastRegisterDataset captures the most recent request so tests
	// can assert on the values forwarded over the wire without having
	// to set a callback on every test.
	lastRegisterDataset *enginepb.RegisterDatasetRequest
	lastDropDataset     *enginepb.DropDatasetRequest
	lastListDatasets    *enginepb.ListDatasetsRequest
	lastRegisterTable   *enginepb.RegisterTableRequest
	registeredTableIDs  []string
	lastDropTable       *enginepb.DropTableRequest
	lastListTables      *enginepb.ListTablesRequest
	lastDescribeTable   *enginepb.DescribeTableRequest
	lastInsertRows      *enginepb.InsertRowsRequest
	lastListRows        *enginepb.ListRowsRequest

	registeredRoutines []*enginepb.RoutineDescriptor
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
	ref := in.GetDataset()
	if ref != nil {
		f.registeredDatasets = append(f.registeredDatasets, ref)
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
	ref := in.GetDataset()
	if ref == nil {
		return &enginepb.DropDatasetResponse{}, nil
	}
	out := f.registeredDatasets[:0]
	for _, ds := range f.registeredDatasets {
		if ds.GetProjectId() != ref.GetProjectId() ||
			ds.GetDatasetId() != ref.GetDatasetId() {
			out = append(out, ds)
		}
	}
	f.registeredDatasets = out
	return &enginepb.DropDatasetResponse{}, nil
}

func (f *fakeCatalogClient) RegisterTable(
	ctx context.Context,
	in *enginepb.RegisterTableRequest,
	_ ...grpc.CallOption,
) (*enginepb.RegisterTableResponse, error) {
	f.lastRegisterTable = in
	if in.GetTable() != nil {
		f.registeredTableIDs = append(f.registeredTableIDs, in.GetTable().GetTableId())
	}
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

func (f *fakeCatalogClient) ListDatasets(
	ctx context.Context,
	in *enginepb.ListDatasetsRequest,
	_ ...grpc.CallOption,
) (*enginepb.ListDatasetsResponse, error) {
	f.lastListDatasets = in
	if f.listDatasetsFn != nil {
		return f.listDatasetsFn(ctx, in)
	}
	var out []*enginepb.DatasetRef
	for _, ref := range f.registeredDatasets {
		if ref.GetProjectId() == in.GetProjectId() {
			out = append(out, ref)
		}
	}
	return &enginepb.ListDatasetsResponse{Datasets: out}, nil
}

func (f *fakeCatalogClient) ListTables(
	ctx context.Context,
	in *enginepb.ListTablesRequest,
	_ ...grpc.CallOption,
) (*enginepb.ListTablesResponse, error) {
	f.lastListTables = in
	if f.listTablesFn != nil {
		return f.listTablesFn(ctx, in)
	}
	return &enginepb.ListTablesResponse{}, nil
}

func (f *fakeCatalogClient) ListRoutines(
	_ context.Context,
	in *enginepb.ListRoutinesRequest,
	_ ...grpc.CallOption,
) (*enginepb.ListRoutinesResponse, error) {
	ds := in.GetDataset()
	if ds == nil {
		return &enginepb.ListRoutinesResponse{}, nil
	}
	var out []*enginepb.RoutineDescriptor
	for _, desc := range f.registeredRoutines {
		ref := desc.GetRoutine()
		if ref == nil {
			continue
		}
		if ref.GetProjectId() == ds.GetProjectId() &&
			ref.GetDatasetId() == ds.GetDatasetId() {
			out = append(out, desc)
		}
	}
	return &enginepb.ListRoutinesResponse{Routines: out}, nil
}

func (f *fakeCatalogClient) GetRoutine(
	_ context.Context,
	in *enginepb.GetRoutineRequest,
	_ ...grpc.CallOption,
) (*enginepb.GetRoutineResponse, error) {
	ref := in.GetRoutine()
	if ref == nil {
		return nil, status.Error(codes.NotFound, "routine not found")
	}
	for _, desc := range f.registeredRoutines {
		got := desc.GetRoutine()
		if got == nil {
			continue
		}
		if got.GetProjectId() == ref.GetProjectId() &&
			got.GetDatasetId() == ref.GetDatasetId() &&
			got.GetRoutineId() == ref.GetRoutineId() {
			return &enginepb.GetRoutineResponse{Routine: desc}, nil
		}
	}
	return nil, status.Error(codes.NotFound, "routine not found")
}

func (f *fakeCatalogClient) UpsertRoutine(
	_ context.Context,
	in *enginepb.UpsertRoutineRequest,
	_ ...grpc.CallOption,
) (*enginepb.UpsertRoutineResponse, error) {
	desc := in.GetRoutine()
	if desc == nil || desc.GetRoutine() == nil {
		return &enginepb.UpsertRoutineResponse{}, nil
	}
	ref := desc.GetRoutine()
	for i, existing := range f.registeredRoutines {
		got := existing.GetRoutine()
		if got == nil {
			continue
		}
		if got.GetProjectId() == ref.GetProjectId() &&
			got.GetDatasetId() == ref.GetDatasetId() &&
			got.GetRoutineId() == ref.GetRoutineId() {
			f.registeredRoutines[i] = desc
			return &enginepb.UpsertRoutineResponse{}, nil
		}
	}
	f.registeredRoutines = append(f.registeredRoutines, desc)
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
