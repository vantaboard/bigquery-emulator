package handlers

import (
	"bytes"
	"context"
	"encoding/json"
	"net/http"
	"net/http/httptest"
	"slices"
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
	"github.com/vantaboard/bigquery-emulator/gateway/external"
	"github.com/vantaboard/bigquery-emulator/gateway/jobs"
	"google.golang.org/grpc"
)

func TestExternalTableInsertPersistsMetadata(t *testing.T) {
	gcs := startFakeGCSCSVServer(t, "name,post_abbr\nWashington,WA\n")
	defer gcs.Close()
	t.Setenv("STORAGE_EMULATOR_HOST", gcs.Listener.Addr().String())

	fake := &fakeCatalogClient{}
	store := NewMetadataStore()
	deps := Dependencies{Catalog: fake, Metadata: store}
	body := `{
	  "tableReference": {"tableId": "us_states"},
	  "schema": {"fields": [
	    {"name": "name", "type": "STRING"},
	    {"name": "post_abbr", "type": "STRING"}
	  ]},
	  "externalDataConfiguration": {
	    "sourceFormat": "CSV",
	    "sourceUris": ["gs://bkt/us-states.csv"],
	    "csvOptions": {"skipLeadingRows": "1"}
	  }
	}`
	req := httptest.NewRequest(http.MethodPost,
		"/bigquery/v2/projects/dev/datasets/ds/tables", bytes.NewBufferString(body))
	req.SetPathValue("projectId", "dev")
	req.SetPathValue("datasetId", "ds")
	w := httptest.NewRecorder()
	TableInsert(deps)(w, req)
	if w.Code != http.StatusOK {
		t.Fatalf("status = %d body = %s", w.Code, w.Body.String())
	}
	var got bqtypes.Table
	if err := json.Unmarshal(w.Body.Bytes(), &got); err != nil {
		t.Fatal(err)
	}
	if got.Type != externalTableType {
		t.Fatalf("type = %q, want EXTERNAL", got.Type)
	}
	if got.ExternalDataConfiguration == nil {
		t.Fatal("externalDataConfiguration missing from insert response")
	}
	cached, ok := store.GetTable("dev", "ds", "us_states")
	if !ok || cached.ExternalDataConfiguration == nil {
		t.Fatal("external config not cached in MetadataStore")
	}
	if fake.lastInsertRows == nil {
		t.Fatal("expected rows inserted into catalog")
	}
}

func TestExternalTableGetRoundTripsConfiguration(t *testing.T) {
	fake := &fakeCatalogClient{
		describeTableFn: func(
			_ context.Context,
			_ *enginepb.DescribeTableRequest,
		) (*enginepb.DescribeTableResponse, error) {
			return &enginepb.DescribeTableResponse{
				Schema: &enginepb.TableSchema{Fields: []*enginepb.FieldSchema{
					{Name: testColumnName, Type: sqlTypeSTRING},
					{Name: "post_abbr", Type: sqlTypeSTRING},
				}},
			}, nil
		},
		listRowsFn: func(
			_ context.Context,
			_ *enginepb.ListRowsRequest,
		) (*enginepb.ListRowsResponse, error) {
			return &enginepb.ListRowsResponse{TotalRows: 1}, nil
		},
	}
	store := NewMetadataStore()
	store.PutTable("dev", "ds", "us_states", bqtypes.Table{
		Type: externalTableType,
		ExternalDataConfiguration: &bqtypes.ExternalDataConfiguration{
			SourceFormat: "CSV",
			SourceURIs:   []string{"gs://bkt/f.csv"},
		},
	})
	deps := Dependencies{Catalog: fake, Metadata: store}
	req := httptest.NewRequest(http.MethodGet,
		"/bigquery/v2/projects/dev/datasets/ds/tables/us_states", nil)
	req.SetPathValue("projectId", "dev")
	req.SetPathValue("datasetId", "ds")
	req.SetPathValue("tableId", "us_states")
	w := httptest.NewRecorder()
	TableGet(deps)(w, req)
	if w.Code != http.StatusOK {
		t.Fatalf("status = %d", w.Code)
	}
	var got bqtypes.Table
	if err := json.Unmarshal(w.Body.Bytes(), &got); err != nil {
		t.Fatal(err)
	}
	if got.ExternalDataConfiguration == nil || got.ExternalDataConfiguration.SourceFormat != "CSV" {
		t.Fatalf("GET external config = %#v", got.ExternalDataConfiguration)
	}
}

func TestExternalTableInsertGoogleSheets501(t *testing.T) {
	fake := &fakeCatalogClient{}
	deps := Dependencies{Catalog: fake, Metadata: NewMetadataStore()}
	body := `{
	  "tableReference": {"tableId": "sheet"},
	  "externalDataConfiguration": {
	    "sourceFormat": "GOOGLE_SHEETS",
	    "sourceUris": ["https://docs.google.com/spreadsheets/d/x/edit"]
	  }
	}`
	req := httptest.NewRequest(http.MethodPost,
		"/bigquery/v2/projects/dev/datasets/ds/tables", bytes.NewBufferString(body))
	req.SetPathValue("projectId", "dev")
	req.SetPathValue("datasetId", "ds")
	w := httptest.NewRecorder()
	TableInsert(deps)(w, req)
	if w.Code != http.StatusNotImplemented {
		t.Fatalf("status = %d, want 501 body=%s", w.Code, w.Body.String())
	}
}

func TestExternalQueryTableDefinitionsMaterializes(t *testing.T) {
	gcs := startFakeGCSCSVServer(t, "name,post_abbr\nWashington,WA\nWyoming,WY\n")
	defer gcs.Close()
	t.Setenv("STORAGE_EMULATOR_HOST", gcs.Listener.Addr().String())

	fake := &fakeCatalogClient{}
	queryFake := &fakeQueryClient{
		executeQueryFn: func(
			_ context.Context,
			in *enginepb.QueryRequest,
		) (grpc.ServerStreamingClient[enginepb.QueryResultRow], error) {
			if in.GetDefaultDatasetId() != external.TempDatasetID {
				t.Fatalf("default dataset = %q, want %q", in.GetDefaultDatasetId(), external.TempDatasetID)
			}
			return &fakeQueryResultStream{msgs: []*enginepb.QueryResultRow{
				{Schema: &enginepb.TableSchema{Fields: []*enginepb.FieldSchema{
					{Name: testColumnName, Type: sqlTypeSTRING},
				}}},
				{Cells: []*enginepb.Cell{{Value: &enginepb.Cell_StringValue{StringValue: "Washington"}}}},
			}}, nil
		},
	}
	deps := Dependencies{Catalog: fake, Query: queryFake, Jobs: jobs.NewRegistry()}
	body := `{
	  "query": "SELECT name FROM us_states WHERE name LIKE 'W%'",
	  "tableDefinitions": {
	    "us_states": {
	      "sourceFormat": "CSV",
	      "sourceUris": ["gs://bkt/states.csv"],
	      "csvOptions": {"skipLeadingRows": "1"},
	      "schema": {"fields": [
	        {"name": "name", "type": "STRING"},
	        {"name": "post_abbr", "type": "STRING"}
	      ]}
	    }
	  }
	}`
	req := httptest.NewRequest(http.MethodPost,
		"/bigquery/v2/projects/dev/queries", bytes.NewBufferString(body))
	req.SetPathValue("projectId", "dev")
	w := httptest.NewRecorder()
	QueryRun(deps)(w, req)
	if w.Code != http.StatusOK {
		t.Fatalf("status = %d body=%s", w.Code, w.Body.String())
	}
	found := slices.Contains(fake.registeredTableIDs, "us_states")
	if !found {
		t.Fatalf("expected ephemeral external table us_states registered, got %v",
			fake.registeredTableIDs)
	}
}

func startFakeGCSCSVServer(t *testing.T, csv string) *httptest.Server {
	t.Helper()
	return httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, _ *http.Request) {
		w.Header().Set("Content-Type", "text/csv")
		if _, err := w.Write([]byte(csv)); err != nil {
			t.Errorf("write response: %v", err)
		}
	}))
}
