# Engine policy

This emulator ships **one C++ engine** behind the
`backend/engine/engine.h` interface: the DuckDB engine. There is no
runtime selector and no fallback bridge; new work targets DuckDB.

> **TL;DR**: build against **DuckDB**. The previous ReferenceImpl
> engine, the FallbackEngine bridge, and the in-memory storage
> backend were removed once DuckDB reached parity for the supported
> surface. See "History" below for what changed.

## The runtime

| Component | Source | Notes |
|---|---|---|
| **DuckDB engine** | `backend/engine/duckdb/` | Sole engine implementation. GoogleSQL analyzes; the transpiler lowers to DuckDB SQL; DuckDB executes. |
| **DuckDB storage** | `backend/storage/duckdb/` | Sole storage implementation. Catalog + table rows persist to a `catalog.duckdb` file under `--data_dir`. |

`emulator_main` no longer accepts `--engine`, `--storage`, or
`--on_unknown_fn`; those flags were removed when the ReferenceImpl
engine and in-memory storage were deleted. The only knobs are
`--host_port` (default `localhost:9060`) and `--data_dir` (default
`$HOME/.bigquery-emulator`, with `./.bigquery-emulator` as the
fallback when `HOME` is unset).

## What this means in practice

1. **New feature work targets DuckDB.** Add the construct to the
   DuckDB transpiler (`backend/engine/duckdb/transpile/`) and the
   DuckDB engine (`backend/engine/duckdb/duckdb_engine.cc`).

2. **Some DML / DDL shapes are still UNIMPLEMENTED on DuckDB.** As
   of this revision the engine returns UNIMPLEMENTED for:
   - `INSERT VALUES` / `INSERT ... SELECT`
   - `UPDATE`
   - `DELETE`
   - Scalar-only `SELECT` (no `FROM` clause)

   Use `tabledata.insertAll` to seed rows for tests and fixtures
   while these gaps are closed. `MERGE`, `CREATE TABLE`,
   `CREATE TABLE AS SELECT`, and `DROP TABLE` are all implemented.

3. **Storage follows the same single-implementation rule.** The
   in-memory storage backend is gone; every persistent state path
   goes through `DuckDBStorage`. Tests that previously used a
   volatile in-memory store now allocate a temp `--data_dir` and
   rely on DuckDB instead.

## Implication for conformance fixtures

The conformance harness (`conformance/cmd/runner`,
`conformance/fixtures/*.yaml`) runs a single profile today:

| Profile | Engine | Storage |
|---|---|---|
| `duckdb` | duckdb | duckdb |

Per the policy above:

- **Leave `profiles:` unset** in new fixtures unless you are
  intentionally targeting a future second profile. Today the
  default profile set is `[duckdb]` and the harness runs every
  fixture against it.
- **Use `rows:` setup steps** (which call `tabledata.insertAll`)
  instead of `sql:` `INSERT VALUES` for seeding, since INSERT is
  UNIMPLEMENTED on the DuckDB engine.
- **Document UNIMPLEMENTED gaps loudly.** If a fixture is blocked
  on a DuckDB gap (INSERT/UPDATE/DELETE/MERGE-only-no), leave it
  out of the suite rather than `t.Skip()`-ing it; the conformance
  harness's purpose is to pin what works.

## History

A previous iteration of the emulator carried two engines
(ReferenceImpl + DuckDB) bridged by a FallbackEngine wrapper that
routed DuckDB-uncovered constructs to ReferenceImpl, plus an
in-memory storage backend for hermetic tests. That layout was
removed because:

- ReferenceImpl coverage was incomplete (no DDL, partial DML,
  missing analytics functions) and not actively maintained.
- The fallback bridge made it ambiguous which engine produced any
  given result; production users reported subtle divergences when
  fixtures ran on `duckdb` but tests ran on `memory`.
- Maintaining two storage backends doubled the test-fixture cost
  for no production benefit, since `docker compose up` always
  used the persistent DuckDB store anyway.

The removal commit landed with a `BREAKING CHANGE:` footer noting
that `--engine=reference_impl`, `--storage=memory`, and
`--on_unknown_fn=fallback` are gone and DuckDB engine + DuckDB
storage is the only supported runtime.

## Cross-references

- `conformance/README.md` — fixture authoring guide; references
  this document from its "Contributing a new fixture" section.
- [`README.md` "Runtime configuration"](../README.md#runtime-configuration) —
  the user-facing version of the flag surface this document
  governs.
