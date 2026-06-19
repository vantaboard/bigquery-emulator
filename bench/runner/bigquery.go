package runner

import (
	"context"
	"errors"
	"fmt"
	"math/rand"
	"net/http"
	"strconv"
	"strings"
	"time"

	"cloud.google.com/go/bigquery"
	"google.golang.org/api/googleapi"
	"google.golang.org/api/iterator"
)

// Exponential-backoff knobs for BigQuery rate-limit/quota retries. These
// are vars (not consts) so tests can shrink the waits. DDL-heavy cases
// (CREATE OR REPLACE TABLE/VIEW on the same object) trip BigQuery's
// per-table metadata-update quota, which only clears over several seconds;
// backoff spreads retries until the window reopens.
//
// bqAttemptTimeout caps a single submission attempt. The BigQuery client
// retries jobRateLimitExceeded *internally* on the call's context until that
// context is done (see runWithRetryExplicit in the client), so without a
// per-attempt cap the client's own retry consumes the entire per-query
// deadline and our backoff loop never runs (the "0 retries" symptom). Capping
// each attempt hands control back to this loop while still leaving the client's
// short internal backoff intact within the slice. It must exceed the slowest
// legitimate query/setup in the suite (a few seconds) by a wide margin.
var (
	bqBaseBackoff    = 1 * time.Second
	bqMaxBackoff     = 32 * time.Second
	bqMaxRetries     = 8
	bqAttemptTimeout = 30 * time.Second
)

// isNotFound reports whether err is a BigQuery 404 (dataset absent).
func isNotFound(err error) bool {
	var apiErr *googleapi.Error
	return errors.As(err, &apiErr) && apiErr.Code == http.StatusNotFound
}

// BigQueryTarget runs cases against real BigQuery via ADC.
type BigQueryTarget struct {
	opts     TargetOptions
	client   *bigquery.Client
	project  string
	datasets []string
	location string
}

func NewBigQueryTarget(opts TargetOptions) *BigQueryTarget {
	return &BigQueryTarget{opts: opts}
}

func (t *BigQueryTarget) Name() TargetName { return TargetBigQuery }

func (t *BigQueryTarget) Start(ctx context.Context) error {
	if t.opts.BQProject == "" {
		return errors.New("BENCH_BQ_PROJECT or --project is required for bigquery target")
	}
	t.project = t.opts.BQProject
	t.location = t.opts.BQLocation
	if t.location == "" {
		t.location = "US"
	}
	client, err := bigquery.NewClient(ctx, t.project)
	if err != nil {
		return fmt.Errorf("bigquery.NewClient: %w", err)
	}
	t.client = client
	return nil
}

// ProjectID returns the billing project for BigQuery runs.
func (t *BigQueryTarget) ProjectID() string { return t.project }

func (t *BigQueryTarget) SetupCase(ctx context.Context, c Case, dataset string) error {
	dsID := strings.TrimPrefix(dataset, t.project+".")
	// Drop any leftover dataset from a previous (interrupted) run so
	// setup always starts from a clean slate. NotFound is the normal
	// case and is ignored.
	if err := t.client.Dataset(dsID).DeleteWithContents(ctx); err != nil && !isNotFound(err) {
		return fmt.Errorf("delete stale dataset %s: %w", dsID, err)
	}
	meta := &bigquery.DatasetMetadata{
		Location:               t.location,
		DefaultTableExpiration: 24 * time.Hour,
	}
	if err := t.client.Dataset(dsID).Create(ctx, meta); err != nil {
		return fmt.Errorf("create dataset %s: %w", dsID, err)
	}
	t.datasets = append(t.datasets, dsID)
	setup, _ := c.Substitute(dataset, t.project)
	for _, sql := range setup {
		if err := t.runSQL(ctx, sql); err != nil {
			return err
		}
	}
	return nil
}

func (t *BigQueryTarget) RunQuery(ctx context.Context, c Case, sql string, timeout time.Duration) (QueryResult, error) {
	if timeout <= 0 {
		timeout = time.Duration(defaultTimeoutMS) * time.Millisecond
	}
	return timedQuery(ctx, func(ctx context.Context) (QueryResult, error) {
		job, err := t.runJob(ctx, sql)
		if err != nil {
			return QueryResult{Error: err.Error()}, err
		}
		status := job.LastStatus()
		metrics, err := extractBQJobMetrics(status)
		if err != nil {
			return QueryResult{Error: err.Error()}, err
		}
		if metrics.cacheHit {
			cacheErr := errors.New("bigquery query cache hit (DisableQueryCache ineffective)")
			return QueryResult{Error: cacheErr.Error()}, cacheErr
		}
		it, err := job.Read(ctx)
		if err != nil {
			return QueryResult{
				Error:          err.Error(),
				ExecutionOnly:  metrics.execution,
				ExecutionValid: true,
				QueueOnly:      metrics.queue,
				SlotMs:         metrics.slotMs,
			}, err
		}
		rows, err := readAllRows(it)
		if err != nil {
			return QueryResult{
				Error:          err.Error(),
				ExecutionOnly:  metrics.execution,
				ExecutionValid: true,
				QueueOnly:      metrics.queue,
				SlotMs:         metrics.slotMs,
			}, err
		}
		maps := bqRowsToMaps(rows)
		hash, _ := HashRows(maps)
		return QueryResult{
			ExecutionOnly:  metrics.execution,
			ExecutionValid: true,
			QueueOnly:      metrics.queue,
			SlotMs:         metrics.slotMs,
			BytesProcessed: metrics.bytesProcessed,
			Rows:           maps,
			RowCount:       len(maps),
			ResultHash:     hash,
		}, nil
	}, timeout)
}

func (t *BigQueryTarget) Cleanup(ctx context.Context) error {
	if t.client == nil {
		return nil
	}
	for _, ds := range t.datasets {
		if err := t.client.Dataset(ds).DeleteWithContents(ctx); err != nil {
			_ = t.client.Close()
			return err
		}
	}
	return t.client.Close()
}

func (t *BigQueryTarget) runSQL(ctx context.Context, sql string) error {
	job, err := t.runJob(ctx, sql)
	if err != nil {
		return err
	}
	status, err := job.Wait(ctx)
	if err != nil {
		return err
	}
	if err := status.Err(); err != nil {
		return err
	}
	return nil
}

func (t *BigQueryTarget) runJob(ctx context.Context, sql string) (*bigquery.Job, error) {
	return retryOnRateLimit(ctx, func(ctx context.Context) (*bigquery.Job, error) {
		return t.runJobOnce(ctx, sql)
	})
}

func (t *BigQueryTarget) runJobOnce(ctx context.Context, sql string) (*bigquery.Job, error) {
	q := t.client.Query(sql)
	// Benchmarks must never read cached results; cache hits yield ~0ms execution.
	q.DisableQueryCache = true
	q.Location = t.location
	job, err := q.Run(ctx)
	if err != nil {
		return nil, err
	}
	status, err := job.Wait(ctx)
	if err != nil {
		return nil, err
	}
	if err := status.Err(); err != nil {
		return nil, err
	}
	return job, nil
}

// isRateLimitErr reports whether err is a BigQuery throttling/quota or
// transient backend error worth retrying with backoff. Rate-limit errors
// arrive in two shapes: a structured googleapi reason, or an HTTP 400 whose
// only signal is the reason text in the message (e.g. the
// "Job exceeded rate limits: ... jobRateLimitExceeded" we see on repeated
// CREATE OR REPLACE statements).
func isRateLimitErr(err error) bool {
	if err == nil {
		return false
	}
	var apiErr *googleapi.Error
	if errors.As(err, &apiErr) {
		switch apiErr.Code {
		case http.StatusTooManyRequests, // 429
			http.StatusInternalServerError, // 500
			http.StatusBadGateway,          // 502
			http.StatusServiceUnavailable:  // 503
			return true
		}
		for _, e := range apiErr.Errors {
			switch e.Reason {
			case "rateLimitExceeded", "jobRateLimitExceeded",
				"quotaExceeded", "backendError", "internalError":
				return true
			}
		}
	}
	msg := strings.ToLower(err.Error())
	for _, frag := range []string{
		"ratelimitexceeded",
		"jobratelimitexceeded",
		"exceeded rate limits",
		"exceeded quota",
		"quotaexceeded",
		"backenderror",
	} {
		if strings.Contains(msg, frag) {
			return true
		}
	}
	return false
}

// retryOnRateLimit runs fn, retrying rate-limit/quota/backend errors with
// exponential backoff and full jitter until success, a non-retryable error,
// the retry budget is exhausted, or ctx expires.
//
// Each attempt runs against a sub-context capped at bqAttemptTimeout so the
// BigQuery client's internal retryer cannot consume the whole parent deadline
// before this loop gets to back off (context.WithTimeout also caps at the
// parent's own deadline, so we never exceed the per-query budget).
func retryOnRateLimit(
	ctx context.Context,
	fn func(context.Context) (*bigquery.Job, error),
) (*bigquery.Job, error) {
	backoff := bqBaseBackoff
	for attempt := 0; ; attempt++ {
		attemptCtx, cancel := context.WithTimeout(ctx, bqAttemptTimeout)
		job, err := fn(attemptCtx)
		cancel()
		if err == nil {
			return job, nil
		}
		// A capped attempt that timed out on the rate-limit reason is still a
		// rate-limit error worth backing off on; isRateLimitErr matches the
		// reason text the client leaves in the wrapped deadline error.
		if !isRateLimitErr(err) {
			return nil, err
		}
		if attempt >= bqMaxRetries {
			return nil, fmt.Errorf("rate limit: exhausted %d attempts: %w", attempt+1, err)
		}
		// Parent deadline/cancellation reached: no budget left to back off.
		if cerr := ctx.Err(); cerr != nil {
			return nil, fmt.Errorf("rate limit: parent context done after %d attempts: %w (last error: %w)",
				attempt+1, cerr, err)
		}
		// Full jitter: wait in [0, backoff] to avoid synchronized retries.
		wait := time.Duration(rand.Int63n(int64(backoff) + 1)) //nolint:gosec // jitter, not crypto
		timer := time.NewTimer(wait)
		select {
		case <-ctx.Done():
			timer.Stop()
			return nil, fmt.Errorf("rate limit: backoff aborted after %d attempts: %w (last error: %w)",
				attempt+1, ctx.Err(), err)
		case <-timer.C:
		}
		if backoff < bqMaxBackoff {
			backoff *= 2
			if backoff > bqMaxBackoff {
				backoff = bqMaxBackoff
			}
		}
	}
}

type bqJobMetrics struct {
	execution      time.Duration
	queue          time.Duration
	slotMs         int64
	cacheHit       bool
	bytesProcessed int64
}

func extractBQJobMetrics(status *bigquery.JobStatus) (bqJobMetrics, error) {
	if status == nil || status.Statistics == nil {
		return bqJobMetrics{}, errors.New("missing BigQuery job statistics")
	}
	st := status.Statistics
	if st.StartTime.IsZero() || st.EndTime.IsZero() {
		return bqJobMetrics{}, errors.New("missing BigQuery startTime or endTime")
	}
	m := bqJobMetrics{
		execution:      st.EndTime.Sub(st.StartTime),
		bytesProcessed: st.TotalBytesProcessed,
	}
	if !st.CreationTime.IsZero() && st.StartTime.After(st.CreationTime) {
		m.queue = st.StartTime.Sub(st.CreationTime)
	}
	if qs, ok := st.Details.(*bigquery.QueryStatistics); ok {
		m.cacheHit = qs.CacheHit
		m.slotMs = qs.SlotMillis
	}
	if m.slotMs == 0 && st.TotalSlotDuration > 0 {
		m.slotMs = st.TotalSlotDuration.Milliseconds()
	}
	return m, nil
}

func readAllRows(it *bigquery.RowIterator) ([]map[string]bigquery.Value, error) {
	var out []map[string]bigquery.Value
	for {
		var row map[string]bigquery.Value
		err := it.Next(&row)
		if errors.Is(err, iterator.Done) {
			break
		}
		if err != nil {
			return nil, err
		}
		out = append(out, row)
	}
	return out, nil
}

func bqRowsToMaps(rows []map[string]bigquery.Value) []map[string]string {
	out := make([]map[string]string, 0, len(rows))
	for _, row := range rows {
		m := make(map[string]string, len(row))
		for k, v := range row {
			m[k] = bqValueToString(v)
		}
		out = append(out, m)
	}
	return out
}

func bqValueToString(v bigquery.Value) string {
	switch t := v.(type) {
	case nil:
		return ""
	case string:
		return t
	case int64:
		return strconv.FormatInt(t, 10)
	case float64:
		return strconv.FormatFloat(t, 'f', -1, 64)
	case bool:
		if t {
			return "true"
		}
		return "false"
	default:
		return fmt.Sprint(t)
	}
}

var _ Target = (*BigQueryTarget)(nil)
