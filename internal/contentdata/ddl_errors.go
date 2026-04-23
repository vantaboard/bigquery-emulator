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
	// DuckDB missing table — require this shape so we do not treat arbitrary Catalog Error text
	// (or wrong DROP identifiers) as HTTP 404 while the physical table still exists.
	if strings.Contains(msg, "Table with name") && strings.Contains(msg, "does not exist") {
		return true
	}
	return false
}
