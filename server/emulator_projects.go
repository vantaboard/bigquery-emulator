package server

import (
	"context"
	"fmt"
	"net/http"
	"strings"

	"github.com/goccy/go-json"
	"github.com/gorilla/mux"
	"github.com/vantaboard/bigquery-emulator/internal/connection"
	"github.com/vantaboard/bigquery-emulator/internal/metadata"
)

// Emulator-only project admin API (not part of the BigQuery REST surface).
const (
	emulatorProjectsCollectionPath = "/emulator/v1/projects"
	emulatorProjectsItemPath       = "/emulator/v1/projects/{id}"
)

type emulatorProjectCreateBody struct {
	ID string `json:"id"`
}

type emulatorProjectJSON struct {
	ID string `json:"id"`
}

func registerEmulatorProjectRoutes(r *mux.Router, s *Server) {
	r.Handle(emulatorProjectsCollectionPath, http.HandlerFunc(func(w http.ResponseWriter, req *http.Request) {
		emulatorProjectsCollection(s, w, req)
	})).Methods(http.MethodGet, http.MethodPost)
	r.Handle(emulatorProjectsItemPath, http.HandlerFunc(func(w http.ResponseWriter, req *http.Request) {
		emulatorProjectsItem(s, w, req)
	})).Methods(http.MethodGet, http.MethodDelete)
}

func emulatorProjectsCollection(s *Server, w http.ResponseWriter, r *http.Request) {
	ctx := r.Context()
	switch r.Method {
	case http.MethodGet:
		projects, err := s.metaRepo.FindAllProjects(ctx)
		if err != nil {
			errorResponse(ctx, w, errInternalError(err.Error()))
			return
		}
		ids := make([]string, len(projects))
		for i, p := range projects {
			ids[i] = p.ID
		}
		w.Header().Set("Content-Type", "application/json")
		if err := json.NewEncoder(w).Encode(ids); err != nil {
			errorResponse(ctx, w, errInternalError(err.Error()))
			return
		}
	case http.MethodPost:
		var body emulatorProjectCreateBody
		if err := json.NewDecoder(r.Body).Decode(&body); err != nil {
			errorResponse(ctx, w, errInvalid(fmt.Sprintf("invalid JSON body: %s", err)))
			return
		}
		id := strings.TrimSpace(body.ID)
		if id == "" {
			errorResponse(ctx, w, errInvalid("id is required"))
			return
		}
		existing, err := s.metaRepo.FindProject(ctx, id)
		if err != nil {
			errorResponse(ctx, w, errInternalError(err.Error()))
			return
		}
		if existing != nil {
			errorResponse(ctx, w, errDuplicate(fmt.Sprintf("project %s already exists", id)))
			return
		}
		err = s.connMgr.ExecuteWithTransaction(ctx, func(ctx context.Context, tx *connection.Tx) error {
			tx.SetProjectAndDataset(id, "")
			if err := tx.MetadataRepoMode(); err != nil {
				return err
			}
			return s.metaRepo.AddProject(ctx, tx.Tx(), metadata.NewProject(s.metaRepo, id))
		})
		if err != nil {
			errorResponse(ctx, w, errInternalError(err.Error()))
			return
		}
		w.Header().Set("Content-Type", "application/json")
		w.WriteHeader(http.StatusCreated)
		if err := json.NewEncoder(w).Encode(emulatorProjectJSON{ID: id}); err != nil {
			errorResponse(ctx, w, errInternalError(err.Error()))
			return
		}
	default:
		http.Error(w, "", http.StatusMethodNotAllowed)
	}
}

func emulatorProjectsItem(s *Server, w http.ResponseWriter, r *http.Request) {
	ctx := r.Context()
	id := strings.TrimSpace(mux.Vars(r)["id"])
	if id == "" {
		errorResponse(ctx, w, errInvalid("missing project id"))
		return
	}

	switch r.Method {
	case http.MethodGet:
		p, err := s.metaRepo.FindProject(ctx, id)
		if err != nil {
			errorResponse(ctx, w, errInternalError(err.Error()))
			return
		}
		if p == nil {
			errorResponse(ctx, w, errNotFound(fmt.Sprintf("project %s is not found", id)))
			return
		}
		w.Header().Set("Content-Type", "application/json")
		if err := json.NewEncoder(w).Encode(emulatorProjectJSON{ID: p.ID}); err != nil {
			errorResponse(ctx, w, errInternalError(err.Error()))
			return
		}
	case http.MethodDelete:
		p, err := s.metaRepo.FindProject(ctx, id)
		if err != nil {
			errorResponse(ctx, w, errInternalError(err.Error()))
			return
		}
		if p == nil {
			errorResponse(ctx, w, errNotFound(fmt.Sprintf("project %s is not found", id)))
			return
		}
		datasets, err := s.metaRepo.FindDatasetsInProject(ctx, id)
		if err != nil {
			errorResponse(ctx, w, errInternalError(err.Error()))
			return
		}
		if len(datasets) > 0 {
			errorResponse(ctx, w, errDuplicate(fmt.Sprintf("project %s has datasets; delete them first", id)))
			return
		}
		err = s.connMgr.ExecuteWithTransaction(ctx, func(ctx context.Context, tx *connection.Tx) error {
			tx.SetProjectAndDataset(id, "")
			if err := tx.MetadataRepoMode(); err != nil {
				return err
			}
			return s.metaRepo.DeleteProject(ctx, tx.Tx(), metadata.NewProject(s.metaRepo, id))
		})
		if err != nil {
			errorResponse(ctx, w, errInternalError(err.Error()))
			return
		}
		w.WriteHeader(http.StatusNoContent)
	default:
		http.Error(w, "", http.StatusMethodNotAllowed)
	}
}
