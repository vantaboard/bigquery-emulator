---
name: gateway-auth-discovery
overview: "Phase 1a: auth middleware and discovery document so client libraries connect cleanly."
todos:
  - id: auth-middleware
    content: "Add gateway/middleware/auth.go: parse Authorization if present, never reject; attach synthetic principal to context"
    status: pending
  - id: discovery-doc
    content: "Implement GET /discovery/v1/apis/bigquery/v2/rest: minimal static JSON listing routed methods per docs/REST_API.md"
    status: pending
isProject: false
---

# Phase 1a: Gateway Auth Discovery

## Verification

```bash
curl -sf localhost:9050/discovery/v1/apis/bigquery/v2/rest | jq .kind
```

## Done criteria

- Discovery returns 200
- Bearer token does not cause 401

## Next plan(s)

- [gateway-legacy-sql_g2b3c4d5.plan.md](gateway-legacy-sql_g2b3c4d5.plan.md)
