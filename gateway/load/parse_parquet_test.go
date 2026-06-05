package load

import (
	"bytes"
	"os"
	"testing"

	"github.com/parquet-go/parquet-go"
	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
)

const (
	testStateName = "Alabama"
	testStateCode = "AL"
	testColName   = "Name"
)

func TestParseParquetRoundTrip(t *testing.T) {
	t.Parallel()
	type row struct {
		Name string `parquet:"name"`
		Code string `parquet:"code"`
	}
	buf := new(bytes.Buffer)
	w := parquet.NewGenericWriter[row](buf)
	if _, err := w.Write([]row{{Name: testStateName, Code: testStateCode}}); err != nil {
		t.Fatal(err)
	}
	if err := w.Close(); err != nil {
		t.Fatal(err)
	}

	got, err := ParseSource("PARQUET", buf.Bytes(), nil, 0, true)
	if err != nil {
		t.Fatalf("ParseSource: %v", err)
	}
	if len(got.Rows) != 1 {
		t.Fatalf("rows = %d, want 1", len(got.Rows))
	}
	if got.Rows[0]["name"] != testStateName || got.Rows[0]["code"] != testStateCode {
		t.Fatalf("row = %#v", got.Rows[0])
	}
}

func TestParseAvroUSStatesFixture(t *testing.T) {
	t.Parallel()
	data, err := os.ReadFile("testdata/us-states.avro")
	if err != nil {
		t.Fatal(err)
	}
	got, err := ParseSource("AVRO", data, nil, 0, true)
	if err != nil {
		t.Fatalf("ParseSource: %v", err)
	}
	if len(got.Rows) != 50 {
		t.Fatalf("rows = %d, want 50", len(got.Rows))
	}
	if got.Rows[0]["name"] != testStateName || got.Rows[0]["post_abbr"] != testStateCode {
		t.Fatalf("row = %#v", got.Rows[0])
	}
	if len(got.Schema.Fields) < 2 {
		t.Fatalf("schema = %#v", got.Schema.Fields)
	}
}

func TestParseORCUSStatesFixture(t *testing.T) {
	t.Parallel()
	data, err := os.ReadFile("testdata/us-states.orc")
	if err != nil {
		t.Fatal(err)
	}
	got, err := ParseSource("ORC", data, nil, 0, true)
	if err != nil {
		t.Fatalf("ParseSource: %v", err)
	}
	if len(got.Rows) != 50 {
		t.Fatalf("rows = %d, want 50", len(got.Rows))
	}
	if got.Rows[0]["name"] != testStateName || got.Rows[0]["post_abbr"] != testStateCode {
		t.Fatalf("row = %#v", got.Rows[0])
	}
}

func TestParseAvroInvalidData(t *testing.T) {
	t.Parallel()
	_, err := ParseSource("AVRO", []byte("not-avro"), nil, 0, true)
	if err == nil {
		t.Fatal("expected AVRO parse error")
	}
}

func TestParseORCInvalidData(t *testing.T) {
	t.Parallel()
	_, err := ParseSource("ORC", []byte("not-orc"), nil, 0, true)
	if err == nil {
		t.Fatal("expected ORC parse error")
	}
}

func TestMergeSchemasAllowFieldAddition(t *testing.T) {
	t.Parallel()
	existing := &bqtypes.TableSchema{Fields: []bqtypes.TableFieldSchema{
		{Name: testColName, Type: fieldTypeString},
		{Name: "Age", Type: fieldTypeInteger},
	}}
	load := &bqtypes.TableSchema{Fields: []bqtypes.TableFieldSchema{
		{Name: testColName, Type: fieldTypeString},
		{Name: "Age", Type: fieldTypeInteger},
		{Name: "IsMagic", Type: fieldTypeBoolean},
	}}
	merged, changed := mergeSchemas(existing, load, []string{schemaUpdateAllowFieldAddition})
	if !changed {
		t.Fatal("expected schema change")
	}
	if len(merged.Fields) != 3 || merged.Fields[2].Name != "IsMagic" {
		t.Fatalf("merged = %#v", merged.Fields)
	}
}

func TestMergeSchemasAllowFieldRelaxation(t *testing.T) {
	t.Parallel()
	existing := &bqtypes.TableSchema{Fields: []bqtypes.TableFieldSchema{
		{Name: "Name", Type: fieldTypeString, Mode: fieldModeRequired},
	}}
	merged, changed := mergeSchemas(existing, nil, []string{schemaUpdateAllowFieldRelaxation})
	if !changed {
		t.Fatal("expected schema change")
	}
	if merged.Fields[0].Mode != "" {
		t.Fatalf("mode = %q, want NULLABLE/empty", merged.Fields[0].Mode)
	}
}
