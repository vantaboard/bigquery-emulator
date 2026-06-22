package runner

import (
	"strings"
	"testing"
	"time"
)

func TestLoadSessionMinimal(t *testing.T) {
	body := []byte(`name: smoke
steps:
  - dataset: ds
  - query: SELECT 1 AS n
    expect_rows:
      - {n: "1"}
`)
	s, err := loadSessionBytes(body, "smoke.yaml")
	if err != nil {
		t.Fatalf("loadSessionBytes: %v", err)
	}
	if s.ProjectID != "proj-session-smoke" {
		t.Fatalf("project_id=%q", s.ProjectID)
	}
}

func TestLoadSessionRepeatGroup(t *testing.T) {
	body := []byte(`name: repeat_case
steps:
  - repeat: 2
    steps:
      - expect_alive: true
`)
	if _, err := loadSessionBytes(body, "repeat.yaml"); err != nil {
		t.Fatalf("loadSessionBytes: %v", err)
	}
}

func TestLoadSessionRejectsEmptyStep(t *testing.T) {
	body := []byte(`name: bad
steps:
  - {}
`)
	if _, err := loadSessionBytes(body, "bad.yaml"); err == nil {
		t.Fatal("expected validation error for empty step")
	}
}

func TestLoadSessionRejectsQueryWithoutExpectation(t *testing.T) {
	body := []byte(`name: bad
steps:
  - query: SELECT 1
`)
	if _, err := loadSessionBytes(body, "bad.yaml"); err == nil {
		t.Fatal("expected validation error for query without expect_rows/error")
	}
}

const (
	testTableOther    = "other"
	testTableProfiles = "profiles"
)

func TestTableListDiff(t *testing.T) {
	exp := &TableListExpect{
		Dataset:     "ds",
		Contains:    []string{testTableProfiles, testTableOther},
		NotContains: []string{"ghost"},
	}
	diff := tableListDiff(exp, []string{testTableProfiles, "extra"})
	if diff == "" {
		t.Fatal("expected diff for missing other + unexpected not tested")
	}
	if !strings.Contains(diff, "missing tables: ["+testTableOther+"]") {
		t.Fatalf("diff=%q", diff)
	}

	diff = tableListDiff(exp, []string{testTableProfiles, testTableOther, "ghost"})
	if !strings.Contains(diff, "forbidden tables present: [ghost]") {
		t.Fatalf("diff=%q", diff)
	}

	diff = tableListDiff(exp, []string{testTableProfiles, testTableOther})
	if diff != "" {
		t.Fatalf("unexpected diff: %q", diff)
	}
}

func TestParseTableListIDs(t *testing.T) {
	body := []byte(`{"tables":[
	  {"tableReference":{"tableId":"profiles"}},
	  {"tableReference":{"tableId":"v"}}
	]}`)
	ids, err := parseTableListIDs(body)
	if err != nil {
		t.Fatal(err)
	}
	if len(ids) != 2 || ids[0] != testTableProfiles || ids[1] != "v" {
		t.Fatalf("ids=%v", ids)
	}
}

func TestResolveRESTURL(t *testing.T) {
	base := "http://127.0.0.1:1234/bigquery/v2/projects/proj"
	cases := []struct {
		path string
		want string
	}{
		{"datasets/ds_main", base + "/datasets/ds_main"},
		{
			"/bigquery/v2/projects/proj/datasets/ds_main",
			"http://127.0.0.1:1234/bigquery/v2/projects/proj/datasets/ds_main",
		},
	}
	for _, tc := range cases {
		got, err := resolveRESTURL(base, tc.path)
		if err != nil {
			t.Fatalf("path=%q: %v", tc.path, err)
		}
		if got != tc.want {
			t.Fatalf("path=%q got=%q want=%q", tc.path, got, tc.want)
		}
	}
}

func TestSessionKnownFailingFlip(t *testing.T) {
	r := Result{Status: StatusFail, Message: "engine died"}
	got := finishSessionMaybeKnown(r, testStart(), true)
	if got.Status != StatusSkip {
		t.Fatalf("status=%s", got.Status)
	}
	if !strings.Contains(got.Message, "known_failing") {
		t.Fatalf("message=%q", got.Message)
	}
}

func TestSessionProjectBaseUsesEnvURL(t *testing.T) {
	env := &EmulatorEnv{BaseURL: "http://127.0.0.1:9999"}
	got := sessionProjectBase(env, "proj-x")
	want := "http://127.0.0.1:9999/bigquery/v2/projects/proj-x"
	if got != want {
		t.Fatalf("got=%q want=%q", got, want)
	}
	env.BaseURL = "http://127.0.0.1:8888"
	got = sessionProjectBase(env, "proj-x")
	want = "http://127.0.0.1:8888/bigquery/v2/projects/proj-x"
	if got != want {
		t.Fatalf("after restart got=%q want=%q", got, want)
	}
}

func testStart() time.Time {
	return time.Now().Add(-time.Millisecond)
}
