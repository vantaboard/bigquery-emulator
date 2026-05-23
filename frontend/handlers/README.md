# `frontend/handlers/`

C++ implementations of the gRPC services declared in
[`../../proto/emulator.proto`](../../proto/emulator.proto).

Planned files (Phase 2+ of [`ROADMAP.md`](../../ROADMAP.md)):

- `catalog.h`, `catalog.cc` — `Catalog` service (datasets, tables, schemas)
- `query.h`, `query.cc` — `Query` service: `ExecuteQuery`, `DryRun`
  - Wires `googlesql::Analyzer::AnalyzeStatement` to the in-memory catalog
    and pipes resolved ASTs into `googlesql::reference_impl::Evaluator`.

Each handler should be lean: schema/catalog state lives in `backend/`,
SQL semantics live in upstream GoogleSQL, and the handlers are just glue.
