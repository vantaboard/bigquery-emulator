package handlers

import (
	"crypto/rand"
	"encoding/hex"
	"encoding/json"
	"io"
	"net/http"
	"sort"
	"strings"
	"sync"
	"time"
)

// BigQuery Migration v2alpha REST shell.
//
// The upstream BigQuery Migration API runs at
// `https://bigquerymigration.googleapis.com/v2alpha/...` (and the
// v2 alias at the same host). Client libraries
// (cloud.google.com/go/bigquery/migration/apiv2alpha,
// google-cloud-bigquery-migration for Python/Node/Java) read
// `BIGQUERY_MIGRATION_EMULATOR_HOST` and fall back to
// `BIGQUERY_EMULATOR_HOST` so this gateway can serve both surfaces
// from the same listener.
//
// This shell keeps workflow metadata in an in-process sync.Map store
// (no AST translator, no LRO store, no subtask catalog). Create
// returns a DRAFT workflow; :start transitions it to RUNNING so
// client startup probes get structurally-valid responses.
//
// Routes registered (for both `v2alpha` and `v2`):
//   GET    /{ver}/projects/{projectId}/locations/{location}/workflows
//   POST   /{ver}/projects/{projectId}/locations/{location}/workflows
//   GET    /{ver}/projects/{projectId}/locations/{location}/workflows/{workflowId}
//   DELETE /{ver}/projects/{projectId}/locations/{location}/workflows/{workflowId}
//   POST   /{ver}/projects/{projectId}/locations/{location}/workflows/{workflowId}:start
//          (dispatched on trailing :start via MigrationWorkflowCustomMethodPOST,
//          because net/http's mux can't match `{workflowId}:start` directly.)

const (
	migrationWorkflowStateDraft   = "DRAFT"
	migrationWorkflowStateRunning = "RUNNING"
)

var migrationWorkflowStore sync.Map // canonical name -> *migrationWorkflowResource

type migrationWorkflowResource struct {
	Name           string `json:"name"`
	DisplayName    string `json:"displayName,omitempty"`
	State          string `json:"state,omitempty"`
	CreateTime     string `json:"createTime,omitempty"`
	LastUpdateTime string `json:"lastUpdateTime,omitempty"`
}

func migrationWorkflowParent(r *http.Request) string {
	return "projects/" + r.PathValue("projectId") +
		"/locations/" + r.PathValue("location")
}

func migrationWorkflowNow() string {
	return time.Now().UTC().Format(time.RFC3339Nano)
}

func migrationWorkflowMintID() string {
	var b [8]byte
	_, _ = rand.Read(b[:])
	return hex.EncodeToString(b[:])
}

func migrationWorkflowByName(name string) (*migrationWorkflowResource, bool) {
	v, ok := migrationWorkflowStore.Load(name)
	if !ok {
		return nil, false
	}
	wf, ok := v.(*migrationWorkflowResource)
	return wf, ok && wf != nil
}

// MigrationWorkflowList implements `migration.workflows.list`.
func MigrationWorkflowList(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		prefix := migrationWorkflowParent(r) + "/workflows/"
		var out []migrationWorkflowResource
		migrationWorkflowStore.Range(func(key, value any) bool {
			name, _ := key.(string)
			if !strings.HasPrefix(name, prefix) {
				return true
			}
			wf, _ := value.(*migrationWorkflowResource)
			if wf != nil {
				out = append(out, *wf)
			}
			return true
		})
		sort.Slice(out, func(i, j int) bool { return out[i].Name < out[j].Name })
		workflows := make([]any, len(out))
		for i := range out {
			workflows[i] = out[i]
		}
		writeJSON(w, http.StatusOK, map[string]any{
			"migrationWorkflows": workflows,
		})
	}
}

// MigrationWorkflowCreate implements `migration.workflows.create`.
func MigrationWorkflowCreate(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		body, err := io.ReadAll(r.Body)
		if err != nil {
			writeError(w, http.StatusBadRequest, reasonInvalid, "invalid body")
			return
		}
		_ = r.Body.Close()
		var in migrationWorkflowResource
		if len(strings.TrimSpace(string(body))) > 0 {
			if err := json.Unmarshal(body, &in); err != nil {
				writeError(w, http.StatusBadRequest, reasonInvalid,
					"invalid json: "+err.Error())
				return
			}
		}
		id := migrationWorkflowMintID()
		name := migrationWorkflowParent(r) + "/workflows/" + id
		now := migrationWorkflowNow()
		rec := migrationWorkflowResource{
			Name:           name,
			DisplayName:    in.DisplayName,
			State:          migrationWorkflowStateDraft,
			CreateTime:     now,
			LastUpdateTime: now,
		}
		migrationWorkflowStore.Store(name, &rec)
		writeJSON(w, http.StatusOK, rec)
	}
}

// MigrationWorkflowGet implements `migration.workflows.get`.
func MigrationWorkflowGet(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		name := migrationWorkflowName(r)
		wf, ok := migrationWorkflowByName(name)
		if !ok {
			writeError(w, http.StatusNotFound, reasonNotFound,
				"Not found: MigrationWorkflow "+name)
			return
		}
		writeJSON(w, http.StatusOK, *wf)
	}
}

// MigrationWorkflowDelete implements `migration.workflows.delete`.
func MigrationWorkflowDelete(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		name := migrationWorkflowName(r)
		if _, ok := migrationWorkflowByName(name); !ok {
			writeError(w, http.StatusNotFound, reasonNotFound,
				"Not found: MigrationWorkflow "+name)
			return
		}
		migrationWorkflowStore.Delete(name)
		writeJSON(w, http.StatusOK, struct{}{})
	}
}

// MigrationWorkflowCustomMethodPOST dispatches the AIP-136 ":start"
// custom method that hangs off a workflow resource.
func MigrationWorkflowCustomMethodPOST(_ Dependencies) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		_, op := splitColonOp(r.PathValue("workflowId"))
		switch op {
		case "start":
			name := migrationWorkflowName(r)
			wf, ok := migrationWorkflowByName(name)
			if !ok {
				writeError(w, http.StatusNotFound, reasonNotFound,
					"Not found: MigrationWorkflow "+name)
				return
			}
			switch wf.State {
			case migrationWorkflowStateDraft:
				wf.State = migrationWorkflowStateRunning
				wf.LastUpdateTime = migrationWorkflowNow()
				migrationWorkflowStore.Store(name, wf)
			case migrationWorkflowStateRunning:
				// no-op
			default:
				writeError(w, http.StatusBadRequest, reasonFailedPrecondition,
					"MigrationWorkflow "+name+" is not in DRAFT or RUNNING state")
				return
			}
			writeJSON(w, http.StatusOK, struct{}{})
		case "":
			writeError(w, http.StatusMethodNotAllowed, reasonInvalid,
				"POST is not allowed on a workflow resource. "+
					"Use POST .../workflows to create or :start to start.")
		default:
			writeError(w, http.StatusNotFound, reasonNotFound,
				"Unknown migration workflow custom method ':"+op+"'.")
		}
	}
}

// migrationWorkflowName reconstructs the canonical resource name from
// the path captures so error envelopes match upstream error text.
func migrationWorkflowName(r *http.Request) string {
	wid, _ := splitColonOp(r.PathValue("workflowId"))
	return "projects/" + r.PathValue("projectId") +
		"/locations/" + r.PathValue("location") +
		"/workflows/" + strings.TrimSpace(wid)
}
