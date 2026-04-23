package contentdata

import (
	"errors"
	"strings"
)

// IsMissingDDLObjectError reports whether err indicates the physical table/view
// was already absent (DuckDB catalog, SQLite, etc.). Callers may map this to HTTP 404.
func IsMissingDDLObjectError(err error) bool {
	for err != nil {
		if isMissingDDLObjectMessage(err.Error()) {
			return true
		}
		err = errors.Unwrap(err)
	}
	return false
}

func isMissingDDLObjectMessage(msg string) bool {
	if strings.Contains(msg, "no such table") {
		return true
	}
	if strings.Contains(msg, "Catalog Error") && strings.Contains(msg, "does not exist") {
		return true
	}
	// go-googlesql-engine: DROP succeeded or table already gone but catalog key mismatch — treat as absent.
	if strings.Contains(msg, "failed to find table spec from map") {
		return true
	}
	return false
}
