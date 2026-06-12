package runner

import (
	"context"
	"errors"
	"fmt"
	"net/http"
	"strconv"
	"strings"
	"time"

	"cloud.google.com/go/bigquery"
	"google.golang.org/api/googleapi"
	"google.golang.org/api/iterator"
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
		var execOnly time.Duration
		var bytesProcessed int64
		if status != nil && status.Statistics != nil {
			st := status.Statistics.EndTime.Sub(status.Statistics.StartTime)
			execOnly = st
			bytesProcessed = status.Statistics.TotalBytesProcessed
		}
		it, err := job.Read(ctx)
		if err != nil {
			return QueryResult{Error: err.Error(), ExecutionOnly: execOnly}, err
		}
		rows, err := readAllRows(it)
		if err != nil {
			return QueryResult{Error: err.Error(), ExecutionOnly: execOnly}, err
		}
		maps := bqRowsToMaps(rows)
		hash, _ := HashRows(maps)
		return QueryResult{
			ExecutionOnly:  execOnly,
			BytesProcessed: bytesProcessed,
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
	q := t.client.Query(sql)
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
