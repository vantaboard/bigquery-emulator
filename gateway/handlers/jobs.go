package handlers

import "net/http"

// JobList implements `bigquery.jobs.list`:
//
//	GET /bigquery/v2/projects/{projectId}/jobs
//
// Supports the documented query parameters `allUsers`, `maxResults`,
// `minCreationTime`, `maxCreationTime`, `pageToken`, `projection`,
// `stateFilter`, and `parentJobId`.
func JobList(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) { NotImplemented(w, r) }
}

// JobInsert implements `bigquery.jobs.insert` (metadata-only variant):
//
//	POST /bigquery/v2/projects/{projectId}/jobs
//
// The body is a Job resource (configurationQuery / configurationLoad /
// configurationCopy / configurationExtract). For load jobs that send
// data inline, clients use the upload variant routed to
// JobInsertUpload.
func JobInsert(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) { NotImplemented(w, r) }
}

// JobInsertUpload implements `bigquery.jobs.insert` (media-upload variant):
//
//	POST /upload/bigquery/v2/projects/{projectId}/jobs
//
// Selected via `?uploadType=multipart` or `?uploadType=resumable`. The
// emulator must accept both because the official client libraries pick
// one based on payload size. Multipart bodies are `multipart/related`
// with a JSON Job metadata part and a data part; resumable uploads
// initiate a session and stream chunks per RFC 7233.
//
// See docs/bigquery/docs/reference/api-uploads.md for the wire format.
func JobInsertUpload(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) { NotImplemented(w, r) }
}

// JobGet implements `bigquery.jobs.get`:
//
//	GET /bigquery/v2/projects/{projectId}/jobs/{jobId}
func JobGet(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) { NotImplemented(w, r) }
}

// JobCancel implements `bigquery.jobs.cancel`:
//
//	POST /bigquery/v2/projects/{projectId}/jobs/{jobId}/cancel
//
// Returns immediately; the client polls `jobs.get` for the final state.
func JobCancel(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) { NotImplemented(w, r) }
}

// JobDelete implements `bigquery.jobs.delete`:
//
//	DELETE /bigquery/v2/projects/{projectId}/jobs/{jobId}/delete
//
// The literal "/delete" suffix is the upstream URL template, not a typo
// (see docs/bigquery/docs/reference/rest/v2/jobs/delete.md). Deletes job
// metadata; if {jobId} is a parent job, child job metadata is also
// deleted.
func JobDelete(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) { NotImplemented(w, r) }
}
