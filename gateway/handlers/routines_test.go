package handlers

import (
	"context"
	"encoding/json"
	"net/http"
	"net/http/httptest"
	"strings"
	"testing"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
)

const (
	routineTestProjectID = "p"
	routineTestDatasetID = "d"
)

func routineDeps() Dependencies {
	return Dependencies{Routines: NewRoutineStore()}
}

func newRoutineReq(method, routineID string, body string) *http.Request {
	url := "/bigquery/v2/projects/" + routineTestProjectID + "/datasets/" + routineTestDatasetID + "/routines"
	if routineID != "" {
		url += "/" + routineID
	}
	var reader *strings.Reader
	if body != "" {
		reader = strings.NewReader(body)
	} else {
		reader = strings.NewReader("")
	}
	req := httptest.NewRequest(method, url, reader)
	req.SetPathValue("projectId", routineTestProjectID)
	req.SetPathValue("datasetId", routineTestDatasetID)
	if routineID != "" {
		req.SetPathValue("routineId", routineID)
	}
	return req
}

func sampleRoutineBody(routineID, body string) string {
	rt := bqtypes.Routine{
		RoutineReference: bqtypes.RoutineReference{RoutineID: routineID},
		RoutineType:      defaultRoutineType,
		Language:         defaultRoutineLanguage,
		DefinitionBody:   body,
		Arguments: []bqtypes.RoutineArgument{{
			Name: "x",
			DataType: &bqtypes.StandardSqlDataType{
				TypeKind: sqlTypeINT64,
			},
		}},
		ReturnType: &bqtypes.StandardSqlDataType{TypeKind: sqlTypeINT64},
	}
	b, _ := json.Marshal(rt)
	return string(b)
}

func TestRoutineCRUDRoundTrip(t *testing.T) {
	deps := routineDeps()
	const routineID = "fn1"

	insertRec := httptest.NewRecorder()
	RoutineInsert(deps)(insertRec, newRoutineReq(http.MethodPost, "", sampleRoutineBody(routineID, "x * 3")))
	if insertRec.Code != http.StatusOK {
		t.Fatalf("insert status = %d, want 200; body=%s", insertRec.Code, insertRec.Body.String())
	}

	got := decodeRoutineResponse(t, deps, routineID, "x * 3")
	if got.RoutineReference.RoutineID != routineID {
		t.Errorf("routineId = %q, want %q", got.RoutineReference.RoutineID, routineID)
	}
	assertRoutineListed(t, deps, 1)

	updateRec := httptest.NewRecorder()
	RoutineUpdate(deps)(updateRec, newRoutineReq(http.MethodPut, routineID, sampleRoutineBody(routineID, "x * 4")))
	if updateRec.Code != http.StatusOK {
		t.Fatalf("update status = %d, want 200; body=%s", updateRec.Code, updateRec.Body.String())
	}
	_ = decodeRoutineResponse(t, deps, routineID, "x * 4")

	deleteRec := httptest.NewRecorder()
	RoutineDelete(deps)(deleteRec, newRoutineReq(http.MethodDelete, routineID, ""))
	if deleteRec.Code != http.StatusOK {
		t.Fatalf("delete status = %d, want 200; body=%s", deleteRec.Code, deleteRec.Body.String())
	}

	missRec := httptest.NewRecorder()
	RoutineGet(deps)(missRec, newRoutineReq(http.MethodGet, routineID, ""))
	if missRec.Code != http.StatusNotFound {
		t.Fatalf("get after delete status = %d, want 404", missRec.Code)
	}
}

func decodeRoutineResponse(t *testing.T, deps Dependencies, routineID, wantBody string) bqtypes.Routine {
	t.Helper()
	rec := httptest.NewRecorder()
	RoutineGet(deps)(rec, newRoutineReq(http.MethodGet, routineID, ""))
	if rec.Code != http.StatusOK {
		t.Fatalf("get status = %d, want 200; body=%s", rec.Code, rec.Body.String())
	}
	var got bqtypes.Routine
	if err := json.NewDecoder(rec.Body).Decode(&got); err != nil {
		t.Fatalf("decode get: %v", err)
	}
	if got.DefinitionBody != wantBody {
		t.Errorf("definitionBody = %q, want %q", got.DefinitionBody, wantBody)
	}
	return got
}

func assertRoutineListed(t *testing.T, deps Dependencies, want int) {
	t.Helper()
	rec := httptest.NewRecorder()
	RoutineList(deps)(rec, newRoutineReq(http.MethodGet, "", ""))
	if rec.Code != http.StatusOK {
		t.Fatalf("list status = %d, want 200", rec.Code)
	}
	var listed struct {
		Routines []bqtypes.Routine `json:"routines"`
	}
	if err := json.NewDecoder(rec.Body).Decode(&listed); err != nil {
		t.Fatalf("decode list: %v", err)
	}
	if len(listed.Routines) != want {
		t.Fatalf("listed routines = %d, want %d", len(listed.Routines), want)
	}
}

func TestRoutineListReturnsEmptyPage(t *testing.T) {
	rec := httptest.NewRecorder()
	RoutineList(routineDeps())(rec, newRoutineReq(http.MethodGet, "", ""))

	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d, want 200; body=%s", rec.Code, rec.Body.String())
	}
	var got struct {
		Kind     string `json:"kind"`
		Routines []any  `json:"routines"`
	}
	if err := json.NewDecoder(rec.Body).Decode(&got); err != nil {
		t.Fatalf("decode body: %v", err)
	}
	if got.Kind != routineListKind {
		t.Errorf("kind = %q, want %q", got.Kind, routineListKind)
	}
	if len(got.Routines) != 0 {
		t.Errorf("routines = %v, want empty page", got.Routines)
	}
}

func TestRoutineGetReturnsNotFound(t *testing.T) {
	rec := httptest.NewRecorder()
	RoutineGet(routineDeps())(rec, newRoutineReq(http.MethodGet, "r1", ""))

	if rec.Code != http.StatusNotFound {
		t.Fatalf("status = %d, want 404; body=%s", rec.Code, rec.Body.String())
	}
	if !strings.Contains(rec.Body.String(), "Not found: Routine p:d.r1") {
		t.Errorf("body missing fqn; got %s", rec.Body.String())
	}
}

func TestRoutineInsertDuplicateReturnsConflict(t *testing.T) {
	deps := routineDeps()
	body := sampleRoutineBody("dup", "x * 3")
	RoutineInsert(deps)(httptest.NewRecorder(), newRoutineReq(http.MethodPost, "", body))
	rec := httptest.NewRecorder()
	RoutineInsert(deps)(rec, newRoutineReq(http.MethodPost, "", body))
	if rec.Code != http.StatusConflict {
		t.Fatalf("status = %d, want 409; body=%s", rec.Code, rec.Body.String())
	}
}

func TestRoutineUpdateReturnsNotFound(t *testing.T) {
	rec := httptest.NewRecorder()
	RoutineUpdate(routineDeps())(rec, newRoutineReq(http.MethodPut, "r1", sampleRoutineBody("r1", "x")))

	if rec.Code != http.StatusNotFound {
		t.Fatalf("status = %d, want 404; body=%s", rec.Code, rec.Body.String())
	}
}

func TestRoutineDeleteReturnsNotFound(t *testing.T) {
	rec := httptest.NewRecorder()
	RoutineDelete(routineDeps())(rec, newRoutineReq(http.MethodDelete, "r1", ""))

	if rec.Code != http.StatusNotFound {
		t.Fatalf("status = %d, want 404; body=%s", rec.Code, rec.Body.String())
	}
}

func TestRoutineGetFallsBackToInMemoryAfterDDL(t *testing.T) {
	deps := Dependencies{Routines: NewRoutineStore(), Catalog: &fakeCatalogClient{}}
	const ddl = `CREATE FUNCTION myfunc(x INT64) RETURNS INT64 AS (x * 3);`
	ref := persistRoutineFromDDL(
		context.Background(), &deps, routineTestProjectID, routineTestDatasetID, ddl)
	if ref == nil || ref.RoutineID != "myfunc" {
		t.Fatalf("ref = %+v", ref)
	}
	rec := httptest.NewRecorder()
	RoutineGet(deps)(rec, newRoutineReq(http.MethodGet, "myfunc", ""))
	if rec.Code != http.StatusOK {
		t.Fatalf("get status = %d, want 200; body=%s", rec.Code, rec.Body.String())
	}
	got := decodeRoutineResponse(t, deps, "myfunc", "x * 3")
	if got.DefinitionBody != "x * 3" {
		t.Errorf("body = %q", got.DefinitionBody)
	}
}
