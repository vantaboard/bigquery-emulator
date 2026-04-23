package server

import (
	"context"
	"encoding/csv"
	"errors"
	"fmt"
	"io"
	"os"

	"github.com/goccy/go-json"
	"github.com/parquet-go/parquet-go"
	"github.com/vantaboard/bigquery-emulator/internal/connection"
	"github.com/vantaboard/bigquery-emulator/types"
	bigqueryv2 "google.golang.org/api/bigquery/v2"
)

const (
	// maxCSVSchemaProbeRows caps how many CSV data rows we buffer only for autodetect inference.
	maxCSVSchemaProbeRows = 8192
	// loadInsertBatchRows bounds rows held in memory before flushing to the content repository.
	loadInsertBatchRows = 384
)

// spoolUploadPayloadToTempFile copies the entire upload body to a temporary file and seeks to the start.
// Call cleanup() once finished to remove the file (typically defer cleanup()).
func spoolUploadPayloadToTempFile(src io.Reader) (f *os.File, cleanup func(), err error) {
	tmp, err := os.CreateTemp("", "bq-emulator-load-*")
	if err != nil {
		return nil, nil, err
	}
	path := tmp.Name()
	if _, err := io.Copy(tmp, src); err != nil {
		_ = tmp.Close()
		_ = os.Remove(path)
		return nil, nil, fmt.Errorf("spool upload payload: %w", err)
	}
	if _, err := tmp.Seek(0, 0); err != nil {
		_ = tmp.Close()
		_ = os.Remove(path)
		return nil, nil, fmt.Errorf("rewind spooled payload: %w", err)
	}
	cleanup = func() {
		_ = tmp.Close()
		_ = os.Remove(path)
	}
	return tmp, cleanup, nil
}

func (h *uploadContentHandler) probeCSVForAutodetect(f *os.File, load *bigqueryv2.JobConfigurationLoad) (*bigqueryv2.TableSchema, error) {
	if _, err := f.Seek(0, 0); err != nil {
		return nil, err
	}
	r := csv.NewReader(f)
	header, err := r.Read()
	if err != nil {
		return nil, fmt.Errorf("failed to read csv header for autodetect: %w", err)
	}
	var rows [][]string
	for len(rows) < maxCSVSchemaProbeRows {
		rec, err := r.Read()
		if errors.Is(err, io.EOF) {
			break
		}
		if err != nil {
			return nil, fmt.Errorf("failed to read csv for autodetect: %w", err)
		}
		rows = append(rows, rec)
	}
	records := append([][]string{header}, rows...)
	return h.detectSchema(records, load.SkipLeadingRows)
}

func (h *uploadContentHandler) streamCSVIntoTable(
	ctx context.Context,
	tx *connection.Tx,
	r *uploadContentRequest,
	f *os.File,
	load *bigqueryv2.JobConfigurationLoad,
	tableContent *bigqueryv2.Table,
	tableRef *bigqueryv2.TableReference,
) error {
	columnToType := map[string]types.Type{}
	for _, field := range tableContent.Schema.Fields {
		columnToType[field.Name] = types.Type(field.Type)
	}

	if _, err := f.Seek(0, 0); err != nil {
		return err
	}
	csvR := csv.NewReader(f)
	header, err := csvR.Read()
	if err != nil {
		return fmt.Errorf("failed to read csv header: %w", err)
	}

	var columns []*types.Column
	var ignoreHeader bool
	for _, col := range header {
		if _, exists := columnToType[col]; !exists {
			ignoreHeader = true
			break
		}
		columns = append(columns, &types.Column{
			Name: col,
			Type: columnToType[col],
		})
	}
	if ignoreHeader {
		columns = []*types.Column{}
		for _, field := range tableContent.Schema.Fields {
			columns = append(columns, &types.Column{
				Name: field.Name,
				Type: types.Type(field.Type),
			})
		}
	}

	skipDataRows := 0
	if load.SkipLeadingRows > 1 {
		skipDataRows = int(load.SkipLeadingRows) - 1
	}
	skipped := 0

	firstOverwrite := load.WriteDisposition == "WRITE_TRUNCATE"
	batch := make(types.Data, 0, loadInsertBatchRows)

	flush := func(data types.Data) error {
		if len(data) == 0 {
			return nil
		}
		tableDef := &types.Table{
			ID:      tableRef.TableId,
			Columns: columns,
			Data:    data,
		}
		err := r.server.contentRepo.AddTableData(ctx, tx, tableRef.ProjectId, tableRef.DatasetId, tableDef, firstOverwrite)
		firstOverwrite = false
		return err
	}

	for {
		record, err := csvR.Read()
		if errors.Is(err, io.EOF) {
			break
		}
		if err != nil {
			return fmt.Errorf("failed to read csv row: %w", err)
		}
		if skipped < skipDataRows {
			skipped++
			continue
		}

		rowData := map[string]interface{}{}
		if len(record) != len(columns) {
			return fmt.Errorf("invalid column number: found broken row data: %v", record)
		}
		for i := 0; i < len(record); i++ {
			colData := record[i]
			converted, err := convertCSVValue(colData, columns[i].Type)
			if err != nil {
				return fmt.Errorf("failed to convert value %q for column %s: %w", colData, columns[i].Name, err)
			}
			rowData[columns[i].Name] = converted
		}
		batch = append(batch, rowData)
		if len(batch) >= loadInsertBatchRows {
			if err := flush(batch); err != nil {
				return err
			}
			batch = batch[:0]
		}
	}
	return flush(batch)
}

func (h *uploadContentHandler) streamParquetIntoTable(
	ctx context.Context,
	tx *connection.Tx,
	r *uploadContentRequest,
	f *os.File,
	load *bigqueryv2.JobConfigurationLoad,
	tableRef *bigqueryv2.TableReference,
) error {
	if _, err := f.Seek(0, 0); err != nil {
		return err
	}
	reader := parquet.NewReader(f)
	defer reader.Close()

	columns := []*types.Column{}
	for _, sf := range load.Schema.Fields {
		columns = append(columns, &types.Column{
			Name: sf.Name,
			Type: types.Type(sf.Type),
		})
	}

	firstOverwrite := load.WriteDisposition == "WRITE_TRUNCATE"
	batch := make(types.Data, 0, loadInsertBatchRows)

	flush := func(data types.Data) error {
		if len(data) == 0 {
			return nil
		}
		tableDef := &types.Table{
			ID:      tableRef.TableId,
			Columns: columns,
			Data:    data,
		}
		err := r.server.contentRepo.AddTableData(ctx, tx, tableRef.ProjectId, tableRef.DatasetId, tableDef, firstOverwrite)
		firstOverwrite = false
		return err
	}

	for i := 0; i < int(reader.NumRows()); i++ {
		var rowData interface{}
		if err := reader.Read(&rowData); err != nil {
			return err
		}
		batch = append(batch, rowData.(map[string]interface{}))
		if len(batch) >= loadInsertBatchRows {
			if err := flush(batch); err != nil {
				return err
			}
			batch = batch[:0]
		}
	}
	return flush(batch)
}

func (h *uploadContentHandler) streamNDJSONIntoTable(
	ctx context.Context,
	tx *connection.Tx,
	r *uploadContentRequest,
	f *os.File,
	tableContent *bigqueryv2.Table,
	tableRef *bigqueryv2.TableReference,
	load *bigqueryv2.JobConfigurationLoad,
) error {
	columns := []*types.Column{}
	for _, field := range tableContent.Schema.Fields {
		columns = append(columns, &types.Column{
			Name: field.Name,
			Type: types.Type(field.Type),
		})
	}
	columnMap := map[string]*types.Column{}
	for _, col := range columns {
		columnMap[col.Name] = col
	}

	if _, err := f.Seek(0, 0); err != nil {
		return err
	}
	decoder := json.NewDecoder(f)
	decoder.UseNumber()

	firstOverwrite := load.WriteDisposition == "WRITE_TRUNCATE"
	batch := make(types.Data, 0, loadInsertBatchRows)

	flush := func(data types.Data) error {
		if len(data) == 0 {
			return nil
		}
		tableDef := &types.Table{
			ID:      tableRef.TableId,
			Columns: columns,
			Data:    data,
		}
		err := r.server.contentRepo.AddTableData(ctx, tx, tableRef.ProjectId, tableRef.DatasetId, tableDef, firstOverwrite)
		firstOverwrite = false
		return err
	}

	recordNum := 0
	for decoder.More() {
		recordNum++
		d := make(map[string]interface{})
		if err := decoder.Decode(&d); err != nil {
			return fmt.Errorf(
				"decode NDJSON record %d (sourceFormat=%s, job_id=%s, destination=%s.%s): %w",
				recordNum,
				load.SourceFormat,
				r.job.ID,
				tableRef.DatasetId,
				tableRef.TableId,
				err,
			)
		}
		h.normalizeColumnNameForJSONData(columnMap, d)
		batch = append(batch, d)
		if len(batch) >= loadInsertBatchRows {
			if err := flush(batch); err != nil {
				return err
			}
			batch = batch[:0]
		}
	}
	return flush(batch)
}
