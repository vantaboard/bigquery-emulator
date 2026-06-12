package handlers

import (
	"net/http"
	"net/url"
	"strconv"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
	"github.com/vantaboard/bigquery-emulator/gateway/jobs"
	"github.com/vantaboard/bigquery-emulator/gateway/middleware"
)

func queryResultFields(job *jobs.Job) (
	schema *bqtypes.TableSchema,
	allRows []bqtypes.Row,
	dmlStats *bqtypes.DmlStats,
	statementType string,
	emulatorRoute string,
	emulatorPhases map[string]int64,
	ddlTargetRoutine *bqtypes.RoutineReference,
) {
	if result := job.Result; result != nil {
		schema = result.Schema
		allRows = result.Rows
		dmlStats = result.DmlStats
		statementType = result.StatementType
		emulatorRoute = result.EmulatorRoute
		emulatorPhases = result.EmulatorPhases
		ddlTargetRoutine = result.DdlTargetRoutine
	}
	return schema, allRows, dmlStats, statementType, emulatorRoute, emulatorPhases, ddlTargetRoutine
}

func getQueryResultsStatistics(
	r *http.Request,
	statementType string,
	emulatorRoute string,
	emulatorPhases map[string]int64,
	ddlTargetRoutine *bqtypes.RoutineReference,
	sessionInfo *bqtypes.SessionInfo,
) *bqtypes.JobStatistics {
	visibleRoute := ""
	visiblePhases := map[string]int64(nil)
	if middleware.IsLoopback(r.Context()) {
		visibleRoute = emulatorRoute
		visiblePhases = emulatorPhases
	}
	if statementType == "" && visibleRoute == "" && len(visiblePhases) == 0 &&
		ddlTargetRoutine == nil && sessionInfo == nil {
		return nil
	}
	stats := &bqtypes.JobStatistics{SessionInfo: sessionInfo}
	if statementType != "" || visibleRoute != "" || len(visiblePhases) > 0 || ddlTargetRoutine != nil {
		stats.Query = &bqtypes.JobStatistics2{
			StatementType:    statementType,
			EmulatorRoute:    visibleRoute,
			EmulatorPhases:   visiblePhases,
			DdlTargetRoutine: ddlTargetRoutine,
		}
	}
	return stats
}

func applyDmlStatsToGetQueryResults(out *bqtypes.QueryResponse, dmlStats *bqtypes.DmlStats) {
	out.DmlStats = dmlStats
	inserted, _ := strconv.ParseInt(dmlStats.InsertedRowCount, 10, 64)
	updated, _ := strconv.ParseInt(dmlStats.UpdatedRowCount, 10, 64)
	deleted, _ := strconv.ParseInt(dmlStats.DeletedRowCount, 10, 64)
	out.NumDmlAffectedRows = strconv.FormatInt(inserted+updated+deleted, 10)
	out.Schema = nil
	out.Rows = nil
	out.TotalRows = "0"
}

// defaultQueryResultsPageSize mirrors BigQuery's documented default
// `maxResults` for jobs.getQueryResults when the caller omits it.
const defaultQueryResultsPageSize uint64 = 10000

// paginateResults slices cached query rows using startIndex,
// maxResults, and pageToken. pageToken (when set) is a decimal string
// encoding the next start row index, matching tabledata.list.
func paginateResults(allRows []bqtypes.Row, q url.Values) ([]bqtypes.Row, string) {
	total := uint64(len(allRows))
	start := parseUintQuery(q, "startIndex", 0)
	if tok := q.Get("pageToken"); tok != "" {
		if off, err := strconv.ParseUint(tok, 10, 64); err == nil {
			start = off
		} else {
			return nil, ""
		}
	}
	limit := defaultQueryResultsPageSize
	if q.Get("maxResults") != "" {
		limit = parseUintQuery(q, "maxResults", defaultQueryResultsPageSize)
	}
	// maxResults=0 means "wait for completion, return zero rows" (browseTable
	// sample). Never mint a pageToken in that case or Node polls forever.
	if limit == 0 {
		return nil, ""
	}
	if start >= total {
		return nil, ""
	}
	end := min(start+limit, total)
	var nextToken string
	if end < total {
		nextToken = strconv.FormatUint(end, 10)
	}
	return allRows[start:end], nextToken
}

// parseUintQuery returns the named query parameter as a uint64,
// falling back to defaultVal when the value is missing or unparsable.
// Pulled out so the pagination helper stops nesting if-inside-if.
func parseUintQuery(q url.Values, key string, defaultVal uint64) uint64 {
	s := q.Get(key)
	if s == "" {
		return defaultVal
	}
	v, err := strconv.ParseUint(s, 10, 64)
	if err != nil {
		return defaultVal
	}
	return v
}
