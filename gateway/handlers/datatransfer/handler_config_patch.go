package datatransfer

import (
	"encoding/json"
	"errors"
	"io"
	"net/http"
	"strings"
)

// handlePatchConfig honors the `disabled` field on the request body;
// because Disabled is *bool, an explicit `"disabled": false` flips a
// disabled config back on (failing-IT row 15:
// ReEnableTransferConfigIT) and `"disabled": true` disables it (row
// 14: DisableTransferConfigIT). Other fields update only when
// non-zero.
//
// updateMask is parsed from the `updateMask` query parameter (gapic
// REST clients append it). The shallow-emulator port keeps the mask
// advisory: the mask names are not enforced, the body's non-zero
// fields drive the patch.
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
