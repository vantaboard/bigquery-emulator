package runner

import (
	"errors"
	"fmt"
	"os"
	"path/filepath"
	"sort"
	"strconv"
	"strings"

	"gopkg.in/yaml.v3"
)

// Session is the in-memory shape of a multi-step session YAML file under
// conformance/sessions/. Every step runs against a single long-lived engine
// process unless a restart step stops and relaunches it (same --data_dir).
type Session struct {
	Name            string        `yaml:"name"`
	Description     string        `yaml:"description,omitempty"`
	Profiles        []string      `yaml:"profiles,omitempty"`
	ProjectID       string        `yaml:"project_id,omitempty"`
	DefaultDataset  string        `yaml:"default_dataset,omitempty"`
	KnownFailing    bool          `yaml:"known_failing,omitempty"`
	KnownFailingRef string        `yaml:"known_failing_ref,omitempty"`
	Steps           []SessionStep `yaml:"steps"`

	Path string `yaml:"-"`
}

// SessionStep is one ordered operation or assertion in a session. Setup-style
// fields mirror Fixture.Setup; assertion fields may stand alone or attach to
// a query step. A repeat group sets Repeat and nested Steps.
type SessionStep struct {
	Dataset        string      `yaml:"dataset,omitempty"`
	Table          *TableSetup `yaml:"table,omitempty"`
	Rows           *RowsSetup  `yaml:"rows,omitempty"`
	SQL            string      `yaml:"sql,omitempty"`
	Query          string      `yaml:"query,omitempty"`
	DefaultDataset string      `yaml:"default_dataset,omitempty"`

	REST *RESTStep `yaml:"rest,omitempty"`

	ExpectRows      []map[string]any `yaml:"expect_rows,omitempty"`
	ExpectError     *ExpectedError   `yaml:"expect_error,omitempty"`
	ExpectTableList *TableListExpect `yaml:"expect_table_list,omitempty"`
	ExpectAlive     *bool            `yaml:"expect_alive,omitempty"`

	Repeat  int           `yaml:"repeat,omitempty"`
	Steps   []SessionStep `yaml:"steps,omitempty"`
	Restart bool          `yaml:"restart,omitempty"`
}

// RESTStep is a generic gateway REST call (method + path + optional JSON body).
type RESTStep struct {
	Method       string         `yaml:"method"`
	Path         string         `yaml:"path"`
	Body         map[string]any `yaml:"body,omitempty"`
	ExpectStatus int            `yaml:"expect_status,omitempty"`
}

// TableListExpect asserts tables.list contains (or omits) table IDs.
type TableListExpect struct {
	Dataset     string   `yaml:"dataset"`
	Contains    []string `yaml:"contains,omitempty"`
	NotContains []string `yaml:"not_contains,omitempty"`
}

// DefaultSessionsDir is the committed session fixture root.
const DefaultSessionsDir = "conformance/sessions"

// LoadSession parses one session YAML file.
func LoadSession(path string) (*Session, error) {
	data, err := os.ReadFile(path) //nolint:gosec // path is CLI-controlled
	if err != nil {
		return nil, fmt.Errorf("read %s: %w", path, err)
	}
	return loadSessionBytes(data, path)
}

func loadSessionBytes(data []byte, path string) (*Session, error) {
	var s Session
	dec := yaml.NewDecoder(strings.NewReader(string(data)))
	dec.KnownFields(true)
	if err := dec.Decode(&s); err != nil {
		return nil, fmt.Errorf("parse %s: %w", path, err)
	}
	s.Path = path
	if err := s.normalize(); err != nil {
		return nil, fmt.Errorf("validate %s: %w", path, err)
	}
	return &s, nil
}

// LoadSessionDir walks a directory (or loads a single file) and returns every
// loadable session, sorted by path. Directories whose basename starts with "_"
// are skipped unless includeSelfTest is true.
func LoadSessionDir(pathOrDir string, includeSelfTest bool) ([]*Session, error) {
	info, err := os.Stat(pathOrDir)
	if err != nil {
		return nil, fmt.Errorf("stat %s: %w", pathOrDir, err)
	}
	if !info.IsDir() {
		s, err := LoadSession(pathOrDir)
		if err != nil {
			return nil, err
		}
		return []*Session{s}, nil
	}
	var sessions []*Session
	walkErr := filepath.Walk(pathOrDir, func(p string, fi os.FileInfo, walkErr error) error {
		if walkErr != nil {
			return walkErr
		}
		if fi.IsDir() {
			base := filepath.Base(p)
			if base != filepath.Base(pathOrDir) && strings.HasPrefix(base, "_") {
				if includeSelfTest {
					return nil
				}
				return filepath.SkipDir
			}
			return nil
		}
		if !includeSelfTest && strings.HasPrefix(filepath.Base(p), "_") {
			return nil
		}
		ext := strings.ToLower(filepath.Ext(p))
		if ext != ".yaml" && ext != ".yml" {
			return nil
		}
		s, err := LoadSession(p)
		if err != nil {
			return err
		}
		sessions = append(sessions, s)
		return nil
	})
	if walkErr != nil {
		return nil, walkErr
	}
	sort.Slice(sessions, func(i, j int) bool { return sessions[i].Path < sessions[j].Path })
	return sessions, nil
}

func (s *Session) normalize() error {
	if strings.TrimSpace(s.Name) == "" {
		return errors.New("name is required")
	}
	if len(s.Steps) == 0 {
		return errors.New("steps must list at least one entry")
	}
	if s.ProjectID == "" {
		s.ProjectID = "proj-session-" + sanitizeID(s.Name)
	}
	if len(s.Profiles) == 0 {
		s.Profiles = append([]string(nil), defaultProfiles...)
	}
	known := make(map[string]bool, len(KnownProfiles()))
	for _, p := range KnownProfiles() {
		known[p.Name] = true
	}
	for _, p := range s.Profiles {
		if !known[p] {
			return fmt.Errorf("unknown profile %q (known: %s)",
				p, strings.Join(profileNames(), ", "))
		}
	}
	for i, step := range s.Steps {
		if err := step.validate(strconv.Itoa(i)); err != nil {
			return err
		}
	}
	return nil
}

func (step *SessionStep) validate(indexPrefix string) error {
	if step.Repeat > 0 {
		if len(step.Steps) == 0 {
			return fmt.Errorf("steps[%s]: repeat requires nested steps", indexPrefix)
		}
		for i, nested := range step.Steps {
			if err := nested.validate(fmt.Sprintf("%s.repeat[%d]", indexPrefix, i)); err != nil {
				return err
			}
		}
		return nil
	}
	kind, err := step.kind()
	if err != nil {
		return fmt.Errorf("steps[%s]: %w", indexPrefix, err)
	}
	switch kind {
	case stepKindQuery:
		if len(step.ExpectRows) == 0 && step.ExpectError == nil {
			return fmt.Errorf("steps[%s]: query step requires expect_rows or expect_error", indexPrefix)
		}
	case stepKindAssertionOnly:
		if step.ExpectAlive == nil && step.ExpectTableList == nil && step.ExpectError == nil {
			return fmt.Errorf(
				"steps[%s]: assertion step must set expect_alive, expect_table_list, or expect_error",
				indexPrefix,
			)
		}
	case stepKindSetup:
		if err := step.asSetupStep().validate(); err != nil {
			return fmt.Errorf("steps[%s]: %w", indexPrefix, err)
		}
	case stepKindREST:
		if err := step.REST.validate(); err != nil {
			return fmt.Errorf("steps[%s]: %w", indexPrefix, err)
		}
	case stepKindRestart:
		if step.Restart {
			return nil
		}
	}
	if step.ExpectTableList != nil {
		if err := step.ExpectTableList.validate(); err != nil {
			return fmt.Errorf("steps[%s]: %w", indexPrefix, err)
		}
	}
	return nil
}

func (r *RESTStep) validate() error {
	if strings.TrimSpace(r.Method) == "" {
		return errors.New("rest.method is required")
	}
	if strings.TrimSpace(r.Path) == "" {
		return errors.New("rest.path is required")
	}
	return nil
}

func (e *TableListExpect) validate() error {
	if e.Dataset == "" {
		return errors.New("expect_table_list.dataset is required")
	}
	if len(e.Contains) == 0 && len(e.NotContains) == 0 {
		return errors.New("expect_table_list must set contains and/or not_contains")
	}
	return nil
}

type sessionStepKind int

const (
	stepKindSetup sessionStepKind = iota
	stepKindQuery
	stepKindREST
	stepKindRestart
	stepKindAssertionOnly
)

func (step *SessionStep) kind() (sessionStepKind, error) {
	if step.Repeat > 0 {
		return 0, errors.New("repeat groups are handled separately")
	}
	count := 0
	var kind sessionStepKind
	if step.Dataset != "" || step.Table != nil || step.Rows != nil || strings.TrimSpace(step.SQL) != "" {
		count++
		kind = stepKindSetup
	}
	if strings.TrimSpace(step.Query) != "" {
		count++
		kind = stepKindQuery
	}
	if step.REST != nil {
		count++
		kind = stepKindREST
	}
	if step.Restart {
		count++
		kind = stepKindRestart
	}
	if count == 0 {
		if step.ExpectAlive != nil || step.ExpectTableList != nil || step.ExpectError != nil {
			return stepKindAssertionOnly, nil
		}
		return 0, errors.New("empty step")
	}
	if count > 1 {
		return 0, errors.New("step must set exactly one operation")
	}
	return kind, nil
}

func (step *SessionStep) asSetupStep() SetupStep {
	return SetupStep{
		Dataset:          step.Dataset,
		Table:            step.Table,
		Rows:             step.Rows,
		SQL:              step.SQL,
		RowAccessPolicy:  nil,
		ColumnGovernance: nil,
	}
}
