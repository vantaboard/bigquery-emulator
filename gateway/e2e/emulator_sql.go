//go:build integration

package e2e

import (
	"context"
	"database/sql"
	"database/sql/driver"
	"encoding/base64"
	"encoding/hex"
	"encoding/json"
	"errors"
	"fmt"
	"io"
	"math"
	"net/http"
	"reflect"
	"regexp"
	"strconv"
	"strings"
	"sync"
	"testing"
	"time"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
)

// emulatorDB drives GoogleSQL statements against the in-process gateway
// backed by emulator_main. It mirrors the database/sql surface the
// query_port_test.go suite uses so the ported tests need minimal
// edits beyond setup.
type emulatorDB struct {
	t         *testing.T
	projectID string
}

// emulatorConn is a per-connection view of emulatorDB (query port tests
// sometimes call db.Conn for statement isolation).
type emulatorConn struct {
	*emulatorDB
}

// emulatorRows implements a subset of *sql.Rows for query results.
type emulatorRows struct {
	columns []string
	types   []string
	rows    [][]any
	idx     int
	err     error
	closed  bool
}

// emulatorRow is a single-row cursor for QueryRowContext.
type emulatorRow struct {
	rows *emulatorRows
}

// emulatorResult satisfies sql.Result for DDL/DML ExecContext calls.
type emulatorResult struct {
	rowsAffected int64
}

// sharedEmulator is started once in TestMain and reused across the ported
// query port tests. Each subtest uses a unique project ID so catalog
// mutations do not collide under t.Parallel().
var (
	sharedEmulator   *emulatorEnv
	sharedEmulatorMu sync.Mutex
)

func openQueryPortTestDB(t *testing.T) (*emulatorDB, context.Context) {
	t.Helper()
	if sharedEmulator == nil {
		t.Fatal("shared emulator not started; TestMain missing?")
	}
	ensureSharedEmulatorHealthy(t)
	pid := projectIDForTest(t.Name())
	return &emulatorDB{
		t:         t,
		projectID: pid,
	}, context.Background()
}

// queryPortSkipIfNotImplemented skips subtests whose SQL hits engine
// surfaces that are intentionally unimplemented in the emulator today.
func queryPortSkipIfNotImplemented(t *testing.T, err error) {
	t.Helper()
	if err == nil {
		return
	}
	msg := err.Error()
	if strings.Contains(msg, "501") &&
		(strings.Contains(msg, "not yet implemented") ||
			strings.Contains(msg, "notImplemented") ||
			strings.Contains(msg, "UNIMPLEMENTED")) {
		t.Skip(msg)
	}
}

// jobsQueryURL returns the live jobs.query endpoint for this test's project.
// It reads sharedEmulator on each call so a mid-suite engine restart (new
// httptest gateway) is visible to long-lived *emulatorDB handles such as the
// single db opened at the start of TestQuery.
func (db *emulatorDB) jobsQueryURL() string {
	sharedEmulatorMu.Lock()
	env := sharedEmulator
	sharedEmulatorMu.Unlock()
	if env == nil {
		return ""
	}
	return env.URL() + "/bigquery/v2/projects/" + db.projectID + "/queries"
}

// projectIDForTest maps a subtest name to the BigQuery project id used on
// jobs.query. TestQuery/* subtests share one project so CREATE FUNCTION in
// create_function is visible to use_function (query port uses one session).
func projectIDForTest(testName string) string {
	if strings.HasPrefix(testName, "TestQuery/") {
		return sanitizeProjectID("TestQuery")
	}
	if i := strings.LastIndex(testName, "/"); i >= 0 {
		testName = testName[:i]
	}
	return sanitizeProjectID(testName)
}

func sanitizeProjectID(testName string) string {
	const prefix = "gsql-"
	name := strings.TrimPrefix(testName, "Test")
	name = strings.ReplaceAll(name, "/", "_")
	var b strings.Builder
	b.Grow(len(prefix) + len(name))
	b.WriteString(prefix)
	for _, r := range name {
		switch {
		case r >= 'a' && r <= 'z', r >= 'A' && r <= 'Z', r >= '0' && r <= '9', r == '_':
			b.WriteRune(r)
		default:
			b.WriteRune('_')
		}
	}
	out := b.String()
	if len(out) > 30 {
		out = out[:30]
	}
	return out
}

func (db *emulatorDB) Close() error { return nil }

func (db *emulatorDB) Conn(ctx context.Context) (*emulatorConn, error) {
	_ = ctx
	return &emulatorConn{emulatorDB: db}, nil
}

func (db *emulatorDB) QueryContext(ctx context.Context, query string, args ...any) (*emulatorRows, error) {
	return db.query(ctx, query, args...)
}

func (db *emulatorDB) QueryRowContext(ctx context.Context, query string, args ...any) *emulatorRow {
	rows, err := db.query(ctx, query, args...)
	if err != nil {
		return &emulatorRow{rows: &emulatorRows{err: err}}
	}
	return &emulatorRow{rows: rows}
}

func (db *emulatorDB) ExecContext(ctx context.Context, query string, args ...any) (sql.Result, error) {
	stmts := splitSQLStatements(unwrapBeginEndBlock(query))
	if len(stmts) == 0 {
		return nil, fmt.Errorf("empty SQL")
	}
	var last *bqtypes.QueryResponse
	var lastErr error
	argIdx := 0
	for _, stmt := range stmts {
		var err error
		last, argIdx, err = db.runSingleQuery(ctx, stmt, argIdx, true, args...)
		lastErr = err
		if err != nil {
			return nil, err
		}
	}
	if last == nil {
		return emulatorResult{}, nil
	}
	var n int64
	if last.NumDmlAffectedRows != "" {
		n, _ = strconv.ParseInt(last.NumDmlAffectedRows, 10, 64)
	}
	return emulatorResult{rowsAffected: n}, lastErr
}

func (c *emulatorConn) Close() error { return nil }

func (c *emulatorConn) QueryContext(ctx context.Context, query string, args ...any) (*emulatorRows, error) {
	return c.emulatorDB.QueryContext(ctx, query, args...)
}

func (c *emulatorConn) QueryRowContext(ctx context.Context, query string, args ...any) *emulatorRow {
	return c.emulatorDB.QueryRowContext(ctx, query, args...)
}

func (c *emulatorConn) ExecContext(ctx context.Context, query string, args ...any) (sql.Result, error) {
	return c.emulatorDB.ExecContext(ctx, query, args...)
}

func (r emulatorResult) LastInsertId() (int64, error) { return 0, errors.New("not supported") }
func (r emulatorResult) RowsAffected() (int64, error) { return r.rowsAffected, nil }

func (db *emulatorDB) query(ctx context.Context, query string, args ...any) (*emulatorRows, error) {
	resp, err := db.runQuery(ctx, query, args...)
	if err != nil {
		return nil, err
	}
	cols, fields := schemaColumns(resp.Schema)
	out := make([][]any, 0, len(resp.Rows))
	for _, row := range resp.Rows {
		scan, err := rowToScanValues(row, fields)
		if err != nil {
			return &emulatorRows{err: err}, nil
		}
		out = append(out, scan)
	}
	return &emulatorRows{columns: cols, rows: out}, nil
}

const queryPortDefaultDatasetID = "_default"

// splitSQLStatements splits a script on semicolons outside of quoted
// string / identifier literals (BigQuery multi-statement scripts).
func splitSQLStatements(sql string) []string {
	var out []string
	var b strings.Builder
	inSingle := false
	inDouble := false
	inBacktick := false
	for i := 0; i < len(sql); i++ {
		c := sql[i]
		if c == '\'' && !inDouble && !inBacktick {
			if inSingle && i+1 < len(sql) && sql[i+1] == '\'' {
				b.WriteByte(c)
				b.WriteByte(sql[i+1])
				i++
				continue
			}
			inSingle = !inSingle
			b.WriteByte(c)
			continue
		}
		if c == '"' && !inSingle && !inBacktick {
			inDouble = !inDouble
			b.WriteByte(c)
			continue
		}
		if c == '`' && !inSingle && !inDouble {
			inBacktick = !inBacktick
			b.WriteByte(c)
			continue
		}
		if c == ';' && !inSingle && !inDouble && !inBacktick {
			if stmt := strings.TrimSpace(b.String()); stmt != "" {
				out = append(out, stmt)
			}
			b.Reset()
			continue
		}
		b.WriteByte(c)
	}
	if stmt := strings.TrimSpace(b.String()); stmt != "" {
		out = append(out, stmt)
	}
	return out
}

var beginEndBlockRE = regexp.MustCompile(`(?is)^\s*BEGIN\s+(.*)\s+END\s*;?\s*$`)

func unwrapBeginEndBlock(sql string) string {
	trimmed := strings.TrimSpace(sql)
	if m := beginEndBlockRE.FindStringSubmatch(trimmed); len(m) == 2 {
		return strings.TrimSpace(m[1])
	}
	return sql
}

func (db *emulatorDB) runQuery(ctx context.Context, query string, args ...any) (*bqtypes.QueryResponse, error) {
	stmts := splitSQLStatements(unwrapBeginEndBlock(query))
	if len(stmts) == 0 {
		return nil, fmt.Errorf("empty SQL")
	}
	if len(stmts) == 1 {
		resp, _, err := db.runSingleQuery(ctx, stmts[0], 0, true, args...)
		return resp, err
	}
	argIdx := 0
	var err error
	for i := 0; i < len(stmts)-1; i++ {
		var resp *bqtypes.QueryResponse
		resp, argIdx, err = db.runSingleQuery(ctx, stmts[i], argIdx, true, args...)
		if err != nil {
			return nil, err
		}
		_ = resp
	}
	resp, _, err := db.runSingleQuery(ctx, stmts[len(stmts)-1], argIdx, true, args...)
	return resp, err
}

func (db *emulatorDB) runSingleQuery(ctx context.Context, query string, argStart int, allowRestart bool, args ...any) (*bqtypes.QueryResponse, int, error) {
	sqlText, params, nextArg, err := bindQueryParams(query, argStart, args)
	if err != nil {
		return nil, argStart, err
	}
	body := map[string]any{
		"query":        sqlText,
		"useLegacySql": false,
		"defaultDataset": map[string]string{
			"datasetId": queryPortDefaultDatasetID,
		},
	}
	if len(params) > 0 {
		body["queryParameters"] = params
	}
	raw, err := json.Marshal(body)
	if err != nil {
		return nil, argStart, err
	}
	queriesURL := db.jobsQueryURL()
	if queriesURL == "" {
		return nil, argStart, fmt.Errorf("shared emulator not available")
	}
	req, err := http.NewRequestWithContext(ctx, http.MethodPost, queriesURL, strings.NewReader(string(raw)))
	if err != nil {
		return nil, argStart, err
	}
	req.Header.Set("Content-Type", "application/json")
	resp, err := http.DefaultClient.Do(req)
	if err != nil {
		if allowRestart && restartSharedEmulatorAfterBackendError(db.t, err) {
			return db.runSingleQuery(ctx, query, argStart, false, args...)
		}
		return nil, argStart, err
	}
	defer resp.Body.Close()
	respBody, err := io.ReadAll(resp.Body)
	if err != nil {
		return nil, argStart, err
	}
	if resp.StatusCode != http.StatusOK {
		qerr := fmt.Errorf("jobs.query -> %d: %s", resp.StatusCode, string(respBody))
		if allowRestart && restartSharedEmulatorAfterBackendError(db.t, qerr) {
			return db.runSingleQuery(ctx, query, argStart, false, args...)
		}
		return nil, argStart, qerr
	}
	var out bqtypes.QueryResponse
	if err := json.Unmarshal(respBody, &out); err != nil {
		return nil, argStart, fmt.Errorf("decode QueryResponse: %w (body=%s)", err, string(respBody))
	}
	if len(out.Errors) > 0 {
		return nil, argStart, fmt.Errorf("%s", out.Errors[0].Message)
	}
	return &out, nextArg, nil
}

func bindQueryParams(query string, argStart int, args []any) (string, []map[string]any, int, error) {
	hasNamed := false
	for _, arg := range args {
		if _, ok := arg.(sql.NamedArg); ok {
			hasNamed = true
			break
		}
	}
	if hasNamed {
		params := make([]map[string]any, 0, len(args))
		for _, arg := range args {
			na, ok := arg.(sql.NamedArg)
			if !ok {
				return "", nil, argStart, fmt.Errorf("cannot mix named and positional parameters")
			}
			pt, pv, err := argToParameter(na.Value)
			if err != nil {
				return "", nil, argStart, err
			}
			params = append(params, map[string]any{
				"name":           na.Name,
				"parameterType":  pt,
				"parameterValue": pv,
			})
		}
		return query, params, argStart, nil
	}
	return bindPositionalParams(query, argStart, args)
}

func bindPositionalParams(query string, argStart int, args []any) (string, []map[string]any, int, error) {
	if len(args) == 0 {
		return query, nil, argStart, nil
	}
	var params []map[string]any
	var b strings.Builder
	argIdx := argStart
	for i := 0; i < len(query); i++ {
		if query[i] == '?' {
			if argIdx >= len(args) {
				return "", nil, argIdx, fmt.Errorf("not enough query arguments")
			}
			name := fmt.Sprintf("p%d", argIdx)
			pt, pv, err := argToParameter(args[argIdx])
			if err != nil {
				return "", nil, argIdx, err
			}
			params = append(params, map[string]any{
				"name":           name,
				"parameterType":  pt,
				"parameterValue": pv,
			})
			b.WriteString("@" + name)
			argIdx++
			continue
		}
		b.WriteByte(query[i])
	}
	return b.String(), params, argIdx, nil
}

func argToParameter(arg any) (map[string]any, map[string]any, error) {
	if arg == nil {
		return map[string]any{"type": "STRING"}, map[string]any{"value": nil}, nil
	}
	switch v := arg.(type) {
	case int:
		return map[string]any{"type": "INT64"}, map[string]any{"value": strconv.FormatInt(int64(v), 10)}, nil
	case int64:
		return map[string]any{"type": "INT64"}, map[string]any{"value": strconv.FormatInt(v, 10)}, nil
	case float64:
		return map[string]any{"type": "FLOAT64"}, map[string]any{"value": strconv.FormatFloat(v, 'f', -1, 64)}, nil
	case float32:
		return map[string]any{"type": "FLOAT64"}, map[string]any{"value": strconv.FormatFloat(float64(v), 'f', -1, 64)}, nil
	case bool:
		return map[string]any{"type": "BOOL"}, map[string]any{"value": strconv.FormatBool(v)}, nil
	case string:
		return map[string]any{"type": "STRING"}, map[string]any{"value": v}, nil
	case []byte:
		return map[string]any{"type": "BYTES"}, map[string]any{"value": base64.StdEncoding.EncodeToString(v)}, nil
	case []string:
		vals := make([]map[string]any, 0, len(v))
		for _, s := range v {
			vals = append(vals, map[string]any{"value": s})
		}
		return map[string]any{
				"type":      "ARRAY",
				"arrayType": map[string]any{"type": "STRING"},
			}, map[string]any{
				"arrayValues": vals,
			}, nil
	default:
		return nil, nil, fmt.Errorf("unsupported query arg type %T", arg)
	}
}

func schemaColumns(schema *bqtypes.TableSchema) ([]string, []bqtypes.TableFieldSchema) {
	if schema == nil {
		return nil, nil
	}
	names := make([]string, 0, len(schema.Fields))
	fields := make([]bqtypes.TableFieldSchema, 0, len(schema.Fields))
	for _, f := range schema.Fields {
		names = append(names, f.Name)
		fields = append(fields, f)
	}
	return names, fields
}

func fieldTypeString(f bqtypes.TableFieldSchema) string {
	typ := f.Type
	if f.Mode == "REPEATED" {
		return "ARRAY<" + typ + ">"
	}
	return typ
}

func rowToScanValues(row bqtypes.Row, fields []bqtypes.TableFieldSchema) ([]any, error) {
	out := make([]any, 0, len(row.F))
	for i, cell := range row.F {
		var field *bqtypes.TableFieldSchema
		if i < len(fields) {
			field = &fields[i]
		}
		v, err := wireCellToAny(cell, field)
		if err != nil {
			return nil, err
		}
		out = append(out, v)
	}
	return out, nil
}

func wireCellToAny(cell bqtypes.Cell, field *bqtypes.TableFieldSchema) (any, error) {
	colType := ""
	var structFields []bqtypes.TableFieldSchema
	if field != nil {
		colType = fieldTypeString(*field)
		structFields = field.Fields
	}
	if cell.V == nil {
		return nil, nil
	}
	switch v := cell.V.(type) {
	case string:
		return decodeScalarString(v, colType)
	case []any:
		elemType := ""
		var elemFields []bqtypes.TableFieldSchema
		if field != nil && field.Mode == "REPEATED" {
			elemType = field.Type
			elemFields = field.Fields
		} else if colType == "ARRAY" || strings.HasPrefix(colType, "ARRAY<") {
			elemType = arrayElementType(colType)
		}
		out := make([]any, 0, len(v))
		for i, el := range v {
			switch e := el.(type) {
			case map[string]any:
				if ff, ok := e["f"].([]any); ok {
					vals := make([]any, 0, len(ff))
					for j, fieldCell := range ff {
						subField := structFieldAt(elemFields, j, elemType)
						if fm, ok := fieldCell.(map[string]any); ok {
							sub, err := wireCellToAny(bqtypes.Cell{V: fm["v"]}, subField)
							if err != nil {
								return nil, err
							}
							vals = append(vals, sub)
						}
					}
					out = append(out, vals)
					continue
				}
				if vv, ok := e["v"]; ok {
					nestedField := field
					if _, isStruct := vv.(map[string]any); isStruct && field != nil && len(field.Fields) > 0 {
						nestedField = field
					} else {
						nestedField = structFieldAt(elemFields, i, elemType)
					}
					sub, err := wireCellToAny(bqtypes.Cell{V: vv}, nestedField)
					if err != nil {
						return nil, err
					}
					out = append(out, sub)
				}
			case []any:
				vals := make([]any, 0, len(e))
				for j, part := range e {
					subField := structFieldAt(elemFields, j, elemType)
					sub, err := wireCellToAny(bqtypes.Cell{V: part}, subField)
					if err != nil {
						return nil, err
					}
					vals = append(vals, sub)
				}
				out = append(out, vals)
			default:
				subField := structFieldAt(elemFields, i, elemType)
				sub, err := wireCellToAny(bqtypes.Cell{V: e}, subField)
				if err != nil {
					return nil, err
				}
				out = append(out, sub)
			}
		}
		return out, nil
	case map[string]any:
		if f, ok := v["f"].([]any); ok {
			vals := make([]any, 0, len(f))
			for i, fieldCell := range f {
				var subField *bqtypes.TableFieldSchema
				if i < len(structFields) {
					subField = &structFields[i]
				}
				if fm, ok := fieldCell.(map[string]any); ok {
					sub, err := wireCellToAny(bqtypes.Cell{V: fm["v"]}, subField)
					if err != nil {
						return nil, err
					}
					vals = append(vals, sub)
				}
			}
			return vals, nil
		}
		return v, nil
	default:
		return v, nil
	}
}

func structFieldAt(fields []bqtypes.TableFieldSchema, idx int, fallbackType string) *bqtypes.TableFieldSchema {
	if idx < len(fields) {
		return &fields[idx]
	}
	if fallbackType != "" {
		return &bqtypes.TableFieldSchema{Type: fallbackType}
	}
	return nil
}

func arrayElementType(colType string) string {
	if !strings.HasPrefix(colType, "ARRAY<") {
		return ""
	}
	inner := strings.TrimSuffix(strings.TrimPrefix(colType, "ARRAY<"), ">")
	return inner
}

// decodeBytesWireCell normalizes BigQuery REST BYTES cells for database/sql
// scanning. The engine may double-encode (base64 of DuckDB's `x{hex}`
// text); unwrap to the canonical base64 form query port expects.
func decodeBytesWireCell(s string) (any, error) {
	raw, err := base64.StdEncoding.DecodeString(s)
	if err != nil {
		return s, nil
	}
	text := string(raw)
	if len(text) > 1 && text[0] == 'x' {
		if h, err := hex.DecodeString(text[1:]); err == nil {
			if len(h) > 1 && h[0] == 'x' {
				if inner, err := hex.DecodeString(string(h[1:])); err == nil {
					return base64.StdEncoding.EncodeToString(inner), nil
				}
			}
			return base64.StdEncoding.EncodeToString(h), nil
		}
	}
	return base64.StdEncoding.EncodeToString(raw), nil
}

func decodeScalarString(s, colType string) (any, error) {
	switch colType {
	case "INT64", "INTEGER":
		return strconv.ParseInt(s, 10, 64)
	case "FLOAT64", "FLOAT":
		if s == "NaN" {
			return math.NaN(), nil
		}
		if s == "Infinity" {
			return math.Inf(1), nil
		}
		if s == "-Infinity" {
			return math.Inf(-1), nil
		}
		return strconv.ParseFloat(s, 64)
	case "BOOL", "BOOLEAN":
		return strconv.ParseBool(s)
	case "BYTES":
		return decodeBytesWireCell(s)
	case "TIMESTAMP":
		return timestampToQueryPortCanonical(s)
	case "DATE", "DATETIME", "TIME", "STRING", "NUMERIC", "BIGNUMERIC", "GEOGRAPHY", "JSON", "":
		return s, nil
	default:
		if strings.HasPrefix(colType, "ARRAY") {
			return s, nil
		}
		return s, nil
	}
}

// timestampToQueryPortCanonical maps BigQuery REST RFC3339 timestamps to the
// canonical form query port uses when scanning into any (see
// createTimestampFormatFromTime in the ported tests).
func timestampToQueryPortCanonical(s string) (string, error) {
	if isEpochMicrosString(s) {
		us, err := strconv.ParseInt(s, 10, 64)
		if err != nil {
			return s, nil
		}
		t := time.UnixMicro(us).UTC()
		if t.Nanosecond()%1_000_000 == 0 {
			return t.Format("2006-01-02 15:04:05+00"), nil
		}
		return t.Format("2006-01-02 15:04:05.000000+00"), nil
	}
	if strings.HasSuffix(s, "+00") || strings.HasSuffix(s, "+00:00") {
		// Already in legacy canonical form.
		if strings.Contains(s, "T") {
			s = strings.Replace(s, "T", " ", 1)
		}
		s = strings.Replace(s, "+00:00", "+00", 1)
		return s, nil
	}
	layouts := []string{
		time.RFC3339Nano,
		time.RFC3339,
		"2006-01-02 15:04:05.999999",
		"2006-01-02 15:04:05",
	}
	var t time.Time
	var err error
	for _, layout := range layouts {
		t, err = time.Parse(layout, s)
		if err == nil {
			break
		}
	}
	if err != nil {
		return s, nil
	}
	utc := t.UTC()
	if utc.Nanosecond() == 0 {
		return utc.Format("2006-01-02 15:04:05+00"), nil
	}
	return utc.Format("2006-01-02 15:04:05.000000+00"), nil
}

func isEpochMicrosString(s string) bool {
	if s == "" {
		return false
	}
	i := 0
	if s[0] == '-' {
		if len(s) == 1 {
			return false
		}
		i = 1
	}
	for ; i < len(s); i++ {
		if s[i] < '0' || s[i] > '9' {
			return false
		}
	}
	return true
}

func (r *emulatorRows) Close() error {
	r.closed = true
	return nil
}

func (r *emulatorRows) Columns() ([]string, error) {
	if r.err != nil {
		return nil, r.err
	}
	return r.columns, nil
}

func (r *emulatorRows) Next() bool {
	if r.err != nil || r.closed {
		return false
	}
	if r.idx >= len(r.rows) {
		return false
	}
	r.idx++
	return true
}

func (r *emulatorRows) Scan(dest ...any) error {
	if r.err != nil {
		return r.err
	}
	if r.idx == 0 || r.idx > len(r.rows) {
		return errors.New("Scan called without Next or after EOF")
	}
	row := r.rows[r.idx-1]
	if len(dest) != len(row) {
		return fmt.Errorf("Scan: expected %d destinations, got %d", len(row), len(dest))
	}
	for i, d := range dest {
		if err := assignScanValue(d, row[i]); err != nil {
			return err
		}
	}
	return nil
}

func (r *emulatorRows) Err() error { return r.err }

func (row *emulatorRow) Scan(dest ...any) error {
	if row.rows.err != nil {
		return row.rows.err
	}
	if len(row.rows.rows) == 0 {
		return sql.ErrNoRows
	}
	if len(dest) != len(row.rows.rows[0]) {
		return fmt.Errorf("Scan: expected %d destinations, got %d", len(row.rows.rows[0]), len(dest))
	}
	for i, d := range dest {
		if err := assignScanValue(d, row.rows.rows[0][i]); err != nil {
			return err
		}
	}
	return nil
}

func assignScanValue(dest any, src any) error {
	switch d := dest.(type) {
	case *any:
		*d = src
		return nil
	case *int64:
		switch v := src.(type) {
		case int64:
			*d = v
			return nil
		case string:
			i, err := strconv.ParseInt(v, 10, 64)
			if err != nil {
				return err
			}
			*d = i
			return nil
		default:
			return fmt.Errorf("cannot scan %T into *int64", src)
		}
	case *float64:
		switch v := src.(type) {
		case float64:
			*d = v
			return nil
		case int64:
			*d = float64(v)
			return nil
		case string:
			f, err := strconv.ParseFloat(v, 64)
			if err != nil {
				return err
			}
			*d = f
			return nil
		default:
			return fmt.Errorf("cannot scan %T into *float64", src)
		}
	case *string:
		switch v := src.(type) {
		case string:
			*d = v
			return nil
		case []byte:
			*d = string(v)
			return nil
		default:
			return fmt.Errorf("cannot scan %T into *string", src)
		}
	case *bool:
		switch v := src.(type) {
		case bool:
			*d = v
			return nil
		case string:
			b, err := strconv.ParseBool(v)
			if err != nil {
				return err
			}
			*d = b
			return nil
		default:
			return fmt.Errorf("cannot scan %T into *bool", src)
		}
	case *sql.NullString:
		if src == nil {
			d.Valid = false
			d.String = ""
			return nil
		}
		s, ok := src.(string)
		if !ok {
			return fmt.Errorf("cannot scan %T into *sql.NullString", src)
		}
		d.String = s
		d.Valid = true
		return nil
	case *sql.NullInt64:
		if src == nil {
			d.Valid = false
			d.Int64 = 0
			return nil
		}
		switch v := src.(type) {
		case int64:
			d.Int64 = v
			d.Valid = true
			return nil
		case string:
			i, err := strconv.ParseInt(v, 10, 64)
			if err != nil {
				return err
			}
			d.Int64 = i
			d.Valid = true
			return nil
		default:
			return fmt.Errorf("cannot scan %T into *sql.NullInt64", src)
		}
	case *sql.NullFloat64:
		if src == nil {
			d.Valid = false
			d.Float64 = 0
			return nil
		}
		switch v := src.(type) {
		case float64:
			d.Float64 = v
			d.Valid = true
			return nil
		case int64:
			d.Float64 = float64(v)
			d.Valid = true
			return nil
		case string:
			f, err := strconv.ParseFloat(v, 64)
			if err != nil {
				return err
			}
			d.Float64 = f
			d.Valid = true
			return nil
		default:
			return fmt.Errorf("cannot scan %T into *sql.NullFloat64", src)
		}
	case *sql.NullBool:
		if src == nil {
			d.Valid = false
			d.Bool = false
			return nil
		}
		switch v := src.(type) {
		case bool:
			d.Bool = v
			d.Valid = true
			return nil
		case string:
			b, err := strconv.ParseBool(v)
			if err != nil {
				return err
			}
			d.Bool = b
			d.Valid = true
			return nil
		default:
			return fmt.Errorf("cannot scan %T into *sql.NullBool", src)
		}
	case *[]byte:
		switch v := src.(type) {
		case string:
			// query port/database/sql expect BYTES columns as base64 text
			// in the destination slice (see TestParameterBindingTypes).
			*d = []byte(v)
			return nil
		case []byte:
			*d = v
			return nil
		default:
			return fmt.Errorf("cannot scan %T into *[]byte", src)
		}
	default:
		rv := reflect.ValueOf(dest)
		if rv.Kind() != reflect.Ptr {
			return fmt.Errorf("destination %T is not a pointer", dest)
		}
		if src == nil {
			rv.Elem().SetZero()
			return nil
		}
		sv := reflect.ValueOf(src)
		dv := rv.Elem()
		if sv.Type().AssignableTo(dv.Type()) {
			dv.Set(sv)
			return nil
		}
		return fmt.Errorf("cannot scan %T into %T", src, dest)
	}
}

// rowDump scans every row from rows into a [][]any (ported helper).
func rowDump(t *testing.T, rows *emulatorRows) [][]any {
	t.Helper()
	defer rows.Close()
	cols, err := rows.Columns()
	if err != nil {
		t.Fatalf("Columns: %v", err)
	}
	var out [][]any
	for rows.Next() {
		buf := make([]any, len(cols))
		ptrs := make([]any, len(cols))
		for i := range buf {
			ptrs[i] = &buf[i]
		}
		if err := rows.Scan(ptrs...); err != nil {
			t.Fatalf("Scan: %v", err)
		}
		out = append(out, buf)
	}
	if err := rows.Err(); err != nil {
		t.Fatalf("rows.Err: %v", err)
	}
	return out
}

// ensure emulatorConn satisfies the interfaces tests use via conn.
var (
	_ interface {
		QueryContext(context.Context, string, ...any) (*emulatorRows, error)
		QueryRowContext(context.Context, string, ...any) *emulatorRow
		ExecContext(context.Context, string, ...any) (sql.Result, error)
		Close() error
	} = (*emulatorConn)(nil)
)

func isJobsQueryBackendTransportError(err error) bool {
	if err == nil {
		return false
	}
	s := err.Error()
	if strings.Contains(s, "connection refused") ||
		strings.Contains(s, "error reading from server") ||
		strings.Contains(s, "EOF") {
		return true
	}
	return strings.Contains(s, "jobs.query -> 503") &&
		(strings.Contains(s, "connection refused") ||
			strings.Contains(s, "error reading from server") ||
			strings.Contains(s, "EOF"))
}

// restartSharedEmulatorAfterBackendError tears down and relaunches the
// shared TestMain emulator when the engine subprocess died (503 EOF /
// connection refused). Returns true when a restart was attempted so the
// caller can retry the RPC once. Callers must post jobs.query to
// jobsQueryURL() (not a URL captured before the restart) so the new
// in-process gateway is used.
func restartSharedEmulatorAfterBackendError(t *testing.T, queryErr error) bool {
	t.Helper()
	if sharedEmulator == nil || !isJobsQueryBackendTransportError(queryErr) {
		return false
	}
	sharedEmulatorMu.Lock()
	defer sharedEmulatorMu.Unlock()
	if sharedEmulator == nil {
		return false
	}
	t.Logf("restarting shared emulator after backend failure: %v", queryErr)
	old := sharedEmulator
	dataDir := old.dataDir
	old.tearDown()
	env, err := launchEmulator(dataDir)
	if err != nil {
		t.Fatalf("restart shared emulator: %v", err)
	}
	sharedEmulator = env
	return true
}

func ensureSharedEmulatorHealthy(t *testing.T) {
	t.Helper()
	sharedEmulatorMu.Lock()
	env := sharedEmulator
	sharedEmulatorMu.Unlock()
	if env == nil || env.client == nil {
		return
	}
	ctx, cancel := context.WithTimeout(context.Background(), 2*time.Second)
	defer cancel()
	if err := env.client.WaitForReady(ctx); err != nil {
		restartSharedEmulatorAfterBackendError(t, fmt.Errorf("engine not ready: %w", err))
	}
}

// silence unused import if a future test needs driver.ErrBadConn
var _ = driver.ErrBadConn
