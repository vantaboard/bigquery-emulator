package handlers

import (
	"context"
	"encoding/json"
	"net/http"
	"net/http/httptest"
	"strconv"
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
	"google.golang.org/grpc"
)

// TestTableMetadataRequirePartitionFilterRoundTrip pins the table-level
// partition-filter flag the python update_table_require_partition_filter
// sample sets via PATCH + GET.
func TestTableMetadataRequirePartitionFilterRoundTrip(t *testing.T) {
	runTableMetadataRoundTrip(t, `{"requirePartitionFilter":true}`, func(t *testing.T, got bqtypes.Table) {
		if got.RequirePartitionFilter == nil || !*got.RequirePartitionFilter {
			t.Errorf("requirePartitionFilter = %v, want true", got.RequirePartitionFilter)
		}
	})
}

// TestTableGetIncludesNumRows asserts tables.get emits numRows for
// engine-backed tables even when the catalog has zero rows stored.
func TestTableGetIncludesNumRows(t *testing.T) {
	fake := &fakeCatalogClient{
		describeTableFn: func(_ context.Context, _ *enginepb.DescribeTableRequest) (*enginepb.DescribeTableResponse, error) {
			return &enginepb.DescribeTableResponse{
				Schema: &enginepb.TableSchema{
					Fields: []*enginepb.FieldSchema{{Name: "id", Type: sqlTypeINT64}},
				},
			}, nil
		},
	}
	deps := Dependencies{Catalog: fake, Metadata: NewMetadataStore()}
	get := newTableReq(http.MethodGet, testTableID, "")
	rec := httptest.NewRecorder()
	TableGet(deps)(rec, get)
	if rec.Code != http.StatusOK {
		t.Fatalf("get: status=%d body=%s", rec.Code, rec.Body.String())
	}
	var got bqtypes.Table
	if err := json.NewDecoder(rec.Body).Decode(&got); err != nil {
		t.Fatalf("decode: %v", err)
	}
	if got.NumRows != "0" {
		t.Errorf("numRows = %q, want %q", got.NumRows, "0")
	}
	if got.Schema == nil || len(got.Schema.Fields) != 1 {
		t.Errorf("schema = %+v, want one field", got.Schema)
	}
}

// TestTableInsertViewRoundTrips asserts tables.insert with a view
// definition stores type=VIEW and view.query for a follow-up GET.
func TestTableInsertViewRoundTrips(t *testing.T) {
	const viewSQL = "SELECT 1 AS x"
	fakeCatalog := &fakeCatalogClient{}
	fakeQuery := &fakeQueryClient{
		dryRunFn: func(_ context.Context, _ *enginepb.QueryRequest) (*enginepb.DryRunResponse, error) {
			return &enginepb.DryRunResponse{
				Schema: &enginepb.TableSchema{
					Fields: []*enginepb.FieldSchema{{Name: "x", Type: sqlTypeINT64}},
				},
			}, nil
		},
		// The view is registered through the engine's CREATE OR REPLACE
		// VIEW path; the runner drains an (empty) result stream.
		executeQueryFn: func(_ context.Context, _ *enginepb.QueryRequest) (grpc.ServerStreamingClient[enginepb.QueryResultRow], error) {
			return &fakeQueryResultStream{}, nil
		},
	}
	store := NewMetadataStore()
	deps := Dependencies{Catalog: fakeCatalog, Query: fakeQuery, Metadata: store}

	body := `{
		"tableReference":{"tableId":"` + testTableID + `"},
		"view":{"query":` + strconv.Quote(viewSQL) + `}
	}`
	insert := newTableReq(http.MethodPost, "", body)
	rec := httptest.NewRecorder()
	TableInsert(deps)(rec, insert)
	if rec.Code != http.StatusOK {
		t.Fatalf("insert: status=%d body=%s", rec.Code, rec.Body.String())
	}

	get := newTableReq(http.MethodGet, testTableID, "")
	rec = httptest.NewRecorder()
	TableGet(deps)(rec, get)
	if rec.Code != http.StatusOK {
		t.Fatalf("get: status=%d body=%s", rec.Code, rec.Body.String())
	}
	var got bqtypes.Table
	if err := json.NewDecoder(rec.Body).Decode(&got); err != nil {
		t.Fatalf("decode: %v", err)
	}
	if got.Type != viewTableType {
		t.Errorf("type = %q, want %q", got.Type, viewTableType)
	}
	if got.View == nil || got.View.Query != viewSQL {
		t.Errorf("view = %+v, want query %q", got.View, viewSQL)
	}
}

func TestTableMetadataDefaultRoundingModeRoundTrip(t *testing.T) {
	runTableMetadataRoundTrip(
		t,
		`{"defaultRoundingMode":"ROUND_HALF_AWAY_FROM_ZERO"}`,
		func(t *testing.T, got bqtypes.Table) {
			if got.DefaultRoundingMode != "ROUND_HALF_AWAY_FROM_ZERO" {
				t.Errorf("defaultRoundingMode = %q, want ROUND_HALF_AWAY_FROM_ZERO", got.DefaultRoundingMode)
			}
		},
	)
}

func TestTableMetadataCaseInsensitiveRoundTrip(t *testing.T) {
	runTableMetadataRoundTrip(t, `{"caseInsensitive":true}`, func(t *testing.T, got bqtypes.Table) {
		if got.CaseInsensitive == nil || !*got.CaseInsensitive {
			t.Errorf("caseInsensitive = %v, want true", got.CaseInsensitive)
		}
	})
}

func TestTableMetadataResourceTagsRoundTrip(t *testing.T) {
	runTableMetadataRoundTrip(t,
		`{"resourceTags":{"123456789012/team":"analytics"}}`,
		func(t *testing.T, got bqtypes.Table) {
			if got.ResourceTags["123456789012/team"] != "analytics" {
				t.Errorf("resourceTags = %v", got.ResourceTags)
			}
		},
	)
}

func TestTableMetadataTableConstraintsRoundTrip(t *testing.T) {
	runTableMetadataRoundTrip(t,
		`{"tableConstraints":{"primaryKey":{"columns":["id"]}}}`,
		func(t *testing.T, got bqtypes.Table) {
			if got.TableConstraints == nil || got.TableConstraints.PrimaryKey == nil {
				t.Fatalf("tableConstraints = %+v", got.TableConstraints)
			}
			if len(got.TableConstraints.PrimaryKey.Columns) != 1 || got.TableConstraints.PrimaryKey.Columns[0] != "id" {
				t.Errorf("primaryKey.columns = %v, want [id]", got.TableConstraints.PrimaryKey.Columns)
			}
		},
	)
}

func TestTableGetIncludesStorageStatsStubZeros(t *testing.T) {
	fake := &fakeCatalogClient{
		describeTableFn: func(_ context.Context, _ *enginepb.DescribeTableRequest) (*enginepb.DescribeTableResponse, error) {
			return &enginepb.DescribeTableResponse{}, nil
		},
	}
	deps := Dependencies{Catalog: fake, Metadata: NewMetadataStore()}
	get := newTableReq(http.MethodGet, testTableID, "")
	rec := httptest.NewRecorder()
	TableGet(deps)(rec, get)
	if rec.Code != http.StatusOK {
		t.Fatalf("get: status=%d body=%s", rec.Code, rec.Body.String())
	}
	var got bqtypes.Table
	if err := json.NewDecoder(rec.Body).Decode(&got); err != nil {
		t.Fatalf("decode: %v", err)
	}
	for _, field := range []struct {
		name string
		val  string
	}{
		{"numBytes", got.NumBytes},
		{"numLongTermBytes", got.NumLongTermBytes},
		{"numActiveLogicalBytes", got.NumActiveLogicalBytes},
		{"numTotalLogicalBytes", got.NumTotalLogicalBytes},
		{"numCurrentPhysicalBytes", got.NumCurrentPhysicalBytes},
		{"numPhysicalBytes", got.NumPhysicalBytes},
		{"numActivePhysicalBytes", got.NumActivePhysicalBytes},
		{"numLongTermPhysicalBytes", got.NumLongTermPhysicalBytes},
		{"numTimeTravelPhysicalBytes", got.NumTimeTravelPhysicalBytes},
	} {
		if field.val != "0" {
			t.Errorf("%s = %q, want %q", field.name, field.val, "0")
		}
	}
}

func TestTableInsertViewUseLegacySqlRoundTrip(t *testing.T) {
	const viewSQL = "SELECT 1 AS x"
	fakeCatalog := &fakeCatalogClient{}
	fakeQuery := &fakeQueryClient{
		dryRunFn: func(_ context.Context, _ *enginepb.QueryRequest) (*enginepb.DryRunResponse, error) {
			return &enginepb.DryRunResponse{
				Schema: &enginepb.TableSchema{
					Fields: []*enginepb.FieldSchema{{Name: "x", Type: sqlTypeINT64}},
				},
			}, nil
		},
		executeQueryFn: func(_ context.Context, _ *enginepb.QueryRequest) (grpc.ServerStreamingClient[enginepb.QueryResultRow], error) {
			return &fakeQueryResultStream{}, nil
		},
	}
	store := NewMetadataStore()
	deps := Dependencies{Catalog: fakeCatalog, Query: fakeQuery, Metadata: store}

	body := `{
		"tableReference":{"tableId":"` + testTableID + `"},
		"view":{"query":` + strconv.Quote(viewSQL) + `,"useLegacySql":true}
	}`
	insert := newTableReq(http.MethodPost, "", body)
	rec := httptest.NewRecorder()
	TableInsert(deps)(rec, insert)
	if rec.Code != http.StatusOK {
		t.Fatalf("insert: status=%d body=%s", rec.Code, rec.Body.String())
	}

	get := newTableReq(http.MethodGet, testTableID, "")
	rec = httptest.NewRecorder()
	TableGet(deps)(rec, get)
	if rec.Code != http.StatusOK {
		t.Fatalf("get: status=%d body=%s", rec.Code, rec.Body.String())
	}
	var got bqtypes.Table
	if err := json.NewDecoder(rec.Body).Decode(&got); err != nil {
		t.Fatalf("decode: %v", err)
	}
	if got.View == nil {
		t.Fatal("view missing")
	}
	if !got.View.UseLegacySQL {
		t.Errorf("view.useLegacySql = false, want true")
	}
}
