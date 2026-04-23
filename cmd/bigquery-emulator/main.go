package main

import (
	"context"
	"errors"
	"fmt"
	"net"
	"net/http"
	"os"
	"os/signal"
	"strings"
	"syscall"

	"github.com/jessevdk/go-flags"
	"github.com/vantaboard/bigquery-emulator/internal/execution"
	"github.com/vantaboard/bigquery-emulator/server"
	"github.com/vantaboard/bigquery-emulator/types"
	"github.com/vantaboard/go-googlesql-engine/pprofserver"
)

const emulatorProjectsAPI = "/emulator/v1/projects"

type option struct {
	Project      string           `description:"[deprecated: use POST /emulator/v1/projects to create projects] optional seed project at startup" long:"project" env:"BIGQUERY_EMULATOR_PROJECT"`
	Dataset      string           `description:"optional seed dataset (only with --project)" long:"dataset" env:"BIGQUERY_EMULATOR_DATASET"`
	Host         string           `description:"specify the host" long:"host" default:"0.0.0.0"`
	HTTPPort     uint16           `description:"specify the http port number. this port used by bigquery api" long:"port" default:"9050"`
	GRPCPort     uint16           `description:"specify the grpc port number. this port used by bigquery storage api" long:"grpc-port" default:"9060"`
	LogLevel     server.LogLevel  `description:"specify the log level (debug/info/warn/error)" long:"log-level" default:"error"`
	LogFormat    server.LogFormat `description:"specify the log format (console/json)" long:"log-format" default:"console"`
	LogFile      string           `description:"append structured logs to this file (still logs to stderr)" long:"log-file" env:"BIGQUERY_EMULATOR_LOG_FILE"`
	Database     string           `description:"specify the database file, use :memory: for in-memory storage. if not specified, it will be a temp file" long:"database"`
	DataFromYAML string           `description:"specify the path to the YAML file that contains the initial data" long:"data-from-yaml"`
	DataFromJSON string           `description:"specify the path to the JSON file that contains the initial data (faster for large, multi-megabyte files)" long:"data-from-json"`
	ExecBackend  string           `description:"physical SQL engine: sqlite (default) or duckdb (requires binary built with -tags duckdb)" long:"execution-backend" env:"BQ_EMULATOR_EXECUTION_BACKEND" default:"sqlite"`
	PprofAddr    string           `description:"if non-empty, serve net/http/pprof on this listen address (e.g. 127.0.0.1:6060) for heap/CPU/mutex/block profiles" long:"pprof-addr" env:"BIGQUERY_EMULATOR_PPROF_ADDR"`
	// The following set process env for go-googlesql-engine before any SQL connection pool opens. See engine env GOOGLESQL_ENGINE_* in duckdb_explain.go.
	DuckExplainAnalyze  string `description:"DuckDB only: go-googlesql-engine GOOGLESQL_ENGINE_DUCK_EXPLAIN_ANALYZE (off|before|after) for read queries; pair with pprof" long:"duck-explain-analyze" env:"BQ_EMULATOR_DUCK_EXPLAIN_ANALYZE"`
	DuckExplainLog      bool   `description:"DuckDB only: set GOOGLESQL_ENGINE_DUCK_EXPLAIN_LOG=1 (EXPLAIN without ANALYZE before DML/CTAS)" long:"duck-explain-log" env:"BQ_EMULATOR_DUCK_EXPLAIN_LOG"`
	LogSQLCorrelation   bool   `description:"set GOOGLESQL_ENGINE_LOG_SQL_CORRELATION=1 (correlation_id on physical SQL logs for heap pairing)" long:"log-sql-correlation" env:"BQ_EMULATOR_LOG_SQL_CORRELATION"`
	DuckExplainMaxBytes string `description:"set GOOGLESQL_ENGINE_DUCK_EXPLAIN_ANALYZE_MAX_BYTES (default 262144)" long:"duck-explain-max-bytes" env:"BQ_EMULATOR_DUCK_EXPLAIN_MAX_BYTES"`
	Version             bool   `description:"print version" long:"version" short:"v"`
}

type exitCode int

const (
	exitOK    exitCode = 0
	exitError exitCode = 1
)

var (
	version  string
	revision string
)

func main() {
	os.Exit(int(run()))
}

func run() exitCode {
	args, opt, err := parseOpt()
	if err != nil {
		flagsErr, ok := err.(*flags.Error)
		if !ok {
			fmt.Fprintf(os.Stderr, "[bigquery-emulator] unknown parsed option error: %[1]T %[1]v\n", err)
			return exitError
		}
		if flagsErr.Type == flags.ErrHelp {
			return exitOK
		}
		return exitError
	}
	if err := runServer(args, opt); err != nil {
		fmt.Fprintln(os.Stderr, err)
		return exitError
	}
	return exitOK
}

func parseOpt() ([]string, option, error) {
	var opt option
	parser := flags.NewParser(&opt, flags.Default)
	args, err := parser.Parse()
	return args, opt, err
}

func runServer(args []string, opt option) error {
	if opt.Version {
		fmt.Fprintf(os.Stdout, "version: %s (%s)\n", version, revision)
		return nil
	}
	applyEmulatorToEngineSQLProfilingEnv(opt)
	pprofAddr := strings.TrimSpace(opt.PprofAddr)
	if pprofAddr == "" {
		pprofAddr = strings.TrimSpace(os.Getenv("GOOGLESQL_ENGINE_PPROF_ADDR"))
	}
	if pprofAddr != "" {
		pprofserver.Start(pprofAddr)
		fmt.Fprintf(os.Stdout, "[bigquery-emulator] pprof: http://%s/debug/pprof/ (heap: /debug/pprof/heap)\n", pprofAddr)
	}
	if opt.Dataset != "" && opt.Project == "" {
		return fmt.Errorf("--dataset requires --project, or omit both and create a project with POST %s (JSON: {\"id\":\"<project-id>\"})", emulatorProjectsAPI)
	}
	if opt.Project != "" {
		fmt.Fprintf(os.Stderr, "[bigquery-emulator] deprecated: --project and BIGQUERY_EMULATOR_PROJECT are deprecated; create projects with POST %s (JSON body: {\"id\":\"<project-id>\"}). This option will be removed in a future release.\n", emulatorProjectsAPI)
	}
	var db server.Storage
	if opt.Database == ":memory:" {
		db = server.MemoryStorage
	} else if opt.Database == "" {
		db = server.TempStorage
	} else {
		db = server.Storage(fmt.Sprintf("file:%s?cache=shared", opt.Database))
	}

	// Default loopback for the merged explorer UI API (internal/explorerapi), which uses the Go
	// BigQuery client against this same process. Override with BIGQUERY_EMULATOR_HOST to point elsewhere.
	if os.Getenv("BIGQUERY_EMULATOR_HOST") == "" {
		host := opt.Host
		if host == "0.0.0.0" || host == "[::]" {
			host = "127.0.0.1"
		}
		os.Setenv("BIGQUERY_EMULATOR_HOST", net.JoinHostPort(host, fmt.Sprintf("%d", opt.HTTPPort)))
	}

	backend, err := execution.ParseBackend(opt.ExecBackend)
	if err != nil {
		return err
	}

	bqServer, err := server.New(db, server.WithExecutionBackend(backend))
	if err != nil {
		return err
	}
	if opt.Project != "" {
		project := types.NewProject(opt.Project)
		if opt.Dataset != "" {
			project.Datasets = append(project.Datasets, types.NewDataset(opt.Dataset))
		}
		if err := bqServer.SetProject(project.ID); err != nil {
			return err
		}
		if err := bqServer.Load(server.StructSource(project)); err != nil {
			return err
		}
	}
	if err := bqServer.SetLogLevel(opt.LogLevel); err != nil {
		return err
	}
	if err := bqServer.SetLogFormat(opt.LogFormat); err != nil {
		return err
	}
	if opt.LogFile != "" {
		if err := bqServer.SetLogFile(opt.LogFile); err != nil {
			return err
		}
	}
	if opt.DataFromYAML != "" {
		if err := bqServer.Load(server.YAMLSource(opt.DataFromYAML)); err != nil {
			return err
		}
	}

	if opt.DataFromJSON != "" {
		if err := bqServer.Load(server.JSONSource(opt.DataFromJSON)); err != nil {
			return err
		}
	}

	ctx := context.Background()
	interrupt := make(chan os.Signal, 1)
	signal.Notify(interrupt, os.Interrupt, syscall.SIGTERM)

	go func() {
		select {
		case s := <-interrupt:
			fmt.Fprintf(os.Stdout, "[bigquery-emulator] receive %s. shutdown gracefully\n", s)
			if err := bqServer.Stop(ctx); err != nil {
				fmt.Fprintf(os.Stderr, "[bigquery-emulator] failed to stop: %v\n", err)
			}
		}
	}()

	done := make(chan error)
	go func() {
		httpAddr := fmt.Sprintf("%s:%d", opt.Host, opt.HTTPPort)
		grpcAddr := fmt.Sprintf("%s:%d", opt.Host, opt.GRPCPort)
		fmt.Fprintf(os.Stdout, "[bigquery-emulator] REST server listening at %s\n", httpAddr)
		fmt.Fprintf(os.Stdout, "[bigquery-emulator] gRPC server listening at %s\n", grpcAddr)
		done <- bqServer.Serve(ctx, httpAddr, grpcAddr)
	}()

	select {
	case err := <-done:
		if errors.Is(err, http.ErrServerClosed) {
			return nil
		}
		return err
	}

	return nil
}

// applyEmulatorToEngineSQLProfilingEnv sets go-googlesql-engine env vars (see internal/duckdb_explain.go)
// before the first database driver connection is created.
func applyEmulatorToEngineSQLProfilingEnv(opt option) {
	if v := strings.TrimSpace(opt.DuckExplainAnalyze); v != "" {
		_ = os.Setenv("GOOGLESQL_ENGINE_DUCK_EXPLAIN_ANALYZE", v)
	}
	if opt.DuckExplainLog {
		_ = os.Setenv("GOOGLESQL_ENGINE_DUCK_EXPLAIN_LOG", "1")
	}
	if opt.LogSQLCorrelation {
		_ = os.Setenv("GOOGLESQL_ENGINE_LOG_SQL_CORRELATION", "1")
	}
	if v := strings.TrimSpace(opt.DuckExplainMaxBytes); v != "" {
		_ = os.Setenv("GOOGLESQL_ENGINE_DUCK_EXPLAIN_ANALYZE_MAX_BYTES", v)
	}
}
