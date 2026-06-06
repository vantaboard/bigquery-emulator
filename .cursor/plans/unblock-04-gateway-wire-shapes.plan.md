---
name: Unblock 04 — Gateway wire shapes
overview: Fix JSON unmarshaling and response-shape mismatches causing ~5 node failures (expirationTime, writeDisposition, labels delete, dataset region).
depends_on: [unblock-03-bigframes-gate]
blocks: [unblock-09-test-isolation]
baseline_log: .logs/thirdparty-20260605-134407.log
est_effort: 1-2 days
isProject: true
todos:
  - id: expiration-time-flex
    content: Custom UnmarshalJSON on expirationTime fields — accept int64 or string in gateway/bqtypes/types.go
    status: pending
  - id: write-disposition-array
    content: Normalize writeDisposition when client sends single-element JSON array
    status: pending
  - id: labels-delete-shape
    content: Return updated dataset/table with labels map (empty {} not omitted) after label delete
    status: pending
  - id: dataset-region-order
    content: Validate dataset location before id collision — error message matches node sample
    status: pending
  - id: handler-tests
    content: go test ./gateway/handlers/... -run 'Label|Expiration|Disposition|Dataset'
    status: pending
  - id: status-commit
    content: Update orchestration-status.md row; lint + commit
    status: pending
---

# Unblock 04 — Gateway wire shapes

## Goal

Clear **~5 node failures** from JSON request unmarshaling and response shape mismatches. Gateway-only — no engine/bazel required.

## Log signatures

```
cannot unmarshal number into Go struct field ...expirationTime of type string
cannot unmarshal array into ...writeDisposition of type string
expected 'nodejs_samples_tests_…' to include 'undefined'   # labels delete
expected 'Already Exists: Dataset…' to include 'Invalid storage region'
```

## Implementation

| Issue | Files | Fix |
|-------|-------|-----|
| `expirationTime` int vs string | [`gateway/bqtypes/types.go`](../../gateway/bqtypes/types.go) | `UnmarshalJSON` accepting `json.Number`, int64, or string → canonical string |
| `writeDisposition` array | Job query config in `types.go` / jobs registry | If JSON array of one string, unwrap to string |
| Labels delete response | [`gateway/handlers/tables.go`](../../gateway/handlers/tables.go), datasets handler | Return full resource; `labels: {}` when empty (see types.go comment L50–54) |
| Dataset region validation | Dataset create in handlers | Check invalid `location` **before** duplicate-id check |

## Investigation

Grep node log section in [`.logs/thirdparty-20260605-134407.log`](../../.logs/thirdparty-20260605-134407.log) for exact test names:

- Tables › expiration / range partition tests (#16, #19)
- Queries › writeDisposition (#13)
- Datasets › labels (#4)
- Datasets › region (#5)

Match upstream sample request bodies under `third_party/node-bigquery-tests/`.

## Fast gate

```bash
go test ./gateway/bqtypes/... ./gateway/handlers/... -count=1 -run 'Label|Expiration|Disposition|Dataset|Unmarshal'
```

## Slow gate (optional)

Re-run node Datasets + affected Tables tests only if a filtered mocha path exists; otherwise defer full node retest to plan 09/10.

## Out of scope

- Query parameter binding (plan 05)
- Load jobs (plan 06)
- DLP taxonomy full implementation — if CLS test needs stub, return honest partial with clear message (document DEFER)

## Done criteria

- [ ] Gateway unit tests cover each wire fix
- [ ] Node wire-shape failures from baseline no longer reproduce on spot-check
- [ ] `task lint:run` clean on touched Go paths
