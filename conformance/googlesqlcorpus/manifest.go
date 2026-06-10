package googlesqlcorpus

import (
	"encoding/json"
	"fmt"
	"os"
	"path/filepath"
	"slices"
)

// Manifest pins the passing subset and triage buckets for the lane.
type Manifest struct {
	// Pinned lists case IDs in "file::name" form that must PASS.
	Pinned []string `json:"pinned"`

	// Triage records first-run bucket assignments keyed by case ID.
	Triage map[string]TriageEntry `json:"triage,omitempty"`

	// UnsupportedFeatures skips any case declaring one of these
	// LanguageFeature tokens (without the FEATURE_ prefix).
	UnsupportedFeatures []string `json:"unsupported_features,omitempty"`
}

// TriageEntry is one case's triage classification.
type TriageEntry struct {
	Bucket  string `json:"bucket"`
	Message string `json:"message,omitempty"`
}

// CaseID returns the stable identifier for a test case.
func CaseID(tc TestCase) string {
	name := tc.Name
	if name == "" {
		name = fmt.Sprintf("line_%d", tc.Line)
	}
	return fmt.Sprintf("%s::%s", filepath.Base(tc.File), name)
}

// LoadManifest reads pinned-passing metadata from disk.
func LoadManifest(path string) (*Manifest, error) {
	b, err := os.ReadFile(path) //nolint:gosec // manifest path is CLI-controlled
	if err != nil {
		return nil, err
	}
	var m Manifest
	if err := json.Unmarshal(b, &m); err != nil {
		return nil, fmt.Errorf("decode manifest: %w", err)
	}
	return &m, nil
}

// SaveManifest writes manifest JSON atomically.
func SaveManifest(path string, m *Manifest) error {
	b, err := json.MarshalIndent(m, "", "  ")
	if err != nil {
		return err
	}
	tmp := path + ".tmp"
	if err := os.WriteFile(tmp, append(b, '\n'), 0o600); err != nil { //nolint:gosec // conformance artifact
		return err
	}
	return os.Rename(tmp, path)
}

// ShouldRun returns false when a case is outside the pinned gate or
// declares an unsupported feature.
func (m *Manifest) ShouldRun(tc TestCase, gatePinned bool) (bool, string) {
	if gatePinned && !m.isPinned(tc) {
		return false, "not in pinned manifest"
	}
	for _, feat := range tc.RequiredFeatures {
		if slices.Contains(m.UnsupportedFeatures, feat) {
			return false, "required feature out of scope: " + feat
		}
	}
	return true, ""
}

func (m *Manifest) isPinned(tc TestCase) bool {
	id := CaseID(tc)
	return slices.Contains(m.Pinned, id)
}
