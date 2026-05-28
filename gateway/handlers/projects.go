package handlers

import (
	"net/http"
	"os"
)

// projectKind is the value the BigQuery REST API returns for the
// `kind` field of a Project resource. See
// docs/bigquery/docs/reference/rest/v2/projects/list.md.
const projectKind = "bigquery#project"

// projectListKind is the `kind` field for a ProjectList response. See
// docs/bigquery/docs/reference/rest/v2/projects/list.md.
const projectListKind = "bigquery#projectList"

// serviceAccountKind is the `kind` field for a GetServiceAccountResponse.
// See docs/bigquery/docs/reference/rest/v2/projects/getServiceAccount.md.
const serviceAccountKind = "bigquery#getServiceAccountResponse"

// defaultProjectEnvVar is the env var clients may set to override the
// synthetic project ID returned by projects.list. The conventional
// emulator project ID is `test-project`, matching the Spanner emulator
// and BigQuery client-library samples.
const defaultProjectEnvVar = "BIGQUERY_EMULATOR_PROJECT"

// defaultProjectID is the synthetic project ID returned by
// projects.list when defaultProjectEnvVar is unset. It is the value
// most BigQuery client-library sample code uses against the official
// emulator container, so callers that don't bother to configure a
// project ID still get something predictable on the wire.
const defaultProjectID = "test-project"

// defaultProjectIDFromEnv returns the synthetic project ID used by
// projects.list. It honors BIGQUERY_EMULATOR_PROJECT and falls back
// to defaultProjectID. Lookups happen per-request (cheap) so the env
// var can be flipped without restarting the gateway, which is the
// behavior tests and `task emulator:watch` users expect.
func defaultProjectIDFromEnv() string {
	if v := os.Getenv(defaultProjectEnvVar); v != "" {
		return v
	}
	return defaultProjectID
}

// projectResource is the per-entry shape inside a ProjectList. The
// fields mirror docs/bigquery/docs/reference/rest/v2/projects/list.md.
type projectResource struct {
	Kind             string           `json:"kind"`
	ID               string           `json:"id"`
	NumericID        string           `json:"numericId,omitempty"`
	ProjectReference projectReference `json:"projectReference"`
	FriendlyName     string           `json:"friendlyName,omitempty"`
}

// projectReference is BigQuery's stable handle to a project (mirrors
// the `ProjectReference` resource referenced by projects.list).
type projectReference struct {
	ProjectID string `json:"projectId"`
}

// ProjectList implements `bigquery.projects.list`:
//
//	GET /bigquery/v2/projects
//
// The emulator does not model IAM, so the response is a single
// synthetic project: BIGQUERY_EMULATOR_PROJECT if set, otherwise
// `test-project`. The shape matches
// docs/bigquery/docs/reference/rest/v2/projects/list.md so client
// libraries can iterate without special-casing the emulator.
func ProjectList(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, _ *http.Request) {
		projectID := defaultProjectIDFromEnv()
		writeJSON(w, http.StatusOK, map[string]any{
			resourceKeyKind: projectListKind,
			resourceKeyProjects: []projectResource{{
				Kind: projectKind,
				ID:   projectID,
				ProjectReference: projectReference{
					ProjectID: projectID,
				},
			}},
			"totalItems": 1,
		})
	}
}

// ProjectGetServiceAccount implements `bigquery.projects.getServiceAccount`:
//
//	GET /bigquery/v2/projects/{projectId}/serviceAccount
//
// Real BigQuery returns the per-project Google-managed service account
// used for KMS interactions. The emulator returns a synthetic email so
// client libraries that hit this endpoint at startup don't fail. The
// email is derived from the path's projectId, matching the documented
// format: `bigquery-emulator@<projectId>.iam.gserviceaccount.com`.
//
// Note: there is no `GET /bigquery/v2/projects/{projectId}` endpoint in
// the public API; this is the endpoint clients actually probe.
func ProjectGetServiceAccount(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		projectID := r.PathValue("projectId")
		if projectID == "" {
			projectID = defaultProjectIDFromEnv()
		}
		writeJSON(w, http.StatusOK, map[string]any{
			resourceKeyKind: serviceAccountKind,
			"email":         "bigquery-emulator@" + projectID + ".iam.gserviceaccount.com",
		})
	}
}
