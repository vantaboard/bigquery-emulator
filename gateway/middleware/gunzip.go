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
	"compress/gzip"
	"io"
	"net/http"
	"strings"
)

// WithGunzipRequestBody wraps next so that when the client sends
// `Content-Encoding: gzip` the request body is transparently
// decompressed before downstream handlers read it. The Java BigQuery
// client gzips JSON POST bodies by default; without this middleware
// every dataset/table create against the emulator REST gateway returns
// `invalid character '\x1f'` because handlers see the raw gzip framing.
//
// The middleware keeps the emulator and full-engine paths aligned on shape: missing/empty
// Content-Encoding short-circuits to next without allocation, an
// invalid gzip stream returns a BigQuery-shaped 400, and on success
// the Content-Encoding header is dropped so handlers don't double-
// decode if they happen to be aware of the encoding.
func WithGunzipRequestBody(next http.Handler) http.Handler {
	return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		if r == nil || r.Body == nil || r.Body == http.NoBody {
			next.ServeHTTP(w, r)
			return
		}
		ce := strings.ToLower(strings.TrimSpace(r.Header.Get("Content-Encoding")))
		if ce == "" || !strings.Contains(ce, "gzip") {
			next.ServeHTTP(w, r)
			return
		}
		gr, err := gzip.NewReader(r.Body)
		if err != nil {
			writeGunzipError(w, "invalid gzip request body: "+err.Error())
			return
		}
		// gzip.Reader.Close does NOT close the underlying io.ReadCloser
		// (see compress/gzip.Reader.Close), so wrap both so the http
		// server's body cleanup still fires.
		r.Body = &gzipRequestBody{gzip: gr, underlying: r.Body}
		// Drop the original Content-Encoding so handlers (and any
		// downstream middleware that inspects the header) don't try
		// to decode again, and clear Content-Length because the
		// inflated stream is necessarily a different size.
		r.Header.Del("Content-Encoding")
		r.Header.Del("Content-Length")
		r.ContentLength = -1
		next.ServeHTTP(w, r)
	})
}

type gzipRequestBody struct {
	gzip       *gzip.Reader
	underlying io.ReadCloser
}

func (b *gzipRequestBody) Read(p []byte) (int, error) {
	return b.gzip.Read(p)
}

func (b *gzipRequestBody) Close() error {
	_ = b.gzip.Close()
	return b.underlying.Close()
}

// writeGunzipError emits a BigQuery-shaped 400 envelope. Delegates to
// the shared [writeJSONError] helper so the envelope shape stays in
// sync with the method-override middleware's 400 path and so the
// "invalid" / "message" string literals are referenced from exactly
// one place (goconst-clean).
func writeGunzipError(w http.ResponseWriter, msg string) {
	writeJSONError(w, http.StatusBadRequest, errReasonInvalid, msg)
}
