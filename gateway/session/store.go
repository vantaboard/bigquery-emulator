// Package session is the gateway-side, in-memory BigQuery session registry.
// Sessions are minted when a query job requests createSession=true and are
// reattached on follow-up queries that pass connectionProperties session_id.
package session

import (
	"crypto/rand"
	"encoding/hex"
	"sync"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
)

// Store tracks server-generated session ids for the lifetime of the gateway
// process. State is volatile; restarts wipe the table.
type Store struct {
	mu   sync.RWMutex
	byID map[string]record
}

type record struct {
	projectID string
	location  string
}

// NewStore returns an empty session registry.
func NewStore() *Store {
	return &Store{byID: map[string]record{}}
}

// Resolve returns sessionInfo for a query/job when createSession is set or when
// connectionProperties carries session_id. Returns nil for non-session queries.
func (s *Store) Resolve(
	projectID, location string,
	createSession bool,
	connProps []bqtypes.ConnectionProperty,
) *bqtypes.SessionInfo {
	if s == nil {
		return nil
	}
	if createSession {
		return s.mint(projectID, location)
	}
	if sid := connectionSessionID(connProps); sid != "" {
		s.register(sid, projectID, location)
		return &bqtypes.SessionInfo{SessionID: sid}
	}
	return nil
}

func (s *Store) mint(projectID, location string) *bqtypes.SessionInfo {
	id := newSessionID()
	s.register(id, projectID, location)
	return &bqtypes.SessionInfo{SessionID: id}
}

func (s *Store) register(id, projectID, location string) {
	s.mu.Lock()
	defer s.mu.Unlock()
	if _, ok := s.byID[id]; !ok {
		s.byID[id] = record{projectID: projectID, location: location}
	}
}

func connectionSessionID(props []bqtypes.ConnectionProperty) string {
	for _, p := range props {
		if p.Key == "session_id" && p.Value != "" {
			return p.Value
		}
	}
	return ""
}

func newSessionID() string {
	b := make([]byte, 16)
	_, _ = rand.Read(b)
	return hex.EncodeToString(b)
}
