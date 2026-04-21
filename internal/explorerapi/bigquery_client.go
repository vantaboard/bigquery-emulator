package explorerapi

import (
	"context"
	"encoding/json"
	"errors"
	"fmt"
	"net/http"
	"os"
	"strings"
	"sync"
	"time"

	"cloud.google.com/go/bigquery"
	"cloud.google.com/go/civil"
	"github.com/gin-gonic/gin"
	"google.golang.org/api/iterator"
	"google.golang.org/api/option"
)

// ErrNoEmulatorProjects is returned when BIGQUERY_EMULATOR_HOST is set but the emulator reports no projects.
var ErrNoEmulatorProjects = errors.New("no BigQuery projects in emulator")

// BigQueryClient handles interactions with BigQuery for the explorer API.
type BigQueryClient struct {
	client       *bigquery.Client
	projectID    string
	emulatorHost string
	httpClient   *http.Client

	mu                sync.Mutex
	defaultClientOnce sync.Once
	defaultClientErr  error
	extraClients      map[string]*bigquery.Client
}

func gcpProjectIDFromEnv() string {
	if id := strings.TrimSpace(os.Getenv("GOOGLE_CLOUD_PROJECT")); id != "" {
		return id
	}
	return strings.TrimSpace(os.Getenv("GCLOUD_PROJECT"))
}

func writeExplorerClientError(c *gin.Context, err error) {
	if errors.Is(err, ErrNoEmulatorProjects) {
		c.JSON(http.StatusNotFound, gin.H{"error": err.Error()})
		return
	}
	c.JSON(http.StatusInternalServerError, gin.H{"error": err.Error()})
}

// newBigQueryClientCore creates a BigQuery client for projectID using the same rules as NewBigQueryClient.
func newBigQueryClientCore(ctx context.Context, projectID, emulatorHost string) (*bigquery.Client, error) {
	if emulatorHost != "" {
		endpoint := fmt.Sprintf("http://%s/bigquery/v2/", emulatorHost)
		return bigquery.NewClient(ctx, projectID,
			option.WithEndpoint(endpoint),
			option.WithoutAuthentication(),
		)
	}
	credentialsPath := os.Getenv("GOOGLE_APPLICATION_CREDENTIALS")
	if credentialsPath != "" {
		return bigquery.NewClient(ctx, projectID, option.WithCredentialsFile(credentialsPath))
	}
	return bigquery.NewClient(ctx, projectID)
}

// NewBigQueryClient creates a new BigQuery client.
// When BIGQUERY_EMULATOR_HOST is set, the default *bigquery.Client is created lazily on first use
// so project discovery can run after the HTTP server is listening. When the host is not set,
// GOOGLE_CLOUD_PROJECT or GCLOUD_PROJECT must identify the GCP project.
func NewBigQueryClient() (*BigQueryClient, error) {
	ctx := context.Background()
	emulatorHost := NormalizeEmulatorHost(os.Getenv("BIGQUERY_EMULATOR_HOST"))
	httpClient := &http.Client{Timeout: 15 * time.Second}

	if emulatorHost != "" {
		return &BigQueryClient{
			emulatorHost: emulatorHost,
			httpClient:   httpClient,
			extraClients: make(map[string]*bigquery.Client),
		}, nil
	}

	projectID := gcpProjectIDFromEnv()
	if projectID == "" {
		return nil, fmt.Errorf("explorer API: set GOOGLE_CLOUD_PROJECT or GCLOUD_PROJECT when BIGQUERY_EMULATOR_HOST is not set")
	}

	client, err := newBigQueryClientCore(ctx, projectID, "")
	if err != nil {
		return nil, fmt.Errorf("failed to create BigQuery client: %v", err)
	}

	return &BigQueryClient{
		client:       client,
		projectID:    projectID,
		emulatorHost: emulatorHost,
		httpClient:   httpClient,
		extraClients: make(map[string]*bigquery.Client),
	}, nil
}

// ensureDefaultClient resolves the default project and client when using the emulator (lazy init).
func (bq *BigQueryClient) ensureDefaultClient(ctx context.Context) error {
	if bq.emulatorHost == "" {
		return nil
	}
	bq.defaultClientOnce.Do(func() {
		ids, err := FetchProjectIDsFromEmulator(ctx, bq.httpClient, bq.emulatorHost)
		if err != nil {
			bq.defaultClientErr = fmt.Errorf("discover emulator projects: %w", err)
			return
		}
		if len(ids) == 0 {
			bq.defaultClientErr = ErrNoEmulatorProjects
			return
		}
		pid := ids[0]
		c, err := newBigQueryClientCore(ctx, pid, bq.emulatorHost)
		if err != nil {
			bq.defaultClientErr = fmt.Errorf("failed to create BigQuery client: %v", err)
			return
		}
		bq.client = c
		bq.projectID = pid
	})
	return bq.defaultClientErr
}

// clientForProject returns a client whose project ID matches the BigQuery jobs.insert path.
// projectID may be empty; the default project is resolved via ensureDefaultClient when using the emulator.
func (bq *BigQueryClient) clientForProject(ctx context.Context, projectID string) (*bigquery.Client, error) {
	pid := strings.TrimSpace(projectID)
	if pid == "" {
		if err := bq.ensureDefaultClient(ctx); err != nil {
			return nil, err
		}
		pid = bq.projectID
	}
	if pid == bq.projectID {
		if err := bq.ensureDefaultClient(ctx); err != nil {
			return nil, err
		}
		return bq.client, nil
	}

	bq.mu.Lock()
	defer bq.mu.Unlock()
	if c, ok := bq.extraClients[pid]; ok {
		return c, nil
	}
	c, err := newBigQueryClientCore(ctx, pid, bq.emulatorHost)
	if err != nil {
		return nil, err
	}
	bq.extraClients[pid] = c
	return c, nil
}

// GetProjects lists available projects (multi-project when using a compatible emulator).
func (bq *BigQueryClient) GetProjects(c *gin.Context) {
	ctx := c.Request.Context()
	if bq.emulatorHost != "" {
		ids, err := FetchProjectIDsFromEmulator(ctx, bq.httpClient, bq.emulatorHost)
		if err != nil {
			c.JSON(http.StatusServiceUnavailable, gin.H{"error": err.Error()})
			return
		}
		if len(ids) == 0 {
			c.JSON(http.StatusNotFound, gin.H{"error": ErrNoEmulatorProjects.Error()})
			return
		}
		ids = ApplyProjectIDListEnv(ids, bq.projectID)
		if len(ids) == 0 {
			c.JSON(http.StatusNotFound, gin.H{"error": "no BigQuery projects match BIGQUERY_PROJECT_IDS filter"})
			return
		}
		c.JSON(http.StatusOK, ids)
		return
	}

	if err := bq.ensureDefaultClient(ctx); err != nil {
		writeExplorerClientError(c, err)
		return
	}
	ids := ApplyProjectIDListEnv([]string{bq.projectID}, bq.projectID)
	if len(ids) == 0 {
		c.JSON(http.StatusNotFound, gin.H{"error": "no BigQuery project configured"})
		return
	}
	c.JSON(http.StatusOK, ids)
}

// GetConfig exposes UI feature flags (e.g. emulator admin API).
func (bq *BigQueryClient) GetConfig(c *gin.Context) {
	allow := os.Getenv("ALLOW_EMULATOR_PROJECT_ADMIN") == "true" && bq.emulatorHost != ""
	c.JSON(http.StatusOK, gin.H{"allowEmulatorProjectAdmin": allow})
}

// CreateEmulatorProject proxies POST to the emulator's /emulator/v1/projects (Vantaboard fork).
func (bq *BigQueryClient) CreateEmulatorProject(c *gin.Context) {
	if os.Getenv("ALLOW_EMULATOR_PROJECT_ADMIN") != "true" || bq.emulatorHost == "" {
		c.JSON(http.StatusForbidden, gin.H{"error": "emulator project admin is disabled"})
		return
	}
	var body struct {
		ID string `json:"id"`
	}
	if err := c.ShouldBindJSON(&body); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{"error": err.Error()})
		return
	}
	id := strings.TrimSpace(body.ID)
	if !ValidateProjectIDForPath(id) {
		c.JSON(http.StatusBadRequest, gin.H{"error": "invalid project id"})
		return
	}
	if err := PostEmulatorCreateProject(c.Request.Context(), bq.httpClient, bq.emulatorHost, id); err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": err.Error()})
		return
	}
	c.JSON(http.StatusCreated, gin.H{"id": id})
}

// GetDatasets lists datasets in a project.
func (bq *BigQueryClient) GetDatasets(c *gin.Context) {
	projectID := c.Param("project_id")
	ctx := context.Background()

	if err := bq.ensureDefaultClient(ctx); err != nil {
		writeExplorerClientError(c, err)
		return
	}

	datasets := []string{}
	it := bq.client.Datasets(ctx)
	it.ProjectID = projectID

	for {
		dataset, err := it.Next()
		if err == iterator.Done {
			break
		}
		if err != nil {
			c.JSON(http.StatusInternalServerError, gin.H{"error": err.Error()})
			return
		}
		datasets = append(datasets, dataset.DatasetID)
	}

	c.JSON(http.StatusOK, datasets)
}

// GetTables lists tables in a dataset.
func (bq *BigQueryClient) GetTables(c *gin.Context) {
	projectID := c.Param("project_id")
	datasetID := c.Param("dataset_id")
	ctx := context.Background()

	if err := bq.ensureDefaultClient(ctx); err != nil {
		writeExplorerClientError(c, err)
		return
	}

	var dataset *bigquery.Dataset
	if projectID != "" && projectID != bq.projectID {
		dataset = bq.client.DatasetInProject(projectID, datasetID)
	} else {
		dataset = bq.client.Dataset(datasetID)
	}

	tables := []string{}
	it := dataset.Tables(ctx)
	for {
		table, err := it.Next()
		if err == iterator.Done {
			break
		}
		if err != nil {
			c.JSON(http.StatusInternalServerError, gin.H{"error": err.Error()})
			return
		}
		tables = append(tables, table.TableID)
	}

	c.JSON(http.StatusOK, tables)
}

// TableSchema represents a BigQuery table schema field.
type TableSchema struct {
	Name        string  `json:"name"`
	Type        string  `json:"type"`
	Mode        string  `json:"mode"`
	Description *string `json:"description"`
}

// TableMetadata represents the table metadata along with its schema.
type TableMetadata struct {
	Schema             []TableSchema `json:"schema"`
	NumRows            uint64        `json:"numRows"`
	NumBytes           int64         `json:"numBytes"`
	CreationTime       string        `json:"creationTime"`
	LastModified       string        `json:"lastModified"`
	Description        string        `json:"description"`
	Type               string        `json:"type"`
	Location           string        `json:"location"`
	FullyQualifiedName string        `json:"fullyQualifiedName"`
}

// GetTableSchema returns a table's schema.
func (bq *BigQueryClient) GetTableSchema(c *gin.Context) {
	projectID := c.Param("project_id")
	datasetID := c.Param("dataset_id")
	tableID := c.Param("table_id")
	ctx := context.Background()

	if err := bq.ensureDefaultClient(ctx); err != nil {
		writeExplorerClientError(c, err)
		return
	}

	var tableRef *bigquery.Table
	if projectID != "" && projectID != bq.projectID {
		tableRef = bq.client.DatasetInProject(projectID, datasetID).Table(tableID)
	} else {
		tableRef = bq.client.Dataset(datasetID).Table(tableID)
	}

	metadata, err := tableRef.Metadata(ctx)
	if err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": err.Error()})
		return
	}

	schema := make([]TableSchema, len(metadata.Schema))
	for i, field := range metadata.Schema {
		mode := "NULLABLE"
		if field.Required {
			mode = "REQUIRED"
		} else if field.Repeated {
			mode = "REPEATED"
		}
		schema[i] = TableSchema{
			Name:        field.Name,
			Type:        string(field.Type),
			Mode:        mode,
			Description: &field.Description,
		}
	}

	creationTime := metadata.CreationTime.Format(time.RFC3339)
	lastModified := metadata.LastModifiedTime.Format(time.RFC3339)

	tableMetadata := TableMetadata{
		Schema:             schema,
		NumRows:            metadata.NumRows,
		NumBytes:           metadata.NumBytes,
		CreationTime:       creationTime,
		LastModified:       lastModified,
		Description:        metadata.Description,
		Type:               string(metadata.Type),
		Location:           metadata.Location,
		FullyQualifiedName: fmt.Sprintf("%s.%s.%s", projectID, datasetID, tableID),
	}

	c.JSON(http.StatusOK, tableMetadata)
}

// QueryRequest represents a query request body.
type QueryRequest struct {
	Query     string `json:"query" binding:"required"`
	ProjectID string `json:"project_id"`
}

// QueryResponse represents a query response.
type QueryResponse struct {
	Columns   []string  `json:"columns"`
	Rows      []gin.H   `json:"rows"`
	TotalRows int       `json:"total_rows"`
}

// RunQuery executes a BigQuery query and returns the results.
func (bq *BigQueryClient) RunQuery(c *gin.Context) {
	var request QueryRequest
	if err := c.ShouldBindJSON(&request); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{"error": err.Error()})
		return
	}

	ctx := context.Background()
	client, err := bq.clientForProject(ctx, request.ProjectID)
	if err != nil {
		writeExplorerClientError(c, fmt.Errorf("BigQuery client: %w", err))
		return
	}
	it, err := client.Query(request.Query).Read(ctx)
	if err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": fmt.Sprintf("Query execution error: %v", err)})
		return
	}

	var firstRow map[string]bigquery.Value = make(map[string]bigquery.Value)
	err = it.Next(&firstRow)
	if err == iterator.Done {
		c.JSON(http.StatusOK, QueryResponse{
			Columns:   []string{},
			Rows:      []gin.H{},
			TotalRows: 0,
		})
		return
	} else if err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": fmt.Sprintf("Error reading row: %v", err)})
		return
	}

	schema := it.Schema
	columnNames := make([]string, len(schema))
	for i, field := range schema {
		columnNames[i] = field.Name
	}

	rows := []gin.H{}
	{
		record := gin.H{}
		for _, col := range columnNames {
			record[col] = formatBigQueryValue(firstRow[col])
		}
		rows = append(rows, record)
	}

	for {
		row := make(map[string]bigquery.Value)
		if err := it.Next(&row); err == iterator.Done {
			break
		} else if err != nil {
			c.JSON(http.StatusInternalServerError, gin.H{"error": fmt.Sprintf("Error reading rows: %v", err)})
			return
		}
		record := gin.H{}
		for _, col := range columnNames {
			record[col] = formatBigQueryValue(row[col])
		}
		rows = append(rows, record)
	}

	response := QueryResponse{
		Columns:   columnNames,
		Rows:      rows,
		TotalRows: len(rows),
	}

	c.JSON(http.StatusOK, response)
}

// formatBigQueryValue ensures BigQuery values are properly formatted for JSON.
func formatBigQueryValue(v interface{}) interface{} {
	if v == nil {
		return nil
	}

	switch value := v.(type) {
	case time.Time:
		return value.Format(time.RFC3339Nano)
	case []byte:
		return string(value)
	case map[string]bigquery.Value:
		result := make(map[string]interface{})
		for k, v := range value {
			result[k] = formatBigQueryValue(v)
		}
		return result
	case []bigquery.Value:
		result := make([]interface{}, len(value))
		for i, v := range value {
			result[i] = formatBigQueryValue(v)
		}
		return result
	case bigquery.NullInt64:
		if value.Valid {
			return value.Int64
		}
		return nil
	case bigquery.NullFloat64:
		if value.Valid {
			return value.Float64
		}
		return nil
	case bigquery.NullBool:
		if value.Valid {
			return value.Bool
		}
		return nil
	case bigquery.NullString:
		if value.Valid {
			return value.StringVal
		}
		return nil
	case bigquery.NullTimestamp:
		if value.Valid {
			return value.Timestamp.Format(time.RFC3339Nano)
		}
		return nil
	case bigquery.NullDate:
		if value.Valid {
			return value.Date.String()
		}
		return nil
	case bigquery.NullGeography:
		if value.Valid {
			return value.GeographyVal
		}
		return nil
	case bigquery.NullJSON:
		if value.Valid {
			var jsonValue interface{}
			if err := json.Unmarshal([]byte(value.JSONVal), &jsonValue); err == nil {
				return jsonValue
			}
			return string(value.JSONVal)
		}
		return nil
	case civil.Date:
		return value.String()
	case civil.Time:
		return value.String()
	case civil.DateTime:
		return value.String()
	default:
		return value
	}
}

// Close closes the BigQuery client.
func (bq *BigQueryClient) Close() error {
	var errs []error
	if bq.client != nil {
		if err := bq.client.Close(); err != nil {
			errs = append(errs, err)
		}
	}
	bq.mu.Lock()
	for _, c := range bq.extraClients {
		if err := c.Close(); err != nil {
			errs = append(errs, err)
		}
	}
	bq.extraClients = nil
	bq.mu.Unlock()
	return errors.Join(errs...)
}
