# `frontend/handlers/`

C++ implementations of the gRPC services declared in
[`../../proto/emulator.proto`](../../proto/emulator.proto).

Current files:

- `catalog.h`, `catalog.cc` — `Catalog` service (datasets, tables, schemas)
- `query.h`, `query.cc` — `Query` service: `ExecuteQuery`, `DryRun`
  - Wires `googlesql::Analyzer::AnalyzeStatement` to the DuckDB-backed
    catalog and forwards resolved ASTs to the DuckDB engine.
- `storage_read.h`, `storage_read.cc` — `StorageRead` service:
  `CreateReadSession`, `ReadRows`.

Each handler should be lean: schema/catalog state lives in `backend/`,
SQL semantics live in upstream GoogleSQL, and the handlers are just glue.
