// Copyright 2026 BigQuery Emulator Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

package gateway

import (
	"log"
	"net/http"
	"time"

	"github.com/vantaboard/bigquery-emulator/gateway/handlers"
)

// NewServer returns the HTTP handler tree implementing the BigQuery REST
// surface. Routes use Go 1.22+ method-aware patterns.
//
// Routes here mirror the public BigQuery v2 REST API:
// https://cloud.google.com/bigquery/docs/reference/rest
//
// Every route is registered, even if the handler currently returns
// http.StatusNotImplemented. That gives client libraries a stable surface
// to probe and lets us flip handlers from stub to real one resource at a
// time, exactly the way Phase 1 of ROADMAP.md prescribes.
func NewServer(opts Options) http.Handler {
	mux := http.NewServeMux()
	deps := handlers.Dependencies{}

	mux.HandleFunc("GET /{$}", handlers.Health)
	mux.HandleFunc("GET /healthz", handlers.Health)
	mux.HandleFunc("/", handlers.NotFound)

	// Discovery
	mux.HandleFunc("GET /discovery/v1/apis/bigquery/v2/rest", handlers.NotImplemented)

	// Projects
	mux.HandleFunc("GET /bigquery/v2/projects", handlers.ProjectList(deps))
	mux.HandleFunc("GET /bigquery/v2/projects/{projectId}", handlers.ProjectGet(deps))

	// Datasets
	mux.HandleFunc("GET /bigquery/v2/projects/{projectId}/datasets", handlers.DatasetList(deps))
	mux.HandleFunc("POST /bigquery/v2/projects/{projectId}/datasets", handlers.DatasetInsert(deps))
	mux.HandleFunc("GET /bigquery/v2/projects/{projectId}/datasets/{datasetId}", handlers.DatasetGet(deps))
	mux.HandleFunc("PUT /bigquery/v2/projects/{projectId}/datasets/{datasetId}", handlers.DatasetUpdate(deps))
	mux.HandleFunc("PATCH /bigquery/v2/projects/{projectId}/datasets/{datasetId}", handlers.DatasetPatch(deps))
	mux.HandleFunc("DELETE /bigquery/v2/projects/{projectId}/datasets/{datasetId}", handlers.DatasetDelete(deps))

	// Tables
	mux.HandleFunc("GET /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables", handlers.TableList(deps))
	mux.HandleFunc("POST /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables", handlers.TableInsert(deps))
	mux.HandleFunc("GET /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}", handlers.TableGet(deps))
	mux.HandleFunc("PUT /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}", handlers.TableUpdate(deps))
	mux.HandleFunc("PATCH /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}", handlers.TablePatch(deps))
	mux.HandleFunc("DELETE /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}", handlers.TableDelete(deps))

	// Tabledata
	mux.HandleFunc("GET /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}/data", handlers.TableDataList(deps))
	mux.HandleFunc("POST /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}/insertAll", handlers.TableDataInsertAll(deps))

	// Jobs
	mux.HandleFunc("GET /bigquery/v2/projects/{projectId}/jobs", handlers.JobList(deps))
	mux.HandleFunc("POST /bigquery/v2/projects/{projectId}/jobs", handlers.JobInsert(deps))
	mux.HandleFunc("GET /bigquery/v2/projects/{projectId}/jobs/{jobId}", handlers.JobGet(deps))
	mux.HandleFunc("POST /bigquery/v2/projects/{projectId}/jobs/{jobId}/cancel", handlers.JobCancel(deps))

	// Queries (synchronous query API + getQueryResults)
	mux.HandleFunc("POST /bigquery/v2/projects/{projectId}/queries", handlers.QueryRun(deps))
	mux.HandleFunc("GET /bigquery/v2/projects/{projectId}/queries/{jobId}", handlers.QueryGetResults(deps))

	if opts.LogRequests {
		return loggingMiddleware(mux)
	}
	return mux
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
