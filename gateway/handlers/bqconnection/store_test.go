package bqconnection

import (
	"os"
	"path/filepath"
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/external/sourceconfig"
)

func TestOpenStoreRecoversFromCorruptJSON(t *testing.T) {
	dataDir := t.TempDir()
	regDir := filepath.Join(dataDir, "external", "connections", registryDir)
	if err := os.MkdirAll(regDir, 0o750); err != nil {
		t.Fatalf("MkdirAll: %v", err)
	}
	registry := filepath.Join(regDir, "connections.json")
	if err := os.WriteFile(registry, []byte("{not-json"), 0o600); err != nil {
		t.Fatalf("WriteFile: %v", err)
	}

	cfg, err := sourceconfig.Load(dataDir)
	if err != nil {
		t.Fatalf("sourceconfig.Load: %v", err)
	}
	st, openErr := OpenStore(cfg)
	if openErr != nil {
		t.Fatalf("OpenStore: %v", openErr)
	}
	if len(st.List("projects/p/locations/US")) != 0 {
		t.Fatalf("connections = %d, want 0 after corrupt recovery", len(st.List("projects/p/locations/US")))
	}
	if _, readErr := os.ReadFile(registry + ".corrupt"); readErr != nil {
		t.Fatalf("corrupt backup: %v", readErr)
	}
	if _, readErr := os.ReadFile(registry); !os.IsNotExist(readErr) {
		t.Fatalf("registry after recovery = %v, want not exist", readErr)
	}
}
