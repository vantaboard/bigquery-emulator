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
	"encoding/json"
	"net/http"
)

// jsonContentType is the Content-Type header value middleware emits on
// JSON error envelopes. Pinned here so a future tweak (utf-16, an
// http/2 specifier, etc.) lands in exactly one place.
const jsonContentType = "application/json; charset=utf-8"

// errReasonInvalid is the BigQuery-shaped error `status` / `errors[].reason`
// value used for 400 responses the gateway-layer middlewares emit
// (gunzip body invalid, method override misuse). The handlers package
// uses the same literal in `handlers.writeError` calls; we keep a
// local copy here because middleware -> handlers is a forbidden import
// direction (handlers depends on middleware for auth context).
const errReasonInvalid = "invalid"

// fieldKeyMessage is the JSON field key used inside the BigQuery
// `errorBody.errors[].message` envelope. Kept as a named const so the
// recurring use across [writeGunzipError] and [writeMethodOverrideError]
// stays goconst-clean.
const fieldKeyMessage = "message"

// writeJSONError emits a BigQuery-shaped JSON error envelope at
// `status` with `reason` and `msg`. Mirrors `handlers.writeError`
// byte-for-byte but keeps middleware free of a handlers import (which
// would close an import cycle — handlers depends on middleware for
// the auth-context lookup).
func writeJSONError(w http.ResponseWriter, status int, reason, msg string) {
	body := map[string]any{
		"error": map[string]any{
			"code":          status,
			fieldKeyMessage: msg,
			"status":        reason,
			"errors": []map[string]any{{
				"reason":        reason,
				fieldKeyMessage: msg,
				"domain":        "global",
			}},
		},
	}
	w.Header().Set("Content-Type", jsonContentType)
	w.WriteHeader(status)
	_ = json.NewEncoder(w).Encode(body)
}
