// Package routines is the gateway-side in-memory registry of BigQuery
// Routine resources (UDFs, TVFs, stored procedures). REST handlers
// and DDL query jobs register routines here so client libraries can
// round-trip insert/get/list/update/delete without an engine catalog RPC.
package routines

import (
	"crypto/rand"
	"encoding/hex"
	"slices"
	"strings"
	"sync"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
)

// Store holds routines keyed by projectId:datasetId.routineId.
type Store struct {
	mu       sync.RWMutex
	routines map[string]bqtypes.Routine
}

// NewStore returns an empty routine registry.
func NewStore() *Store {
	return &Store{
		routines: map[string]bqtypes.Routine{},
	}
}

func routineKey(projectID, datasetID, routineID string) string {
	return projectID + ":" + datasetID + "." + routineID
}

// Insert registers a new routine. Returns false when the key exists.
func (s *Store) Insert(rt bqtypes.Routine) bool {
	if s == nil {
		return false
	}
	ref := rt.RoutineReference
	key := routineKey(ref.ProjectID, ref.DatasetID, ref.RoutineID)
	s.mu.Lock()
	defer s.mu.Unlock()
	if _, ok := s.routines[key]; ok {
		return false
	}
	s.routines[key] = cloneRoutine(rt)
	return true
}

// Upsert registers or replaces a routine (CREATE OR REPLACE DDL).
func (s *Store) Upsert(rt bqtypes.Routine) {
	if s == nil {
		return
	}
	ref := rt.RoutineReference
	key := routineKey(ref.ProjectID, ref.DatasetID, ref.RoutineID)
	s.mu.Lock()
	defer s.mu.Unlock()
	s.routines[key] = cloneRoutine(rt)
}

// Get returns a routine snapshot and whether it was found.
func (s *Store) Get(projectID, datasetID, routineID string) (bqtypes.Routine, bool) {
	if s == nil {
		return bqtypes.Routine{}, false
	}
	s.mu.RLock()
	defer s.mu.RUnlock()
	rt, ok := s.routines[routineKey(projectID, datasetID, routineID)]
	return cloneRoutine(rt), ok
}

// Delete removes a routine. Returns false when absent.
func (s *Store) Delete(projectID, datasetID, routineID string) bool {
	if s == nil {
		return false
	}
	s.mu.Lock()
	defer s.mu.Unlock()
	key := routineKey(projectID, datasetID, routineID)
	if _, ok := s.routines[key]; !ok {
		return false
	}
	delete(s.routines, key)
	return true
}

// List returns routines in the dataset, optionally filtered by
// routineType (filter format: routineType:SCALAR_FUNCTION).
func (s *Store) List(projectID, datasetID, filter string) []bqtypes.Routine {
	if s == nil {
		return nil
	}
	wantType := parseRoutineTypeFilter(filter)
	prefix := projectID + ":" + datasetID + "."
	s.mu.RLock()
	defer s.mu.RUnlock()
	keys := make([]string, 0, len(s.routines))
	for k := range s.routines {
		if strings.HasPrefix(k, prefix) {
			keys = append(keys, k)
		}
	}
	slices.Sort(keys)
	out := make([]bqtypes.Routine, 0, len(keys))
	for _, k := range keys {
		rt := s.routines[k]
		if wantType != "" && rt.RoutineType != wantType {
			continue
		}
		out = append(out, cloneRoutine(rt))
	}
	return out
}

func parseRoutineTypeFilter(filter string) string {
	const prefix = "routineType:"
	if filter == "" || !strings.HasPrefix(filter, prefix) {
		return ""
	}
	return strings.TrimSpace(filter[len(prefix):])
}

func cloneRoutine(rt bqtypes.Routine) bqtypes.Routine {
	return rt
}

// MintEtag returns a random hex etag for a routine resource.
func MintEtag() string {
	var b [8]byte
	_, _ = rand.Read(b[:])
	return hex.EncodeToString(b[:])
}
