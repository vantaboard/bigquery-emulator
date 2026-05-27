package seed

import (
	"encoding/json"
	"errors"
	"fmt"
	"os"
	"strings"
	"time"
)

// SeedRequest is the JSON body the seed API accepts on
// `POST /api/emulator/seed`. The contract mirrors what
// go-googlesql's emulatorseed package exposes (see
// /home/brighten-tompkins/Code/go-googlesql/api/emulatorseed/types.go)
// so operators with existing tooling can swap one emulator for the
// other without rewriting their request bodies.
type SeedRequest struct {
	// Source is the production-side resource the seeder reads
	// from. Required.
	Source SeedEndpointRef `json:"source"`

	// Destination is the emulator-side resource the seeder writes
	// into. When omitted, the seeder mirrors Source 1:1 (same
	// project/dataset/table ids on this emulator).
	Destination *SeedDestinationRef `json:"destination,omitempty"`

	// MaxRowsPerTable bounds the number of rows the seeder will
	// copy from any single source table. Zero / negative means
	// "no limit". This is the dominant safety knob -- operators
	// trying to mirror billion-row tables into a local emulator
	// should set this aggressively.
	MaxRowsPerTable int64 `json:"maxRowsPerTable,omitempty"`

	// BillingProject is the GCP project the BigQuery jobs the
	// production read issues are billed against. When omitted
	// the seeder falls back through the documented chain (see
	// ResolveBillingProject).
	BillingProject string `json:"billingProject,omitempty"`
}

// SeedEndpointRef names a production resource. Either Project (full
// project scope), Project+Dataset (dataset scope), or
// Project+Dataset+Table (single-table scope) is supported. Source
// requests with no Project are rejected up front -- BigQuery has no
// well-defined notion of "default project" on the wire.
type SeedEndpointRef struct {
	Project string `json:"project"`
	Dataset string `json:"dataset,omitempty"`
	Table   string `json:"table,omitempty"`
}

// SeedDestinationRef is the same shape as SeedEndpointRef but the
// dataset/table fields are optional remappings. When omitted the
// seeder mirrors the source name verbatim; when set it copies
// `Source.Project.Source.Dataset.Source.Table` into
// `Destination.Project.Destination.Dataset.Destination.Table`.
type SeedDestinationRef struct {
	Project string `json:"project,omitempty"`
	Dataset string `json:"dataset,omitempty"`
	Table   string `json:"table,omitempty"`
}

// SeedResult is what we report back to the caller once an operation
// finishes. Counters mirror the go-googlesql shape so dashboards /
// scripts that already consume that emulator's seed response can
// read this one without changes.
type SeedResult struct {
	// Started / Finished are RFC 3339 timestamps.
	Started  string `json:"started"`
	Finished string `json:"finished,omitempty"`

	// DatasetsCreated counts datasets the seeder added to the
	// emulator on this run. Idempotent reruns surface 0 here and
	// a positive DatasetsSkipped.
	DatasetsCreated int `json:"datasetsCreated"`
	DatasetsSkipped int `json:"datasetsSkipped"`

	// TablesCreated / TablesSkipped are the same shape for
	// physical-table resources. Views, materialized views,
	// external tables, and routines all fold into
	// ResourceErrors (one entry per unsupported resource) for
	// the initial integration; see ROADMAP for the support
	// matrix.
	TablesCreated int `json:"tablesCreated"`
	TablesSkipped int `json:"tablesSkipped"`

	// RowsCopied is the wall-total of rows the seeder
	// successfully inserted into the emulator across every
	// destination table this operation touched.
	RowsCopied int64 `json:"rowsCopied"`

	// ResourceErrors holds per-resource failures. The operation
	// itself can still finish "DONE" while individual tables
	// failed -- this is how go-googlesql ships partial-failure
	// data without forcing the caller to retry the entire
	// scope.
	ResourceErrors []SeedResourceError `json:"resourceErrors,omitempty"`
}

// SeedResourceError captures a per-resource failure (one table, one
// view, one routine, ...). The presence of any non-empty Error in
// ResourceErrors is what the operation polling endpoint surfaces in
// the public Operation.error field; the operation as a whole only
// fails when something catastrophic happens (the production project
// is unreachable, ADC credentials are missing, etc).
type SeedResourceError struct {
	// Resource is a human-readable identifier
	// ("dataset:proj.ds", "table:proj.ds.tbl", "view:proj.ds.v", ...).
	Resource string `json:"resource"`
	// Kind classifies why the failure happened
	// ("unsupported", "rpc", "read", "write", "skipped").
	Kind  string `json:"kind"`
	Error string `json:"error"`
}

// Validate runs cheap input checks the orchestrator depends on
// before it touches the network. Returns ErrInvalidRequest with a
// human-readable message so the HTTP handler can surface a 400 with
// the right reason.
func (r *SeedRequest) Validate() error {
	if r == nil {
		return fmt.Errorf("%w: nil request body", ErrInvalidRequest)
	}
	if strings.TrimSpace(r.Source.Project) == "" {
		return fmt.Errorf("%w: source.project is required", ErrInvalidRequest)
	}
	if r.Source.Table != "" && r.Source.Dataset == "" {
		return fmt.Errorf("%w: source.table requires source.dataset", ErrInvalidRequest)
	}
	if r.Destination != nil {
		if r.Destination.Table != "" && r.Destination.Dataset == "" {
			return fmt.Errorf("%w: destination.table requires destination.dataset", ErrInvalidRequest)
		}
		if r.Source.Dataset == "" && r.Destination.Dataset != "" {
			return fmt.Errorf(
				"%w: destination.dataset requires source.dataset (cannot remap a project-scope seed to a single dataset)",
				ErrInvalidRequest,
			)
		}
		if r.Source.Table == "" && r.Destination.Table != "" {
			return fmt.Errorf("%w: destination.table requires source.table", ErrInvalidRequest)
		}
	}
	if r.MaxRowsPerTable < 0 {
		return fmt.Errorf("%w: maxRowsPerTable must be >= 0", ErrInvalidRequest)
	}
	return nil
}

// Env var names walked by ResolveBillingProject's fallback chain.
// Exported as package-level constants so tests and callers reference
// the same strings the implementation looks up.
const (
	EnvGoogleCloudQuotaProject = "GOOGLE_CLOUD_QUOTA_PROJECT"
	EnvGoogleCloudProject      = "GOOGLE_CLOUD_PROJECT"
	EnvGcloudProject           = "GCLOUD_PROJECT"
)

// billingEnvChain is the documented env-var fallback order
// ResolveBillingProject walks; pulled into a package-level var so
// the order stays self-documenting and tests assert against the
// same source of truth.
var billingEnvChain = []string{
	EnvGoogleCloudQuotaProject,
	EnvGoogleCloudProject,
	EnvGcloudProject,
}

// ResolveBillingProject picks the GCP project the seeder bills its
// production reads against. The fallback chain mirrors what
// go-googlesql documents (and what gcloud's tooling follows):
//
//  1. Request body's `billingProject`.
//  2. Gateway default project (--project-id).
//  3. $GOOGLE_CLOUD_QUOTA_PROJECT.
//  4. $GOOGLE_CLOUD_PROJECT.
//  5. $GCLOUD_PROJECT.
//  6. Source project (the read project itself).
//
// `getenv` is injected so tests don't depend on os.Environ; the
// production caller passes os.LookupEnv. A nil getenv is treated
// as "no env vars set" so production code that forgot to pass one
// still gets sane behavior.
func ResolveBillingProject(req SeedRequest, gatewayDefault string, getenv func(string) (string, bool)) string {
	if v := strings.TrimSpace(req.BillingProject); v != "" {
		return v
	}
	if v := strings.TrimSpace(gatewayDefault); v != "" {
		return v
	}
	if getenv == nil {
		return strings.TrimSpace(req.Source.Project)
	}
	for _, key := range billingEnvChain {
		if v, ok := getenv(key); ok && strings.TrimSpace(v) != "" {
			return strings.TrimSpace(v)
		}
	}
	return strings.TrimSpace(req.Source.Project)
}

// ErrInvalidRequest is the sentinel SeedRequest.Validate wraps so
// callers can detect "this is a 400, not a 500" without string
// matching. Compare via errors.Is.
var ErrInvalidRequest = errors.New("seed: invalid request")

// ErrProductionUnsupported is returned by NewProductionOrchestrator
// when the build does not include a live production client. The
// initial integration intentionally ships without a hard dependency
// on cloud.google.com/go/bigquery so operators who only need the
// YAML seed loader (or the per-PR test runner) don't pay for it; a
// future build tag will swap in the real implementation.
var ErrProductionUnsupported = errors.New("seed: production seeding is not compiled into this build")

// LookupEnvOrEmpty is the production env lookup used by the seed
// handler when it needs to consult the documented env chain (see
// ResolveBillingProject). Pulled out so tests can stub it via the
// type alias rather than the global.
var LookupEnvOrEmpty = os.LookupEnv

// nowRFC3339 returns the current UTC time in the format the
// SeedResult timestamps use. Wrapped in a package-level var so
// tests can pin time.
var nowRFC3339 = func() string {
	return time.Now().UTC().Format(time.RFC3339)
}

// DecodeRequest parses a JSON request body into a SeedRequest. The
// helper exists so the handler doesn't have to know whether to
// configure json.Decoder.DisallowUnknownFields (we do, so a typo
// like "billing_project" surfaces as a 400 rather than silently
// ignoring the operator's intent).
func DecodeRequest(b []byte) (SeedRequest, error) {
	var req SeedRequest
	dec := json.NewDecoder(strings.NewReader(string(b)))
	dec.DisallowUnknownFields()
	if err := dec.Decode(&req); err != nil {
		return SeedRequest{}, fmt.Errorf("%w: %w", ErrInvalidRequest, err)
	}
	return req, nil
}
