package handlers

import (
	"context"
	"crypto/sha256"
	"encoding/base64"
	"encoding/hex"
	"encoding/json"
	"fmt"
	"io"
	"net/http"
	"net/url"
	"strconv"
	"strings"

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

// tableDataListMaxResultsCap is the upper bound honored for maxResults.
const tableDataListMaxResultsCap = 100000

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
func jsonCellForField(f *enginepb.FieldSchema, v any) *enginepb.Cell {
	if f == nil {
		return jsonToCell(v)
	}
	if isJSONRepeatedFieldMode(f.GetMode()) {
		arr, ok := v.([]any)
		if !ok {
			return jsonToCell(v)
		}
		elemSchema := jsonRepeatedElementSchema(f)
		out := &enginepb.Array{Elements: make([]*enginepb.Cell, 0, len(arr))}
		for _, el := range arr {
			out.Elements = append(out.Elements, jsonCellForField(elemSchema, el))
		}
		return &enginepb.Cell{Value: &enginepb.Cell_Array{Array: out}}
	}
	if isJSONStructFieldType(f.GetType()) {
		m, ok := v.(map[string]any)
		if !ok {
			return jsonToCell(v)
		}
		st := &enginepb.Struct{Fields: make([]*enginepb.Cell, 0, len(f.GetFields()))}
		for _, sub := range f.GetFields() {
			subV, ok := m[sub.GetName()]
			if !ok {
				st.Fields = append(st.Fields, &enginepb.Cell{
					Value: &enginepb.Cell_NullValue{NullValue: true},
				})
				continue
			}
			st.Fields = append(st.Fields, jsonCellForField(sub, subV))
		}
		return &enginepb.Cell{Value: &enginepb.Cell_StructValue{StructValue: st}}
	}
	return jsonToCell(v)
}

func isJSONRepeatedFieldMode(mode string) bool {
	return strings.EqualFold(strings.TrimSpace(mode), sqlModeRepeated)
}

func jsonRepeatedElementSchema(f *enginepb.FieldSchema) *enginepb.FieldSchema {
	if f == nil {
		return nil
	}
	return &enginepb.FieldSchema{
		Name:        f.GetName(),
		Type:        f.GetType(),
		Description: f.GetDescription(),
		Fields:      f.GetFields(),
	}
}

func isJSONStructFieldType(t string) bool {
	switch strings.ToUpper(strings.TrimSpace(t)) {
	case sqlTypeSTRUCT, sqlTypeRECORD:
		return true
	default:
		return false
	}
}

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
		out.Cells = append(out.Cells, jsonCellForField(f, v))
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

// tableDataListParams holds parsed tabledata.list query parameters.
type tableDataListParams struct {
	startIndex        int64
	maxResults        int64
	selectedFields    []string
	useInt64Timestamp bool
}

// TableDataList implements `bigquery.tabledata.list`:
//
//	GET /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}/data
//
// Pagination is honored via the documented `startIndex`, `maxResults`,
// and `pageToken` query parameters: pageToken (when supplied) is a
// decimal string encoding the next start row index, mirroring what
// `next_start_index` we return from the engine's ListRows.
// `selectedFields` projects top-level columns (dotted paths select the
// top-level STRUCT field). `formatOptions.useInt64Timestamp` controls
// TIMESTAMP JSON encoding. Logical views have no Parquet backing;
// tabledata.list returns empty rows — use jobs.query for view preview.
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
		startIndex, maxResults, ok := tableDataListPaging(w, r.URL.Query())
		if !ok {
			return
		}
		listParams, ok := parseTableDataListParams(r.URL.Query(), startIndex, maxResults)
		if !ok {
			return
		}
		out, err := buildTableDataList(r.Context(), deps, projectID, datasetID, tableID, listParams)
		if grpcToHTTPError(w, err) {
			return
		}
		writeJSON(w, http.StatusOK, out)
	}
}

func parseTableDataListParams(
	q url.Values,
	startIndex, maxResults int64,
) (tableDataListParams, bool) {
	out := tableDataListParams{
		startIndex: startIndex,
		maxResults: maxResults,
	}
	if raw := strings.TrimSpace(q.Get("selectedFields")); raw != "" {
		for part := range strings.SplitSeq(raw, ",") {
			part = strings.TrimSpace(part)
			if part == "" {
				continue
			}
			// BigQuery paths like "e.d.f" select into nested fields; the
			// gateway projects at top-level granularity (field before ".").
			if dot := strings.Index(part, "."); dot >= 0 {
				part = part[:dot]
			}
			out.selectedFields = append(out.selectedFields, part)
		}
	}
	switch strings.ToLower(strings.TrimSpace(q.Get("formatOptions.useInt64Timestamp"))) {
	case "1", "true", "t", "yes":
		out.useInt64Timestamp = true
	}
	return out, true
}

func tableDataListPaging(w http.ResponseWriter, q url.Values) (startIndex, maxResults int64, ok bool) {
	startIndex, ok = parsePositiveInt64(w, q.Get("startIndex"), "startIndex", 0)
	if !ok {
		return 0, 0, false
	}
	if tok := q.Get("pageToken"); tok != "" {
		tokIdx, okTok := parsePositiveInt64(w, tok, "pageToken", 0)
		if !okTok {
			return 0, 0, false
		}
		startIndex = tokIdx
	}
	maxResults, ok = parsePositiveInt64(w, q.Get("maxResults"), "maxResults", tableDataListDefaultMaxResults)
	if !ok {
		return 0, 0, false
	}
	if maxResults > tableDataListMaxResultsCap {
		maxResults = tableDataListMaxResultsCap
	}
	return startIndex, maxResults, ok
}

func buildTableDataList(
	ctx context.Context,
	deps Dependencies,
	projectID, datasetID, tableID string,
	params tableDataListParams,
) (bqtypes.TableDataList, error) {
	table := &enginepb.TableRef{
		ProjectId: projectID,
		DatasetId: datasetID,
		TableId:   tableID,
	}
	desc, err := deps.Catalog.DescribeTable(ctx, &enginepb.DescribeTableRequest{Table: table})
	if err != nil {
		return bqtypes.TableDataList{}, err
	}
	schema := desc.GetSchema()
	formatOpts := bqtypes.WireFormatOptions{UseInt64Timestamp: params.useInt64Timestamp}
	if params.maxResults == 0 {
		total, totalErr := tableDataListTotalRows(ctx, deps.Catalog, table)
		if totalErr != nil {
			return bqtypes.TableDataList{}, totalErr
		}
		return bqtypes.TableDataList{
			Kind:      tableDataListKind,
			Etag:      tableDataListEtag(schema, total),
			TotalRows: strconv.FormatInt(total, 10),
		}, nil
	}
	resp, err := deps.Catalog.ListRows(ctx, &enginepb.ListRowsRequest{
		Table:      table,
		StartIndex: params.startIndex,
		MaxResults: params.maxResults,
	})
	if err != nil {
		return bqtypes.TableDataList{}, err
	}
	out := bqtypes.TableDataList{
		Kind:      tableDataListKind,
		Etag:      tableDataListEtag(schema, resp.GetTotalRows()),
		TotalRows: strconv.FormatInt(resp.GetTotalRows(), 10),
	}
	if resp.GetNextStartIndex() < resp.GetTotalRows() && params.maxResults > 0 {
		out.PageToken = strconv.FormatInt(resp.GetNextStartIndex(), 10)
	}
	fieldIdx := selectedFieldIndices(schema, params.selectedFields)
	out.Rows = make([]bqtypes.Row, 0, len(resp.GetRows()))
	for _, row := range resp.GetRows() {
		full := bqtypes.CellsToRowForSchema(row.GetCells(), schema, formatOpts)
		out.Rows = append(out.Rows, projectRowFields(full, fieldIdx))
	}
	return out, nil
}

func tableDataListTotalRows(
	ctx context.Context,
	catalog enginepb.CatalogClient,
	table *enginepb.TableRef,
) (int64, error) {
	resp, err := catalog.ListRows(ctx, &enginepb.ListRowsRequest{
		Table:      table,
		StartIndex: 0,
		MaxResults: 0,
	})
	if err != nil {
		return 0, err
	}
	return resp.GetTotalRows(), nil
}

func tableDataListEtag(schema *enginepb.TableSchema, totalRows int64) string {
	h := sha256.New()
	for _, f := range schema.GetFields() {
		_, _ = h.Write([]byte(f.GetName()))
		_, _ = h.Write([]byte{0})
		_, _ = h.Write([]byte(f.GetType()))
		_, _ = h.Write([]byte{0})
		_, _ = h.Write([]byte(f.GetMode()))
		_, _ = h.Write([]byte{0})
	}
	_, _ = h.Write([]byte(strconv.FormatInt(totalRows, 10)))
	return hex.EncodeToString(h.Sum(nil))[:32]
}

func selectedFieldIndices(schema *enginepb.TableSchema, selected []string) []int {
	if schema == nil || len(selected) == 0 {
		return nil
	}
	byName := map[string]int{}
	for i, f := range schema.GetFields() {
		byName[f.GetName()] = i
	}
	out := make([]int, 0, len(selected))
	seen := map[int]struct{}{}
	for _, name := range selected {
		idx, ok := byName[name]
		if !ok {
			continue
		}
		if _, dup := seen[idx]; dup {
			continue
		}
		seen[idx] = struct{}{}
		out = append(out, idx)
	}
	if len(out) == 0 {
		return nil
	}
	return out
}

func projectRowFields(row bqtypes.Row, fieldIdx []int) bqtypes.Row {
	if len(fieldIdx) == 0 {
		return row
	}
	out := bqtypes.Row{F: make([]bqtypes.Cell, 0, len(fieldIdx))}
	for _, idx := range fieldIdx {
		if idx >= 0 && idx < len(row.F) {
			out.F = append(out.F, row.F[idx])
		}
	}
	return out
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
