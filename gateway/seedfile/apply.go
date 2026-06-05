package seedfile

import (
	"context"
	"fmt"
	"strings"
	"time"

	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
	"github.com/vantaboard/bigquery-emulator/gateway/seed"
)

// applyTimeout bounds the total wall time the loader spends talking
// to the engine when applying one file. Large seeds (thousands of
// rows) easily fit inside this budget on a local engine; a value
// much smaller risks flaky startups on CI when the engine is still
// warming caches.
const applyTimeout = 2 * time.Minute

// ApplyFiles loads and applies each YAML seed file in order. The
// loader fails fast on the first file that does not parse or
// validate so operators see the actual schema error rather than a
// stream of confusing follow-on failures.
//
// `defaults` carries the gateway-level fallbacks (project id, dataset
// location) used when a file omits them. `applier` is the engine-
// facing surface; in production this is seed.NewCatalogApplier over
// the live CatalogClient.
func ApplyFiles(paths []string, applier seed.Applier, defaults seed.Defaults) error {
	ctx, cancel := context.WithTimeout(context.Background(), applyTimeout)
	defer cancel()
	return ApplyFilesContext(ctx, paths, applier, defaults)
}

// ApplyFilesContext is the context-aware twin of ApplyFiles. Use
// this from tests so they can pass a short-deadline context to
// exercise cancellation behavior.
func ApplyFilesContext(ctx context.Context, paths []string, applier seed.Applier, defaults seed.Defaults) error {
	for _, p := range paths {
		f, err := Load(p)
		if err != nil {
			return err
		}
		if err := Apply(ctx, f, applier, defaults); err != nil {
			return fmt.Errorf("seedfile %s: %w", p, err)
		}
	}
	return nil
}

// Apply materializes one decoded File against the engine via the
// supplied applier. The order is deterministic: datasets in the
// order they appear in the file, tables within a dataset in
// declaration order, rows in declaration order. Operators rely on
// this for reproducible seeds (e.g. autoincrement-style ids).
//
// The function is forgiving on "already exists" errors at the
// dataset and table level: the applier returns created=false in
// that case. Rows are inserted only when the table was newly
// created so gateway restarts against a persistent data_dir do
// not duplicate seed data.
func Apply(ctx context.Context, f *File, applier seed.Applier, defaults seed.Defaults) error {
	if f == nil {
		return nil
	}
	for i, ds := range f.Datasets {
		project := firstNonEmpty(ds.ProjectID, f.DefaultProjectID, defaults.ProjectID)
		if project == "" {
			return fmt.Errorf(
				"datasets[%d] (id=%q): no project_id set (file default, dataset entry, and --project-id all empty)",
				i,
				ds.ID,
			)
		}
		location := firstNonEmpty(ds.Location, f.DefaultLocation, defaults.DatasetLocation)
		if _, err := applier.EnsureDataset(ctx, project, ds.ID, location); err != nil {
			return fmt.Errorf("ensure dataset %s.%s: %w", project, ds.ID, err)
		}
		for j, tbl := range ds.Tables {
			schema := fieldsToProto(tbl.Schema)
			ref := seed.TableRef{
				ProjectID: project,
				DatasetID: ds.ID,
				TableID:   tbl.ID,
			}
			created, err := applier.EnsureTable(ctx, ref, schema)
			if err != nil {
				return fmt.Errorf("ensure table %s.%s.%s: %w",
					project, ds.ID, tbl.ID, err)
			}
			if len(tbl.Rows) == 0 || !created {
				continue
			}
			if _, err := applier.InsertRows(ctx, ref, schema, tbl.Rows); err != nil {
				return fmt.Errorf("insert rows for %s.%s.%s (file datasets[%d].tables[%d]): %w",
					project, ds.ID, tbl.ID, i, j, err)
			}
		}
	}
	return nil
}

// firstNonEmpty returns the first trim-non-empty string from the
// supplied values. Used to walk the (entry > file-default >
// gateway-default) precedence chain for project id and location.
func firstNonEmpty(vs ...string) string {
	for _, v := range vs {
		if t := strings.TrimSpace(v); t != "" {
			return t
		}
	}
	return ""
}

// fieldsToProto recursively converts the YAML FieldSchema slice
// into the engine's proto TableSchema. Nested STRUCT/RECORD fields
// are walked verbatim.
func fieldsToProto(fields []FieldSchema) *enginepb.TableSchema {
	out := &enginepb.TableSchema{Fields: make([]*enginepb.FieldSchema, 0, len(fields))}
	for _, f := range fields {
		out.Fields = append(out.Fields, fieldToProto(f))
	}
	return out
}

func fieldToProto(f FieldSchema) *enginepb.FieldSchema {
	pf := &enginepb.FieldSchema{
		Name:        f.Name,
		Type:        f.Type,
		Mode:        f.Mode,
		Description: f.Description,
	}
	for _, sub := range f.Fields {
		pf.Fields = append(pf.Fields, fieldToProto(sub))
	}
	return pf
}
