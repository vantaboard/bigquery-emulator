package server

import (
	"database/sql"
	"fmt"
	"strings"

	"github.com/vantaboard/bigquery-emulator/internal/execution"
)

const googlesqlDuckDriverName = "googlesqlduck"

func duckDriverRegistered() bool {
	for _, d := range sql.Drivers() {
		if d == googlesqlDuckDriverName {
			return true
		}
	}
	return false
}

// duckDSNFromStorage maps emulator Storage to a DuckDB DSN (path or empty for in-memory).
// See [MemoryStorage] and TempStorage resolution in [New] (file:path?cache=shared).
func duckDSNFromStorage(s Storage) string {
	str := string(s)
	if str == string(MemoryStorage) {
		return ""
	}
	const prefix = "file:"
	if !strings.HasPrefix(str, prefix) {
		return str
	}
	rest := strings.TrimPrefix(str, prefix)
	// file::memory:?cache=shared
	if strings.HasPrefix(rest, ":") {
		return ""
	}
	if i := strings.Index(rest, "?"); i >= 0 {
		rest = rest[:i]
	}
	return rest
}

func openExecutionDB(b execution.Backend, storage Storage) (*sql.DB, error) {
	switch b {
	case execution.BackendSQLite:
		dsn := string(storageWithSQLiteDefaults(storage))
		return sql.Open("googlesqlite", dsn)
	case execution.BackendDuckDB:
		if !duckDriverRegistered() {
			return nil, fmt.Errorf(
				`execution backend "duckdb" requires a binary built with -tags duckdb `+
					`(and duckdb_use_lib + libduckdb on the loader path when using dynamic linking); `+
					`driver %q is not registered`,
				googlesqlDuckDriverName,
			)
		}
		dsn := duckDSNFromStorage(storage)
		return sql.Open(googlesqlDuckDriverName, dsn)
	default:
		return nil, fmt.Errorf("unsupported execution backend %q", b)
	}
}
