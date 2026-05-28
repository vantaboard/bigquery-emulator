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
	"errors"
	"fmt"
	"net/http"
	"strings"
	"time"
)

type startManualTransferRunsResponse struct {
	Runs []transferRunResource `json:"runs"`
}

func (h *Handler) allocRunID() string {
	n := h.nextRunID.Add(1)
	return fmt.Sprintf("run_%d", n)
}

func scheduledQueryText(params map[string]any) (string, error) {
	if params == nil {
		return "", errors.New("scheduled_query transfer config requires params.query")
	}
	raw, ok := params["query"]
	if !ok || raw == nil {
		return "", errors.New("scheduled_query transfer config requires params.query")
	}
	s, ok := raw.(string)
	if !ok {
		return "", errors.New("params.query must be a string")
	}
	s = strings.TrimSpace(s)
	if s == "" {
		return "", errors.New("params.query must be non-empty")
	}
	return s, nil
}

func destinationDatasetID(cfg *transferConfigResource) string {
	if cfg == nil {
		return ""
	}
	if cfg.DestinationDataset != nil && cfg.DestinationDataset.DatasetReference != nil {
		if did := strings.TrimSpace(cfg.DestinationDataset.DatasetReference.DatasetID); did != "" {
			return did
		}
	}
	return strings.TrimSpace(cfg.DestinationDatasetID)
}

// handleConfigPostSegment dispatches AIP-136 custom-method POST
// endpoints (`{configId}:scheduleRuns`, `:checkValidCreds`,
// `:startManualRuns`). Go's net/http mux can't match a literal
// segment after a wildcard, so we register the parent
// `{configSeg}` and split on the trailing `:op`.
func (h *Handler) handleConfigPostSegment(w http.ResponseWriter, r *http.Request) {
	seg := r.PathValue("configSeg")
	id, action, ok := strings.Cut(seg, ":")
	if !ok || id == "" || action == "" {
		writeAPIError(h.logger(), w, http.StatusNotFound, "Not found")
		return
	}
	project := r.PathValue("projectId")
	location := r.PathValue("location")
	switch action {
	case "scheduleRuns":
		writeAPIError(h.logger(), w, http.StatusNotImplemented,
			"scheduleRuns is not supported by the emulator (no backfill or cron execution)")
	case "checkValidCreds":
		if err := readOptionalJSONProbe(r); err != nil {
			writeAPIError(h.logger(), w, http.StatusBadRequest, err.Error())
			return
		}
		// No live OAuth or vendor credential checks; clients can probe
		// predictably.
		writeJSON(h.logger(), w, http.StatusOK, map[string]any{"hasValidCreds": false})
	case "startManualRuns":
		if err := readOptionalJSONProbe(r); err != nil {
			writeAPIError(h.logger(), w, http.StatusBadRequest, err.Error())
			return
		}
		h.handleStartManualRuns(w, project, location, id)
	default:
		writeAPIError(h.logger(), w, http.StatusNotFound, "Not found")
	}
}

func (h *Handler) handleStartManualRuns(w http.ResponseWriter, project, location, configID string) {
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

	ds := strings.TrimSpace(cp.DataSourceID)
	if ds != dataSourceScheduledQuery {
		writeAPIError(h.logger(), w, http.StatusNotImplemented,
			fmt.Sprintf("manual runs for data source %q are not supported by the emulator", ds))
		return
	}
	if h.Runner == nil {
		writeAPIError(h.logger(), w, http.StatusNotImplemented,
			"scheduled_query execution is not configured (emulator metadata-only mode)")
		return
	}
	sql, err := scheduledQueryText(cp.Params)
	if err != nil {
		writeAPIError(h.logger(), w, http.StatusBadRequest, err.Error())
		return
	}
	defDS := destinationDatasetID(&cp)
	if defDS == "" {
		defDS = strings.TrimSpace(cp.DestinationDatasetID)
	}
	runID := h.allocRunID()
	runFull := runName(project, location, configID, runID)
	now := time.Now().UTC().Format(time.RFC3339Nano)
	run := &transferRunResource{
		Name:          runFull,
		DataSourceID:  cp.DataSourceID,
		Params:        cp.Params,
		DatasetRegion: cp.DatasetRegion,
		ScheduleTime:  now,
		RunTime:       now,
		UpdateTime:    now,
	}
	if cp.DestinationDataset != nil {
		run.DestinationDataset = cp.DestinationDataset
	}
	if err := h.Runner.RunScheduledQueryTransfer(project, location, sql, defDS); err != nil {
		run.State = transferStateFailed
		run.Errors = []any{transferRunErrorPayload(err.Error())}
	} else {
		run.State = transferStateSucceeded
	}
	h.mu.Lock()
	h.runs[runFull] = run
	h.mu.Unlock()
	out := startManualTransferRunsResponse{Runs: []transferRunResource{*run}}
	writeJSON(h.logger(), w, http.StatusOK, out)
}
