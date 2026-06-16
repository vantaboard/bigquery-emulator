package main

import (
	"encoding/json"
	"fmt"
	"io"
	"os"
	"strings"
)

const (
	missingMessage = "n/a"
	colorBlue      = "blue"
	colorOrange    = "orange"
	colorLightgrey = "lightgrey"
)

type badgeJSON struct {
	SchemaVersion int    `json:"schemaVersion"`
	Label         string `json:"label"`
	Message       string `json:"message"`
	Color         string `json:"color"`
}

func badgeMessage(version string) string {
	version = strings.TrimSpace(version)
	if version == "" {
		return missingMessage
	}
	return version
}

func badgeColor(version string) string {
	version = strings.TrimSpace(version)
	if version == "" {
		return colorLightgrey
	}
	if isPreRelease(version) {
		return colorOrange
	}
	return colorBlue
}

func isPreRelease(tag string) bool {
	tag = strings.TrimPrefix(tag, "v")
	return strings.Contains(tag, "-")
}

func runBadge(args []string, stdout, stderr io.Writer) error {
	fs := flagSet("badge", stderr)
	version := fs.String("version", "", "release tag to render (empty renders n/a)")
	out := fs.String("out", "", "output badge JSON path (default: stdout)")
	label := fs.String("label", "release", "badge label")
	if err := fs.Parse(args); err != nil {
		return err
	}

	payload := badgeJSON{
		SchemaVersion: 1,
		Label:         *label,
		Message:       badgeMessage(*version),
		Color:         badgeColor(*version),
	}
	return emitBadge(&payload, *out, stdout)
}

func emitBadge(b *badgeJSON, outPath string, stdout io.Writer) error {
	buf, err := json.MarshalIndent(b, "", "  ")
	if err != nil {
		return fmt.Errorf("marshal badge: %w", err)
	}
	buf = append(buf, '\n')

	if outPath == "" {
		_, err = stdout.Write(buf)
		return err
	}
	//nolint:gosec // 0o644 is the right mode for a CI-published JSON artifact.
	if err := os.WriteFile(outPath, buf, 0o644); err != nil {
		return fmt.Errorf("write %q: %w", outPath, err)
	}
	return nil
}
