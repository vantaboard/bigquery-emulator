// differential-record captures production BigQuery output for the differential corpus.
package main

import (
	"context"
	"encoding/json"
	"errors"
	"flag"
	"fmt"
	"os"
	"os/signal"
	"path/filepath"
	"strconv"
	"strings"
	"syscall"
	"time"

	"cloud.google.com/go/bigquery"
	"github.com/vantaboard/bigquery-emulator/conformance/differential"
	"github.com/vantaboard/bigquery-emulator/conformance/runner"
	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
	"google.golang.org/api/iterator"
)

func main() {
	if err := run(); err != nil {
		_, _ = fmt.Fprintln(os.Stderr, "differential-record:", err)
		os.Exit(2)
	}
}

func run() error {
	fs := flag.NewFlagSet("differential-record", flag.ContinueOnError)
	fs.SetOutput(os.Stderr)
	corpus := fs.String("corpus", differential.DefaultCorpusDir, "corpus directory or single YAML")
	oracleDir := fs.String("oracle-dir", differential.DefaultOracleDir, "directory to write oracle JSON files")
	project := fs.String("project", "", "GCP project (default: BIGQUERY_DIFFERENTIAL_PROJECT)")
	dryRun := fs.Bool("dry-run", false, "print actions without writing oracle files")
	if err := fs.Parse(os.Args[1:]); err != nil {
		if errors.Is(err, flag.ErrHelp) {
			return nil
		}
		return err
	}

	projectID := strings.TrimSpace(*project)
	if projectID == "" {
		projectID = strings.TrimSpace(os.Getenv("BIGQUERY_DIFFERENTIAL_PROJECT"))
	}
	if projectID == "" {
		printSkipInstructions()
		return nil
	}

	cases, err := differential.LoadCorpusDir(*corpus, false)
	if err != nil {
		return err
	}

	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()
	sigCh := make(chan os.Signal, 1)
	signal.Notify(sigCh, os.Interrupt, syscall.SIGTERM)
	go func() {
		<-sigCh
		cancel()
	}()
	defer signal.Stop(sigCh)

	client, err := bigquery.NewClient(ctx, projectID)
	if err != nil {
		return fmt.Errorf("bigquery client: %w", err)
	}
	defer func() { _ = client.Close() }()

	for _, c := range cases {
		if err := recordCase(ctx, client, projectID, *oracleDir, c, *dryRun); err != nil {
			return fmt.Errorf("%s: %w", c.Name, err)
		}
	}
	return nil
}

func printSkipInstructions() {
	_, _ = fmt.Fprintln(os.Stderr, `differential-record: skipped — no GCP project configured.

Set BIGQUERY_DIFFERENTIAL_PROJECT to a project where you can create ephemeral
datasets, or pass --project=<id>. Application Default Credentials must be
available (gcloud auth application-default login).

The committed oracle JSON under conformance/differential/oracle/ is what CI
replays; recording is manual/opt-in. When GCP access is unavailable, pin
oracle expectations from bq query output and mark oracle_source: bq-cli in
the corpus YAML (see .cursor/rules/conformance-bq-validation.mdc).`)
}

func recordCase(
	ctx context.Context,
	client *bigquery.Client,
	project, oracleDir string,
	c *differential.CorpusCase,
	dryRun bool,
) error {
	dsID := fmt.Sprintf("diff_record_%s_%d", sanitizeDatasetID(c.Name), time.Now().Unix())
	ds := client.Dataset(dsID)
	if err := ds.Create(ctx, &bigquery.DatasetMetadata{Location: "US"}); err != nil {
		return fmt.Errorf("create dataset %s: %w", dsID, err)
	}
	defer func() { _ = ds.DeleteWithContents(ctx) }()

	if err := applySetup(ctx, client, project, dsID, c); err != nil {
		return err
	}
	return captureQueryOracle(ctx, client, project, dsID, oracleDir, c, dryRun)
}

func captureQueryOracle(
	ctx context.Context,
	client *bigquery.Client,
	project, dsID, oracleDir string,
	c *differential.CorpusCase,
	dryRun bool,
) error {
	q := client.Query(c.Query)
	q.DefaultProjectID = project
	q.DefaultDatasetID = dsID
	if c.DefaultDataset != "" {
		q.DefaultDatasetID = c.DefaultDataset
	}
	if len(c.QueryParameters) > 0 {
		q.Parameters = toBQParams(c.QueryParameters)
	}

	job, err := q.Run(ctx)
	if err != nil {
		return fmt.Errorf("run query: %w", err)
	}
	status, err := job.Wait(ctx)
	if err != nil {
		return fmt.Errorf("wait job: %w", err)
	}
	jobID := job.ID()

	if status.Err() != nil {
		o := &differential.Oracle{
			Project: project, OracleSource: "recorded", Match: c.Match,
			Success: false, JobID: jobID,
			Error: &differential.OracleError{Message: status.Err().Error()},
		}
		return writeOracle(oracleDir, c, o, dryRun)
	}

	it, err := job.Read(ctx)
	if err != nil {
		return fmt.Errorf("read results: %w", err)
	}
	schema, rows, err := readAllRows(it)
	if err != nil {
		return err
	}
	o := &differential.Oracle{
		Project: project, OracleSource: "recorded", Match: c.Match,
		Success: true, JobID: jobID, Schema: schema, Rows: rows,
		JobReference: &bqtypes.JobReference{
			ProjectID: project, JobID: jobID, Location: "US",
		},
	}
	return writeOracle(oracleDir, c, o, dryRun)
}

func writeOracle(oracleDir string, c *differential.CorpusCase, o *differential.Oracle, dryRun bool) error {
	path := filepath.Join(oracleDir, c.OracleRef)
	if dryRun {
		_, _ = fmt.Fprintf(os.Stderr, "would write oracle %s for %s\n", path, c.Name)
		return nil
	}
	if err := os.MkdirAll(oracleDir, 0o750); err != nil {
		return err
	}
	return differential.WriteOracle(path, o)
}

func applySetup(ctx context.Context, client *bigquery.Client, project, dsID string, c *differential.CorpusCase) error {
	for i, step := range c.Setup {
		switch {
		case step.Dataset != "":
			target := client.Dataset(step.Dataset)
			if err := target.Create(ctx, &bigquery.DatasetMetadata{Location: "US"}); err != nil {
				return fmt.Errorf("setup[%d] dataset %s: %w", i, step.Dataset, err)
			}
		case step.Table != nil:
			if err := createTable(ctx, client, step.Table); err != nil {
				return fmt.Errorf("setup[%d] table: %w", i, err)
			}
		case step.Rows != nil:
			if err := insertRows(ctx, client, step.Rows); err != nil {
				return fmt.Errorf("setup[%d] rows: %w", i, err)
			}
		case strings.TrimSpace(step.SQL) != "":
			q := client.Query(step.SQL)
			q.DefaultProjectID = project
			q.DefaultDatasetID = dsID
			job, err := q.Run(ctx)
			if err != nil {
				return fmt.Errorf("setup[%d] sql run: %w", i, err)
			}
			if st, err := job.Wait(ctx); err != nil {
				return fmt.Errorf("setup[%d] sql wait: %w", i, err)
			} else if st.Err() != nil {
				return fmt.Errorf("setup[%d] sql error: %w", i, st.Err())
			}
		default:
			return fmt.Errorf("setup[%d]: unsupported step for recorder", i)
		}
	}
	return nil
}

func createTable(ctx context.Context, client *bigquery.Client, t *runner.TableSetup) error {
	meta := &bigquery.TableMetadata{}
	if t.View != nil {
		meta.ViewQuery = t.View.Query
	} else if len(t.Schema) > 0 {
		meta.Schema = toBQSchema(t.Schema)
	}
	table := client.Dataset(t.Dataset).Table(t.ID)
	return table.Create(ctx, meta)
}

func insertRows(ctx context.Context, client *bigquery.Client, rs *runner.RowsSetup) error {
	inserter := client.Dataset(rs.Dataset).Table(rs.Table).Inserter()
	return inserter.Put(ctx, rs.Rows)
}

func toBQSchema(cols []runner.SchemaColumn) bigquery.Schema {
	out := make(bigquery.Schema, 0, len(cols))
	for _, c := range cols {
		fs := &bigquery.FieldSchema{
			Name: c.Name,
			Type: bigquery.FieldType(strings.ToUpper(c.Type)),
		}
		switch strings.ToUpper(c.Mode) {
		case "REQUIRED":
			fs.Required = true
		case "REPEATED":
			fs.Repeated = true
		}
		out = append(out, fs)
	}
	return out
}

func toBQParams(params []differential.QueryParameterYAML) []bigquery.QueryParameter {
	out := make([]bigquery.QueryParameter, 0, len(params))
	for _, p := range params {
		out = append(out, bigquery.QueryParameter{
			Name:  p.Name,
			Value: p.Value,
		})
	}
	return out
}

func readAllRows(it *bigquery.RowIterator) (*bqtypes.TableSchema, []bqtypes.Row, error) {
	schema := wireSchema(it.Schema)
	var rows []bqtypes.Row
	for {
		var vals []bigquery.Value
		err := it.Next(&vals)
		if errors.Is(err, iterator.Done) {
			break
		}
		if err != nil {
			return nil, nil, err
		}
		row := bqtypes.Row{F: make([]bqtypes.Cell, len(vals))}
		for i, v := range vals {
			row.F[i] = bqtypes.Cell{V: wireCell(v)}
		}
		rows = append(rows, row)
	}
	return schema, rows, nil
}

func wireSchema(s bigquery.Schema) *bqtypes.TableSchema {
	if s == nil {
		return nil
	}
	fields := make([]bqtypes.TableFieldSchema, len(s))
	for i, f := range s {
		mode := "NULLABLE"
		if f.Repeated {
			mode = "REPEATED"
		} else if f.Required {
			mode = "REQUIRED"
		}
		fields[i] = bqtypes.TableFieldSchema{
			Name: f.Name,
			Type: string(f.Type),
			Mode: mode,
		}
	}
	return &bqtypes.TableSchema{Fields: fields}
}

func wireCell(v bigquery.Value) any {
	if v == nil {
		return nil
	}
	switch x := v.(type) {
	case string:
		return x
	case int64:
		return strconv.FormatInt(x, 10)
	case float64:
		return fmt.Sprintf("%g", x)
	case bool:
		if x {
			return "true"
		}
		return "false"
	default:
		b, _ := json.Marshal(v)
		return string(b)
	}
}

func sanitizeDatasetID(name string) string {
	var b strings.Builder
	for _, r := range strings.ToLower(name) {
		if (r >= 'a' && r <= 'z') || (r >= '0' && r <= '9') || r == '_' {
			b.WriteRune(r)
		} else {
			b.WriteRune('_')
		}
	}
	out := b.String()
	if len(out) > 40 {
		out = out[:40]
	}
	return out
}
