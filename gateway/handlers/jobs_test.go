package handlers

import (
	"encoding/json"
	"net/http"
	"net/http/httptest"
	"strconv"
	"testing"
	"time"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
	"github.com/vantaboard/bigquery-emulator/gateway/jobs"
)

// makeListReq builds an authenticated `bigquery.jobs.list` request
// scoped to `testProjectID`. The rawQuery is appended verbatim to
// the URL. Pulled out of the per-test bodies so the boilerplate is
// in one place; the rest of the package's tests follow the same
// pattern (see `runQueryWithDeps` in `queries_helpers_test.go`).
func makeListReq(t *testing.T, rawQuery string) *http.Request {
	t.Helper()
	url := "/bigquery/v2/projects/" + testProjectID + "/jobs"
	if rawQuery != "" {
		url += "?" + rawQuery
	}
	req := httptest.NewRequest(http.MethodGet, url, nil)
	req.SetPathValue("projectId", testProjectID)
	return req
}

// makeJobIDReq builds a path-templated request the JobGet / JobCancel /
// JobDelete tests use. Method is variable because the three handlers
// answer different verbs.
func makeJobIDReq(t *testing.T, method, projectID, jobID string) *http.Request {
	t.Helper()
	url := "/bigquery/v2/projects/" + projectID + "/jobs/" + jobID
	req := httptest.NewRequest(method, url, nil)
	req.SetPathValue("projectId", projectID)
	req.SetPathValue("jobId", jobID)
	return req
}

// seedDoneJob registers a synthetic DONE entry against the supplied
// registry. The creation/start/end timestamps offset by `seq` so a
// chronological ordering test can sort entries deterministically.
// `parent` chains the new job to a script parent when non-empty.
func seedDoneJob(reg *jobs.Registry, projectID, jobID, parent string, seq int) *jobs.Job {
	creation := time.Unix(1_700_000_000+int64(seq), 0).UTC()
	j := &jobs.Job{
		Kind: jobs.JobKind,
		ID:   projectID + ":" + jobID,
		JobReference: bqtypes.JobReference{
			ProjectID: projectID,
			JobID:     jobID,
			Location:  "US",
		},
		Status: jobs.Status{State: jobs.JobStateDone},
		Statistics: jobs.Statistics{
			CreationTime:        strconv.FormatInt(creation.UnixMilli(), 10),
			StartTime:           strconv.FormatInt(creation.UnixMilli(), 10),
			EndTime:             strconv.FormatInt(creation.Add(time.Second).UnixMilli(), 10),
			TotalBytesProcessed: "0",
		},
		ParentJobID: parent,
	}
	reg.Register(j)
	return j
}

// jobListResponse mirrors the wire shape `JobList` emits. Only the
// fields the tests assert on are modelled; trailing fields stay in
// the catch-all `map[string]any` so a schema growth in JobList
// doesn't quietly break decoding.
type jobListResponse struct {
	Kind          string      `json:"kind"`
	Jobs          []*jobs.Job `json:"jobs"`
	NextPageToken string      `json:"nextPageToken,omitempty"`
}

// TestJobListPagination drives the cursor end-to-end: seed three
// jobs, ask for them two per page, walk forward, confirm we see the
// expected newest-first order and that the terminal page does NOT
// emit a nextPageToken (so polling clients exit cleanly).
func TestJobListPagination(t *testing.T) {
	t.Parallel()
	reg := jobs.NewRegistry()
	for i := range 3 {
		seedDoneJob(reg, testProjectID, "job_"+strconv.Itoa(i), "", i)
	}
	deps := Dependencies{Jobs: reg}

	rec := httptest.NewRecorder()
	JobList(deps)(rec, makeListReq(t, "maxResults=2"))
	if rec.Code != http.StatusOK {
		t.Fatalf("page 1 status = %d, want 200; body=%s", rec.Code, rec.Body.String())
	}
	var page1 jobListResponse
	if err := json.NewDecoder(rec.Body).Decode(&page1); err != nil {
		t.Fatalf("page 1 decode: %v", err)
	}
	if page1.Kind != jobListKind {
		t.Errorf("page 1 kind = %q, want %q", page1.Kind, jobListKind)
	}
	if len(page1.Jobs) != 2 {
		t.Fatalf("page 1 jobs = %d, want 2", len(page1.Jobs))
	}
	if page1.Jobs[0].JobReference.JobID != "job_2" {
		t.Errorf("page 1 jobs[0] = %q, want %q (newest-first)",
			page1.Jobs[0].JobReference.JobID, "job_2")
	}
	if page1.Jobs[1].JobReference.JobID != "job_1" {
		t.Errorf("page 1 jobs[1] = %q, want %q (newest-first)",
			page1.Jobs[1].JobReference.JobID, "job_1")
	}
	if page1.NextPageToken == "" {
		t.Fatal("page 1 missing nextPageToken; more results remain")
	}

	rec = httptest.NewRecorder()
	JobList(deps)(rec, makeListReq(t,
		"maxResults=2&pageToken="+page1.NextPageToken))
	if rec.Code != http.StatusOK {
		t.Fatalf("page 2 status = %d, want 200; body=%s", rec.Code, rec.Body.String())
	}
	var page2 jobListResponse
	if err := json.NewDecoder(rec.Body).Decode(&page2); err != nil {
		t.Fatalf("page 2 decode: %v", err)
	}
	if len(page2.Jobs) != 1 {
		t.Fatalf("page 2 jobs = %d, want 1 (tail page)", len(page2.Jobs))
	}
	if page2.Jobs[0].JobReference.JobID != "job_0" {
		t.Errorf("page 2 jobs[0] = %q, want %q", page2.Jobs[0].JobReference.JobID, "job_0")
	}
	if page2.NextPageToken != "" {
		t.Errorf("page 2 nextPageToken = %q, want empty on terminal page",
			page2.NextPageToken)
	}
}

// TestJobListScopedToProject confirms the project filter rejects
// jobs minted under a sibling projectId, so a tenant scoped to one
// project never sees another's job ids.
func TestJobListScopedToProject(t *testing.T) {
	t.Parallel()
	reg := jobs.NewRegistry()
	seedDoneJob(reg, testProjectID, "mine", "", 0)
	seedDoneJob(reg, "other", "theirs", "", 1)
	deps := Dependencies{Jobs: reg}

	rec := httptest.NewRecorder()
	JobList(deps)(rec, makeListReq(t, ""))
	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d, want 200", rec.Code)
	}
	var resp jobListResponse
	if err := json.NewDecoder(rec.Body).Decode(&resp); err != nil {
		t.Fatalf("decode: %v", err)
	}
	if len(resp.Jobs) != 1 {
		t.Fatalf("jobs = %d, want 1", len(resp.Jobs))
	}
	if resp.Jobs[0].JobReference.JobID != "mine" {
		t.Errorf("jobs[0] = %q, want %q (other project leaked)",
			resp.Jobs[0].JobReference.JobID, "mine")
	}
}

// TestJobListRejectsAllUsers asserts the documented 501 the handler
// returns for `allUsers=true`. The emulator has no auth context, so
// honoring the parameter would silently hand back results scoped to
// the wrong tenant.
func TestJobListRejectsAllUsers(t *testing.T) {
	t.Parallel()
	deps := Dependencies{Jobs: jobs.NewRegistry()}
	rec := httptest.NewRecorder()
	JobList(deps)(rec, makeListReq(t, "allUsers=true"))
	if rec.Code != http.StatusNotImplemented {
		t.Fatalf("status = %d, want 501", rec.Code)
	}
	var env errorEnvelope
	if err := json.NewDecoder(rec.Body).Decode(&env); err != nil {
		t.Fatalf("decode: %v", err)
	}
	if env.Error.Status != reasonNotImplemented {
		t.Errorf("error.status = %q, want %q", env.Error.Status, reasonNotImplemented)
	}
}

// TestJobGetMissingReturns404 pins the BigQuery-shaped envelope a
// lookup against an unknown jobId emits. The match against the
// upstream wording `Not found: Job <project>:<jobId>` matters because
// the Node BigQuery client's `Job#exists` predicate scrapes the
// message text to distinguish 404 from 500.
func TestJobGetMissingReturns404(t *testing.T) {
	t.Parallel()
	deps := Dependencies{Jobs: jobs.NewRegistry()}
	rec := httptest.NewRecorder()
	JobGet(deps)(rec, makeJobIDReq(t, http.MethodGet, testProjectID, "ghost"))
	if rec.Code != http.StatusNotFound {
		t.Fatalf("status = %d, want 404", rec.Code)
	}
	var env errorEnvelope
	if err := json.NewDecoder(rec.Body).Decode(&env); err != nil {
		t.Fatalf("decode: %v", err)
	}
	if env.Error.Status != reasonNotFound {
		t.Errorf("error.status = %q, want %q", env.Error.Status, reasonNotFound)
	}
	want := "Not found: Job " + testProjectID + ":ghost"
	if env.Error.Message != want {
		t.Errorf("error.message = %q, want %q", env.Error.Message, want)
	}
}

// TestJobGetWrongProjectReturns404 confirms a job minted under
// project A is hidden behind 404 when looked up via project B's URL,
// matching the upstream behavior (cross-project visibility is not a
// permission error, it's a "not found" envelope).
func TestJobGetWrongProjectReturns404(t *testing.T) {
	t.Parallel()
	reg := jobs.NewRegistry()
	seedDoneJob(reg, testProjectID, "secret", "", 0)
	deps := Dependencies{Jobs: reg}

	rec := httptest.NewRecorder()
	JobGet(deps)(rec, makeJobIDReq(t, http.MethodGet, "other", "secret"))
	if rec.Code != http.StatusNotFound {
		t.Fatalf("status = %d, want 404 (cross-project must be hidden)", rec.Code)
	}
}

// TestJobGetSuccessRoundTrips ensures the stored Job is emitted
// verbatim — kind, id, jobReference, status, statistics all
// preserved.
func TestJobGetSuccessRoundTrips(t *testing.T) {
	t.Parallel()
	reg := jobs.NewRegistry()
	seeded := seedDoneJob(reg, testProjectID, "abc", "", 0)
	deps := Dependencies{Jobs: reg}

	rec := httptest.NewRecorder()
	JobGet(deps)(rec, makeJobIDReq(t, http.MethodGet, testProjectID, "abc"))
	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d, want 200; body=%s", rec.Code, rec.Body.String())
	}
	var got jobs.Job
	if err := json.NewDecoder(rec.Body).Decode(&got); err != nil {
		t.Fatalf("decode: %v", err)
	}
	if got.JobReference.JobID != seeded.JobReference.JobID {
		t.Errorf("jobReference.jobId = %q, want %q",
			got.JobReference.JobID, seeded.JobReference.JobID)
	}
	if got.Status.State != jobs.JobStateDone {
		t.Errorf("status.state = %q, want %q", got.Status.State, jobs.JobStateDone)
	}
	if got.Statistics.CreationTime != seeded.Statistics.CreationTime {
		t.Errorf("statistics.creationTime = %q, want %q",
			got.Statistics.CreationTime, seeded.Statistics.CreationTime)
	}
}

// jobCancelResponse mirrors `bigquery#jobCancelResponse`.
type jobCancelResponse struct {
	Kind string    `json:"kind"`
	Job  *jobs.Job `json:"job"`
}

// TestJobCancelTerminalIsNoop pins the idempotent contract: cancel
// against a DONE job returns the job with cancelRequested=true
// stamped on its status, but the state itself stays DONE — the
// upstream API never reopens a terminal entry.
func TestJobCancelTerminalIsNoop(t *testing.T) {
	t.Parallel()
	reg := jobs.NewRegistry()
	seeded := seedDoneJob(reg, testProjectID, "terminal", "", 0)
	wantEnd := seeded.Statistics.EndTime
	deps := Dependencies{Jobs: reg}

	rec := httptest.NewRecorder()
	JobCancel(deps)(rec, makeJobIDReq(t, http.MethodPost, testProjectID, "terminal"))
	if rec.Code != http.StatusOK {
		t.Fatalf("status = %d, want 200", rec.Code)
	}
	var got jobCancelResponse
	if err := json.NewDecoder(rec.Body).Decode(&got); err != nil {
		t.Fatalf("decode: %v", err)
	}
	if got.Kind != jobCancelKind {
		t.Errorf("kind = %q, want %q", got.Kind, jobCancelKind)
	}
	if got.Job == nil {
		t.Fatal("job missing on cancel envelope")
	}
	if got.Job.Status.State != jobs.JobStateDone {
		t.Errorf("status.state = %q, want %q (terminal must not reopen)",
			got.Job.Status.State, jobs.JobStateDone)
	}
	if !got.Job.Status.CancelRequested {
		t.Error("status.cancelRequested = false, want true (cancel always flips the flag)")
	}
	if got.Job.Statistics.EndTime != wantEnd {
		t.Errorf("statistics.endTime = %q, want %q (cancel-on-terminal must not bump)",
			got.Job.Statistics.EndTime, wantEnd)
	}
}

// TestJobCancelMissingReturns404 asserts the standard not-found
// envelope when the handler is asked to cancel a job that was never
// registered.
func TestJobCancelMissingReturns404(t *testing.T) {
	t.Parallel()
	deps := Dependencies{Jobs: jobs.NewRegistry()}
	rec := httptest.NewRecorder()
	JobCancel(deps)(rec, makeJobIDReq(t, http.MethodPost, testProjectID, "ghost"))
	if rec.Code != http.StatusNotFound {
		t.Fatalf("status = %d, want 404", rec.Code)
	}
}

// TestJobDeleteCascadesChildren walks the script-parent contract:
// delete on a parent removes the parent AND every entry whose
// ParentJobID matches. Sibling jobs (different parent / no parent)
// stay put.
func TestJobDeleteCascadesChildren(t *testing.T) {
	t.Parallel()
	reg := jobs.NewRegistry()
	parent := seedDoneJob(reg, testProjectID, "script", "", 0)
	seedDoneJob(reg, testProjectID, "child_1", parent.JobReference.JobID, 1)
	seedDoneJob(reg, testProjectID, "child_2", parent.JobReference.JobID, 2)
	seedDoneJob(reg, testProjectID, "stranger", "", 3)
	deps := Dependencies{Jobs: reg}

	rec := httptest.NewRecorder()
	JobDelete(deps)(rec, makeJobIDReq(t, http.MethodDelete, testProjectID, "script"))
	if rec.Code != http.StatusNoContent {
		t.Fatalf("status = %d, want 204", rec.Code)
	}

	if _, ok := reg.Get("script"); ok {
		t.Error("parent script job still in registry after delete")
	}
	if _, ok := reg.Get("child_1"); ok {
		t.Error("child_1 should have cascaded out with the parent")
	}
	if _, ok := reg.Get("child_2"); ok {
		t.Error("child_2 should have cascaded out with the parent")
	}
	if _, ok := reg.Get("stranger"); !ok {
		t.Error("unrelated job evicted by cascade; only parent's children should drop")
	}
}

// TestJobDeleteMissingReturns404 asserts the standard not-found
// envelope when the handler is asked to delete an unknown jobId.
func TestJobDeleteMissingReturns404(t *testing.T) {
	t.Parallel()
	deps := Dependencies{Jobs: jobs.NewRegistry()}
	rec := httptest.NewRecorder()
	JobDelete(deps)(rec, makeJobIDReq(t, http.MethodDelete, testProjectID, "ghost"))
	if rec.Code != http.StatusNotFound {
		t.Fatalf("status = %d, want 404", rec.Code)
	}
}
