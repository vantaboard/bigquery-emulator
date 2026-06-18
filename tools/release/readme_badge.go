package main

import (
	"fmt"
	"io"
	"os"
	"regexp"
	"strings"
)

const defaultReadmePath = "README.md"

// badgeReleaseCacheBusterRe matches the shields.io endpoint URL cache-buster
// query param on the README release badge line.
var badgeReleaseCacheBusterRe = regexp.MustCompile(
	`(badge-release\.json&v=)[^)]+`,
)

func readmeBadgeCacheVersion(version string) (string, error) {
	version = strings.TrimSpace(version)
	if version == "" {
		return "", fmt.Errorf("version is required")
	}
	return strings.TrimPrefix(version, "v"), nil
}

func patchReadmeBadgeCacheBuster(readme []byte, cacheVersion string) ([]byte, bool, error) {
	if !badgeReleaseCacheBusterRe.Match(readme) {
		return nil, false, fmt.Errorf("README release badge cache buster not found")
	}
	replacement := []byte("${1}" + cacheVersion)
	out := badgeReleaseCacheBusterRe.ReplaceAll(readme, replacement)
	return out, !bytesEqual(readme, out), nil
}

func bytesEqual(a, b []byte) bool {
	if len(a) != len(b) {
		return false
	}
	for i := range a {
		if a[i] != b[i] {
			return false
		}
	}
	return true
}

func runReadmeBadge(args []string, stdout, stderr io.Writer) error {
	fs := flagSet("readme-badge", stderr)
	version := fs.String("version", "", "release tag to render (e.g. v0.3.0)")
	readmePath := fs.String("readme", defaultReadmePath, "README path to patch")
	if err := fs.Parse(args); err != nil {
		return err
	}

	cacheVersion, err := readmeBadgeCacheVersion(*version)
	if err != nil {
		return err
	}

	raw, err := os.ReadFile(*readmePath)
	if err != nil {
		return fmt.Errorf("read %q: %w", *readmePath, err)
	}

	patched, changed, err := patchReadmeBadgeCacheBuster(raw, cacheVersion)
	if err != nil {
		return err
	}
	if !changed {
		_, _ = fmt.Fprintf(stdout, "readme-badge: %s already at &v=%s\n", *readmePath, cacheVersion)
		return nil
	}

	//nolint:gosec // 0o644 is the right mode for a tracked README.
	if err := os.WriteFile(*readmePath, patched, 0o644); err != nil {
		return fmt.Errorf("write %q: %w", *readmePath, err)
	}
	_, _ = fmt.Fprintf(stdout, "readme-badge: patched %s to &v=%s\n", *readmePath, cacheVersion)
	return nil
}
