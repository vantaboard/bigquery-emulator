// Package differential replays a corpus of SQL cases against the local
// emulator and diffs results against committed production-BigQuery oracles.
package differential

import (
	"errors"
	"fmt"
	"os"
	"path/filepath"
	"sort"
	"strings"

	"github.com/vantaboard/bigquery-emulator/conformance/runner"
	"gopkg.in/yaml.v3"
)

// CorpusCase is one differential-lane YAML under conformance/differential/corpus/.
// It reuses the fixture setup/query schema but pins expectations in a separate
// oracle JSON file referenced by OracleRef.
type CorpusCase struct {
	Name           string   `yaml:"name"`
	Description    string   `yaml:"description,omitempty"`
	Profiles       []string `yaml:"profiles,omitempty"`
	ProjectID      string   `yaml:"project_id,omitempty"`
	DefaultDataset string   `yaml:"default_dataset,omitempty"`

	OracleRef       string               `yaml:"oracle_ref"`
	OracleSource    string               `yaml:"oracle_source,omitempty"`
	Match           runner.MatchMode     `yaml:"match,omitempty"`
	KnownFailing    bool                 `yaml:"known_failing,omitempty"`
	QueryParameters []QueryParameterYAML `yaml:"query_parameters,omitempty"`

	Setup []runner.SetupStep `yaml:"setup,omitempty"`
	Query string             `yaml:"query"`

	Path string `yaml:"-"`
}

// QueryParameterYAML is the corpus-side spelling of a named query parameter.
type QueryParameterYAML struct {
	Name  string `yaml:"name"`
	Type  string `yaml:"type"`
	Value string `yaml:"value"`
}

// DefaultCorpusDir is the committed corpus root.
const DefaultCorpusDir = "conformance/differential/corpus"

// DefaultOracleDir is the committed oracle JSON root.
const DefaultOracleDir = "conformance/differential/oracle"

// LoadCorpus parses a single corpus YAML file.
func LoadCorpus(path string) (*CorpusCase, error) {
	data, err := os.ReadFile(path) //nolint:gosec // path is CLI-controlled
	if err != nil {
		return nil, fmt.Errorf("read %s: %w", path, err)
	}
	return loadCorpusBytes(data, path)
}

func loadCorpusBytes(data []byte, path string) (*CorpusCase, error) {
	var c CorpusCase
	dec := yaml.NewDecoder(strings.NewReader(string(data)))
	dec.KnownFields(true)
	if err := dec.Decode(&c); err != nil {
		return nil, fmt.Errorf("parse %s: %w", path, err)
	}
	c.Path = path
	if err := c.normalize(); err != nil {
		return nil, fmt.Errorf("validate %s: %w", path, err)
	}
	return &c, nil
}

// LoadCorpusDir walks dir (or loads a single file) and returns every corpus
// case. Files and directories whose basename starts with "_" are skipped
// unless includeSelfTest is true (unit/self-test lane).
func LoadCorpusDir(pathOrDir string, includeSelfTest bool) ([]*CorpusCase, error) {
	info, err := os.Stat(pathOrDir)
	if err != nil {
		return nil, fmt.Errorf("stat %s: %w", pathOrDir, err)
	}
	if !info.IsDir() {
		c, err := LoadCorpus(pathOrDir)
		if err != nil {
			return nil, err
		}
		return []*CorpusCase{c}, nil
	}

	var cases []*CorpusCase
	walkErr := filepath.Walk(pathOrDir, func(p string, fi os.FileInfo, walkErr error) error {
		if walkErr != nil {
			return walkErr
		}
		if fi.IsDir() {
			base := filepath.Base(p)
			if base != filepath.Base(pathOrDir) && strings.HasPrefix(base, "_") && !includeSelfTest {
				return filepath.SkipDir
			}
			return nil
		}
		ext := strings.ToLower(filepath.Ext(p))
		if ext != ".yaml" && ext != ".yml" {
			return nil
		}
		base := filepath.Base(p)
		if strings.HasPrefix(base, "_") && !includeSelfTest {
			return nil
		}
		c, err := LoadCorpus(p)
		if err != nil {
			return err
		}
		cases = append(cases, c)
		return nil
	})
	if walkErr != nil {
		return nil, walkErr
	}
	sort.Slice(cases, func(i, j int) bool { return cases[i].Path < cases[j].Path })
	return cases, nil
}

func (c *CorpusCase) normalize() error {
	if strings.TrimSpace(c.Name) == "" {
		return errors.New("name is required")
	}
	if strings.TrimSpace(c.Query) == "" {
		return errors.New("query is required")
	}
	if strings.TrimSpace(c.OracleRef) == "" {
		return errors.New("oracle_ref is required")
	}
	if c.ProjectID == "" {
		c.ProjectID = "proj-diff-" + sanitizeID(c.Name)
	}
	if len(c.Profiles) == 0 {
		c.Profiles = []string{runner.ProfileDuckDB}
	}
	known := make(map[string]bool, len(runner.KnownProfiles()))
	for _, p := range runner.KnownProfiles() {
		known[p.Name] = true
	}
	for _, p := range c.Profiles {
		if !known[p] {
			return fmt.Errorf("unknown profile %q", p)
		}
	}
	for i, step := range c.Setup {
		if err := step.ValidateExported(); err != nil {
			return fmt.Errorf("setup[%d]: %w", i, err)
		}
	}
	return nil
}

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
