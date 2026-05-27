package storagetmpl

import (
	"os"
	"path/filepath"
	"testing"
)

// Test-fixture literals reused across the materializer's table
// tests. Pulled into named constants so goconst doesn't trip on
// the same sentinel filename and the same nested data path that
// every "is the tree copied correctly?" case references.
const (
	sentinelFile = "catalog.duckdb"
	peopleData   = "data/people.parquet"
)

// writeTree builds a small directory tree under root using the
// path->contents map for input. Used to assemble template inputs that
// the materializer copies and to verify the destination shape after
// the copy.
func writeTree(t *testing.T, root string, files map[string]string) {
	t.Helper()
	for rel, contents := range files {
		full := filepath.Join(root, rel)
		if err := os.MkdirAll(filepath.Dir(full), 0o755); err != nil {
			t.Fatalf("mkdir %s: %v", filepath.Dir(full), err)
		}
		if err := os.WriteFile(full, []byte(contents), 0o644); err != nil {
			t.Fatalf("write %s: %v", full, err)
		}
	}
}

// readFile is a t.Fatalf-flavored wrapper so test bodies stay easy
// to read.
func readFile(t *testing.T, path string) string {
	t.Helper()
	b, err := os.ReadFile(path)
	if err != nil {
		t.Fatalf("read %s: %v", path, err)
	}
	return string(b)
}

// TestMaybeMaterialize_NoOpInputs locks in the four no-op conditions
// the operator-facing contract documents. None of them must touch
// the destination.
func TestMaybeMaterialize_NoOpInputs(t *testing.T) {
	t.Run("empty-template", func(t *testing.T) {
		dst := t.TempDir()
		if err := MaybeMaterialize("", dst); err != nil {
			t.Fatalf("MaybeMaterialize(\"\", dst): %v", err)
		}
		entries, _ := os.ReadDir(dst)
		if len(entries) != 0 {
			t.Errorf("dst populated despite empty template: %v", entries)
		}
	})

	t.Run("empty-dataDir", func(t *testing.T) {
		// dst="" means the engine will use its in-memory
		// fallback; no on-disk seed work is possible.
		src := t.TempDir()
		writeTree(t, src, map[string]string{sentinelFile: "x"})
		if err := MaybeMaterialize(src, ""); err != nil {
			t.Fatalf("MaybeMaterialize(src, \"\"): %v", err)
		}
	})

	t.Run("already-initialized", func(t *testing.T) {
		src := t.TempDir()
		writeTree(t, src, map[string]string{
			sentinelFile:       "template-version",
			"data/foo.parquet": "template-foo",
		})
		dst := t.TempDir()
		writeTree(t, dst, map[string]string{
			sentinelFile: "existing",
		})
		if err := MaybeMaterialize(src, dst); err != nil {
			t.Fatalf("MaybeMaterialize: %v", err)
		}
		// Existing sentinel must not be overwritten and nothing
		// new must appear in dst.
		if got := readFile(t, filepath.Join(dst, sentinelFile)); got != "existing" {
			t.Errorf(
				"catalog.duckdb overwritten: got %q, want %q (sentinel must protect existing data)",
				got,
				"existing",
			)
		}
		if _, err := os.Stat(filepath.Join(dst, "data", "foo.parquet")); err == nil {
			t.Error("template's data/foo.parquet copied despite sentinel; reseeding would clobber operator data")
		}
	})
}

// TestMaybeMaterialize_CopiesTemplate is the happy path: a fresh
// data-dir gets the whole template tree, including nested
// subdirectories.
func TestMaybeMaterialize_CopiesTemplate(t *testing.T) {
	src := t.TempDir()
	writeTree(t, src, map[string]string{
		sentinelFile:               "cat-data",
		"meta/dataset_a.meta.json": "{}",
		peopleData:                 "parquet-bytes",
		"data/nested/deeply/x":     "x",
	})

	// Destination does not exist yet; the helper should create it.
	dst := filepath.Join(t.TempDir(), "data_dir")
	if err := MaybeMaterialize(src, dst); err != nil {
		t.Fatalf("MaybeMaterialize: %v", err)
	}

	for rel, want := range map[string]string{
		sentinelFile:               "cat-data",
		"meta/dataset_a.meta.json": "{}",
		peopleData:                 "parquet-bytes",
		"data/nested/deeply/x":     "x",
	} {
		got := readFile(t, filepath.Join(dst, rel))
		if got != want {
			t.Errorf("%s: got %q, want %q", rel, got, want)
		}
	}
}

// TestMaybeMaterialize_FailsOnNonDirectoryDst guards the operator
// against accidentally pointing data-dir at a file (e.g. a stray
// `--data-dir=./catalog.duckdb`).
func TestMaybeMaterialize_FailsOnNonDirectoryDst(t *testing.T) {
	src := t.TempDir()
	writeTree(t, src, map[string]string{sentinelFile: "x"})

	dstFile := filepath.Join(t.TempDir(), "data_dir")
	if err := os.WriteFile(dstFile, []byte("not a dir"), 0o644); err != nil {
		t.Fatalf("seed dst file: %v", err)
	}
	if err := MaybeMaterialize(src, dstFile); err == nil {
		t.Fatal("MaybeMaterialize succeeded for file dst; want error")
	}
}

// TestMaybeMaterialize_FailsOnMissingTemplate surfaces operator
// errors rather than silently no-op'ing.
func TestMaybeMaterialize_FailsOnMissingTemplate(t *testing.T) {
	dst := t.TempDir()
	missing := filepath.Join(t.TempDir(), "absent")
	if err := MaybeMaterialize(missing, dst); err == nil {
		t.Fatal("MaybeMaterialize succeeded for missing template; want error")
	}
}

// TestMaybeMaterialize_SecondCallIsIdempotent verifies the
// "already-initialized" gate works after a successful first call.
// The second invocation must be a no-op (even when the template tree
// has since changed) so subsequent boots respect operator writes.
func TestMaybeMaterialize_SecondCallIsIdempotent(t *testing.T) {
	src := t.TempDir()
	writeTree(t, src, map[string]string{
		sentinelFile: "v1",
		peopleData:   "p1",
	})
	dst := t.TempDir()

	if err := MaybeMaterialize(src, dst); err != nil {
		t.Fatalf("first MaybeMaterialize: %v", err)
	}
	// Simulate the engine writing new data after first boot.
	if err := os.WriteFile(filepath.Join(dst, peopleData),
		[]byte("operator-write"), 0o644); err != nil {
		t.Fatalf("simulate engine write: %v", err)
	}
	// Template version bump; this must NOT propagate on a
	// subsequent boot because the sentinel is present.
	writeTree(t, src, map[string]string{
		sentinelFile: "v2",
		peopleData:   "p2-template",
	})
	if err := MaybeMaterialize(src, dst); err != nil {
		t.Fatalf("second MaybeMaterialize: %v", err)
	}

	if got := readFile(t, filepath.Join(dst, sentinelFile)); got != "v1" {
		t.Errorf("catalog overwritten on re-run: got %q, want %q", got, "v1")
	}
	if got := readFile(t, filepath.Join(dst, peopleData)); got != "operator-write" {
		t.Errorf("operator data overwritten on re-run: got %q, want %q",
			got, "operator-write")
	}
}
