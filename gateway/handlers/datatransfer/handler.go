// Package datatransfer implements a minimal BigQuery Data Transfer Service
// REST shell on the emulator's HTTP mux: dataSources catalog, transferConfigs
// CRUD (in-memory), transferRuns CRUD, and the AIP-136 custom methods
// (`scheduleRuns`, `checkValidCreds`, `startManualRuns`).
//
// Ported from go-googlesql `api/datatransfer/` (handler.go, handler_runs.go,
// handler_scheduled_query.go, handlers_project_scoped.go, catalog.go,
// paging.go) per `.cursor/plans/java-its-shallow-emulators_b8c9d0e1.plan.md`.
// Adapter shim: go-googlesql's `apiregion.CheckHTTP` location-mismatch gate
// is intentionally dropped here — this repo's REST surface does not yet
// surface regional endpoints and the emulator's docker-compose listener is
// always loopback. Logging goes through `log/slog.New(slog.DiscardHandler)`
// when the caller does not provide one (mirrors go-googlesql).
//
// The package is wired by `gateway/server.go` via `(*Handler).Register(mux)`.
// Routes registered:
//
//	GET    /v1/projects/{projectId}/locations/{location}/dataSources
//	GET    /v1/projects/{projectId}/locations/{location}/dataSources/{dataSourceId}
//	GET    /v1/projects/{projectId}/transferConfigs
//	POST   /v1/projects/{projectId}/transferConfigs
//	GET    /v1/projects/{projectId}/locations/{location}/transferConfigs
//	POST   /v1/projects/{projectId}/locations/{location}/transferConfigs
//	GET    /v1/projects/{projectId}/locations/{location}/transferConfigs/{configId}
//	PATCH  /v1/projects/{projectId}/locations/{location}/transferConfigs/{configId}
//	DELETE /v1/projects/{projectId}/locations/{location}/transferConfigs/{configId}
//	POST   /v1/projects/{projectId}/locations/{location}/transferConfigs/{configSeg} (AIP-136 :scheduleRuns / :checkValidCreds / :startManualRuns)
//	GET    /v1/projects/{projectId}/locations/{location}/transferConfigs/{configId}/runs
//	POST   /v1/projects/{projectId}/locations/{location}/transferConfigs/{configId}/runs
//	GET    /v1/projects/{projectId}/locations/{location}/transferConfigs/{configId}/runs/{runId}
package datatransfer

import (
	"crypto/rand"
	"encoding/hex"
	"encoding/json"
	"errors"
	"fmt"
	"io"
	"log/slog"
	"net/http"
	"sort"
	"strconv"
	"strings"
	"sync"
	"sync/atomic"
	"time"
)

// transferStateSucceeded is the JSON state string for a completed
// transfer config or run.
const transferStateSucceeded = "SUCCEEDED"

// transferStateFailed is the JSON state string for a failed transfer
// run.
const transferStateFailed = "FAILED"

// dataSourceScheduledQuery is the dataSourceId for the scheduled SQL
// connector. The emulator only executes this surface when a Runner is
// wired (Phase C).
const dataSourceScheduledQuery = "scheduled_query"

// dataSourceAmazonS3 is a metadata-only stub for third-party connector
// discovery (Phase A baseline row 13: CreateAmazonS3TransferIT).
const dataSourceAmazonS3 = "amazon_s3"

// The following dataSourceId constants are the connector identifiers
// the upstream `Create*Transfer.java` driver classes send on
// CreateTransferConfig. They are metadata-only stubs; the emulator
// does not perform any third-party traffic. The connector IDs come
// directly from the snippet drivers (see e.g. CreateAdManagerTransfer
// → `dfp_dt`, CreateAdsTransfer → `adwords`, CreateTeradataTransfer →
// `on_premises`). Phase C's plan listed three IDs that diverge from
// what the drivers send (`admanager_transfer`, `google_ads`,
// `teradata`); registering the driver-side IDs is what actually moves
// CreateTransferConfig forward, so we follow the drivers here.
const (
	dataSourceAdManager           = "dfp_dt"
	dataSourceGoogleAds           = "adwords"
	dataSourceCampaignManager     = "dcm_dt"
	dataSourcePlay                = "play"
	dataSourceRedshift            = "redshift"
	dataSourceOnPremises          = "on_premises"
	dataSourceYoutubeChannel      = "youtube_channel"
	dataSourceYoutubeContentOwner = "youtube_content_owner"
)

const transferRunErrorMessageKey = "message"

func transferRunErrorPayload(msg string) map[string]any {
	return map[string]any{transferRunErrorMessageKey: msg}
}

// ScheduledQueryRunner executes scheduled_query transfer SQL against
// the emulator catalog. Phase B keeps this as a hook the gateway can
// fill in once `gateway/handlers/queries.go` is reachable from the
// gRPC-free unit-test path; left nil for now (no SQL execution).
type ScheduledQueryRunner interface {
	RunScheduledQueryTransfer(project, location, sql, defaultDatasetID string) error
}

// Handler stores transfer config and run metadata in memory.
type Handler struct {
	Log *slog.Logger
	// Runner optional; when set, scheduled_query manual runs and run
	// inserts execute SQL locally.
	Runner ScheduledQueryRunner
	// DataSourceCatalogExtras are merged into the built-in dataSources
	// catalog (same dataSourceId: extras win).
	DataSourceCatalogExtras []DataSourceCatalogEntry

	mu        sync.Mutex
	nextRunID atomic.Uint64
	configs   map[string]*transferConfigResource
	runs      map[string]*transferRunResource
}

// NewHandler returns an empty in-memory transfer service shell.
func NewHandler(log *slog.Logger) *Handler {
	return &Handler{
		Log:     log,
		configs: make(map[string]*transferConfigResource),
		runs:    make(map[string]*transferRunResource),
	}
}

func (h *Handler) logger() *slog.Logger {
	if h != nil && h.Log != nil {
		return h.Log
	}
	return slog.New(slog.DiscardHandler)
}

// Register wires v1 transfer config + run + dataSource routes into the
// caller's mux. The path shape matches the upstream BigQuery Data
// Transfer API (the gapic clients construct paths under `/v1/...`).
func (h *Handler) Register(mux *http.ServeMux) {
	loc := "/v1/projects/{projectId}/locations/{location}"
	mux.HandleFunc(http.MethodGet+" "+loc+"/dataSources", h.handleListDataSources)
	mux.HandleFunc(http.MethodGet+" "+loc+"/dataSources/{dataSourceId}", h.handleGetDataSource)

	// Project-scoped (no /locations/) variant: gapic Go REST clients
	// construct parent="projects/{p}" for create/list.
	projBase := "/v1/projects/{projectId}/transferConfigs"
	mux.HandleFunc(http.MethodGet+" "+projBase, h.handleListConfigsProjectScoped)
	mux.HandleFunc(http.MethodPost+" "+projBase, h.handleCreateConfigProjectScoped)

	base := loc + "/transferConfigs"
	mux.HandleFunc(http.MethodGet+" "+base, h.handleListConfigs)
	mux.HandleFunc(http.MethodPost+" "+base, h.handleCreateConfig)
	mux.HandleFunc(http.MethodGet+" "+base+"/{configId}", h.handleGetConfig)
	mux.HandleFunc(http.MethodPatch+" "+base+"/{configId}", h.handlePatchConfig)
	mux.HandleFunc(http.MethodDelete+" "+base+"/{configId}", h.handleDeleteConfig)
	mux.HandleFunc(http.MethodPost+" "+base+"/{configSeg}", h.handleConfigPostSegment)

	runsBase := base + "/{configId}/runs"
	mux.HandleFunc(http.MethodGet+" "+runsBase, h.handleListRuns)
	mux.HandleFunc(http.MethodPost+" "+runsBase, h.handleCreateRun)
	mux.HandleFunc(http.MethodGet+" "+runsBase+"/{runId}", h.handleGetRun)
}

// transferConfigResource is the JSON-on-wire shape for a single
// transfer config. Mirrors the proto3 field names the upstream gapic
// clients emit (camelCase). Disabled is *bool (not bool) so the patch
// path can distinguish "not in mask" from "set to false" — that is the
// fix Phase A row 14 (DisableTransferConfigIT) and 15
// (ReEnableTransferConfigIT) exercise.
type transferConfigResource struct {
	Name                 string         `json:"name,omitempty"`
	DisplayName          string         `json:"displayName,omitempty"`
	DataSourceID         string         `json:"dataSourceId,omitempty"`
	Schedule             string         `json:"schedule,omitempty"`
	Params               map[string]any `json:"params,omitempty"`
	State                string         `json:"state,omitempty"`
	Disabled             *bool          `json:"disabled,omitempty"`
	CreateTime           string         `json:"createTime,omitempty"`
	NextRunTime          string         `json:"nextRunTime,omitempty"`
	UserID               int64          `json:"userId,omitempty"`
	DatasetRegion        string         `json:"datasetRegion,omitempty"`
	DestinationDatasetID string         `json:"destinationDatasetId,omitempty"`
	DestinationDataset   *struct {
		DatasetReference *struct {
			ProjectID string `json:"projectId,omitempty"`
			DatasetID string `json:"datasetId,omitempty"`
		} `json:"datasetReference,omitempty"`
	} `json:"destinationDataset,omitempty"`
	DisableAutoScheduling bool `json:"disableAutoScheduling,omitempty"`
}

type listConfigsResponse struct {
	TransferConfigs []transferConfigResource `json:"transferConfigs"`
	NextPageToken   string                   `json:"nextPageToken,omitempty"`
}

type dataSourceResource struct {
	Name                           string `json:"name"`
	DataSourceID                   string `json:"dataSourceId"`
	DisplayName                    string `json:"displayName,omitempty"`
	Description                    string `json:"description,omitempty"`
	AuthorizationType              string `json:"authorizationType,omitempty"`
	DefaultDataRefreshIntervalDays int32  `json:"defaultDataRefreshIntervalDays,omitempty"`
	AuthorizationURL               string `json:"authorizationUrl,omitempty"`
}

type listDataSourcesResponse struct {
	DataSources   []dataSourceResource `json:"dataSources"`
	NextPageToken string               `json:"nextPageToken,omitempty"`
}

type transferRunResource struct {
	Name               string         `json:"name"`
	State              string         `json:"state,omitempty"`
	Errors             []any          `json:"errors,omitempty"`
	ScheduleTime       string         `json:"scheduleTime,omitempty"`
	RunTime            string         `json:"runTime,omitempty"`
	UpdateTime         string         `json:"updateTime,omitempty"`
	DataSourceID       string         `json:"dataSourceId,omitempty"`
	Params             map[string]any `json:"params,omitempty"`
	DatasetRegion      string         `json:"datasetRegion,omitempty"`
	DestinationDataset *struct {
		DatasetReference *struct {
			ProjectID string `json:"projectId,omitempty"`
			DatasetID string `json:"datasetId,omitempty"`
		} `json:"datasetReference,omitempty"`
	} `json:"destinationDataset,omitempty"`
}

func configName(project, location, id string) string {
	return fmt.Sprintf("projects/%s/locations/%s/transferConfigs/%s", project, location, id)
}

func runName(project, location, configID, runID string) string {
	return configName(project, location, configID) + "/runs/" + runID
}

// writeAPIError emits a Google-style error envelope (mirrors
// `gateway/handlers.writeError`'s shape). Localised here so the
// package does not import handlers (and doesn't pull the
// engine-client deps in with it).
func writeAPIError(log *slog.Logger, w http.ResponseWriter, status int, msg string) {
	if log != nil {
		log.Error("datatransfer api error",
			slog.Int("status", status),
			slog.String("message", msg),
		)
	}
	body := map[string]any{
		"error": map[string]any{
			"code":    status,
			"message": msg,
			"status":  apiErrorReason(status),
			"errors": []map[string]any{{
				"reason":  apiErrorReason(status),
				"message": msg,
				"domain":  "global",
			}},
		},
	}
	writeJSON(log, w, status, body)
}

func apiErrorReason(status int) string {
	switch status {
	case http.StatusBadRequest:
		return "badRequest"
	case http.StatusUnauthorized:
		return "unauthorized"
	case http.StatusForbidden:
		return "forbidden"
	case http.StatusNotFound:
		return "notFound"
	case http.StatusConflict:
		return "alreadyExists"
	case http.StatusInternalServerError:
		return "internalError"
	case http.StatusNotImplemented:
		return "notImplemented"
	case http.StatusMethodNotAllowed:
		return "methodNotAllowed"
	default:
		if status >= 500 {
			return "internalError"
		}
		if status >= 400 {
			return "badRequest"
		}
		return "unknown"
	}
}

func writeJSON(log *slog.Logger, w http.ResponseWriter, status int, v any) {
	w.Header().Set("Content-Type", "application/json; charset=UTF-8")
	w.WriteHeader(status)
	if err := json.NewEncoder(w).Encode(v); err != nil && log != nil {
		log.Error("datatransfer: encode json response", slog.String("err", err.Error()))
	}
}

func (h *Handler) handleListDataSources(w http.ResponseWriter, r *http.Request) {
	project := r.PathValue("projectId")
	location := r.PathValue("location")
	entries := h.mergedCatalogResources(project, location)
	start, end := pageWindow(len(entries), r.URL.Query().Get("pageSize"), r.URL.Query().Get("pageToken"))
	page := entries[start:end]
	resp := listDataSourcesResponse{DataSources: page}
	if end < len(entries) {
		resp.NextPageToken = strconv.Itoa(end)
	}
	writeJSON(h.logger(), w, http.StatusOK, resp)
}

func (h *Handler) mergedCatalogResources(project, location string) []dataSourceResource {
	entries := h.mergedCatalogEntries()
	out := make([]dataSourceResource, 0, len(entries))
	for _, e := range entries {
		out = append(out, h.dataSourceResource(project, location, e))
	}
	return out
}

func (h *Handler) handleGetDataSource(w http.ResponseWriter, r *http.Request) {
	project := r.PathValue("projectId")
	location := r.PathValue("location")
	dsID := strings.TrimSpace(r.PathValue("dataSourceId"))
	entry, ok := h.catalogEntryByID(dsID)
	if !ok {
		writeAPIError(h.logger(), w, http.StatusNotFound,
			"Not found: DataSource "+dsID)
		return
	}
	out := h.dataSourceResource(project, location, entry)
	writeJSON(h.logger(), w, http.StatusOK, out)
}

func (h *Handler) handleListConfigs(w http.ResponseWriter, r *http.Request) {
	project := r.PathValue("projectId")
	location := r.PathValue("location")
	prefix := fmt.Sprintf("projects/%s/locations/%s/transferConfigs/", project, location)

	h.mu.Lock()
	var keys []string
	for k := range h.configs {
		if strings.HasPrefix(k, prefix) {
			keys = append(keys, k)
		}
	}
	h.mu.Unlock()
	sort.Strings(keys)
	start, end := pageWindow(len(keys), r.URL.Query().Get("pageSize"), r.URL.Query().Get("pageToken"))
	pageKeys := keys[start:end]

	h.mu.Lock()
	defer h.mu.Unlock()
	out := make([]transferConfigResource, 0, len(pageKeys))
	for _, k := range pageKeys {
		if c, ok := h.configs[k]; ok {
			out = append(out, *c)
		}
	}
	resp := listConfigsResponse{TransferConfigs: out}
	if end < len(keys) {
		resp.NextPageToken = strconv.Itoa(end)
	}
	writeJSON(h.logger(), w, http.StatusOK, resp)
}

func (h *Handler) handleCreateConfig(w http.ResponseWriter, r *http.Request) {
	project := r.PathValue("projectId")
	location := r.PathValue("location")
	body, err := io.ReadAll(r.Body)
	if err != nil {
		writeAPIError(h.logger(), w, http.StatusBadRequest, "invalid body")
		return
	}
	_ = r.Body.Close()
	var in transferConfigResource
	if len(strings.TrimSpace(string(body))) > 0 {
		if err := json.Unmarshal(body, &in); err != nil {
			writeAPIError(h.logger(), w, http.StatusBadRequest, "invalid json: "+err.Error())
			return
		}
	}
	normalizeTransferConfigInput(project, &in)
	h.finishCreateTransferConfig(w, project, location, in)
}

func (h *Handler) finishCreateTransferConfig(
	w http.ResponseWriter,
	project, location string,
	in transferConfigResource,
) {
	id := pathSuffixOrGen(in.Name, "tc_"+randomHex(16))
	name := configName(project, location, id)
	now := time.Now().UTC().Format(time.RFC3339Nano)
	nextRun := strings.TrimSpace(in.NextRunTime)
	uid := int64(1)
	if in.UserID != 0 {
		uid = in.UserID
	}
	rec := transferConfigResource{
		Name:                  name,
		DisplayName:           in.DisplayName,
		DataSourceID:          in.DataSourceID,
		Schedule:              in.Schedule,
		Params:                in.Params,
		State:                 transferStateSucceeded,
		Disabled:              in.Disabled,
		CreateTime:            now,
		NextRunTime:           nextRun,
		UserID:                uid,
		DatasetRegion:         strings.TrimSpace(in.DatasetRegion),
		DestinationDatasetID:  strings.TrimSpace(in.DestinationDatasetID),
		DisableAutoScheduling: in.DisableAutoScheduling,
	}
	if in.DestinationDataset != nil {
		rec.DestinationDataset = in.DestinationDataset
	}

	h.mu.Lock()
	if _, dup := h.configs[name]; dup {
		h.mu.Unlock()
		writeAPIError(h.logger(), w, http.StatusConflict, "transfer config already exists")
		return
	}
	h.configs[name] = &rec
	h.maybeSeedInitialScheduledQueryRun(project, location, id, &rec)
	out := rec
	h.mu.Unlock()
	writeJSON(h.logger(), w, http.StatusOK, out)
}

func (h *Handler) maybeSeedInitialScheduledQueryRun(project, location, configID string, cfg *transferConfigResource) {
	if h == nil || cfg == nil {
		return
	}
	if strings.TrimSpace(cfg.DataSourceID) != dataSourceScheduledQuery {
		return
	}
	if h.Runner == nil || cfg.DisableAutoScheduling || strings.TrimSpace(cfg.Schedule) == "" {
		return
	}
	run := h.newTransferRun(project, location, configID, cfg)
	if stop, _, msg := h.maybeExecuteScheduledQueryOnRun(project, location, cfg, run); stop {
		run.State = transferStateFailed
		run.Errors = []any{transferRunErrorPayload(msg)}
	}
	h.runs[run.Name] = run
}

func pathSuffixOrGen(name, fallbackID string) string {
	name = strings.TrimSpace(name)
	if name == "" {
		return fallbackID
	}
	if i := strings.LastIndex(name, "/"); i >= 0 {
		return name[i+1:]
	}
	return fallbackID
}

func randomHex(n int) string {
	buf := make([]byte, n)
	_, _ = rand.Read(buf)
	return hex.EncodeToString(buf)
}

func (h *Handler) handleGetConfig(w http.ResponseWriter, r *http.Request) {
	project := r.PathValue("projectId")
	location := r.PathValue("location")
	id := r.PathValue("configId")
	name := configName(project, location, id)

	h.mu.Lock()
	defer h.mu.Unlock()
	c, ok := h.configs[name]
	if !ok {
		writeAPIError(h.logger(), w, http.StatusNotFound, "Not found: TransferConfig "+id)
		return
	}
	out := *c
	writeJSON(h.logger(), w, http.StatusOK, out)
}

// handlePatchConfig honors the `disabled` field on the request body;
// because Disabled is *bool, an explicit `"disabled": false` flips a
// disabled config back on (Phase A row 15: ReEnableTransferConfigIT)
// and `"disabled": true` disables it (row 14:
// DisableTransferConfigIT). Other fields update only when non-zero.
//
// updateMask is parsed from the `updateMask` query parameter (gapic
// REST clients append it). Phase B keeps the mask advisory: the mask
// names are not enforced, the body's non-zero fields drive the patch.
// That matches the existing emulator pattern for other PATCH
// endpoints.
func (h *Handler) handlePatchConfig(w http.ResponseWriter, r *http.Request) {
	project := r.PathValue("projectId")
	location := r.PathValue("location")
	id := r.PathValue("configId")
	name := configName(project, location, id)
	body, err := io.ReadAll(r.Body)
	if err != nil {
		writeAPIError(h.logger(), w, http.StatusBadRequest, "invalid body")
		return
	}
	_ = r.Body.Close()
	var patch transferConfigResource
	if err := json.Unmarshal(body, &patch); err != nil {
		writeAPIError(h.logger(), w, http.StatusBadRequest, "invalid json: "+err.Error())
		return
	}

	h.mu.Lock()
	defer h.mu.Unlock()
	cur, ok := h.configs[name]
	if !ok {
		writeAPIError(h.logger(), w, http.StatusNotFound, "Not found: TransferConfig "+id)
		return
	}
	if patch.DisplayName != "" {
		cur.DisplayName = patch.DisplayName
	}
	if patch.Schedule != "" {
		cur.Schedule = patch.Schedule
	}
	if patch.Params != nil {
		cur.Params = patch.Params
	}
	if patch.DatasetRegion != "" {
		cur.DatasetRegion = patch.DatasetRegion
	}
	if patch.DestinationDatasetID != "" {
		cur.DestinationDatasetID = patch.DestinationDatasetID
	}
	if patch.DestinationDataset != nil {
		cur.DestinationDataset = patch.DestinationDataset
	}
	if patch.NextRunTime != "" {
		cur.NextRunTime = patch.NextRunTime
	}
	if patch.Disabled != nil {
		cur.Disabled = patch.Disabled
	}
	out := *cur
	writeJSON(h.logger(), w, http.StatusOK, out)
}

func (h *Handler) handleDeleteConfig(w http.ResponseWriter, r *http.Request) {
	project := r.PathValue("projectId")
	location := r.PathValue("location")
	id := r.PathValue("configId")
	name := configName(project, location, id)

	h.mu.Lock()
	if _, ok := h.configs[name]; !ok {
		h.mu.Unlock()
		writeAPIError(h.logger(), w, http.StatusNotFound, "Not found: TransferConfig "+id)
		return
	}
	delete(h.configs, name)
	prefix := name + "/runs/"
	for k := range h.runs {
		if strings.HasPrefix(k, prefix) {
			delete(h.runs, k)
		}
	}
	h.mu.Unlock()
	w.WriteHeader(http.StatusOK)
}

// readOptionalJSONProbe consumes an optional JSON body for the AIP-136
// custom methods (`:checkValidCreds`, `:startManualRuns`). Returning
// nil means the caller may proceed; a non-nil error is the wire-shape
// reason for a 400.
func readOptionalJSONProbe(r *http.Request) error {
	body, err := io.ReadAll(io.LimitReader(r.Body, 1<<20))
	if err != nil {
		return errors.New("invalid body")
	}
	_ = r.Body.Close()
	if len(strings.TrimSpace(string(body))) == 0 {
		return nil
	}
	var probe map[string]any
	if err := json.Unmarshal(body, &probe); err != nil {
		return errors.New("invalid json: " + err.Error())
	}
	return nil
}
