package main

import (
	"bufio"
	"errors"
	"fmt"
	"io"
	"os"
	"path/filepath"
	"sort"
	"strings"
)

// readBaseline parses a baseline file at `path`. The format is one
// repo-relative path per non-empty line; `#` introduces a
// line-level comment. A missing file is not an error — we treat
// "no baseline file" as "no entries", which is the default state
// before maintainers explicitly grandfather a path.
//
// The baseline is intentionally a flat list rather than a structured
// (path, max_lines) table: the goal is to grandfather existing
// offenders so the lint gate can land, NOT to encode per-file
// thresholds long term. Each entry should reference an issue in
// the surrounding comment so the burndown is auditable.
func readBaseline(path string) (map[string]struct{}, error) {
	f, err := os.Open(path) //nolint:gosec // path is supplied by the maintainer-controlled flag, not user input
	if errors.Is(err, os.ErrNotExist) {
		return map[string]struct{}{}, nil
	}
	if err != nil {
		return nil, err
	}
	defer f.Close() //nolint:errcheck // read-only; close errors are not actionable

	out := map[string]struct{}{}
	scanner := bufio.NewScanner(f)
	scanner.Buffer(make([]byte, 0, 64*1024), 1024*1024)
	for scanner.Scan() {
		entry := stripBaselineLine(scanner.Text())
		if entry == "" {
			continue
		}
		out[entry] = struct{}{}
	}
	if err := scanner.Err(); err != nil {
		return nil, fmt.Errorf("scan baseline: %w", err)
	}
	return out, nil
}

// stripBaselineLine removes inline comments and surrounding
// whitespace. Returning an empty string signals "skip this line".
// Backslashes are normalised to forward slashes so a copy-pasted
// Windows-style path round-trips correctly.
func stripBaselineLine(line string) string {
	if idx := strings.Index(line, "#"); idx >= 0 {
		line = line[:idx]
	}
	return strings.TrimSpace(strings.ReplaceAll(line, `\`, "/"))
}

// runBaseline is the `cpp-lint baseline` subcommand. It rewrites
// the baseline file so it reflects the current state of the tree:
// every first-party file whose length still exceeds `--max-lines`
// is listed, and every previously-listed file that has shrunk
// below the threshold is dropped.
//
// The subcommand is a maintainer ergonomics affordance — `task
// lint:cpp:baseline` runs it after a refactor so a contributor
// does not have to hand-edit the file. CI never invokes it.
func runBaseline(args []string, stdout, stderr io.Writer) error {
	fs := flagSet("baseline", stderr)
	maxLines := fs.Int("max-lines", 500, "the same line cap that 'check' enforces")
	output := fs.String("output", filepath.FromSlash("tools/lint/cpp/baseline.txt"), "destination path for the rewritten baseline")
	if err := fs.Parse(args); err != nil {
		return errUsage
	}
	if fs.NArg() != 0 {
		fmt.Fprintln(stderr, "cpp-lint baseline: takes no positional arguments")
		return errUsage
	}

	files, root, err := readSources()
	if err != nil {
		return err
	}
	var oversized []string
	for _, rel := range files {
		body, rerr := readFile(resolveAgainstRoot(root, rel))
		if rerr != nil {
			return fmt.Errorf("read %s: %w", rel, rerr)
		}
		if countLines(body) > *maxLines {
			oversized = append(oversized, rel)
		}
	}
	sort.Strings(oversized)

	dest := filepath.Join(root, *output)
	if err := writeBaseline(dest, oversized); err != nil {
		return err
	}
	_, _ = fmt.Fprintf(stdout, "wrote %d entries to %s\n", len(oversized), *output)
	return nil
}

// writeBaseline rewrites the baseline file at `path` with the
// supplied entries. We always write the same header so a
// `git diff` of the baseline reads cleanly; the entries
// themselves are sorted by the caller.
func writeBaseline(path string, entries []string) error {
	const header = `# tools/lint/cpp/baseline.txt
#
# Files in this list are allowed to exceed the first-party C++
# whole-file line cap enforced by ` + "`task lint:cpp:source`" + `.
# Each entry is one repo-relative path per line; '#' starts a
# comment. The list is intentionally a ratchet: regenerate via
# ` + "`task lint:cpp:baseline`" + ` after a refactor so the gate stays
# tight without forcing a sweeping rewrite up-front.
#
# Every grandfathered entry should be paired with a follow-up
# issue tracking the eventual split. Do NOT add a new file here
# without that issue link in the commit message.

`
	tmp := path + ".tmp"
	f, err := os.Create(tmp) //nolint:gosec // path is maintainer-controlled
	if err != nil {
		return err
	}
	if _, err := io.WriteString(f, header); err != nil {
		_ = f.Close()
		return err
	}
	for _, e := range entries {
		if _, err := fmt.Fprintln(f, e); err != nil {
			_ = f.Close()
			return err
		}
	}
	if err := f.Close(); err != nil {
		return err
	}
	return os.Rename(tmp, path)
}
