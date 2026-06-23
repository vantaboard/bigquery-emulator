package conformance

import (
	"os"
	"path/filepath"
	"regexp"
	"runtime"
	"strings"
	"testing"
)

// regressionsMD is the machine-readable index at conformance/REGRESSIONS.md.
// The test parses ```regressions-index YAML blocks and asserts every listed
// path exists relative to the repo root.
func repoRoot(t *testing.T) string {
	t.Helper()
	_, here, _, ok := runtime.Caller(0)
	if !ok {
		t.Fatal("runtime.Caller(0) failed")
	}
	return filepath.Clean(filepath.Join(filepath.Dir(here), ".."))
}

func TestRegressionsIndexPathsExist(t *testing.T) {
	root := repoRoot(t)
	data, err := os.ReadFile(filepath.Join(root, "conformance", "REGRESSIONS.md"))
	if err != nil {
		t.Fatalf("read REGRESSIONS.md: %v", err)
	}

	re := regexp.MustCompile("(?s)```regressions-index\\s*(.*?)```")
	m := re.FindSubmatch(data)
	if len(m) != 2 {
		t.Fatal("REGRESSIONS.md missing ```regressions-index``` YAML block")
	}

	tagRe := regexp.MustCompile(`(?m)^(R[1-9][0-9]*):`)
	tags := tagRe.FindAllStringSubmatch(string(m[1]), -1)
	if len(tags) != 10 {
		t.Fatalf("expected 10 regression tags R1–R10, found %d", len(tags))
	}

	pathRe := regexp.MustCompile(`(?m)^  - (.+)$`)
	paths := pathRe.FindAllStringSubmatch(string(m[1]), -1)
	if len(paths) == 0 {
		t.Fatal("no paths listed under regressions-index")
	}

	seen := make(map[string]struct{})
	for _, pm := range paths {
		rel := strings.TrimSpace(pm[1])
		if rel == "" {
			t.Fatal("empty path in regressions-index")
		}
		if _, dup := seen[rel]; dup {
			t.Errorf("duplicate path in index: %s", rel)
			continue
		}
		seen[rel] = struct{}{}
		abs := filepath.Join(root, filepath.FromSlash(rel))
		if st, err := os.Stat(abs); err != nil || st.IsDir() {
			t.Errorf("missing regression artifact %s: %v", rel, err)
		}
	}
}
