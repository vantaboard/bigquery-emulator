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
