package server

import (
	"compress/gzip"
	"context"
	"fmt"
	"net/http"
	"runtime"
	"strings"
	"sync"
	"time"

	connection2 "github.com/vantaboard/bigquery-emulator/internal/connection"
	"github.com/vantaboard/bigquery-emulator/internal/explorerapi"
	"github.com/vantaboard/bigquery-emulator/internal/metadata"

	"github.com/gorilla/mux"
	bigqueryv2 "google.golang.org/api/bigquery/v2"
	"log/slog"

	"github.com/vantaboard/bigquery-emulator/internal/logger"
)

func recoveryMiddleware(s *Server) func(http.Handler) http.Handler {
	return func(next http.Handler) http.Handler {
		return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
			defer func() {
				if err := recover(); err != nil {
					ctx := logger.WithLogger(r.Context(), s.logger)
					ctx = logger.WithHTTPRequest(ctx, r)
					errorResponse(ctx, w, errInternalError(fmt.Sprintf("%+v", err)))
					var frame int = 1
					for {
						_, file, line, ok := runtime.Caller(frame)
						if !ok {
							break
						}
						s.logger.Error("panic stack frame",
							slog.Int("frame", frame),
							slog.String("file", file),
							slog.Int("line", line),
						)
						frame++
					}
					return
				}
			}()
			next.ServeHTTP(w, r)
		})
	}
}

func loggerMiddleware(s *Server) func(http.Handler) http.Handler {
	return func(next http.Handler) http.Handler {
		return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
			ctx := r.Context()
			ctx = logger.WithLogger(ctx, s.logger)
			ctx = logger.WithHTTPRequest(ctx, r)
			next.ServeHTTP(w, r.WithContext(ctx))
		})
	}
}

// accessLogExtrasKey carries per-request fields from handlers back to [accessLogMiddleware]
// without relying on the outer *http.Request context (middleware stacks replace r.Context).
type accessLogExtrasKey struct{}

// accessLogExtras is mutated by handlers (e.g. jobs.getQueryResults) for richer completion logs.
type accessLogExtras struct {
	mu sync.Mutex
	// jobs.getQueryResults response (optional)
	jobComplete *bool
	totalRows   *uint64
	// Populated in withJobMiddleware for job-scoped routes (query preview for poll correlation).
	jobSQLPreview  *string
	jobDestination *string
}

func (e *accessLogExtras) recordGetQueryResults(jobComplete bool, totalRows uint64) {
	if e == nil {
		return
	}
	e.mu.Lock()
	defer e.mu.Unlock()
	jc := jobComplete
	e.jobComplete = &jc
	tr := totalRows
	e.totalRows = &tr
}

func jobAccessLogQueryFields(content *bigqueryv2.Job) (preview string, destination string) {
	if content == nil || content.Configuration == nil || content.Configuration.Query == nil {
		return "", ""
	}
	q := content.Configuration.Query.Query
	if q != "" {
		preview = q
	}
	if dt := content.Configuration.Query.DestinationTable; dt != nil {
		destination = fmt.Sprintf("%s.%s.%s", dt.ProjectId, dt.DatasetId, dt.TableId)
	}
	return preview, destination
}

func recordJobQueryIntoAccessLogExtras(ctx context.Context, job *metadata.Job) {
	if job == nil {
		return
	}
	v := ctx.Value(accessLogExtrasKey{})
	ex, ok := v.(*accessLogExtras)
	if !ok || ex == nil {
		return
	}
	preview, dest := jobAccessLogQueryFields(job.Content())
	if preview == "" && dest == "" {
		return
	}
	ex.mu.Lock()
	defer ex.mu.Unlock()
	if preview != "" {
		p := preview
		ex.jobSQLPreview = &p
	}
	if dest != "" {
		d := dest
		ex.jobDestination = &d
	}
}

// accessLogStartLevel demotes high-frequency jobs.getQueryResults request starts to DEBUG (SQL preview
// appears on the matching "completed" line after withJobMiddleware runs).
func accessLogStartLevel(method, rawPath string) slog.Level {
	path := strings.TrimPrefix(rawPath, "/bigquery/v2")
	path = strings.Trim(path, "/")
	segs := strings.Split(path, "/")
	if len(segs) >= 4 && segs[0] == "projects" && segs[2] == "queries" && method == http.MethodGet {
		return slog.LevelDebug
	}
	return slog.LevelInfo
}

// bigQueryRESTAccessAttrs labels common BigQuery REST paths so access logs state the RPC, not only the URL.
func bigQueryRESTAccessAttrs(method, rawPath string) []slog.Attr {
	path := strings.TrimPrefix(rawPath, "/bigquery/v2")
	path = strings.Trim(path, "/")
	if path == "" {
		return nil
	}
	segs := strings.Split(path, "/")
	if len(segs) < 2 || segs[0] != "projects" {
		return nil
	}
	projectID := segs[1]
	attrs := []slog.Attr{slog.String("project_id", projectID)}
	switch {
	case len(segs) >= 4 && segs[2] == "queries" && method == http.MethodGet:
		attrs = append(attrs,
			slog.String("bq_operation", "jobs.getQueryResults"),
			slog.String("job_id", segs[3]),
			slog.String("bq_op_hint", "poll async query job; JSON includes jobComplete, totalRows, rows"),
		)
	case len(segs) >= 4 && segs[2] == "jobs" && method == http.MethodGet:
		attrs = append(attrs,
			slog.String("bq_operation", "jobs.get"),
			slog.String("job_id", segs[3]),
			slog.String("bq_op_hint", "fetch job metadata; JSON includes status.state"),
		)
	case len(segs) == 3 && segs[2] == "jobs" && method == http.MethodPost:
		attrs = append(attrs,
			slog.String("bq_operation", "jobs.insert"),
			slog.String("bq_op_hint", "submit load/extract/query job"),
		)
	case len(segs) == 3 && segs[2] == "datasets" && method == http.MethodGet:
		attrs = append(attrs,
			slog.String("bq_operation", "datasets.list"),
			slog.String("bq_op_hint", "list datasets in project"),
		)
	case len(segs) >= 4 && segs[2] == "datasets" && method == http.MethodGet && len(segs) == 4:
		attrs = append(attrs,
			slog.String("bq_operation", "datasets.get"),
			slog.String("dataset_id", segs[3]),
		)
	case len(segs) >= 6 && segs[2] == "datasets" && segs[4] == "tables" && method == http.MethodGet:
		attrs = append(attrs,
			slog.String("bq_operation", "tables.get"),
			slog.String("dataset_id", segs[3]),
			slog.String("table_id", segs[5]),
		)
	}
	return attrs
}

// accessLogCompletedLevel demotes noisy jobs.getQueryResults poll completions while jobComplete is false.
func accessLogCompletedLevel(method, rawPath string, extras *accessLogExtras) slog.Level {
	if extras == nil || extras.jobComplete == nil || *extras.jobComplete {
		return slog.LevelInfo
	}
	path := strings.TrimPrefix(rawPath, "/bigquery/v2")
	path = strings.Trim(path, "/")
	segs := strings.Split(path, "/")
	if len(segs) >= 4 && segs[0] == "projects" && segs[2] == "queries" && method == http.MethodGet {
		return slog.LevelDebug
	}
	return slog.LevelInfo
}

func accessLogMiddleware() func(http.Handler) http.Handler {
	return func(next http.Handler) http.Handler {
		return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
			extras := &accessLogExtras{}
			r = r.WithContext(context.WithValue(r.Context(), accessLogExtrasKey{}, extras))

			startAttrs := append(
				[]slog.Attr{slog.String("query", r.URL.RawQuery)},
				bigQueryRESTAccessAttrs(r.Method, r.URL.Path)...,
			)
			logger.Logger(r.Context()).LogAttrs(
				r.Context(),
				accessLogStartLevel(r.Method, r.URL.Path),
				fmt.Sprintf("%s %s", r.Method, r.URL.Path),
				startAttrs...,
			)
			start := time.Now()
			next.ServeHTTP(w, r)

			doneAttrs := append(
				[]slog.Attr{
					slog.String("query", r.URL.RawQuery),
					slog.Duration("duration", time.Since(start)),
				},
				bigQueryRESTAccessAttrs(r.Method, r.URL.Path)...,
			)
			extras.mu.Lock()
			if extras.jobComplete != nil {
				doneAttrs = append(doneAttrs, slog.Bool("job_complete", *extras.jobComplete))
			}
			if extras.totalRows != nil {
				doneAttrs = append(doneAttrs, slog.Uint64("total_rows", *extras.totalRows))
			}
			if extras.jobSQLPreview != nil {
				p := truncateStringForAccessLog(*extras.jobSQLPreview, accessLogSQLPreviewMax())
				doneAttrs = append(doneAttrs, slog.String("job_sql_preview", p))
			}
			if extras.jobDestination != nil {
				doneAttrs = append(doneAttrs, slog.String("job_destination", *extras.jobDestination))
			}
			extras.mu.Unlock()

			logger.Logger(r.Context()).LogAttrs(
				r.Context(),
				accessLogCompletedLevel(r.Method, r.URL.Path, extras),
				fmt.Sprintf("%s %s completed", r.Method, r.URL.Path),
				doneAttrs...,
			)
		})
	}
}

const (
	contentEncoding  = "Content-Encoding"
	encodingTypeGzip = "gzip"
)

func decompressMiddleware() func(http.Handler) http.Handler {
	return func(next http.Handler) http.Handler {
		return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
			if r.Header.Get(contentEncoding) != encodingTypeGzip {
				next.ServeHTTP(w, r)
				return
			}
			ctx := r.Context()
			reader, err := gzip.NewReader(r.Body)
			if err != nil {
				errorResponse(ctx, w, errInvalid(fmt.Sprintf("failed to decode gzip content: %s", err)))
				return
			}
			defer reader.Close()
			r.Body = reader
			next.ServeHTTP(w, r)
		})
	}
}

func withServerMiddleware(s *Server) func(http.Handler) http.Handler {
	return func(next http.Handler) http.Handler {
		return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
			next.ServeHTTP(
				w,
				r.WithContext(withServer(r.Context(), s)),
			)
		})
	}
}

func projectIDFromParams(params map[string]string) (string, bool) {
	projectID, exists := params["projectId"]
	if exists {
		return projectID, true
	}
	projectID, exists = params["projectsId"]
	return projectID, exists
}

func datasetIDFromParams(params map[string]string) (string, bool) {
	datasetID, exists := params["datasetId"]
	if exists {
		return datasetID, true
	}
	datasetID, exists = params["datasetsId"]
	return datasetID, exists
}

func jobIDFromParams(params map[string]string) (string, bool) {
	jobID, exists := params["jobId"]
	if exists {
		return jobID, true
	}
	jobID, exists = params["jobsId"]
	return jobID, exists
}

func tableIDFromParams(params map[string]string) (string, bool) {
	tableID, exists := params["tableId"]
	if exists {
		return tableID, true
	}
	tableID, exists = params["tablesId"]
	return tableID, exists
}

func modelIDFromParams(params map[string]string) (string, bool) {
	modelID, exists := params["modelId"]
	if exists {
		return modelID, true
	}
	modelID, exists = params["modelsId"]
	return modelID, exists
}

func routineIDFromParams(params map[string]string) (string, bool) {
	routineID, exists := params["routineId"]
	if exists {
		return routineID, true
	}
	routineID, exists = params["routinesId"]
	return routineID, exists
}

func withConnectionMiddleware() func(http.Handler) http.Handler {
	return func(next http.Handler) http.Handler {
		return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
			// Explorer /api routes use the cloud.google.com/go/bigquery client over HTTP to this server,
			// not the managed sqlite connection used by BigQuery REST handlers.
			if strings.HasPrefix(r.URL.Path, explorerapi.APIPrefix) {
				next.ServeHTTP(w, r)
				return
			}

			ctx := r.Context()

			server := serverFromContext(ctx)
			err := server.connMgr.WithManagedConnection(ctx, func(ctx context.Context, conn *connection2.ManagedConnection) error {
				ctx = withConnection(ctx, conn)

				next.ServeHTTP(
					w,
					r.WithContext(ctx),
				)

				return nil
			})
			if err != nil {
				w.WriteHeader(http.StatusInternalServerError)
				fmt.Fprintln(w, err)
				return
			}
		})
	}
}

func withProjectMiddleware() func(http.Handler) http.Handler {
	return func(next http.Handler) http.Handler {
		return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
			ctx := r.Context()
			params := mux.Vars(r)
			projectID, exists := projectIDFromParams(params)
			if exists {
				srv := serverFromContext(ctx)
				jobID, jobInPath := jobIDFromParams(params)
				if jobInPath {
					project, job, err := srv.findProjectAndJobUsingRequestConnection(ctx, projectID, jobID)
					if err != nil {
						w.WriteHeader(http.StatusInternalServerError)
						fmt.Fprintln(w, err)
						return
					}
					if project == nil {
						errorResponse(ctx, w, errNotFound(fmt.Sprintf("project %s is not found", projectID)))
						return
					}
					ctx = withProject(ctx, project)
					if job == nil {
						errorResponse(ctx, w, errNotFound(fmt.Sprintf("job %s is not found", jobID)))
						return
					}
					ctx = withJob(ctx, job)
				} else {
					project, err := srv.findProjectUsingRequestConnection(ctx, projectID)
					if err != nil {
						w.WriteHeader(http.StatusInternalServerError)
						fmt.Fprintln(w, err)
						return
					}
					if project == nil {
						errorResponse(ctx, w, errNotFound(fmt.Sprintf("project %s is not found", projectID)))
						return
					}
					ctx = withProject(ctx, project)
				}
			}
			next.ServeHTTP(
				w,
				r.WithContext(ctx),
			)
		})
	}
}

func withDatasetMiddleware() func(http.Handler) http.Handler {
	return func(next http.Handler) http.Handler {
		return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
			ctx := r.Context()
			params := mux.Vars(r)
			datasetID, exists := datasetIDFromParams(params)
			if exists {
				project := projectFromContext(ctx)
				dataset, err := project.Dataset(ctx, datasetID)
				if err != nil {
					errorResponse(ctx, w, errNotFound(fmt.Sprintf("%s", err)))
					return
				}
				if dataset == nil {
					errorResponse(ctx, w, errNotFound(fmt.Sprintf("dataset %s is not found", datasetID)))
					return
				}
				ctx = withDataset(ctx, dataset)
			}
			next.ServeHTTP(
				w,
				r.WithContext(ctx),
			)
		})
	}
}

func withJobMiddleware() func(http.Handler) http.Handler {
	return func(next http.Handler) http.Handler {
		return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
			ctx := r.Context()
			params := mux.Vars(r)
			jobID, exists := jobIDFromParams(params)
			if exists {
				// withProjectMiddleware may have already loaded the job in one tx with the project.
				if v := ctx.Value(jobKey{}); v != nil {
					if j, ok := v.(*metadata.Job); ok && j != nil {
						recordJobQueryIntoAccessLogExtras(ctx, j)
						next.ServeHTTP(w, r.WithContext(ctx))
						return
					}
				}
				project := projectFromContext(ctx)
				srv := serverFromContext(ctx)
				job, err := srv.findJobUsingRequestConnection(ctx, project.ID, jobID)
				if err != nil {
					errorResponse(ctx, w, errInternalError(fmt.Sprintf("error finding job %s: %s", jobID, err)))
					return
				}
				if job == nil {
					errorResponse(ctx, w, errNotFound(fmt.Sprintf("job %s is not found", jobID)))
					return
				}
				ctx = withJob(ctx, job)
				recordJobQueryIntoAccessLogExtras(ctx, job)
			}
			next.ServeHTTP(
				w,
				r.WithContext(ctx),
			)
		})
	}
}

func withTableMiddleware() func(http.Handler) http.Handler {
	return func(next http.Handler) http.Handler {
		return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
			ctx := r.Context()
			params := mux.Vars(r)
			tableID, exists := tableIDFromParams(params)
			if exists {
				project := projectFromContext(ctx)
				dataset := datasetFromContext(ctx)
				srv := serverFromContext(ctx)
				table, err := srv.findTableUsingRequestConnection(ctx, project.ID, dataset.ID, tableID)
				if err != nil {
					errorResponse(ctx, w, errInternalError(fmt.Sprintf("could not fetch table %s: %s", tableID, err)))
					return
				}
				if table == nil {
					errorResponse(ctx, w, errNotFound(fmt.Sprintf("table %s is not found", tableID)))
					return
				}
				ctx = withTable(ctx, table)
			}
			next.ServeHTTP(
				w,
				r.WithContext(ctx),
			)
		})
	}
}

func withModelMiddleware() func(http.Handler) http.Handler {
	return func(next http.Handler) http.Handler {
		return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
			ctx := r.Context()
			params := mux.Vars(r)
			modelID, exists := modelIDFromParams(params)
			if exists {
				dataset := datasetFromContext(ctx)
				model, err := dataset.Model(ctx, modelID)
				if err != nil {
					errorResponse(ctx, w, errInternalError(fmt.Sprintf("failed to find model: %s", err)))
					return
				}
				if model == nil {
					errorResponse(ctx, w, errNotFound(fmt.Sprintf("model %s is not found", modelID)))
					return
				}
				ctx = withModel(ctx, model)
			}
			next.ServeHTTP(
				w,
				r.WithContext(ctx),
			)
		})
	}
}

func withRoutineMiddleware() func(http.Handler) http.Handler {
	return func(next http.Handler) http.Handler {
		return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
			ctx := r.Context()
			params := mux.Vars(r)
			routineID, exists := routineIDFromParams(params)
			if exists {
				dataset := datasetFromContext(ctx)
				routine, err := dataset.Routine(ctx, routineID)
				if err != nil {
					errorResponse(ctx, w, errInternalError(fmt.Sprintf("failed to find routine: %s", err)))
					return
				}
				if routine == nil {
					errorResponse(ctx, w, errNotFound(fmt.Sprintf("routine %s is not found", routineID)))
					return
				}
				ctx = withRoutine(ctx, routine)
			}
			next.ServeHTTP(
				w,
				r.WithContext(ctx),
			)
		})
	}
}
