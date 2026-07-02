package runner

import (
	"encoding/json"
	"errors"
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"strings"
)

const packagePreflightScript = `
import importlib.util
import json
import sys

def _module_name(pkg):
    for sep in ("==", ">=", "<=", "!=", "~=", "<", ">"):
        if sep in pkg:
            return pkg.split(sep, 1)[0].strip()
    return pkg.strip()

packages = json.load(sys.stdin)
missing = []
for pkg in packages:
    mod = _module_name(pkg)
    if importlib.util.find_spec(mod) is None:
        missing.append(pkg)
json.dump({"missing": missing}, sys.stdout)
`

// optionalDependencySkipReason returns a non-empty message when any listed
// optional dependency is absent from the configured Python interpreter.
func optionalDependencySkipReason(deps []string) string {
	if len(deps) == 0 {
		return ""
	}
	python, err := resolveConformancePython()
	if err != nil {
		return "optional dependency check: " + err.Error()
	}
	missing, err := missingPythonPackages(python, deps)
	if err != nil {
		return "optional dependency check: " + err.Error()
	}
	if len(missing) == 0 {
		return ""
	}
	return "optional dependencies not available: " + strings.Join(missing, ", ")
}

func resolveConformancePython() (string, error) {
	if p := os.Getenv("BIGQUERY_EMULATOR_PYTHON"); p != "" {
		if _, err := os.Stat(p); err != nil {
			return "", fmt.Errorf("BIGQUERY_EMULATOR_PYTHON %q: %w", p, err)
		}
		return p, nil
	}
	if dataDir := os.Getenv("BIGQUERY_EMULATOR_DATA_DIR"); dataDir != "" {
		managed := filepath.Join(dataDir, "python-udf-env", "bin", "python3")
		if st, err := os.Stat(managed); err == nil && st.Mode()&0o111 != 0 {
			return managed, nil
		}
	}
	if path, err := exec.LookPath("python3"); err == nil {
		return path, nil
	}
	return "", errors.New("python3 not found on PATH")
}

func missingPythonPackages(python string, packages []string) ([]string, error) {
	payload, err := json.Marshal(packages)
	if err != nil {
		return nil, err
	}
	cmd := exec.Command(python, "-c", packagePreflightScript)
	cmd.Stdin = strings.NewReader(string(payload))
	out, err := cmd.Output()
	if err != nil {
		return nil, fmt.Errorf("python preflight: %w", err)
	}
	var parsed struct {
		Missing []string `json:"missing"`
	}
	if err := json.Unmarshal(out, &parsed); err != nil {
		return nil, fmt.Errorf("decode preflight response: %w", err)
	}
	return parsed.Missing, nil
}
