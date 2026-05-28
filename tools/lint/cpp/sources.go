package main

import (
	"bufio"
	"fmt"
	"io"
	"os"
	"os/exec"
	"path/filepath"
	"sort"
	"strings"
)

// firstPartyIncludeRoots names the only directories that contain
// hand-written, repo-owned C++ that the lint stack is allowed to
// touch. Every other root is either vendored upstream code,
// generated output, a Bazel cache, or build / ops artifacts that
// must not be reformatted by us.
//
// The list is intentionally short and explicit: adding a new
// first-party C++ tree should be a deliberate edit here, not a side
// effect of someone dropping a `.cc` file into an unrelated
// directory.
var firstPartyIncludeRoots = []string{
	"backend/",
	"binaries/",
	"frontend/",
	"tools/googlesql-prebuilt/smoke/",
}

// firstPartyExcludePrefixes lists path prefixes that look first
// party at a glance but are either generated or wrappers around
// upstream code. They are excluded after the include filter so a
// future first-party file under one of the parent directories does
// not accidentally inherit the exclusion.
//
//   - `tools/googlesql-prebuilt/templates/` is `cc_library` glue we
//     stamp into the prebuilt artifact's BUILD file; the C++ headers
//     under it shadow GoogleSQL surface types and follow upstream
//     style.
//   - `binaries/emulator_main/version.cc` is genrule output that
//     `version_gen.sh` writes from `version.cc.tmpl`. The template
//     itself stays in the source list because we hand-write it.
var firstPartyExcludePrefixes = []string{
	"tools/googlesql-prebuilt/templates/",
}

// firstPartyExcludePaths lists individual files that match the
// include filter but must never be linted. Generated artifacts and
// Bazel-stamped outputs go here.
var firstPartyExcludePaths = map[string]struct{}{
	// The genrule output for `binaries/emulator_main:version_cc`.
	// Bazel may stage it under `bazel-out/`, but a stray symlink
	// inside the worktree (or a `bazel run` artifact) must not
	// pull it into the lint set.
	"binaries/emulator_main/version.cc": {},
}

// firstPartyExtensions lists the file extensions we treat as C++
// sources for lint purposes. Headers and source files are listed
// together because clang-format, clang-tidy, and cppcheck all
// expect both to share a single configuration set, and the
// source-only checks (file size, banned logging) apply uniformly.
var firstPartyExtensions = []string{".cc", ".cpp", ".cxx", ".h", ".hpp", ".hh"}

// SourceLister returns the set of tracked C++ files we own. The
// real implementation shells out to `git ls-files`; tests provide a
// fixture-driven stub via the same interface so they do not need a
// live git repo.
type SourceLister interface {
	List() ([]string, error)
}

// gitSourceLister is the production SourceLister. It uses
// `git ls-files` so that .gitignore and the working tree state
// determine the answer — not a brittle filesystem walk that would
// pick up Bazel symlinks (`bazel-*`), the `.cache/` tree, or
// deleted-but-still-on-disk files.
type gitSourceLister struct {
	// repoRoot is passed to `git -C` so the lister works from any
	// subdirectory and from inside test temp dirs.
	repoRoot string
}

// newGitSourceLister returns a SourceLister rooted at the current
// working directory's enclosing git repo. The repo discovery is
// done eagerly so callers fail fast when run outside a checkout.
func newGitSourceLister() (*gitSourceLister, error) {
	root, err := repoRoot()
	if err != nil {
		return nil, err
	}
	return &gitSourceLister{repoRoot: root}, nil
}

// List enumerates the first-party C++ files in repo order. The
// order is stable across runs because `git ls-files` already sorts
// by path; we sort again defensively after applying the filters so
// downstream tools see a deterministic ordering even if git's
// internal order ever changes.
func (g *gitSourceLister) List() ([]string, error) {
	out, err := gitLsFiles(g.repoRoot)
	if err != nil {
		return nil, err
	}
	return filterFirstParty(out), nil
}

// repoRoot returns the absolute path of the enclosing git repo.
// We prefer `git rev-parse --show-toplevel` over walking up looking
// for a `.git` directory because the latter misbehaves inside git
// worktrees (`.git` is a regular file there, not a directory).
func repoRoot() (string, error) {
	cmd := exec.Command("git", "rev-parse", "--show-toplevel")
	out, err := cmd.Output()
	if err != nil {
		return "", fmt.Errorf("git rev-parse: %w", err)
	}
	return strings.TrimSpace(string(out)), nil
}

// gitLsFiles asks git for every tracked file in the working tree.
// We deliberately do NOT pass globs here: a `.cc` file under
// `third_party/` is still tracked, and we want the include /
// exclude lists below to be the single source of truth for what
// "first party" means. Filtering server-side via globs would let a
// pattern bug silently include vendored code.
func gitLsFiles(dir string) ([]string, error) {
	cmd := exec.Command("git", "-C", dir, "ls-files")
	out, err := cmd.Output()
	if err != nil {
		return nil, fmt.Errorf("git ls-files: %w", err)
	}
	scanner := bufio.NewScanner(strings.NewReader(string(out)))
	scanner.Buffer(make([]byte, 0, 64*1024), 1024*1024)
	var files []string
	for scanner.Scan() {
		line := strings.TrimSpace(scanner.Text())
		if line == "" {
			continue
		}
		files = append(files, line)
	}
	if err := scanner.Err(); err != nil {
		return nil, fmt.Errorf("scan ls-files: %w", err)
	}
	return files, nil
}

// filterFirstParty applies the include / exclude rules to a raw
// `git ls-files` output. It is the single function tests pin so
// the ownership boundary cannot drift silently.
func filterFirstParty(all []string) []string {
	var out []string
	for _, p := range all {
		// Normalise forward slashes early. `git ls-files` always
		// uses `/` on every platform git supports, but a stray
		// backslash from a hand-built test fixture or a Windows
		// host should still route through the include filter,
		// so replace backslashes directly. (filepath.ToSlash is
		// a no-op on linux.)
		p = strings.ReplaceAll(p, `\`, "/")
		if !hasCPPExtension(p) {
			continue
		}
		if !underAnyPrefix(p, firstPartyIncludeRoots) {
			continue
		}
		if underAnyPrefix(p, firstPartyExcludePrefixes) {
			continue
		}
		if _, dropped := firstPartyExcludePaths[p]; dropped {
			continue
		}
		out = append(out, p)
	}
	sort.Strings(out)
	return out
}

// hasCPPExtension returns true when `path` looks like a C++ source
// or header file by extension. The check is intentionally
// case-sensitive because every first-party file in this repo
// already uses lowercase extensions and the surrounding tooling
// (clang-format, clang-tidy) follows the same convention.
func hasCPPExtension(path string) bool {
	for _, ext := range firstPartyExtensions {
		if strings.HasSuffix(path, ext) {
			return true
		}
	}
	return false
}

// underAnyPrefix returns true when `path` lives under at least one
// of the supplied directory prefixes. Prefixes must end with `/`
// so a directory named like `backend2/` cannot match
// `backend/`.
func underAnyPrefix(path string, prefixes []string) bool {
	for _, prefix := range prefixes {
		if strings.HasPrefix(path, prefix) {
			return true
		}
	}
	return false
}

// IsTestFile returns true for paths that look like first-party C++
// tests. The convention in this repo is `*_test.cc` next to the
// implementation file, identical to googletest's recommendation;
// we do not have any test-only headers today.
//
// Source-only rules that need to relax for tests (e.g. allowing
// `std::cout` in fixture printers) consult this helper rather than
// hard-coding a path list, so an `*_test.cc` added to a new
// directory is treated correctly without an extra edit here.
func IsTestFile(path string) bool {
	base := filepath.Base(path)
	return strings.HasSuffix(base, "_test.cc") || strings.HasSuffix(base, "_test.cpp")
}

// runList is the `cpp-lint list` subcommand. It prints the
// first-party C++ source list, one path per line, suitable for
// piping into `xargs clang-format`, `xargs clang-tidy`, or any
// other downstream tool.
func runList(args []string, stdout, stderr io.Writer) error {
	fs := flagSet("list", stderr)
	withTests := fs.Bool("tests", true, "include *_test.cc files in the output")
	if err := fs.Parse(args); err != nil {
		return errUsage
	}
	if fs.NArg() != 0 {
		fmt.Fprintln(stderr, "cpp-lint list: takes no positional arguments")
		return errUsage
	}
	lister, err := newGitSourceLister()
	if err != nil {
		return err
	}
	files, err := lister.List()
	if err != nil {
		return err
	}
	for _, f := range files {
		if !*withTests && IsTestFile(f) {
			continue
		}
		_, _ = fmt.Fprintln(stdout, f)
	}
	return nil
}

// readSources returns the first-party source list using the
// production lister. Subcommands that need the list call this
// helper rather than constructing a lister themselves so the test
// suite has a single seam to swap in fixtures.
//
// `repoRoot` is returned alongside so callers can resolve relative
// paths against the same root the lister used. Tests override the
// resolver via the package-level `currentRepoRoot` variable below.
func readSources() ([]string, string, error) {
	root := currentRepoRoot()
	if root == "" {
		discovered, err := repoRoot()
		if err != nil {
			return nil, "", err
		}
		root = discovered
	}
	lister := &gitSourceLister{repoRoot: root}
	files, err := lister.List()
	if err != nil {
		return nil, "", err
	}
	return files, root, nil
}

// currentRepoRoot is a test seam. Tests set it via setRepoRoot to
// pin the lister at a fixture worktree without exporting the
// internal type. The variable is consulted only by readSources()
// and resolveAgainstRoot() so production code paths stay
// unaffected when it is empty.
var testRepoRoot string //nolint:gochecknoglobals // test seam, see setRepoRoot

func currentRepoRoot() string { return testRepoRoot }

// setRepoRoot pins the test-only repo root. It returns a cleanup
// function so test cases can use `defer setRepoRoot(t, dir)()` to
// restore the previous value (always the empty string in
// practice).
func setRepoRoot(dir string) func() {
	prev := testRepoRoot
	testRepoRoot = dir
	return func() { testRepoRoot = prev }
}

// resolveAgainstRoot joins a first-party-relative path with the
// repo root. We never accept absolute paths from the source lister
// because every downstream consumer expects repo-relative output;
// the repo root is only spliced back in when the check needs to
// open the file from disk.
func resolveAgainstRoot(root, rel string) string {
	return filepath.Join(root, filepath.FromSlash(rel))
}

// readFile returns the file contents at path. Centralised so the
// check helpers do not each grow their own ioutil-style boilerplate
// (and so a future swap to memory-mapped reads has a single seam).
func readFile(path string) ([]byte, error) {
	//nolint:gosec // Paths come from the first-party source lister, which is itself tested.
	return os.ReadFile(path)
}
