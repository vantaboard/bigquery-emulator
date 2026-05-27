package gateway

import (
	"context"
	"os"

	"github.com/vantaboard/bigquery-emulator/gateway/engine"
	"github.com/vantaboard/bigquery-emulator/gateway/seed"
)

// newSeedRunner constructs the production seed.Runner the
// `POST /api/emulator/seed` handler dispatches to when
// EnableSeedAPI is true.
//
// The default build returns a runner whose underlying
// ProductionReader is the "unsupported" stub
// (seed.NewProductionReader without a build tag), so the seed
// handler still surfaces a clean 501 instead of a panic when the
// operator forgot to build with `-tags=seed_production_live`. The
// CatalogClient half is always real -- it's what the gateway
// already uses for every REST handler.
//
// The returned Runner closes over the engine client; callers must
// keep the engine subprocess alive for as long as they expect to
// service seed requests.
func newSeedRunner(opts Options, eng *engine.Client) seed.Runner {
	defaults := DefaultsFromOptions(opts)
	return &lazyProductionRunner{
		defaults:  defaults,
		applier:   seed.NewCatalogApplier(eng.Catalog),
		envLookup: os.LookupEnv,
	}
}

// DefaultsFromOptions projects the seeding-relevant fields of an
// Options struct into the small seed.Defaults shape the orchestrator
// and YAML loader consume. Lives in the gateway package (rather
// than gateway/seed) so the seed package never imports the gateway
// package, which would create an import cycle through the route
// registration in gateway/server.go.
func DefaultsFromOptions(o Options) seed.Defaults {
	return seed.Defaults{
		ProjectID:       o.DefaultProjectID,
		DatasetLocation: o.DefaultDatasetLocation,
	}
}

// lazyProductionRunner defers ProductionReader construction until
// the first request. That keeps gateway startup time cheap when
// nobody actually invokes the seed API and lets us surface ADC /
// quota / billing errors as a per-operation failure rather than a
// hard fail at gateway boot.
type lazyProductionRunner struct {
	defaults  seed.Defaults
	applier   seed.Applier
	envLookup func(string) (string, bool)
}

// Run satisfies seed.Runner. The first invocation constructs the
// production reader (which fails with ErrProductionUnsupported in
// the default build); subsequent invocations get their own reader
// so the cloud client's connection lifecycle stays scoped to one
// request (matches what go-googlesql's orchestrator does).
func (l *lazyProductionRunner) Run(ctx context.Context, req seed.SeedRequest) (*seed.SeedResult, error) {
	billing := seed.ResolveBillingProject(req, l.defaults.ProjectID, l.envLookup)
	reader, err := seed.NewProductionReader(ctx, billing, l.envLookup)
	if err != nil {
		return nil, err
	}
	orch := seed.NewOrchestrator(reader, l.applier, l.defaults)
	orch.EnvLookup = l.envLookup
	return orch.Run(ctx, req)
}
