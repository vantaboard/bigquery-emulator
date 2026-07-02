# Python UDFs (local)

The emulator evaluates scalar `CREATE FUNCTION ... LANGUAGE python` bodies
locally via a sandboxed `python3` subprocess (`python_udf_runtime.cc`). This
guide covers third-party `packages` declared in `OPTIONS` — resolution against
an operator-provisioned environment, not live PyPI at query time.

## Interpreter resolution order

At call time the runtime picks a Python interpreter in this order:

1. **`BIGQUERY_EMULATOR_PYTHON`** — explicit path (venv `bin/python3`, container
   interpreter, etc.).
2. **`$BIGQUERY_EMULATOR_DATA_DIR/python-udf-env/bin/python3`** — managed venv
   when present and executable. `emulator_main` sets
   `BIGQUERY_EMULATOR_DATA_DIR` from `--data_dir`.
3. **Host `python3`** — stdlib plus whatever happens to be importable (e.g.
   distro `python3-lxml`).

## Declaring packages

```sql
CREATE FUNCTION py_tag(x STRING) RETURNS STRING
LANGUAGE python
OPTIONS (entry_point='do_tag', packages=['lxml'])
AS r"""
from lxml import etree
def do_tag(x):
  return etree.fromstring(x).tag
""";
```

- `packages` is parsed at `CREATE FUNCTION` time, persisted with the routine
  DDL, and echoed on `routines.get` as `pythonOptions.packages`.
- Before evaluating a UDF, the runtime preflights each package with
  `importlib.util.find_spec` in the chosen interpreter. A missing package
  surfaces a BigQuery-shaped `invalidQuery` error naming the package and pointing
  here — not a Python traceback.

## Provisioning (operator-only)

**No `pip install` runs during query execution.** To install declared packages
into the managed venv:

```bash
export BIGQUERY_EMULATOR_DATA_DIR="${BIGQUERY_EMULATOR_DATA_DIR:-$HOME/.bigquery-emulator}"
export BIGQUERY_EMULATOR_PYTHON_ALLOW_PIP=1   # required opt-in
task python-udf:provision -- lxml pandas
```

With no package arguments, `task python-udf:provision` scans
`$BIGQUERY_EMULATOR_DATA_DIR/catalog.duckdb` (`__bqemu_routines`) for the
union of `packages` declared on registered `LANGUAGE python` routines and
installs that set into the managed venv.

```bash
export BIGQUERY_EMULATOR_PYTHON_ALLOW_PIP=1
task python-udf:provision
```

This creates or updates `$BIGQUERY_EMULATOR_DATA_DIR/python-udf-env` and runs
`pip install` only inside that provisioning step.

To pin the interpreter explicitly after provisioning:

```bash
export BIGQUERY_EMULATOR_PYTHON="$BIGQUERY_EMULATOR_DATA_DIR/python-udf-env/bin/python3"
```

## Conformance fixtures

- `conformance/fixtures/udf/python_packages_lxml.yaml` — happy path when `lxml`
  is importable (CI installs `python3-lxml`).
- `conformance/fixtures/udf/python_packages_missing.yaml` — structured error for
  a deliberately absent package name.

## Out of scope

- Arbitrary/unpinned live `pip` at query time.
- Package version pinning semantics matching BigQuery container images.
- `CREATE AGGREGATE FUNCTION` / `CREATE TABLE FUNCTION` with `LANGUAGE python`
  (production BigQuery rejects these shapes; the emulator mirrors that).

See also [`docs/ENGINE_POLICY.md`](../ENGINE_POLICY.md) and
[`ROADMAP.md`](https://github.com/vantaboard/bigquery-emulator/blob/main/ROADMAP.md)
§Python UDFs.
