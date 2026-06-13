package runner

import (
	"context"
	"time"
)

// TargetName identifies a benchmark backend.
type TargetName string

const (
	TargetEmulator TargetName = "emulator"
	TargetGoccy    TargetName = "goccy"
	TargetBigQuery TargetName = "bigquery"
)

// Outcome is the per-(case,target) result classification.
type Outcome string

const (
	OutcomeOK          Outcome = "ok"
	OutcomeError       Outcome = "error"
	OutcomeWrongResult Outcome = "wrong_result"
	OutcomeTimeout     Outcome = "timeout"
	OutcomeSkipped     Outcome = "skipped"
)

// QueryResult holds one query execution sample.
type QueryResult struct {
	Elapsed        time.Duration
	ExecutionOnly  time.Duration
	ExecutionValid bool // true when BQ startTime/endTime were present
	QueueOnly      time.Duration
	SlotMs         int64
	BytesProcessed int64
	Rows           []map[string]string
	RowCount       int
	ResultHash     string
	Route          string
	Phases         map[string]int64 // phase name -> microseconds
	Error          string
}

// Target runs benchmark cases against one backend.
type Target interface {
	Name() TargetName
	Start(ctx context.Context) error
	SetupCase(ctx context.Context, c Case, dataset string) error
	RunQuery(ctx context.Context, c Case, sql string, timeout time.Duration) (QueryResult, error)
	Cleanup(ctx context.Context) error
}

// TargetOptions configures benchmark targets.
type TargetOptions struct {
	EngineBinary string
	GoccyImage   string
	BQProject    string
	BQLocation   string
	Timeout      time.Duration
}
