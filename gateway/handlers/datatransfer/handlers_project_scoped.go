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
	"encoding/json"
	"io"
	"net/http"
	"sort"
	"strconv"
	"strings"
)

// emulatorDefaultTransferLocation matches the location BigQuery gapic
// REST clients assume when they POST to project-scoped
// `.../projects/{p}/transferConfigs` (no `/locations/` segment in the
// URL). Live BigQuery routes those to the multi-region `us`; the
// emulator stores them at the same key so the per-location LIST below
// can still surface them.
const emulatorDefaultTransferLocation = "us"

func (h *Handler) handleListConfigsProjectScoped(w http.ResponseWriter, r *http.Request) {
	project := r.PathValue("projectId")
	locPrefix := "projects/" + project + "/locations/"

	h.mu.Lock()
	var keys []string
	for k := range h.configs {
		if strings.HasPrefix(k, locPrefix) && strings.Contains(k, "/transferConfigs/") {
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

// normalizeTransferConfigInput projects a bare `destinationDatasetId`
// onto the proto-shaped `destinationDataset.datasetReference` so the
// in-memory record carries both forms (gapic clients consume one or
// the other depending on transport).
func normalizeTransferConfigInput(projectID string, in *transferConfigResource) {
	did := strings.TrimSpace(in.DestinationDatasetID)
	if did == "" || in.DestinationDataset != nil {
		return
	}
	in.DestinationDataset = &struct {
		DatasetReference *struct {
			ProjectID string `json:"projectId,omitempty"`
			DatasetID string `json:"datasetId,omitempty"`
		} `json:"datasetReference,omitempty"`
	}{
		DatasetReference: &struct {
			ProjectID string `json:"projectId,omitempty"`
			DatasetID string `json:"datasetId,omitempty"`
		}{ProjectID: projectID, DatasetID: did},
	}
}

func (h *Handler) handleCreateConfigProjectScoped(w http.ResponseWriter, r *http.Request) {
	project := r.PathValue("projectId")
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
	h.finishCreateTransferConfig(w, project, emulatorDefaultTransferLocation, in)
}
