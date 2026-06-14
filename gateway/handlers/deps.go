package handlers

import (
	"github.com/vantaboard/bigquery-emulator/gateway/engine"
	"github.com/vantaboard/bigquery-emulator/gateway/external"
	"github.com/vantaboard/bigquery-emulator/gateway/external/sourceconfig"
	"github.com/vantaboard/bigquery-emulator/gateway/jobs"
)

// DepsOptions carries gateway-level settings threaded into handler deps.
type DepsOptions struct {
	DataDir string
}

// BuildDependencies constructs the shared handler dependency bundle used by
// both the REST gateway and the public gRPC surface. When eng is nil
// (gateway-only mode / unit tests) Catalog and Query stay nil and
// handlers fall back to their NotImplemented stubs.
func BuildDependencies(eng *engine.Client) Dependencies {
	return BuildDependenciesWith(eng, DepsOptions{})
}

// BuildDependenciesWith constructs deps with optional data-dir / external config.
func BuildDependenciesWith(eng *engine.Client, opts DepsOptions) Dependencies {
	var extCfg *sourceconfig.Config
	if opts.DataDir != "" {
		if c, err := sourceconfig.Load(opts.DataDir); err == nil {
			extCfg = c
		}
	}
	deps := Dependencies{
		Jobs:            jobs.NewRegistry(),
		Metadata:        NewMetadataStore(),
		Snapshots:       NewSnapshotStore(),
		Routines:        NewRoutineStore(),
		Models:          NewModelStore(),
		Sessions:        NewSessionStore(),
		DataDir:         opts.DataDir,
		ExternalSources: extCfg,
	}
	if eng != nil {
		deps.Catalog = eng.Catalog
		deps.Query = eng.Query
	}
	return deps
}

// externalResolver returns the materialization resolver for deps.
func externalResolver(deps Dependencies) *external.Resolver {
	return external.NewResolver(deps.ExternalSources)
}
