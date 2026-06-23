package handlers

import (
	"context"
	"strconv"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
	"github.com/vantaboard/bigquery-emulator/gateway/load"
)

// tableListItem builds one tables.list entry from Catalog.ListTables
// output plus metadata overlay and optional DescribeTable view query.
func tableListItem(ctx context.Context, deps Dependencies, ref *enginepb.TableRef) map[string]any {
	overlay, hasOverlay := deps.Metadata.GetTable(
		ref.GetProjectId(), ref.GetDatasetId(), ref.GetTableId(),
	)
	labels := bqtypes.ResourceLabels{}
	if hasOverlay && overlay.Labels != nil {
		labels = overlay.Labels
	}
	tableType := defaultTableType
	if hasOverlay && overlay.Type != "" {
		tableType = overlay.Type
	} else if refType := ref.GetTableType(); refType != "" {
		tableType = refType
	}
	item := map[string]any{
		"kind": tableKind,
		"id": ref.GetProjectId() + ":" + ref.GetDatasetId() +
			"." + ref.GetTableId(),
		"tableReference": bqtypes.TableReference{
			ProjectID: ref.GetProjectId(),
			DatasetID: ref.GetDatasetId(),
			TableID:   ref.GetTableId(),
		},
		"type":   tableType,
		"labels": labels,
	}
	if hasOverlay {
		mergeListViewQueryFromOverlay(overlay, item)
	}
	if _, hasView := item["view"]; !hasView {
		if _, hasMV := item["materializedView"]; !hasMV {
			mergeListViewQueryFromCatalog(ctx, deps, ref, tableType, item)
		}
	}
	return item
}

func mergeListViewQueryFromOverlay(overlay bqtypes.Table, item map[string]any) {
	if overlay.View != nil && overlay.View.Query != "" {
		item["view"] = map[string]any{discoveryMethodQuery: overlay.View.Query}
	}
	if overlay.MaterializedView != nil && overlay.MaterializedView.Query != "" {
		item["materializedView"] = map[string]any{
			discoveryMethodQuery: overlay.MaterializedView.Query,
		}
	}
}

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
		if ct, ok := deps.Snapshots.CreationTimeMs(projectID, datasetID, tableID); ok && t.CreationTime == "" {
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
	applyTableStorageStats(&t)
	return t
}

// mergeListViewQueryFromCatalog attaches view.query (or
// materializedView.query) from Catalog.DescribeTable when the metadata
// overlay did not stash DDL text — e.g. three-segment backtick CREATE
// VIEW forms the gateway parser does not mirror into overlay.
func mergeListViewQueryFromCatalog(
	ctx context.Context,
	deps Dependencies,
	ref *enginepb.TableRef,
	tableType string,
	item map[string]any,
) {
	if deps.Catalog == nil {
		return
	}
	isView := tableType == viewTableType || ref.GetTableType() == viewTableType
	isMV := tableType == materializedViewTableType ||
		ref.GetTableType() == materializedViewTableType
	if !isView && !isMV {
		return
	}
	desc, err := deps.Catalog.DescribeTable(ctx, &enginepb.DescribeTableRequest{
		Table: &enginepb.TableRef{
			ProjectId: ref.GetProjectId(),
			DatasetId: ref.GetDatasetId(),
			TableId:   ref.GetTableId(),
		},
	})
	if err != nil {
		return
	}
	if isMV {
		if q := desc.GetViewQuery(); q != "" {
			item["materializedView"] = map[string]any{discoveryMethodQuery: q}
		}
		return
	}
	if q := desc.GetViewQuery(); q != "" {
		item["view"] = map[string]any{discoveryMethodQuery: q}
	}
}

// applyTableStorageStats fills output-only byte counters so the console
// Details tab shows explicit zeros instead of em dashes. NumRows is
// computed from Catalog.ListRows; byte breakdowns are stubbed until
// the engine exposes storage statistics RPCs.
func applyTableStorageStats(t *bqtypes.Table) {
	if t.NumBytes == "" {
		t.NumBytes = "0"
	}
	if t.NumLongTermBytes == "" {
		t.NumLongTermBytes = "0"
	}
	if t.NumActiveLogicalBytes == "" {
		t.NumActiveLogicalBytes = "0"
	}
	if t.NumTotalLogicalBytes == "" {
		t.NumTotalLogicalBytes = "0"
	}
	if t.NumCurrentPhysicalBytes == "" {
		t.NumCurrentPhysicalBytes = "0"
	}
	if t.NumPhysicalBytes == "" {
		t.NumPhysicalBytes = "0"
	}
	if t.NumActivePhysicalBytes == "" {
		t.NumActivePhysicalBytes = "0"
	}
	if t.NumLongTermPhysicalBytes == "" {
		t.NumLongTermPhysicalBytes = "0"
	}
	if t.NumTimeTravelPhysicalBytes == "" {
		t.NumTimeTravelPhysicalBytes = "0"
	}
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
