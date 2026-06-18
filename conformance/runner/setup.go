package runner

import (
	"context"
	"encoding/json"
	"errors"
	"fmt"
	"strings"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
)

// runSetupStep dispatches one setup step to the matching helper.
// Errors bubble up unchanged; the caller wraps them with the step
// index for the diff message.
func runSetupStep(ctx context.Context, base string, step SetupStep, defaultDataset string) error {
	switch {
	case step.Dataset != "":
		return setupDataset(ctx, base, step.Dataset)
	case step.Table != nil:
		return setupTable(ctx, base, step.Table)
	case step.Rows != nil:
		return setupRows(ctx, base, step.Rows)
	case strings.TrimSpace(step.SQL) != "":
		return setupSQL(ctx, base, step.SQL, defaultDataset)
	case step.RowAccessPolicy != nil:
		return setupRowAccessPolicy(ctx, base, step.RowAccessPolicy)
	case step.ColumnGovernance != nil:
		return setupColumnGovernance(ctx, base, step.ColumnGovernance)
	default:
		return errors.New("empty setup step (validated at load time)")
	}
}

// setupDataset issues a `datasets.insert` for the synthesized
// fixture project / dataset pair. Location is hardcoded to US to
// match the gateway's default; fixtures that want a different
// location have to use a SQL setup step.
func setupDataset(ctx context.Context, base, dataset string) error {
	body := fmt.Sprintf(
		`{"datasetReference":{"projectId":"%s","datasetId":"%s"},"location":"US"}`,
		projectIDFromBase(base), dataset)
	status, respBody, err := doRequest(ctx, base+"/datasets", []byte(body))
	if err != nil {
		return err
	}
	if status < 200 || status >= 300 {
		return fmt.Errorf("datasets.insert -> %d: %s", status, snippet(respBody))
	}
	return nil
}

// setupTable issues a `tables.insert` with the fixture's column
// schema. STRUCT children round-trip through columnToTableField.
func setupTable(ctx context.Context, base string, t *TableSetup) error {
	tableBody := struct {
		TableReference bqtypes.TableReference `json:"tableReference"`
		Schema         *struct {
			Fields []bqtypes.TableFieldSchema `json:"fields"`
		} `json:"schema,omitempty"`
		ExternalDataConfiguration *bqtypes.ExternalDataConfiguration `json:"externalDataConfiguration,omitempty"`
		View                      *bqtypes.ViewDefinition            `json:"view,omitempty"`
	}{}
	tableBody.TableReference = bqtypes.TableReference{
		ProjectID: projectIDFromBase(base),
		DatasetID: t.Dataset,
		TableID:   t.ID,
	}
	if t.External != nil {
		tableBody.ExternalDataConfiguration = &bqtypes.ExternalDataConfiguration{
			SourceFormat: t.External.SourceFormat,
			SourceURIs:   append([]string(nil), t.External.SourceURIs...),
			Autodetect:   t.External.Autodetect,
		}
	}
	if t.View != nil {
		tableBody.View = &bqtypes.ViewDefinition{Query: t.View.Query}
	}
	if len(t.Schema) > 0 {
		tableBody.Schema = &struct {
			Fields []bqtypes.TableFieldSchema `json:"fields"`
		}{}
		for _, c := range t.Schema {
			tableBody.Schema.Fields = append(tableBody.Schema.Fields,
				columnToTableField(c))
		}
	}
	jsonBody, err := json.Marshal(tableBody)
	if err != nil {
		return fmt.Errorf("marshal table body: %w", err)
	}
	url := fmt.Sprintf("%s/datasets/%s/tables", base, t.Dataset)
	status, respBody, err := doRequest(ctx, url, jsonBody)
	if err != nil {
		return err
	}
	if status < 200 || status >= 300 {
		return fmt.Errorf("tables.insert -> %d: %s", status, snippet(respBody))
	}
	return nil
}

// setupRows issues a `tabledata.insertAll`. It is the only way to
// seed rows on the DuckDB engine today: INSERT VALUES returns
// UNIMPLEMENTED. The wire shape matches Google's REST API spec
// (each row is wrapped in `{json: {...}}`).
func setupRows(ctx context.Context, base string, rs *RowsSetup) error {
	type insertAllRow struct {
		JSON map[string]any `json:"json"`
	}
	body := struct {
		Kind string         `json:"kind"`
		Rows []insertAllRow `json:"rows"`
	}{
		Kind: "bigquery#tableDataInsertAllRequest",
		Rows: make([]insertAllRow, 0, len(rs.Rows)),
	}
	for _, r := range rs.Rows {
		body.Rows = append(body.Rows, insertAllRow{JSON: r})
	}
	jsonBody, err := json.Marshal(body)
	if err != nil {
		return fmt.Errorf("marshal insertAll body: %w", err)
	}
	url := fmt.Sprintf("%s/datasets/%s/tables/%s/insertAll",
		base, rs.Dataset, rs.Table)
	status, respBody, err := doRequest(ctx, url, jsonBody)
	if err != nil {
		return err
	}
	if status < 200 || status >= 300 {
		return fmt.Errorf("tabledata.insertAll -> %d: %s",
			status, snippet(respBody))
	}
	return nil
}

// setupSQL runs an arbitrary statement through the gateway's
// `/queries` endpoint. Used for setup phases that do not fit the
// dataset/table/rows shape (e.g. preparing a temp UDF).
func setupSQL(ctx context.Context, base, sql, defaultDataset string) error {
	queryBody, err := marshalJobsQueryBody(sql, defaultDataset)
	if err != nil {
		return err
	}
	status, respBody, err := doRequest(ctx, base+"/queries", queryBody)
	if err != nil {
		return err
	}
	if status < 200 || status >= 300 {
		return fmt.Errorf("setup sql -> %d: %s", status, snippet(respBody))
	}
	return nil
}

// columnToTableField copies our YAML-decoded SchemaColumn onto the
// `bqtypes.TableFieldSchema` wire shape, recursing for STRUCT
// children so nested fields round-trip cleanly.
func columnToTableField(c SchemaColumn) bqtypes.TableFieldSchema {
	out := bqtypes.TableFieldSchema{
		Name:        c.Name,
		Type:        c.Type,
		Mode:        c.Mode,
		Description: c.Description,
	}
	if len(c.PolicyTags) > 0 {
		out.PolicyTags = &bqtypes.PolicyTagList{Names: append([]string(nil), c.PolicyTags...)}
	}
	for _, f := range c.Fields {
		out.Fields = append(out.Fields, columnToTableField(f))
	}
	return out
}

func setupRowAccessPolicy(ctx context.Context, base string, rap *RowAccessPolicySetup) error {
	body := map[string]any{
		"rowAccessPolicyReference": map[string]string{
			"projectId": projectIDFromBase(base),
			"datasetId": rap.Dataset,
			"tableId":   rap.Table,
			"policyId":  rap.PolicyID,
		},
		"filterPredicate": rap.FilterPredicate,
	}
	if len(rap.Grantees) > 0 {
		body["grantees"] = rap.Grantees
	}
	jsonBody, err := json.Marshal(body)
	if err != nil {
		return fmt.Errorf("marshal row access policy: %w", err)
	}
	url := fmt.Sprintf("%s/datasets/%s/tables/%s/rowAccessPolicies", base, rap.Dataset, rap.Table)
	status, respBody, err := doRequest(ctx, url, jsonBody)
	if err != nil {
		return err
	}
	if status < 200 || status >= 300 {
		return fmt.Errorf("rowAccessPolicies.insert -> %d: %s", status, snippet(respBody))
	}
	return nil
}

func setupColumnGovernance(ctx context.Context, base string, cg *ColumnGovernanceSetup) error {
	field := map[string]any{
		"name":     cg.Column,
		"type":     "STRING",
		"maskKind": cg.MaskKind,
	}
	if cg.PolicyTag != "" {
		field["policyTags"] = map[string]any{"names": []string{cg.PolicyTag}}
	}
	patchBody := map[string]any{
		"schema": map[string]any{"fields": []map[string]any{field}},
	}
	jsonBody, err := json.Marshal(patchBody)
	if err != nil {
		return fmt.Errorf("marshal column governance patch: %w", err)
	}
	url := fmt.Sprintf("%s/datasets/%s/tables/%s", base, cg.Dataset, cg.Table)
	status, respBody, err := doPatchRequest(ctx, url, jsonBody)
	if err != nil {
		return err
	}
	if status < 200 || status >= 300 {
		return fmt.Errorf("tables.patch column governance -> %d: %s", status, snippet(respBody))
	}
	return nil
}

// projectIDFromBase pulls the projectId from a URL of the form
// .../bigquery/v2/projects/<projectId>. Returning the inner segment
// keeps the setup-step builders from having to thread projectId
// through their signatures.
func projectIDFromBase(base string) string {
	const marker = "/projects/"
	i := strings.LastIndex(base, marker)
	if i < 0 {
		return ""
	}
	return base[i+len(marker):]
}
