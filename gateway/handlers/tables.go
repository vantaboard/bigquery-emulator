package handlers

import (
	"encoding/json"
	"errors"
	"fmt"
	"net/http"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
)

// tableKind is the value the BigQuery REST API returns for the `kind`
// field of a Table resource. See
// docs/bigquery/docs/reference/rest/v2/tables/get.md.
const tableKind = "bigquery#table"

// tableListKind is the `kind` field for a TableList response.
const tableListKind = "bigquery#tableList"

// defaultTableType is the value of the Table.type field for the
// non-view, non-external tables the emulator's Catalog tracks today.
const defaultTableType = "TABLE"

// viewTableType is the BigQuery REST type string for views created
// via tables.insert with a view definition.
const viewTableType = "VIEW"

// materializedViewTableType is the BigQuery REST type string for
// materialized views created via tables.insert with a materializedView
// definition (see QueryMaterializedViewIT).
const materializedViewTableType = "MATERIALIZED_VIEW"

// externalTableType is the BigQuery REST type string for GCS-backed
// external tables (tables.insert with externalDataConfiguration).
const externalTableType = "EXTERNAL"

// snapshotTableType is the BigQuery REST type string for table snapshots
// created via configuration.copy jobs with operationType=SNAPSHOT.
const snapshotTableType = "SNAPSHOT"

// tableIDFromPath returns the {projectId}/{datasetId}/{tableId}
// triple captured by the route pattern. It strips any AIP-136 custom-
// method suffix (e.g. ":getIamPolicy") from the tableId so the same
// helper can be reused by TableCustomMethodPOST.
func tableIDFromPath(r *http.Request) (projectID, datasetID, tableID string) {
	projectID = r.PathValue("projectId")
	datasetID = r.PathValue("datasetId")
	tableID, _ = splitColonOp(r.PathValue("tableId"))
	return projectID, datasetID, tableID
}

// tableResource builds a Table resource for a successful response.
// Preserves any caller-supplied Schema/FriendlyName/Description that
// the engine does not need to know about, and stamps the bookkeeping
// fields (Kind, ID, Type, timestamps) the REST client expects.
//
// Labels is materialized to an empty map when nil so the upstream
// `getTableLabels` sample's `Object.entries(table.metadata.labels)`
// call returns an empty iterator instead of erroring with
// `TypeError: Cannot convert undefined or null to object`. The
// bqtypes.Table.Labels tag omits `omitempty` so the empty map
// round-trips as `"labels":{}` on the wire. Mirrors datasetResource.
func tableResource(projectID, datasetID, tableID string, t bqtypes.Table) bqtypes.Table {
	t.Kind = tableKind
	t.ID = projectID + ":" + datasetID + "." + tableID
	t.TableReference = bqtypes.TableReference{
		ProjectID: projectID,
		DatasetID: datasetID,
		TableID:   tableID,
	}
	if t.Type == "" {
		t.Type = defaultTableType
	}
	if t.CreationTime == "" {
		t.CreationTime = nowMillis()
	}
	if t.LastModifiedTime == "" {
		t.LastModifiedTime = t.CreationTime
	}
	if t.Labels == nil {
		t.Labels = bqtypes.ResourceLabels{}
	}
	if t.Location == "" {
		t.Location = "US"
	}
	applyTableStorageStats(&t)
	return t
}

// TableList implements `bigquery.tables.list`:
//
//	GET /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables
//
// Calls the Catalog.ListTables RPC and folds the (deterministically
// ordered, ascending table_id) result into a BigQuery tableList
// envelope. Mirrors DatasetList's pagination posture: no
// `nextPageToken` today, every entry in one page.
//
// Per-entry shape matches upstream's tableList item: kind, id
// (projectId:datasetId.tableId), tableReference, type (defaulting to
// "TABLE"), and an empty labels object so node samples that call
// `Object.entries(item.metadata.labels)` on each iteration item do
// not raise.
func TableList(deps Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		projectID := r.PathValue("projectId")
		datasetID := r.PathValue("datasetId")
		if deps.Catalog == nil {
			writeJSON(w, http.StatusOK, map[string]any{
				resourceKeyKind:       tableListKind,
				resourceKeyTables:     []bqtypes.Table{},
				resourceKeyTotalItems: 0,
			})
			return
		}
		resp, err := deps.Catalog.ListTables(r.Context(), &enginepb.ListTablesRequest{
			Dataset: &enginepb.DatasetRef{
				ProjectId: projectID,
				DatasetId: datasetID,
			},
		})
		if grpcToHTTPError(w, err) {
			return
		}
		items := make([]map[string]any, 0, len(resp.GetTables()))
		for _, ref := range resp.GetTables() {
			items = append(items, tableListItem(r.Context(), deps, ref))
		}
		writeJSON(w, http.StatusOK, map[string]any{
			resourceKeyKind:       tableListKind,
			resourceKeyTables:     items,
			resourceKeyTotalItems: len(items),
		})
	}
}

// TableInsert implements `bigquery.tables.insert`:
//
//	POST /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables
//
// Decodes the Table body, forwards the (TableRef, schema) pair to
// Catalog.RegisterTable, and returns the new Table resource on
// success. tableReference.tableId in the body is required.
func TableInsert(deps Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		projectID := r.PathValue("projectId")
		datasetID := r.PathValue("datasetId")
		t, ok := decodeTableBody(w, r)
		if !ok {
			return
		}
		tableID := t.TableReference.TableID
		if tableID == "" {
			writeError(w, http.StatusBadRequest, "invalid",
				"tableReference.tableId is required")
			return
		}
		if deps.Catalog == nil {
			NotImplemented(w, r)
			return
		}
		if !populateMaterializedViewSchema(w, deps, r, projectID, &t) {
			return
		}
		if !populateViewSchema(w, deps, r, projectID, &t) {
			return
		}
		if !registerInsertedTable(w, r, deps, projectID, datasetID, tableID, &t) {
			return
		}
		writeInsertedTableResponse(w, deps, r, projectID, datasetID, tableID, t)
	}
}

// populateMaterializedViewSchema fills Type and Schema on REST MV inserts
// when the client omits schema. Dry-running the MV query lets SELECT *
// expand to analyzed columns instead of zero. Returns false when the
// handler already wrote an error response.
func populateMaterializedViewSchema(
	w http.ResponseWriter,
	deps Dependencies,
	r *http.Request,
	projectID string,
	t *bqtypes.Table,
) bool {
	if t.MaterializedView == nil || t.MaterializedView.Query == "" {
		return true
	}
	if t.Type == "" {
		t.Type = materializedViewTableType
	}
	if t.Schema != nil && len(t.Schema.Fields) > 0 {
		return true
	}
	inferred, inferErr := inferTableSchemaFromQuery(
		deps, r, projectID, t.MaterializedView.Query)
	if inferErr != nil {
		if queryGRPCToHTTPError(w, inferErr) {
			return false
		}
		writeError(w, http.StatusInternalServerError, reasonInternalError,
			"Could not infer materialized view schema: "+inferErr.Error())
		return false
	}
	if inferred != nil {
		t.Schema = inferred
	}
	return true
}

// populateViewSchema fills Type and Schema on REST view inserts when
// the client omits schema. Dry-running the view query lets SELECT *
// expand to analyzed columns instead of zero.
func populateViewSchema(
	w http.ResponseWriter,
	deps Dependencies,
	r *http.Request,
	projectID string,
	t *bqtypes.Table,
) bool {
	if t.View == nil || t.View.Query == "" {
		return true
	}
	if t.Type == "" {
		t.Type = viewTableType
	}
	if t.Schema != nil && len(t.Schema.Fields) > 0 {
		return true
	}
	inferred, inferErr := inferTableSchemaFromQuery(deps, r, projectID, t.View.Query)
	if inferErr != nil {
		if queryGRPCToHTTPError(w, inferErr) {
			return false
		}
		writeError(w, http.StatusInternalServerError, reasonInternalError,
			"Could not infer view schema: "+inferErr.Error())
		return false
	}
	if inferred != nil {
		t.Schema = inferred
	}
	return true
}

// insertLogicalView registers a REST-created logical view in the
// engine by issuing a `CREATE OR REPLACE VIEW` statement — the same
// path `CREATE VIEW` DDL takes. That lands the view in the engine's
// view registry so a later `SELECT ... FROM <view>` has its stored
// definition inlined at analyze time and returns the base rows. The
// alternative (registering an empty backing table) shadows the view in
// the engine catalog and makes reads return nothing.
//
// Each name component is backtick-quoted independently so project IDs
// with hyphens (and other names that are not bare identifiers) resolve
// as a three-part `project.dataset.view` path rather than a single
// dotted identifier. Returns false (after writing an HTTP error) when
// registration fails.
func insertLogicalView(
	w http.ResponseWriter,
	r *http.Request,
	deps Dependencies,
	projectID, datasetID, tableID, viewQuery string,
) bool {
	if deps.Query == nil {
		writeError(w, http.StatusNotImplemented, reasonInternalError,
			"engine query client is not configured for view registration")
		return false
	}
	ddl := fmt.Sprintf("CREATE OR REPLACE VIEW `%s`.`%s`.`%s` AS\n%s",
		projectID, datasetID, tableID, viewQuery)
	stream, err := deps.Query.ExecuteQuery(r.Context(), &enginepb.QueryRequest{
		ProjectId: projectID,
		Sql:       ddl,
	})
	if err == nil && stream == nil {
		err = errors.New("engine returned no result stream for view registration")
	}
	if err == nil {
		_, _, _, _, _, _, err = drainSyncStream(stream)
	}
	if err != nil {
		if queryGRPCToHTTPError(w, err) {
			return false
		}
		writeError(w, http.StatusInternalServerError, reasonInternalError,
			"Could not register view: "+err.Error())
		return false
	}
	return true
}

// inferTableSchemaFromQuery runs the MV definition query through the
// engine DryRun RPC and returns the analyzed output schema as REST
// TableSchema. Returns (nil, nil) when Query client is nil or sql is
// empty so callers can still register a schema-less table.
func inferTableSchemaFromQuery(deps Dependencies, r *http.Request,
	projectID, sql string,
) (*bqtypes.TableSchema, error) {
	if deps.Query == nil || sql == "" {
		return nil, nil
	}
	resp, err := deps.Query.DryRun(r.Context(), &enginepb.QueryRequest{
		ProjectId: projectID,
		Sql:       sql,
	})
	if err != nil {
		return nil, err
	}
	return schemaFromProto(resp.GetSchema()), nil
}

// tableFromDescribeResponse maps a Catalog.DescribeTable RPC payload
// into the REST Table shape, including logical-view metadata when the
// engine resolved the target from the view registry.
func tableFromDescribeResponse(resp *enginepb.DescribeTableResponse) bqtypes.Table {
	t := bqtypes.Table{Schema: normalizeRESTTableSchema(schemaFromProto(resp.GetSchema()))}
	if tableType := resp.GetTableType(); tableType != "" {
		t.Type = tableType
	}
	if viewQuery := resp.GetViewQuery(); viewQuery != "" {
		t.View = &bqtypes.ViewDefinition{
			Query:        viewQuery,
			UseLegacySQL: resp.GetViewUseLegacySql(),
		}
	}
	return t
}

// TableGet implements `bigquery.tables.get`:
//
//	GET /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}
//
// Resolves the table via Catalog.DescribeTable so a missing table
// surfaces as 404. The response composites the (Kind, TableReference,
// schema) into a Table resource; other metadata is left empty until
// Storage tracks it.
func TableGet(deps Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		projectID, datasetID, tableID := tableIDFromPath(r)
		if deps.Catalog == nil {
			writeJSON(w, http.StatusOK, tableResource(projectID, datasetID, tableID, bqtypes.Table{}))
			return
		}
		resp, err := deps.Catalog.DescribeTable(r.Context(), &enginepb.DescribeTableRequest{
			Table: &enginepb.TableRef{
				ProjectId: projectID,
				DatasetId: datasetID,
				TableId:   tableID,
			},
		})
		if err != nil {
			// A logical view has no backing storage table, so the
			// engine's DescribeTable returns NotFound. Serve it from
			// the REST metadata overlay recorded at tables.insert
			// instead of 404 so a `create_table(view)` + `get_table`
			// round-trip keeps working (the view rows still come from
			// the query path, which inlines the registered definition).
			if overlay, ok := deps.Metadata.GetTable(projectID, datasetID, tableID); ok &&
				(overlay.View != nil || overlay.MaterializedView != nil) {
				writeJSON(w, http.StatusOK,
					tableResource(projectID, datasetID, tableID, overlay))
				return
			}
			grpcToHTTPError(w, err)
			return
		}
		t := catalogTable(r.Context(), deps, projectID, datasetID, tableID, resp)
		writeJSON(w, http.StatusOK, tableResource(projectID, datasetID, tableID, t))
	}
}

// TableUpdate implements `bigquery.tables.update`:
//
//	PUT /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}
//
// Full replacement of the Table metadata. The engine has no update RPC
// yet, so the handler echoes the request body back as the canonical
// resource (stamping kind/id/timestamps). The REST-only metadata
// fields (labels, expirationTime, rangePartitioning, ...) are also
// stashed in the in-memory MetadataStore so a follow-up GET returns
// the updated values instead of the engine-only schema view.
func TableUpdate(deps Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		projectID, datasetID, tableID := tableIDFromPath(r)
		t, ok := decodeTableBody(w, r)
		if !ok {
			return
		}
		deps.Metadata.PutTable(projectID, datasetID, tableID, t)
		SyncColumnGovernanceFromSchema(r.Context(), deps, projectID, datasetID, tableID, t.Schema)
		writeJSON(w, http.StatusOK, tableResource(projectID, datasetID, tableID, t))
	}
}

// TablePatch implements `bigquery.tables.patch`:
//
//	PATCH /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}
//
// Sparse update; mirrors TableUpdate's metadata-stash posture so
// upstream `setMetadata` + `getMetadata` sequences roundtrip the
// REST-only fields. The engine has no true patch RPC yet.
func TablePatch(deps Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		projectID, datasetID, tableID := tableIDFromPath(r)
		t, ok := decodeTableBody(w, r)
		if !ok {
			return
		}
		deps.Metadata.MergeTable(projectID, datasetID, tableID, t)
		if err := syncPatchedTableSchema(r.Context(), deps, projectID, datasetID, tableID, t.Schema); err != nil {
			writeError(w, http.StatusBadRequest, reasonInvalid, err.Error())
			return
		}
		SyncColumnGovernanceFromSchema(r.Context(), deps, projectID, datasetID, tableID, t.Schema)
		if deps.Catalog == nil {
			out := t
			if merged, ok := deps.Metadata.GetTable(projectID, datasetID, tableID); ok {
				out = merged
			}
			if t.LabelsPatchPresent() && len(out.Labels) == 0 {
				out.SetOmitEmptyLabelsOnWire(true)
			}
			writeJSON(w, http.StatusOK, tableResource(projectID, datasetID, tableID, out))
			return
		}
		tableRef := &enginepb.TableRef{
			ProjectId: projectID,
			DatasetId: datasetID,
			TableId:   tableID,
		}
		desc, err := deps.Catalog.DescribeTable(r.Context(), &enginepb.DescribeTableRequest{Table: tableRef})
		if err != nil {
			grpcToHTTPError(w, err)
			return
		}
		out := catalogTable(r.Context(), deps, projectID, datasetID, tableID, desc)
		if t.LabelsPatchPresent() && len(out.Labels) == 0 {
			out.SetOmitEmptyLabelsOnWire(true)
		}
		writeJSON(w, http.StatusOK, tableResource(projectID, datasetID, tableID, out))
	}
}

// TableDelete implements `bigquery.tables.delete`:
//
//	DELETE /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}
func TableDelete(deps Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		projectID, datasetID, tableID := tableIDFromPath(r)
		if deps.Catalog == nil {
			NotImplemented(w, r)
			return
		}
		if deps.Snapshots != nil {
			_ = deps.Snapshots.CaptureBeforeDelete(r.Context(), deps.Catalog,
				projectID, datasetID, tableID)
		}
		_, err := deps.Catalog.DropTable(r.Context(), &enginepb.DropTableRequest{
			Table: &enginepb.TableRef{
				ProjectId: projectID,
				DatasetId: datasetID,
				TableId:   tableID,
			},
		})
		if grpcToHTTPError(w, err) {
			return
		}
		deps.Metadata.DeleteTable(projectID, datasetID, tableID)
		writeJSON(w, http.StatusOK, struct{}{})
	}
}

// localStubIamPolicyEtag is the deterministic etag returned by the
// emulator's metadata-only table IAM stub (no real ACL store).
const localStubIamPolicyEtag = "BwWWja0YfJA="

func localStubEmptyIamPolicy() map[string]any {
	return map[string]any{
		"version":  1,
		"bindings": []any{},
		"etag":     localStubIamPolicyEtag,
	}
}

// TableGetIamPolicy implements `bigquery.tables.getIamPolicy`:
//
//	POST /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}:getIamPolicy
//
// Reached via TableCustomMethodPOST after parsing the trailing :op.
func TableGetIamPolicy(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, _ *http.Request) {
		writeJSON(w, http.StatusOK, localStubEmptyIamPolicy())
	}
}

// TableSetIamPolicy implements `bigquery.tables.setIamPolicy`:
//
//	POST /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}:setIamPolicy
//
// Reached via TableCustomMethodPOST after parsing the trailing :op.
func TableSetIamPolicy(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		var req struct {
			Policy map[string]any `json:"policy"`
		}
		_ = json.NewDecoder(r.Body).Decode(&req)
		pol := req.Policy
		if pol == nil {
			pol = localStubEmptyIamPolicy()
		} else {
			if _, ok := pol["bindings"]; !ok {
				pol["bindings"] = []any{}
			}
			if _, ok := pol["etag"]; !ok {
				pol["etag"] = localStubIamPolicyEtag
			}
		}
		writeJSON(w, http.StatusOK, pol)
	}
}

// TableTestIamPermissions implements `bigquery.tables.testIamPermissions`:
//
//	POST /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}:testIamPermissions
//
// Reached via TableCustomMethodPOST after parsing the trailing :op.
func TableTestIamPermissions(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) { NotImplemented(w, r) }
}

// TableCustomMethodPOST dispatches the AIP-136 custom-method POST
// endpoints registered against `/tables/{tableId}` -- the three IAM
// helpers BigQuery exposes for table resources.
func TableCustomMethodPOST(deps Dependencies) http.HandlerFunc {
	getPolicy := TableGetIamPolicy(deps)
	setPolicy := TableSetIamPolicy(deps)
	testPerms := TableTestIamPermissions(deps)
	return func(w http.ResponseWriter, r *http.Request) {
		_, op := splitColonOp(r.PathValue("tableId"))
		switch op {
		case "getIamPolicy":
			getPolicy(w, r)
		case "setIamPolicy":
			setPolicy(w, r)
		case "testIamPermissions":
			testPerms(w, r)
		case "":
			writeError(w, http.StatusMethodNotAllowed, "invalid",
				"POST is not allowed on a table resource. "+
					"Use POST /tables to create, /insertAll to stream rows, "+
					"or a documented :op IAM custom method.")
		default:
			writeError(w, http.StatusNotFound, "notFound",
				"Unknown table custom method ':"+op+"'.")
		}
	}
}
