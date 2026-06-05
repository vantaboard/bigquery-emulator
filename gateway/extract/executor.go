// Package extract implements synchronous BigQuery EXTRACT jobs.
package extract

import (
	"bytes"
	"compress/gzip"
	"context"
	"encoding/csv"
	"encoding/json"
	"errors"
	"fmt"
	"strconv"
	"strings"

	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
	"github.com/vantaboard/bigquery-emulator/gateway/jobs"
	"github.com/vantaboard/bigquery-emulator/gateway/load"
)

const listPageSize = 10_000

// Result captures extract-job statistics.
type Result struct {
	InputBytes               int64
	DestinationURIFileCounts []int64
}

// Execute runs a synchronous EXTRACT job.
func Execute(ctx context.Context, catalog enginepb.CatalogClient, cfg *jobs.JobConfigurationExtract,
	defaultProject string,
) (Result, error) {
	if cfg == nil {
		return Result{}, errors.New("extract configuration is required")
	}
	if cfg.SourceTable == nil || cfg.SourceTable.TableID == "" {
		return Result{}, errors.New("sourceTable.tableId is required")
	}
	if len(cfg.DestinationURIs) == 0 {
		return Result{}, errors.New("destinationUris is required")
	}

	projectID := cfg.SourceTable.ProjectID
	if projectID == "" {
		projectID = defaultProject
	}
	datasetID := cfg.SourceTable.DatasetID
	tableID := cfg.SourceTable.TableID

	ref := &enginepb.TableRef{
		ProjectId: projectID,
		DatasetId: datasetID,
		TableId:   tableID,
	}
	desc, err := catalog.DescribeTable(ctx, &enginepb.DescribeTableRequest{Table: ref})
	if err != nil {
		return Result{}, fmt.Errorf("source table: %w", err)
	}
	schema := desc.GetSchema()
	rows, err := listAllRows(ctx, catalog, ref)
	if err != nil {
		return Result{}, err
	}

	format := cfg.DestinationFormat
	if format == "" {
		format = "CSV"
	}
	payload, contentType, err := serializeRows(schema, rows, format)
	if err != nil {
		return Result{}, err
	}
	payload, contentType, err = maybeGzip(cfg.Compression, payload, contentType)
	if err != nil {
		return Result{}, err
	}
	counts, err := uploadDestinations(ctx, cfg.DestinationURIs, contentType, payload)
	if err != nil {
		return Result{}, err
	}
	return Result{
		InputBytes:               int64(len(payload)),
		DestinationURIFileCounts: counts,
	}, nil
}

func maybeGzip(compression string, payload []byte, contentType string) ([]byte, string, error) {
	if !strings.EqualFold(compression, "GZIP") {
		return payload, contentType, nil
	}
	out, err := gzipBytes(payload)
	if err != nil {
		return nil, "", err
	}
	return out, "application/gzip", nil
}

func uploadDestinations(ctx context.Context, uris []string, contentType string, payload []byte) ([]int64, error) {
	counts := make([]int64, len(uris))
	for i, uri := range uris {
		if err := load.PutGCS(ctx, uri, contentType, payload); err != nil {
			return nil, err
		}
		counts[i] = 1
	}
	return counts, nil
}

func listAllRows(ctx context.Context, catalog enginepb.CatalogClient, ref *enginepb.TableRef,
) ([]*enginepb.DataRow, error) {
	var out []*enginepb.DataRow
	start := int64(0)
	for {
		resp, err := catalog.ListRows(ctx, &enginepb.ListRowsRequest{
			Table:      ref,
			StartIndex: start,
			MaxResults: listPageSize,
		})
		if err != nil {
			return nil, err
		}
		rows := resp.GetRows()
		if len(rows) == 0 {
			break
		}
		out = append(out, rows...)
		start += int64(len(rows))
		if start >= resp.GetTotalRows() {
			break
		}
	}
	return out, nil
}

func serializeRows(schema *enginepb.TableSchema, rows []*enginepb.DataRow, format string) ([]byte, string, error) {
	switch strings.ToUpper(format) {
	case "CSV":
		return serializeCSV(schema, rows)
	case "NEWLINE_DELIMITED_JSON":
		return serializeNDJSON(schema, rows)
	default:
		return nil, "", fmt.Errorf("unsupported destinationFormat %q", format)
	}
}

func serializeCSV(schema *enginepb.TableSchema, rows []*enginepb.DataRow) ([]byte, string, error) {
	var buf bytes.Buffer
	w := csv.NewWriter(&buf)
	fields := schema.GetFields()
	header := make([]string, len(fields))
	for i, f := range fields {
		header[i] = f.GetName()
	}
	if err := w.Write(header); err != nil {
		return nil, "", err
	}
	for _, row := range rows {
		record := make([]string, len(fields))
		cells := row.GetCells()
		for i := range fields {
			if i < len(cells) {
				record[i] = cellString(cells[i])
			}
		}
		if err := w.Write(record); err != nil {
			return nil, "", err
		}
	}
	w.Flush()
	if err := w.Error(); err != nil {
		return nil, "", err
	}
	return buf.Bytes(), "text/csv", nil
}

func serializeNDJSON(schema *enginepb.TableSchema, rows []*enginepb.DataRow) ([]byte, string, error) {
	var buf bytes.Buffer
	fields := schema.GetFields()
	for _, row := range rows {
		obj := make(map[string]any, len(fields))
		cells := row.GetCells()
		for i, f := range fields {
			if i < len(cells) {
				obj[f.GetName()] = cellJSONValue(cells[i], f.GetType())
			}
		}
		line, err := json.Marshal(obj)
		if err != nil {
			return nil, "", err
		}
		buf.Write(line)
		buf.WriteByte('\n')
	}
	return buf.Bytes(), "application/json", nil
}

func cellString(c *enginepb.Cell) string {
	if c == nil || c.GetNullValue() {
		return ""
	}
	return c.GetStringValue()
}

func cellJSONValue(c *enginepb.Cell, typ string) any {
	if c == nil || c.GetNullValue() {
		return nil
	}
	s := c.GetStringValue()
	switch strings.ToUpper(typ) {
	case "INTEGER", "INT64":
		if n, err := strconv.ParseInt(s, 10, 64); err == nil {
			return n
		}
	case "FLOAT", "FLOAT64", "NUMERIC", "BIGNUMERIC":
		if f, err := strconv.ParseFloat(s, 64); err == nil {
			return f
		}
	case "BOOLEAN", "BOOL":
		return s == "true"
	}
	return s
}

func gzipBytes(data []byte) ([]byte, error) {
	var buf bytes.Buffer
	zw := gzip.NewWriter(&buf)
	if _, err := zw.Write(data); err != nil {
		return nil, err
	}
	if err := zw.Close(); err != nil {
		return nil, err
	}
	return buf.Bytes(), nil
}

// FormatStatistics maps Result into jobs.ExtractStatistics.
func FormatStatistics(r Result) *jobs.ExtractStatistics {
	counts := make([]string, len(r.DestinationURIFileCounts))
	for i, c := range r.DestinationURIFileCounts {
		counts[i] = strconv.FormatInt(c, 10)
	}
	return &jobs.ExtractStatistics{
		DestinationURIFileCounts: counts,
		InputBytes:               strconv.FormatInt(r.InputBytes, 10),
	}
}
