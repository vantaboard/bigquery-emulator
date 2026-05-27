package gateway

import (
	"log"
	"net/http"
	"time"

	"github.com/vantaboard/bigquery-emulator/gateway/engine"
	"github.com/vantaboard/bigquery-emulator/gateway/handlers"
	"github.com/vantaboard/bigquery-emulator/gateway/jobs"
	"github.com/vantaboard/bigquery-emulator/gateway/middleware"
	"github.com/vantaboard/bigquery-emulator/gateway/seed"
)

// NewServer returns the HTTP handler tree implementing the BigQuery REST
// surface. Routes use Go 1.22+ method-aware patterns.
//
// Routes here mirror the public BigQuery v2 REST API. The canonical
// emulator-side mapping (with handler pointers and status) lives in
// docs/REST_API.md; the upstream documentation we cross-check against
// lives under docs/bigquery/docs/reference/rest/v2/.
//
// Every endpoint listed in docs/REST_API.md is registered here, even if
// the handler currently returns http.StatusNotImplemented. That gives
// client libraries a stable surface to probe and lets us flip handlers
// from stub to real one resource at a time, exactly the way Phase 1 of
// ROADMAP.md prescribes.
//
// Custom-method endpoints (the AIP-136 "{resource}:operation" shape used
// by datasets.undelete and the three tables IAM endpoints) cannot be
// expressed directly in net/http's mux pattern syntax, which requires
// every wildcard segment to end with `}`. For those, we register the
// parent path and dispatch on the trailing `:op` inside the handler.
func NewServer(opts Options, eng *engine.Client) http.Handler {
	mux := http.NewServeMux()
	// One Registry per gateway process: shared across the jobs.query,
	// jobs.get, and jobs.list handlers so a job minted by the sync
	// query API is discoverable by subsequent reads.
	deps := handlers.Dependencies{Jobs: jobs.NewRegistry()}
	if eng != nil {
		// Engine subprocess is wired up; surface the gRPC clients to
		// handlers. When eng is nil (Phase 1 / unit tests / `task
		// emulator:run` with --engine_binary=""), Dependencies stays
		// zero-valued and handlers fall back to their NotImplemented
		// stubs.
		deps.Catalog = eng.Catalog
		deps.Query = eng.Query
	}

	mux.HandleFunc("GET /{$}", handlers.Health)
	mux.HandleFunc("GET /healthz", handlers.Health)
	mux.HandleFunc("/", handlers.NotFound)

	mux.HandleFunc("GET /discovery/v1/apis/bigquery/v2/rest", handlers.Discovery(deps))

	mux.HandleFunc("GET /bigquery/v2/projects", handlers.ProjectList(deps))
	mux.HandleFunc("GET /bigquery/v2/projects/{projectId}/serviceAccount", handlers.ProjectGetServiceAccount(deps))

	mux.HandleFunc("GET /bigquery/v2/projects/{projectId}/datasets", handlers.DatasetList(deps))
	mux.HandleFunc("POST /bigquery/v2/projects/{projectId}/datasets", handlers.DatasetInsert(deps))
	mux.HandleFunc("GET /bigquery/v2/projects/{projectId}/datasets/{datasetId}", handlers.DatasetGet(deps))
	mux.HandleFunc("PUT /bigquery/v2/projects/{projectId}/datasets/{datasetId}", handlers.DatasetUpdate(deps))
	mux.HandleFunc("PATCH /bigquery/v2/projects/{projectId}/datasets/{datasetId}", handlers.DatasetPatch(deps))
	mux.HandleFunc("DELETE /bigquery/v2/projects/{projectId}/datasets/{datasetId}", handlers.DatasetDelete(deps))
	// datasets.undelete: POST /datasets/{datasetId}:undelete, dispatched
	// on the trailing :undelete in the wildcard.
	mux.HandleFunc(
		"POST /bigquery/v2/projects/{projectId}/datasets/{datasetId}",
		handlers.DatasetCustomMethodPOST(deps),
	)

	mux.HandleFunc("GET /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables", handlers.TableList(deps))
	mux.HandleFunc("POST /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables", handlers.TableInsert(deps))
	mux.HandleFunc(
		"GET /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}",
		handlers.TableGet(deps),
	)
	mux.HandleFunc(
		"PUT /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}",
		handlers.TableUpdate(deps),
	)
	mux.HandleFunc(
		"PATCH /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}",
		handlers.TablePatch(deps),
	)
	mux.HandleFunc(
		"DELETE /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}",
		handlers.TableDelete(deps),
	)
	// tables IAM custom methods: POST /tables/{tableId}:getIamPolicy,
	// :setIamPolicy, :testIamPermissions. Dispatched on trailing :op.
	mux.HandleFunc(
		"POST /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}",
		handlers.TableCustomMethodPOST(deps),
	)

	mux.HandleFunc(
		"GET /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}/data",
		handlers.TableDataList(deps),
	)
	mux.HandleFunc(
		"POST /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}/insertAll",
		handlers.TableDataInsertAll(deps),
	)

	// Models (BQML). Engine has no model store; list returns the
	// BigQuery-shaped empty page so client probes succeed, get/delete
	// return 404 so list-get-delete sample loops behave predictably.
	// See gateway/handlers/models.go.
	mux.HandleFunc(
		"GET /bigquery/v2/projects/{projectId}/datasets/{datasetId}/models",
		handlers.ModelList(deps),
	)
	mux.HandleFunc(
		"GET /bigquery/v2/projects/{projectId}/datasets/{datasetId}/models/{modelId}",
		handlers.ModelGet(deps),
	)
	mux.HandleFunc(
		"PATCH /bigquery/v2/projects/{projectId}/datasets/{datasetId}/models/{modelId}",
		handlers.ModelPatch(deps),
	)
	mux.HandleFunc(
		"DELETE /bigquery/v2/projects/{projectId}/datasets/{datasetId}/models/{modelId}",
		handlers.ModelDelete(deps),
	)

	// Routines (UDFs / TVFs / stored procedures). Same wired-stub
	// posture as models. See gateway/handlers/routines.go.
	mux.HandleFunc(
		"GET /bigquery/v2/projects/{projectId}/datasets/{datasetId}/routines",
		handlers.RoutineList(deps),
	)
	mux.HandleFunc(
		"POST /bigquery/v2/projects/{projectId}/datasets/{datasetId}/routines",
		handlers.RoutineInsert(deps),
	)
	mux.HandleFunc(
		"GET /bigquery/v2/projects/{projectId}/datasets/{datasetId}/routines/{routineId}",
		handlers.RoutineGet(deps),
	)
	mux.HandleFunc(
		"PUT /bigquery/v2/projects/{projectId}/datasets/{datasetId}/routines/{routineId}",
		handlers.RoutineUpdate(deps),
	)
	mux.HandleFunc(
		"DELETE /bigquery/v2/projects/{projectId}/datasets/{datasetId}/routines/{routineId}",
		handlers.RoutineDelete(deps),
	)

	// Row-access policies (table-scoped row-level security). No
	// policy store yet; list returns the empty page, IAM custom
	// methods return 501. See gateway/handlers/row_access_policies.go.
	mux.HandleFunc(
		"GET /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}/rowAccessPolicies",
		handlers.RowAccessPolicyList(deps),
	)

	mux.HandleFunc("GET /bigquery/v2/projects/{projectId}/jobs", handlers.JobList(deps))
	mux.HandleFunc("POST /bigquery/v2/projects/{projectId}/jobs", handlers.JobInsert(deps))
	// jobs.insert media-upload variant.
	mux.HandleFunc("POST /upload/bigquery/v2/projects/{projectId}/jobs", handlers.JobInsertUpload(deps))
	mux.HandleFunc("GET /bigquery/v2/projects/{projectId}/jobs/{jobId}", handlers.JobGet(deps))
	mux.HandleFunc("POST /bigquery/v2/projects/{projectId}/jobs/{jobId}/cancel", handlers.JobCancel(deps))
	// jobs.delete: literal "/delete" suffix is not a typo, see
	// docs/bigquery/docs/reference/rest/v2/jobs/delete.md.
	mux.HandleFunc("DELETE /bigquery/v2/projects/{projectId}/jobs/{jobId}/delete", handlers.JobDelete(deps))

	mux.HandleFunc("POST /bigquery/v2/projects/{projectId}/queries", handlers.QueryRun(deps))
	mux.HandleFunc("GET /bigquery/v2/projects/{projectId}/queries/{jobId}", handlers.QueryGetResults(deps))

	// BigQuery Migration v2alpha (alias-served at v2 too). The official
	// client libraries read BIGQUERY_MIGRATION_EMULATOR_HOST and fall
	// back to BIGQUERY_EMULATOR_HOST, so this gateway covers both
	// surfaces from the same listener. List returns the empty page so
	// startup probes succeed; create/start/get/delete return the
	// documented 404/501. See gateway/handlers/migration.go.
	for _, ver := range []string{"v2alpha", "v2"} {
		base := "/" + ver + "/projects/{projectId}/locations/{location}/workflows"
		mux.HandleFunc("GET "+base, handlers.MigrationWorkflowList(deps))
		mux.HandleFunc("POST "+base, handlers.MigrationWorkflowCreate(deps))
		mux.HandleFunc("GET "+base+"/{workflowId}", handlers.MigrationWorkflowGet(deps))
		mux.HandleFunc("DELETE "+base+"/{workflowId}", handlers.MigrationWorkflowDelete(deps))
		// AIP-136 custom methods (only :start today) — Go's mux can't
		// match `{workflowId}:start` directly, so dispatch in-handler.
		mux.HandleFunc("POST "+base+"/{workflowId}", handlers.MigrationWorkflowCustomMethodPOST(deps))
	}

	// BigQuery Data Transfer Service v1. Same shell posture as the
	// migration surface: empty list pages so client probes succeed,
	// 404 for specific resources, 501 for create.
	// See gateway/handlers/data_transfer.go.
	mux.HandleFunc(
		"GET /v1/projects/{projectId}/dataSources",
		handlers.DataTransferDataSourceList(deps),
	)
	mux.HandleFunc(
		"GET /v1/projects/{projectId}/dataSources/{dataSourceId}",
		handlers.DataTransferDataSourceGet(deps),
	)
	mux.HandleFunc(
		"GET /v1/projects/{projectId}/locations/{location}/dataSources",
		handlers.DataTransferDataSourceList(deps),
	)
	mux.HandleFunc(
		"GET /v1/projects/{projectId}/locations/{location}/dataSources/{dataSourceId}",
		handlers.DataTransferDataSourceGet(deps),
	)
	mux.HandleFunc(
		"GET /v1/projects/{projectId}/transferConfigs",
		handlers.DataTransferConfigList(deps),
	)
	mux.HandleFunc(
		"POST /v1/projects/{projectId}/transferConfigs",
		handlers.DataTransferConfigCreate(deps),
	)
	mux.HandleFunc(
		"GET /v1/projects/{projectId}/transferConfigs/{configId}",
		handlers.DataTransferConfigGet(deps),
	)
	mux.HandleFunc(
		"GET /v1/projects/{projectId}/locations/{location}/transferConfigs",
		handlers.DataTransferConfigList(deps),
	)
	mux.HandleFunc(
		"POST /v1/projects/{projectId}/locations/{location}/transferConfigs",
		handlers.DataTransferConfigCreate(deps),
	)
	mux.HandleFunc(
		"GET /v1/projects/{projectId}/locations/{location}/transferConfigs/{configId}",
		handlers.DataTransferConfigGet(deps),
	)

	// Seed API: registered only when explicitly enabled via
	// --enable-seed-api. The routes refuse non-loopback callers
	// by default; an operator who needs CI/CD reach must combine
	// `--seed-api-allow-remote` with `--seed-api-seed-token` for
	// the documented defense-in-depth posture. The Runner is left
	// nil here because the default build does not link
	// cloud.google.com/go/bigquery; building with
	// `-tags=seed_production_live` adds the production runner.
	// In Runner=nil mode the POST handler returns 501 with the
	// documented "use --seed-data-file" message so operators see
	// a meaningful error instead of a hung op.
	if opts.EnableSeedAPI {
		var runner seed.Runner
		if eng != nil {
			runner = newSeedRunner(opts, eng)
		}
		seed.RegisterRoutes(mux, seed.HandlerDeps{
			Access: seed.AccessConfig{
				AllowRemote: opts.SeedAPIAllowRemote,
				Token:       opts.SeedAPISeedToken,
			},
			Store:  seed.NewStore(),
			Runner: runner,
		})
	}

	// Auth middleware always runs: it parses (but never validates) the
	// Authorization header and attaches a synthetic principal to the
	// request context. Per docs/REST_API.md and ROADMAP.md Phase 1,
	// the emulator must never 401, so this is permissive by design.
	handler := middleware.WithAuth(mux)
	if opts.LogRequests {
		handler = loggingMiddleware(handler)
	}
	return handler
}

func loggingMiddleware(next http.Handler) http.Handler {
	return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		start := time.Now()
		rw := &statusRecorder{ResponseWriter: w, status: http.StatusOK}
		next.ServeHTTP(rw, r)
		log.Printf("%s %s -> %d (%s)",
			r.Method, r.URL.RequestURI(), rw.status, time.Since(start))
	})
}

type statusRecorder struct {
	http.ResponseWriter
	status int
}

func (s *statusRecorder) WriteHeader(code int) {
	s.status = code
	s.ResponseWriter.WriteHeader(code)
}
