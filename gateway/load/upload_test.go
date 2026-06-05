package load

import (
	"strings"
	"testing"
)

func TestParseMultipartJob(t *testing.T) {
	t.Parallel()
	body := strings.Join([]string{
		"--BOUNDARY",
		"Content-Type: application/json; charset=UTF-8",
		"",
		`{"configuration":{"load":{"sourceFormat":"CSV"}}}`,
		"--BOUNDARY",
		"Content-Type: */*",
		"",
		"name,code\nAlabama,AL\n",
		"--BOUNDARY--",
		"",
	}, "\r\n")
	meta, media, err := ParseMultipartJob([]byte(body), `multipart/related; boundary="BOUNDARY"`)
	if err != nil {
		t.Fatalf("ParseMultipartJob: %v", err)
	}
	if !strings.Contains(string(meta), "sourceFormat") {
		t.Fatalf("metadata = %q", meta)
	}
	if string(media) != "name,code\nAlabama,AL\n" {
		t.Fatalf("media = %q", media)
	}
}

func TestParseContentRange(t *testing.T) {
	t.Parallel()
	start, end, total, ok := ParseContentRange("bytes 0-9/10")
	if !ok || start != 0 || end != 9 || total != 10 {
		t.Fatalf("range = %d-%d/%d ok=%v", start, end, total, ok)
	}
	_, _, starTotal, ok := ParseContentRange("bytes */2000")
	if !ok || starTotal != 2000 {
		t.Fatalf("star total = %d ok=%v", starTotal, ok)
	}
}

func TestUploadStoreResumableAppend(t *testing.T) {
	t.Parallel()
	store := NewUploadStore()
	id := store.CreateSession("dev", []byte(`{"configuration":{"load":{}}}`), 5)
	if err := store.AppendBytes(id, []byte("hello"), 0); err != nil {
		t.Fatal(err)
	}
	if got := store.ReceivedBytes(id); got != 5 {
		t.Fatalf("received = %d, want 5", got)
	}
}
