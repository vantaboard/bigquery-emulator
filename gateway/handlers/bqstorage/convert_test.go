package bqstorage

import (
	"testing"

	"cloud.google.com/go/bigquery/storage/apiv1/storagepb"
	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
)

func TestEngineTableSchemaToPublicKeepsNestedFields(t *testing.T) {
	t.Parallel()
	in := &enginepb.TableSchema{Fields: []*enginepb.FieldSchema{
		{Name: "properties", Type: "RECORD", Mode: "NULLABLE", Fields: []*enginepb.FieldSchema{
			{Name: "article", Type: "RECORD", Mode: "NULLABLE", Fields: []*enginepb.FieldSchema{
				{Name: "title", Type: "STRING", Mode: "NULLABLE"},
			}},
			{Name: "score", Type: "FLOAT64", Mode: "NULLABLE"},
		}},
	}}

	got := engineTableSchemaToPublic(in).GetFields()

	if len(got) != 1 || got[0].GetType() != storagepb.TableFieldSchema_STRUCT {
		t.Fatalf("top-level fields = %v, want one STRUCT", got)
	}
	properties := got[0].GetFields()
	if len(properties) != 2 || properties[0].GetName() != "article" || properties[1].GetName() != "score" {
		t.Fatalf("properties fields = %v, want article and score", properties)
	}
	article := properties[0].GetFields()
	if len(article) != 1 || article[0].GetName() != "title" || article[0].GetType() != storagepb.TableFieldSchema_STRING {
		t.Fatalf("article fields = %v, want title STRING", article)
	}
}
