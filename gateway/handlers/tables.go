package handlers

import (
	"net/http"
	"strconv"

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
	t.LastModifiedTime = nowMillis()
	if t.Labels == nil {
		t.Labels = bqtypes.ResourceLabels{}
	}
	if t.Location == "" {
		t.Location = "US"
	}
	return t
}

// schemaToProto converts a REST TableSchema into the gRPC TableSchema
// the engine accepts. Returns nil when the REST schema is nil so the
// proto's default zero-value gets sent on the wire.
func schemaToProto(s *bqtypes.TableSchema) *enginepb.TableSchema {
	if s == nil {
		return nil
	}
	out := &enginepb.TableSchema{Fields: make([]*enginepb.FieldSchema, 0, len(s.Fields))}
	for i := range s.Fields {
		out.Fields = append(out.Fields, fieldToProto(s.Fields[i]))
	}
	return out
}

// fieldToProto recursively converts a REST TableFieldSchema into the
// gRPC FieldSchema. Nested STRUCT/RECORD fields are walked verbatim.
func fieldToProto(f bqtypes.TableFieldSchema) *enginepb.FieldSchema {
	out := &enginepb.FieldSchema{
		Name:        f.Name,
		Type:        f.Type,
		Mode:        f.Mode,
		Description: f.Description,
	}
	for i := range f.Fields {
		out.Fields = append(out.Fields, fieldToProto(f.Fields[i]))
	}
	return out
}

// schemaFromProto is the inverse of schemaToProto: turns a gRPC
// TableSchema into the REST TableSchema. Returns nil for an absent or
// empty schema so the JSON response omits the field.
func schemaFromProto(s *enginepb.TableSchema) *bqtypes.TableSchema {
	if s == nil || len(s.Fields) == 0 {
		return nil
	}
	out := &bqtypes.TableSchema{Fields: make([]bqtypes.TableFieldSchema, 0, len(s.Fields))}
	for _, f := range s.Fields {
		out.Fields = append(out.Fields, fieldFromProto(f))
	}
	return out
}

func fieldFromProto(f *enginepb.FieldSchema) bqtypes.TableFieldSchema {
	out := bqtypes.TableFieldSchema{
		Name:        f.GetName(),
		Type:        f.GetType(),
		Mode:        f.GetMode(),
		Description: f.GetDescription(),
	}
	for _, sub := range f.GetFields() {
		out.Fields = append(out.Fields, fieldFromProto(sub))
	}
	return out
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
			labels := bqtypes.ResourceLabels{}
			if overlay, ok := deps.Metadata.GetTable(
				ref.GetProjectId(), ref.GetDatasetId(), ref.GetTableId(),
			); ok && overlay.Labels != nil {
				labels = overlay.Labels
			}
			tableType := defaultTableType
			if overlay, ok := deps.Metadata.GetTable(
				ref.GetProjectId(), ref.GetDatasetId(), ref.GetTableId(),
			); ok && overlay.Type != "" {
				tableType = overlay.Type
			}
			items = append(items, map[string]any{
				"kind": tableKind,
				"id": ref.GetProjectId() + ":" + ref.GetDatasetId() +
					"." + ref.GetTableId(),
				"tableReference": bqtypes.TableReference{
					ProjectID: ref.GetProjectId(),
					DatasetID: ref.GetDatasetId(),
					TableID:   ref.GetTableId(),
				},
				"type":   tableType,
				"labels": labels,
			})
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
		if t.ExternalDataConfiguration != nil {
			if !insertExternalTable(w, r, deps, projectID, datasetID, tableID, &t) {
				return
			}
		} else if _, err := deps.Catalog.RegisterTable(r.Context(), &enginepb.RegisterTableRequest{
			Table: &enginepb.TableRef{
				ProjectId: projectID,
				DatasetId: datasetID,
				TableId:   tableID,
			},
			Schema: schemaToProto(t.Schema),
		}); grpcToHTTPError(w, err) {
			return
		}
		deps.Metadata.PutTable(projectID, datasetID, tableID, t)
		created := nowMillis()
		if deps.Snapshots != nil {
			if ms, parseErr := strconv.ParseInt(created, 10, 64); parseErr == nil {
				deps.Snapshots.RecordCreation(projectID, datasetID, tableID, ms)
			}
		}
		writeJSON(w, http.StatusOK, tableResource(projectID, datasetID, tableID, t))
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
		if grpcToHTTPError(w, err) {
			return
		}
		t := bqtypes.Table{Schema: schemaFromProto(resp.GetSchema())}
		if overlay, ok := deps.Metadata.GetDataset(projectID, datasetID); ok && overlay.Location != "" {
			t.Location = overlay.Location
		}
		if overlay, ok := deps.Metadata.GetTable(projectID, datasetID, tableID); ok {
			t = applyTableMetadataOverlay(t, overlay)
		}
		if deps.Snapshots != nil {
			if ct, ok := deps.Snapshots.CreationTimeMs(projectID, datasetID, tableID); ok {
				t.CreationTime = strconv.FormatInt(ct, 10)
			}
		}
		if rowsResp, listErr := deps.Catalog.ListRows(r.Context(), &enginepb.ListRowsRequest{
			Table: &enginepb.TableRef{
				ProjectId: projectID,
				DatasetId: datasetID,
				TableId:   tableID,
			},
			StartIndex: 0,
			MaxResults: 0,
		}); listErr == nil {
			t.NumRows = strconv.FormatInt(rowsResp.GetTotalRows(), 10)
		} else if t.NumRows == "" {
			t.NumRows = "0"
		}
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
		if overlay, ok := deps.Metadata.GetTable(projectID, datasetID, tableID); ok {
			t = applyTableMetadataOverlay(t, overlay)
		}
		writeJSON(w, http.StatusOK, tableResource(projectID, datasetID, tableID, t))
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

// TableGetIamPolicy implements `bigquery.tables.getIamPolicy`:
//
//	POST /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}:getIamPolicy
//
// Reached via TableCustomMethodPOST after parsing the trailing :op.
func TableGetIamPolicy(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) { NotImplemented(w, r) }
}

// TableSetIamPolicy implements `bigquery.tables.setIamPolicy`:
//
//	POST /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}:setIamPolicy
//
// Reached via TableCustomMethodPOST after parsing the trailing :op.
func TableSetIamPolicy(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) { NotImplemented(w, r) }
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
