// Package connectionfixture seeds EXTERNAL_QUERY snapshot files under
// $data_dir/external/connections/<conn_id>/.
package connectionfixture

import (
	"encoding/json"
	"errors"
	"io"
	"os"
	"path/filepath"
)

// ManifestEntry maps a query string or alias to a result filename.
type ManifestEntry struct {
	Query  string `json:"query,omitempty" yaml:"query,omitempty"`
	Alias  string `json:"alias,omitempty" yaml:"alias,omitempty"`
	Result string `json:"result"          yaml:"result"`
}

// Manifest is the on-disk queries.yaml / queries.json shape.
type Manifest struct {
	Queries []ManifestEntry `json:"queries" yaml:"queries"`
}

// ResultFile is schema + rows for one EXTERNAL_QUERY snapshot.
type ResultFile struct {
	Schema []Column         `json:"schema" yaml:"schema"`
	Rows   []map[string]any `json:"rows"   yaml:"rows"`
}

// Column is one output field in a fixture result.
type Column struct {
	Name string `json:"name" yaml:"name"`
	Type string `json:"type" yaml:"type"`
}

// CopyTree copies committed fixture files from srcDir into
// dataDir/external/connections/connID/.
func CopyTree(dataDir, connID, srcDir string) error {
	if dataDir == "" || connID == "" || srcDir == "" {
		return errors.New("dataDir, connID, and srcDir are required")
	}
	dst := filepath.Join(dataDir, "external", "connections", connID)
	if err := os.MkdirAll(dst, 0o750); err != nil {
		return err
	}
	return filepath.WalkDir(srcDir, func(path string, d os.DirEntry, walkErr error) error {
		if walkErr != nil {
			return walkErr
		}
		if d.IsDir() {
			return nil
		}
		rel, relErr := filepath.Rel(srcDir, path)
		if relErr != nil {
			return relErr
		}
		in, openErr := os.Open(path) //nolint:gosec // test fixture path
		if openErr != nil {
			return openErr
		}
		defer func() { _ = in.Close() }()
		outPath := filepath.Join(dst, rel)
		if mkdirErr := os.MkdirAll(filepath.Dir(outPath), 0o750); mkdirErr != nil {
			return mkdirErr
		}
		out, outErr := os.OpenFile(
			outPath,
			os.O_CREATE|os.O_TRUNC|os.O_WRONLY,
			0o600,
		) //nolint:gosec // fixture path under dataDir
		if outErr != nil {
			return outErr
		}
		_, cpErr := io.Copy(out, in)
		closeErr := out.Close()
		if cpErr != nil {
			return cpErr
		}
		return closeErr
	})
}

// WriteInline materializes manifest + one result file under dataDir.
func WriteInline(dataDir, connID string, manifest Manifest, resultName string, result ResultFile) error {
	if dataDir == "" || connID == "" {
		return errors.New("dataDir and connID are required")
	}
	root := filepath.Join(dataDir, "external", "connections", connID)
	if err := os.MkdirAll(root, 0o750); err != nil {
		return err
	}
	manifestPath := filepath.Join(root, "queries.json")
	manifestRaw, marshalErr := json.MarshalIndent(manifest, "", "  ")
	if marshalErr != nil {
		return marshalErr
	}
	if writeErr := os.WriteFile(manifestPath, manifestRaw, 0o600); writeErr != nil {
		return writeErr
	}
	if resultName == "" {
		resultName = "result.json"
	}
	resultRaw, resultMarshalErr := json.MarshalIndent(result, "", "  ")
	if resultMarshalErr != nil {
		return resultMarshalErr
	}
	return os.WriteFile(filepath.Join(root, resultName), resultRaw, 0o600)
}
