---
name: Python UDF packages
overview: Bounded support for the CREATE FUNCTION packages option — resolve declared packages against a local, operator-provisioned environment instead of live pip installs.
todos:
  - id: parse-persist-packages
    content: Parse the packages OPTIONS list at CREATE FUNCTION time, persist it with the routine, surface it in routines.get
    status: pending
  - id: env-resolution
    content: "Resolve packages against a local env: BIGQUERY_EMULATOR_PYTHON venv first, then an optional offline wheel dir; never call live PyPI by default"
    status: pending
  - id: missing-package-envelope
    content: Missing package at call time surfaces a BigQuery-shaped error naming the package and the provisioning command
    status: pending
  - id: docs-fixtures
    content: docs/guides provisioning walkthrough + conformance fixture using a stdlib-adjacent package (lxml) + ROADMAP/ENGINE_POLICY updates
    status: pending
isProject: false
---

# 04 — Python UDF `packages` (bounded real)

- **Roadmap row:** §Python UDFs — "⏳ Arbitrary `packages` pip installs remain
  unsupported (pre-installed stdlib + `lxml` when present on the host
  interpreter)"

## Current state at HEAD (grounded)

- `python_udf_runtime.cc` runs UDF bodies in a sandboxed `python3` subprocess
  using the host interpreter (or `BIGQUERY_EMULATOR_PYTHON` override). Only
  the stdlib plus whatever happens to be importable on that interpreter
  (e.g. `lxml`, which the bqutils `cw_xml_extract` golden relies on) works.
- The `packages=[...]` option in `CREATE FUNCTION ... OPTIONS(...)` is not
  parsed into anything actionable — a UDF declaring packages either fails at
  import time with a raw Python traceback or silently works if the host has
  the module.

## Design constraint

**No live `pip install` at query time by default.** The emulator's posture is
local-only + deterministic; letting a `CREATE FUNCTION` reach PyPI would be a
supply-chain and reproducibility hole. The plan is *resolution against an
operator-provisioned environment*, with the provisioning step documented and
scriptable.

## Done-criteria

1. `packages` declared in OPTIONS round-trips through `routines.get`.
2. A UDF whose packages are all importable in the configured interpreter
   evaluates normally.
3. A UDF whose package is missing fails with a BigQuery-shaped
   `INVALID_ARGUMENT`/`FAILED_PRECONDITION` naming the package and pointing at
   the provisioning doc — not a Python traceback.
4. An explicit opt-in (`BIGQUERY_EMULATOR_PYTHON_ALLOW_PIP=1` or a task
   helper) can pre-install declared packages into a dedicated venv under
   `$data_dir` — as a provisioning action, never mid-query.

## Implementation steps

### Step 1 — parse + persist

Extract `packages` from the resolved CREATE FUNCTION options in the Python
registry path, store it in the persisted routine row (`__bqemu_routines`),
and echo it in the REST `routines.get` response.

### Step 2 — interpreter selection + venv layout

Teach the runtime a resolution order: (1) explicit
`BIGQUERY_EMULATOR_PYTHON`; (2) a managed venv at
`$data_dir/python-udf-env/` if present; (3) host `python3`. Add a task helper
(`task python-udf:provision`) that creates the venv and installs the union of
packages declared by registered routines (this is the only place pip runs,
and only when the operator invokes it or sets the opt-in env var).

### Step 3 — preflight check + envelope

Before evaluating a UDF with declared packages, preflight
`importlib.util.find_spec` for each (cache per interpreter). On a miss,
surface the structured error from done-criterion 3 instead of executing the
body.

## Tests

- Unit: options parsing, resolution order, preflight-miss envelope.
- Conformance: `conformance/fixtures/udf/python_packages_lxml.yaml` — declares
  `packages=['lxml']`, skipped-or-passing depending on host availability
  (mirror how existing host-dependent fixtures gate), plus a
  `python_packages_missing.yaml` pinning the error envelope with a
  deliberately absent package name.
- CI: provision the venv with `lxml` in the conformance workflow so the happy
  path is exercised deterministically.

## Out of scope

- Arbitrary/unpinned live pip at query time (permanently, per posture).
- Package version pinning semantics matching BigQuery's container images.
- Non-Python (JS) dependency management.

## Touch list

`backend/catalog/python_udf_registry.cc`,
`backend/engine/semantic/python_udf_runtime.{h,cc}`, `taskfiles/` (new
provision task), `conformance/fixtures/udf/`, `.github/workflows/conformance.yml`,
`docs/guides/python-udfs.md` (new), `docs/ENGINE_POLICY.md`, `ROADMAP.md`.
