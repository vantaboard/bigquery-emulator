package handlers

import "net/http"

// JobList implements `bigquery.jobs.list`.
func JobList(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) { NotImplemented(w, r) }
}

// JobInsert implements `bigquery.jobs.insert` (the asynchronous job
// submission API). Real implementation should accept a Job resource,
// dispatch query/load/extract/copy work to the engine, and return the
// Job back with a generated jobReference.
func JobInsert(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) { NotImplemented(w, r) }
}

// JobGet implements `bigquery.jobs.get`.
func JobGet(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) { NotImplemented(w, r) }
}

// JobCancel implements `bigquery.jobs.cancel`.
func JobCancel(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) { NotImplemented(w, r) }
}
