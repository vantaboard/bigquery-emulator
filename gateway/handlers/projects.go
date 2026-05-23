package handlers

import "net/http"

// ProjectList implements `bigquery.projects.list`:
//
//	GET /bigquery/v2/projects
//
// Returns the projects the caller has any role on. The emulator does not
// model IAM, so this will eventually return a single synthetic project
// matching whatever ID the caller passes through `BIGQUERY_EMULATOR_HOST`-
// based clients.
func ProjectList(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		NotImplemented(w, r)
	}
}

// ProjectGetServiceAccount implements `bigquery.projects.getServiceAccount`:
//
//	GET /bigquery/v2/projects/{projectId}/serviceAccount
//
// Real BigQuery returns the per-project Google-managed service account
// used for KMS interactions. The emulator returns a synthetic email so
// client libraries that hit this endpoint at startup don't fail.
//
// Note: there is no `GET /bigquery/v2/projects/{projectId}` endpoint in
// the public API; this is the endpoint clients actually probe.
func ProjectGetServiceAccount(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		NotImplemented(w, r)
	}
}
