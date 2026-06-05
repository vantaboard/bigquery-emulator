package load

import (
	"context"
	"net/http"
	"net/http/httptest"
	"os"
	"path/filepath"
	"testing"
)

func TestFetchSourceFileURI(t *testing.T) {
	t.Parallel()
	dir := t.TempDir()
	path := filepath.Join(dir, "data.csv")
	if err := os.WriteFile(path, []byte("a,b\n1,2\n"), 0o644); err != nil {
		t.Fatal(err)
	}
	got, err := FetchSource(context.Background(), "file://"+path)
	if err != nil {
		t.Fatalf("FetchSource: %v", err)
	}
	if string(got) != "a,b\n1,2\n" {
		t.Fatalf("body = %q", string(got))
	}
}

func TestStorageEmulatorBaseUnsetDefaultsLocalhost(t *testing.T) {
	t.Setenv("STORAGE_EMULATOR_HOST", "")
	t.Setenv("FAKE_GCS_PORT", "9999")
	if got := storageEmulatorBase(); got != "http://127.0.0.1:9999" {
		t.Fatalf("storageEmulatorBase() = %q, want http://127.0.0.1:9999", got)
	}
}

func TestStorageEmulatorBaseDockerHostname(t *testing.T) {
	t.Setenv("STORAGE_EMULATOR_HOST", "http://fake-gcs-server:4443")
	if got := storageEmulatorBase(); got != "http://fake-gcs-server:4443" {
		t.Fatalf("storageEmulatorBase() = %q, want http://fake-gcs-server:4443", got)
	}
}

func TestFetchSourceGCSViaEmulator(t *testing.T) {
	srv := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		if r.URL.Path != "/storage/v1/b/bucket/o/object.csv" {
			http.NotFound(w, r)
			return
		}
		if r.URL.Query().Get("alt") != "media" {
			http.NotFound(w, r)
			return
		}
		_, _ = w.Write([]byte("x,y\np,q\n"))
	}))
	defer srv.Close()
	t.Setenv("STORAGE_EMULATOR_HOST", srv.URL)

	got, err := FetchSource(context.Background(), "gs://bucket/object.csv")
	if err != nil {
		t.Fatalf("FetchSource: %v", err)
	}
	if string(got) != "x,y\np,q\n" {
		t.Fatalf("body = %q", string(got))
	}
}
