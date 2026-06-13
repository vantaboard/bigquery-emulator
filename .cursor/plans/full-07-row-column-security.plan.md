---
name: Full 07 — Row-access & column-level security
overview: Move rowAccessPolicies and column-level security / data masking from structural stubs to real local enforcement - apply row-access filter predicates and column policy-tag masking to query results - so the emulator behaves like BigQuery for governance-aware queries instead of just accepting the CRUD calls.
est_effort: ~2 weeks
isProject: true
todos:
  - id: rap-crud
    content: "Audit the existing rowAccessPolicies REST routes + bqv2grpc adapters: confirm create/list/get/delete persist policy definitions (filter predicate + grantees) in the catalog. Promote from accept-and-forget to persisted definitions if needed."
    status: pending
  - id: rap-enforcement
    content: "Enforce row-access policies at query time: when a table has row-access policies, AND the policy filter predicates into the scan (BigQuery applies the UNION of predicates the caller's principal is granted). Given the emulator's single synthetic principal (emulator@bigquery.local), decide the enforcement model - default to `all policies grant access` unless a policy explicitly targets a different principal - and document it."
    status: pending
  - id: column-policy-tags
    content: "Column-level security: honor policy tags / OPTIONS on columns. Apply column masking (NULL / SHA256 / DEFAULT mask per the data-masking rules) or access denial to results for columns marked restricted; carry the metadata through ALTER TABLE SET OPTIONS / CREATE TABLE."
    status: pending
  - id: data-masking
    content: "Implement the documented masking routines (e.g. SHA256, default-value, nullify, hash) applied to masked columns in the result-marshaling path; reference docs/bigquery/docs/column-data-masking-intro.md."
    status: pending
  - id: errors-introspection
    content: "Surface governance metadata through INFORMATION_SCHEMA where full-01 added the views (e.g. column policy tags) and through getIamPolicy/test paths; map access-denied to BigQuery's accessDenied error shape."
    status: pending
  - id: fixtures-trackers
    content: "conformance/fixtures/security/ fixtures (row-access filtered select, masked column select, denied column); update ROADMAP REST surface bullet + ENGINE_POLICY; drop dbt grant_access / column_policy skip-matrix entries that now pass and re-run."
    status: pending
---

# Full 07 — Row-access & column-level security

## Why

The peripheral governance surfaces (rowAccessPolicies, column policy
tags / data masking) are registered as structurally-valid stubs so
client-library probes succeed (ROADMAP "wired stubs"), but they do not
*enforce* anything — a query over a row-access-policy-protected table
returns all rows, and a masked column returns clear values. Real
governance-aware tooling (and the dbt `grant_access` / `column_policy`
tests) needs enforcement.

## The emulator-principal question

The emulator attaches one synthetic principal
(`emulator@bigquery.local`, `gateway/middleware/auth.go`) and never
returns 401. Row/column security is principal-scoped in BigQuery. This
plan must pick an explicit, documented enforcement model rather than
faking multi-principal auth — recommended default: policies whose
grantee list is empty or includes the synthetic principal grant access;
policies targeting a *different* explicit principal filter/mask. This
keeps enforcement observable and testable without inventing an auth
system the emulator deliberately omits.

## Key files

- `gateway/handlers/` (rowAccessPolicies routes) + `gateway/handlers/bqv2grpc/`
- [`backend/engine/coordinator/route_classifier_visitor.cc`](../../backend/engine/coordinator/route_classifier_visitor.cc) / scan path — predicate injection
- result marshaling path — column masking
- [`backend/catalog/`](../../backend/catalog/) — policy persistence + column option metadata
- `docs/bigquery/docs/row-level-security-intro.md`, `column-level-security-intro.md`, `column-data-masking-intro.md`

## Steps

1. Persist row-access policy definitions (predicate + grantees).
2. Inject policy predicates into the scan at query time (documented
   principal model).
3. Carry column policy-tag metadata through DDL; mask/deny in results.
4. Implement the masking routines.
5. Introspection + error shapes.
6. Fixtures + doc updates + dbt skip-row removal.

## Verify

```bash
task emulator:build-engine:bazel
task conformance:run
task lint:dispositions
task thirdparty:dbt-bigquery-tests   # grant_access / column_policy
task bazel:shutdown && task bazel:status
```

## Out of scope

- Real IAM / multi-principal auth (the emulator's no-auth posture
  stands; this is policy *enforcement* against the synthetic principal).
- Taxonomy / Data Catalog policy-tag management API.
