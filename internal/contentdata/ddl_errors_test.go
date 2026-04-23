package contentdata

import (
	"fmt"
	"testing"
)

func TestIsMissingDDLObjectError(t *testing.T) {
	duck := fmt.Errorf("wrap: %w", fmt.Errorf("Catalog Error: Table with name x does not exist!"))
	if !IsMissingDDLObjectError(duck) {
		t.Fatal("expected duckdb missing table")
	}
	sqlite := fmt.Errorf("wrap: %w", fmt.Errorf("SQLITE_UNKNOWN: no such table: foo"))
	if !IsMissingDDLObjectError(sqlite) {
		t.Fatal("expected sqlite missing table")
	}
	internal := fmt.Errorf("failed to find table spec from map")
	if IsMissingDDLObjectError(internal) {
		t.Fatal("catalog map miss must not map to HTTP 404 (avoid rollback + ghost physical tables)")
	}
}
