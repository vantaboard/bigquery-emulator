package main

import (
	"bytes"
	"encoding/json"
	"path/filepath"
	"strings"
	"testing"
)

// TestBadgeColor pins the colour bands so a future refactor of
// badgeColor stays inside the codecov-compatible ranges the README
// already advertises.
func TestBadgeColor(t *testing.T) {
	cases := []struct {
		pct  float64
		want string
	}{
		{-1, colorLightgrey},
		{0, colorRed},
		{39.9, colorRed},
		{40, colorOrange},
		{59.9, colorOrange},
		{60, colorYellow},
		{69.9, colorYellow},
		{70, colorYellowGreen},
		{79.9, colorYellowGreen},
		{80, colorGreen},
		{89.9, colorGreen},
		{90, colorBrightGreen},
		{100, colorBrightGreen},
	}
	for _, c := range cases {
		got := badgeColor(c.pct)
		if got != c.want {
			t.Errorf("badgeColor(%v) = %q, want %q", c.pct, got, c.want)
		}
	}
}

// TestBadgeMessage pins the message format: one decimal place for real
// data, the missing-data sentinel for the negative input.
func TestBadgeMessage(t *testing.T) {
	cases := map[float64]string{
		-1:   missingMessage,
		0:    "0.0%",
		71.3: "71.3%",
		100:  "100.0%",
	}
	for in, want := range cases {
		got := badgeMessage(in)
		if got != want {
			t.Errorf("badgeMessage(%v) = %q, want %q", in, got, want)
		}
	}
}

// TestRunBadge_EndToEnd exercises the full subcommand: summarize a
// fixture, then render the "go" field as a shields.io endpoint JSON.
// The recorded shape matches what shields.io's endpoint badge
// consumer expects.
func TestRunBadge_EndToEnd(t *testing.T) {
	dir := t.TempDir()
	summaryPath := filepath.Join(dir, "summary.json")
	badgePath := filepath.Join(dir, "badge.json")

	var out, errBuf bytes.Buffer
	if err := run([]string{
		cmdSummarize,
		goFixtureFlag,
		"--lcov=testdata/coverage.dat",
		"--out=" + summaryPath,
	}, &out, &errBuf); err != nil {
		t.Fatalf("summarize: %v\nstderr=%s", err, errBuf.String())
	}

	out.Reset()
	errBuf.Reset()
	if err := run([]string{
		cmdBadge,
		"--in=" + summaryPath,
		"--out=" + badgePath,
		"--field=go",
		"--label=coverage (go)",
	}, &out, &errBuf); err != nil {
		t.Fatalf("badge: %v\nstderr=%s", err, errBuf.String())
	}

	var b badgeJSON
	if err := json.Unmarshal(mustRead(t, badgePath), &b); err != nil {
		t.Fatalf("decode badge: %v", err)
	}
	if b.SchemaVersion != 1 {
		t.Errorf("schemaVersion = %d, want 1", b.SchemaVersion)
	}
	if b.Label != "coverage (go)" {
		t.Errorf("label = %q, want %q", b.Label, "coverage (go)")
	}
	if b.Message != "58.3%" {
		t.Errorf("message = %q, want 58.3%%", b.Message)
	}
	if b.Color != colorOrange {
		t.Errorf("color = %q, want %q", b.Color, colorOrange)
	}
}

// TestRunBadge_UnknownField documents that a typo in --field fails
// loudly so a workflow misconfiguration cannot ship a "total" badge
// labelled "go" by accident.
func TestRunBadge_UnknownField(t *testing.T) {
	dir := t.TempDir()
	summaryPath := filepath.Join(dir, "summary.json")
	var out, errBuf bytes.Buffer
	if err := run([]string{
		cmdSummarize,
		goFixtureFlag,
		"--out=" + summaryPath,
	}, &out, &errBuf); err != nil {
		t.Fatalf("summarize: %v", err)
	}

	out.Reset()
	errBuf.Reset()
	err := run([]string{
		cmdBadge,
		"--in=" + summaryPath,
		"--field=does-not-exist",
	}, &out, &errBuf)
	if err == nil {
		t.Fatal("err = nil, want error")
	}
	if !strings.Contains(err.Error(), "unknown --field") {
		t.Errorf("err %q should mention unknown --field", err)
	}
}
