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
	if got.Rows[0]["name"] != testStateName || got.Rows[0]["post_abbr"] != testStateCode {
		t.Fatalf("row0 = %#v", got.Rows[0])
	}
}

func TestParseCSVAutodetectUsesHeaderRow(t *testing.T) {
	t.Parallel()
	data := []byte("full_name,age\nPhred Phlyntstone,32\nWylma Phlyntstone,29\n")
	got, err := ParseSource("CSV", data, nil, 1, true)
	if err != nil {
		t.Fatalf("ParseSource: %v", err)
	}
	if got.Schema.Fields[0].Name != "full_name" || got.Schema.Fields[1].Name != "age" {
		t.Fatalf("schema = %#v", got.Schema.Fields)
	}
	if got.Schema.Fields[1].Type != fieldTypeInteger {
		t.Fatalf("age type = %q, want %q", got.Schema.Fields[1].Type, fieldTypeInteger)
	}
	if age, ok := got.Rows[1]["age"].(int); !ok || age != 29 {
		t.Fatalf("row1 age = %#v", got.Rows[1]["age"])
	}
}

func TestParseDatastoreBackupJSONEntities(t *testing.T) {
	t.Parallel()
	data := []byte(
		`{"entities":[{"properties":{"name":{"stringValue":"Alabama"},"post_abbr":{"stringValue":"AL"},"year":{"integerValue":"2019"}}}]}`,
	)
	got, err := ParseSource("DATASTORE_BACKUP", data, nil, 0, true)
	if err != nil {
		t.Fatalf("ParseSource: %v", err)
	}
	if len(got.Rows) != 1 {
		t.Fatalf("rows = %d, want 1", len(got.Rows))
	}
	if got.Rows[0]["name"] != "Alabama" {
		t.Fatalf("name = %#v", got.Rows[0]["name"])
	}
}

func TestParseCSVTimestampDatetime(t *testing.T) {
	t.Parallel()
	schema := &bqtypes.TableSchema{Fields: []bqtypes.TableFieldSchema{
		{Name: "release_date", Type: "TIMESTAMP"},
		{Name: "dvd_release", Type: "DATETIME"},
	}}
	data := []byte("release_date,dvd_release\n2012-03-04T05:06:07+00:00,2012-03-04T05:06:07\n")
	got, err := ParseSource("CSV", data, schema, 1, false)
	if err != nil {
		t.Fatalf("ParseSource: %v", err)
	}
	if len(got.Rows) != 1 {
		t.Fatalf("rows = %d, want 1", len(got.Rows))
	}
	if _, ok := got.Rows[0]["release_date"].(string); !ok {
		t.Fatalf("release_date = %#v, want RFC3339 string", got.Rows[0]["release_date"])
	}
	if _, ok := got.Rows[0]["dvd_release"].(string); !ok {
		t.Fatalf("dvd_release = %#v, want datetime string", got.Rows[0]["dvd_release"])
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
