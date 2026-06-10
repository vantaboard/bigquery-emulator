package seed

import (
	"context"
	"errors"
	"fmt"
	"strings"

	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
)

// ProductionReader is the narrow surface the orchestrator drives
// against the live production BigQuery side. The concrete
// implementation (gateway/seed/production_live.go, gated behind the
// `seed_production_live` build tag) wraps cloud.google.com/go/bigquery
// so the orchestrator package itself never pulls in the heavy cloud
// dependency tree unless the operator explicitly opts in.
//
// Tests stub this interface to feed deterministic metadata and rows
// without a network round-trip.
type ProductionReader interface {
	// ListDatasets enumerates the datasets in `projectID` that
	// are visible to the calling principal. Implementations may
	// stream results (cloud.google.com/go/bigquery's iterator does)
	// but the gateway-side API stays slice-shaped so the test
	// fakes are easy to write.
	ListDatasets(ctx context.Context, projectID string) ([]ProductionDataset, error)

	// ListTables enumerates tables, views, and external tables
	// inside `projectID.datasetID`. Each entry carries enough
	// metadata for the orchestrator to decide whether the table
	// is supported (physical) or needs a SeedResourceError
	// (view, model, ...).
	ListTables(ctx context.Context, projectID, datasetID string) ([]ProductionTable, error)

	// DescribeTable returns the physical schema for one table.
	// Only called for entries ListTables reported as
	// supportable (physical tables, snapshots).
	DescribeTable(ctx context.Context, projectID, datasetID, tableID string) (*enginepb.TableSchema, error)

	// ReadRows pages through one table's rows. The orchestrator
	// uses maxRows (the request's MaxRowsPerTable knob) to cap
	// the read; an implementation may stop early.
	ReadRows(ctx context.Context, projectID, datasetID, tableID string, maxRows int64) ([]map[string]any, error)
}

// ProductionDataset is the slimmed-down view of a BigQuery dataset
// the orchestrator needs. The fields map 1:1 to the dataset
// resource in cloud.google.com/go/bigquery so the live adapter is
// a thin lift.
type ProductionDataset struct {
	ProjectID string
	DatasetID string
	Location  string
}

// ProductionTable similarly carries the fields the orchestrator
// reads off the cloud client's TableMetadata.
type ProductionTable struct {
	ProjectID string
	DatasetID string
	TableID   string
	// Type is the cloud library's table-type string ("TABLE",
	// "VIEW", "MATERIALIZED_VIEW", "EXTERNAL", "MODEL", ...).
	// Anything other than "TABLE" lands in ResourceErrors with
	// kind=unsupported until the engine learns to persist them.
	Type string
}

// Orchestrator wires a ProductionReader to an Applier and runs one
// SeedRequest through them. It is the production-side
// implementation of Runner.
type Orchestrator struct {
	Reader   ProductionReader
	Applier  Applier
	Defaults Defaults

	// EnvLookup is consulted when ResolveBillingProject walks
	// the env fallback chain. Defaults to os.LookupEnv when nil
	// so production calls work without explicit wiring.
	EnvLookup func(string) (string, bool)
}

// NewOrchestrator constructs an Orchestrator with sensible defaults.
// reader must be non-nil; the constructor panics rather than letting
// the caller pass nil here because a nil reader produces confusing
// "method on nil receiver" failures deep inside Run.
func NewOrchestrator(reader ProductionReader, applier Applier, defaults Defaults) *Orchestrator {
	if reader == nil {
		panic("seed: NewOrchestrator: reader must be non-nil; pass NewProductionReader or a test stub")
	}
	if applier == nil {
		panic("seed: NewOrchestrator: applier must be non-nil")
	}
	return &Orchestrator{
		Reader:    reader,
		Applier:   applier,
		Defaults:  defaults,
		EnvLookup: LookupEnvOrEmpty,
	}
}

// Run executes one SeedRequest. The request must have already
// passed SeedRequest.Validate; the orchestrator double-checks
// anyway so a malformed request from a non-HTTP caller surfaces as
// ErrInvalidRequest rather than a panic.
//
// The returned SeedResult is always non-nil on a successful Run --
// individual resource failures accumulate in Result.ResourceErrors
// rather than aborting the whole operation. Run returns a non-nil
// error only when the entire seed cannot proceed (missing creds,
// project doesn't exist, list RPC failed).
func (o *Orchestrator) Run(ctx context.Context, req SeedRequest) (*SeedResult, error) {
	if err := req.Validate(); err != nil {
		return nil, err
	}
	result := &SeedResult{Started: nowRFC3339()}
	// BillingProject is currently consumed by the live reader;
	// computing it here lets us validate the fallback chain even
	// when tests don't go through the live adapter.
	_ = ResolveBillingProject(req, o.Defaults.ProjectID, o.envLookup())

	dest := destinationOf(req)
	switch {
	case req.Source.Table != "":
		o.seedTable(ctx, req.Source.Project, req.Source.Dataset, req.Source.Table,
			dest.Project, dest.Dataset, dest.Table,
			req.MaxRowsPerTable, result)
	case req.Source.Dataset != "":
		o.seedDataset(ctx, req.Source.Project, req.Source.Dataset,
			dest.Project, dest.Dataset, req.MaxRowsPerTable, result)
	default:
		o.seedProject(ctx, req.Source.Project, dest.Project, req.MaxRowsPerTable, result)
	}
	result.Finished = nowRFC3339()
	return result, nil
}

// envLookup returns the orchestrator's configured lookup or a
// no-op when unset. Centralizing the nil-check keeps Run readable.
func (o *Orchestrator) envLookup() func(string) (string, bool) {
	if o.EnvLookup != nil {
		return o.EnvLookup
	}
	return func(string) (string, bool) { return "", false }
}

// seedProject copies every dataset under sourceProject into
// destProject. A ListDatasets failure is the only error path that
// short-circuits the entire op; per-dataset failures fold into
// ResourceErrors.
func (o *Orchestrator) seedProject(
	ctx context.Context,
	sourceProject, destProject string,
	maxRows int64,
	result *SeedResult,
) {
	datasets, err := o.Reader.ListDatasets(ctx, sourceProject)
	if err != nil {
		result.ResourceErrors = append(result.ResourceErrors, SeedResourceError{
			Resource: "project:" + sourceProject,
			Kind:     resourceKindRPC,
			Error:    fmt.Sprintf("ListDatasets: %v", err),
		})
		return
	}
	for _, ds := range datasets {
		o.seedDataset(ctx, sourceProject, ds.DatasetID, destProject, ds.DatasetID, maxRows, result)
	}
}

// seedDataset copies one source dataset into the destination
// project + dataset name. Caller is responsible for choosing the
// destination dataset id (mirror or override).
func (o *Orchestrator) seedDataset(
	ctx context.Context,
	sourceProject, sourceDataset, destProject, destDataset string,
	maxRows int64,
	result *SeedResult,
) {
	if destDataset == "" {
		destDataset = sourceDataset
	}
	location := ""
	if dsList, _ := o.Reader.ListDatasets(ctx, sourceProject); len(dsList) > 0 {
		for _, d := range dsList {
			if d.DatasetID == sourceDataset {
				location = d.Location
				break
			}
		}
	}
	if location == "" {
		location = o.Defaults.DatasetLocation
	}
	created, err := o.Applier.EnsureDataset(ctx, destProject, destDataset, location)
	if err != nil {
		result.ResourceErrors = append(result.ResourceErrors, SeedResourceError{
			Resource: fmt.Sprintf("dataset:%s.%s", destProject, destDataset),
			Kind:     resourceKindWrite,
			Error:    err.Error(),
		})
		return
	}
	if created {
		result.DatasetsCreated++
	} else {
		result.DatasetsSkipped++
	}

	tables, err := o.Reader.ListTables(ctx, sourceProject, sourceDataset)
	if err != nil {
		result.ResourceErrors = append(result.ResourceErrors, SeedResourceError{
			Resource: fmt.Sprintf("dataset:%s.%s", sourceProject, sourceDataset),
			Kind:     resourceKindRPC,
			Error:    fmt.Sprintf("ListTables: %v", err),
		})
		return
	}
	for _, tbl := range tables {
		o.seedTable(ctx, sourceProject, sourceDataset, tbl.TableID,
			destProject, destDataset, tbl.TableID, maxRows, result)
	}
}

// seedTable copies a single source table into the destination.
// Unsupported source types (views, materialized views, models,
// external) are reported as ResourceErrors without aborting the
// surrounding seed.
//
// The body is split across resolveSourceTable / writeTableMetadata
// / copyTableRows helpers; the function reads top-down and never
// produces partial state because each helper short-circuits by
// appending to result.ResourceErrors.
func (o *Orchestrator) seedTable(
	ctx context.Context,
	sourceProject, sourceDataset, sourceTable, destProject, destDataset, destTable string,
	maxRows int64,
	result *SeedResult,
) {
	if destTable == "" {
		destTable = sourceTable
	}
	match, ok := o.resolveSourceTable(ctx, sourceProject, sourceDataset, sourceTable, result)
	if !ok {
		return
	}
	_ = match // already validated to be a TABLE by resolveSourceTable

	schema, ok := o.describeSourceSchema(ctx, sourceProject, sourceDataset, sourceTable, result)
	if !ok {
		return
	}
	ref := TableRef{ProjectID: destProject, DatasetID: destDataset, TableID: destTable}
	if !o.writeTableMetadata(ctx, ref, schema, result) {
		return
	}
	o.copyTableRows(ctx, sourceProject, sourceDataset, sourceTable, ref, schema, maxRows, result)
}

// resolveSourceTable looks up the source table's metadata and
// returns it when it's a supported (physical TABLE) entry.
// Unsupported types fold into a ResourceError and ok=false.
func (o *Orchestrator) resolveSourceTable(
	ctx context.Context,
	sourceProject, sourceDataset, sourceTable string,
	result *SeedResult,
) (*ProductionTable, bool) {
	tables, err := o.Reader.ListTables(ctx, sourceProject, sourceDataset)
	if err != nil {
		result.ResourceErrors = append(result.ResourceErrors, SeedResourceError{
			Resource: fmt.Sprintf("table:%s.%s.%s", sourceProject, sourceDataset, sourceTable),
			Kind:     resourceKindRPC,
			Error:    fmt.Sprintf("ListTables: %v", err),
		})
		return nil, false
	}
	var match *ProductionTable
	for i := range tables {
		if tables[i].TableID == sourceTable {
			match = &tables[i]
			break
		}
	}
	if match == nil {
		result.ResourceErrors = append(result.ResourceErrors, SeedResourceError{
			Resource: fmt.Sprintf("table:%s.%s.%s", sourceProject, sourceDataset, sourceTable),
			Kind:     resourceKindRead,
			Error:    "table not found",
		})
		return nil, false
	}
	if !strings.EqualFold(match.Type, "TABLE") && match.Type != "" {
		result.ResourceErrors = append(result.ResourceErrors, SeedResourceError{
			Resource: fmt.Sprintf("table:%s.%s.%s", sourceProject, sourceDataset, sourceTable),
			Kind:     resourceKindUnsupported,
			Error: fmt.Sprintf(
				"type %q is not yet supported by the BigQuery emulator; only physical TABLE entries are seeded",
				match.Type,
			),
		})
		return nil, false
	}
	return match, true
}

// describeSourceSchema fetches the source table's schema.
func (o *Orchestrator) describeSourceSchema(
	ctx context.Context,
	sourceProject, sourceDataset, sourceTable string,
	result *SeedResult,
) (*enginepb.TableSchema, bool) {
	schema, err := o.Reader.DescribeTable(ctx, sourceProject, sourceDataset, sourceTable)
	if err != nil {
		result.ResourceErrors = append(result.ResourceErrors, SeedResourceError{
			Resource: fmt.Sprintf("table:%s.%s.%s", sourceProject, sourceDataset, sourceTable),
			Kind:     resourceKindRead,
			Error:    fmt.Sprintf("DescribeTable: %v", err),
		})
		return nil, false
	}
	return schema, true
}

// writeTableMetadata creates the destination table and bumps the
// Created/Skipped counters. Returns false when the EnsureTable call
// errored (and a ResourceError was already appended).
func (o *Orchestrator) writeTableMetadata(
	ctx context.Context,
	ref TableRef,
	schema *enginepb.TableSchema,
	result *SeedResult,
) bool {
	created, err := o.Applier.EnsureTable(ctx, ref, schema)
	if err != nil {
		result.ResourceErrors = append(result.ResourceErrors, SeedResourceError{
			Resource: fmt.Sprintf("table:%s.%s.%s", ref.ProjectID, ref.DatasetID, ref.TableID),
			Kind:     resourceKindWrite,
			Error:    err.Error(),
		})
		return false
	}
	if created {
		result.TablesCreated++
	} else {
		result.TablesSkipped++
	}
	return true
}

// copyTableRows reads up to maxRows rows from the source table and
// inserts them into the destination. Empty-row reads short-circuit
// the InsertRows RPC.
func (o *Orchestrator) copyTableRows(
	ctx context.Context,
	sourceProject, sourceDataset, sourceTable string,
	ref TableRef,
	schema *enginepb.TableSchema,
	maxRows int64,
	result *SeedResult,
) {
	rows, err := o.Reader.ReadRows(ctx, sourceProject, sourceDataset, sourceTable, maxRows)
	if err != nil {
		result.ResourceErrors = append(result.ResourceErrors, SeedResourceError{
			Resource: fmt.Sprintf("table:%s.%s.%s", sourceProject, sourceDataset, sourceTable),
			Kind:     resourceKindRead,
			Error:    fmt.Sprintf("ReadRows: %v", err),
		})
		return
	}
	if len(rows) == 0 {
		return
	}
	n, err := o.Applier.InsertRows(ctx, ref, schema, rows)
	if err != nil {
		result.ResourceErrors = append(result.ResourceErrors, SeedResourceError{
			Resource: fmt.Sprintf("table:%s.%s.%s", ref.ProjectID, ref.DatasetID, ref.TableID),
			Kind:     resourceKindWrite,
			Error:    err.Error(),
		})
		return
	}
	result.RowsCopied += int64(n)
}

// Resource error classification strings. Pulled into named
// constants so the orchestrator and tests both reference one
// source of truth, and so a future renamed kind value updates one
// place.
const (
	resourceKindRPC         = "rpc"
	resourceKindRead        = "read"
	resourceKindWrite       = "write"
	resourceKindUnsupported = "unsupported"
)

// destinationOf folds the optional Destination override into a
// concrete (project, dataset, table) triple the seed helpers act
// on. Empty fields map to the source values.
func destinationOf(req SeedRequest) struct{ Project, Dataset, Table string } {
	out := struct{ Project, Dataset, Table string }{
		Project: req.Source.Project,
		Dataset: req.Source.Dataset,
		Table:   req.Source.Table,
	}
	if req.Destination == nil {
		return out
	}
	if v := strings.TrimSpace(req.Destination.Project); v != "" {
		out.Project = v
	}
	if v := strings.TrimSpace(req.Destination.Dataset); v != "" {
		out.Dataset = v
	}
	if v := strings.TrimSpace(req.Destination.Table); v != "" {
		out.Table = v
	}
	return out
}

// NewProductionReader is the constructor the gateway calls when the
// operator enables the seed API. The default build returns
// ErrProductionUnsupported; building with `-tags=seed_production_live`
// swaps in the cloud.google.com/go/bigquery-backed implementation
// (gateway/seed/production_live.go).
//
// Callers that just want the YAML loader (gateway/seedfile) don't
// need to call this and don't pay for the heavy cloud deps.
//
// The unused parameter list is intentional: the live impl needs
// the project to set ADC quota and the env lookup to derive
// fallbacks for the production client signature).
func NewProductionReader(
	ctx context.Context,
	billingProject string,
	getenv func(string) (string, bool),
) (ProductionReader, error) {
	return nil, errors.New(ErrProductionUnsupported.Error())
}
