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
	"net/http"
	"strings"
)

// methodOverrideHeader is the canonical header name the
// google-api-client `MethodOverride` interceptor sets when its
// underlying transport reports that a method (typically PATCH) is
// unsupported. See `com.google.api.client.googleapis.MethodOverride`
// in google-api-client-2.x and `NetHttpTransport.SUPPORTED_METHODS`
// in google-http-client (PATCH is intentionally omitted from that
// list, so PATCH-shaped REST calls travel as POST + this header).
const methodOverrideHeader = "X-HTTP-Method-Override"

// WithMethodOverride returns middleware that honors the
// `X-HTTP-Method-Override` request header so emulator clients written
// against transports that do not support PATCH (notably the Java
// google-api-client + java.net.HttpURLConnection combo) can drive
// PATCH/PUT/DELETE handlers via a tunneled POST.
//
// Why we need this: the Java BigQuery client's default
// `NetHttpTransport` advertises support for GET, HEAD, OPTIONS, POST,
// PUT, DELETE, TRACE — but not PATCH. The
// `com.google.api.client.googleapis.MethodOverride` interceptor (a
// default `HttpExecuteInterceptor` on every google-api-client request)
// rewrites unsupported methods to POST and sets
// `X-HTTP-Method-Override: <originalMethod>`. The gateway's mux
// otherwise mounts dataset/table/job updates at PATCH and PUT, so
// without this middleware Java callers like AuthorizeDatasetIT land on
// `DatasetCustomMethodPOST` (the `/datasets/{id}:undelete` dispatcher)
// and get a 405 against a request that was logically a PATCH.
// Mounting the override at the middleware layer fixes the entire
// gateway surface with one rewrite point instead of teaching every
// `*CustomMethodPOST` handler to also accept ACL bodies.
//
// Behavior:
//   - Header absent → pass through unchanged. The middleware never
//     allocates anything for the common case.
//   - Header set + request method is POST + override is one of
//     PATCH/PUT/DELETE (case-insensitive) → rewrite `r.Method` to the
//     uppercase override and continue. Mux dispatch then routes the
//     request to the genuine PATCH/PUT/DELETE handler.
//   - Header set + request method is not POST → 400. The
//     google-api-client interceptor only ever sets the header on a
//     POST it just rewrote, so a non-POST + override is either a
//     misconfigured client or an attempt to confuse the dispatcher;
//     either way, refusing is the safe answer.
//   - Header set + override value is anything other than
//     PATCH/PUT/DELETE → 400. We don't honor `GET`/`HEAD`/`OPTIONS`
//     because they are not what the Java client tunnels and we want
//     a tight surface.
//
// Mounting: place the middleware after the access-log middleware so
// the access log records the original POST + override-header pair,
// but before any middleware or handler that routes on `r.Method`.
// In `gateway/server.go::wrapMiddleware` it sits between the
// loopback-tag middleware and the request-log layer; see that
// function for the canonical chain order.
func WithMethodOverride(next http.Handler) http.Handler {
	return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		raw := r.Header.Get(methodOverrideHeader)
		if raw == "" {
			next.ServeHTTP(w, r)
			return
		}
		if r.Method != http.MethodPost {
			writeMethodOverrideError(w,
				"X-HTTP-Method-Override is only valid on POST requests; "+
					"received "+r.Method)
			return
		}
		upper := strings.ToUpper(strings.TrimSpace(raw))
		switch upper {
		case http.MethodPatch, http.MethodPut, http.MethodDelete:
			r.Method = upper
		default:
			writeMethodOverrideError(w,
				"X-HTTP-Method-Override must be one of PATCH, PUT, "+
					"or DELETE; received "+raw)
			return
		}
		next.ServeHTTP(w, r)
	})
}

// writeMethodOverrideError emits a BigQuery-shaped 400 envelope.
// Delegates to the shared [writeJSONError] helper so the envelope
// shape stays in lockstep with [writeGunzipError]; consolidating the
// JSON layout in one place also keeps the goconst linter from
// flagging recurring `"invalid"` / `"message"` literals once a third
// middleware needs to emit a 400.
func writeMethodOverrideError(w http.ResponseWriter, msg string) {
	writeJSONError(w, http.StatusBadRequest, errReasonInvalid, msg)
}
