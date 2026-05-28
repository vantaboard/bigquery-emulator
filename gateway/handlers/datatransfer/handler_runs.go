// Copyright 2026 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

package datatransfer

import (
	"fmt"
	"net/http"
	"sort"
	"strconv"
	"strings"
	"time"
)

type listRunsResponse struct {
	TransferRuns  []transferRunResource `json:"transferRuns"`
	NextPageToken string                `json:"nextPageToken,omitempty"`
}

func (h *Handler) handleListRuns(w http.ResponseWriter, r *http.Request) {
	project := r.PathValue("projectId")
	location := r.PathValue("location")
	configID := r.PathValue("configId")
	prefix := configName(project, location, configID) + "/runs/"

	h.mu.Lock()
	var keys []string
	for k := range h.runs {
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
	out := make([]transferRunResource, 0, len(pageKeys))
	for _, k := range pageKeys {
		if run, ok := h.runs[k]; ok {
			out = append(out, *run)
		}
	}
	resp := listRunsResponse{TransferRuns: out}
	if end < len(keys) {
		resp.NextPageToken = strconv.Itoa(end)
	}
	writeJSON(h.logger(), w, http.StatusOK, resp)
}

func unsupportedCreateRunDataSource(ds string) (string, bool) {
	ds = strings.TrimSpace(ds)
	if ds != "" && ds != dataSourceScheduledQuery {
		return fmt.Sprintf(
			"transfer run creation for data source %q is not supported by the emulator (metadata catalog only)",
			ds,
		), true
	}
	return "", false
}

func (h *Handler) newTransferRun(project, location, configID string, cp *transferConfigResource) *transferRunResource {
	runID := h.allocRunID()
	name := runName(project, location, configID, runID)
	now := time.Now().UTC().Format(time.RFC3339Nano)
	run := &transferRunResource{
		Name:          name,
		DataSourceID:  cp.DataSourceID,
		Params:        cp.Params,
		DatasetRegion: cp.DatasetRegion,
		ScheduleTime:  now,
		RunTime:       now,
		UpdateTime:    now,
		State:         transferStateSucceeded,
	}
	if cp.DestinationDataset != nil {
		run.DestinationDataset = cp.DestinationDataset
	}
	return run
}

// maybeExecuteScheduledQueryOnRun runs SQL when a Runner is wired;
// mutates run state on failure. Returns stop=true with status+message
// only when the input is invalid (bad params); a Runner failure
// itself becomes a FAILED run, not a 4xx.
func (h *Handler) maybeExecuteScheduledQueryOnRun(
	project, location string,
	cp *transferConfigResource,
	run *transferRunResource,
) (stop bool, status int, msg string) {
	if h.Runner == nil || strings.TrimSpace(cp.DataSourceID) != dataSourceScheduledQuery {
		return false, 0, ""
	}
	sql, err := scheduledQueryText(cp.Params)
	if err != nil {
		return true, http.StatusBadRequest, err.Error()
	}
	defDS := destinationDatasetID(cp)
	if defDS == "" {
		defDS = strings.TrimSpace(cp.DestinationDatasetID)
	}
	if err := h.Runner.RunScheduledQueryTransfer(project, location, sql, defDS); err != nil {
		run.State = transferStateFailed
		run.Errors = []any{transferRunErrorPayload(err.Error())}
	} else {
		run.State = transferStateSucceeded
	}
	return false, 0, ""
}

func (h *Handler) handleCreateRun(w http.ResponseWriter, r *http.Request) {
	project := r.PathValue("projectId")
	location := r.PathValue("location")
	configID := r.PathValue("configId")
	cfgName := configName(project, location, configID)

	h.mu.Lock()
	cfg, ok := h.configs[cfgName]
	if !ok {
		h.mu.Unlock()
		writeAPIError(h.logger(), w, http.StatusNotFound, "Not found: TransferConfig "+configID)
		return
	}
	cp := *cfg
	h.mu.Unlock()

	if msg, bad := unsupportedCreateRunDataSource(cp.DataSourceID); bad {
		writeAPIError(h.logger(), w, http.StatusNotImplemented, msg)
		return
	}

	run := h.newTransferRun(project, location, configID, &cp)
	if stop, st, m := h.maybeExecuteScheduledQueryOnRun(project, location, &cp, run); stop {
		writeAPIError(h.logger(), w, st, m)
		return
	}

	h.mu.Lock()
	h.runs[run.Name] = run
	out := *run
	h.mu.Unlock()
	writeJSON(h.logger(), w, http.StatusOK, out)
}

func (h *Handler) handleGetRun(w http.ResponseWriter, r *http.Request) {
	project := r.PathValue("projectId")
	location := r.PathValue("location")
	configID := r.PathValue("configId")
	runID := r.PathValue("runId")
	name := runName(project, location, configID, runID)

	h.mu.Lock()
	defer h.mu.Unlock()
	run, ok := h.runs[name]
	if !ok {
		writeAPIError(h.logger(), w, http.StatusNotFound, "Not found: TransferRun "+runID)
		return
	}
	out := *run
	writeJSON(h.logger(), w, http.StatusOK, out)
}
