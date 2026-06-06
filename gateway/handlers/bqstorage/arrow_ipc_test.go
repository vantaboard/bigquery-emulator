package bqstorage

import (
	"bytes"
	"testing"

	"github.com/apache/arrow/go/v18/arrow"
	"github.com/apache/arrow/go/v18/arrow/array"
	"github.com/apache/arrow/go/v18/arrow/ipc"
	"github.com/apache/arrow/go/v18/arrow/memory"
)

func TestSerializeArrowIPCRecordBatchExcludesSchema(t *testing.T) {
	mem := memory.NewGoAllocator()
	schema := arrow.NewSchema([]arrow.Field{
		{Name: "id", Type: arrow.PrimitiveTypes.Int64},
	}, nil)
	b := array.NewRecordBuilder(mem, schema)
	defer b.Release()
	b.Field(0).(*array.Int64Builder).AppendValues([]int64{1, 2}, nil)
	rec := b.NewRecord()
	defer rec.Release()

	batchBytes, err := serializeArrowIPCRecordBatch(schema, rec)
	if err != nil {
		t.Fatalf("serializeArrowIPCRecordBatch: %v", err)
	}

	msgRdr := ipc.NewMessageReader(bytes.NewReader(batchBytes))
	defer msgRdr.Release()
	msg, err := msgRdr.Message()
	if err != nil {
		t.Fatalf("Message: %v", err)
	}
	defer msg.Release()
	if msg.Type() != ipc.MessageRecordBatch {
		t.Fatalf("message type = %v, want RecordBatch", msg.Type())
	}
}
