package bqstorage

import (
	"bytes"
	"strings"
	"testing"

	"github.com/apache/arrow/go/v18/arrow/array"
	"github.com/apache/arrow/go/v18/arrow/ipc"
)

func TestRowsToArrowBatchNested(t *testing.T) {
	schema := nestedTestSchema()
	arrowSchema, err := serializeArrowSchema(schema)
	if err != nil {
		t.Fatalf("serializeArrowSchema: %v", err)
	}
	batch, err := rowsToArrowBatch(schema, nestedTestRows())
	if err != nil {
		t.Fatalf("rowsToArrowBatch: %v", err)
	}

	stream := append(append([]byte{}, arrowSchema.GetSerializedSchema()...), batch.GetSerializedRecordBatch()...)
	rdr, err := ipc.NewReader(bytes.NewReader(stream))
	if err != nil {
		t.Fatalf("ipc.NewReader: %v", err)
	}
	defer rdr.Release()
	if !rdr.Next() {
		t.Fatalf("no record batch: %v", rdr.Err())
	}

	var got bytes.Buffer
	if err := array.RecordToJSON(rdr.Record(), &got); err != nil {
		t.Fatalf("RecordToJSON: %v", err)
	}
	want := strings.Join([]string{
		`{"events":[{"kind":"view"}],"id":1,"ids":[3,5],"tags":["a","b"],` +
			`"user":{"address":{"city":"Paris"},"login":"ada"}}`,
		`{"events":[],"id":2,"ids":[],"tags":[],"user":null}`,
		"",
	}, "\n")
	if got.String() != want {
		t.Errorf("rows =\n%s\nwant\n%s", got.String(), want)
	}
}
