package main

import (
	"bytes"
	"encoding/json"
	"os"
	"path/filepath"
	"testing"
)

const sampleReleaseTag = "v0.1.0"

func TestBadgeMessage(t *testing.T) {
	cases := map[string]string{
		"":               missingMessage,
		"  ":             missingMessage,
		sampleReleaseTag: sampleReleaseTag,
	}
	for in, want := range cases {
		if got := badgeMessage(in); got != want {
			t.Errorf("badgeMessage(%q) = %q, want %q", in, got, want)
		}
	}
}

func TestBadgeColor(t *testing.T) {
	cases := []struct {
		version string
		want    string
	}{
		{"", colorLightgrey},
		{sampleReleaseTag, colorBlue},
		{"v1.0.0-rc1", colorOrange},
		{"v0.0.1-beta.1", colorOrange},
	}
	for _, c := range cases {
		if got := badgeColor(c.version); got != c.want {
			t.Errorf("badgeColor(%q) = %q, want %q", c.version, got, c.want)
		}
	}
}

func TestRunBadge_EndToEnd(t *testing.T) {
	dir := t.TempDir()
	badgePath := filepath.Join(dir, "badge-release.json")

	var out, errBuf bytes.Buffer
	if err := run([]string{
		cmdBadge,
		"--version=" + sampleReleaseTag,
		"--out=" + badgePath,
	}, &out, &errBuf); err != nil {
		t.Fatalf("badge: %v\nstderr=%s", err, errBuf.String())
	}

	var b badgeJSON
	raw, err := os.ReadFile(badgePath)
	if err != nil {
		t.Fatalf("read badge: %v", err)
	}
	if err := json.Unmarshal(raw, &b); err != nil {
		t.Fatalf("decode badge: %v", err)
	}
	if b.SchemaVersion != 1 {
		t.Errorf("schemaVersion = %d, want 1", b.SchemaVersion)
	}
	if b.Label != "release" {
		t.Errorf("label = %q, want release", b.Label)
	}
	if b.Message != sampleReleaseTag {
		t.Errorf("message = %q, want %s", b.Message, sampleReleaseTag)
	}
	if b.Color != colorBlue {
		t.Errorf("color = %q, want %q", b.Color, colorBlue)
	}
}
