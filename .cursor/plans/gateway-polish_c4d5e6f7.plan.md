---
name: gateway-polish
overview: "Complete Phase 1 remainder on the Go gateway: discovery document stub, auth middleware (parse/ignore bearer tokens), useLegacySql rejection, and BIGQUERY_EMULATOR_HOST documentation. Routes are already wired; this plan makes client libraries happy at connect time."
todos:
  - id: auth-middleware
    content: "Add gateway/middleware/auth.go: parse Authorization header if present, never reject; attach synthetic principal to request context for future job metadata"
    status: pending
  - id: discovery-doc
    content: "Implement GET /discovery/v1/apis/bigquery/v2/rest: return minimal static JSON listing implemented methods (subset of upstream discovery doc); start with methods we have routes for per docs/REST_API.md"
    status: pending
  - id: use-legacy-sql
    content: "In gateway/handlers/queries.go QueryRun: decode bqtypes.QueryRequest; if useLegacySql != nil && *useLegacySql == true, return HTTP 400 with reason invalidQuery before hitting engine"
    status: pending
  - id: use-legacy-sql-test
    content: "Add gateway/handlers/queries_test.go covering useLegacySql=true rejection and unset/false passthrough to NotImplemented"
    status: pending
  - id: emulator-host-doc
    content: "Verify README.md documents BIGQUERY_EMULATOR_HOST; add gateway note in docs/REST_API.md auth section if missing"
    status: pending
  - id: update-rest-api-status
    content: "Flip docs/REST_API.md status for discovery/auth from todo to done once implemented"
    status: pending
isProject: false
---

# Phase 1 (remainder): Gateway polish

## Prerequisites

- Route table complete ([gateway/server.go](../../gateway/server.go), [docs/REST_API.md](../../docs/REST_API.md)).
- [gateway/server_test.go](../../gateway/server_test.go) route smoke tests passing.

## Scope

**In:** Auth middleware, discovery endpoint, `useLegacySql` guard, tests.

**Out:** Real handler implementations (catalog CRUD returns real data — Phase 05). Result wire encoding (Phase 07).

## Key files

- `gateway/middleware/auth.go` (new)
- `gateway/server.go` — wrap mux with auth middleware
- `gateway/handlers/queries.go` — useLegacySql check
- `gateway/handlers/discovery.go` (new) or inline in handlers.go
- `gateway/handlers/queries_test.go` (new)
- `docs/REST_API.md`

## Reference docs

- [docs/bigquery/docs/authentication.md](../../docs/bigquery/docs/authentication.md) — emulator ignores ADC
- [docs/bigquery/docs/reference/rest/v2/jobs/query.md](../../docs/bigquery/docs/reference/rest/v2/jobs/query.md) — `useLegacySql` default true on wire

## Verification

```bash
go test ./gateway/...
./bin/gateway_main --engine_binary="" --http_port=9050 &
curl -sf http://localhost:9050/discovery/v1/apis/bigquery/v2/rest | jq .kind
curl -sf -X POST http://localhost:9050/bigquery/v2/projects/p/queries \
  -H 'Authorization: Bearer fake' \
  -H 'Content-Type: application/json' \
  -d '{"query":"SELECT 1","useLegacySql":true}' | jq .error.errors[0].reason
# expect "invalidQuery"
```

## Done criteria

- Discovery returns 200 with valid JSON (not 501).
- Bearer token in header does not cause 401.
- `useLegacySql=true` → 400 `invalidQuery`; `false`/unset → 501 (engine not ready yet).
- All plan todos completed.

## Next plan

[catalog-grpc-tabledata-e2e_1a2b3c4d.plan.md](catalog-grpc-tabledata-e2e_1a2b3c4d.plan.md) after grpc-contract + cpp-interfaces-memory land.
