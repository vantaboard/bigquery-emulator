package handlers

import (
	"context"
	"fmt"
	"net/http"

	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
)

// catalogDatasetExists reports whether `datasetID` is registered in the
// engine catalog for `projectID`. When Catalog is nil (gateway-only unit
// tests) the function returns (true, nil) so handlers keep the legacy
// synthesized GET posture.
func catalogDatasetExists(
	ctx context.Context,
	deps Dependencies,
	projectID, datasetID string,
) (bool, error) {
	if deps.Catalog == nil {
		return true, nil
	}
	resp, err := deps.Catalog.ListDatasets(ctx, &enginepb.ListDatasetsRequest{
		ProjectId: projectID,
	})
	if err != nil {
		return false, err
	}
	for _, ref := range resp.GetDatasets() {
		if ref.GetDatasetId() == datasetID {
			return true, nil
		}
	}
	return false, nil
}

// writeDatasetNotFound writes the canonical BigQuery REST 404 for a
// missing dataset resource.
func writeDatasetNotFound(w http.ResponseWriter, projectID, datasetID string) {
	writeError(w, http.StatusNotFound, reasonNotFound,
		fmt.Sprintf("Not found: Dataset %s:%s", projectID, datasetID))
}
