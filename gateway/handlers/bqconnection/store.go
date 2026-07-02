package bqconnection

import (
	"encoding/json"
	"errors"
	"os"
	"path/filepath"
	"sync"

	"cloud.google.com/go/bigquery/connection/apiv1/connectionpb"
	"github.com/vantaboard/bigquery-emulator/gateway/external/sourceconfig"
	"google.golang.org/protobuf/encoding/protojson"
)

const registryDir = "_registry"

// Store persists Connection records under $data_dir/external/connections/_registry/.
type Store struct {
	path   string
	mu     sync.RWMutex
	byName map[string]*connectionpb.Connection
}

// OpenStore loads or creates the connection registry for dataDir.
func OpenStore(cfg *sourceconfig.Config) (*Store, error) {
	root := ""
	if cfg != nil {
		root = cfg.ConnectionFixtureRoot()
	}
	if root == "" {
		return &Store{byName: map[string]*connectionpb.Connection{}}, nil
	}
	dir := filepath.Join(root, registryDir)
	if err := os.MkdirAll(dir, 0o750); err != nil {
		return nil, err
	}
	path := filepath.Join(dir, "connections.json")
	s := &Store{path: path, byName: map[string]*connectionpb.Connection{}}
	if err := s.load(); err != nil {
		return nil, err
	}
	return s, nil
}

func (s *Store) load() error {
	if s == nil || s.path == "" {
		return nil
	}
	raw, err := os.ReadFile(s.path) //nolint:gosec // operator-controlled data dir
	if err != nil {
		if errors.Is(err, os.ErrNotExist) {
			return nil
		}
		return err
	}
	var envelope struct {
		Connections []*connectionpb.Connection `json:"connections"`
	}
	if err := json.Unmarshal(raw, &envelope); err != nil {
		return err
	}
	for _, c := range envelope.Connections {
		if c == nil || c.Name == "" {
			continue
		}
		s.byName[c.Name] = c
	}
	return nil
}

func (s *Store) persist() error {
	if s == nil || s.path == "" {
		return nil
	}
	s.mu.RLock()
	items := make([]*connectionpb.Connection, 0, len(s.byName))
	for _, c := range s.byName {
		items = append(items, c)
	}
	s.mu.RUnlock()
	raw, err := json.MarshalIndent(struct {
		Connections []*connectionpb.Connection `json:"connections"`
	}{Connections: items}, "", "  ")
	if err != nil {
		return err
	}
	tmp := s.path + ".tmp"
	if err := os.WriteFile(tmp, raw, 0o600); err != nil {
		return err
	}
	return os.Rename(tmp, s.path)
}

// Put stores or replaces a connection by name.
func (s *Store) Put(conn *connectionpb.Connection) error {
	if s == nil || conn == nil || conn.Name == "" {
		return errors.New("connection name is required")
	}
	s.mu.Lock()
	s.byName[conn.Name] = conn
	s.mu.Unlock()
	return s.persist()
}

// Get returns a connection by resource name.
func (s *Store) Get(name string) (*connectionpb.Connection, bool) {
	if s == nil {
		return nil, false
	}
	s.mu.RLock()
	defer s.mu.RUnlock()
	c, ok := s.byName[name]
	return c, ok
}

// Delete removes a connection by name.
func (s *Store) Delete(name string) error {
	if s == nil || name == "" {
		return errors.New("connection name is required")
	}
	s.mu.Lock()
	delete(s.byName, name)
	s.mu.Unlock()
	return s.persist()
}

// List returns all connections whose name has the given parent prefix.
func (s *Store) List(parent string) []*connectionpb.Connection {
	if s == nil {
		return nil
	}
	prefix := parent + "/connections/"
	s.mu.RLock()
	defer s.mu.RUnlock()
	out := make([]*connectionpb.Connection, 0)
	for name, c := range s.byName {
		if c == nil {
			continue
		}
		if parent != "" && !hasParentPrefix(name, prefix) {
			continue
		}
		out = append(out, c)
	}
	return out
}

func hasParentPrefix(name, prefix string) bool {
	return len(name) > len(prefix) && name[:len(prefix)] == prefix
}

// CloneConnection returns a protojson round-tripped copy for safe mutation.
func CloneConnection(in *connectionpb.Connection) (*connectionpb.Connection, error) {
	if in == nil {
		return &connectionpb.Connection{}, nil
	}
	raw, err := protojson.Marshal(in)
	if err != nil {
		return nil, err
	}
	out := &connectionpb.Connection{}
	if err := protojson.Unmarshal(raw, out); err != nil {
		return nil, err
	}
	return out, nil
}
