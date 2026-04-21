package explorerapi

import (
	"reflect"
	"testing"
)

func TestParseBigQueryV2ProjectListJSON(t *testing.T) {
	t.Parallel()
	cases := []struct {
		name    string
		json    string
		want    []string
		wantErr bool
	}{
		{
			name: "projects lowercase id",
			json: `{"projects":[{"id":"p-a"},{"id":"p-b"}]}`,
			want: []string{"p-a", "p-b"},
		},
		{
			name: "Projects capital P",
			json: `{"Projects":[{"Id":"x"}]}`,
			want: []string{"x"},
		},
		{
			name: "projectReference",
			json: `{"projects":[{"projectReference":{"projectId":"ref1"}}]}`,
			want: []string{"ref1"},
		},
		{
			name:    "empty projects",
			json:    `{"projects":[]}`,
			wantErr: true,
		},
		{
			name:    "invalid json",
			json:    `{`,
			wantErr: true,
		},
	}
	for _, tc := range cases {
		tc := tc
		t.Run(tc.name, func(t *testing.T) {
			t.Parallel()
			got, err := parseBigQueryV2ProjectListJSON([]byte(tc.json))
			if tc.wantErr {
				if err == nil {
					t.Fatalf("expected error, got nil (%v)", got)
				}
				return
			}
			if err != nil {
				t.Fatalf("parse: %v", err)
			}
			if !reflect.DeepEqual(got, tc.want) {
				t.Fatalf("got %#v want %#v", got, tc.want)
			}
		})
	}
}

func TestDedupeSorted(t *testing.T) {
	t.Parallel()
	got := dedupeSorted([]string{"b", "a", "b", ""})
	want := []string{"a", "b"}
	if !reflect.DeepEqual(got, want) {
		t.Fatalf("got %#v want %#v", got, want)
	}
}

func TestApplyProjectIDListEnv(t *testing.T) {
	t.Run("merge unions and sorts", func(t *testing.T) {
		t.Setenv("BIGQUERY_PROJECT_IDS", "z,alpha")
		t.Setenv("BIGQUERY_PROJECT_IDS_MODE", "")
		got := ApplyProjectIDListEnv([]string{"m"}, "def")
		want := []string{"alpha", "m", "z"}
		if !reflect.DeepEqual(got, want) {
			t.Fatalf("got %#v want %#v", got, want)
		}
	})
	t.Run("override uses env only", func(t *testing.T) {
		t.Setenv("BIGQUERY_PROJECT_IDS", "only")
		t.Setenv("BIGQUERY_PROJECT_IDS_MODE", "override")
		got := ApplyProjectIDListEnv([]string{"disc"}, "def")
		want := []string{"only"}
		if !reflect.DeepEqual(got, want) {
			t.Fatalf("got %#v want %#v", got, want)
		}
	})
	t.Run("override empty falls back to default project", func(t *testing.T) {
		t.Setenv("BIGQUERY_PROJECT_IDS", "")
		t.Setenv("BIGQUERY_PROJECT_IDS_MODE", "override")
		got := ApplyProjectIDListEnv([]string{"x"}, "def")
		want := []string{"def"}
		if !reflect.DeepEqual(got, want) {
			t.Fatalf("got %#v want %#v", got, want)
		}
	})
}

func TestValidateProjectIDForPath(t *testing.T) {
	t.Parallel()
	if !ValidateProjectIDForPath("valid-id_01") {
		t.Fatal("expected valid")
	}
	for _, bad := range []string{"", "a/b", "..x", "http://x"} {
		if ValidateProjectIDForPath(bad) {
			t.Fatalf("expected invalid: %q", bad)
		}
	}
}

func TestNormalizeEmulatorHost(t *testing.T) {
	t.Parallel()
	if got := NormalizeEmulatorHost(" http://host:9050/ "); got != "host:9050" {
		t.Fatalf("got %q", got)
	}
}
