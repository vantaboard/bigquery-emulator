package handlers

import (
	"context"
	"fmt"
	"net/http"
	"regexp"
	"strconv"
	"strings"
	"time"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
	"github.com/vantaboard/bigquery-emulator/gateway/jobs"
	"github.com/vantaboard/bigquery-emulator/gateway/middleware"
	"github.com/vantaboard/bigquery-emulator/gateway/query"
)

// isMultiStatementScript reports whether sql is a DECLARE/SET script the
// gateway executes statement-by-statement.
func stripBlockComments(sql string) string {
	var out strings.Builder
	out.Grow(len(sql))
	for i := 0; i < len(sql); {
		if i+1 < len(sql) && sql[i] == '/' && sql[i+1] == '*' {
			i += 2
			for i+1 < len(sql) && (sql[i] != '*' || sql[i+1] != '/') {
				i++
			}
			if i+1 < len(sql) {
				i += 2
			}
			continue
		}
		out.WriteByte(sql[i])
		i++
	}
	return out.String()
}

func trimLeadingSQLComments(sql string) string {
	var kept []string
	for line := range strings.SplitSeq(sql, "\n") {
		if strings.HasPrefix(strings.TrimSpace(line), "--") {
			continue
		}
		kept = append(kept, line)
	}
	return strings.TrimSpace(strings.Join(kept, "\n"))
}

func sqlForScriptDetection(sql string) string {
	return trimLeadingSQLComments(stripBlockComments(sql))
}

var setKeywordRE = regexp.MustCompile(`(?i)\bSET\b`)

var beginEndBlockRE = regexp.MustCompile(`(?is)^\s*BEGIN\s+(.*)\s+END\s*;?\s*$`)

func unwrapBeginEndBlock(sql string) string {
	trimmed := strings.TrimSpace(sql)
	if m := beginEndBlockRE.FindStringSubmatch(trimmed); len(m) == 2 {
		return strings.TrimSpace(m[1])
	}
	return sql
}

func isMultiStatementScript(sql string) bool {
	trimmed := strings.TrimSpace(sql)
	upper := strings.ToUpper(sqlForScriptDetection(trimmed))
	// DDL setup statements (CREATE PROCEDURE bodies embed BEGIN/SET) must
	// not enter the script splitter.
	if strings.HasPrefix(upper, "CREATE ") ||
		strings.HasPrefix(upper, "DROP ") ||
		strings.HasPrefix(upper, "ALTER ") {
		return false
	}
	sql = unwrapBeginEndBlock(trimmed)
	detected := sqlForScriptDetection(sql)
	upper = strings.ToUpper(detected)
	return strings.Contains(upper, "DECLARE ") ||
		strings.Contains(upper, "CALL ") ||
		(strings.Count(detected, ";") >= 2 && setKeywordRE.MatchString(upper))
}

// needsEngineScriptExecution reports whether the script must run as one
// engine round-trip so DECLARE/CALL variable scope survives (the engine's
// ExecuteMultiStmtScript path). Legacy SET+UNNEST substitution scripts
// without DECLARE/CALL stay on the per-statement split path.
func needsEngineScriptExecution(sql string) bool {
	trimmed := strings.TrimSpace(sql)
	upper := strings.ToUpper(trimLeadingSQLComments(trimmed))
	return strings.HasPrefix(upper, "BEGIN") ||
		strings.Contains(upper, "DECLARE ") ||
		strings.Contains(upper, "CALL ")
}

// splitScriptStatements splits script SQL on semicolons outside quotes.
func splitScriptStatements(sql string) []string {
	var out []string
	var b strings.Builder
	inQuote := false
	for i := 0; i < len(sql); i++ {
		c := sql[i]
		if c == '\'' {
			inQuote = !inQuote
			b.WriteByte(c)
			continue
		}
		if c == ';' && !inQuote {
			stmt := strings.TrimSpace(b.String())
			if stmt != "" && !isCommentOnlyStatement(stmt) {
				out = append(out, stmt)
			}
			b.Reset()
			continue
		}
		b.WriteByte(c)
	}
	if tail := strings.TrimSpace(b.String()); tail != "" && !isCommentOnlyStatement(tail) {
		out = append(out, tail)
	}
	return out
}

func isCommentOnlyStatement(s string) bool {
	for line := range strings.SplitSeq(s, "\n") {
		t := strings.TrimSpace(line)
		if t == "" {
			continue
		}
		if !strings.HasPrefix(t, "--") {
			return false
		}
	}
	return true
}

type scriptStmtKind int

const (
	scriptStmtDeclare scriptStmtKind = iota
	scriptStmtSet
	scriptStmtCall
	scriptStmtQuery
)

type scriptStatement struct {
	kind scriptStmtKind
	sql  string
	name string
}

func classifyScriptStatement(sql string) scriptStatement {
	trim := trimLeadingSQLComments(sql)
	upper := strings.ToUpper(trim)
	switch {
	case strings.HasPrefix(upper, "DECLARE "):
		rest := strings.TrimSpace(trim[8:])
		name := rest
		if sp := strings.IndexAny(rest, " \t"); sp > 0 {
			name = rest[:sp]
		}
		return scriptStatement{kind: scriptStmtDeclare, sql: trim, name: name}
	case strings.HasPrefix(upper, "SET "):
		name, body := parseSetStatement(trim)
		return scriptStatement{kind: scriptStmtSet, sql: body, name: name}
	case strings.HasPrefix(upper, "CALL "):
		return scriptStatement{kind: scriptStmtCall, sql: trim}
	default:
		return scriptStatement{kind: scriptStmtQuery, sql: trim}
	}
}

func parseSetStatement(sql string) (name, body string) {
	rest := strings.TrimSpace(sql[4:])
	before, after, ok := strings.Cut(rest, "=")
	if !ok {
		return "", sql
	}
	name = strings.TrimSpace(before)
	body = strings.TrimSpace(after)
	body = strings.TrimSuffix(body, ";")
	return name, body
}

func substituteScriptVars(sql string, vars map[string][]string) string {
	out := sql
	for name, vals := range vars {
		if len(vals) == 0 {
			continue
		}
		quoted := make([]string, len(vals))
		for i, s := range vals {
			quoted[i] = fmt.Sprintf("'%s'", strings.ReplaceAll(s, "'", "''"))
		}
		list := strings.Join(quoted, ", ")
		out = strings.ReplaceAll(out, "UNNEST("+name+")", "UNNEST(["+list+"])")
		out = strings.ReplaceAll(out, "UNNEST(`"+name+"`)", "UNNEST(["+list+"])")
	}
	return out
}

func arrayFromRow(rows []bqtypes.Row) []string {
	if len(rows) != 1 || len(rows[0].F) != 1 {
		return nil
	}
	arr, ok := rows[0].F[0].V.([]bqtypes.Cell)
	if !ok {
		return nil
	}
	out := make([]string, 0, len(arr))
	for _, c := range arr {
		if s, ok := c.V.(string); ok {
			out = append(out, s)
		}
	}
	return out
}

func executeScriptStatement(
	ctx context.Context,
	deps Dependencies,
	projectID, defaultDataset, sql string,
	useLegacy bool,
) (*enginepb.TableSchema, []bqtypes.Row, string, string, error) {
	sql, err := query.PrepareEngineSQL(useLegacy, sql, projectID, defaultDataset)
	if err != nil {
		return nil, nil, "", "", err
	}
	engineReq := &enginepb.QueryRequest{
		ProjectId:        projectID,
		DefaultDatasetId: defaultDataset,
		Sql:              sql,
		UseLegacySql:     false,
	}
	stream, err := deps.Query.ExecuteQuery(ctx, engineReq)
	if err != nil {
		return nil, nil, "", "", err
	}
	schema, _, rows, statementType, emulatorRoute, _, streamErr := drainSyncStream(stream)
	if streamErr != nil {
		return nil, nil, "", "", streamErr
	}
	return schema, rows, statementType, emulatorRoute, nil
}

func stampChildJobParent(job *jobs.Job, parentID string) {
	job.ParentJobID = parentID
	job.Statistics.ParentJobID = parentID
}

type scriptExecOutcome struct {
	childCount    int
	finalSchema   *enginepb.TableSchema
	finalRows     []bqtypes.Row
	finalStmtType string
	finalRoute    string
}

// declareToCreateConstant lowers DECLARE to CREATE CONSTANT for the engine's
// AnalyzeNextStatement script loop (DECLARE is script-only parse syntax).
func declareToCreateConstant(stmt string) string {
	trim := trimLeadingSQLComments(strings.TrimSpace(stmt))
	if !strings.HasPrefix(strings.ToUpper(trim), "DECLARE ") {
		return trim
	}
	rest := strings.TrimSpace(trim[8:])
	rest = strings.TrimSuffix(rest, ";")
	defaultPart := ""
	if idx := strings.Index(strings.ToUpper(rest), " DEFAULT "); idx >= 0 {
		defaultPart = strings.TrimSpace(rest[idx+len(" DEFAULT "):])
		rest = strings.TrimSpace(rest[:idx])
	}
	before, after, ok := strings.Cut(rest, " ")
	if !ok {
		return trim
	}
	name := strings.TrimSpace(before)
	typeName := strings.TrimSpace(after)
	if defaultPart != "" {
		return fmt.Sprintf("CREATE CONSTANT %s = %s", name, defaultPart)
	}
	return fmt.Sprintf("CREATE CONSTANT %s = CAST(NULL AS %s)", name, typeName)
}

func transformScriptDeclares(sql string) string {
	inner := unwrapBeginEndBlock(sql)
	// Control-flow scripts must reach googlesql::ScriptExecutor with DECLARE
	// syntax and intact IF/WHILE bodies. Per-statement splitting breaks
	// semicolons inside THEN/ELSE branches.
	if scriptNeedsGoogleSQLExecutor(inner) {
		return strings.TrimSpace(sql)
	}
	parts := splitScriptStatements(inner)
	if len(parts) == 0 {
		return inner
	}
	out := make([]string, 0, len(parts))
	for _, p := range parts {
		out = append(out, declareToCreateConstant(p))
	}
	return strings.Join(out, ";\n")
}

func runLegacySplitScript(
	ctx context.Context,
	deps Dependencies,
	r *http.Request,
	projectID string,
	parent *jobs.Job,
	posted *jobs.Job,
	cfg *jobs.JobConfiguration,
	sql string,
	defaultDataset string,
	useLegacy bool,
) (*scriptExecOutcome, error) {
	vars := make(map[string][]string)
	out := &scriptExecOutcome{}
	for _, raw := range splitScriptStatements(unwrapBeginEndBlock(sql)) {
		st := classifyScriptStatement(raw)
		switch st.kind {
		case scriptStmtDeclare:
			vars[st.name] = nil
			continue
		case scriptStmtCall, scriptStmtSet, scriptStmtQuery:
			stmtSQL := st.sql
			if st.kind == scriptStmtQuery {
				stmtSQL = substituteScriptVars(stmtSQL, vars)
			}
			childPosted := *posted
			childPosted.JobReference.JobID = ""
			childCfg := *cfg
			qCopy := *cfg.Query
			qCopy.Query = stmtSQL
			childCfg.Query = &qCopy
			child := newPendingJob(deps, projectID, &childPosted, &childCfg)
			stampChildJobParent(child, parent.JobReference.JobID)
			childStart := time.Now().UTC()
			schema, rows, statementType, emulatorRoute, err := executeScriptStatement(
				ctx, deps, projectID, defaultDataset, stmtSQL, useLegacy)
			if err != nil {
				return nil, err
			}
			if st.kind == scriptStmtSet && st.name != "" {
				if arr := arrayFromRow(rows); len(arr) > 0 {
					vars[st.name] = arr
				}
			}
			childEnd := time.Now().UTC()
			finalizeDoneJob(deps, child, childStart, childEnd,
				schema, nil, rows, statementType, emulatorRoute, nil, nil, nil, r)
			stampChildJobParent(child, parent.JobReference.JobID)
			out.childCount++
			if st.kind == scriptStmtQuery {
				out.finalSchema = schema
				out.finalRows = rows
				out.finalStmtType = statementType
				out.finalRoute = emulatorRoute
			}
		}
	}
	return out, nil
}

func runScriptStatements(
	ctx context.Context,
	deps Dependencies,
	r *http.Request,
	projectID string,
	parent *jobs.Job,
	posted *jobs.Job,
	cfg *jobs.JobConfiguration,
	sql string,
	defaultDataset string,
	useLegacy bool,
) (*scriptExecOutcome, error) {
	if needsEngineScriptExecution(sql) {
		return runEngineScript(
			ctx, deps, r, projectID, parent, posted, cfg,
			defaultDataset, sql, useLegacy)
	}
	return runLegacySplitScript(
		ctx, deps, r, projectID, parent, posted, cfg,
		sql, defaultDataset, useLegacy)
}

func finalizeScriptParentJob(
	parent *jobs.Job,
	parentStart, parentEnd time.Time,
	out *scriptExecOutcome,
) {
	parent.Status.State = jobs.JobStateDone
	parent.Statistics.StartTime = millisString(parentStart)
	parent.Statistics.EndTime = millisString(parentEnd)
	parent.Statistics.NumChildJobs = strconv.Itoa(out.childCount)
	if out.finalRows != nil || out.finalSchema != nil {
		restSchema := schemaFromProto(out.finalSchema)
		parent.Result = &jobs.QueryResult{
			Schema:        restSchema,
			Rows:          out.finalRows,
			StatementType: out.finalStmtType,
			EmulatorRoute: out.finalRoute,
		}
		if out.finalStmtType != "" {
			parent.Statistics.Query = &bqtypes.JobStatistics2{StatementType: out.finalStmtType}
		}
	}
}

// runSyncScriptQueryInsert executes DECLARE/SET/SELECT scripts and
// registers a parent job plus per-statement child jobs.
func runSyncScriptQueryInsert(
	deps Dependencies,
	w http.ResponseWriter,
	r *http.Request,
	posted *jobs.Job,
	cfg *jobs.JobConfiguration,
) {
	projectID := r.PathValue("projectId")
	parent := newPendingJob(deps, projectID, posted, cfg)
	parentStart := time.Now().UTC()

	useLegacy := false
	if cfg.Query.UseLegacySQL != nil {
		useLegacy = *cfg.Query.UseLegacySQL
	}
	defaultDataset := defaultDatasetID(cfg.Query.DefaultDataset)

	out, err := runScriptStatements(
		r.Context(), deps, r, projectID, parent, posted, cfg,
		cfg.Query.Query, defaultDataset, useLegacy)
	if err != nil {
		finalizeFailedJob(deps, parent, parentStart, err)
		if queryGRPCToHTTPError(w, err) {
			return
		}
		writeError(w, http.StatusBadRequest, reasonInvalidQuery, err.Error())
		return
	}
	finalizeScriptParentJob(parent, parentStart, time.Now().UTC(), out)
	writeJSON(w, http.StatusOK, parent)
}

// runQueryScriptExecute handles the jobs.query path for multi-statement
// scripts (client.query uses jobs.query when the request body is simple).
func runQueryScriptExecute(
	deps Dependencies,
	w http.ResponseWriter,
	r *http.Request,
	req *bqtypes.QueryRequest,
	defaultDataset string,
) {
	projectID := r.PathValue("projectId")
	parentStart := time.Now().UTC()
	posted := &jobs.Job{JobReference: bqtypes.JobReference{
		ProjectID: projectID,
		Location:  req.Location,
	}}
	cfg := &jobs.JobConfiguration{
		JobType: jobConfigurationKindQuery,
		Query:   &jobs.JobConfigurationQuery{Query: req.Query},
	}
	parent := newPendingJob(deps, projectID, posted, cfg)

	useLegacy := req.UseLegacySQL != nil && *req.UseLegacySQL
	out, err := runScriptStatements(
		r.Context(), deps, r, projectID, parent, posted, cfg,
		req.Query, defaultDataset, useLegacy)
	if err != nil {
		finalizeFailedJob(deps, parent, parentStart, err)
		if queryGRPCToHTTPError(w, err) {
			return
		}
		writeError(w, http.StatusBadRequest, reasonInvalidQuery, err.Error())
		return
	}
	parentEnd := time.Now().UTC()
	finalizeScriptParentJob(parent, parentStart, parentEnd, out)
	restSchema := schemaFromProto(out.finalSchema)
	visibleRoute := ""
	if middleware.IsLoopback(r.Context()) {
		visibleRoute = out.finalRoute
	}
	sessionInfo := sessionStore(&deps).Resolve(
		projectID, req.Location, req.CreateSession, req.ConnProperties)
	stampJobSessionInfo(parent, sessionInfo)
	outResp := assembleQueryResponse(
		parent, restSchema, out.finalRows, nil, nil,
		out.finalStmtType, visibleRoute, nil, nil, sessionInfo)
	writeJSON(w, http.StatusOK, outResp)
}
