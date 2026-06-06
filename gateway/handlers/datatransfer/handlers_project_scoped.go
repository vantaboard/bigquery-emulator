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
	dsFilter := parseDataSourceIDsFilter(r.URL.Query())

	h.mu.Lock()
	var keys []string
	for k := range h.configs {
		if strings.HasPrefix(k, locPrefix) && strings.Contains(k, "/transferConfigs/") {
			keys = append(keys, k)
		}
	}
	h.mu.Unlock()
	sort.Strings(keys)

	h.mu.Lock()
	filtered := make([]string, 0, len(keys))
	for _, k := range keys {
		if c, ok := h.configs[k]; ok && configMatchesDataSourceFilter(c, dsFilter) {
			filtered = append(filtered, k)
		}
	}
	h.mu.Unlock()

	start, end := pageWindow(len(filtered), r.URL.Query().Get("pageSize"), r.URL.Query().Get("pageToken"))
	pageKeys := filtered[start:end]

	h.mu.Lock()
	defer h.mu.Unlock()
	out := make([]transferConfigResource, 0, len(pageKeys))
	for _, k := range pageKeys {
		if c, ok := h.configs[k]; ok {
			out = append(out, *c)
		}
	}
	resp := listConfigsResponse{TransferConfigs: out}
	if end < len(filtered) {
		resp.NextPageToken = strconv.Itoa(end)
	}
	writeJSON(h.logger(), w, http.StatusOK, resp)
}

// normalizeTransferConfigInput normalizes the destination oneof gapic
// REST clients send (`destinationDatasetId` and/or nested
// `destinationDataset.datasetReference`) so the in-memory record
// carries both wire forms.
func normalizeTransferConfigInput(projectID string, in *transferConfigResource) {
	if in == nil {
		return
	}
	if strings.TrimSpace(in.DestinationDatasetID) == "" &&
		in.DestinationDataset != nil &&
		in.DestinationDataset.DatasetReference != nil {
		in.DestinationDatasetID = strings.TrimSpace(
			in.DestinationDataset.DatasetReference.DatasetID)
	}
	did := strings.TrimSpace(in.DestinationDatasetID)
	if did == "" {
		return
	}
	if in.DestinationDataset == nil {
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
		return
	}
	if in.DestinationDataset.DatasetReference == nil {
		in.DestinationDataset.DatasetReference = &struct {
			ProjectID string `json:"projectId,omitempty"`
			DatasetID string `json:"datasetId,omitempty"`
		}{ProjectID: projectID, DatasetID: did}
		return
	}
	ref := in.DestinationDataset.DatasetReference
	if strings.TrimSpace(ref.DatasetID) == "" {
		ref.DatasetID = did
	}
	if strings.TrimSpace(ref.ProjectID) == "" {
		ref.ProjectID = projectID
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
