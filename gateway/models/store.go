// Package models is the gateway-side in-memory registry of BigQuery ML
// model metadata registered by CREATE MODEL DDL stubs. REST handlers
// and query jobs register models here so client libraries can round-trip
// list/get/delete without a trained-model store.
package models

import (
	"crypto/rand"
	"encoding/hex"
	"maps"
	"slices"
	"strings"
	"sync"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
)

// Store holds model metadata keyed by projectId:datasetId.modelId.
type Store struct {
	mu     sync.RWMutex
	models map[string]bqtypes.Model
}

// NewStore returns an empty model registry.
func NewStore() *Store {
	return &Store{models: map[string]bqtypes.Model{}}
}

func modelKey(projectID, datasetID, modelID string) string {
	return projectID + ":" + datasetID + "." + modelID
}

// Upsert registers or replaces model metadata.
func (s *Store) Upsert(m bqtypes.Model) {
	if s == nil {
		return
	}
	ref := m.ModelReference
	key := modelKey(ref.ProjectID, ref.DatasetID, ref.ModelID)
	s.mu.Lock()
	defer s.mu.Unlock()
	s.models[key] = cloneModel(m)
}

// Get returns a model snapshot and whether it was found.
func (s *Store) Get(projectID, datasetID, modelID string) (bqtypes.Model, bool) {
	if s == nil {
		return bqtypes.Model{}, false
	}
	s.mu.RLock()
	defer s.mu.RUnlock()
	m, ok := s.models[modelKey(projectID, datasetID, modelID)]
	return cloneModel(m), ok
}

// Delete removes a model. Returns false when absent.
func (s *Store) Delete(projectID, datasetID, modelID string) bool {
	if s == nil {
		return false
	}
	s.mu.Lock()
	defer s.mu.Unlock()
	key := modelKey(projectID, datasetID, modelID)
	if _, ok := s.models[key]; !ok {
		return false
	}
	delete(s.models, key)
	return true
}

// List returns every model in the dataset, optionally filtered by a
// BigQuery list filter string (only `model_id=<id>` is supported today).
func (s *Store) List(projectID, datasetID, filter string) []bqtypes.Model {
	if s == nil {
		return nil
	}
	wantID := parseModelIDFilter(filter)
	prefix := projectID + ":" + datasetID + "."
	s.mu.RLock()
	defer s.mu.RUnlock()
	out := make([]bqtypes.Model, 0)
	for key, m := range s.models {
		if !strings.HasPrefix(key, prefix) {
			continue
		}
		if wantID != "" && m.ModelReference.ModelID != wantID {
			continue
		}
		out = append(out, cloneModel(m))
	}
	slices.SortFunc(out, func(a, b bqtypes.Model) int {
		return strings.Compare(a.ModelReference.ModelID, b.ModelReference.ModelID)
	})
	return out
}

func parseModelIDFilter(filter string) string {
	filter = strings.TrimSpace(filter)
	if filter == "" {
		return ""
	}
	const prefix = "model_id="
	if strings.HasPrefix(filter, prefix) {
		return strings.TrimSpace(filter[len(prefix):])
	}
	return ""
}

// MintEtag returns a random etag for optimistic concurrency.
func MintEtag() string {
	var b [8]byte
	_, _ = rand.Read(b[:])
	return hex.EncodeToString(b[:])
}

func cloneModel(m bqtypes.Model) bqtypes.Model {
	out := m
	if len(m.Labels) > 0 {
		out.Labels = make(map[string]string, len(m.Labels))
		maps.Copy(out.Labels, m.Labels)
	}
	return out
}
