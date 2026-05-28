package handlers

import (
	"encoding/json"
	"net/http"
	"net/http/httptest"
	"testing"
)

// TestProjectListReturnsDefaultProject pins the synthetic single-project
// shape returned when BIGQUERY_EMULATOR_PROJECT is unset. Clients use
// this to enumerate projects at startup, so the resource entry must
// carry kind/id/projectReference exactly the way docs/bigquery/docs/
// reference/rest/v2/projects/list.md describes.
func TestProjectListReturnsDefaultProject(t *testing.T) {
	t.Setenv(defaultProjectEnvVar, "")

	rec := httptest.NewRecorder()
	req := httptest.NewRequest(http.MethodGet, "/bigquery/v2/projects", nil)
	ProjectList(Dependencies{})(rec, req)

	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d, want 200", rec.Code)
	}
	if ct := rec.Header().Get("Content-Type"); ct != contentTypeJSON {
		t.Fatalf("Content-Type = %q, want application/json", ct)
	}

	var doc map[string]any
	if err := json.NewDecoder(rec.Body).Decode(&doc); err != nil {
		t.Fatalf("decode body: %v", err)
	}
	if doc["kind"] != "bigquery#projectList" {
		t.Errorf("kind = %v, want %q", doc["kind"], "bigquery#projectList")
	}
	projects, ok := doc["projects"].([]any)
	if !ok {
		t.Fatalf("projects field = %T, want []any", doc["projects"])
	}
	if len(projects) != 1 {
		t.Fatalf("len(projects) = %d, want 1", len(projects))
	}
	entry, ok := projects[0].(map[string]any)
	if !ok {
		t.Fatalf("projects[0] = %T, want map", projects[0])
	}
	if entry["kind"] != "bigquery#project" {
		t.Errorf("projects[0].kind = %v, want %q", entry["kind"], "bigquery#project")
	}
	if entry["id"] != defaultProjectID {
		t.Errorf("projects[0].id = %v, want %q", entry["id"], defaultProjectID)
	}
	ref, ok := entry["projectReference"].(map[string]any)
	if !ok {
		t.Fatalf("projects[0].projectReference = %T, want map", entry["projectReference"])
	}
	if ref["projectId"] != defaultProjectID {
		t.Errorf("projectReference.projectId = %v, want %q", ref["projectId"], defaultProjectID)
	}
	if doc["totalItems"] != float64(1) {
		t.Errorf("totalItems = %v, want 1", doc["totalItems"])
	}
}

// TestProjectListHonorsEnvOverride verifies BIGQUERY_EMULATOR_PROJECT
// is the source of truth when set. Compose stacks and CI jobs use this
// to pretend the emulator owns a specific project ID.
func TestProjectListHonorsEnvOverride(t *testing.T) {
	t.Setenv(defaultProjectEnvVar, "my-project")

	rec := httptest.NewRecorder()
	req := httptest.NewRequest(http.MethodGet, "/bigquery/v2/projects", nil)
	ProjectList(Dependencies{})(rec, req)

	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d, want 200", rec.Code)
	}
	var doc map[string]any
	if err := json.NewDecoder(rec.Body).Decode(&doc); err != nil {
		t.Fatalf("decode body: %v", err)
	}
	projects := doc["projects"].([]any)
	entry := projects[0].(map[string]any)
	if entry["id"] != "my-project" {
		t.Errorf("projects[0].id = %v, want %q", entry["id"], "my-project")
	}
	ref := entry["projectReference"].(map[string]any)
	if ref["projectId"] != "my-project" {
		t.Errorf("projectReference.projectId = %v, want %q", ref["projectId"], "my-project")
	}
}

// TestProjectGetServiceAccountUsesPathProject pins the documented
// email format: bigquery-emulator@<projectId>.iam.gserviceaccount.com.
// Client libraries derive KMS-related behavior from this email, so the
// format must be stable and the projectId must come from the URL (not
// the env override) to match real BigQuery semantics.
func TestProjectGetServiceAccountUsesPathProject(t *testing.T) {
	t.Setenv(defaultProjectEnvVar, "ignored-env-project")

	req := httptest.NewRequest(http.MethodGet,
		"/bigquery/v2/projects/some-proj/serviceAccount", nil)
	req.SetPathValue("projectId", "some-proj")
	rec := httptest.NewRecorder()
	ProjectGetServiceAccount(Dependencies{})(rec, req)

	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d, want 200", rec.Code)
	}
	if ct := rec.Header().Get("Content-Type"); ct != contentTypeJSON {
		t.Fatalf("Content-Type = %q, want application/json", ct)
	}
	var doc map[string]any
	if err := json.NewDecoder(rec.Body).Decode(&doc); err != nil {
		t.Fatalf("decode body: %v", err)
	}
	if doc["kind"] != "bigquery#getServiceAccountResponse" {
		t.Errorf("kind = %v, want %q", doc["kind"], "bigquery#getServiceAccountResponse")
	}
	want := "bigquery-emulator@some-proj.iam.gserviceaccount.com"
	if doc["email"] != want {
		t.Errorf("email = %v, want %q", doc["email"], want)
	}
}

// TestProjectGetServiceAccountFallsBackToEnv guards the edge case
// where PathValue("projectId") is empty (test setup or odd routing).
// In that case the handler must fall back to the env-derived project
// so the email never contains an empty subdomain.
func TestProjectGetServiceAccountFallsBackToEnv(t *testing.T) {
	t.Setenv(defaultProjectEnvVar, "envproj")

	req := httptest.NewRequest(http.MethodGet,
		"/bigquery/v2/projects//serviceAccount", nil)
	rec := httptest.NewRecorder()
	ProjectGetServiceAccount(Dependencies{})(rec, req)

	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d, want 200", rec.Code)
	}
	var doc map[string]any
	if err := json.NewDecoder(rec.Body).Decode(&doc); err != nil {
		t.Fatalf("decode body: %v", err)
	}
	want := "bigquery-emulator@envproj.iam.gserviceaccount.com"
	if doc["email"] != want {
		t.Errorf("email = %v, want %q", doc["email"], want)
	}
}
