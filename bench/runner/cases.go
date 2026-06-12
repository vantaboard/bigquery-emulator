package runner

import (
	"crypto/sha256"
	"encoding/hex"
	"fmt"
	"os"
	"path/filepath"
	"slices"
	"sort"
	"strings"
	"time"

	"gopkg.in/yaml.v3"
)

// Case is one YAML benchmark definition under bench/cases/.
type Case struct {
	Name        string       `yaml:"name"`
	Tags        []string     `yaml:"tags,omitempty"`
	SetupSQL    []string     `yaml:"-"`
	Query       string       `yaml:"query"`
	Iterations  int          `yaml:"iterations,omitempty"`
	Warmup      int          `yaml:"warmup,omitempty"`
	MaxRatio    float64      `yaml:"max_ratio,omitempty"`
	MaxMS       int64        `yaml:"max_ms,omitempty"`
	ProjectID   string       `yaml:"project_id,omitempty"`
	SkipTargets []TargetName `yaml:"skip_targets,omitempty"`
	SkipReason  string       `yaml:"skip_reason,omitempty"`
	Path        string       `yaml:"-"`
	ContentHash string       `yaml:"-"`
}

const (
	defaultIterations = 10
	defaultWarmup     = 2
	defaultMaxRatio   = 1.5
	defaultMaxMS      = 30_000
	defaultTimeoutMS  = 60_000
)

// LoadCases reads every *.yaml file in dir, sorted by name.
func LoadCases(dir string) ([]Case, error) {
	entries, err := os.ReadDir(dir)
	if err != nil {
		return nil, fmt.Errorf("read cases dir %s: %w", dir, err)
	}
	var paths []string
	for _, e := range entries {
		if e.IsDir() || !strings.HasSuffix(e.Name(), ".yaml") {
			continue
		}
		paths = append(paths, filepath.Join(dir, e.Name()))
	}
	sort.Strings(paths)
	out := make([]Case, 0, len(paths))
	for _, p := range paths {
		c, err := LoadCase(p)
		if err != nil {
			return nil, err
		}
		out = append(out, c)
	}
	return out, nil
}

// LoadCase parses a single benchmark case file.
func LoadCase(path string) (Case, error) {
	raw, err := os.ReadFile(path) //nolint:gosec // case path comes from bench/cases discovery
	if err != nil {
		return Case{}, fmt.Errorf("read %s: %w", path, err)
	}
	var c Case
	if err := yaml.Unmarshal(raw, &c); err != nil {
		return Case{}, fmt.Errorf("parse %s: %w", path, err)
	}
	if c.Name == "" {
		c.Name = strings.TrimSuffix(filepath.Base(path), filepath.Ext(path))
	}
	if c.Query == "" {
		return Case{}, fmt.Errorf("%s: query is required", path)
	}
	if c.Iterations <= 0 {
		c.Iterations = defaultIterations
	}
	if c.Warmup < 0 {
		c.Warmup = 0
	}
	if c.Warmup >= c.Iterations {
		c.Warmup = max(c.Iterations-1, 0)
	}
	if c.MaxRatio <= 0 {
		c.MaxRatio = defaultMaxRatio
	}
	if c.MaxMS <= 0 {
		c.MaxMS = defaultMaxMS
	}
	if c.ProjectID == "" {
		c.ProjectID = "bench-" + c.Name
	}
	c.Path = path
	c.ContentHash = hashContent(string(raw))
	return c, nil
}

// SkippedFor reports whether a target should not run this case.
func (c Case) SkippedFor(target TargetName) (bool, string) {
	if slices.Contains(c.SkipTargets, target) {
		reason := c.SkipReason
		if reason == "" {
			reason = "skipped for " + string(target)
		}
		return true, reason
	}
	return false, ""
}

// Substitute replaces {{ds}} and {{project}} placeholders.
func (c Case) Substitute(dataset, project string) (setup []string, query string) {
	repl := func(s string) string {
		s = strings.ReplaceAll(s, "{{ds}}", dataset)
		s = strings.ReplaceAll(s, "{{project}}", project)
		return s
	}
	setup = make([]string, len(c.SetupSQL))
	for i, s := range c.SetupSQL {
		setup[i] = repl(s)
	}
	return setup, repl(c.Query)
}

// UnmarshalYAML accepts setup as {sql: ...} objects.
func (c *Case) UnmarshalYAML(value *yaml.Node) error {
	type plain struct {
		Name        string       `yaml:"name"`
		Tags        []string     `yaml:"tags,omitempty"`
		Query       string       `yaml:"query"`
		Iterations  int          `yaml:"iterations,omitempty"`
		Warmup      int          `yaml:"warmup,omitempty"`
		MaxRatio    float64      `yaml:"max_ratio,omitempty"`
		MaxMS       int64        `yaml:"max_ms,omitempty"`
		ProjectID   string       `yaml:"project_id,omitempty"`
		SkipTargets []TargetName `yaml:"skip_targets,omitempty"`
		SkipReason  string       `yaml:"skip_reason,omitempty"`
		Setup       []struct {
			SQL string `yaml:"sql"`
		} `yaml:"setup"`
	}
	var aux plain
	if err := value.Decode(&aux); err != nil {
		return err
	}
	c.Name = aux.Name
	c.Tags = aux.Tags
	c.Query = aux.Query
	c.Iterations = aux.Iterations
	c.Warmup = aux.Warmup
	c.MaxRatio = aux.MaxRatio
	c.MaxMS = aux.MaxMS
	c.ProjectID = aux.ProjectID
	c.SkipTargets = aux.SkipTargets
	c.SkipReason = aux.SkipReason
	c.SetupSQL = make([]string, 0, len(aux.Setup))
	for _, step := range aux.Setup {
		if step.SQL != "" {
			c.SetupSQL = append(c.SetupSQL, step.SQL)
		}
	}
	return nil
}

func hashContent(s string) string {
	sum := sha256.Sum256([]byte(s))
	return hex.EncodeToString(sum[:8])
}

// QueryTimeout returns the wall-clock cap for query iterations. Cases
// that set max_ms above the default baseline cap use that value so
// slow targets (notably goccy on large joins) can finish.
func (c Case) QueryTimeout(fallback time.Duration) time.Duration {
	if c.MaxMS > defaultMaxMS {
		return time.Duration(c.MaxMS) * time.Millisecond
	}
	return fallback
}
