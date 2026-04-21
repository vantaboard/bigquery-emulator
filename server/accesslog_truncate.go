package server

import (
	"os"
	"strconv"
	"strings"
)

const accessLogSQLPreviewMaxDefault = 256

// accessLogSQLPreviewMax returns the max rune length for job_sql_preview on access log lines
// (jobs.getQueryResults, etc.). BQ_EMULATOR_ACCESS_LOG_SQL_PREVIEW_MAX: empty/unset uses 256;
// 0 means no limit (log full string; not recommended for large SQL).
func accessLogSQLPreviewMax() int {
	s := strings.TrimSpace(os.Getenv("BQ_EMULATOR_ACCESS_LOG_SQL_PREVIEW_MAX"))
	if s == "" {
		return accessLogSQLPreviewMaxDefault
	}
	n, err := strconv.Atoi(s)
	if err != nil {
		return accessLogSQLPreviewMaxDefault
	}
	return n
}

// truncateStringForAccessLog shortens a string for structured logging. Truncation is by bytes
// on the string prefix, matching existing truncateAsyncJobQueryForLog / truncateQueryForLog
// style (Go string slice is per-byte for ASCII SQL).
func truncateStringForAccessLog(s string, max int) string {
	if max <= 0 || len(s) <= max {
		return s
	}
	return s[:max] + "…"
}
