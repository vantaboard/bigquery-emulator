package handlers

import (
	"encoding/json"
	"io"
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

// decodeTableBody reads and unmarshals a Table JSON body. An empty
// body is treated as an empty Table so handlers can choose whether to
// require fields explicitly.
func decodeTableBody(w http.ResponseWriter, r *http.Request) (bqtypes.Table, bool) {
	var t bqtypes.Table
	body, err := io.ReadAll(r.Body)
	if err != nil {
		writeError(w, http.StatusBadRequest, "invalid",
			"Could not read table request body: "+err.Error())
		return t, false
	}
	if len(body) == 0 {
		return t, true
	}
	if err := json.Unmarshal(body, &t); err != nil {
		writeError(w, http.StatusBadRequest, "invalid",
			"Could not parse table request body as JSON: "+err.Error())
		return t, false
	}
	return t, true
}

// tableResource builds a Table resource for a successful response.
// Preserves any caller-supplied Schema/FriendlyName/Description that
// the engine does not need to know about, and stamps the bookkeeping
// fields (Kind, ID, Type, timestamps) the REST client expects.
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
// Same caveat as DatasetList: the engine catalog does not yet expose
// a list RPC, so the handler returns the BigQuery-shaped empty page.
func TableList(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, _ *http.Request) {
		writeJSON(w, http.StatusOK, map[string]any{
			"kind":       tableListKind,
			"tables":     []bqtypes.Table{},
			"totalItems": 0,
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
		_, err := deps.Catalog.RegisterTable(r.Context(), &enginepb.RegisterTableRequest{
			Table: &enginepb.TableRef{
				ProjectId: projectID,
				DatasetId: datasetID,
				TableId:   tableID,
			},
			Schema: schemaToProto(t.Schema),
		})
		if grpcToHTTPError(w, err) {
			return
		}
		writeJSON(w, http.StatusOK, tableResource(projectID, datasetID, tableID, t))
	}
}

// TableGet implements `bigquery.tables.get`:
//
//	GET /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}
//
// Resolves the table via Catalog.DescribeTable so a missing table
// surfaces as 404. The response composites the (Kind, TableReference,
// schema) into a Table resource; other metadata is left empty until
// Storage tracks it (Phase 6).
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
		writeJSON(w, http.StatusOK, tableResource(projectID, datasetID, tableID, t))
	}
}

// TableUpdate implements `bigquery.tables.update`:
//
//	PUT /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}
//
// Full replacement of the Table metadata. The engine has no update RPC
// yet, so the handler echoes the request body back as the canonical
// resource (stamping kind/id/timestamps).
func TableUpdate(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		projectID, datasetID, tableID := tableIDFromPath(r)
		t, ok := decodeTableBody(w, r)
		if !ok {
			return
		}
		writeJSON(w, http.StatusOK, tableResource(projectID, datasetID, tableID, t))
	}
}

// TablePatch implements `bigquery.tables.patch`:
//
//	PATCH /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}
//
// Sparse update; same Phase-3 echo behavior as TableUpdate.
func TablePatch(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		projectID, datasetID, tableID := tableIDFromPath(r)
		t, ok := decodeTableBody(w, r)
		if !ok {
			return
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
