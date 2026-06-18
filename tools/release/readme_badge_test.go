package main

import (
	"os"
	"path/filepath"
	"strings"
	"testing"
)

const sampleReadmeBadgeLine = `[![release](https://img.shields.io/endpoint?url=https://raw.githubusercontent.com/vantaboard/bigquery-emulator/gh-pages/badge-release.json&v=1)](https://github.com/vantaboard/bigquery-emulator/releases/latest)`

func TestReadmeBadgeCacheVersion(t *testing.T) {
	cases := map[string]string{
		"v0.3.0": "0.3.0",
		"0.3.0":  "0.3.0",
	}
	for in, want := range cases {
		got, err := readmeBadgeCacheVersion(in)
		if err != nil {
			t.Fatalf("readmeBadgeCacheVersion(%q): %v", in, err)
		}
		if got != want {
			t.Errorf("readmeBadgeCacheVersion(%q) = %q, want %q", in, got, want)
		}
	}
	if _, err := readmeBadgeCacheVersion(""); err == nil {
		t.Fatal("readmeBadgeCacheVersion(\"\") expected error")
	}
}

func TestPatchReadmeBadgeCacheBuster(t *testing.T) {
	raw := []byte("# Title\n\n" + sampleReadmeBadgeLine + "\n")
	patched, changed, err := patchReadmeBadgeCacheBuster(raw, "0.3.0")
	if err != nil {
		t.Fatalf("patchReadmeBadgeCacheBuster: %v", err)
	}
	if !changed {
		t.Fatal("expected changed=true")
	}
	want := "badge-release.json&v=0.3.0"
	if !strings.Contains(string(patched), want) {
		t.Fatalf("patched README missing %q:\n%s", want, patched)
	}
	if strings.Contains(string(patched), "&v=1") {
		t.Fatalf("patched README still contains old cache buster:\n%s", patched)
	}
}

func TestRunReadmeBadge_EndToEnd(t *testing.T) {
	dir := t.TempDir()
	readmePath := filepath.Join(dir, "README.md")
	if err := os.WriteFile(readmePath, []byte(sampleReadmeBadgeLine+"\n"), 0o644); err != nil {
		t.Fatalf("write readme: %v", err)
	}

	if err := run([]string{
		cmdReadmeBadge,
		"--version=v0.3.0",
		"--readme=" + readmePath,
	}, os.Stdout, os.Stderr); err != nil {
		t.Fatalf("readme-badge: %v", err)
	}

	raw, err := os.ReadFile(readmePath)
	if err != nil {
		t.Fatalf("read readme: %v", err)
	}
	if !strings.Contains(string(raw), "badge-release.json&v=0.3.0") {
		t.Fatalf("README not patched:\n%s", raw)
	}
}
