---
name: catalog-grpc-tabledata-e2e
overview: "Phase 3 part 3: implement Catalog gRPC handlers against Storage, wire Go REST handlers for dataset/table CRUD and tabledata.insertAll/list end-to-end through gRPC."
todos:
  - id: catalog-handlers
    content: "Implement frontend/handlers/catalog.cc: RegisterDataset, DropDataset, RegisterTable, DropTable, DescribeTable delegating to Storage; map errors to grpc Status codes"
    status: pending
  - id: go-catalog-client
    content: "Wire gateway/handlers/datasets.go and tables.go: decode REST JSON, call enginepb CatalogClient, encode BigQuery-shaped responses (Dataset, Table resources)"
    status: pending
  - id: projects-list-stub
    content: "Implement projects.list returning single synthetic project from path projectId or env BIGQUERY_EMULATOR_PROJECT default 'test-project'"
    status: pending
  - id: service-account-stub
    content: "Implement projects.getServiceAccount returning fixed email bigquery-emulator@test-project.iam.gserviceaccount.com"
    status: pending
  - id: insert-all-handler
    content: "Implement tabledata.insertAll: decode insertAll request body, convert rows[] to proto, Catalog AppendRows via new rpc or direct storage call through engine"
    status: pending
  - id: tabledata-list-handler
    content: "Implement tabledata.list: paginated GET with startIndex/maxResults/pageToken; return TableDataList with f/v rows (wire encoding can be simplified strings initially)"
    status: pending
  - id: e2e-test-go
    content: "Add gateway/e2e/catalog_test.go (build tag integration): httptest against real gateway+engine subprocess; create dataset, create table with schema, insertAll 2 rows, list returns 2 rows"
    status: pending
  - id: update-rest-api
    content: "Update docs/REST_API.md status column from wired→done for implemented catalog/tabledata methods"
    status: pending
isProject: false
---

# Phase 3 (part 3): Catalog gRPC + tabledata E2E

## Prerequisites

- [grpc-contract-go-cpp_8a9b0c1d.plan.md](grpc-contract-go-cpp_8a9b0c1d.plan.md)
- [cpp-interfaces-memory-storage_2e3f4a5b.plan.md](cpp-interfaces-memory-storage_2e3f4a5b.plan.md)
- [gateway-polish_c4d5e6f7.plan.md](gateway-polish_c4d5e6f7.plan.md) recommended

## Scope

**In:** Full catalog CRUD over REST↔gRPC↔Storage; insertAll + list E2E on memory storage.

**Out:** Query execution. IAM policy stubs remain 501. Jobs remain 501.

## Key files

- `frontend/handlers/catalog.{h,cc}`
- `gateway/handlers/datasets.go`, `tables.go`, `projects.go`
- `gateway/engine/client.go`
- `gateway/e2e/catalog_test.go` (new)
- `docs/REST_API.md`

## Reference docs

- [docs/bigquery/docs/reference/rest/v2/datasets/](../../docs/bigquery/docs/reference/rest/v2/datasets/)
- [docs/bigquery/docs/reference/rest/v2/tabledata/insertAll.md](../../docs/bigquery/docs/reference/rest/v2/tabledata/insertAll.md)
- [docs/bigquery/docs/reference/rest/v2/tabledata/list.md](../../docs/bigquery/docs/reference/rest/v2/tabledata/list.md)

## Verification

```bash
go test -tags=integration ./gateway/e2e/...   # or manual curl script
curl -X POST localhost:9050/bigquery/v2/projects/test/datasets \
  -d '{"datasetReference":{"datasetId":"ds1"}}'
curl -X POST localhost:9050/bigquery/v2/projects/test/datasets/ds1/tables \
  -d '{"tableReference":{"tableId":"t1"},"schema":{"fields":[{"name":"x","type":"INT64"}]}}'
curl -X POST localhost:9050/bigquery/v2/projects/test/datasets/ds1/tables/t1/insertAll \
  -d '{"rows":[{"json":{"x":"1"}},{"json":{"x":"2"}}]}'
curl localhost:9050/bigquery/v2/projects/test/datasets/ds1/tables/t1/data
```

## Done criteria

- Dataset + table CRUD returns 200 with valid BigQuery JSON shapes (not 501).
- insertAll + list round-trip 2 rows on `--storage=memory`.
- Integration test passes in CI (may skip if no engine binary — use build tag).

## Next plan

[query-analysis-dryrun_5e6f7a8b.plan.md](query-analysis-dryrun_5e6f7a8b.plan.md)
