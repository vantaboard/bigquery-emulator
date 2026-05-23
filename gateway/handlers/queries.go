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
