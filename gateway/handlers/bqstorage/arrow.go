package bqstorage

import (
	"errors"
	"fmt"
	"strconv"
	"strings"
	"time"

	"cloud.google.com/go/bigquery/storage/apiv1/storagepb"
	"github.com/apache/arrow/go/v18/arrow"
	"github.com/apache/arrow/go/v18/arrow/array"
	"github.com/apache/arrow/go/v18/arrow/memory"
	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
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
	case "INT64", "INTEGER":
		return arrow.PrimitiveTypes.Int64, nil
	case "FLOAT64", "FLOAT":
		return arrow.PrimitiveTypes.Float64, nil
	case "BOOL":
		return arrow.FixedWidthTypes.Boolean, nil
	case "TIMESTAMP":
		return &arrow.TimestampType{Unit: arrow.Microsecond, TimeZone: "UTC"}, nil
	case "DATETIME":
		return &arrow.TimestampType{Unit: arrow.Microsecond}, nil
	case "STRING", "JSON", "GEOGRAPHY", "DATE", "TIME",
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
	schemaBytes, err := serializeArrowIPCSchema(as)
	if err != nil {
		return nil, err
	}
	return &storagepb.ArrowSchema{SerializedSchema: schemaBytes}, nil
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

	batchBytes, err := serializeArrowIPCRecordBatch(as, rec)
	if err != nil {
		return nil, err
	}
	return &storagepb.ArrowRecordBatch{
		SerializedRecordBatch: batchBytes,
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
	case "INT64", "INTEGER":
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
	case "FLOAT64", "FLOAT":
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
	case "TIMESTAMP":
		tb := builder.(*array.TimestampBuilder)
		for _, row := range rows {
			if colIdx >= len(row.GetCells()) || row.GetCells()[colIdx].GetNullValue() {
				tb.AppendNull()
				continue
			}
			micros, err := timestampCellToMicros(row.GetCells()[colIdx].GetStringValue())
			if err != nil {
				return fmt.Errorf("column %d TIMESTAMP parse: %w", colIdx, err)
			}
			tb.Append(arrow.Timestamp(micros))
		}
	case "DATETIME":
		tb := builder.(*array.TimestampBuilder)
		for _, row := range rows {
			if colIdx >= len(row.GetCells()) || row.GetCells()[colIdx].GetNullValue() {
				tb.AppendNull()
				continue
			}
			micros, err := datetimeCellToMicros(row.GetCells()[colIdx].GetStringValue())
			if err != nil {
				return fmt.Errorf("column %d DATETIME parse: %w", colIdx, err)
			}
			tb.Append(arrow.Timestamp(micros))
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

func timestampCellToMicros(s string) (int64, error) {
	if strings.TrimSpace(s) == "" {
		return 0, errors.New("empty timestamp")
	}
	microsStr, err := bqtypes.TimestampStringToMicros(s)
	if err != nil {
		return 0, err
	}
	return strconv.ParseInt(microsStr, 10, 64)
}

func datetimeCellToMicros(s string) (int64, error) {
	s = strings.TrimSpace(s)
	if s == "" {
		return 0, errors.New("empty datetime")
	}
	s = strings.Replace(s, "T", " ", 1)
	layouts := []string{
		"2006-01-02 15:04:05.999999",
		"2006-01-02 15:04:05",
	}
	var lastErr error
	for _, layout := range layouts {
		t, err := time.Parse(layout, s)
		if err == nil {
			return t.Unix()*1_000_000 + int64(t.Nanosecond()/1000), nil
		}
		lastErr = err
	}
	return 0, lastErr
}
