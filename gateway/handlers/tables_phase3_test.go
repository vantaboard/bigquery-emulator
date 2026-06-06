package handlers

import (
	"context"
	"encoding/json"
	"io"
	"net/http"
	"net/http/httptest"
	"os"
	"path/filepath"
	"strings"
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
	"github.com/vantaboard/bigquery-emulator/gateway/jobs"
)

func TestJobInsertLoadPersistsCMEKMetadata(t *testing.T) {
	t.Parallel()
	dir := t.TempDir()
	csvPath := filepath.Join(dir, "data.csv")
	if err := os.WriteFile(csvPath, []byte("x\na\n"), 0o644); err != nil {
		t.Fatal(err)
	}
	kmsKey := bqtypes.EmulatorCMEKKeyUSCentral("dev", "test")
	cat := &fakeCatalogClient{}
	meta := NewMetadataStore()
	reg := jobs.NewRegistry()
	deps := Dependencies{Catalog: cat, Jobs: reg, Metadata: meta}
	body := `{"configuration":{"load":{"sourceUris":["file://` + csvPath + `"],` +
		`"destinationTable":{"projectId":"dev","datasetId":"ds","tableId":"t"},` +
		`"sourceFormat":"CSV","schema":{"fields":[{"name":"x","type":"` + sqlTypeSTRING + `"}]},` +
		`"destinationEncryptionConfiguration":{"kmsKeyName":"` + kmsKey + `"}}}}`

	rec := runJobInsert(t, deps, body)
	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d, body=%s", rec.Code, rec.Body.String())
	}

	req := httptest.NewRequest(http.MethodGet,
		"http://example/bigquery/v2/projects/dev/datasets/ds/tables/t", nil)
	req.SetPathValue("projectId", "dev")
	req.SetPathValue("datasetId", "ds")
	req.SetPathValue("tableId", "t")
	recGet := httptest.NewRecorder()
	TableGet(deps)(recGet, req)
	if recGet.Code != http.StatusOK {
		t.Fatalf("TableGet -> %d: %s", recGet.Code, recGet.Body.String())
	}
	var got bqtypes.Table
	if err := json.Unmarshal(recGet.Body.Bytes(), &got); err != nil {
		t.Fatalf("decode: %v", err)
	}
	if got.EncryptionConfiguration == nil || got.EncryptionConfiguration.KMSKeyName != kmsKey {
		t.Fatalf("encryptionConfiguration = %#v", got.EncryptionConfiguration)
	}
}

func TestTableUpdatePolicyTagsRoundTrip(t *testing.T) {
	t.Parallel()
	tag := "projects/p/location/us/taxonomies/t/policyTags/pt"
	meta := NewMetadataStore()
	deps := Dependencies{
		Catalog: &fakeCatalogClient{
			describeTableFn: func(_ context.Context, _ *enginepb.DescribeTableRequest) (*enginepb.DescribeTableResponse, error) {
				return &enginepb.DescribeTableResponse{Schema: &enginepb.TableSchema{Fields: []*enginepb.FieldSchema{
					{Name: "Name", Type: sqlTypeSTRING},
					{Name: "Age", Type: sqlTypeINTEGER},
				}}}, nil
			},
		},
		Metadata: meta,
	}
	patchBody := `{"schema":{"fields":[` +
		`{"name":"Name","type":"` + sqlTypeSTRING + `"},` +
		`{"name":"Age","type":"` + sqlTypeINTEGER + `","policyTags":{"names":["` + tag + `"]}}` +
		`]}}`
	reqPatch := httptest.NewRequest(http.MethodPatch,
		"http://example/bigquery/v2/projects/dev/datasets/ds/tables/t",
		io.NopCloser(strings.NewReader(patchBody)))
	reqPatch.SetPathValue("projectId", "dev")
	reqPatch.SetPathValue("datasetId", "ds")
	reqPatch.SetPathValue("tableId", "t")
	reqPatch.Header.Set("Content-Type", "application/json")
	recPatch := httptest.NewRecorder()
	TablePatch(deps)(recPatch, reqPatch)
	if recPatch.Code != http.StatusOK {
		t.Fatalf("TablePatch -> %d: %s", recPatch.Code, recPatch.Body.String())
	}

	reqGet := httptest.NewRequest(http.MethodGet,
		"http://example/bigquery/v2/projects/dev/datasets/ds/tables/t", nil)
	reqGet.SetPathValue("projectId", "dev")
	reqGet.SetPathValue("datasetId", "ds")
	reqGet.SetPathValue("tableId", "t")
	recGet := httptest.NewRecorder()
	TableGet(deps)(recGet, reqGet)
	var got bqtypes.Table
	if err := json.Unmarshal(recGet.Body.Bytes(), &got); err != nil {
		t.Fatalf("decode: %v", err)
	}
	if got.Schema == nil || len(got.Schema.Fields) < 2 || got.Schema.Fields[1].PolicyTags == nil {
		t.Fatalf("schema = %#v", got.Schema)
	}
	if got.Schema.Fields[1].PolicyTags.Names[0] != tag {
		t.Fatalf("policyTags = %#v", got.Schema.Fields[1].PolicyTags)
	}
}
