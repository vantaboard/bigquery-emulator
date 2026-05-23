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

package handlers

import "net/http"

// QueryRun implements `bigquery.jobs.query`, the synchronous query API.
//
// Real implementation should:
//  1. Decode a QueryRequest from the request body.
//  2. Forward it to the engine over gRPC (proto/emulator.proto's Query
//     service) and stream rows back.
//  3. Marshal results into BigQuery's QueryResponse shape (schema, rows,
//     jobReference, totalRows, jobComplete, etc.).
func QueryRun(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) { NotImplemented(w, r) }
}

// QueryGetResults implements `bigquery.jobs.getQueryResults`.
func QueryGetResults(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) { NotImplemented(w, r) }
}
