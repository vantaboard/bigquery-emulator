package gateway

import (
	"log"
	"net/http"
	"time"

	"github.com/vantaboard/bigquery-emulator/gateway/engine"
	"github.com/vantaboard/bigquery-emulator/gateway/handlers"
	"github.com/vantaboard/bigquery-emulator/gateway/middleware"
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
	deps := handlers.Dependencies{}
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
	mux.HandleFunc("POST /bigquery/v2/projects/{projectId}/datasets/{datasetId}", handlers.DatasetCustomMethodPOST(deps))

	mux.HandleFunc("GET /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables", handlers.TableList(deps))
	mux.HandleFunc("POST /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables", handlers.TableInsert(deps))
	mux.HandleFunc("GET /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}", handlers.TableGet(deps))
	mux.HandleFunc("PUT /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}", handlers.TableUpdate(deps))
	mux.HandleFunc("PATCH /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}", handlers.TablePatch(deps))
	mux.HandleFunc("DELETE /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}", handlers.TableDelete(deps))
	// tables IAM custom methods: POST /tables/{tableId}:getIamPolicy,
	// :setIamPolicy, :testIamPermissions. Dispatched on trailing :op.
	mux.HandleFunc("POST /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}", handlers.TableCustomMethodPOST(deps))

	mux.HandleFunc("GET /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}/data", handlers.TableDataList(deps))
	mux.HandleFunc("POST /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}/insertAll", handlers.TableDataInsertAll(deps))

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

	// Auth middleware always runs: it parses (but never validates) the
	// Authorization header and attaches a synthetic principal to the
	// request context. Per docs/REST_API.md and ROADMAP.md Phase 1,
	// the emulator must never 401, so this is permissive by design.
	var handler http.Handler = middleware.WithAuth(mux)
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
