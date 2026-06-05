package seedfile

import (
	"context"
	"errors"
	"os"
	"path/filepath"
	"reflect"
	"strings"
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
	"github.com/vantaboard/bigquery-emulator/gateway/seed"
)

// colNameField is the shared "name" column literal used by the
// table-fixture tests. Pulled into a constant so goconst doesn't
// trip on a string that recurs four+ times across one schema +
// three row maps.
const colNameField = "name"

const (
	testProjectDev   = "dev"
	testFieldTypeInt = "INT64"
)

// captureApplier records every call seedfile makes so tests can
// assert on the order of ensure/insert operations.
type captureApplier struct {
	datasets []string
	tables   []string
	inserts  map[string][]map[string]any

	ensureDatasetErr error
	ensureTableErr   error
	insertErr        error
}

func (c *captureApplier) EnsureDataset(_ context.Context, project, dataset, _ string) (bool, error) {
	if c.ensureDatasetErr != nil {
		return false, c.ensureDatasetErr
	}
	c.datasets = append(c.datasets, project+"."+dataset)
	return true, nil
}

func (c *captureApplier) EnsureTable(_ context.Context, ref seed.TableRef, _ *enginepb.TableSchema) (bool, error) {
	if c.ensureTableErr != nil {
		return false, c.ensureTableErr
	}
	c.tables = append(c.tables, ref.ProjectID+"."+ref.DatasetID+"."+ref.TableID)
	return true, nil
}

func (c *captureApplier) InsertRows(
	_ context.Context,
	ref seed.TableRef,
	_ *enginepb.TableSchema,
	rows []map[string]any,
) (int, error) {
	if c.insertErr != nil {
		return 0, c.insertErr
	}
	if c.inserts == nil {
		c.inserts = make(map[string][]map[string]any)
	}
	key := ref.ProjectID + "." + ref.DatasetID + "." + ref.TableID
	c.inserts[key] = append(c.inserts[key], rows...)
	return len(rows), nil
}

// TestDecode_HappyPath pins the canonical schema example from the
// package doc -- it must parse without warnings.
func TestDecode_HappyPath(t *testing.T) {
	const y = `
project_id: dev
location: US
datasets:
  - id: ds
    tables:
      - id: people
        schema:
          - name: id
            type: INT64
            mode: REQUIRED
          - name: name
            type: STRING
        rows:
          - {id: 1, name: ada}
          - {id: 2, name: bob}
`
	f, err := Decode([]byte(y), "doc-example.yaml")
	if err != nil {
		t.Fatalf("Decode: %v", err)
	}
	if f.DefaultProjectID != testProjectDev || f.DefaultLocation != "US" {
		t.Errorf("file defaults = %+v", f)
	}
	if len(f.Datasets) != 1 {
		t.Fatalf("len(datasets)=%d, want 1", len(f.Datasets))
	}
	ds := f.Datasets[0]
	if ds.ID != "ds" {
		t.Errorf("dataset id=%q", ds.ID)
	}
	if len(ds.Tables) != 1 || ds.Tables[0].ID != "people" {
		t.Errorf("tables=%+v", ds.Tables)
	}
	if len(ds.Tables[0].Rows) != 2 {
		t.Errorf("rows=%d, want 2", len(ds.Tables[0].Rows))
	}
}

// TestDecode_RejectsUnknownKeys pins KnownFields(true): a typo at
// the top level surfaces immediately instead of silently doing
// nothing.
func TestDecode_RejectsUnknownKeys(t *testing.T) {
	_, err := Decode([]byte(`projects:\n  - id: foo\n`), "typo.yaml")
	if err == nil {
		t.Fatal("Decode accepted unknown top-level key")
	}
}

// TestDecode_ValidationErrors locks the per-field validation so
// operators get useful error messages rather than a generic
// "decode failed".
func TestDecode_ValidationErrors(t *testing.T) {
	cases := []struct {
		name string
		yaml string
		want string
	}{
		{
			name: "dataset-missing-id",
			yaml: "datasets:\n  - location: US\n",
			want: "datasets[0].id",
		},
		{
			name: "table-missing-id",
			yaml: "datasets:\n  - id: ds\n    tables:\n      - schema:\n          - {name: id, type: INT64}\n",
			want: "datasets[0].tables[0].id",
		},
		{
			name: "rows-without-schema",
			yaml: "datasets:\n  - id: ds\n    tables:\n      - id: t\n        rows:\n          - {x: 1}\n",
			want: "schema is required",
		},
		{
			name: "schema-field-missing-name",
			yaml: "datasets:\n  - id: ds\n    tables:\n      - id: t\n        schema:\n          - {type: INT64}\n",
			want: ".name",
		},
		{
			name: "schema-field-missing-type",
			yaml: "datasets:\n  - id: ds\n    tables:\n      - id: t\n        schema:\n          - {name: x}\n",
			want: ".type",
		},
	}
	for _, c := range cases {
		t.Run(c.name, func(t *testing.T) {
			_, err := Decode([]byte(c.yaml), c.name+".yaml")
			if err == nil {
				t.Fatal("Decode succeeded; want validation error")
			}
			if !strings.Contains(err.Error(), c.want) {
				t.Errorf("err=%q does not mention %q", err, c.want)
			}
		})
	}
}

// TestDecode_EmptyFileIsValid documents the no-op file case so a
// CI pipeline that templates an empty seed file doesn't fail.
func TestDecode_EmptyFileIsValid(t *testing.T) {
	f, err := Decode([]byte(""), "empty.yaml")
	if err != nil {
		t.Fatalf("Decode empty: %v", err)
	}
	if len(f.Datasets) != 0 {
		t.Errorf("empty file produced datasets: %+v", f)
	}
}

// TestApply_RequiresProjectFallback pins the precedence chain:
// table entry > file default > gateway default. If all three are
// empty Apply must return a clear error so the operator can fix
// their config.
func TestApply_RequiresProjectFallback(t *testing.T) {
	f := &File{Datasets: []Dataset{{ID: "ds"}}}
	app := &captureApplier{}
	err := Apply(context.Background(), f, app, seed.Defaults{})
	if err == nil {
		t.Fatal("Apply succeeded without any project; should error")
	}
	if !strings.Contains(err.Error(), "project") {
		t.Errorf("err=%q does not mention project", err)
	}
}

// TestApply_PrecedenceOrder confirms the documented chain produces
// the expected destination project for each row.
func TestApply_PrecedenceOrder(t *testing.T) {
	f := &File{
		DefaultProjectID: "file-default",
		Datasets: []Dataset{
			{ID: "a"},                      // -> file-default
			{ID: "b", ProjectID: "ds-pin"}, // -> ds-pin
		},
	}
	app := &captureApplier{}
	if err := Apply(context.Background(), f, app, seed.Defaults{ProjectID: "gw-default"}); err != nil {
		t.Fatalf("Apply: %v", err)
	}
	want := []string{"file-default.a", "ds-pin.b"}
	if !reflect.DeepEqual(app.datasets, want) {
		t.Errorf("datasets=%v, want %v", app.datasets, want)
	}
}

// TestApply_HappyPathWritesRowsInOrder pins the most important
// guarantee for operators who write reproducible seeds: row order
// is preserved.
func TestApply_HappyPathWritesRowsInOrder(t *testing.T) {
	f := &File{
		DefaultProjectID: testProjectDev,
		Datasets: []Dataset{
			{ID: "ds", Tables: []Table{
				{
					ID: "t",
					Schema: []FieldSchema{
						{Name: "id", Type: testFieldTypeInt},
						{Name: colNameField, Type: "STRING"},
					},
					Rows: []map[string]any{
						{"id": 1, colNameField: "ada"},
						{"id": 2, colNameField: "bob"},
						{"id": 3, colNameField: "carol"},
					},
				},
			}},
		},
	}
	app := &captureApplier{}
	if err := Apply(context.Background(), f, app, seed.Defaults{}); err != nil {
		t.Fatalf("Apply: %v", err)
	}
	got := app.inserts["dev.ds.t"]
	if len(got) != 3 {
		t.Fatalf("inserts=%d, want 3", len(got))
	}
	for i, want := range []float64{1, 2, 3} {
		switch v := got[i]["id"].(type) {
		case int:
			if float64(v) != want {
				t.Errorf("row %d id=%v, want %v", i, v, want)
			}
		case float64:
			if v != want {
				t.Errorf("row %d id=%v, want %v", i, v, want)
			}
		default:
			t.Errorf("row %d id is %T, expected int/float64", i, v)
		}
	}
}

// TestApplyFiles_OrderMatchesArgs pins the documented contract: the
// loader applies files in the order they were specified to
// `--seed-data-file`, not alphabetical / random.
func TestApplyFiles_OrderMatchesArgs(t *testing.T) {
	dir := t.TempDir()
	writeYAML := func(name, body string) string {
		path := filepath.Join(dir, name)
		if err := os.WriteFile(path, []byte(body), 0o644); err != nil {
			t.Fatalf("write %s: %v", path, err)
		}
		return path
	}
	p1 := writeYAML("alpha.yaml", `
project_id: p
datasets:
  - id: ds_first
`)
	p2 := writeYAML("beta.yaml", `
project_id: p
datasets:
  - id: ds_second
`)

	app := &captureApplier{}
	if err := ApplyFiles([]string{p2, p1}, app, seed.Defaults{}); err != nil {
		t.Fatalf("ApplyFiles: %v", err)
	}
	want := []string{"p.ds_second", "p.ds_first"}
	if !reflect.DeepEqual(app.datasets, want) {
		t.Errorf("datasets=%v, want %v (loader must follow argv order, not alphabetical)",
			app.datasets, want)
	}
}

// TestApply_PropagatesApplierError pins the failure surface: a
// single ensure failure aborts the whole seed (the orchestrator's
// "fold into ResourceErrors" behavior is for the production runner,
// not for declarative YAML seeds where the operator needs to see
// the failure up front).
func TestApply_PropagatesApplierError(t *testing.T) {
	f := &File{Datasets: []Dataset{{ID: "ds", ProjectID: "p"}}}
	app := &captureApplier{ensureDatasetErr: errors.New("write failed")}
	err := Apply(context.Background(), f, app, seed.Defaults{})
	if err == nil {
		t.Fatal("Apply ignored EnsureDataset error")
	}
}

// TestLoad_ReadsFromDisk is a smoke test for the disk-IO wrapper
// over Decode; the rest of the validation is covered by the
// in-memory tests.
func TestLoad_ReadsFromDisk(t *testing.T) {
	dir := t.TempDir()
	path := filepath.Join(dir, "seed.yaml")
	if err := os.WriteFile(path, []byte(`
project_id: dev
datasets:
  - id: ds
`), 0o644); err != nil {
		t.Fatalf("write: %v", err)
	}
	f, err := Load(path)
	if err != nil {
		t.Fatalf("Load: %v", err)
	}
	if len(f.Datasets) != 1 || f.Datasets[0].ID != "ds" {
		t.Errorf("Load result: %+v", f)
	}
}

// TestLoad_MissingFile bubbles a clear error rather than silently
// returning an empty file.
func TestLoad_MissingFile(t *testing.T) {
	_, err := Load(filepath.Join(t.TempDir(), "no.yaml"))
	if err == nil {
		t.Fatal("Load missing file succeeded")
	}
}

// TestApply_StructFieldSchemaWalks pins that nested STRUCT fields
// propagate through to the engine's schema proto.
func TestApply_StructFieldSchemaWalks(t *testing.T) {
	f := &File{
		Datasets: []Dataset{{
			ID:        "ds",
			ProjectID: "p",
			Tables: []Table{{
				ID: "t",
				Schema: []FieldSchema{
					{Name: "id", Type: "INT64"},
					{Name: "addr", Type: "STRUCT", Fields: []FieldSchema{
						{Name: "city", Type: "STRING"},
					}},
				},
			}},
		}},
	}
	// We don't need to inspect what the captureApplier saw beyond
	// the ensure path; the schema proto is built by fieldsToProto
	// and the orchestrator/applier tests already pin the wire
	// shape it produces.
	if err := Apply(context.Background(), f, &captureApplier{}, seed.Defaults{}); err != nil {
		t.Fatalf("Apply: %v", err)
	}
}
