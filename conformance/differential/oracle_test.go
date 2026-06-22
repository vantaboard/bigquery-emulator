package differential

import (
	"os"
	"path/filepath"
	"testing"

	"github.com/vantaboard/bigquery-emulator/conformance/runner"
)

func TestLoadOracleGolden(t *testing.T) {
	t.Parallel()
	root, err := repoRoot()
	if err != nil {
		t.Skip(err)
	}
	dir := filepath.Join(root, DefaultOracleDir)
	o, err := LoadOracle(dir, "select_literal.json")
	if err != nil {
		t.Fatalf("LoadOracle: %v", err)
	}
	exp := ExpectationFromOracle(o, "")
	if len(exp.Rows) != 1 || exp.Rows[0]["n"] != "1" {
		t.Fatalf("unexpected rows: %#v", exp.Rows)
	}
}

func TestExpectationFromOracleError(t *testing.T) {
	t.Parallel()
	o := &Oracle{
		CapturedAt:   "2026-06-22T00:00:00Z",
		Project:      "p",
		OracleSource: "test",
		Success:      false,
		Error:        &OracleError{Code: 400, Message: "bad query"},
	}
	exp := ExpectationFromOracle(o, runner.MatchOrdered)
	if exp.Error == nil || exp.Error.Code != 400 {
		t.Fatalf("expected error block: %#v", exp.Error)
	}
}

func TestLoadCorpusRequiresOracleRef(t *testing.T) {
	t.Parallel()
	data := []byte(`name: bad
query: SELECT 1
`)
	_, err := loadCorpusBytes(data, "bad.yaml")
	if err == nil {
		t.Fatal("want error for missing oracle_ref")
	}
}

func TestLoadCorpusDirSkipsSelfTestByDefault(t *testing.T) {
	t.Parallel()
	root, err := repoRoot()
	if err != nil {
		t.Skip(err)
	}
	corpus := filepath.Join(root, DefaultCorpusDir)
	cases, err := LoadCorpusDir(corpus, false)
	if err != nil {
		t.Fatalf("LoadCorpusDir: %v", err)
	}
	for _, c := range cases {
		if c.Name == "_selftest_pass" || c.Name == "_selftest_fail" {
			t.Fatalf("selftest case %q should be skipped by default", c.Name)
		}
	}
}

func repoRoot() (string, error) {
	wd, err := os.Getwd()
	if err != nil {
		return "", err
	}
	dir := wd
	for {
		if _, err := os.Stat(filepath.Join(dir, "go.mod")); err == nil {
			return dir, nil
		}
		parent := filepath.Dir(dir)
		if parent == dir {
			return "", os.ErrNotExist
		}
		dir = parent
	}
}

func TestWriteOracleRoundTrip(t *testing.T) {
	t.Parallel()
	dir := t.TempDir()
	path := filepath.Join(dir, "roundtrip.json")
	o := &Oracle{
		CapturedAt:   "2026-06-22T00:00:00Z",
		Project:      "proj",
		OracleSource: "test",
		Success:      false,
		Error:        &OracleError{Code: 400, Message: "nope"},
	}
	if err := WriteOracle(path, o); err != nil {
		t.Fatalf("WriteOracle: %v", err)
	}
	got, err := LoadOracle(dir, "roundtrip.json")
	if err != nil {
		t.Fatalf("LoadOracle: %v", err)
	}
	if got.Error == nil || got.Error.Message != "nope" {
		t.Fatalf("roundtrip mismatch: %#v", got.Error)
	}
	_ = os.Remove(path)
}
