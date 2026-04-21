package server

import (
	"context"
	"database/sql"
	"fmt"
	"io"
	"log/slog"
	"net"
	"net/http"
	"os"
	"strings"
	"time"

	"golang.org/x/sync/errgroup"
	"google.golang.org/grpc"

	"github.com/gorilla/mux"
	"github.com/vantaboard/bigquery-emulator/internal/connection"
	"github.com/vantaboard/bigquery-emulator/internal/contentdata"
	"github.com/vantaboard/bigquery-emulator/internal/explorerapi"
	"github.com/vantaboard/bigquery-emulator/internal/metadata"
)

type Server struct {
	Handler           http.Handler
	storage           Storage
	db                *sql.DB
	logLevel          *slog.LevelVar
	logFormat         LogFormat
	logFile           *os.File
	logger            *slog.Logger
	connMgr           *connection.Manager
	metaRepo          *metadata.Repository
	contentRepo       *contentdata.Repository
	queryExec         *JobExecutor
	heavyQuerySem     chan struct{}
	storageReadSem    chan struct{}
	fileCleanup       func() error
	explorerCleanup   func() error
	autoscaleCancel   context.CancelFunc
	httpServer        *http.Server
	grpcServer        *grpc.Server
}

func (s *Server) rebuildLogger() {
	opts := &slog.HandlerOptions{Level: s.logLevel}
	w := io.Writer(os.Stderr)
	if s.logFile != nil {
		w = io.MultiWriter(os.Stderr, s.logFile)
	}
	var h slog.Handler
	switch s.logFormat {
	case LogFormatJSON:
		h = slog.NewJSONHandler(w, opts)
	default:
		h = slog.NewTextHandler(w, opts)
	}
	s.logger = slog.New(h)
}

// SetLogFile appends structured logs to path in addition to stderr. Pass "" to
// stop writing to a file and close any previously opened file.
func (s *Server) SetLogFile(path string) error {
	if s.logFile != nil {
		if err := s.logFile.Close(); err != nil {
			return fmt.Errorf("close previous log file: %w", err)
		}
		s.logFile = nil
	}
	if path == "" {
		s.rebuildLogger()
		return nil
	}
	f, err := os.OpenFile(path, os.O_CREATE|os.O_APPEND|os.O_WRONLY, 0o644)
	if err != nil {
		return fmt.Errorf("open log file: %w", err)
	}
	s.logFile = f
	s.rebuildLogger()
	return nil
}

// storageWithSQLiteDefaults appends modernc.org/sqlite URI parameters so native SQLite pragmas
// run at open time (not via googlesqlite Exec, which only accepts GoogleSQL).
// WAL lets readers (job polls, table GETs) proceed while an async worker holds a long CTAS write.
func storageWithSQLiteDefaults(s Storage) Storage {
	str := string(s)
	if strings.Contains(str, "_pragma=") && strings.Contains(str, "journal_mode") {
		return s
	}
	sep := "?"
	if strings.Contains(str, "?") {
		sep = "&"
	}
	// See modernc.org/sqlite applyQueryParams: _pragma values are executed as "pragma "+v
	return Storage(str + sep + "_pragma=journal_mode%3DWAL&_pragma=busy_timeout%3D30000")
}

type serverConfig struct {
	poolOverride int
}

// ServerOption configures [New].
type ServerOption func(*serverConfig) error

// WithConnectionPoolSize sets a fixed pooled sqlite connection count (disables elastic autoscaling).
// When unset, [connection.PoolConfigFromEnv] applies (see Environment on [New]).
func WithConnectionPoolSize(n int) ServerOption {
	return func(c *serverConfig) error {
		if n > 0 {
			c.poolOverride = n
		}
		return nil
	}
}

// New builds a Server.
//
// Environment:
//   - BQ_EMULATOR_POOL_SIZE: fixed pool size (disables elastic pool).
//   - BQ_EMULATOR_POOL_MIN / BQ_EMULATOR_POOL_MAX: optional clamps when using elastic pool.
//   - BQ_EMULATOR_POOL_AUTOSCALE=0: disable background pool cap tuning (elastic mode only).
//   - BQ_EMULATOR_CONN_METRICS=1: enable timing in [connection.SnapshotConnMetrics] (see that package).
//   - BQ_EMULATOR_ASYNC_JOB_HEARTBEAT_SECS: async query progress logs every N seconds at INFO (default 30; 0 disables).
func New(storage Storage, opts ...ServerOption) (*Server, error) {
	cfg := serverConfig{}
	for _, o := range opts {
		if o == nil {
			continue
		}
		if err := o(&cfg); err != nil {
			return nil, err
		}
	}

	server := &Server{storage: storage}
	if storage == TempStorage {
		f, err := os.CreateTemp("", "")
		if err != nil {
			return nil, fmt.Errorf("failed to create temporary file: %w", err)
		}
		storage = Storage(fmt.Sprintf("file:%s?cache=shared", f.Name()))
		server.storage = storage
		server.fileCleanup = func() error {
			return os.Remove(f.Name())
		}
	}
	storage = storageWithSQLiteDefaults(storage)
	server.storage = storage

	db, err := sql.Open("googlesqlite", string(storage))
	if err != nil {
		return nil, err
	}
	db.SetConnMaxIdleTime(-1)
	db.SetConnMaxLifetime(1<<63 - 1)
	server.db = db

	poolCfg := connection.PoolConfigFromEnv()
	if cfg.poolOverride > 0 {
		connection.WithPoolSize(cfg.poolOverride)(&poolCfg)
	}
	db.SetMaxOpenConns(poolCfg.PoolMaxHard)

	lv := new(slog.LevelVar)
	lv.Set(slog.LevelError)
	server.logLevel = lv
	server.logFormat = LogFormatConsole
	server.rebuildLogger()

	connectionManager, err := connection.NewManager(context.Background(), db, connection.WithPoolConfig(poolCfg))
	if err != nil {
		return nil, err
	}
	server.connMgr = connectionManager

	if poolCfg.AutoscaleLoop && poolCfg.PoolMin < poolCfg.PoolMaxHard && poolCfg.FixedSize == 0 {
		ascCtx, ascCancel := context.WithCancel(context.Background())
		server.autoscaleCancel = ascCancel
		connection.StartAutoscaleLoop(ascCtx, connectionManager, 0)
	}
	metaRepo, err := metadata.NewRepository(db, connectionManager)
	if err != nil {
		return nil, err
	}
	server.metaRepo = metaRepo
	server.contentRepo = contentdata.NewRepository(db)

	server.queryExec = newJobExecutor(server, defaultQueryWorkers, defaultQueryQueueDepth)
	registerQueryJobCanceller(server.queryExec)
	server.heavyQuerySem = make(chan struct{}, defaultQueryWorkers)
	server.storageReadSem = make(chan struct{}, defaultStorageReadSlots)

	r := mux.NewRouter()
	for _, handler := range handlers {
		r.Handle(handler.Path, handler.Handler).Methods(handler.HTTPMethod)
		r.Handle(fmt.Sprintf("/bigquery/v2%s", handler.Path), handler.Handler).Methods(handler.HTTPMethod)
	}
	r.Handle(discoveryAPIEndpoint, newDiscoveryHandler(server)).Methods("GET")
	r.Handle(newDiscoveryAPIEndpoint, newDiscoveryHandler(server)).Methods("GET")
	r.Handle(uploadAPIEndpoint, &uploadHandler{}).Methods("POST")
	r.Handle(uploadAPIEndpoint, &uploadContentHandler{}).Methods("PUT")
	registerEmulatorProjectRoutes(r, server)

	explorerHandler, explorerCleanup, err := explorerapi.NewHTTPHandler()
	if err != nil {
		return nil, fmt.Errorf("explorer api: %w", err)
	}
	server.explorerCleanup = explorerCleanup
	r.PathPrefix(explorerapi.APIPrefix).Handler(explorerHandler)

	r.PathPrefix("/").Handler(&defaultHandler{})
	r.Use(recoveryMiddleware(server))
	r.Use(loggerMiddleware(server))
	r.Use(accessLogMiddleware())
	r.Use(decompressMiddleware())
	r.Use(withServerMiddleware(server))
	r.Use(withConnectionMiddleware())
	r.Use(withProjectMiddleware())
	r.Use(withDatasetMiddleware())
	r.Use(withJobMiddleware())
	r.Use(withTableMiddleware())
	r.Use(withModelMiddleware())
	r.Use(withRoutineMiddleware())
	server.Handler = r
	return server, nil
}

func (s *Server) Close() error {
	defer func() {
		if s.logFile != nil {
			if err := s.logFile.Close(); err != nil {
				fmt.Fprintf(os.Stderr, "[bigquery-emulator] failed to close log file: %v\n", err)
			}
			s.logFile = nil
		}
	}()
	defer func() {
		if s.fileCleanup != nil {
			if err := s.fileCleanup(); err != nil {
				s.logger.Error("failed to cleanup file", "err", err)
			}
		}
	}()
	if s.autoscaleCancel != nil {
		s.autoscaleCancel()
	}
	if s.queryExec != nil {
		s.queryExec.Stop()
	}
	registerQueryJobCanceller(nil)
	if s.explorerCleanup != nil {
		if err := s.explorerCleanup(); err != nil {
			s.logger.Error("failed to close explorer api client", "err", err)
		}
	}
	if s.connMgr != nil {
		if err := s.connMgr.Close(); err != nil {
			s.logger.Error("failed to close connection manager", "err", err)
		}
	}
	if err := s.db.Close(); err != nil {
		s.logger.Error("failed to close database", "err", err)
		return err
	}
	return nil
}

func (s *Server) SetProject(id string) error {
	ctx := context.Background()
	err := s.connMgr.ExecuteWithTransaction(ctx, func(ctx context.Context, tx *connection.Tx) error {
		tx.SetProjectAndDataset(id, "")
		if err := tx.MetadataRepoMode(); err != nil {
			return err
		}
		if err := s.metaRepo.AddProjectIfNotExists(
			ctx,
			tx.Tx(),
			metadata.NewProject(s.metaRepo, id),
		); err != nil {
			return err
		}
		return nil
	})
	if err != nil {
		return err
	}
	return nil
}

// findProjectUsingRequestConnection loads project metadata using only the HTTP request's
// pooled connection. metaRepo.FindProject would nest ExecuteWithTransaction and acquire a
// second connection while withConnectionMiddleware still holds the first.
func (s *Server) findProjectUsingRequestConnection(ctx context.Context, projectID string) (*metadata.Project, error) {
	mc := connectionFromContext(ctx)
	tx, err := mc.Begin(ctx)
	if err != nil {
		return nil, err
	}
	defer tx.RollbackIfNotCommitted()
	if err := tx.MetadataRepoMode(); err != nil {
		return nil, err
	}
	tx.SetProjectAndDataset(projectID, "")
	project, err := s.metaRepo.FindProjectWithConn(ctx, tx.Tx(), projectID)
	if err != nil {
		return nil, err
	}
	if err := tx.Commit(); err != nil {
		return nil, err
	}
	return project, nil
}

// findDatasetUsingRequestConnection loads dataset metadata using only the HTTP request connection.
func (s *Server) findDatasetUsingRequestConnection(ctx context.Context, projectID, datasetID string) (*metadata.Dataset, error) {
	mc := connectionFromContext(ctx)
	tx, err := mc.Begin(ctx)
	if err != nil {
		return nil, err
	}
	defer tx.RollbackIfNotCommitted()
	if err := tx.MetadataRepoMode(); err != nil {
		return nil, err
	}
	tx.SetProjectAndDataset(projectID, "")
	ds, err := s.metaRepo.FindDatasetWithConnection(ctx, tx.Tx(), projectID, datasetID)
	if err != nil {
		return nil, err
	}
	if err := tx.Commit(); err != nil {
		return nil, err
	}
	return ds, nil
}

// findTableUsingRequestConnection loads table metadata using only the HTTP request connection.
func (s *Server) findTableUsingRequestConnection(ctx context.Context, projectID, datasetID, tableID string) (*metadata.Table, error) {
	mc := connectionFromContext(ctx)
	tx, err := mc.Begin(ctx)
	if err != nil {
		return nil, err
	}
	defer tx.RollbackIfNotCommitted()
	if err := tx.MetadataRepoMode(); err != nil {
		return nil, err
	}
	tx.SetProjectAndDataset(projectID, datasetID)
	tbl, err := s.metaRepo.FindTableWithConnection(ctx, tx.Tx(), projectID, datasetID, tableID)
	if err != nil {
		return nil, err
	}
	if err := tx.Commit(); err != nil {
		return nil, err
	}
	return tbl, nil
}

// findProjectAndJobUsingRequestConnection loads project and job in a single transaction on the
// request connection. Calling findProject then findJob separately runs two transactions and was
// observed to increase errors under concurrent polls plus async work.
func (s *Server) findProjectAndJobUsingRequestConnection(ctx context.Context, projectID, jobID string) (*metadata.Project, *metadata.Job, error) {
	mc := connectionFromContext(ctx)
	tx, err := mc.Begin(ctx)
	if err != nil {
		return nil, nil, err
	}
	defer tx.RollbackIfNotCommitted()
	if err := tx.MetadataRepoMode(); err != nil {
		return nil, nil, err
	}
	tx.SetProjectAndDataset(projectID, "")
	project, err := s.metaRepo.FindProjectWithConn(ctx, tx.Tx(), projectID)
	if err != nil {
		return nil, nil, err
	}
	job, err := s.metaRepo.FindJobWithConn(ctx, tx.Tx(), projectID, jobID)
	if err != nil {
		return nil, nil, err
	}
	if err := tx.Commit(); err != nil {
		return nil, nil, err
	}
	return project, job, nil
}

// findJobUsingRequestConnection loads job metadata using only the HTTP request's
// pooled connection. project.Job uses FindJob, which acquires a second pool
// connection; nested holds can exhaust the pool and deadlock while an async query
// worker also holds a connection (polls then block until the query finishes).
func (s *Server) findJobUsingRequestConnection(ctx context.Context, projectID, jobID string) (*metadata.Job, error) {
	start := time.Now()
	mc := connectionFromContext(ctx)
	tx, err := mc.Begin(ctx)
	if err != nil {
		return nil, err
	}
	defer tx.RollbackIfNotCommitted()
	if err := tx.MetadataRepoMode(); err != nil {
		return nil, err
	}
	tx.SetProjectAndDataset(projectID, "")
	job, err := s.metaRepo.FindJobWithConn(ctx, tx.Tx(), projectID, jobID)
	if err != nil {
		return nil, err
	}
	if err := tx.Commit(); err != nil {
		return nil, err
	}
	if elapsed := time.Since(start); elapsed >= 5*time.Millisecond {
		s.logger.Debug("findJobUsingRequestConnection slow",
			"project_id", projectID,
			"job_id", jobID,
			"elapsed_ms", elapsed.Milliseconds(),
		)
	}
	return job, nil
}

type LogLevel string

const (
	LogLevelUnknown LogLevel = "unknown"
	LogLevelDebug   LogLevel = "debug"
	LogLevelInfo    LogLevel = "info"
	LogLevelWarn    LogLevel = "warn"
	LogLevelError   LogLevel = "error"
	LogLevelFatal   LogLevel = "fatal"
)

func (s *Server) SetLogLevel(level LogLevel) error {
	var lv slog.Level
	switch level {
	case LogLevelDebug:
		lv = slog.LevelDebug
	case LogLevelInfo:
		lv = slog.LevelInfo
	case LogLevelWarn:
		lv = slog.LevelWarn
	case LogLevelError:
		lv = slog.LevelError
	case LogLevelFatal:
		lv = slog.LevelError
	default:
		return fmt.Errorf("unexpected log level %s", level)
	}
	s.logLevel.Set(lv)
	return nil
}

type LogFormat string

const (
	LogFormatConsole LogFormat = "console"
	LogFormatJSON    LogFormat = "json"
)

func (s *Server) SetLogFormat(format LogFormat) error {
	switch format {
	case LogFormatConsole, LogFormatJSON:
		s.logFormat = format
	default:
		return fmt.Errorf("unexpected log format %s", format)
	}
	s.rebuildLogger()
	return nil
}

func (s *Server) Load(sources ...Source) error {
	for _, source := range sources {
		if err := source(s); err != nil {
			return err
		}
	}
	return nil
}

func (s *Server) Serve(ctx context.Context, httpAddr, grpcAddr string) error {
	httpServer := &http.Server{
		Handler:      s.Handler,
		Addr:         httpAddr,
		WriteTimeout: 5 * time.Minute,
		ReadTimeout:  15 * time.Second,
	}
	s.httpServer = httpServer

	grpcServer := grpc.NewServer()
	registerStorageServer(grpcServer, s)
	s.grpcServer = grpcServer

	httpListener, err := net.Listen("tcp", httpAddr)
	if err != nil {
		return err
	}
	grpcListener, err := net.Listen("tcp", grpcAddr)
	if err != nil {
		return err
	}

	var eg errgroup.Group
	eg.Go(func() error { return grpcServer.Serve(grpcListener) })
	eg.Go(func() error { return httpServer.Serve(httpListener) })
	return eg.Wait()
}

func (s *Server) Stop(ctx context.Context) error {
	defer s.Close()

	if s.grpcServer != nil {
		s.grpcServer.GracefulStop()
	}
	if s.httpServer != nil {
		return s.httpServer.Shutdown(ctx)
	}
	return nil
}
