package runner

import (
	"path/filepath"
	"testing"
)

func TestLoadCases(t *testing.T) {
	root := filepath.Join("..", "cases")
	cases, err := LoadCases(root)
	if err != nil {
		t.Fatal(err)
	}
	if len(cases) < 12 {
		t.Fatalf("expected at least 12 cases, got %d", len(cases))
	}
}

func TestCaseSubstitute(t *testing.T) {
	c := Case{
		Name:     "x",
		SetupSQL: []string{"CREATE TABLE {{ds}}.t AS SELECT 1"},
		Query:    "SELECT * FROM {{ds}}.t",
	}
	setup, query := c.Substitute("ds_x", "proj")
	if setup[0] != "CREATE TABLE ds_x.t AS SELECT 1" {
		t.Fatalf("setup: %q", setup[0])
	}
	if query != "SELECT * FROM ds_x.t" {
		t.Fatalf("query: %q", query)
	}
}
