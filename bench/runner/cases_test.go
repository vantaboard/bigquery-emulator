package runner

import (
	"path/filepath"
	"testing"
	"time"
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

func TestCaseSkippedFor(t *testing.T) {
	c := Case{
		SkipTargets: []TargetName{TargetGoccy},
		SkipReason:  "upstream bug",
	}
	if skipped, _ := c.SkippedFor(TargetEmulator); skipped {
		t.Fatal("emulator should not be skipped")
	}
	skipped, reason := c.SkippedFor(TargetGoccy)
	if !skipped || reason != "upstream bug" {
		t.Fatalf("goccy skip: %v %q", skipped, reason)
	}
}

func TestCaseQueryTimeout(t *testing.T) {
	fallback := 60 * time.Second
	if got := (Case{}).QueryTimeout(fallback); got != fallback {
		t.Fatalf("default: got %v", got)
	}
	if got := (Case{MaxMS: 180_000}).QueryTimeout(fallback); got != 180*time.Second {
		t.Fatalf("raised max_ms: got %v", got)
	}
	if got := (Case{QueryTimeoutMS: 90_000}).QueryTimeout(fallback); got != 90*time.Second {
		t.Fatalf("query_timeout_ms: got %v", got)
	}
}

func TestCaseQueryTimeoutForTargetGoccy(t *testing.T) {
	fallback := 60 * time.Second
	heavy := Case{MaxMS: 180_000}
	if got := heavy.QueryTimeoutForTarget(TargetEmulator, fallback); got != 180*time.Second {
		t.Fatalf("emulator heavy: got %v want 180s", got)
	}
	if got := heavy.QueryTimeoutForTarget(TargetGoccy, fallback); got != 10*time.Minute {
		t.Fatalf("goccy heavy floor: got %v want 10m", got)
	}
	explicit := Case{MaxMS: 180_000, QueryTimeoutMS: 240_000}
	if got := explicit.QueryTimeoutForTarget(TargetGoccy, fallback); got != 240*time.Second {
		t.Fatalf("explicit query_timeout_ms: got %v", got)
	}
}
