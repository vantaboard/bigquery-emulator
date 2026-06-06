package handlers

import (
	"sync"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
)

// MetadataStore caches REST-only Dataset/Table metadata that the engine
// catalog does not yet persist on the C++ side: `labels`,
// `defaultCollation`, `expirationTime`, `rangePartitioning`,
// `timePartitioning`, `clustering`, plus the smaller bookkeeping fields
// (`friendlyName`, `description`). The handler layer populates the
// store from Insert/Patch/Update bodies and reads it back in Get so
// client libraries observe the values they wrote.
//
// Lifetime: in-memory, per-gateway-process. Survives until the gateway
// restarts. For thirdparty test runs, the `THIRDPARTY_FRESH_VOLUME=1`
// path in `taskfiles/thirdparty.yml` wipes the engine volume on
// bringup, which intentionally aligns with the cache being empty at
// startup.
//
// Persistence is a separate (larger) plan: extending the engine
// `RegisterTable` / `DescribeTable` protos and the on-disk meta
// sidecar so values survive restart. Until then the in-memory store is
// the minimum-viable round-trip and is gated on the gateway speaking
// to the engine; gateway-only (`--engine_binary=""`) modes keep the
// echo posture they had before.
//
// Thread-safety: protected by an RWMutex. Lookups (the hot path for
// list samples) take the read lock; mutations take the write lock.
type MetadataStore struct {
	mu       sync.RWMutex
	tables   map[string]bqtypes.Table
	datasets map[string]bqtypes.Dataset
}

// NewMetadataStore returns an empty, thread-safe MetadataStore.
func NewMetadataStore() *MetadataStore {
	return &MetadataStore{
		tables:   map[string]bqtypes.Table{},
		datasets: map[string]bqtypes.Dataset{},
	}
}

func tableKey(projectID, datasetID, tableID string) string {
	return projectID + ":" + datasetID + "." + tableID
}

func datasetKey(projectID, datasetID string) string {
	return projectID + ":" + datasetID
}

// PutTable records the round-trippable metadata fields for a table.
// Only the REST-only fields (labels, expirationTime, rangePartitioning,
// clustering, defaultCollation, friendlyName, description, view, type,
// requirePartitionFilter) are kept; engine-owned fields like Schema /
// NumRows fall through to the engine's DescribeTable response on Get.
func (s *MetadataStore) PutTable(projectID, datasetID, tableID string, t bqtypes.Table) {
	if s == nil {
		return
	}
	s.mu.Lock()
	defer s.mu.Unlock()
	s.tables[tableKey(projectID, datasetID, tableID)] = stripEngineOwnedTableFields(t)
}

// MergeTable overlays sparse PATCH/UPDATE fields onto any cached entry.
func (s *MetadataStore) MergeTable(projectID, datasetID, tableID string, patch bqtypes.Table) {
	if s == nil {
		return
	}
	s.mu.Lock()
	defer s.mu.Unlock()
	key := tableKey(projectID, datasetID, tableID)
	existing := s.tables[key]
	s.tables[key] = mergeTableMetadataOverlay(existing, stripEngineOwnedTableFields(patch))
}

// GetTable returns the cached REST-only metadata for the table and a
// bool indicating whether the entry was present. Callers must merge
// the result with the engine's DescribeTable response themselves to
// build the full GET shape.
func (s *MetadataStore) GetTable(projectID, datasetID, tableID string) (bqtypes.Table, bool) {
	if s == nil {
		return bqtypes.Table{}, false
	}
	s.mu.RLock()
	defer s.mu.RUnlock()
	t, ok := s.tables[tableKey(projectID, datasetID, tableID)]
	return t, ok
}

// DeleteTable evicts the table entry so a subsequent Insert against
// the same ID does not surface stale metadata.
func (s *MetadataStore) DeleteTable(projectID, datasetID, tableID string) {
	if s == nil {
		return
	}
	s.mu.Lock()
	defer s.mu.Unlock()
	delete(s.tables, tableKey(projectID, datasetID, tableID))
}

// PutDataset records the round-trippable metadata fields for a dataset.
func (s *MetadataStore) PutDataset(projectID, datasetID string, ds bqtypes.Dataset) {
	if s == nil {
		return
	}
	s.mu.Lock()
	defer s.mu.Unlock()
	s.datasets[datasetKey(projectID, datasetID)] = stripEngineOwnedDatasetFields(ds)
}

// MergeDataset overlays sparse PATCH/UPDATE fields onto any cached entry.
func (s *MetadataStore) MergeDataset(projectID, datasetID string, patch bqtypes.Dataset) {
	if s == nil {
		return
	}
	s.mu.Lock()
	defer s.mu.Unlock()
	key := datasetKey(projectID, datasetID)
	existing := s.datasets[key]
	s.datasets[key] = mergeDatasetMetadataOverlay(existing, stripEngineOwnedDatasetFields(patch))
}

// GetDataset returns the cached REST-only metadata for the dataset.
func (s *MetadataStore) GetDataset(projectID, datasetID string) (bqtypes.Dataset, bool) {
	if s == nil {
		return bqtypes.Dataset{}, false
	}
	s.mu.RLock()
	defer s.mu.RUnlock()
	ds, ok := s.datasets[datasetKey(projectID, datasetID)]
	return ds, ok
}

// DeleteDataset evicts the dataset entry. Does NOT cascade into the
// per-table entries: DatasetDelete with `deleteContents=true` does
// that explicitly because the handler knows the dataset's tables.
func (s *MetadataStore) DeleteDataset(projectID, datasetID string) {
	if s == nil {
		return
	}
	s.mu.Lock()
	defer s.mu.Unlock()
	delete(s.datasets, datasetKey(projectID, datasetID))
}

// DeleteTablesInDataset removes every cached table entry that belongs
// to the given dataset. Called from DatasetDelete when the caller sets
// `deleteContents=true` so a recreate with the same dataset ID does
// not inherit stale table metadata.
func (s *MetadataStore) DeleteTablesInDataset(projectID, datasetID string) {
	if s == nil {
		return
	}
	prefix := projectID + ":" + datasetID + "."
	s.mu.Lock()
	defer s.mu.Unlock()
	for k := range s.tables {
		if len(k) > len(prefix) && k[:len(prefix)] == prefix {
			delete(s.tables, k)
		}
	}
}

// stripEngineOwnedTableFields keeps only the REST-only metadata
// fields. Bookkeeping fields the handler stamps (Kind/ID/Timestamps)
// and engine-owned fields (Schema/NumRows/NumBytes) are dropped so a
// PATCH that echoes the prior GET cannot recursively store a stale
// schema. The Get handler re-merges the engine-side schema on every
// read.
func stripEngineOwnedTableFields(t bqtypes.Table) bqtypes.Table {
	return bqtypes.Table{
		FriendlyName:              t.FriendlyName,
		Description:               t.Description,
		Labels:                    t.Labels,
		ExpirationTime:            t.ExpirationTime,
		RangePartitioning:         t.RangePartitioning,
		TimePartitioning:          t.TimePartitioning,
		Clustering:                t.Clustering,
		DefaultCollation:          t.DefaultCollation,
		DefaultCollationSet:       t.DefaultCollationSet,
		Type:                      t.Type,
		View:                      t.View,
		MaterializedView:          t.MaterializedView,
		RequirePartitionFilter:    t.RequirePartitionFilter,
		ExternalDataConfiguration: t.ExternalDataConfiguration,
		EncryptionConfiguration:   t.EncryptionConfiguration,
		Schema:                    bqtypes.ExtractSchemaPolicyOverlay(t.Schema),
	}
}

// stripEngineOwnedDatasetFields is the dataset analogue.
func stripEngineOwnedDatasetFields(ds bqtypes.Dataset) bqtypes.Dataset {
	return bqtypes.Dataset{
		FriendlyName:                 ds.FriendlyName,
		Description:                  ds.Description,
		Location:                     ds.Location,
		Access:                       ds.Access,
		Labels:                       ds.Labels,
		DefaultTableExpirationMs:     ds.DefaultTableExpirationMs,
		DefaultPartitionExpirationMs: ds.DefaultPartitionExpirationMs,
		DefaultCollation:             ds.DefaultCollation,
		DefaultCollationSet:          ds.DefaultCollationSet,
	}
}

// applyTableMetadataOverlay merges the cached REST-only fields onto
// the engine-derived table resource. Cached values win over the engine
// shape for the REST-only fields, but engine-owned fields (Schema,
// NumRows, ...) are preserved.
func applyTableMetadataOverlay(base bqtypes.Table, overlay bqtypes.Table) bqtypes.Table {
	if overlay.FriendlyName != "" {
		base.FriendlyName = overlay.FriendlyName
	}
	if overlay.Description != "" {
		base.Description = overlay.Description
	}
	if overlay.Labels != nil {
		base.Labels = overlay.Labels
	}
	if overlay.ExpirationTime != "" {
		base.ExpirationTime = overlay.ExpirationTime
	}
	if overlay.RangePartitioning != nil {
		base.RangePartitioning = overlay.RangePartitioning
	}
	if overlay.TimePartitioning != nil {
		base.TimePartitioning = overlay.TimePartitioning
	}
	if overlay.Clustering != nil {
		base.Clustering = overlay.Clustering
	}
	if overlay.DefaultCollationSet {
		base.DefaultCollation = overlay.DefaultCollation
		base.DefaultCollationSet = true
	}
	if overlay.Type != "" {
		base.Type = overlay.Type
	}
	if overlay.View != nil {
		base.View = overlay.View
	}
	if overlay.MaterializedView != nil {
		base.MaterializedView = overlay.MaterializedView
	}
	if overlay.RequirePartitionFilter != nil {
		base.RequirePartitionFilter = overlay.RequirePartitionFilter
	}
	if overlay.ExternalDataConfiguration != nil {
		base.ExternalDataConfiguration = overlay.ExternalDataConfiguration
	}
	if overlay.EncryptionConfiguration != nil {
		base.EncryptionConfiguration = overlay.EncryptionConfiguration
	}
	if overlay.Schema != nil {
		base.Schema = bqtypes.MergeSchemaPolicyTags(base.Schema, overlay.Schema)
	}
	return base
}

// mergeTableMetadataOverlay merges sparse metadata updates onto a
// cached table entry. Unlike applyTableMetadataOverlay (used at GET
// time against engine-derived resources), this helper treats empty
// strings and nil maps as "not provided" so PATCH bodies can carry
// only the fields being changed.
func mergeTableMetadataOverlay(base, patch bqtypes.Table) bqtypes.Table {
	if patch.FriendlyName != "" {
		base.FriendlyName = patch.FriendlyName
	}
	if patch.Description != "" {
		base.Description = patch.Description
	}
	if patch.LabelsPatchPresent() {
		base.Labels = bqtypes.ApplyLabelsPatch(
			base.Labels, true, patch.Labels, patch.LabelsToDelete(),
		)
	} else if patch.Labels != nil {
		base.Labels = patch.Labels
	}
	if patch.ExpirationTime != "" {
		base.ExpirationTime = patch.ExpirationTime
	}
	if patch.RangePartitioning != nil {
		base.RangePartitioning = patch.RangePartitioning
	}
	if patch.TimePartitioning != nil {
		base.TimePartitioning = patch.TimePartitioning
	}
	if patch.Clustering != nil {
		base.Clustering = patch.Clustering
	}
	if patch.DefaultCollationSet {
		base.DefaultCollation = patch.DefaultCollation
		base.DefaultCollationSet = true
	}
	if patch.Type != "" {
		base.Type = patch.Type
	}
	if patch.View != nil {
		base.View = patch.View
	}
	if patch.MaterializedView != nil {
		base.MaterializedView = patch.MaterializedView
	}
	if patch.RequirePartitionFilter != nil {
		base.RequirePartitionFilter = patch.RequirePartitionFilter
	}
	if patch.ExternalDataConfiguration != nil {
		base.ExternalDataConfiguration = patch.ExternalDataConfiguration
	}
	if patch.EncryptionConfiguration != nil {
		base.EncryptionConfiguration = patch.EncryptionConfiguration
	}
	if patch.Schema != nil {
		base.Schema = bqtypes.MergeSchemaPolicyTags(base.Schema, patch.Schema)
	}
	return base
}

// applyDatasetMetadataOverlay is the dataset analogue.
func applyDatasetMetadataOverlay(base bqtypes.Dataset, overlay bqtypes.Dataset) bqtypes.Dataset {
	if overlay.FriendlyName != "" {
		base.FriendlyName = overlay.FriendlyName
	}
	if overlay.Description != "" {
		base.Description = overlay.Description
	}
	if overlay.Location != "" {
		base.Location = overlay.Location
	}
	if overlay.Access != nil {
		base.Access = overlay.Access
	}
	if overlay.Labels != nil {
		base.Labels = overlay.Labels
	}
	if overlay.DefaultTableExpirationMs != "" {
		base.DefaultTableExpirationMs = overlay.DefaultTableExpirationMs
	}
	if overlay.DefaultPartitionExpirationMs != "" {
		base.DefaultPartitionExpirationMs = overlay.DefaultPartitionExpirationMs
	}
	if overlay.DefaultCollationSet {
		base.DefaultCollation = overlay.DefaultCollation
		base.DefaultCollationSet = true
	}
	return base
}

// mergeDatasetMetadataOverlay merges sparse dataset metadata updates.
func mergeDatasetMetadataOverlay(base, patch bqtypes.Dataset) bqtypes.Dataset {
	if patch.FriendlyName != "" {
		base.FriendlyName = patch.FriendlyName
	}
	if patch.Description != "" {
		base.Description = patch.Description
	}
	if patch.Location != "" {
		base.Location = patch.Location
	}
	if patch.Access != nil {
		base.Access = patch.Access
	}
	if patch.LabelsPatchPresent() {
		base.Labels = bqtypes.ApplyLabelsPatch(
			base.Labels, true, patch.Labels, patch.LabelsToDelete(),
		)
	} else if patch.Labels != nil {
		base.Labels = patch.Labels
	}
	if patch.DefaultTableExpirationMs != "" {
		base.DefaultTableExpirationMs = patch.DefaultTableExpirationMs
	}
	if patch.DefaultPartitionExpirationMs != "" {
		base.DefaultPartitionExpirationMs = patch.DefaultPartitionExpirationMs
	}
	if patch.DefaultCollationSet {
		base.DefaultCollation = patch.DefaultCollation
		base.DefaultCollationSet = true
	}
	return base
}
