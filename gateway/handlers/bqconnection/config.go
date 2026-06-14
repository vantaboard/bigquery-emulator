package bqconnection

import (
	"strings"

	"github.com/vantaboard/bigquery-emulator/gateway/external/sourceconfig"
)

// FixtureAnnotation is stored on Connection.description when created in
// fixture mode so EXTERNAL_QUERY resolution can locate snapshot data.
const FixtureAnnotation = "bqemu:fixture"

// ModeForConnection resolves the config mode for a connection resource name.
func ModeForConnection(cfg *sourceconfig.Config, name string) sourceconfig.Mode {
	if cfg == nil {
		return sourceconfig.ModeFixture
	}
	return cfg.ResolveConnection(name)
}

// AnnotateFixtureDescription prefixes description when mode is fixture.
func AnnotateFixtureDescription(cfg *sourceconfig.Config, name, description string) string {
	if ModeForConnection(cfg, name) != sourceconfig.ModeFixture {
		return description
	}
	if strings.Contains(description, FixtureAnnotation) {
		return description
	}
	if description == "" {
		return FixtureAnnotation
	}
	return FixtureAnnotation + " " + description
}
