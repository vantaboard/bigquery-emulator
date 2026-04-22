package server

import (
	"testing"
)

func TestDuckDSNFromStorage(t *testing.T) {
	t.Parallel()
	cases := []struct {
		in   Storage
		want string
	}{
		{MemoryStorage, ""},
		{Storage("file::memory:?cache=shared"), ""},
		{Storage("file:/tmp/bq.db?cache=shared"), "/tmp/bq.db"},
		{Storage("file:C:/data/x.duckdb?mode=rwc"), "C:/data/x.duckdb"},
		{Storage("/plain/path"), "/plain/path"},
	}
	for _, tc := range cases {
		if got := duckDSNFromStorage(tc.in); got != tc.want {
			t.Fatalf("duckDSNFromStorage(%q) = %q, want %q", tc.in, got, tc.want)
		}
	}
}
