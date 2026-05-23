// Copyright 2026 BigQuery Emulator Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Package handlers contains HTTP handlers for the BigQuery REST surface.
//
// At this stage of the project most handlers are intentional stubs that
// return http.StatusNotImplemented. They exist so that:
//
//   - The route table in gateway/server.go is exhaustive and easy to scan,
//     which doubles as a checklist for ROADMAP.md Phase 1.
//   - Client libraries get a structurally-valid BigQuery error envelope
//     instead of a 404 when they hit something we have not implemented yet.
//   - Each handler can be flipped to a real implementation in isolation.
package handlers

import (
	"encoding/json"
	"net/http"
)

// Dependencies bundles everything a handler might need to reach (engine
// gRPC client, in-memory catalog, logger, etc.). For now it is empty and
// will grow as Phase 2+ wires in real backends.
type Dependencies struct{}

// Health is a trivial liveness endpoint useful for `docker-compose`
// health checks and CI smoke tests.
func Health(w http.ResponseWriter, r *http.Request) {
	writeJSON(w, http.StatusOK, map[string]string{
		"status":  "ok",
		"service": "bigquery-emulator",
	})
}

// NotImplemented returns a BigQuery-shaped 501 response. Used by routes
// that are registered but not yet implemented.
func NotImplemented(w http.ResponseWriter, r *http.Request) {
	writeError(w, http.StatusNotImplemented, "notImplemented",
		"This BigQuery emulator route is registered but not yet implemented. "+
			"See ROADMAP.md.")
}

// NotFound is the catch-all handler for paths not in the route table. It
// returns a BigQuery-shaped 404 so client libraries see a structured error.
func NotFound(w http.ResponseWriter, r *http.Request) {
	writeError(w, http.StatusNotFound, "notFound",
		"No route matches "+r.Method+" "+r.URL.Path+".")
}

// errorEnvelope matches the shape BigQuery returns for non-2xx responses.
// See https://cloud.google.com/bigquery/docs/reference/rest -> error format.
type errorEnvelope struct {
	Error errorBody `json:"error"`
}

type errorBody struct {
	Code    int           `json:"code"`
	Message string        `json:"message"`
	Errors  []errorDetail `json:"errors,omitempty"`
	Status  string        `json:"status,omitempty"`
}

type errorDetail struct {
	Reason  string `json:"reason"`
	Message string `json:"message"`
	Domain  string `json:"domain,omitempty"`
}

func writeJSON(w http.ResponseWriter, status int, body any) {
	w.Header().Set("Content-Type", "application/json; charset=utf-8")
	w.WriteHeader(status)
	_ = json.NewEncoder(w).Encode(body)
}

func writeError(w http.ResponseWriter, status int, reason, msg string) {
	writeJSON(w, status, errorEnvelope{
		Error: errorBody{
			Code:    status,
			Message: msg,
			Status:  reason,
			Errors: []errorDetail{{
				Reason:  reason,
				Message: msg,
				Domain:  "global",
			}},
		},
	})
}
