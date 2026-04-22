// Package execution identifies the physical SQL engine used by go-googlesqlite (SQLite vs DuckDB).
package execution

import (
	"fmt"
	"strings"
)

// Backend selects which database/sql driver name the emulator opens (googlesqlite vs googlesqlduck).
type Backend string

const (
	BackendSQLite Backend = "sqlite"
	BackendDuckDB Backend = "duckdb"
)

// ParseBackend normalizes CLI/env values. Empty string defaults to SQLite.
func ParseBackend(s string) (Backend, error) {
	switch strings.ToLower(strings.TrimSpace(s)) {
	case "", "sqlite":
		return BackendSQLite, nil
	case "duckdb":
		return BackendDuckDB, nil
	default:
		return "", fmt.Errorf("unknown execution backend %q (want sqlite or duckdb)", s)
	}
}
