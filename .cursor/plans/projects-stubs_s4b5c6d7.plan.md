---
name: projects-stubs
overview: "Phase 3i: projects.list and getServiceAccount stubs."
todos:
  - id: projects-list-stub
    content: "Implement projects.list: single synthetic project from path or BIGQUERY_EMULATOR_PROJECT default"
    status: completed
  - id: service-account-stub
    content: "Implement getServiceAccount: fixed email bigquery-emulator@test-project.iam.gserviceaccount.com"
    status: completed
isProject: false
---

# Phase 3i: Projects Stubs

## Prerequisites

- [catalog-rest-go_r3a4b5c6.plan.md](catalog-rest-go_r3a4b5c6.plan.md)

## Verification

```bash
curl localhost:9050/bigquery/v2/projects/test/projects
```

## Done criteria

- projects.list and getServiceAccount return valid JSON

## Next plan(s)

- [tabledata-e2e_t5c6d7e8.plan.md](tabledata-e2e_t5c6d7e8.plan.md)
