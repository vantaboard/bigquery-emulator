package runner

import (
	"strings"
	"testing"
)

func TestResolveProfilesDefaultsToAll(t *testing.T) {
	got, err := resolveProfiles(nil)
	if err != nil {
		t.Fatalf("resolveProfiles(nil): %v", err)
	}
	if len(got) != len(KnownProfiles()) {
		t.Errorf("len=%d, want %d", len(got), len(KnownProfiles()))
	}
}

func TestResolveProfilesFiltersToSubset(t *testing.T) {
	got, err := resolveProfiles([]string{ProfileDuckDB})
	if err != nil {
		t.Fatalf("resolveProfiles: %v", err)
	}
	if len(got) != 1 || got[0].Name != ProfileDuckDB {
		t.Fatalf("got=%v, want [duckdb]", got)
	}
}

func TestResolveProfilesRejectsUnknown(t *testing.T) {
	_, err := resolveProfiles([]string{testBogus})
	if err == nil || !strings.Contains(err.Error(), "unknown") {
		t.Fatalf("want unknown-profile error, got %v", err)
	}
}

// TestRunReportExitCode exercises the exit-code derivation so the
// CLI's exit-mapping is unit-testable without a real subprocess.
func TestRunReportExitCode(t *testing.T) {
	cases := []struct {
		name string
		r    *Report
		want int
	}{
		{"nil", nil, 2},
		{"all pass", &Report{Summary: Summary{Total: 3, Passed: 3}}, 0},
		{"one fail", &Report{Summary: Summary{Total: 3, Passed: 2, Failed: 1}}, 1},
	}
	for _, c := range cases {
		t.Run(c.name, func(t *testing.T) {
			if got := c.r.ExitCode(); got != c.want {
				t.Errorf("ExitCode=%d, want %d", got, c.want)
			}
		})
	}
}
