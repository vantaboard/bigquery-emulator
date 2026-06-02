package gateway

import (
	"log/slog"
	"net/http"
	"time"

	"github.com/vantaboard/bigquery-emulator/gateway/engine"
	"github.com/vantaboard/bigquery-emulator/gateway/handlers"
	"github.com/vantaboard/bigquery-emulator/gateway/handlers/datatransfer"
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
// from stub to real one resource at a time, exactly the way the
// gateway-HTTP-surface section of ROADMAP.md prescribes.
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
	deps := handlers.Dependencies{
		Jobs:     jobs.NewRegistry(),
		Metadata: handlers.NewMetadataStore(),
	}
	if eng != nil {
		// Engine subprocess is wired up; surface the gRPC clients to
		// handlers. When eng is nil (gateway-only mode / unit tests /
		// `task emulator:run` with --engine_binary=""), Dependencies
		// stays zero-valued and handlers fall back to their
		// NotImplemented stubs.
		deps.Catalog = eng.Catalog
		deps.Query = eng.Query
	}

	mux.HandleFunc("GET /{$}", handlers.Health)
	mux.HandleFunc("GET /healthz", handlers.Health)
	mux.HandleFunc("/", handlers.NotFound)
	mux.HandleFunc("GET /discovery/v1/apis/bigquery/v2/rest", handlers.Discovery(deps))

	mountBigQueryV2(mux, deps)
	mountMigration(mux, deps)
	mountDataTransfer(mux)
	mountSeedAPI(mux, opts, eng)

	return wrapMiddleware(opts, mux)
}

// mountBigQueryV2 registers every BigQuery v2 endpoint under both the
// `/bigquery/v2/...` prefix (what gcloud, bq, and clients pointed at
// real `*.googleapis.com` use) AND the bare `/...` form. The bare
// form is required because the official client libraries treat
// BIGQUERY_EMULATOR_HOST as the verbatim baseUrl with no version
// segment — for example @google-cloud/bigquery v8's bigquery.js
// sets `baseUrl = EMULATOR_HOST || ${apiEndpoint}/bigquery/v2`,
// which means a client configured via BIGQUERY_EMULATOR_HOST issues
// `POST /projects/{p}/queries` (no `/bigquery/v2`). Mirroring both
// forms keeps the public REST surface working for both invocation
// styles without a StripPrefix middleware that would have to fork
// on the other top-level prefixes (`/discovery/...`, `/upload/...`,
// `/v2alpha/...`, `/v2/...`, `/v1/...`, `/healthz`).
func mountBigQueryV2(mux *http.ServeMux, deps handlers.Dependencies) {
	mountBQv2 := func(method, path string, h http.HandlerFunc) {
		mux.HandleFunc(method+" /bigquery/v2"+path, h)
		mux.HandleFunc(method+" "+path, h)
	}
	mountProjectsAndDatasets(mountBQv2, deps)
	mountTables(mountBQv2, deps)
	mountModelsAndRoutines(mountBQv2, deps)
	mountJobsAndQueries(mux, mountBQv2, deps)
}

// mountFunc is the per-method mounting helper used by the BigQuery
// v2 sub-mounters. It registers a handler under both `/bigquery/v2`
// and bare-prefix mux patterns (see mountBigQueryV2 doc-comment for
// why the bare form is required).
type mountFunc = func(method, path string, h http.HandlerFunc)

// mountProjectsAndDatasets registers projects.* and datasets.*
// (including datasets.undelete on the trailing `:undelete` segment).
func mountProjectsAndDatasets(mount mountFunc, deps handlers.Dependencies) {
	mount("GET", "/projects", handlers.ProjectList(deps))
	mount("GET", "/projects/{projectId}/serviceAccount", handlers.ProjectGetServiceAccount(deps))

	mount("GET", "/projects/{projectId}/datasets", handlers.DatasetList(deps))
	mount("POST", "/projects/{projectId}/datasets", handlers.DatasetInsert(deps))
	mount("GET", "/projects/{projectId}/datasets/{datasetId}", handlers.DatasetGet(deps))
	mount("PUT", "/projects/{projectId}/datasets/{datasetId}", handlers.DatasetUpdate(deps))
	mount("PATCH", "/projects/{projectId}/datasets/{datasetId}", handlers.DatasetPatch(deps))
	mount("DELETE", "/projects/{projectId}/datasets/{datasetId}", handlers.DatasetDelete(deps))
	// datasets.undelete: POST /datasets/{datasetId}:undelete, dispatched
	// on the trailing :undelete in the wildcard.
	mount("POST", "/projects/{projectId}/datasets/{datasetId}", handlers.DatasetCustomMethodPOST(deps))
}

// mountTables registers tables.*, tabledata.*, and the table-scoped
// rowAccessPolicies surface. The trailing `:getIamPolicy` /
// `:setIamPolicy` / `:testIamPermissions` custom methods are
// dispatched in-handler because Go's mux can't match them directly.
func mountTables(mount mountFunc, deps handlers.Dependencies) {
	mount("GET", "/projects/{projectId}/datasets/{datasetId}/tables", handlers.TableList(deps))
	mount("POST", "/projects/{projectId}/datasets/{datasetId}/tables", handlers.TableInsert(deps))
	mount("GET", "/projects/{projectId}/datasets/{datasetId}/tables/{tableId}", handlers.TableGet(deps))
	mount("PUT", "/projects/{projectId}/datasets/{datasetId}/tables/{tableId}", handlers.TableUpdate(deps))
	mount("PATCH", "/projects/{projectId}/datasets/{datasetId}/tables/{tableId}", handlers.TablePatch(deps))
	mount("DELETE", "/projects/{projectId}/datasets/{datasetId}/tables/{tableId}", handlers.TableDelete(deps))
	mount(
		"POST",
		"/projects/{projectId}/datasets/{datasetId}/tables/{tableId}",
		handlers.TableCustomMethodPOST(deps),
	)

	mount("GET", "/projects/{projectId}/datasets/{datasetId}/tables/{tableId}/data", handlers.TableDataList(deps))
	mount(
		"POST",
		"/projects/{projectId}/datasets/{datasetId}/tables/{tableId}/insertAll",
		handlers.TableDataInsertAll(deps),
	)

	// Row-access policies (table-scoped row-level security). No
	// policy store yet; list returns the empty page, IAM custom
	// methods return 501. See gateway/handlers/row_access_policies.go.
	mount(
		"GET",
		"/projects/{projectId}/datasets/{datasetId}/tables/{tableId}/rowAccessPolicies",
		handlers.RowAccessPolicyList(deps),
	)
}

// mountModelsAndRoutines registers the BQML and routines (UDF / TVF
// / stored procedure) endpoints. Engine has no model or routine
// store today; the handlers return wired stubs (empty list page,
// 404 on get/delete) so client probes behave predictably.
func mountModelsAndRoutines(mount mountFunc, deps handlers.Dependencies) {
	mount("GET", "/projects/{projectId}/datasets/{datasetId}/models", handlers.ModelList(deps))
	mount("GET", "/projects/{projectId}/datasets/{datasetId}/models/{modelId}", handlers.ModelGet(deps))
	mount("PATCH", "/projects/{projectId}/datasets/{datasetId}/models/{modelId}", handlers.ModelPatch(deps))
	mount("DELETE", "/projects/{projectId}/datasets/{datasetId}/models/{modelId}", handlers.ModelDelete(deps))

	mount("GET", "/projects/{projectId}/datasets/{datasetId}/routines", handlers.RoutineList(deps))
	mount("POST", "/projects/{projectId}/datasets/{datasetId}/routines", handlers.RoutineInsert(deps))
	mount("GET", "/projects/{projectId}/datasets/{datasetId}/routines/{routineId}", handlers.RoutineGet(deps))
	mount("PUT", "/projects/{projectId}/datasets/{datasetId}/routines/{routineId}", handlers.RoutineUpdate(deps))
	mount("DELETE", "/projects/{projectId}/datasets/{datasetId}/routines/{routineId}", handlers.RoutineDelete(deps))
}

// mountJobsAndQueries registers jobs.* (including the upload variant
// of jobs.insert) and the synchronous queries.* endpoints. The
// trailing `/delete` on jobs.delete is not a typo; see
// docs/bigquery/docs/reference/rest/v2/jobs/delete.md.
func mountJobsAndQueries(mux *http.ServeMux, mount mountFunc, deps handlers.Dependencies) {
	mount("GET", "/projects/{projectId}/jobs", handlers.JobList(deps))
	mount("POST", "/projects/{projectId}/jobs", handlers.JobInsert(deps))
	// jobs.insert media-upload variant. The upload prefix is fixed by
	// the public BigQuery API and the client libraries hardcode it, so
	// only the `/upload/bigquery/v2/...` form is registered here.
	mux.HandleFunc("POST /upload/bigquery/v2/projects/{projectId}/jobs", handlers.JobInsertUpload(deps))
	mount("GET", "/projects/{projectId}/jobs/{jobId}", handlers.JobGet(deps))
	mount("POST", "/projects/{projectId}/jobs/{jobId}/cancel", handlers.JobCancel(deps))
	mount("DELETE", "/projects/{projectId}/jobs/{jobId}/delete", handlers.JobDelete(deps))

	mount("POST", "/projects/{projectId}/queries", handlers.QueryRun(deps))
	mount("GET", "/projects/{projectId}/queries/{jobId}", handlers.QueryGetResults(deps))
}

// mountMigration registers the BigQuery Migration v2alpha surface
// (alias-served at v2 too). The official client libraries read
// BIGQUERY_MIGRATION_EMULATOR_HOST and fall back to
// BIGQUERY_EMULATOR_HOST, so this gateway covers both surfaces from
// the same listener. List returns the empty page so startup probes
// succeed; create/start/get/delete return the documented 404/501.
// See gateway/handlers/migration.go.
func mountMigration(mux *http.ServeMux, deps handlers.Dependencies) {
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
}

// mountDataTransfer registers the BigQuery Data Transfer Service v1
// surface. The shallow-emulator port of go-googlesql's
// `api/datatransfer/` package replaces the empty shell that lived in
// gateway/handlers/data_transfer.go: dataSources catalog
// (`scheduled_query`, `amazon_s3`), in-memory CRUD for
// transferConfigs + transferRuns, and the AIP-136 custom methods
// (`scheduleRuns`, `checkValidCreds`, `startManualRuns`). See
// `.cursor/plans/java-its-shallow-emulators_b8c9d0e1.plan.md`.
func mountDataTransfer(mux *http.ServeMux) {
	dts := datatransfer.NewHandler(nil)
	dts.Register(mux)
}

// mountSeedAPI registers the seed API surface only when explicitly
// enabled via --enable-seed-api. The routes refuse non-loopback
// callers by default; an operator who needs CI/CD reach must combine
// `--seed-api-allow-remote` with `--seed-api-seed-token` for the
// documented defense-in-depth posture. The Runner is left nil
// when eng is nil because the default build does not link
// cloud.google.com/go/bigquery; building with
// `-tags=seed_production_live` adds the production runner. In
// Runner=nil mode the POST handler returns 501 with the documented
// "use --seed-data-file" message so operators see a meaningful error
// instead of a hung op.
func mountSeedAPI(mux *http.ServeMux, opts Options, eng *engine.Client) {
	if !opts.EnableSeedAPI {
		return
	}
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

// wrapMiddleware applies the gateway's standing middleware stack
// (gunzip, auth, optional structured request log) on top of the
// raw mux. Returned handler is what the gateway listens on.
func wrapMiddleware(opts Options, mux http.Handler) http.Handler {
	// Gunzip middleware runs FIRST so handlers see the inflated JSON
	// body. The Java BigQuery client sets `Content-Encoding: gzip` on
	// every POST/PUT/PATCH by default; without this the gateway's
	// JSON decoders trip on the gzip framing magic byte (`\x1f`) and
	// emit `invalid character '\x1f' looking for beginning of value`.
	// See gateway/middleware/gunzip.go for the contract.
	handler := middleware.WithGunzipRequestBody(mux)
	// Auth middleware always runs: it parses (but never validates) the
	// Authorization header and attaches a synthetic principal to the
	// request context. Per docs/REST_API.md and the
	// gateway-HTTP-surface section of ROADMAP.md, the emulator must
	// never 401, so this is permissive by design.
	handler = middleware.WithAuth(handler)
	// Loopback tag middleware always runs: it records whether the
	// request arrived from a loopback caller so handlers can gate
	// emulator-internal debug fields on it. The single user today is
	// the synchronous query handler, which surfaces
	// `Job.statistics.query.emulatorRoute` (the C++ coordinator's
	// canonical route disposition string) only to loopback callers
	// per `.cursor/plans/conformance-routing-matrix.plan.md`.
	handler = middleware.WithLoopbackTag(handler)
	if opts.LogRequests {
		logger := opts.Logger
		if logger == nil {
			logger = slog.New(slog.DiscardHandler)
		}
		handler = loggingMiddleware(logger, handler)
	}
	return handler
}

// loggingMiddleware logs each completed HTTP request as a structured
// slog event. Routing the request line through key/value pairs (instead
// of `log.Printf("%s %s ...", ...)`) keeps the logger's typed-value
// path between gateway and handler, defangs gosec G706's
// log-injection finding (the attacker-controlled URI never lands in a
// format-string position), and lets operators ship the JSON output to
// structured backends.
func loggingMiddleware(logger *slog.Logger, next http.Handler) http.Handler {
	return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		start := time.Now()
		rw := &statusRecorder{ResponseWriter: w, status: http.StatusOK}
		next.ServeHTTP(rw, r)
		logger.InfoContext(r.Context(), "request",
			slog.String("method", r.Method),
			slog.String("uri", r.URL.RequestURI()),
			slog.Int("status", rw.status),
			slog.Duration("dur", time.Since(start)),
		)
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
