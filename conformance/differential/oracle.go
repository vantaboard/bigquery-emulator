package differential

import (
	"encoding/json"
	"errors"
	"fmt"
	"os"
	"path/filepath"
	"strings"
	"time"

	"github.com/vantaboard/bigquery-emulator/conformance/runner"
	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
)

// OracleError is the recorded production BigQuery error envelope.
type OracleError struct {
	Code    int    `json:"code"`
	Message string `json:"message"`
}

// Oracle is a committed production-BigQuery capture for one corpus case.
// Rows/schema mirror bqtypes.QueryResponse so the fixture comparator is reused.
type Oracle struct {
	CapturedAt   string           `json:"captured_at"`
	Project      string           `json:"project"`
	JobID        string           `json:"job_id,omitempty"`
	OracleSource string           `json:"oracle_source,omitempty"`
	Match        runner.MatchMode `json:"match,omitempty"`
	Success      bool             `json:"success"`

	Schema       *bqtypes.TableSchema  `json:"schema,omitempty"`
	Rows         []bqtypes.Row         `json:"rows,omitempty"`
	JobReference *bqtypes.JobReference `json:"jobReference,omitempty"`
	Error        *OracleError          `json:"error,omitempty"`
}

// LoadOracle reads oracle JSON referenced by ref (basename or relative path).
func LoadOracle(oracleDir, ref string) (*Oracle, error) {
	path := ref
	if !strings.Contains(ref, string(os.PathSeparator)) {
		path = filepath.Join(oracleDir, ref)
	}
	data, err := os.ReadFile(path) //nolint:gosec // oracle dir is CLI-controlled
	if err != nil {
		return nil, fmt.Errorf("read oracle %s: %w", path, err)
	}
	var o Oracle
	if err := json.Unmarshal(data, &o); err != nil {
		return nil, fmt.Errorf("parse oracle %s: %w", path, err)
	}
	if err := o.validate(); err != nil {
		return nil, fmt.Errorf("validate oracle %s: %w", path, err)
	}
	return &o, nil
}

func (o *Oracle) validate() error {
	if o.CapturedAt == "" {
		return errors.New("captured_at is required")
	}
	if o.Project == "" {
		return errors.New("project is required")
	}
	if o.Success {
		if o.Schema == nil || len(o.Schema.Fields) == 0 {
			return errors.New("success oracle requires schema.fields")
		}
		return nil
	}
	if o.Error == nil {
		return errors.New("error oracle requires error block")
	}
	if o.Error.Code == 0 && o.Error.Message == "" {
		return errors.New("error oracle requires code or message")
	}
	return nil
}

// ExpectationFromOracle converts wire rows into the runner Expectation used by
// CompareRows / CompareError.
func ExpectationFromOracle(o *Oracle, caseMatch runner.MatchMode) runner.Expectation {
	match := o.Match
	if match == "" {
		match = caseMatch
	}
	if match == "" {
		match = runner.MatchOrdered
	}
	if !o.Success {
		exp := runner.ExpectedError{MessageContains: o.Error.Message}
		if o.Error.Code != 0 {
			exp.Code = o.Error.Code
		}
		return runner.Expectation{Match: match, Error: &exp}
	}
	cols := schemaColumns(o.Schema)
	rows := make([]map[string]any, 0, len(o.Rows))
	for _, r := range o.Rows {
		row := make(map[string]any, len(r.F))
		for i, cell := range r.F {
			name := positionalName(cols, i)
			row[name] = oracleCellValue(cell.V)
		}
		rows = append(rows, row)
	}
	return runner.Expectation{Match: match, Rows: rows}
}

// WriteOracle atomically writes an oracle JSON file.
func WriteOracle(path string, o *Oracle) error {
	if o.CapturedAt == "" {
		o.CapturedAt = time.Now().UTC().Format(time.RFC3339)
	}
	data, err := json.MarshalIndent(o, "", "  ")
	if err != nil {
		return err
	}
	data = append(data, '\n')
	dir := filepath.Dir(path)
	tmp, err := os.CreateTemp(dir, ".oracle-*.tmp")
	if err != nil {
		return err
	}
	tmpName := tmp.Name()
	if _, err := tmp.Write(data); err != nil {
		_ = tmp.Close()
		_ = os.Remove(tmpName)
		return err
	}
	if err := tmp.Close(); err != nil {
		_ = os.Remove(tmpName)
		return err
	}
	return os.Rename(tmpName, path)
}

func oracleCellValue(v any) any {
	if v == nil {
		return nil
	}
	if s, ok := v.(string); ok {
		return s
	}
	return v
}

func schemaColumns(schema *bqtypes.TableSchema) []string {
	if schema == nil {
		return nil
	}
	out := make([]string, len(schema.Fields))
	for i, f := range schema.Fields {
		out[i] = f.Name
	}
	return out
}

func positionalName(cols []string, i int) string {
	if i < len(cols) {
		return cols[i]
	}
	return fmt.Sprintf("col%d", i)
}

// OracleFromQueryResponse builds a success oracle from a gateway QueryResponse body.
func OracleFromQueryResponse(project string, source string, match runner.MatchMode, body []byte) (*Oracle, error) {
	var run bqtypes.QueryResponse
	if err := json.Unmarshal(body, &run); err != nil {
		return nil, fmt.Errorf("decode QueryResponse: %w", err)
	}
	o := &Oracle{
		Project:      project,
		OracleSource: source,
		Match:        match,
		Success:      true,
		Schema:       run.Schema,
		Rows:         run.Rows,
		JobReference: run.JobReference,
	}
	if run.JobReference != nil {
		o.JobID = run.JobReference.JobID
	}
	return o, nil
}
