package handlers

import (
	"context"
	"encoding/json"
	"net/http"
	"strconv"
	"time"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"
)

// rowAccessPolicyListKind is the `kind` field for a
// rowAccessPolicies.list response. See
// docs/bigquery/docs/reference/rest/v2/rowAccessPolicies/list.md.
const rowAccessPolicyListKind = "bigquery#listRowAccessPoliciesResponse"

const rowAccessPolicyKind = "bigquery#rowAccessPolicy"

type rowAccessPolicyReference struct {
	ProjectID string `json:"projectId"`
	DatasetID string `json:"datasetId"`
	TableID   string `json:"tableId"`
	PolicyID  string `json:"policyId"`
}

type rowAccessPolicyWire struct {
	Kind                     string                   `json:"kind,omitempty"`
	Etag                     string                   `json:"etag,omitempty"`
	RowAccessPolicyReference rowAccessPolicyReference `json:"rowAccessPolicyReference"`
	FilterPredicate          string                   `json:"filterPredicate"`
	CreationTime             string                   `json:"creationTime,omitempty"`
	LastModifiedTime         string                   `json:"lastModifiedTime,omitempty"`
	Grantees                 []string                 `json:"grantees,omitempty"`
}

func rowAccessPolicyPathValues(r *http.Request) (projectID, datasetID, tableID, policyID string) {
	return r.PathValue("projectId"), r.PathValue("datasetId"),
		r.PathValue("tableId"), r.PathValue("policyId")
}

func tableRef(projectID, datasetID, tableID string) *enginepb.TableRef {
	return &enginepb.TableRef{
		ProjectId: projectID,
		DatasetId: datasetID,
		TableId:   tableID,
	}
}

func msToTimestampString(ms int64) string {
	if ms <= 0 {
		return ""
	}
	return strconv.FormatInt(ms, 10)
}

func policyToWire(p *enginepb.RowAccessPolicy) rowAccessPolicyWire {
	ref := rowAccessPolicyReference{
		ProjectID: p.GetTable().GetProjectId(),
		DatasetID: p.GetTable().GetDatasetId(),
		TableID:   p.GetTable().GetTableId(),
		PolicyID:  p.GetPolicyId(),
	}
	return rowAccessPolicyWire{
		Kind:                     rowAccessPolicyKind,
		Etag:                     p.GetPolicyId(),
		RowAccessPolicyReference: ref,
		FilterPredicate:          p.GetFilterPredicate(),
		CreationTime:             msToTimestampString(p.GetCreationTimeMs()),
		LastModifiedTime:         msToTimestampString(p.GetLastModifiedTimeMs()),
		Grantees:                 append([]string(nil), p.GetGrantees()...),
	}
}

func decodeRowAccessPolicyBody(r *http.Request) (rowAccessPolicyWire, error) {
	var body rowAccessPolicyWire
	if err := json.NewDecoder(r.Body).Decode(&body); err != nil {
		return rowAccessPolicyWire{}, err
	}
	return body, nil
}

// RowAccessPolicyList implements `bigquery.rowAccessPolicies.list`.
func RowAccessPolicyList(deps Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		projectID, datasetID, tableID, _ := rowAccessPolicyPathValues(r)
		if deps.Catalog == nil {
			writeJSON(w, http.StatusOK, map[string]any{
				resourceKeyKind:     rowAccessPolicyListKind,
				"rowAccessPolicies": []any{},
			})
			return
		}
		resp, err := deps.Catalog.ListRowAccessPolicies(r.Context(),
			&enginepb.ListRowAccessPoliciesRequest{
				Table: tableRef(projectID, datasetID, tableID),
			})
		if err != nil {
			writeGRPCError(w, err)
			return
		}
		policies := make([]rowAccessPolicyWire, 0, len(resp.GetPolicies()))
		for _, p := range resp.GetPolicies() {
			policies = append(policies, policyToWire(p))
		}
		writeJSON(w, http.StatusOK, map[string]any{
			resourceKeyKind:     rowAccessPolicyListKind,
			"rowAccessPolicies": policies,
		})
	}
}

// RowAccessPolicyInsert implements `bigquery.rowAccessPolicies.insert`.
func RowAccessPolicyInsert(deps Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		projectID, datasetID, tableID, _ := rowAccessPolicyPathValues(r)
		if deps.Catalog == nil {
			NotImplemented(w, r)
			return
		}
		body, err := decodeRowAccessPolicyBody(r)
		if err != nil {
			writeError(w, http.StatusBadRequest, reasonInvalid, err.Error())
			return
		}
		policyID := body.RowAccessPolicyReference.PolicyID
		if policyID == "" {
			policyID = r.URL.Query().Get("policyId")
		}
		if policyID == "" {
			writeError(w, http.StatusBadRequest, reasonInvalid, "policyId is required")
			return
		}
		now := time.Now().UnixMilli()
		resp, err := deps.Catalog.UpsertRowAccessPolicy(r.Context(),
			&enginepb.UpsertRowAccessPolicyRequest{
				Policy: &enginepb.RowAccessPolicy{
					Table:              tableRef(projectID, datasetID, tableID),
					PolicyId:           policyID,
					FilterPredicate:    body.FilterPredicate,
					Grantees:           body.Grantees,
					CreationTimeMs:     now,
					LastModifiedTimeMs: now,
				},
			})
		if err != nil {
			writeGRPCError(w, err)
			return
		}
		writeJSON(w, http.StatusOK, policyToWire(resp.GetPolicy()))
	}
}

// RowAccessPolicyGet implements `bigquery.rowAccessPolicies.get`.
func RowAccessPolicyGet(deps Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		projectID, datasetID, tableID, policyID := rowAccessPolicyPathValues(r)
		if deps.Catalog == nil {
			NotFound(w, r)
			return
		}
		resp, err := deps.Catalog.ListRowAccessPolicies(r.Context(),
			&enginepb.ListRowAccessPoliciesRequest{
				Table: tableRef(projectID, datasetID, tableID),
			})
		if err != nil {
			writeGRPCError(w, err)
			return
		}
		for _, p := range resp.GetPolicies() {
			if p.GetPolicyId() == policyID {
				writeJSON(w, http.StatusOK, policyToWire(p))
				return
			}
		}
		NotFound(w, r)
	}
}

// RowAccessPolicyUpdate implements `bigquery.rowAccessPolicies.update`.
func RowAccessPolicyUpdate(deps Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		projectID, datasetID, tableID, policyID := rowAccessPolicyPathValues(r)
		if deps.Catalog == nil {
			NotImplemented(w, r)
			return
		}
		body, err := decodeRowAccessPolicyBody(r)
		if err != nil {
			writeError(w, http.StatusBadRequest, reasonInvalid, err.Error())
			return
		}
		now := time.Now().UnixMilli()
		resp, err := deps.Catalog.UpsertRowAccessPolicy(r.Context(),
			&enginepb.UpsertRowAccessPolicyRequest{
				Policy: &enginepb.RowAccessPolicy{
					Table:              tableRef(projectID, datasetID, tableID),
					PolicyId:           policyID,
					FilterPredicate:    body.FilterPredicate,
					Grantees:           body.Grantees,
					LastModifiedTimeMs: now,
				},
			})
		if err != nil {
			writeGRPCError(w, err)
			return
		}
		writeJSON(w, http.StatusOK, policyToWire(resp.GetPolicy()))
	}
}

// RowAccessPolicyDelete implements `bigquery.rowAccessPolicies.delete`.
func RowAccessPolicyDelete(deps Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		projectID, datasetID, tableID, policyID := rowAccessPolicyPathValues(r)
		if deps.Catalog == nil {
			NotImplemented(w, r)
			return
		}
		_, err := deps.Catalog.DeleteRowAccessPolicy(r.Context(),
			&enginepb.DeleteRowAccessPolicyRequest{
				Table:    tableRef(projectID, datasetID, tableID),
				PolicyId: policyID,
			})
		if err != nil {
			writeGRPCError(w, err)
			return
		}
		w.WriteHeader(http.StatusOK)
	}
}

// RowAccessPolicyDispatch routes table-scoped rowAccessPolicies methods.
func RowAccessPolicyDispatch(deps Dependencies) http.HandlerFunc {
	list := RowAccessPolicyList(deps)
	insert := RowAccessPolicyInsert(deps)
	get := RowAccessPolicyGet(deps)
	update := RowAccessPolicyUpdate(deps)
	del := RowAccessPolicyDelete(deps)
	iam := RowAccessPolicyIamPolicy(deps)
	return func(w http.ResponseWriter, r *http.Request) {
		policyID := r.PathValue("policyId")
		if policyID != "" {
			if r.Method == http.MethodGet {
				get(w, r)
				return
			}
			if r.Method == http.MethodPut || r.Method == http.MethodPatch {
				update(w, r)
				return
			}
			if r.Method == http.MethodDelete {
				del(w, r)
				return
			}
			if r.Method == http.MethodPost {
				iam(w, r)
				return
			}
			writeError(w, http.StatusMethodNotAllowed, reasonInvalid,
				"HTTP method not supported for this rowAccessPolicies endpoint")
			return
		}
		switch r.Method {
		case http.MethodGet:
			list(w, r)
		case http.MethodPost:
			insert(w, r)
		default:
			writeError(w, http.StatusMethodNotAllowed, reasonInvalid,
				"HTTP method not supported for this rowAccessPolicies endpoint")
		}
	}
}

func writeGRPCError(w http.ResponseWriter, err error) {
	st, ok := status.FromError(err)
	if !ok {
		writeError(w, http.StatusInternalServerError, reasonInternalError, err.Error())
		return
	}
	switch st.Code() {
	case codes.NotFound:
		writeError(w, http.StatusNotFound, reasonNotFound, st.Message())
	case codes.InvalidArgument:
		writeError(w, http.StatusBadRequest, reasonInvalid, st.Message())
	case codes.PermissionDenied:
		writeError(w, http.StatusForbidden, reasonAccessDenied, st.Message())
	case codes.Unimplemented:
		NotImplemented(w, nil)
	default:
		writeError(w, http.StatusInternalServerError, reasonInternalError, st.Message())
	}
}

// SyncColumnGovernanceFromSchema persists policy tags from a REST schema
// patch into the engine catalog for query-time masking.
func SyncColumnGovernanceFromSchema(
	ctx context.Context,
	deps Dependencies,
	projectID, datasetID, tableID string,
	schema *bqtypes.TableSchema,
) {
	if deps.Catalog == nil || schema == nil {
		return
	}
	for _, field := range schema.Fields {
		maskKind := field.MaskKind
		if maskKind == "" && field.PolicyTags != nil && len(field.PolicyTags.Names) > 0 {
			maskKind = "SHA256"
		}
		if maskKind == "" {
			continue
		}
		col := &enginepb.ColumnGovernance{
			ColumnName: field.Name,
			MaskKind:   maskKind,
		}
		if field.PolicyTags != nil {
			col.PolicyTags = append([]string(nil), field.PolicyTags.Names...)
		}
		_, _ = deps.Catalog.SetColumnGovernance(ctx, &enginepb.SetColumnGovernanceRequest{
			Table:  tableRef(projectID, datasetID, tableID),
			Column: col,
		})
	}
}

func RowAccessPolicyIamPolicy(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) { NotImplemented(w, r) }
}
