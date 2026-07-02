package runner

import (
	"os"
	"strings"
	"testing"
)

func TestOptionalDependencySkipReasonEmpty(t *testing.T) {
	if got := optionalDependencySkipReason(nil); got != "" {
		t.Fatalf("optionalDependencySkipReason(nil)=%q, want empty", got)
	}
}

func TestOptionalDependencySkipReasonMissingPackage(t *testing.T) {
	reason := optionalDependencySkipReason([]string{"bqemu_nonexistent_pkg_xyz"})
	if reason == "" {
		t.Fatal("expected skip reason for absent package")
	}
	if !strings.Contains(reason, "bqemu_nonexistent_pkg_xyz") {
		t.Fatalf("reason %q should name the missing package", reason)
	}
}

func TestResolveConformancePythonPrefersExplicitEnv(t *testing.T) {
	t.Setenv("BIGQUERY_EMULATOR_PYTHON", os.Args[0])
	t.Setenv("BIGQUERY_EMULATOR_DATA_DIR", t.TempDir())
	got, err := resolveConformancePython()
	if err != nil {
		t.Fatalf("resolveConformancePython: %v", err)
	}
	if got != os.Args[0] {
		t.Fatalf("got %q, want %q", got, os.Args[0])
	}
}
