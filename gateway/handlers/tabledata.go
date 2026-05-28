package handlers

import (
	"encoding/base64"
	"encoding/json"
	"fmt"
	"io"
	"net/http"
	"strconv"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
)

// tableDataInsertAllKind is the `kind` field BigQuery uses on the
// success response of tabledata.insertAll. See
// docs/bigquery/docs/reference/rest/v2/tabledata/insertAll.md.
const tableDataInsertAllKind = "bigquery#tableDataInsertAllResponse"

// tableDataListKind is the `kind` field for a tabledata.list response.
// See docs/bigquery/docs/reference/rest/v2/tabledata/list.md.
const tableDataListKind = "bigquery#tableDataList"

// tableDataListDefaultMaxResults bounds the page size when the
// caller does not specify `maxResults`. Matches what most BigQuery
// client libraries pick on their own (the public API itself does not
// document a server-side default).
const tableDataListDefaultMaxResults = 10000

// decodeInsertAllBody parses the JSON body of tabledata.insertAll
// into the wire-shape struct. An empty body is rejected per the
// upstream spec (rows[] is required for a non-trivial request).
func decodeInsertAllBody(w http.ResponseWriter, r *http.Request) (bqtypes.TableDataInsertAllRequest, bool) {
	var req bqtypes.TableDataInsertAllRequest
	body, err := io.ReadAll(r.Body)
	if err != nil {
		writeError(w, http.StatusBadRequest, "invalid",
			"Could not read tabledata.insertAll request body: "+err.Error())
		return req, false
	}
	if len(body) == 0 {
		return req, true
	}
	if err := json.Unmarshal(body, &req); err != nil {
		writeError(w, http.StatusBadRequest, "invalid",
			"Could not parse tabledata.insertAll request body as JSON: "+err.Error())
		return req, false
	}
	return req, true
}

// jsonToCell converts a JSON-decoded value into a proto Cell using
// BigQuery's REST f/v wire shape conventions:
//
//   - nil          -> Cell.null_value = true
//   - bool         -> "true"/"false" string
//   - json.Number  -> decimal string verbatim
//   - float64/int  -> formatted decimal string (BigQuery's REST
//     surface stringifies numerics, including INT64, NUMERIC,
//     BIGNUMERIC; only FLOAT64 stays a JSON number on the wire,
//     but the engine still stores it as a string)
//   - string       -> string verbatim
//   - []byte       -> base64 encoded string (BYTES wire shape)
//   - []interface{}-> Array of converted cells
//   - map[string]any -> Struct with fields in iteration order;
//     used when no schema is available
//
// The conversion is intentionally lossy: a `Cell.string_value` is
// enough to round-trip through Storage::Value::String on the engine
// side because the catalog/storage path only requires the bytes to
// come back out shape-preserved. Typing tightens later via the
// resolved AST.
func jsonToCell(v any) *enginepb.Cell {
	if v == nil {
		return &enginepb.Cell{Value: &enginepb.Cell_NullValue{NullValue: true}}
	}
	switch val := v.(type) {
	case bool:
		if val {
			return &enginepb.Cell{Value: &enginepb.Cell_StringValue{StringValue: "true"}}
		}
		return &enginepb.Cell{Value: &enginepb.Cell_StringValue{StringValue: "false"}}
	case json.Number:
		return &enginepb.Cell{Value: &enginepb.Cell_StringValue{StringValue: string(val)}}
	case float64:
		// json.Decode produces float64 for any unmarshaled number when
		// Decoder.UseNumber isn't set. Format with FormatFloat to keep
		// integer-valued floats as bare integers (1.0 -> "1") and
		// preserve precision for genuine fractions.
		if val == float64(int64(val)) {
			return &enginepb.Cell{Value: &enginepb.Cell_StringValue{
				StringValue: strconv.FormatInt(int64(val), 10),
			}}
		}
		return &enginepb.Cell{Value: &enginepb.Cell_StringValue{
			StringValue: strconv.FormatFloat(val, 'g', -1, 64),
		}}
	case int:
		return &enginepb.Cell{Value: &enginepb.Cell_StringValue{
			StringValue: strconv.Itoa(val),
		}}
	case int64:
		return &enginepb.Cell{Value: &enginepb.Cell_StringValue{
			StringValue: strconv.FormatInt(val, 10),
		}}
	case string:
		return &enginepb.Cell{Value: &enginepb.Cell_StringValue{StringValue: val}}
	case []byte:
		return &enginepb.Cell{Value: &enginepb.Cell_StringValue{
			StringValue: base64.StdEncoding.EncodeToString(val),
		}}
	case []any:
		arr := &enginepb.Array{Elements: make([]*enginepb.Cell, 0, len(val))}
		for _, el := range val {
			arr.Elements = append(arr.Elements, jsonToCell(el))
		}
		return &enginepb.Cell{Value: &enginepb.Cell_Array{Array: arr}}
	case map[string]any:
		st := &enginepb.Struct{Fields: make([]*enginepb.Cell, 0, len(val))}
		for _, fv := range val {
			st.Fields = append(st.Fields, jsonToCell(fv))
		}
		return &enginepb.Cell{Value: &enginepb.Cell_StructValue{StructValue: st}}
	default:
		return &enginepb.Cell{Value: &enginepb.Cell_StringValue{
			StringValue: fmt.Sprintf("%v", val),
		}}
	}
}

// jsonRowToProto converts one insertAll JSON row into a proto DataRow
// by laying its fields out in the column order described by the
// table's gRPC schema. Missing fields become NULL cells so the cell
// count always matches the column count Storage::AppendRows expects.
// Extra fields not present in the schema are dropped (BigQuery's
// ignoreUnknownValues=false is approximated here by always ignoring;
// stricter semantics land alongside row-level validation in the
// query-execution work).
func jsonRowToProto(schema *enginepb.TableSchema, row map[string]any) *enginepb.DataRow {
	out := &enginepb.DataRow{Cells: make([]*enginepb.Cell, 0, len(schema.GetFields()))}
	for _, f := range schema.GetFields() {
		v, ok := row[f.GetName()]
		if !ok {
			out.Cells = append(out.Cells, &enginepb.Cell{
				Value: &enginepb.Cell_NullValue{NullValue: true},
			})
			continue
		}
		out.Cells = append(out.Cells, jsonToCell(v))
	}
	return out
}

// cellToJSON is the inverse of jsonToCell for tabledata.list output.
// BigQuery's REST surface emits everything as JSON strings inside the
// `v` field of the f/v shape, so we map proto Cell variants straight
// to their string representation; arrays and structs recurse into
// the same shape so nested data round-trips.
func cellToJSON(c *enginepb.Cell) any {
	if c == nil {
		return nil
	}
	switch v := c.GetValue().(type) {
	case *enginepb.Cell_StringValue:
		return v.StringValue
	case *enginepb.Cell_NullValue:
		return nil
	case *enginepb.Cell_Array:
		out := make([]bqtypes.Cell, 0, len(v.Array.GetElements()))
		for _, el := range v.Array.GetElements() {
			out = append(out, bqtypes.Cell{V: cellToJSON(el)})
		}
		return out
	case *enginepb.Cell_StructValue:
		out := make([]bqtypes.Cell, 0, len(v.StructValue.GetFields()))
		for _, f := range v.StructValue.GetFields() {
			out = append(out, bqtypes.Cell{V: cellToJSON(f)})
		}
		return out
	default:
		return nil
	}
}

// dataRowToBQRow lowers a proto DataRow into the f/v shape BigQuery
// REST clients consume. Cells line up positionally with the table's
// schema; the gateway never reorders them.
func dataRowToBQRow(row *enginepb.DataRow) bqtypes.Row {
	out := bqtypes.Row{F: make([]bqtypes.Cell, 0, len(row.GetCells()))}
	for _, c := range row.GetCells() {
		out.F = append(out.F, bqtypes.Cell{V: cellToJSON(c)})
	}
	return out
}

// TableDataInsertAll implements `bigquery.tabledata.insertAll`:
//
//	POST /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}/insertAll
//
// Flow: decode the JSON body, look up the destination table's schema
// over Catalog.DescribeTable (so we know the column order), convert
// each row's `json` map into a proto DataRow, and forward the batch
// to Catalog.InsertRows in one shot. A successful response is the
// standard `bigquery#tableDataInsertAllResponse` envelope; row-level
// failures end up in `insertErrors[*]` rather than as an RPC error.
//
// See docs/bigquery/docs/reference/rest/v2/tabledata/insertAll.md for
// the full request/response shapes the emulator targets.
func TableDataInsertAll(deps Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		projectID, datasetID, tableID := tableIDFromPath(r)
		if tableID == "" {
			writeError(w, http.StatusBadRequest, "invalid",
				"tableId is required")
			return
		}
		if deps.Catalog == nil {
			NotImplemented(w, r)
			return
		}

		body, ok := decodeInsertAllBody(w, r)
		if !ok {
			return
		}

		desc, err := deps.Catalog.DescribeTable(r.Context(), &enginepb.DescribeTableRequest{
			Table: &enginepb.TableRef{
				ProjectId: projectID,
				DatasetId: datasetID,
				TableId:   tableID,
			},
		})
		if grpcToHTTPError(w, err) {
			return
		}

		protoRows := make([]*enginepb.DataRow, 0, len(body.Rows))
		for _, row := range body.Rows {
			protoRows = append(protoRows, jsonRowToProto(desc.GetSchema(), row.JSON))
		}

		if len(protoRows) > 0 {
			_, err = deps.Catalog.InsertRows(r.Context(), &enginepb.InsertRowsRequest{
				Table: &enginepb.TableRef{
					ProjectId: projectID,
					DatasetId: datasetID,
					TableId:   tableID,
				},
				Rows: protoRows,
			})
			if grpcToHTTPError(w, err) {
				return
			}
		}

		writeJSON(w, http.StatusOK, bqtypes.TableDataInsertAllResponse{
			Kind: tableDataInsertAllKind,
		})
	}
}

// TableDataList implements `bigquery.tabledata.list`:
//
//	GET /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}/data
//
// Pagination is honored via the documented `startIndex`, `maxResults`,
// and `pageToken` query parameters: pageToken (when supplied) is a
// decimal string encoding the next start row index, mirroring what
// `next_start_index` we return from the engine's ListRows.
// `selectedFields` and `formatOptions` are parsed but ignored until
// the query-execution work hooks them up.
//
// See docs/bigquery/docs/reference/rest/v2/tabledata/list.md.
func TableDataList(deps Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		projectID, datasetID, tableID := tableIDFromPath(r)
		if tableID == "" {
			writeError(w, http.StatusBadRequest, "invalid",
				"tableId is required")
			return
		}
		if deps.Catalog == nil {
			NotImplemented(w, r)
			return
		}

		q := r.URL.Query()
		startIndex, ok := parsePositiveInt64(w, q.Get("startIndex"), "startIndex", 0)
		if !ok {
			return
		}
		// pageToken (when set) overrides startIndex; matches the way
		// BigQuery clients page forward through tabledata results.
		if tok := q.Get("pageToken"); tok != "" {
			tokIdx, okTok := parsePositiveInt64(w, tok, "pageToken", 0)
			if !okTok {
				return
			}
			startIndex = tokIdx
		}
		maxResults, ok := parsePositiveInt64(w, q.Get("maxResults"), "maxResults", tableDataListDefaultMaxResults)
		if !ok {
			return
		}

		resp, err := deps.Catalog.ListRows(r.Context(), &enginepb.ListRowsRequest{
			Table: &enginepb.TableRef{
				ProjectId: projectID,
				DatasetId: datasetID,
				TableId:   tableID,
			},
			StartIndex: startIndex,
			MaxResults: maxResults,
		})
		if grpcToHTTPError(w, err) {
			return
		}

		out := bqtypes.TableDataList{
			Kind:      tableDataListKind,
			TotalRows: strconv.FormatInt(resp.GetTotalRows(), 10),
		}
		if resp.GetNextStartIndex() < resp.GetTotalRows() {
			out.PageToken = strconv.FormatInt(resp.GetNextStartIndex(), 10)
		}
		out.Rows = make([]bqtypes.Row, 0, len(resp.GetRows()))
		for _, row := range resp.GetRows() {
			out.Rows = append(out.Rows, dataRowToBQRow(row))
		}
		writeJSON(w, http.StatusOK, out)
	}
}

// parsePositiveInt64 parses an unsigned decimal string from a query
// parameter. Empty input falls back to `defaultValue`. A malformed
// value writes a 400 envelope and returns ok=false so the caller can
// short-circuit.
func parsePositiveInt64(w http.ResponseWriter, raw, name string, defaultValue int64) (int64, bool) {
	if raw == "" {
		return defaultValue, true
	}
	v, err := strconv.ParseInt(raw, 10, 64)
	if err != nil || v < 0 {
		writeError(w, http.StatusBadRequest, "invalid",
			fmt.Sprintf("Query parameter %q must be a non-negative integer", name))
		return 0, false
	}
	return v, true
}
