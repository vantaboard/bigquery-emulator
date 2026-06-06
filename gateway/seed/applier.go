// Package seed contains the production-side seeding orchestrator plus
// shared types the YAML seed-file loader (gateway/seedfile) reuses
// when it applies declarative data to the engine.
//
// Both code paths ultimately call into the engine's CatalogClient
// over gRPC -- the same surface the REST handlers
// (gateway/handlers/datasets.go, tables.go, tabledata.go) drive --
// so seeded state is indistinguishable from state created via the
// public REST API.
package seed

import (
	"context"
	"encoding/base64"
	"encoding/json"
	"errors"
	"fmt"
	"strconv"
	"strings"

	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
)

// Applier is the narrow surface seeding code drives to mutate the
// emulator's catalog and rows. It is implemented on top of an
// enginepb.CatalogClient by NewCatalogApplier; tests pass a fake
// implementation so the orchestrator and YAML loader can run without
// a live engine.
type Applier interface {
	// EnsureDataset registers (project, dataset) with the engine.
	// Implementations treat an ALREADY_EXISTS response as success
	// so seeding is idempotent across reruns. Created reports
	// whether the call actually changed engine state, so callers
	// can tally a "created vs skipped" counter the way
	// go-googlesql does in its orchestrator metrics.
	EnsureDataset(ctx context.Context, projectID, datasetID, location string) (created bool, err error)

	// EnsureTable registers (project, dataset, table) with the
	// given schema. Same idempotency contract as EnsureDataset:
	// ALREADY_EXISTS surfaces as `created=false, err=nil`. The
	// schema is taken at face value; callers that want to evolve
	// schemas across runs are responsible for dropping and
	// re-registering tables themselves.
	EnsureTable(ctx context.Context, ref TableRef, schema *enginepb.TableSchema) (created bool, err error)

	// InsertRows appends rows to (ref) in a single RPC. Schema is
	// the table's column order so callers can pass a generic
	// map-shaped row and the applier lays cells out positionally.
	// Returns the number of rows inserted on success.
	InsertRows(ctx context.Context, ref TableRef, schema *enginepb.TableSchema, rows []map[string]any) (int, error)
}

// TableRef is the (project, dataset, table) triple the applier API
// passes around. We keep it in this package -- rather than reusing
// enginepb.TableRef directly -- so callers don't need to import the
// generated proto package just to name a destination.
type TableRef struct {
	ProjectID string
	DatasetID string
	TableID   string
}

// catalogApplier is the production Applier implementation backed by
// the gRPC CatalogClient.
type catalogApplier struct {
	client enginepb.CatalogClient
}

// NewCatalogApplier wraps a CatalogClient so it satisfies Applier.
// The returned applier holds no state of its own; passing the same
// CatalogClient to multiple appliers is safe.
func NewCatalogApplier(c enginepb.CatalogClient) Applier {
	return &catalogApplier{client: c}
}

// EnsureDataset wraps Catalog.RegisterDataset with idempotency: an
// ALREADY_EXISTS response is treated as a successful no-op so seed
// reruns don't fail the entire batch.
func (a *catalogApplier) EnsureDataset(ctx context.Context, projectID, datasetID, location string) (bool, error) {
	if a == nil || a.client == nil {
		return false, errors.New("seed: nil CatalogClient; engine subprocess required to ensure dataset")
	}
	_, err := a.client.RegisterDataset(ctx, &enginepb.RegisterDatasetRequest{
		Dataset: &enginepb.DatasetRef{
			ProjectId: projectID,
			DatasetId: datasetID,
		},
		Location: location,
	})
	if err != nil {
		if isAlreadyExists(err) {
			return false, nil
		}
		return false, fmt.Errorf("RegisterDataset %s.%s: %w", projectID, datasetID, err)
	}
	return true, nil
}

// EnsureTable wraps Catalog.RegisterTable with the same idempotency
// contract as EnsureDataset.
func (a *catalogApplier) EnsureTable(ctx context.Context, ref TableRef, schema *enginepb.TableSchema) (bool, error) {
	if a == nil || a.client == nil {
		return false, errors.New("seed: nil CatalogClient; engine subprocess required to ensure table")
	}
	_, err := a.client.RegisterTable(ctx, &enginepb.RegisterTableRequest{
		Table: &enginepb.TableRef{
			ProjectId: ref.ProjectID,
			DatasetId: ref.DatasetID,
			TableId:   ref.TableID,
		},
		Schema: schema,
	})
	if err != nil {
		if isAlreadyExists(err) {
			return false, nil
		}
		return false, fmt.Errorf("RegisterTable %s.%s.%s: %w",
			ref.ProjectID, ref.DatasetID, ref.TableID, err)
	}
	return true, nil
}

// InsertRows lays each map-shaped row out positionally against the
// table's schema before forwarding to Catalog.InsertRows. Missing
// columns become NULL cells so the cell count stays in sync with
// the column count Storage::AppendRows expects (mirrors the same
// rule TableDataInsertAll applies for REST inserts).
func (a *catalogApplier) InsertRows(
	ctx context.Context,
	ref TableRef,
	schema *enginepb.TableSchema,
	rows []map[string]any,
) (int, error) {
	if a == nil || a.client == nil {
		return 0, errors.New("seed: nil CatalogClient; engine subprocess required to insert rows")
	}
	if len(rows) == 0 {
		return 0, nil
	}
	dataRows := make([]*enginepb.DataRow, 0, len(rows))
	for _, row := range rows {
		dataRows = append(dataRows, rowToProto(schema, row))
	}
	_, err := a.client.InsertRows(ctx, &enginepb.InsertRowsRequest{
		Table: &enginepb.TableRef{
			ProjectId: ref.ProjectID,
			DatasetId: ref.DatasetID,
			TableId:   ref.TableID,
		},
		Rows: dataRows,
	})
	if err != nil {
		return 0, fmt.Errorf("InsertRows %s.%s.%s (%d rows): %w",
			ref.ProjectID, ref.DatasetID, ref.TableID, len(rows), err)
	}
	return len(rows), nil
}

// rowToProto lays a map-shaped row out positionally against the
// schema, mirroring jsonRowToProto in gateway/handlers/tabledata.go.
// Pulled into its own helper so both seeding paths (production
// orchestrator and YAML loader) emit the same wire shape.
func rowToProto(schema *enginepb.TableSchema, row map[string]any) *enginepb.DataRow {
	out := &enginepb.DataRow{Cells: make([]*enginepb.Cell, 0, len(schema.GetFields()))}
	for _, f := range schema.GetFields() {
		v, ok := row[f.GetName()]
		if !ok {
			out.Cells = append(out.Cells, nullCell())
			continue
		}
		out.Cells = append(out.Cells, cellFromJSONForField(f, v))
	}
	return out
}

func cellFromJSONForField(f *enginepb.FieldSchema, v any) *enginepb.Cell {
	if f == nil {
		return ValueToCell(v)
	}
	if isStructFieldType(f.GetType()) {
		m, ok := v.(map[string]any)
		if !ok {
			return ValueToCell(v)
		}
		st := &enginepb.Struct{Fields: make([]*enginepb.Cell, 0, len(f.GetFields()))}
		for _, sub := range f.GetFields() {
			subV, ok := m[sub.GetName()]
			if !ok {
				st.Fields = append(st.Fields, nullCell())
				continue
			}
			st.Fields = append(st.Fields, cellFromJSONForField(sub, subV))
		}
		return &enginepb.Cell{Value: &enginepb.Cell_StructValue{StructValue: st}}
	}
	return ValueToCell(v)
}

func isStructFieldType(t string) bool {
	switch strings.ToUpper(strings.TrimSpace(t)) {
	case bqTypeStruct, bqTypeRecord:
		return true
	default:
		return false
	}
}

func nullCell() *enginepb.Cell {
	return &enginepb.Cell{Value: &enginepb.Cell_NullValue{NullValue: true}}
}

// ValueToCell converts a generic Go value into a proto Cell using the
// same conventions as gateway/handlers/tabledata.jsonToCell. Exported
// so the YAML loader and tests can reuse the conversion without
// reimplementing the (long, type-switch-heavy) logic.
//
// Conventions:
//   - nil          -> Cell.null_value = true
//   - bool         -> "true"/"false"
//   - json.Number  -> decimal string verbatim
//   - float64/int  -> formatted decimal string
//   - string       -> string verbatim
//   - []byte       -> base64-encoded string (BYTES wire shape)
//   - []any        -> Array of converted cells
//   - map[string]any -> Struct (field order = map iteration order;
//     callers that need a deterministic order should pre-marshal to
//     a slice of {k, v} pairs and pass it through []any).
func ValueToCell(v any) *enginepb.Cell {
	if v == nil {
		return nullCell()
	}
	switch val := v.(type) {
	case bool:
		if val {
			return &enginepb.Cell{Value: &enginepb.Cell_StringValue{StringValue: "true"}}
		}
		return &enginepb.Cell{Value: &enginepb.Cell_StringValue{StringValue: "false"}}
	case json.Number:
		return &enginepb.Cell{Value: &enginepb.Cell_StringValue{StringValue: string(val)}}
	case float64:
		if val == float64(int64(val)) {
			return &enginepb.Cell{Value: &enginepb.Cell_StringValue{
				StringValue: strconv.FormatInt(int64(val), 10),
			}}
		}
		return &enginepb.Cell{Value: &enginepb.Cell_StringValue{
			StringValue: strconv.FormatFloat(val, 'g', -1, 64),
		}}
	case int:
		return &enginepb.Cell{Value: &enginepb.Cell_StringValue{
			StringValue: strconv.Itoa(val),
		}}
	case int64:
		return &enginepb.Cell{Value: &enginepb.Cell_StringValue{
			StringValue: strconv.FormatInt(val, 10),
		}}
	case string:
		return &enginepb.Cell{Value: &enginepb.Cell_StringValue{StringValue: val}}
	case []byte:
		return &enginepb.Cell{Value: &enginepb.Cell_StringValue{
			StringValue: base64.StdEncoding.EncodeToString(val),
		}}
	case []any:
		arr := &enginepb.Array{Elements: make([]*enginepb.Cell, 0, len(val))}
		for _, el := range val {
			arr.Elements = append(arr.Elements, ValueToCell(el))
		}
		return &enginepb.Cell{Value: &enginepb.Cell_Array{Array: arr}}
	case map[string]any:
		st := &enginepb.Struct{Fields: make([]*enginepb.Cell, 0, len(val))}
		for _, fv := range val {
			st.Fields = append(st.Fields, ValueToCell(fv))
		}
		return &enginepb.Cell{Value: &enginepb.Cell_StructValue{StructValue: st}}
	default:
		return &enginepb.Cell{Value: &enginepb.Cell_StringValue{
			StringValue: fmt.Sprintf("%v", val),
		}}
	}
}

// Defaults captures the gateway-level fallback values seeding uses
// when callers (REST clients or YAML files) leave a project or
// dataset location empty. The gateway package builds one of these
// from its Options struct (see gateway/seed_runner.go) so the seed
// package itself never imports the gateway package and the two
// can stay free of import cycles.
type Defaults struct {
	ProjectID       string
	DatasetLocation string
}
