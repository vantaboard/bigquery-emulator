package load

import "testing"

func TestAbsoluteSessionLocation(t *testing.T) {
	t.Parallel()
	got := AbsoluteSessionLocation("http://localhost:9050", "dev", "abc")
	want := "http://localhost:9050/upload/bigquery/v2/projects/dev/jobs?uploadType=resumable&upload_id=abc"
	if got != want {
		t.Fatalf("got %q, want %q", got, want)
	}
	rel := SessionLocation("dev", "abc")
	if got := AbsoluteSessionLocation("", "dev", "abc"); got != rel {
		t.Fatalf("empty base should return relative path %q, got %q", rel, got)
	}
}
