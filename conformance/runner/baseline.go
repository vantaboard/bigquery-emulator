package runner

import (
	"encoding/json"
	"fmt"
	"os"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
	"gopkg.in/yaml.v3"
)

// rewriteFixtureRows captures the gateway's QueryResponse rows back
// into the fixture's `expected.rows` block and writes the YAML to
// disk. Used by --update-baselines to bootstrap a new fixture.
//
// We intentionally re-marshal the entire fixture rather than try to
// surgically replace the `expected:` node. Comments above the
// fixture's `name:` survive (yaml.v3 keeps them on the root node when
// we re-encode), but inline comments inside the `expected:` block
// are dropped -- the trade-off is documented in
// `conformance/README.md`.
func rewriteFixtureRows(fx *Fixture, body []byte) error {
	var run bqtypes.QueryResponse
	if err := json.Unmarshal(body, &run); err != nil {
		return fmt.Errorf("decode QueryResponse for baseline: %w", err)
	}
	cols := schemaColumns(run.Schema)
	rows := make([]map[string]any, 0, len(run.Rows))
	for _, r := range run.Rows {
		row := make(map[string]any, len(r.F))
		for i, cell := range r.F {
			name := positionalName(cols, i)
			row[name] = baselineCellValue(cell.V)
		}
		rows = append(rows, row)
	}
	// Preserve the fixture's existing Match mode; baseline
	// rewriting is a values-only operation, not a mode flip.
	fx.Expected = Expectation{Match: fx.Expected.Match, Rows: rows}
	return writeFixture(fx)
}

// rewriteFixtureError captures the gateway's error envelope back
// into the fixture's `expected.error` block. The runner pins the
// observed HTTP code; the message is captured as the BigQuery
// envelope's top-level `error.message` (or the first per-error
// `errors[].message` if the top-level field is empty).
func rewriteFixtureError(fx *Fixture, status int, body []byte) error {
	var env struct {
		Error struct {
			Message string `json:"message"`
			Errors  []struct {
				Message string `json:"message"`
			} `json:"errors"`
		} `json:"error"`
	}
	_ = json.Unmarshal(body, &env)
	msg := env.Error.Message
	if msg == "" && len(env.Error.Errors) > 0 {
		msg = env.Error.Errors[0].Message
	}
	fx.Expected = Expectation{Error: &ExpectedError{
		Code:            status,
		MessageContains: msg,
	}}
	return writeFixture(fx)
}

// baselineCellValue maps a wire-format cell value (string scalar,
// nested object, or REPEATED array) onto the YAML form that should
// be written back into the fixture's `expected.rows` block. Scalars
// keep their string form (BigQuery encodes everything as strings on
// the wire), NULLs land as YAML `null`, and nested structures are
// passed through `any` so the YAML encoder renders them as inline
// maps/sequences.
func baselineCellValue(v any) any {
	if v == nil {
		return nil
	}
	if s, ok := v.(string); ok {
		return s
	}
	return v
}

// writeFixture serializes the fixture and atomically replaces the
// file on disk via the standard "write-temp-then-rename" pattern.
func writeFixture(fx *Fixture) error {
	data, err := yaml.Marshal(fx)
	if err != nil {
		return fmt.Errorf("marshal fixture: %w", err)
	}
	tmp := fx.Path + ".tmp"
	if err := os.WriteFile(tmp, data, 0o600); err != nil {
		return fmt.Errorf("write tmp %s: %w", tmp, err)
	}
	if err := os.Rename(tmp, fx.Path); err != nil {
		_ = os.Remove(tmp)
		return fmt.Errorf("rename %s -> %s: %w", tmp, fx.Path, err)
	}
	return nil
}
