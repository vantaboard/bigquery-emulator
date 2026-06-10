// Package storagetmpl materializes an initial-data template tree into
// the persistent storage root the engine reads on startup.
//
// An operator (or a
// container image) ships a pre-populated catalog plus row files, and
// the emulator should copy them into the runtime data-dir on first
// boot so a downstream client sees a populated catalog at t=0. Once
// the data-dir is initialized, subsequent boots are no-ops -- the
// template is only ever copied into an empty (or absent) destination
// so we never clobber writes a previous run committed.
//
// "Initialized" detection. This repo's engine uses DuckDB, which
// keeps its catalog in a single file named `catalog.duckdb` plus
// sidecar `.parquet` / `.meta.json` files (see
// backend/storage/duckdb/duckdb_storage.cc). The presence of
// `catalog.duckdb` is therefore our sentinel: when it already
// exists in the destination we leave the tree alone and assume a
// prior run owns it. When the destination is absent OR exists but
// does not yet contain a `catalog.duckdb` file, we treat it as a
// fresh data-dir and copy the entire template tree in.
package storagetmpl

import (
	"errors"
	"fmt"
	"io"
	"io/fs"
	"os"
	"path/filepath"
)

// catalogSentinel is the file whose presence in the destination
// data-dir means the engine has already initialized this catalog.
// Exported as a package-level var so tests can swap it for a
// storage-backend-specific sentinel without rewriting the helper.
var catalogSentinel = "catalog.duckdb"

// MaybeMaterialize copies `template` into `dataDir` when `dataDir`
// does not yet contain the engine's initialized-catalog sentinel.
//
// Returns nil (no-op) when:
//   - template is "" (operator did not configure an initial-data-dir).
//   - dataDir is "" (the engine will use its in-memory fallback; there
//     is no on-disk tree to seed).
//   - dataDir already contains the sentinel file (a previous run owns
//     the catalog; copying would clobber writes).
//
// Returns an error when:
//   - template does not exist, is not a directory, or is unreadable.
//   - dataDir exists but is not a directory.
//   - any file copy fails (disk full, permission denied, ...).
//
// The copy preserves file mode bits but does NOT preserve
// ownership/atime/mtime; the engine does not depend on either, and
// `os.Chown` would require CAP_CHOWN for cross-uid operator
// scenarios that are out of scope here.
func MaybeMaterialize(template, dataDir string) error {
	if template == "" || dataDir == "" {
		return nil
	}
	tplInfo, err := os.Stat(template)
	if err != nil {
		return fmt.Errorf("storagetmpl: stat template %q: %w", template, err)
	}
	if !tplInfo.IsDir() {
		return fmt.Errorf("storagetmpl: template %q is not a directory", template)
	}

	// If the destination already exists, ensure it's a directory.
	dstInfo, err := os.Stat(dataDir)
	switch {
	case err == nil:
		if !dstInfo.IsDir() {
			return fmt.Errorf("storagetmpl: data-dir %q exists but is not a directory", dataDir)
		}
		// Already initialized? Sentinel presence wins; treat
		// this run as a continuation rather than reseeding.
		if isInitialized(dataDir) {
			return nil
		}
	case errors.Is(err, fs.ErrNotExist):
		if mkErr := os.MkdirAll(dataDir, 0o750); mkErr != nil {
			return fmt.Errorf("storagetmpl: create data-dir %q: %w", dataDir, mkErr)
		}
	default:
		return fmt.Errorf("storagetmpl: stat data-dir %q: %w", dataDir, err)
	}

	if err := copyTree(template, dataDir); err != nil {
		return fmt.Errorf("storagetmpl: copy %q -> %q: %w", template, dataDir, err)
	}
	return nil
}

// isInitialized reports whether dataDir already contains the
// catalog sentinel. Errors are treated as "not initialized" so a
// permission glitch causes the copy to proceed and fail with a
// concrete error rather than silently skipping.
func isInitialized(dataDir string) bool {
	info, err := os.Stat(filepath.Join(dataDir, catalogSentinel))
	return err == nil && !info.IsDir()
}

// copyTree walks src and mirrors its layout under dst. Existing
// destination files are overwritten -- callers gate this entire
// function on `isInitialized` so the operator must explicitly
// remove `catalog.duckdb` to reseed.
func copyTree(src, dst string) error {
	return filepath.WalkDir(src, func(path string, d fs.DirEntry, err error) error {
		if err != nil {
			return err
		}
		rel, err := filepath.Rel(src, path)
		if err != nil {
			return err
		}
		target := filepath.Join(dst, rel)

		if d.IsDir() {
			info, infoErr := d.Info()
			if infoErr != nil {
				return infoErr
			}
			return os.MkdirAll(target, info.Mode().Perm())
		}
		if d.Type()&fs.ModeSymlink != 0 {
			// Resolve and copy the symlink's content; the
			// template is operator-supplied so we never
			// preserve the link itself (avoids escapes
			// outside dst).
			resolved, linkErr := os.Readlink(path)
			if linkErr != nil {
				return linkErr
			}
			realPath := resolved
			if !filepath.IsAbs(realPath) {
				realPath = filepath.Join(filepath.Dir(path), resolved)
			}
			realInfo, statErr := os.Stat(realPath)
			if statErr != nil {
				return statErr
			}
			if realInfo.IsDir() {
				return os.MkdirAll(target, realInfo.Mode().Perm())
			}
			return copyFile(realPath, target, realInfo.Mode().Perm())
		}
		info, infoErr := d.Info()
		if infoErr != nil {
			return infoErr
		}
		return copyFile(path, target, info.Mode().Perm())
	})
}

// copyFile copies src -> dst with the given mode. The destination's
// parent is created on demand so a deeply nested template root works
// without the caller pre-creating directories.
//
// `src` and `dst` come from an operator-supplied template tree
// walked by filepath.WalkDir; gosec G304/G306 fire on the variable
// open / Chmod paths but that is the entire point of the helper
// (we're materializing an operator-named directory), so we suppress
// the warnings inline.
func copyFile(src, dst string, mode fs.FileMode) error {
	if mkErr := os.MkdirAll(filepath.Dir(dst), 0o750); mkErr != nil {
		return mkErr
	}
	in, err := os.Open(src) //nolint:gosec // src walks an operator-supplied template tree
	if err != nil {
		return err
	}
	defer func() { _ = in.Close() }()

	// Use O_TRUNC so a partially-copied destination from a prior
	// crashed boot does not produce a frankenfile. dst here is
	// the operator-supplied data-dir; the gosec warning is the
	// entire point of the helper.
	out, err := os.OpenFile( //nolint:gosec // dst is the operator-supplied data-dir
		dst, os.O_WRONLY|os.O_CREATE|os.O_TRUNC, mode,
	)
	if err != nil {
		return err
	}
	if _, copyErr := io.Copy(out, in); copyErr != nil {
		_ = out.Close()
		return copyErr
	}
	return out.Close()
}
