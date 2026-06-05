// Package snapshots retains soft-deleted table data so COPY jobs can
// read snapshot decorators (table@epoch) for undelete samples.
package snapshots

import (
	"context"
	"fmt"
	"slices"
	"strconv"
	"strings"
	"sync"
	"time"

	"github.com/vantaboard/bigquery-emulator/gateway/enginepb"
)

const defaultPageSize = 10_000

// Entry is a point-in-time capture of a table's schema and rows.
type Entry struct {
	Schema         *enginepb.TableSchema
	Rows           []*enginepb.DataRow
	CreationTimeMs int64
	DeletionTimeMs int64
}

// Store retains deleted-table snapshots and live-table creation times
// for snapshot decorator resolution.
type Store struct {
	mu            sync.RWMutex
	creationTimes map[string]int64
	deleted       map[string][]Entry
}

// NewStore returns an empty snapshot store.
func NewStore() *Store {
	return &Store{
		creationTimes: map[string]int64{},
		deleted:       map[string][]Entry{},
	}
}

func tableKey(projectID, datasetID, tableID string) string {
	return projectID + ":" + datasetID + "." + tableID
}

// RecordCreation stamps the creation time for a live table. Called when
// a table is first registered so tables.get returns a stable epoch.
func (s *Store) RecordCreation(projectID, datasetID, tableID string, createdMs int64) {
	if s == nil {
		return
	}
	s.mu.Lock()
	defer s.mu.Unlock()
	key := tableKey(projectID, datasetID, tableID)
	if _, ok := s.creationTimes[key]; !ok {
		s.creationTimes[key] = createdMs
	}
}

// CreationTimeMs returns the recorded creation epoch for a live table.
func (s *Store) CreationTimeMs(projectID, datasetID, tableID string) (int64, bool) {
	if s == nil {
		return 0, false
	}
	s.mu.RLock()
	defer s.mu.RUnlock()
	t, ok := s.creationTimes[tableKey(projectID, datasetID, tableID)]
	return t, ok
}

// CaptureBeforeDelete snapshots schema and rows before DropTable.
func (s *Store) CaptureBeforeDelete(ctx context.Context, catalog enginepb.CatalogClient,
	projectID, datasetID, tableID string,
) error {
	if s == nil || catalog == nil {
		return nil
	}
	ref := &enginepb.TableRef{
		ProjectId: projectID,
		DatasetId: datasetID,
		TableId:   tableID,
	}
	desc, err := catalog.DescribeTable(ctx, &enginepb.DescribeTableRequest{Table: ref})
	if err != nil {
		return fmt.Errorf("describe table for snapshot: %w", err)
	}
	rows, err := listAllRows(ctx, catalog, ref, desc.GetSchema())
	if err != nil {
		return err
	}
	now := time.Now().UTC().UnixMilli()
	s.mu.Lock()
	defer s.mu.Unlock()
	key := tableKey(projectID, datasetID, tableID)
	created := s.creationTimes[key]
	if created == 0 {
		created = now
	}
	s.deleted[key] = append(s.deleted[key], Entry{
		Schema:         desc.GetSchema(),
		Rows:           rows,
		CreationTimeMs: created,
		DeletionTimeMs: now,
	})
	delete(s.creationTimes, key)
	return nil
}

// ResolveAtEpoch returns snapshot data for table@epoch decorators.
func (s *Store) ResolveAtEpoch(projectID, datasetID, tableID string, epochMs int64,
) (*Entry, error) {
	if s == nil {
		return nil, fmt.Errorf("table %s.%s.%s@%d not found (snapshot store unavailable)",
			projectID, datasetID, tableID, epochMs)
	}
	s.mu.RLock()
	defer s.mu.RUnlock()
	entries := s.deleted[tableKey(projectID, datasetID, tableID)]
	for _, v := range slices.Backward(entries) {
		e := v
		if epochMs >= e.CreationTimeMs && epochMs <= e.DeletionTimeMs {
			return &e, nil
		}
	}
	return nil, fmt.Errorf("not found: Table %s:%s.%s@%d", projectID, datasetID, tableID, epochMs)
}

// ParseDecorator splits tableId@epoch into base id and epoch milliseconds.
// Supports absolute (@123) and relative (@-3600000) decorators.
func ParseDecorator(tableID string) (base string, epochMs int64, decorated bool) {
	at := strings.LastIndex(tableID, "@")
	if at <= 0 || at == len(tableID)-1 {
		return tableID, 0, false
	}
	base = tableID[:at]
	raw := tableID[at+1:]
	if strings.HasPrefix(raw, "-") {
		offset, err := strconv.ParseInt(raw, 10, 64)
		if err != nil {
			return tableID, 0, false
		}
		return base, time.Now().UTC().UnixMilli() + offset, true
	}
	epoch, err := strconv.ParseInt(raw, 10, 64)
	if err != nil {
		return tableID, 0, false
	}
	return base, epoch, true
}

func listAllRows(ctx context.Context, catalog enginepb.CatalogClient,
	ref *enginepb.TableRef, schema *enginepb.TableSchema,
) ([]*enginepb.DataRow, error) {
	var out []*enginepb.DataRow
	start := int64(0)
	for {
		resp, err := catalog.ListRows(ctx, &enginepb.ListRowsRequest{
			Table:      ref,
			StartIndex: start,
			MaxResults: defaultPageSize,
		})
		if err != nil {
			return nil, fmt.Errorf("list rows for snapshot: %w", err)
		}
		rows := resp.GetRows()
		if len(rows) == 0 {
			break
		}
		out = append(out, rows...)
		start += int64(len(rows))
		if start >= resp.GetTotalRows() {
			break
		}
	}
	_ = schema
	return out, nil
}
