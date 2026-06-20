package handlers

import (
	"context"
	"strconv"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
	"github.com/vantaboard/bigquery-emulator/gateway/load"
)

// catalogTable builds the REST Table resource the same way TableGet does
// after a successful DescribeTable (engine schema + metadata overlay).
func catalogTable(
	ctx context.Context,
	deps Dependencies,
	projectID, datasetID, tableID string,
	resp *enginepb.DescribeTableResponse,
) bqtypes.Table {
	t := tableFromDescribeResponse(resp)
	if overlay, ok := deps.Metadata.GetDataset(projectID, datasetID); ok && overlay.Location != "" {
		t.Location = overlay.Location
	}
	if overlay, ok := deps.Metadata.GetTable(projectID, datasetID, tableID); ok {
		t = applyTableMetadataOverlay(t, overlay)
	}
	if t.DefaultCollation != "" {
		t.Schema = bqtypes.ApplyDefaultCollationToStringFields(t.Schema, t.DefaultCollation)
	}
	if deps.Snapshots != nil {
		if ct, ok := deps.Snapshots.CreationTimeMs(projectID, datasetID, tableID); ok {
			t.CreationTime = strconv.FormatInt(ct, 10)
		}
	}
	if deps.Catalog != nil {
		if rowsResp, listErr := deps.Catalog.ListRows(ctx, &enginepb.ListRowsRequest{
			Table: &enginepb.TableRef{
				ProjectId: projectID,
				DatasetId: datasetID,
				TableId:   tableID,
			},
			StartIndex: 0,
			MaxResults: 0,
		}); listErr == nil {
			t.NumRows = strconv.FormatInt(rowsResp.GetTotalRows(), 10)
		} else if t.NumRows == "" {
			t.NumRows = "0"
		}
	}
	return t
}

// syncPatchedTableSchema registers schema fields added via tables.patch
// (setMetadata) so tables.get returns engine-backed column types instead
// of overlay-only stubs.
func syncPatchedTableSchema(
	ctx context.Context,
	deps Dependencies,
	projectID, datasetID, tableID string,
	patchSchema *bqtypes.TableSchema,
) error {
	if deps.Catalog == nil || patchSchema == nil || len(patchSchema.Fields) == 0 {
		return nil
	}
	tableRef := &enginepb.TableRef{
		ProjectId: projectID,
		DatasetId: datasetID,
		TableId:   tableID,
	}
	desc, err := deps.Catalog.DescribeTable(ctx, &enginepb.DescribeTableRequest{Table: tableRef})
	if err != nil {
		return err
	}
	existing := schemaFromProto(desc.GetSchema())
	merged, changed, err := load.MergeSchemasForTablePatch(existing, patchSchema)
	if err != nil {
		return err
	}
	if !changed {
		return nil
	}
	_, err = load.ApplySchemaUpdate(ctx, deps.Catalog, tableRef, merged, load.TablePatchSchemaOptions)
	return err
}
