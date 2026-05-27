package handlers

import (
	"net/http"
	"strings"
)

// BigQuery Migration v2alpha REST shell.
//
// The upstream BigQuery Migration API runs at
// `https://bigquerymigration.googleapis.com/v2alpha/...` (and the
// v2 alias at the same host). Client libraries
// (cloud.google.com/go/bigquery/migration/apiv2alpha,
// google-cloud-bigquery-migration for Python/Node/Java) read
// `BIGQUERY_MIGRATION_EMULATOR_HOST` and fall back to
// `BIGQUERY_EMULATOR_HOST` so this gateway can serve both surfaces
// from the same listener.
//
// This shell is intentionally *not* proto-backed: the engine has no
// migration workflow state to drive (no AST translator, no LRO store,
// no subtask catalog), and pulling in
// cloud.google.com/go/bigquery/migration would add a heavyweight
// transitive dependency for a surface no in-tree test exercises yet.
// When workflow execution lands the wire shape stays the same — list
// already returns the documented `migrationWorkflows`/`nextPageToken`
// envelope, get returns the standard 404, create/start return 501.
//
// Routes registered (for both `v2alpha` and `v2`):
//   GET    /{ver}/projects/{projectId}/locations/{location}/workflows
//   POST   /{ver}/projects/{projectId}/locations/{location}/workflows
//   GET    /{ver}/projects/{projectId}/locations/{location}/workflows/{workflowId}
//   DELETE /{ver}/projects/{projectId}/locations/{location}/workflows/{workflowId}
//   POST   /{ver}/projects/{projectId}/locations/{location}/workflows/{workflowId}:start
//          (dispatched on trailing :start via MigrationWorkflowCustomMethodPOST,
//          because net/http's mux can't match `{workflowId}:start` directly.)

// MigrationWorkflowList implements `migration.workflows.list`. Returns
// the BigQuery-shaped empty page so client libraries that probe at
// startup get a structurally-valid response.
func MigrationWorkflowList(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, _ *http.Request) {
		writeJSON(w, http.StatusOK, map[string]any{
			"migrationWorkflows": []any{},
		})
	}
}

// MigrationWorkflowCreate implements `migration.workflows.create`.
// The emulator has no translator yet, so the handler returns 501
// (rather than echoing a fake DRAFT workflow back), keeping the
// contract honest for callers that read the response.
func MigrationWorkflowCreate(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) { NotImplemented(w, r) }
}

// MigrationWorkflowGet implements `migration.workflows.get`. Returns
// 404 with the BigQuery-shaped error envelope: by definition any
// workflowId is unknown because there is no workflow store yet.
func MigrationWorkflowGet(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		writeError(w, http.StatusNotFound, "notFound",
			"Not found: MigrationWorkflow "+migrationWorkflowName(r))
	}
}

// MigrationWorkflowDelete implements `migration.workflows.delete`.
// Returns 404 (no store), matching the Get behavior so a
// list-get-delete loop is consistent.
func MigrationWorkflowDelete(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		writeError(w, http.StatusNotFound, "notFound",
			"Not found: MigrationWorkflow "+migrationWorkflowName(r))
	}
}

// MigrationWorkflowCustomMethodPOST dispatches the AIP-136 ":start"
// custom method that hangs off a workflow resource. Same pattern as
// DatasetCustomMethodPOST: register the parent path and parse the
// trailing `:op` from the captured wildcard. Today the only
// recognized op is `:start`, returned as 501 because there is no LRO
// store yet.
func MigrationWorkflowCustomMethodPOST(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		_, op := splitColonOp(r.PathValue("workflowId"))
		switch op {
		case "start":
			NotImplemented(w, r)
		case "":
			writeError(w, http.StatusMethodNotAllowed, "invalid",
				"POST is not allowed on a workflow resource. "+
					"Use POST .../workflows to create or :start to start.")
		default:
			writeError(w, http.StatusNotFound, "notFound",
				"Unknown migration workflow custom method ':"+op+"'.")
		}
	}
}

// migrationWorkflowName reconstructs the canonical resource name from
// the path captures so the 404 envelopes match upstream error text.
func migrationWorkflowName(r *http.Request) string {
	wid, _ := splitColonOp(r.PathValue("workflowId"))
	return "projects/" + r.PathValue("projectId") +
		"/locations/" + r.PathValue("location") +
		"/workflows/" + strings.TrimSpace(wid)
}
