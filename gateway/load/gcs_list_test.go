package load

import (
	"context"
	"encoding/json"
	"net/http"
	"net/http/httptest"
	"testing"
)

func TestExpandSourceURIsWildcard(t *testing.T) {
	const listBody = `{
	  "items": [
	    {"name": "data/customlayout/"},
	    {"name": "data/customlayout/pkey=foo/file.csv"},
	    {"name": "data/customlayout/pkey=bar/file.csv"}
	  ]
	}`
	srv := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		if r.URL.Path != "/storage/v1/b/bkt/o" {
			http.NotFound(w, r)
			return
		}
		w.Header().Set("Content-Type", "application/json")
		_, _ = w.Write([]byte(listBody))
	}))
	defer srv.Close()
	t.Setenv("STORAGE_EMULATOR_HOST", srv.URL)

	got, err := ExpandSourceURIs(context.Background(), []string{"gs://bkt/data/customlayout/*"})
	if err != nil {
		t.Fatalf("ExpandSourceURIs: %v", err)
	}
	if len(got) != 2 {
		t.Fatalf("expanded = %#v, want 2 object URIs", got)
	}
}

func TestListGCSObjectsPaginates(t *testing.T) {
	page := 0
	srv := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		if r.URL.Path != "/storage/v1/b/bkt/o" {
			http.NotFound(w, r)
			return
		}
		page++
		resp := map[string]any{
			"items": []map[string]string{{"name": "pfx/a"}},
		}
		if page == 1 {
			resp["nextPageToken"] = "tok"
		}
		_ = json.NewEncoder(w).Encode(resp)
	}))
	defer srv.Close()
	t.Setenv("STORAGE_EMULATOR_HOST", srv.URL)

	got, err := ListGCSObjects(context.Background(), "bkt", "pfx/")
	if err != nil {
		t.Fatalf("ListGCSObjects: %v", err)
	}
	if len(got) != 2 || page != 2 {
		t.Fatalf("names = %#v pages = %d, want 2 names across 2 pages", got, page)
	}
}

func TestMatchesGCSWildcard(t *testing.T) {
	t.Parallel()
	const wildcardPathPrefix = "path/"
	cases := []struct {
		name, prefix, suffix string
		want                 bool
	}{
		{"path/a.csv", wildcardPathPrefix, ".csv", true},
		{wildcardPathPrefix, wildcardPathPrefix, "", false},
		{"other/a.csv", wildcardPathPrefix, ".csv", false},
	}
	for _, tc := range cases {
		if got := matchesGCSWildcard(tc.name, tc.prefix, tc.suffix); got != tc.want {
			t.Fatalf("matchesGCSWildcard(%q) = %v, want %v", tc.name, got, tc.want)
		}
	}
}
