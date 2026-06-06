package bqstorage

import (
	"bytes"
	"fmt"
	"strconv"
	"strings"

	"cloud.google.com/go/bigquery/storage/apiv1/storagepb"
	"github.com/apache/arrow/go/v18/arrow"
	"github.com/apache/arrow/go/v18/arrow/array"
	"github.com/apache/arrow/go/v18/arrow/ipc"
	"github.com/apache/arrow/go/v18/arrow/memory"
	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
)

func arrowSchemaFromEngine(schema *enginepb.TableSchema) (*arrow.Schema, error) {
	if schema == nil || len(schema.GetFields()) == 0 {
		return arrow.NewSchema(nil, nil), nil
	}
	fields := make([]arrow.Field, 0, len(schema.GetFields()))
	for _, f := range schema.GetFields() {
		dt, err := arrowTypeForBQ(f.GetType())
		if err != nil {
			return nil, err
		}
		fields = append(fields, arrow.Field{
			Name:     f.GetName(),
			Type:     dt,
			Nullable: strings.ToUpper(f.GetMode()) != "REQUIRED",
		})
	}
	return arrow.NewSchema(fields, nil), nil
}

func arrowTypeForBQ(t string) (arrow.DataType, error) {
	switch strings.ToUpper(strings.TrimSpace(t)) {
	case "INT64":
		return arrow.PrimitiveTypes.Int64, nil
	case "FLOAT64":
		return arrow.PrimitiveTypes.Float64, nil
	case "BOOL":
		return arrow.FixedWidthTypes.Boolean, nil
	case "STRING", "JSON", "GEOGRAPHY", "DATE", "TIME", "DATETIME", "TIMESTAMP",
		"BYTES", "NUMERIC", "BIGNUMERIC", "STRUCT", "RECORD":
		return arrow.BinaryTypes.String, nil
	default:
		return arrow.BinaryTypes.String, nil
	}
}

func serializeArrowSchema(schema *enginepb.TableSchema) (*storagepb.ArrowSchema, error) {
	as, err := arrowSchemaFromEngine(schema)
	if err != nil {
		return nil, err
	}
	var buf bytes.Buffer
	w := ipc.NewWriter(&buf, ipc.WithSchema(as))
	if err := w.Close(); err != nil {
		return nil, err
	}
	return &storagepb.ArrowSchema{SerializedSchema: buf.Bytes()}, nil
}

func rowsToArrowBatch(
	schema *enginepb.TableSchema,
	rows []*enginepb.DataRow,
) (*storagepb.ArrowRecordBatch, error) {
	as, err := arrowSchemaFromEngine(schema)
	if err != nil {
		return nil, err
	}
	mem := memory.NewGoAllocator()
	b := array.NewRecordBuilder(mem, as)
	defer b.Release()

	for colIdx, field := range schema.GetFields() {
		if err := appendColumnValues(b.Field(colIdx), field.GetType(), rows, colIdx); err != nil {
			return nil, err
		}
	}

	rec := b.NewRecord()
	defer rec.Release()

	var buf bytes.Buffer
	w := ipc.NewWriter(&buf, ipc.WithSchema(as))
	if err := w.Write(rec); err != nil {
		return nil, err
	}
	if err := w.Close(); err != nil {
		return nil, err
	}
	return &storagepb.ArrowRecordBatch{
		SerializedRecordBatch: buf.Bytes(),
		RowCount:              int64(len(rows)),
	}, nil
}

func appendColumnValues(
	builder array.Builder,
	bqType string,
	rows []*enginepb.DataRow,
	colIdx int,
) error {
	switch strings.ToUpper(strings.TrimSpace(bqType)) {
	case "INT64":
		ib := builder.(*array.Int64Builder)
		for _, row := range rows {
			if colIdx >= len(row.GetCells()) || row.GetCells()[colIdx].GetNullValue() {
				ib.AppendNull()
				continue
			}
			v, err := strconv.ParseInt(row.GetCells()[colIdx].GetStringValue(), 10, 64)
			if err != nil {
				return fmt.Errorf("column %d INT64 parse: %w", colIdx, err)
			}
			ib.Append(v)
		}
	case "FLOAT64":
		fb := builder.(*array.Float64Builder)
		for _, row := range rows {
			if colIdx >= len(row.GetCells()) || row.GetCells()[colIdx].GetNullValue() {
				fb.AppendNull()
				continue
			}
			v, err := strconv.ParseFloat(row.GetCells()[colIdx].GetStringValue(), 64)
			if err != nil {
				return fmt.Errorf("column %d FLOAT64 parse: %w", colIdx, err)
			}
			fb.Append(v)
		}
	case "BOOL":
		bb := builder.(*array.BooleanBuilder)
		for _, row := range rows {
			if colIdx >= len(row.GetCells()) || row.GetCells()[colIdx].GetNullValue() {
				bb.AppendNull()
				continue
			}
			v, err := strconv.ParseBool(row.GetCells()[colIdx].GetStringValue())
			if err != nil {
				return fmt.Errorf("column %d BOOL parse: %w", colIdx, err)
			}
			bb.Append(v)
		}
	default:
		sb := builder.(*array.StringBuilder)
		for _, row := range rows {
			if colIdx >= len(row.GetCells()) || row.GetCells()[colIdx].GetNullValue() {
				sb.AppendNull()
				continue
			}
			sb.Append(row.GetCells()[colIdx].GetStringValue())
		}
	}
	return nil
}
