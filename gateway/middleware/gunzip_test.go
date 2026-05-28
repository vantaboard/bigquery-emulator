// Copyright 2026 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

package middleware

import (
	"bytes"
	"compress/gzip"
	"io"
	"net/http"
	"net/http/httptest"
	"strings"
	"testing"
)

func gzipBytes(t *testing.T, plain string) []byte {
	t.Helper()
	var buf bytes.Buffer
	gw := gzip.NewWriter(&buf)
	if _, err := gw.Write([]byte(plain)); err != nil {
		t.Fatalf("gzip write: %v", err)
	}
	if err := gw.Close(); err != nil {
		t.Fatalf("gzip close: %v", err)
	}
	return buf.Bytes()
}

func TestWithGunzipRequestBodyDecodesGzipPOST(t *testing.T) {
	const plain = `{"datasetReference":{"projectId":"p","datasetId":"d"}}`
	gz := gzipBytes(t, plain)

	var got []byte
	next := http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		b, err := io.ReadAll(r.Body)
		if err != nil {
			t.Fatalf("handler read body: %v", err)
		}
		got = b
		if got := r.Header.Get("Content-Encoding"); got != "" {
			t.Errorf("Content-Encoding still set in handler: %q", got)
		}
		w.WriteHeader(http.StatusOK)
	})

	req := httptest.NewRequest(http.MethodPost, "/v1/x", bytes.NewReader(gz))
	req.Header.Set("Content-Type", "application/json")
	req.Header.Set("Content-Encoding", "gzip")
	rec := httptest.NewRecorder()
	WithGunzipRequestBody(next).ServeHTTP(rec, req)

	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d, want 200; body=%s", rec.Code, rec.Body.String())
	}
	if string(got) != plain {
		t.Errorf("decoded body = %q, want %q", string(got), plain)
	}
}

func TestWithGunzipRequestBodyPassesThroughPlain(t *testing.T) {
	const plain = `{"hello":"world"}`

	var got []byte
	next := http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		b, _ := io.ReadAll(r.Body)
		got = b
		w.WriteHeader(http.StatusOK)
	})

	req := httptest.NewRequest(http.MethodPost, "/v1/x", strings.NewReader(plain))
	req.Header.Set("Content-Type", "application/json")
	rec := httptest.NewRecorder()
	WithGunzipRequestBody(next).ServeHTTP(rec, req)

	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d, want 200; body=%s", rec.Code, rec.Body.String())
	}
	if string(got) != plain {
		t.Errorf("body = %q, want %q (middleware mutated a non-gzip body)", string(got), plain)
	}
}

func TestWithGunzipRequestBodyNoBodyShortCircuits(t *testing.T) {
	called := false
	next := http.HandlerFunc(func(w http.ResponseWriter, _ *http.Request) {
		called = true
		w.WriteHeader(http.StatusOK)
	})

	req := httptest.NewRequest(http.MethodGet, "/v1/x", http.NoBody)
	rec := httptest.NewRecorder()
	WithGunzipRequestBody(next).ServeHTTP(rec, req)

	if !called {
		t.Fatalf("next handler was not invoked for an empty-body GET")
	}
}

func TestWithGunzipRequestBodyInvalidGzipReturns400(t *testing.T) {
	called := false
	next := http.HandlerFunc(func(w http.ResponseWriter, _ *http.Request) {
		called = true
		w.WriteHeader(http.StatusOK)
	})

	req := httptest.NewRequest(http.MethodPost, "/v1/x",
		strings.NewReader("this is not a gzip stream"))
	req.Header.Set("Content-Encoding", "gzip")
	rec := httptest.NewRecorder()
	WithGunzipRequestBody(next).ServeHTTP(rec, req)

	if rec.Code != http.StatusBadRequest {
		t.Fatalf("status = %d, want 400; body=%s", rec.Code, rec.Body.String())
	}
	if called {
		t.Errorf("next handler should not be invoked when gzip framing is broken")
	}
	if !strings.Contains(rec.Body.String(), "invalid gzip request body") {
		t.Errorf("body = %q, want it to mention invalid gzip", rec.Body.String())
	}
}
