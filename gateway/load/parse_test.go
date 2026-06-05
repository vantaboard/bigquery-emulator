package load

import (
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
)

func TestParseCSVWithSchemaAndSkipHeader(t *testing.T) {
	t.Parallel()
	schema := &bqtypes.TableSchema{Fields: []bqtypes.TableFieldSchema{
		{Name: "name", Type: fieldTypeString},
		{Name: "post_abbr", Type: fieldTypeString},
	}}
	data := []byte("name,post_abbr\nAlabama,AL\nAlaska,AK\n")
	got, err := ParseSource("CSV", data, schema, 1, false)
	if err != nil {
		t.Fatalf("ParseSource: %v", err)
	}
	if len(got.Rows) != 2 {
		t.Fatalf("rows = %d, want 2", len(got.Rows))
	}
	if got.Rows[0]["name"] != "Alabama" || got.Rows[0]["post_abbr"] != "AL" {
		t.Fatalf("row0 = %#v", got.Rows[0])
	}
}

func TestParseNDJSONAutodetect(t *testing.T) {
	t.Parallel()
	data := []byte(`{"id":"1","name":"a"}
{"id":"2","name":"b"}
`)
	got, err := ParseSource("NEWLINE_DELIMITED_JSON", data, nil, 0, true)
	if err != nil {
		t.Fatalf("ParseSource: %v", err)
	}
	if len(got.Schema.Fields) != 2 || len(got.Rows) != 2 {
		t.Fatalf("schema=%d fields rows=%d", len(got.Schema.Fields), len(got.Rows))
	}
}
