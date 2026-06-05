package handlers

import (
	"net/http"
	"regexp"
	"strings"
	"time"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
	"github.com/vantaboard/bigquery-emulator/gateway/jobs"
)

// abortSessionRE matches `CALL BQ.ABORT_SESSION([session_id])` system
// procedure calls bigframes issues when closing a session.
var abortSessionRE = regexp.MustCompile(
	`(?is)^\s*CALL\s+BQ\.ABORT_SESSION\s*(?:\(\s*(?:'([^']*)'|SESSION_ID\s*\(\s*\))?\s*\))?\s*;?\s*$`)

func parseAbortSessionSQL(sql string) bool {
	return abortSessionRE.MatchString(strings.TrimSpace(sql))
}

// handleAbortSessionQuery is a no-op stub for BQ.ABORT_SESSION so bigframes
// session teardown succeeds against the emulator.
func handleAbortSessionQuery(
	deps Dependencies,
	w http.ResponseWriter,
	projectID, location string,
	connProps []bqtypes.ConnectionProperty,
) {
	start := time.Now().UTC()
	end := start
	sessionInfo := sessionStore(&deps).Resolve(projectID, location, false, connProps)
	job := deps.Jobs.CompleteQueryWithResult(projectID, location, 0, start, end, &jobs.QueryResult{})
	stampJobSessionInfo(job, sessionInfo)
	out := assembleQueryResponse(job, nil, nil, nil, nil, "", "", nil, sessionInfo)
	writeJSON(w, http.StatusOK, out)
}
