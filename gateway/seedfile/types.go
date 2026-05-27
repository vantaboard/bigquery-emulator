// Package seedfile loads a declarative YAML file at gateway startup
// and applies its datasets / tables / rows to the engine via the
// shared seed.Applier surface (gateway/seed).
//
// The YAML schema is intentionally close to the BigQuery REST API's
// dataset / table / row shape so operators who know one can read the
// other:
//
//	project_id: dev            # default project (optional; can also
//	                           # set per-dataset)
//	location: US               # default location (optional)
//	datasets:
//	  - id: ds
//	    project_id: dev        # optional override
//	    location: US           # optional override
//	    tables:
//	      - id: people
//	        schema:
//	          - {name: id, type: INT64, mode: REQUIRED}
//	          - {name: name, type: STRING}
//	        rows:
//	          - {id: 1, name: ada}
//	          - {id: 2, name: bob}
//
// The schema is the runtime seed schema; it is deliberately
// independent from the conformance/runner.Fixture format (which
// carries test-only expectations) so production seeding doesn't pick
// up assertions that have no meaning at runtime.
package seedfile

import (
	"errors"
	"fmt"
	"os"
	"strings"

	"gopkg.in/yaml.v3"
)

// File is the top-level YAML schema. Defaults at this level apply
// when a per-dataset field is empty.
type File struct {
	// DefaultProjectID is the project a dataset belongs to when
	// the dataset itself omits project_id. When both are empty
	// the loader falls back to seed.Defaults.ProjectID (the
	// gateway-level --project-id), and finally errors if even
	// that is missing.
	DefaultProjectID string `yaml:"project_id"`

	// DefaultLocation is the BigQuery location stamped on a
	// dataset when neither the dataset entry nor the gateway
	// supply one. Empty stays empty -- the engine will accept
	// the dataset without a location and clients can read it
	// back as such.
	DefaultLocation string `yaml:"location"`

	// Datasets enumerates the resources the loader will materialize.
	Datasets []Dataset `yaml:"datasets"`
}

// Dataset describes one logical BigQuery dataset and the tables
// inside it.
type Dataset struct {
	// ID is the dataset's BigQuery id. Required.
	ID string `yaml:"id"`

	// ProjectID overrides the file-level default for this
	// dataset. Optional.
	ProjectID string `yaml:"project_id"`

	// Location overrides the file-level default. Optional.
	Location string `yaml:"location"`

	// Tables is the per-dataset table list. May be empty so
	// operators can pre-create empty datasets (matches BigQuery's
	// "dataset without any tables" state).
	Tables []Table `yaml:"tables"`
}

// Table mirrors the REST API's Table resource. Schemas are
// positional (the column order defines the row layout) and rows
// are key/value maps keyed by column name.
type Table struct {
	// ID is the table's BigQuery id. Required.
	ID string `yaml:"id"`

	// Schema enumerates the table's columns. Required for tables
	// that include rows so the loader can lay cells out
	// positionally. Empty schema is allowed for "register the
	// table, no rows" workflows.
	Schema []FieldSchema `yaml:"schema"`

	// Rows is the per-table row list. Each row is a map keyed by
	// column name; missing columns become NULL cells. Extra keys
	// not in the schema are silently dropped, matching the
	// `tabledata.insertAll` handler's behavior.
	Rows []map[string]any `yaml:"rows"`
}

// FieldSchema mirrors enginepb.FieldSchema. We don't reuse the
// proto struct directly because the YAML decoder is happier with
// plain Go tags than with the generated protobuf struct.
type FieldSchema struct {
	// Name is the column name. Required.
	Name string `yaml:"name"`

	// Type is the BigQuery type name (STRING, INT64, BOOL, ...).
	// Required.
	Type string `yaml:"type"`

	// Mode is one of NULLABLE | REQUIRED | REPEATED. Empty
	// defaults to NULLABLE on the engine side.
	Mode string `yaml:"mode"`

	// Description is a free-form column description. Optional.
	Description string `yaml:"description"`

	// Fields holds nested STRUCT/RECORD fields. Walked
	// recursively when present.
	Fields []FieldSchema `yaml:"fields"`
}

// Load reads a YAML file from disk and decodes it into File. We
// reject unknown top-level keys so a typo (e.g. `projects:` instead
// of `datasets:`) surfaces as an error rather than silently
// producing an empty seed.
//
// `path` is operator-supplied via --seed-data-file; the gosec G304
// warning is expected (the whole point of the helper is to read
// from a caller-named path) and suppressed inline.
func Load(path string) (*File, error) {
	b, err := os.ReadFile(path) //nolint:gosec // path is the operator-supplied --seed-data-file
	if err != nil {
		return nil, fmt.Errorf("seedfile: read %s: %w", path, err)
	}
	return Decode(b, path)
}

// Decode parses YAML bytes into a File. The `source` argument is
// only used in error messages; pass the originating path when
// available, "" for in-memory inputs.
func Decode(data []byte, source string) (*File, error) {
	var f File
	dec := yaml.NewDecoder(strings.NewReader(string(data)))
	dec.KnownFields(true)
	if err := dec.Decode(&f); err != nil && !errors.Is(err, ErrEmptyFile) {
		// io.EOF from gopkg.in/yaml.v3 means the file is
		// effectively empty -- treat that as a valid no-op
		// rather than a parse error.
		if err.Error() == "EOF" {
			return &File{}, nil
		}
		return nil, fmt.Errorf("seedfile: parse %s: %w", labelSource(source), err)
	}
	if err := f.Validate(); err != nil {
		return nil, fmt.Errorf("seedfile: validate %s: %w", labelSource(source), err)
	}
	return &f, nil
}

// ErrEmptyFile is returned by Decode when the input is empty.
// Wrapped so callers can detect it with errors.Is.
var ErrEmptyFile = errors.New("seedfile: empty input")

// labelSource returns a non-empty descriptor for error messages.
func labelSource(s string) string {
	if s == "" {
		return "<input>"
	}
	return s
}

// Validate runs cheap structural checks before the loader starts
// talking to the engine. The error wording aims to point at the
// exact field so operator fixes are quick.
func (f *File) Validate() error {
	for i, ds := range f.Datasets {
		if strings.TrimSpace(ds.ID) == "" {
			return fmt.Errorf("datasets[%d].id is required", i)
		}
		for j, tbl := range ds.Tables {
			if strings.TrimSpace(tbl.ID) == "" {
				return fmt.Errorf("datasets[%d].tables[%d].id is required", i, j)
			}
			if len(tbl.Rows) > 0 && len(tbl.Schema) == 0 {
				return fmt.Errorf("datasets[%d].tables[%d].schema is required when rows are present",
					i, j)
			}
			for k, field := range tbl.Schema {
				if strings.TrimSpace(field.Name) == "" {
					return fmt.Errorf("datasets[%d].tables[%d].schema[%d].name is required",
						i, j, k)
				}
				if strings.TrimSpace(field.Type) == "" {
					return fmt.Errorf("datasets[%d].tables[%d].schema[%d].type is required",
						i, j, k)
				}
			}
		}
	}
	return nil
}
