package server

import (
	"context"
	"database/sql"
	"fmt"
	"log/slog"
	"net"
	"net/http"
	"os"
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
	Handler         http.Handler
	storage         Storage
	db              *sql.DB
	logLevel        *slog.LevelVar
	logFormat       LogFormat
	logger          *slog.Logger
	connMgr         *connection.Manager
	metaRepo        *metadata.Repository
	contentRepo     *contentdata.Repository
	queryExec       *JobExecutor
	heavyQuerySem   chan struct{}
	storageReadSem  chan struct{}
	fileCleanup     func() error
	explorerCleanup func() error
	httpServer      *http.Server
	grpcServer      *grpc.Server
}

func (s *Server) rebuildLogger() {
	opts := &slog.HandlerOptions{Level: s.logLevel}
	var h slog.Handler
	switch s.logFormat {
	case LogFormatJSON:
		h = slog.NewJSONHandler(os.Stderr, opts)
	default:
		h = slog.NewTextHandler(os.Stderr, opts)
	}
	s.logger = slog.New(h)
}

func New(storage Storage) (*Server, error) {
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
	db, err := sql.Open("googlesqlite", string(storage))
	if err != nil {
		return nil, err
	}
	db.SetConnMaxIdleTime(-1)
	db.SetConnMaxLifetime(1<<63 - 1)
	server.db = db

	lv := new(slog.LevelVar)
	lv.Set(slog.LevelError)
	server.logLevel = lv
	server.logFormat = LogFormatConsole
	server.rebuildLogger()

	connectionManager, err := connection.NewManager(context.Background(), db, connection.WithPoolSize(defaultConnectionPoolSize))
	if err != nil {
		return nil, err
	}
	server.connMgr = connectionManager
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
		if s.fileCleanup != nil {
			if err := s.fileCleanup(); err != nil {
				s.logger.Error("failed to cleanup file", "err", err)
			}
		}
	}()
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
	s.logger.Debug("findJobUsingRequestConnection",
		"project_id", projectID,
		"job_id", jobID,
		"elapsed_ms", time.Since(start).Milliseconds(),
	)
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
