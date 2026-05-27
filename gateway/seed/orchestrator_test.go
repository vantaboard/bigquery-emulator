package seed

import (
	"context"
	"errors"
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
)

// fakeReader is the in-memory ProductionReader used by orchestrator
// tests. Populating its maps lets each test stand a synthetic
// production project up without a network round-trip.
type fakeReader struct {
	datasets map[string][]ProductionDataset // keyed by project
	tables   map[string][]ProductionTable   // keyed by "proj.ds"
	schemas  map[string]*enginepb.TableSchema
	rows     map[string][]map[string]any

	listDatasetsErr error
	listTablesErr   error
	describeErr     error
	readErr         error
}

func (f *fakeReader) ListDatasets(_ context.Context, project string) ([]ProductionDataset, error) {
	if f.listDatasetsErr != nil {
		return nil, f.listDatasetsErr
	}
	return f.datasets[project], nil
}

func (f *fakeReader) ListTables(_ context.Context, project, dataset string) ([]ProductionTable, error) {
	if f.listTablesErr != nil {
		return nil, f.listTablesErr
	}
	return f.tables[project+"."+dataset], nil
}

func (f *fakeReader) DescribeTable(_ context.Context, project, dataset, table string) (*enginepb.TableSchema, error) {
	if f.describeErr != nil {
		return nil, f.describeErr
	}
	return f.schemas[project+"."+dataset+"."+table], nil
}

func (f *fakeReader) ReadRows(_ context.Context, project, dataset, table string, _ int64) ([]map[string]any, error) {
	if f.readErr != nil {
		return nil, f.readErr
	}
	return f.rows[project+"."+dataset+"."+table], nil
}

// fakeApplier captures every call so tests can assert on the
// resulting state. It also lets a test inject failures on specific
// operations.
type fakeApplier struct {
	ensuredDatasets []string
	ensuredTables   []string
	inserted        map[string][]map[string]any

	ensureDatasetErr error
	ensureTableErr   error
	insertErr        error
}

func (f *fakeApplier) EnsureDataset(_ context.Context, projectID, datasetID, _ string) (bool, error) {
	if f.ensureDatasetErr != nil {
		return false, f.ensureDatasetErr
	}
	f.ensuredDatasets = append(f.ensuredDatasets, projectID+"."+datasetID)
	return true, nil
}

func (f *fakeApplier) EnsureTable(_ context.Context, ref TableRef, _ *enginepb.TableSchema) (bool, error) {
	if f.ensureTableErr != nil {
		return false, f.ensureTableErr
	}
	f.ensuredTables = append(f.ensuredTables, ref.ProjectID+"."+ref.DatasetID+"."+ref.TableID)
	return true, nil
}

func (f *fakeApplier) InsertRows(
	_ context.Context,
	ref TableRef,
	_ *enginepb.TableSchema,
	rows []map[string]any,
) (int, error) {
	if f.insertErr != nil {
		return 0, f.insertErr
	}
	if f.inserted == nil {
		f.inserted = make(map[string][]map[string]any)
	}
	key := ref.ProjectID + "." + ref.DatasetID + "." + ref.TableID
	f.inserted[key] = append(f.inserted[key], rows...)
	return len(rows), nil
}

// pinNow swaps nowRFC3339 for a deterministic stamp so result
// timestamps are easy to assert.
func pinNow(t *testing.T) {
	orig := nowRFC3339
	nowRFC3339 = func() string { return "2026-01-02T03:04:05Z" }
	t.Cleanup(func() { nowRFC3339 = orig })
}

// TestOrchestrator_TableScope_HappyPath walks the smallest scope:
// one source table -> one destination table with rows.
func TestOrchestrator_TableScope_HappyPath(t *testing.T) {
	pinNow(t)
	reader := &fakeReader{
		datasets: map[string][]ProductionDataset{
			testProjectProd: {{ProjectID: testProjectProd, DatasetID: testDatasetDS, Location: "US"}},
		},
		tables: map[string][]ProductionTable{
			testProjectProd + "." + testDatasetDS: {
				{ProjectID: testProjectProd, DatasetID: testDatasetDS, TableID: "people", Type: tableKindTable},
			},
		},
		schemas: map[string]*enginepb.TableSchema{
			tableRefPeople: {Fields: []*enginepb.FieldSchema{
				{Name: "id", Type: bqTypeInt64},
				{Name: colName, Type: bqTypeString},
			}},
		},
		rows: map[string][]map[string]any{
			tableRefPeople: {
				{"id": 1, colName: rowValueAda},
				{"id": 2, colName: "bob"},
			},
		},
	}
	applier := &fakeApplier{}
	orch := NewOrchestrator(reader, applier, Defaults{DatasetLocation: "EU"})

	res, err := orch.Run(context.Background(), SeedRequest{
		Source: SeedEndpointRef{Project: testProjectProd, Dataset: testDatasetDS, Table: "people"},
	})
	if err != nil {
		t.Fatalf("Run: %v", err)
	}
	if res.TablesCreated != 1 {
		t.Errorf("TablesCreated=%d, want 1", res.TablesCreated)
	}
	if res.RowsCopied != 2 {
		t.Errorf("RowsCopied=%d, want 2", res.RowsCopied)
	}
	if len(res.ResourceErrors) != 0 {
		t.Errorf("ResourceErrors=%+v", res.ResourceErrors)
	}
	if got := applier.ensuredTables; len(got) != 1 || got[0] != tableRefPeople {
		t.Errorf("ensuredTables=%v", got)
	}
	if got := applier.inserted[tableRefPeople]; len(got) != 2 {
		t.Errorf("inserted rows=%d, want 2", len(got))
	}
}

// TestOrchestrator_DatasetScope_CopiesEveryTable walks dataset
// scope: the orchestrator fans out across every table in the source
// dataset.
func TestOrchestrator_DatasetScope_CopiesEveryTable(t *testing.T) {
	pinNow(t)
	reader := &fakeReader{
		datasets: map[string][]ProductionDataset{
			testProjectProd: {{ProjectID: testProjectProd, DatasetID: testDatasetDS, Location: "US"}},
		},
		tables: map[string][]ProductionTable{
			testProjectProd + "." + testDatasetDS: {
				{ProjectID: testProjectProd, DatasetID: testDatasetDS, TableID: "a", Type: tableKindTable},
				{ProjectID: testProjectProd, DatasetID: testDatasetDS, TableID: "b", Type: tableKindTable},
			},
		},
		schemas: map[string]*enginepb.TableSchema{
			tableRefProdDSA: {Fields: []*enginepb.FieldSchema{{Name: "id", Type: bqTypeInt64}}},
			"prod.ds.b":     {Fields: []*enginepb.FieldSchema{{Name: "id", Type: bqTypeInt64}}},
		},
		rows: map[string][]map[string]any{
			tableRefProdDSA: {{"id": 1}},
			"prod.ds.b":     {{"id": 2}, {"id": 3}},
		},
	}
	applier := &fakeApplier{}
	orch := NewOrchestrator(reader, applier, Defaults{})

	res, err := orch.Run(context.Background(), SeedRequest{
		Source: SeedEndpointRef{Project: testProjectProd, Dataset: testDatasetDS},
	})
	if err != nil {
		t.Fatalf("Run: %v", err)
	}
	if res.TablesCreated != 2 {
		t.Errorf("TablesCreated=%d, want 2", res.TablesCreated)
	}
	if res.RowsCopied != 3 {
		t.Errorf("RowsCopied=%d, want 3", res.RowsCopied)
	}
}

// TestOrchestrator_UnsupportedTypesAreSkipped pins the documented
// graceful-degradation behavior for resources the engine doesn't
// persist yet: VIEW / MATERIALIZED_VIEW / MODEL / EXTERNAL show up
// as ResourceErrors with kind=unsupported and the rest of the seed
// proceeds.
func TestOrchestrator_UnsupportedTypesAreSkipped(t *testing.T) {
	pinNow(t)
	reader := &fakeReader{
		datasets: map[string][]ProductionDataset{
			testProjectProd: {{ProjectID: testProjectProd, DatasetID: testDatasetDS}},
		},
		tables: map[string][]ProductionTable{
			testProjectProd + "." + testDatasetDS: {
				{ProjectID: testProjectProd, DatasetID: testDatasetDS, TableID: "phys", Type: tableKindTable},
				{ProjectID: testProjectProd, DatasetID: testDatasetDS, TableID: "view1", Type: "VIEW"},
				{ProjectID: testProjectProd, DatasetID: testDatasetDS, TableID: "ext1", Type: "EXTERNAL"},
			},
		},
		schemas: map[string]*enginepb.TableSchema{
			"prod.ds.phys": {Fields: []*enginepb.FieldSchema{{Name: "id", Type: bqTypeInt64}}},
		},
		rows: map[string][]map[string]any{
			"prod.ds.phys": {{"id": 1}},
		},
	}
	applier := &fakeApplier{}
	orch := NewOrchestrator(reader, applier, Defaults{})

	res, err := orch.Run(context.Background(), SeedRequest{
		Source: SeedEndpointRef{Project: testProjectProd, Dataset: testDatasetDS},
	})
	if err != nil {
		t.Fatalf("Run: %v", err)
	}
	if res.TablesCreated != 1 {
		t.Errorf("TablesCreated=%d, want 1", res.TablesCreated)
	}
	if len(res.ResourceErrors) != 2 {
		t.Fatalf("ResourceErrors=%+v, want 2", res.ResourceErrors)
	}
	for _, re := range res.ResourceErrors {
		if re.Kind != resourceKindUnsupported {
			t.Errorf("kind=%q, want unsupported", re.Kind)
		}
	}
}

// TestOrchestrator_DestinationRemap covers the case where the
// operator copies prod.ds.tbl into emu.ds2.tbl2 -- the orchestrator
// must write to the new triple, not the source triple.
func TestOrchestrator_DestinationRemap(t *testing.T) {
	pinNow(t)
	reader := &fakeReader{
		tables: map[string][]ProductionTable{
			testProjectProd + "." + testDatasetDS: {
				{ProjectID: testProjectProd, DatasetID: testDatasetDS, TableID: "tbl", Type: tableKindTable},
			},
		},
		schemas: map[string]*enginepb.TableSchema{
			"prod.ds.tbl": {Fields: []*enginepb.FieldSchema{{Name: "id", Type: bqTypeInt64}}},
		},
		rows: map[string][]map[string]any{
			"prod.ds.tbl": {{"id": 1}},
		},
	}
	applier := &fakeApplier{}
	orch := NewOrchestrator(reader, applier, Defaults{})

	res, err := orch.Run(context.Background(), SeedRequest{
		Source: SeedEndpointRef{Project: testProjectProd, Dataset: testDatasetDS, Table: "tbl"},
		Destination: &SeedDestinationRef{
			Project: "emu", Dataset: "ds2", Table: "tbl2",
		},
	})
	if err != nil {
		t.Fatalf("Run: %v", err)
	}
	if res.TablesCreated != 1 {
		t.Errorf("TablesCreated=%d, want 1", res.TablesCreated)
	}
	if got := applier.ensuredTables; len(got) != 1 || got[0] != "emu.ds2.tbl2" {
		t.Errorf("ensuredTables=%v, want [emu.ds2.tbl2]", got)
	}
	if got := applier.inserted["emu.ds2.tbl2"]; len(got) != 1 {
		t.Errorf("inserted rows for emu.ds2.tbl2=%d, want 1", len(got))
	}
}

// TestOrchestrator_PerResourceFailureFolds verifies that an
// EnsureTable failure surfaces as a ResourceError without aborting
// the rest of the seed.
func TestOrchestrator_PerResourceFailureFolds(t *testing.T) {
	pinNow(t)
	reader := &fakeReader{
		tables: map[string][]ProductionTable{
			testProjectProd + "." + testDatasetDS: {
				{ProjectID: testProjectProd, DatasetID: testDatasetDS, TableID: "a", Type: tableKindTable},
			},
		},
		schemas: map[string]*enginepb.TableSchema{
			tableRefProdDSA: {Fields: []*enginepb.FieldSchema{{Name: "id", Type: bqTypeInt64}}},
		},
	}
	applier := &fakeApplier{ensureTableErr: errors.New("write failed")}
	orch := NewOrchestrator(reader, applier, Defaults{})
	res, err := orch.Run(context.Background(), SeedRequest{
		Source: SeedEndpointRef{Project: testProjectProd, Dataset: testDatasetDS},
	})
	if err != nil {
		t.Fatalf("Run: %v", err)
	}
	if res.TablesCreated != 0 {
		t.Errorf("TablesCreated=%d, want 0", res.TablesCreated)
	}
	if len(res.ResourceErrors) != 1 || res.ResourceErrors[0].Kind != resourceKindWrite {
		t.Errorf("ResourceErrors=%+v", res.ResourceErrors)
	}
}

// TestOrchestrator_InvalidRequestSurfaces ensures Run double-checks
// the request rather than blindly trusting the caller; downstream
// "passed validation" callers (the HTTP handler) already validate
// but a programmatic call should still get a clean error.
func TestOrchestrator_InvalidRequestSurfaces(t *testing.T) {
	orch := NewOrchestrator(&fakeReader{}, &fakeApplier{}, Defaults{})
	_, err := orch.Run(context.Background(), SeedRequest{})
	if err == nil {
		t.Fatal("Run accepted invalid request")
	}
	if !errors.Is(err, ErrInvalidRequest) {
		t.Errorf("error not ErrInvalidRequest: %v", err)
	}
}

// TestNewOrchestrator_PanicsOnNilReader documents the fatal-misuse
// case so callers don't have to debug a deep nil-deref crash.
func TestNewOrchestrator_PanicsOnNilReader(t *testing.T) {
	defer func() {
		if r := recover(); r == nil {
			t.Error("NewOrchestrator(nil, applier, _) did not panic")
		}
	}()
	NewOrchestrator(nil, &fakeApplier{}, Defaults{})
}

// TestNewProductionReader_DefaultBuildIsUnsupported pins the
// default-build behavior so anyone wiring the seed API by mistake
// gets a clear "build with -tags=seed_production_live" error
// instead of a confusing nil deref.
func TestNewProductionReader_DefaultBuildIsUnsupported(t *testing.T) {
	_, err := NewProductionReader(context.Background(), "p", LookupEnvOrEmpty)
	if err == nil {
		t.Fatal("default build returned a non-nil reader; should be ErrProductionUnsupported")
	}
	if err.Error() != ErrProductionUnsupported.Error() {
		t.Errorf("error=%q, want %q", err, ErrProductionUnsupported)
	}
}
