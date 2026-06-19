package runner

import "testing"

func TestRewriteGoccyCreateOrReplace_View(t *testing.T) {
	t.Parallel()
	drop, create, ok := rewriteGoccyCreateOrReplace("CREATE OR REPLACE VIEW ds.v AS\nSELECT 1 AS n")
	if !ok {
		t.Fatal("expected rewrite")
	}
	if drop != "DROP VIEW IF EXISTS ds.v" {
		t.Fatalf("drop = %q", drop)
	}
	if create != "CREATE VIEW ds.v AS SELECT 1 AS n" {
		t.Fatalf("create = %q", create)
	}
}

func TestRewriteGoccyCreateOrReplace_Table(t *testing.T) {
	t.Parallel()
	drop, create, ok := rewriteGoccyCreateOrReplace("CREATE OR REPLACE TABLE ds.dst AS SELECT k FROM ds.src")
	if !ok {
		t.Fatal("expected rewrite")
	}
	if drop != "DROP TABLE IF EXISTS ds.dst" {
		t.Fatalf("drop = %q", drop)
	}
	if create != "CREATE TABLE ds.dst AS SELECT k FROM ds.src" {
		t.Fatalf("create = %q", create)
	}
}

func TestRewriteGoccyCreateOrReplace_Passthrough(t *testing.T) {
	t.Parallel()
	_, create, ok := rewriteGoccyCreateOrReplace("SELECT 1")
	if ok {
		t.Fatal("expected no rewrite")
	}
	if create != "SELECT 1" {
		t.Fatalf("create = %q", create)
	}
}
