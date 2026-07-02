package seedfile

import (
	"context"
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
	"github.com/vantaboard/bigquery-emulator/gateway/seed"
	"google.golang.org/grpc"
)

// insertRowsCaptureClient is a minimal enginepb.CatalogClient stub
// that records InsertRows requests so seedfile tests can assert on
// the proto wire shape after YAML decode + Apply.
type insertRowsCaptureClient struct {
	lastInsertRows *enginepb.InsertRowsRequest
}

func (c *insertRowsCaptureClient) RegisterDataset(
	_ context.Context,
	_ *enginepb.RegisterDatasetRequest,
	_ ...grpc.CallOption,
) (*enginepb.RegisterDatasetResponse, error) {
	return &enginepb.RegisterDatasetResponse{}, nil
}

func (c *insertRowsCaptureClient) DropDataset(
	_ context.Context,
	_ *enginepb.DropDatasetRequest,
	_ ...grpc.CallOption,
) (*enginepb.DropDatasetResponse, error) {
	return &enginepb.DropDatasetResponse{}, nil
}

func (c *insertRowsCaptureClient) UndeleteDataset(
	_ context.Context,
	_ *enginepb.UndeleteDatasetRequest,
	_ ...grpc.CallOption,
) (*enginepb.UndeleteDatasetResponse, error) {
	return &enginepb.UndeleteDatasetResponse{}, nil
}

func (c *insertRowsCaptureClient) ListDatasets(
	_ context.Context,
	_ *enginepb.ListDatasetsRequest,
	_ ...grpc.CallOption,
) (*enginepb.ListDatasetsResponse, error) {
	return &enginepb.ListDatasetsResponse{}, nil
}

func (c *insertRowsCaptureClient) RegisterTable(
	_ context.Context,
	_ *enginepb.RegisterTableRequest,
	_ ...grpc.CallOption,
) (*enginepb.RegisterTableResponse, error) {
	return &enginepb.RegisterTableResponse{}, nil
}

func (c *insertRowsCaptureClient) DropTable(
	_ context.Context,
	_ *enginepb.DropTableRequest,
	_ ...grpc.CallOption,
) (*enginepb.DropTableResponse, error) {
	return &enginepb.DropTableResponse{}, nil
}

func (c *insertRowsCaptureClient) ListTables(
	_ context.Context,
	_ *enginepb.ListTablesRequest,
	_ ...grpc.CallOption,
) (*enginepb.ListTablesResponse, error) {
	return &enginepb.ListTablesResponse{}, nil
}

func (c *insertRowsCaptureClient) DescribeTable(
	_ context.Context,
	_ *enginepb.DescribeTableRequest,
	_ ...grpc.CallOption,
) (*enginepb.DescribeTableResponse, error) {
	return &enginepb.DescribeTableResponse{}, nil
}

func (c *insertRowsCaptureClient) InsertRows(
	_ context.Context,
	in *enginepb.InsertRowsRequest,
	_ ...grpc.CallOption,
) (*enginepb.InsertRowsResponse, error) {
	c.lastInsertRows = in
	return &enginepb.InsertRowsResponse{}, nil
}

func (c *insertRowsCaptureClient) ListRows(
	_ context.Context,
	_ *enginepb.ListRowsRequest,
	_ ...grpc.CallOption,
) (*enginepb.ListRowsResponse, error) {
	return &enginepb.ListRowsResponse{}, nil
}

func (c *insertRowsCaptureClient) ListRoutines(
	_ context.Context,
	_ *enginepb.ListRoutinesRequest,
	_ ...grpc.CallOption,
) (*enginepb.ListRoutinesResponse, error) {
	return &enginepb.ListRoutinesResponse{}, nil
}

func (c *insertRowsCaptureClient) GetRoutine(
	_ context.Context,
	_ *enginepb.GetRoutineRequest,
	_ ...grpc.CallOption,
) (*enginepb.GetRoutineResponse, error) {
	return &enginepb.GetRoutineResponse{}, nil
}

func (c *insertRowsCaptureClient) UpsertRoutine(
	_ context.Context,
	_ *enginepb.UpsertRoutineRequest,
	_ ...grpc.CallOption,
) (*enginepb.UpsertRoutineResponse, error) {
	return &enginepb.UpsertRoutineResponse{}, nil
}

func (c *insertRowsCaptureClient) DeleteRoutine(
	_ context.Context,
	_ *enginepb.DeleteRoutineRequest,
	_ ...grpc.CallOption,
) (*enginepb.DeleteRoutineResponse, error) {
	return &enginepb.DeleteRoutineResponse{}, nil
}

func (c *insertRowsCaptureClient) UpsertRowAccessPolicy(
	_ context.Context,
	_ *enginepb.UpsertRowAccessPolicyRequest,
	_ ...grpc.CallOption,
) (*enginepb.UpsertRowAccessPolicyResponse, error) {
	return &enginepb.UpsertRowAccessPolicyResponse{}, nil
}

func (c *insertRowsCaptureClient) DeleteRowAccessPolicy(
	_ context.Context,
	_ *enginepb.DeleteRowAccessPolicyRequest,
	_ ...grpc.CallOption,
) (*enginepb.DeleteRowAccessPolicyResponse, error) {
	return &enginepb.DeleteRowAccessPolicyResponse{}, nil
}

func (c *insertRowsCaptureClient) ListRowAccessPolicies(
	_ context.Context,
	_ *enginepb.ListRowAccessPoliciesRequest,
	_ ...grpc.CallOption,
) (*enginepb.ListRowAccessPoliciesResponse, error) {
	return &enginepb.ListRowAccessPoliciesResponse{}, nil
}

func (c *insertRowsCaptureClient) SetColumnGovernance(
	_ context.Context,
	_ *enginepb.SetColumnGovernanceRequest,
	_ ...grpc.CallOption,
) (*enginepb.SetColumnGovernanceResponse, error) {
	return &enginepb.SetColumnGovernanceResponse{}, nil
}

const repeatedStructYAML = `
project_id: dev
datasets:
  - id: ds
    tables:
      - id: t
        schema:
          - name: structarr
            type: STRUCT
            mode: REPEATED
            fields:
              - {name: key, type: STRING}
              - {name: value, type: JSON}
        rows:
          - structarr:
              - key: profile
                value: '{"age": 10}'
`

// TestDecode_RepeatedStructRow confirms YAML for a REPEATED STRUCT
// column decodes to []any of map[string]any before Apply runs.
func TestDecode_RepeatedStructRow(t *testing.T) {
	f, err := Decode([]byte(repeatedStructYAML), "repeated-struct.yaml")
	if err != nil {
		t.Fatalf("Decode: %v", err)
	}
	rows := f.Datasets[0].Tables[0].Rows
	if len(rows) != 1 {
		t.Fatalf("len(rows)=%d, want 1", len(rows))
	}
	arr, ok := rows[0]["structarr"].([]any)
	if !ok {
		t.Fatalf("structarr is %T, want []any", rows[0]["structarr"])
	}
	if len(arr) != 1 {
		t.Fatalf("len(structarr)=%d, want 1", len(arr))
	}
	el, ok := arr[0].(map[string]any)
	if !ok {
		t.Fatalf("structarr[0] is %T, want map[string]any", arr[0])
	}
	if got := el["key"]; got != "profile" {
		t.Errorf("key=%v, want profile", got)
	}
	if got := el["value"]; got != `{"age": 10}` {
		t.Errorf("value=%v, want {\"age\": 10}", got)
	}
}

// TestApply_RepeatedStructJSONSubfield is the end-to-end YAML→Apply
// regression for REPEATED STRUCT columns whose subfields include
// JSON. Without schema-aware name mapping the key/value slots swap
// and JSON parsing fails on the string column.
func TestApply_RepeatedStructJSONSubfield(t *testing.T) {
	f, err := Decode([]byte(repeatedStructYAML), "repeated-struct.yaml")
	if err != nil {
		t.Fatalf("Decode: %v", err)
	}
	catalog := &insertRowsCaptureClient{}
	app := seed.NewCatalogApplier(catalog)
	if err := Apply(context.Background(), f, app, seed.Defaults{}); err != nil {
		t.Fatalf("Apply: %v", err)
	}
	if catalog.lastInsertRows == nil {
		t.Fatal("InsertRows not called")
	}
	got := catalog.lastInsertRows.GetRows()
	if len(got) != 1 {
		t.Fatalf("len(rows)=%d, want 1", len(got))
	}
	arr := got[0].GetCells()[0].GetArray()
	if arr == nil || len(arr.GetElements()) != 1 {
		t.Fatalf("structarr cell: %+v", got[0].GetCells()[0])
	}
	st := arr.GetElements()[0].GetStructValue()
	if st == nil || len(st.GetFields()) != 2 {
		t.Fatalf("struct element: %+v", arr.GetElements()[0])
	}
	if got := st.GetFields()[0].GetStringValue(); got != "profile" {
		t.Errorf("key=%q, want profile", got)
	}
	if got := st.GetFields()[1].GetStringValue(); got != `{"age": 10}` {
		t.Errorf("value=%q, want {\"age\": 10}", got)
	}
}
