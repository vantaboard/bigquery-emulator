package seedfile

import (
	"context"
	"os"
	"path/filepath"
	"reflect"
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
	"github.com/vantaboard/bigquery-emulator/gateway/seed"
)

func repoRoot(t *testing.T) string {
	t.Helper()
	dir, err := os.Getwd()
	if err != nil {
		t.Fatalf("Getwd: %v", err)
	}
	for {
		if _, err := os.Stat(filepath.Join(dir, "go.mod")); err == nil {
			return dir
		}
		parent := filepath.Dir(dir)
		if parent == dir {
			t.Fatal("could not find repo root (go.mod)")
		}
		dir = parent
	}
}

func TestPublicDataFixtureLoads(t *testing.T) {
	path := PublicDataSeedPathFromRoot(repoRoot(t))
	f, err := Load(path)
	if err != nil {
		t.Fatalf("Load(%q): %v", path, err)
	}
	if f.DefaultLocation != "US" {
		t.Errorf("location=%q, want US", f.DefaultLocation)
	}
	if len(f.Datasets) != 3 {
		t.Fatalf("datasets=%d, want 3", len(f.Datasets))
	}
	wantTables := map[string]int{
		"usa_names":     1,
		"samples":       1,
		"stackoverflow": 1,
	}
	for _, ds := range f.Datasets {
		if ds.ProjectID != PublicDataProject {
			t.Errorf("dataset %q project_id=%q, want %q", ds.ID, ds.ProjectID, PublicDataProject)
		}
		want, ok := wantTables[ds.ID]
		if !ok {
			t.Errorf("unexpected dataset id %q", ds.ID)
			continue
		}
		if len(ds.Tables) != want {
			t.Errorf("dataset %q tables=%d, want %d", ds.ID, len(ds.Tables), want)
		}
		delete(wantTables, ds.ID)
	}
	if len(wantTables) != 0 {
		t.Errorf("missing datasets: %v", wantTables)
	}
}

func TestPublicDataFixtureApply(t *testing.T) {
	path := PublicDataSeedPathFromRoot(repoRoot(t))
	f, err := Load(path)
	if err != nil {
		t.Fatalf("Load: %v", err)
	}
	app := &captureApplier{}
	if err := Apply(context.Background(), f, app, seed.Defaults{}); err != nil {
		t.Fatalf("Apply: %v", err)
	}
	wantDatasets := []string{
		PublicDataProject + ".usa_names",
		PublicDataProject + ".samples",
		PublicDataProject + ".stackoverflow",
	}
	if !reflect.DeepEqual(app.datasets, wantDatasets) {
		t.Errorf("datasets=%v, want %v", app.datasets, wantDatasets)
	}
	for _, tbl := range SeededPublicTables {
		if _, ok := app.inserts[tbl]; !ok {
			t.Errorf("missing inserts for %s", tbl)
		}
	}
}

func TestPublicDataRefsFullySeeded(t *testing.T) {
	cases := []struct {
		name string
		sql  string
		want bool
	}{
		{
			name: "usa_names only",
			sql:  "SELECT name FROM `bigquery-public-data.usa_names.usa_1910_2013`",
			want: true,
		},
		{
			name: "shakespeare only",
			sql:  "SELECT corpus FROM `bigquery-public-data.samples.shakespeare`",
			want: true,
		},
		{
			name: "stackoverflow only",
			sql:  "SELECT id FROM `bigquery-public-data.stackoverflow.posts_questions`",
			want: true,
		},
		{
			name: "unseeded natality",
			sql:  "SELECT * FROM `bigquery-public-data.samples.natality`",
			want: false,
		},
		{
			name: "mixed seeded and unseeded",
			sql:  "FROM `bigquery-public-data.usa_names.usa_1910_2013` JOIN `bigquery-public-data.samples.natality`",
			want: false,
		},
		{
			name: "legacy colon syntax",
			sql:  "SELECT word FROM [bigquery-public-data:samples.shakespeare]",
			want: true,
		},
	}
	for _, tc := range cases {
		t.Run(tc.name, func(t *testing.T) {
			if got := PublicDataRefsFullySeeded(tc.sql); got != tc.want {
				t.Errorf("PublicDataRefsFullySeeded()=%v, want %v", got, tc.want)
			}
		})
	}
}

func TestApply_SkipsRowInsertWhenTableExists(t *testing.T) {
	f := &File{
		DefaultProjectID: testProjectDev,
		Datasets: []Dataset{
			{ID: "ds", Tables: []Table{
				{
					ID: "t",
					Schema: []FieldSchema{
						{Name: "id", Type: testFieldTypeInt},
					},
					Rows: []map[string]any{{"id": 1}},
				},
			}},
		},
	}
	app := &existingTableApplier{}
	if err := Apply(context.Background(), f, app, seed.Defaults{}); err != nil {
		t.Fatalf("Apply: %v", err)
	}
	if len(app.inserts) != 0 {
		t.Errorf("inserts=%v, want none when table already exists", app.inserts)
	}
}

// existingTableApplier reports tables as already present (created=false).
type existingTableApplier struct {
	inserts map[string][]map[string]any
}

func (e *existingTableApplier) EnsureDataset(_ context.Context, project, dataset, _ string) (bool, error) {
	_ = project + "." + dataset
	return false, nil
}

func (e *existingTableApplier) EnsureTable(
	_ context.Context,
	ref seed.TableRef,
	_ *enginepb.TableSchema,
) (bool, error) {
	_ = ref
	return false, nil
}

func (e *existingTableApplier) InsertRows(
	_ context.Context,
	ref seed.TableRef,
	_ *enginepb.TableSchema,
	rows []map[string]any,
) (int, error) {
	if e.inserts == nil {
		e.inserts = make(map[string][]map[string]any)
	}
	key := ref.ProjectID + "." + ref.DatasetID + "." + ref.TableID
	e.inserts[key] = append(e.inserts[key], rows...)
	return len(rows), nil
}
