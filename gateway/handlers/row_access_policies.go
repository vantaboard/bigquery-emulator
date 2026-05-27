package handlers

import (
	"net/http"
)

// rowAccessPolicyListKind is the `kind` field for a
// rowAccessPolicies.list response. See
// docs/bigquery/docs/reference/rest/v2/rowAccessPolicies/list.md.
const rowAccessPolicyListKind = "bigquery#listRowAccessPoliciesResponse"

// RowAccessPolicyList implements `bigquery.rowAccessPolicies.list`:
//
//	GET /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}/rowAccessPolicies
//
// Row-level access policies have no engine backing today (they require
// a policy store + per-query rewrite at analysis time), so the handler
// returns the BigQuery-shaped empty page. Client libraries that probe
// this endpoint at startup get a structurally-valid response instead
// of a 404 from the catch-all `NotFound` handler.
func RowAccessPolicyList(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, _ *http.Request) {
		writeJSON(w, http.StatusOK, map[string]any{
			"kind":              rowAccessPolicyListKind,
			"rowAccessPolicies": []any{},
		})
	}
}

// RowAccessPolicyIamPolicy backs the AIP-136 custom methods that hang
// off a row-access-policy resource:
//
//	POST /bigquery/v2/projects/{projectId}/.../rowAccessPolicies/{policyId}:getIamPolicy
//	POST /bigquery/v2/projects/{projectId}/.../rowAccessPolicies/{policyId}:setIamPolicy
//	POST /bigquery/v2/projects/{projectId}/.../rowAccessPolicies/{policyId}:testIamPermissions
//
// All three currently return 501; promoting to a real IAM store would
// land alongside the row-access-policy engine work. Wired as a single
// dispatcher so the discovery doc can advertise all three IDs without
// triplicating routes.
func RowAccessPolicyIamPolicy(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) { NotImplemented(w, r) }
}
