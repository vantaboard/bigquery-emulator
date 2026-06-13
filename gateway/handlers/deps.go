package handlers

import (
	"github.com/vantaboard/bigquery-emulator/gateway/engine"
	"github.com/vantaboard/bigquery-emulator/gateway/jobs"
)

// BuildDependencies constructs the shared handler dependency bundle used by
// both the REST gateway and the public gRPC surface. When eng is nil
// (gateway-only mode / unit tests) Catalog and Query stay nil and
// handlers fall back to their NotImplemented stubs.
func BuildDependencies(eng *engine.Client) Dependencies {
	deps := Dependencies{
		Jobs:      jobs.NewRegistry(),
		Metadata:  NewMetadataStore(),
		Snapshots: NewSnapshotStore(),
		Routines:  NewRoutineStore(),
		Models:    NewModelStore(),
		Sessions:  NewSessionStore(),
	}
	if eng != nil {
		deps.Catalog = eng.Catalog
		deps.Query = eng.Query
	}
	return deps
}
