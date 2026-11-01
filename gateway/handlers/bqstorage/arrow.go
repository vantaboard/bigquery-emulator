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

func arrowSchemaFromEngine(schema *enginepb.TableSchema) *arrow.Schema {
	if schema == nil || len(schema.GetFields()) == 0 {
		return arrow.NewSchema(nil, nil)
	}
	return arrow.NewSchema(arrowFields(schema.GetFields()), nil)
}

func arrowFields(fields []*enginepb.FieldSchema) []arrow.Field {
	out := make([]arrow.Field, 0, len(fields))
	for _, f := range fields {
		out = append(out, arrowField(f))
	}
	return out
}

func arrowField(f *enginepb.FieldSchema) arrow.Field {
	if element, ok := arrayElement(f); ok {
		return arrow.Field{Name: f.GetName(), Type: arrow.ListOf(arrowValueType(element))}
	}
	return arrow.Field{
		Name:     f.GetName(),
		Type:     arrowValueType(f),
		Nullable: strings.ToUpper(f.GetMode()) != bqModeRequired,
	}
}

func arrowValueType(f *enginepb.FieldSchema) arrow.DataType {
	if isStruct(f) {
		return arrow.StructOf(arrowFields(f.GetFields())...)
	}
	return arrowTypeForBQ(f.GetType())
}

func arrowTypeForBQ(t string) arrow.DataType {
	switch strings.ToUpper(strings.TrimSpace(t)) {
	case bqTypeINT64, bqTypeINTEGER:
		return arrow.PrimitiveTypes.Int64
	case bqTypeFLOAT64, bqTypeFLOAT:
		return arrow.PrimitiveTypes.Float64
	case bqTypeBOOL:
		return arrow.FixedWidthTypes.Boolean
	case bqTypeTIMESTAMP:
		return &arrow.TimestampType{Unit: arrow.Microsecond, TimeZone: "UTC"}
	case bqTypeDATETIME:
		return &arrow.TimestampType{Unit: arrow.Microsecond}
	default:
		return arrow.BinaryTypes.String
	}
}

func serializeArrowSchema(schema *enginepb.TableSchema) (*storagepb.ArrowSchema, error) {
	as := arrowSchemaFromEngine(schema)
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
	as := arrowSchemaFromEngine(schema)
	mem := memory.NewGoAllocator()
	b := array.NewRecordBuilder(mem, as)
	defer b.Release()

	for colIdx, field := range schema.GetFields() {
		for _, row := range rows {
			var cell *enginepb.Cell
			if colIdx < len(row.GetCells()) {
				cell = row.GetCells()[colIdx]
			}
			if err := appendArrowCell(b.Field(colIdx), field, cell); err != nil {
				return nil, fmt.Errorf("column %q: %w", field.GetName(), err)
			}
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

func appendArrowCell(builder array.Builder, field *enginepb.FieldSchema, cell *enginepb.Cell) error {
	if element, ok := arrayElement(field); ok {
		lb := builder.(*array.ListBuilder)
		lb.Append(true)
		for i, e := range cell.GetArray().GetElements() {
			if e == nil || e.GetNullValue() {
				return fmt.Errorf("array element %d is null", i)
			}
			if err := appendArrowValue(lb.ValueBuilder(), element, e); err != nil {
				return fmt.Errorf("array element %d: %w", i, err)
			}
		}
		return nil
	}
	if cell == nil || cell.GetNullValue() {
		builder.AppendNull()
		return nil
	}
	return appendArrowValue(builder, field, cell)
}

func appendArrowValue(builder array.Builder, field *enginepb.FieldSchema, cell *enginepb.Cell) error {
	raw := cell.GetStringValue()
	switch b := builder.(type) {
	case *array.StructBuilder:
		b.Append(true)
		cells := cell.GetStructValue().GetFields()
		for i, child := range field.GetFields() {
			var c *enginepb.Cell
			if i < len(cells) {
				c = cells[i]
			}
			if err := appendArrowCell(b.FieldBuilder(i), child, c); err != nil {
				return fmt.Errorf("field %q: %w", child.GetName(), err)
			}
		}
	case *array.Int64Builder:
		v, err := strconv.ParseInt(raw, 10, 64)
		if err != nil {
			return fmt.Errorf("INT64 parse: %w", err)
		}
		b.Append(v)
	case *array.Float64Builder:
		v, err := strconv.ParseFloat(raw, 64)
		if err != nil {
			return fmt.Errorf("FLOAT64 parse: %w", err)
		}
		b.Append(v)
	case *array.BooleanBuilder:
		v, err := strconv.ParseBool(raw)
		if err != nil {
			return fmt.Errorf("BOOL parse: %w", err)
		}
		b.Append(v)
	case *array.TimestampBuilder:
		parse := timestampCellToMicros
		if strings.ToUpper(strings.TrimSpace(field.GetType())) == bqTypeDATETIME {
			parse = datetimeCellToMicros
		}
		micros, err := parse(raw)
		if err != nil {
			return fmt.Errorf("%s parse: %w", field.GetType(), err)
		}
		b.Append(arrow.Timestamp(micros))
	case *array.StringBuilder:
		b.Append(raw)
	default:
		return fmt.Errorf("unsupported Arrow builder %T", builder)
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
