// Package runner is the engine half of the conformance harness:
// fixture loading, profile resolution, REST execution, and row /
// error diffing. The CLI entry point lives in
// `conformance/cmd/runner`; tests that exercise the runner against a
// real `emulator_main` subprocess live alongside the CLI behind the
// `//go:build integration` tag.
//
// The package is structured so the parsing and diff logic can be unit
// tested without a running engine: see `runner_test.go`. The harness
// half (`harness.go`) is the only code that touches subprocesses.
package runner

import (
	"errors"
	"fmt"
	"os"
	"path/filepath"
	"sort"
	"strings"

	"gopkg.in/yaml.v3"
)

// Fixture is the in-memory shape of a single YAML conformance file.
//
// See `conformance/README.md` for the worked schema. Every field on
// the wire is optional except `name` and `query`; the loader supplies
// safe defaults for the others so a fixture writer only has to spell
// out the fields they need.
type Fixture struct {
	// Name identifies the fixture in logs and diff output. By
	// convention it matches the YAML filename (without extension)
	// so a divergence between the two is easy to spot.
	Name string `yaml:"name"`

	// Description is free-form prose that gets echoed under the
	// fixture title in `--output text`. Optional.
	Description string `yaml:"description,omitempty"`

	// Profiles is the runtime matrix the fixture applies to. Empty
	// means the default (DuckDB-only) profile set. Unknown profile
	// names are an error at load time so a typo is caught
	// immediately rather than masked as "fixture ran on zero
	// profiles".
	Profiles []string `yaml:"profiles,omitempty"`

	// ProjectID is the BigQuery project the runner POSTs catalog +
	// query work against. Defaults to `proj-conformance-<name>` so
	// fixtures stay isolated even when share an emulator (via
	// `--connect`).
	ProjectID string `yaml:"project_id,omitempty"`

	// DatasetID is a documentation hint; the runner does not
	// auto-create it. Use a `setup` step with `dataset: <id>` to
	// actually create the dataset.
	DatasetID string `yaml:"dataset_id,omitempty"`

	// Setup runs in order before `Query`. Each step is dispatched
	// on which discriminator field is set (`dataset`, `table`,
	// `sql`); see SetupStep.
	Setup []SetupStep `yaml:"setup,omitempty"`

	// Query is the SQL the runner POSTs to /queries and asserts on.
	// Required. For DML-only fixtures, prefer encoding the assertion
	// as a SELECT after the mutation so the diff stays declarative.
	Query string `yaml:"query"`

	// Expected pins either the expected row set or the expected
	// HTTP error envelope. Exactly one of the two must be set.
	Expected Expectation `yaml:"expected"`

	// Path is filled in by Load; not parsed from YAML.
	Path string `yaml:"-"`
}

// SetupStep is one entry in `Fixture.Setup`. The four discriminator
// fields are mutually exclusive: `Dataset` for a dataset create,
// `Table` for a table create, `Rows` for a `tabledata.insertAll`
// seed, and `SQL` for a query (typically DML or DDL). The loader
// rejects steps that set more than one or none.
type SetupStep struct {
	// Dataset is the dataset ID to create. The runner POSTs a
	// minimal `{datasetReference, location:"US"}` body against
	// `/bigquery/v2/projects/<projectId>/datasets`.
	Dataset string `yaml:"dataset,omitempty"`

	// Table is the table to create. The runner POSTs against
	// `/bigquery/v2/projects/<projectId>/datasets/<datasetId>/tables`.
	Table *TableSetup `yaml:"table,omitempty"`

	// Rows seeds a previously created table by POSTing
	// `tabledata.insertAll`. The streaming-insert path is the right
	// tool when the fixture wants to assert the streaming side of
	// the wire (separate from the DML envelope); INSERT VALUES /
	// UPDATE / DELETE now land via the local DML executor
	// (`backend/engine/semantic/dml/`, plan 9), so fixtures that
	// just want seed data may use either `rows:` or an `sql:` step.
	Rows *RowsSetup `yaml:"rows,omitempty"`

	// SQL is a query the runner POSTs to /queries. Errors from the
	// gateway abort the fixture (counted as runner-internal failure,
	// not a fixture mismatch). Use this for MERGE, CREATE TABLE,
	// DROP TABLE, and the INSERT VALUES / UPDATE / DELETE shapes
	// now landed on the local DML executor (see `Rows` for the
	// streaming-insert alternative).
	SQL string `yaml:"sql,omitempty"`
}

// RowsSetup describes a `tabledata.insertAll` setup step. Each entry
// in `Rows` is a column-name -> cell-value map, matching the same
// shape as `Expectation.Rows`.
type RowsSetup struct {
	Dataset string           `yaml:"dataset"`
	Table   string           `yaml:"table"`
	Rows    []map[string]any `yaml:"rows"`
}

// TableSetup describes a table to create via REST. The schema is the
// usual BigQuery TableFieldSchema shape.
type TableSetup struct {
	Dataset string         `yaml:"dataset"`
	ID      string         `yaml:"id"`
	Schema  []SchemaColumn `yaml:"schema"`
}

// SchemaColumn maps directly to `bqtypes.TableFieldSchema`. We keep
// this as a runner-local struct so the YAML field names (lower-snake)
// stay decoupled from the wire-shape Go struct.
type SchemaColumn struct {
	Name        string         `yaml:"name"`
	Type        string         `yaml:"type"`
	Mode        string         `yaml:"mode,omitempty"`
	Description string         `yaml:"description,omitempty"`
	Fields      []SchemaColumn `yaml:"fields,omitempty"`
}

// Expectation captures one of two assertion modes. Exactly one of
// `Rows` or `Error` must be set (with the exception of
// `Match==schema_only`, which may set neither and rely on the
// gateway-returned schema alone).
type Expectation struct {
	// Match controls how Rows are compared against the gateway's
	// response. One of `ordered` (default), `unordered`, or
	// `schema_only`. See `conformance/README.md` for the matching
	// semantics each mode implies.
	Match MatchMode `yaml:"match,omitempty"`

	// Schema is the optional list of expected output columns. The
	// diff engine uses it for two things:
	//
	//   1. `schema_only` mode: required for the schema-vs-schema
	//      assertion (the engine compares this list against the
	//      `QueryResponse.schema` returned by the gateway).
	//   2. `ordered` / `unordered` modes: advisory, used to
	//      double-check the column set the query actually returned
	//      before diffing rows. When omitted, the runner trusts
	//      the gateway-supplied schema.
	Schema []ExpectedColumn `yaml:"schema,omitempty"`

	// Rows is the expected row set for a successful query. Each
	// row is a column-name -> cell-value map. The diff engine
	// normalizes both sides per the column's SQL type from the
	// gateway's `QueryResponse.schema` (so INT64 `1` matches
	// `"1"`, FLOAT64 compares with a relative epsilon, NULL stays
	// distinct from the literal string "NULL", etc.). See
	// `conformance/README.md` for the full type table.
	//
	// Ignored when `Match==schema_only`.
	Rows []map[string]any `yaml:"rows,omitempty"`

	// Error pins the expected error envelope when the fixture
	// intends to verify a failure mode (e.g. invalid SQL).
	Error *ExpectedError `yaml:"error,omitempty"`
}

// MatchMode is the row-comparison strategy declared by a fixture.
// Default is MatchOrdered.
type MatchMode string

const (
	// MatchOrdered (the default) compares rows pairwise in
	// declaration order. Use `ORDER BY` in the fixture query so the
	// comparison stays deterministic.
	MatchOrdered MatchMode = "ordered"

	// MatchUnordered compares rows as a multiset; the diff engine
	// canonicalizes every row to a type-normalized string and
	// asserts the two multisets are equal. Useful when the query
	// does not declare an ORDER BY and the storage engine returns
	// rows in implementation-defined order (DuckDB, parallel
	// scans, etc.).
	MatchUnordered MatchMode = "unordered"

	// MatchSchemaOnly ignores `Rows` entirely and only validates
	// the column names + types returned by the query. Useful for
	// queries whose row values are non-deterministic (CURRENT_*,
	// generated IDs) and for "dryRun" style smoke checks.
	MatchSchemaOnly MatchMode = "schema_only"
)

// ExpectedColumn is one entry in `Expectation.Schema`. The Type field
// is compared case-insensitively against the gateway's wire-format
// type (`STRING`, `INT64`, `FLOAT64`, etc.) so a fixture pinning
// `INTEGER` will still match a response advertising `INT64`.
type ExpectedColumn struct {
	Name string `yaml:"name"`
	Type string `yaml:"type"`
}

// ExpectedError captures the assertion vocabulary for the error path.
// Both fields are optional; the runner asserts only on what is set.
type ExpectedError struct {
	// Code is the expected HTTP status code, e.g. 400 / 404 / 501.
	// Zero means "do not assert on the status code". A fixture
	// must set at least one of Code or MessageContains.
	Code int `yaml:"code,omitempty"`

	// MessageContains is a substring the runner expects to find
	// in the BigQuery error envelope's top-level `error.message`
	// field (with a fallback to `error.errors[0].message`).
	MessageContains string `yaml:"message_contains,omitempty"`
}

// defaultProfiles is the set Fixture.Profiles defaults to when the
// fixture omits it. Keep alphabetized so iteration order is stable
// across the matrix.
var defaultProfiles = []string{ProfileDuckDB}

// Load parses a single YAML file into a Fixture. It validates the
// shape (required fields, exclusivity of expectation, known profile
// names) so callers can rely on the returned Fixture being usable.
func Load(path string) (*Fixture, error) {
	// #nosec G304 -- path is fixture-discovery output controlled by
	// --fixtures flag in a CLI dev tool.
	data, err := os.ReadFile(path)
	if err != nil {
		return nil, fmt.Errorf("read %s: %w", path, err)
	}
	return loadBytes(data, path)
}

// loadBytes is the test seam for Load. Tests construct YAML in memory
// and pass it through here; production code goes via Load (which is a
// thin file-read wrapper).
func loadBytes(data []byte, path string) (*Fixture, error) {
	var f Fixture
	dec := yaml.NewDecoder(strings.NewReader(string(data)))
	dec.KnownFields(true)
	if err := dec.Decode(&f); err != nil {
		return nil, fmt.Errorf("parse %s: %w", path, err)
	}
	f.Path = path
	if err := f.normalize(); err != nil {
		return nil, fmt.Errorf("validate %s: %w", path, err)
	}
	return &f, nil
}

// LoadDir walks a directory (recursively) and returns every loadable
// `.yaml` / `.yml` fixture, sorted by path. If `pathOrDir` points at
// a regular file it loads just that file. Returns the slice and the
// first error encountered (mirroring `filepath.Walk` semantics) so a
// single bad fixture stops the run with a clear pointer rather than
// silently dropping it.
func LoadDir(pathOrDir string) ([]*Fixture, error) {
	info, err := os.Stat(pathOrDir)
	if err != nil {
		return nil, fmt.Errorf("stat %s: %w", pathOrDir, err)
	}
	if !info.IsDir() {
		f, err := Load(pathOrDir)
		if err != nil {
			return nil, err
		}
		return []*Fixture{f}, nil
	}
	var fixtures []*Fixture
	walkErr := filepath.Walk(pathOrDir, func(p string, fi os.FileInfo, walkErr error) error {
		if walkErr != nil {
			return walkErr
		}
		if fi.IsDir() {
			return nil
		}
		ext := strings.ToLower(filepath.Ext(p))
		if ext != ".yaml" && ext != ".yml" {
			return nil
		}
		f, err := Load(p)
		if err != nil {
			return err
		}
		fixtures = append(fixtures, f)
		return nil
	})
	if walkErr != nil {
		return nil, walkErr
	}
	sort.Slice(fixtures, func(i, j int) bool {
		return fixtures[i].Path < fixtures[j].Path
	})
	return fixtures, nil
}

// normalize applies defaults and validates required fields.
func (f *Fixture) normalize() error {
	if strings.TrimSpace(f.Name) == "" {
		return errors.New("name is required")
	}
	if strings.TrimSpace(f.Query) == "" {
		return errors.New("query is required")
	}
	if f.ProjectID == "" {
		f.ProjectID = "proj-conformance-" + sanitizeID(f.Name)
	}
	if len(f.Profiles) == 0 {
		f.Profiles = append([]string(nil), defaultProfiles...)
	}
	known := make(map[string]bool, len(KnownProfiles()))
	for _, p := range KnownProfiles() {
		known[p.Name] = true
	}
	for _, p := range f.Profiles {
		if !known[p] {
			return fmt.Errorf("unknown profile %q (known: %s)",
				p, strings.Join(profileNames(), ", "))
		}
	}
	if err := f.validateExpectation(); err != nil {
		return err
	}
	for i, step := range f.Setup {
		if err := step.validate(); err != nil {
			return fmt.Errorf("setup[%d]: %w", i, err)
		}
	}
	return nil
}

func (f *Fixture) validateExpectation() error {
	if f.Expected.Match == "" {
		f.Expected.Match = MatchOrdered
	}
	switch f.Expected.Match {
	case MatchOrdered, MatchUnordered, MatchSchemaOnly:
	default:
		return fmt.Errorf(
			"expected.match=%q is not one of ordered, unordered, schema_only",
			f.Expected.Match)
	}

	hasRows := f.Expected.Rows != nil
	hasSchema := len(f.Expected.Schema) > 0
	hasErr := f.Expected.Error != nil
	if hasErr && (hasRows || hasSchema) {
		return errors.New(
			"expected: error cannot be combined with rows or schema")
	}
	switch f.Expected.Match {
	case MatchSchemaOnly:
		// schema_only fixtures must either declare an explicit
		// schema: block OR a rows: block (whose first row's keys
		// are used as the expected column-name set). Otherwise
		// there is nothing to assert on.
		if !hasErr && !hasRows && !hasSchema {
			return errors.New(
				"expected: match=schema_only requires schema or rows (column names)")
		}
	default:
		// ordered / unordered must set rows: or error:.
		if !hasRows && !hasErr {
			return errors.New("expected: must set either rows or error")
		}
	}
	if hasErr {
		e := f.Expected.Error
		if e.Code == 0 && e.MessageContains == "" {
			return errors.New("expected.error: must set at least one of code or message_contains")
		}
	}
	return nil
}

func (s SetupStep) validate() error {
	count := 0
	if s.Dataset != "" {
		count++
	}
	if s.Table != nil {
		count++
		if s.Table.Dataset == "" {
			return errors.New("table.dataset is required")
		}
		if s.Table.ID == "" {
			return errors.New("table.id is required")
		}
		if len(s.Table.Schema) == 0 {
			return errors.New("table.schema must list at least one column")
		}
	}
	if s.Rows != nil {
		count++
		if s.Rows.Dataset == "" {
			return errors.New("rows.dataset is required")
		}
		if s.Rows.Table == "" {
			return errors.New("rows.table is required")
		}
		if len(s.Rows.Rows) == 0 {
			return errors.New("rows.rows must list at least one row")
		}
	}
	if strings.TrimSpace(s.SQL) != "" {
		count++
	}
	switch count {
	case 0:
		return errors.New("setup step must set exactly one of dataset, table, rows, sql")
	case 1:
		return nil
	default:
		return errors.New("setup step must set exactly one of dataset, table, rows, sql")
	}
}

// sanitizeID lowercases the fixture name and replaces non-[a-z0-9-]
// characters with `-`. Used to derive default project IDs that
// satisfy BigQuery's project-ID grammar (the emulator does not
// strictly enforce it today, but we keep the defaults compatible so
// fixtures port to a real backend cleanly).
func sanitizeID(s string) string {
	var b strings.Builder
	b.Grow(len(s))
	for _, r := range strings.ToLower(s) {
		switch {
		case r >= 'a' && r <= 'z', r >= '0' && r <= '9':
			b.WriteRune(r)
		case r == '-':
			b.WriteRune('-')
		default:
			b.WriteRune('-')
		}
	}
	return b.String()
}
