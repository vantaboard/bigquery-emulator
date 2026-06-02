---
name: X-HTTP-Method-Override middleware
overview: "Add a small early-stage middleware (next to gateway/middleware/loopback.go) that rewrites `r.Method` from POST to PATCH/PUT/DELETE when the `X-HTTP-Method-Override` request header is set to one of those values. Unblocks Java google-http-client flows across the entire gateway surface — concretely AuthorizeDatasetIT, but the fix is general."
todos:
  - id: tp05_capture
    content: "Confirm the Java client behavior with a packet capture: `docker compose logs bigquery-emulator | rg -A2 'POST /bigquery/v2/projects'` while the AuthorizeDatasetIT runs."
    status: pending
  - id: tp05_middleware
    content: "Add gateway/middleware/method_override.go with a single MethodOverride wrapper; reject method-override-from-non-POST and unknown override targets with 400."
    status: pending
  - id: tp05_wire
    content: "Wire the middleware into gateway/server.go wrapMiddleware (early, before route resolution)."
    status: pending
  - id: tp05_tests
    content: "Unit-test the middleware (override happy path, missing header is no-op, non-POST + header is 400, unsupported override target is 400)."
    status: pending
  - id: tp05_it
    content: "Re-run `task thirdparty:java-bigquery-tests -- -Dit.test=AuthorizeDatasetIT`; expect the previous POST-405 to disappear."
    status: pending
  - id: tp05_index
    content: "Flip thirdparty-00-completion-index.plan.md todo `tp05` to `completed`."
    status: pending
isProject: false
---

# Thirdparty 05 — honor `X-HTTP-Method-Override`

## Source

- `fix_thirdparty_failures_cbe91a41.plan.md` Tier 3.1 (now deleted).
- Failing log line `.logs/thirdparty-20260602-134739.log:3276`:
  `com.google.cloud.bigquery.BigQueryException: POST is not allowed on
  a dataset resource. Use POST /datasets to create, or a documented
  :op custom method (e.g. :undelete).`
  The 405 originates from
  [`gateway/handlers/datasets.go:322-326`](../../gateway/handlers/datasets.go).
- The Java sample
  `third_party/java-bigquery-tests/java-bigquery/samples/snippets/src/main/java/com/example/bigquery/AuthorizeDataset.java`
  calls `sourceDataset.toBuilder().setAcl(...).build().update()` which
  the Java client normally sends as `PATCH`. The gateway already
  mounts both PUT and PATCH at
  [`gateway/server.go:106-107`](../../gateway/server.go).

## Prerequisites

None.

## Scope

Add an early middleware (mounted before route resolution) that
inspects `X-HTTP-Method-Override`. When the request method is `POST`
and the header value is one of `PATCH`, `PUT`, or `DELETE`
(case-insensitive), rewrite `r.Method` to the override value and let
the existing mux handle dispatch.

Out of scope for this plan: changing dataset CustomMethodPOST itself.
The middleware is the right layer because it fixes the entire surface
in one place; rewriting `DatasetCustomMethodPOST` to also accept ACL
bodies (the narrower alternative) only fixes one route.

## Implementation

1. **Capture first.** Run AuthorizeDatasetIT with verbose access
   logging enabled and grep the gateway logs for the offending POST:

   ```bash
   docker compose logs bigquery-emulator \
     | rg -A2 'POST /bigquery/v2/projects'
   ```

   Confirm the inbound request is literal `POST` with the override
   header set. If it is not (e.g. the Java client truly sends a
   literal `POST` body), abandon this plan and re-scope as a
   `DatasetCustomMethodPOST` extension instead.

2. **Add the middleware.** Create
   `gateway/middleware/method_override.go` mirroring the existing
   loopback middleware's style. Pseudocode:

   ```go
   func MethodOverride(next http.Handler) http.Handler {
       return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
           override := r.Header.Get("X-HTTP-Method-Override")
           if override == "" {
               next.ServeHTTP(w, r)
               return
           }
           if r.Method != http.MethodPost {
               // Reject: override only meaningful on POST.
               writeError(w, http.StatusBadRequest, ...)
               return
           }
           upper := strings.ToUpper(override)
           switch upper {
           case http.MethodPatch, http.MethodPut, http.MethodDelete:
               r.Method = upper
           default:
               writeError(w, http.StatusBadRequest, ...)
               return
           }
           next.ServeHTTP(w, r)
       })
   }
   ```

3. **Wire it.** Add the middleware in
   [`gateway/server.go`](../../gateway/server.go) inside the
   `wrapMiddleware` chain, **before** the route dispatcher and
   **before** any middleware that reads `r.Method` for routing
   decisions. Place it after the access-log middleware so the log
   shows the original method.

4. **Tests.** Add table-driven tests under
   `gateway/middleware/method_override_test.go` covering:
   - Header absent — no-op, passes through.
   - Header set + POST + PATCH/PUT/DELETE — `r.Method` is rewritten.
   - Header set + GET/HEAD/OPTIONS — 400 with a structured error.
   - Header set to an unsupported value (e.g. `OPTIONS`) — 400.
   - Header set to a casing variant (`patch`, `Patch`) — accepted.

## Tests

- Unit tests above.
- `task thirdparty:java-bigquery-tests -- -Dit.test=AuthorizeDatasetIT`
  passes.
- No regression in the rest of the Java IT suite (run the full
  thirdparty job).

## Done criteria

- `gateway/middleware/method_override.go` lands with unit tests.
- The middleware is wired in `gateway/server.go`.
- `AuthorizeDatasetIT` passes.
- `thirdparty-00-completion-index.plan.md` todo `tp05` flipped to
  `completed`.
