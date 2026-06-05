package load

import (
	"bytes"
	"crypto/rand"
	"encoding/hex"
	"errors"
	"fmt"
	"io"
	"mime"
	"mime/multipart"
	"net/http"
	"strconv"
	"strings"
	"sync"
	"time"
)

// UploadSession tracks a resumable jobs.insert media upload.
type UploadSession struct {
	ProjectID string
	Metadata  []byte
	Data      []byte
	Total     int64
	Expires   time.Time
}

// UploadStore holds in-process resumable upload sessions.
type UploadStore struct {
	mu       sync.Mutex
	sessions map[string]*UploadSession
}

// NewUploadStore returns a fresh upload session table.
func NewUploadStore() *UploadStore {
	return &UploadStore{sessions: map[string]*UploadSession{}}
}

var defaultUploadStore = NewUploadStore()

// DefaultUploadStore is the process-local resumable upload session table.
func DefaultUploadStore() *UploadStore { return defaultUploadStore }

// CreateSession registers a resumable upload session and returns its id.
func (s *UploadStore) CreateSession(projectID string, metadata []byte, total int64) string {
	id := newUploadID()
	s.mu.Lock()
	defer s.mu.Unlock()
	s.sessions[id] = &UploadSession{
		ProjectID: projectID,
		Metadata:  append([]byte(nil), metadata...),
		Total:     total,
		Expires:   time.Now().UTC().Add(24 * time.Hour),
	}
	return id
}

// Get returns the session for id, or nil when unknown or expired.
func (s *UploadStore) Get(id string) *UploadSession {
	s.mu.Lock()
	defer s.mu.Unlock()
	sess, ok := s.sessions[id]
	if !ok || time.Now().UTC().After(sess.Expires) {
		delete(s.sessions, id)
		return nil
	}
	return sess
}

// Delete removes a completed or abandoned session.
func (s *UploadStore) Delete(id string) {
	s.mu.Lock()
	defer s.mu.Unlock()
	delete(s.sessions, id)
}

// AppendBytes appends chunk data for a resumable session.
func (s *UploadStore) AppendBytes(id string, chunk []byte, start int64) error {
	s.mu.Lock()
	defer s.mu.Unlock()
	sess, ok := s.sessions[id]
	if !ok || time.Now().UTC().After(sess.Expires) {
		delete(s.sessions, id)
		return errors.New("upload session not found")
	}
	need := int(start) + len(chunk)
	if need > len(sess.Data) {
		grown := make([]byte, need)
		copy(grown, sess.Data)
		sess.Data = grown
	}
	copy(sess.Data[start:], chunk)
	return nil
}

// ReceivedBytes returns how many bytes have been stored for the session.
func (s *UploadStore) ReceivedBytes(id string) int64 {
	s.mu.Lock()
	defer s.mu.Unlock()
	sess, ok := s.sessions[id]
	if !ok {
		return 0
	}
	return int64(len(sess.Data))
}

// ParseMultipartJob extracts the metadata JSON and media bytes from a
// multipart/related jobs.insert upload body.
func ParseMultipartJob(body []byte, contentType string) (metadata, media []byte, err error) {
	mediaType, params, err := mime.ParseMediaType(contentType)
	if err != nil {
		return nil, nil, fmt.Errorf("parse Content-Type: %w", err)
	}
	if !strings.HasPrefix(mediaType, "multipart/") {
		return nil, nil, fmt.Errorf("expected multipart Content-Type, got %q", mediaType)
	}
	boundary := params["boundary"]
	if boundary == "" {
		return nil, nil, errors.New("multipart boundary missing")
	}
	reader := multipart.NewReader(bytes.NewReader(body), boundary)
	for partIndex := 0; ; partIndex++ {
		part, perr := reader.NextPart()
		if perr == io.EOF {
			break
		}
		if perr != nil {
			return nil, nil, fmt.Errorf("read multipart part: %w", perr)
		}
		data, rerr := io.ReadAll(part)
		if rerr != nil {
			return nil, nil, fmt.Errorf("read multipart body: %w", rerr)
		}
		switch partIndex {
		case 0:
			metadata = data
		case 1:
			media = data
		}
	}
	if len(metadata) == 0 {
		return nil, nil, errors.New("multipart upload missing metadata part")
	}
	return metadata, media, nil
}

// ParseContentRange parses a Content-Range header (bytes start-end/total).
// When total is unknown the third return is -1.
func ParseContentRange(header string) (start, end, total int64, ok bool) {
	header = strings.TrimSpace(header)
	if !strings.HasPrefix(header, "bytes ") {
		return 0, 0, 0, false
	}
	rest := strings.TrimPrefix(header, "bytes ")
	parts := strings.Split(rest, "/")
	if len(parts) != 2 {
		return 0, 0, 0, false
	}
	if parts[0] == "*" {
		if parts[1] == "*" {
			return 0, 0, -1, true
		}
		t, err := strconv.ParseInt(parts[1], 10, 64)
		if err != nil {
			return 0, 0, 0, false
		}
		return 0, 0, t, true
	}
	rangeParts := strings.Split(parts[0], "-")
	if len(rangeParts) != 2 {
		return 0, 0, 0, false
	}
	start, err := strconv.ParseInt(rangeParts[0], 10, 64)
	if err != nil {
		return 0, 0, 0, false
	}
	end, err = strconv.ParseInt(rangeParts[1], 10, 64)
	if err != nil {
		return 0, 0, 0, false
	}
	if parts[1] == "*" {
		return start, end, -1, true
	}
	total, err = strconv.ParseInt(parts[1], 10, 64)
	if err != nil {
		return 0, 0, 0, false
	}
	return start, end, total, true
}

// WriteResumeIncomplete responds with HTTP 308 for partial resumable uploads.
func WriteResumeIncomplete(w http.ResponseWriter, received int64) {
	if received > 0 {
		w.Header().Set("Range", fmt.Sprintf("0-%d", received-1))
	}
	w.WriteHeader(308) // Resume Incomplete per api-uploads.md
}

func newUploadID() string {
	var b [16]byte
	_, _ = rand.Read(b[:])
	return hex.EncodeToString(b[:])
}

// SessionLocation builds the resumable session URI returned in Location.
func SessionLocation(projectID, uploadID string) string {
	return fmt.Sprintf(
		"/upload/bigquery/v2/projects/%s/jobs?uploadType=resumable&upload_id=%s",
		projectID, uploadID,
	)
}
