package bqstorage

import (
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
)

func TestPublicAppendResponseFromEngineCarriesUpdatedSchema(t *testing.T) {
	t.Parallel()
	out := publicAppendResponseFromEngine("stream", &enginepb.AppendRowsResponse{
		Response: &enginepb.AppendRowsResponse_AppendResult_{
			AppendResult: &enginepb.AppendRowsResponse_AppendResult{Offset: 1},
		},
		UpdatedSchema: &enginepb.TableSchema{
			Fields: []*enginepb.FieldSchema{{Name: "id", Type: "INT64"}, {Name: "nickname", Type: "STRING"}},
		},
	})
	if got := out.GetAppendResult().GetOffset().GetValue(); got != 1 {
		t.Fatalf("offset = %d, want 1", got)
	}
	fields := out.GetUpdatedSchema().GetFields()
	if len(fields) != 2 || fields[1].GetName() != "nickname" {
		t.Fatalf("updated_schema = %v, want id and nickname", out.GetUpdatedSchema())
	}
}

func TestPublicAppendResponseFromEngineOmitsUnchangedSchema(t *testing.T) {
	t.Parallel()
	out := publicAppendResponseFromEngine("stream", &enginepb.AppendRowsResponse{
		Response: &enginepb.AppendRowsResponse_AppendResult_{
			AppendResult: &enginepb.AppendRowsResponse_AppendResult{},
		},
	})
	if out.GetUpdatedSchema() != nil {
		t.Fatalf("updated_schema = %v, want nil", out.GetUpdatedSchema())
	}
}
