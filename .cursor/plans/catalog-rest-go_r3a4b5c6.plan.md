---
name: catalog-rest-go
overview: "Phase 3h: Wire Go REST handlers for dataset and table CRUD via gRPC."
todos:
  - id: go-catalog-client
    content: "Wire datasets.go and tables.go: decode REST JSON, call CatalogClient, encode Dataset/Table resources"
    status: completed
  - id: update-rest-api-crud
    content: "Update docs/REST_API.md status for dataset/table CRUD from wired→done"
    status: completed
isProject: false
---

# Phase 3h: Catalog Rest Go

## Prerequisites

- [catalog-grpc-cpp_q2f3a4b5.plan.md](catalog-grpc-cpp_q2f3a4b5.plan.md)
- [gateway-legacy-sql_g2b3c4d5.plan.md](gateway-legacy-sql_g2b3c4d5.plan.md)

## Verification

```bash
curl -X POST localhost:9050/bigquery/v2/projects/test/datasets -d '{"datasetReference":{"datasetId":"ds1"}}'
```

## Done criteria

- Dataset + table CRUD return 200 not 501

## Next plan(s)

- [projects-stubs_s4b5c6d7.plan.md](projects-stubs_s4b5c6d7.plan.md)
