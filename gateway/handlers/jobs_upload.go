package handlers

import (
	"encoding/json"
	"io"
	"net/http"
	"strconv"
	"strings"
	"time"

	"github.com/vantaboard/bigquery-emulator/gateway/bqtypes"
	"github.com/vantaboard/bigquery-emulator/gateway/copy"
	"github.com/vantaboard/bigquery-emulator/gateway/extract"
	"github.com/vantaboard/bigquery-emulator/gateway/jobs"
	"github.com/vantaboard/bigquery-emulator/gateway/load"
)

// runSyncLoadInsert accepts a load-job body, fetches source bytes, parses
// supported formats, and bulk-inserts into the destination table.
func runSyncLoadInsert(deps Dependencies, w http.ResponseWriter, r *http.Request,
	posted *jobs.Job, cfg *jobs.JobConfiguration,
) {
	projectID := r.PathValue("projectId")
	job := newPendingJob(deps, projectID, posted, cfg)
	start := time.Now().UTC()
	if deps.Catalog == nil {
		finalizeDeferredDataPlaneJob(job, cfg, start, "load")
		writeJSON(w, http.StatusOK, job)
		return
	}
	result, err := load.Execute(r.Context(), deps.Catalog, cfg.Load, projectID)
	if err != nil {
		finalizeFailedDataPlaneJob(job, start, err)
		writeJSON(w, http.StatusOK, job)
		return
	}
	persistLoadTableMetadata(deps, cfg.Load, projectID)
	finalizeSuccessfulLoadJob(job, start, result)
	writeJSON(w, http.StatusOK, job)
}

// runSyncCopyInsert executes a copy job via engine SQL or catalog row copy.
func runSyncCopyInsert(deps Dependencies, w http.ResponseWriter, r *http.Request,
	posted *jobs.Job, cfg *jobs.JobConfiguration,
) {
	projectID := r.PathValue("projectId")
	job := newPendingJob(deps, projectID, posted, cfg)
	start := time.Now().UTC()
	if deps.Catalog == nil {
		finalizeDeferredDataPlaneJob(job, cfg, start, "copy")
		writeJSON(w, http.StatusOK, job)
		return
	}
	result, err := copy.Execute(r.Context(), deps.Catalog, deps.Query, deps.Snapshots, cfg.Copy, projectID)
	if err != nil {
		finalizeFailedDataPlaneJob(job, start, err)
		writeJSON(w, http.StatusOK, job)
		return
	}
	finalizeSuccessfulCopyJob(job, start, result)
	writeJSON(w, http.StatusOK, job)
}

// runSyncExtractInsert reads table rows and uploads CSV/JSON to GCS.
func runSyncExtractInsert(deps Dependencies, w http.ResponseWriter, r *http.Request,
	posted *jobs.Job, cfg *jobs.JobConfiguration,
) {
	projectID := r.PathValue("projectId")
	job := newPendingJob(deps, projectID, posted, cfg)
	start := time.Now().UTC()
	if deps.Catalog == nil {
		finalizeDeferredDataPlaneJob(job, cfg, start, "extract")
		writeJSON(w, http.StatusOK, job)
		return
	}
	result, err := extract.Execute(r.Context(), deps.Catalog, cfg.Extract, projectID)
	if err != nil {
		finalizeFailedDataPlaneJob(job, start, err)
		writeJSON(w, http.StatusOK, job)
		return
	}
	finalizeSuccessfulExtractJob(job, start, result)
	writeJSON(w, http.StatusOK, job)
}

func finalizeSuccessfulCopyJob(job *jobs.Job, start time.Time, result copy.Result) {
	end := time.Now().UTC()
	job.Status.State = jobs.JobStateDone
	job.Status.ErrorResult = nil
	job.Statistics.StartTime = millisString(start)
	job.Statistics.EndTime = millisString(end)
	job.Statistics.Copy = copy.FormatStatistics(result)
}

func finalizeSuccessfulExtractJob(job *jobs.Job, start time.Time, result extract.Result) {
	end := time.Now().UTC()
	job.Status.State = jobs.JobStateDone
	job.Status.ErrorResult = nil
	job.Statistics.StartTime = millisString(start)
	job.Statistics.EndTime = millisString(end)
	job.Statistics.Extract = extract.FormatStatistics(result)
}

func finalizeSuccessfulLoadJob(job *jobs.Job, start time.Time, result load.Result) {
	end := time.Now().UTC()
	job.Status.State = jobs.JobStateDone
	job.Status.ErrorResult = nil
	job.Statistics.StartTime = millisString(start)
	job.Statistics.EndTime = millisString(end)
	job.Statistics.Load = load.FormatStatistics(result)
}

// persistLoadTableMetadata stashes REST-only destination metadata (CMEK,
// clustering, time partitioning) so tables.get round-trips what the load
// job supplied.
func persistLoadTableMetadata(deps Dependencies, cfg *jobs.JobConfigurationLoad, projectID string) {
	if deps.Metadata == nil || cfg == nil || cfg.DestinationTable == nil {
		return
	}
	if cfg.DestinationEncryptionConfiguration == nil &&
		cfg.Clustering == nil && cfg.TimePartitioning == nil {
		return
	}
	destProject := cfg.DestinationTable.ProjectID
	if destProject == "" {
		destProject = projectID
	}
	deps.Metadata.MergeTable(destProject, cfg.DestinationTable.DatasetID,
		cfg.DestinationTable.TableID, bqtypes.Table{
			EncryptionConfiguration: cfg.DestinationEncryptionConfiguration,
			Clustering:              cfg.Clustering,
			TimePartitioning:        cfg.TimePartitioning,
		})
}

func finalizeDeferredDataPlaneJob(job *jobs.Job, cfg *jobs.JobConfiguration, start time.Time, kind string) {
	end := time.Now().UTC()
	job.Status.State = jobs.JobStateDone
	job.Status.ErrorResult = &bqtypes.ErrorProto{
		Reason: reasonNotImplemented,
		Message: "jobs.insert: " + kind + " job data plane is unavailable; " +
			"load / copy / extract execution requires an engine catalog connection.",
	}
	job.Statistics.StartTime = millisString(start)
	job.Statistics.EndTime = millisString(end)
	switch kind {
	case "load":
		inputFiles := "0"
		if cfg.Load != nil {
			inputFiles = strconv.Itoa(len(cfg.Load.SourceURIs))
		}
		job.Statistics.Load = &jobs.LoadStatistics{
			InputFiles:     inputFiles,
			InputFileBytes: "0",
			OutputRows:     "0",
			OutputBytes:    "0",
			BadRecords:     "0",
		}
	case "copy":
		job.Statistics.Copy = &jobs.CopyStatistics{
			CopiedRows:         "0",
			CopiedLogicalBytes: "0",
		}
	case "extract":
		var counts []string
		if cfg.Extract != nil && len(cfg.Extract.DestinationURIs) > 0 {
			counts = make([]string, len(cfg.Extract.DestinationURIs))
			for i := range counts {
				counts[i] = "0"
			}
		}
		job.Statistics.Extract = &jobs.ExtractStatistics{
			DestinationURIFileCounts: counts,
			InputBytes:               "0",
		}
	}
}

func handleJobInsertUploadPost(deps Dependencies, store *load.UploadStore,
	w http.ResponseWriter, r *http.Request,
) {
	uploadType := r.URL.Query().Get("uploadType")
	switch uploadType {
	case "multipart":
		handleMultipartLoadUpload(deps, w, r)
	case "resumable":
		handleResumableLoadUploadInit(store, w, r)
	default:
		writeError(w, http.StatusBadRequest, reasonInvalid,
			"uploadType must be multipart or resumable")
	}
}

func handleMultipartLoadUpload(deps Dependencies, w http.ResponseWriter, r *http.Request) {
	body, err := io.ReadAll(r.Body)
	if err != nil {
		writeError(w, http.StatusBadRequest, reasonInvalid,
			"Could not read upload body: "+err.Error())
		return
	}
	metadata, media, err := load.ParseMultipartJob(body, r.Header.Get("Content-Type"))
	if err != nil {
		writeError(w, http.StatusBadRequest, reasonInvalid, err.Error())
		return
	}
	runUploadedLoadJob(deps, w, r, metadata, media)
}

func handleResumableLoadUploadInit(store *load.UploadStore, w http.ResponseWriter, r *http.Request) {
	projectID := r.PathValue("projectId")
	body, err := io.ReadAll(r.Body)
	if err != nil {
		writeError(w, http.StatusBadRequest, reasonInvalid,
			"Could not read upload metadata: "+err.Error())
		return
	}
	var total int64 = -1
	if v := strings.TrimSpace(r.Header.Get("X-Upload-Content-Length")); v != "" {
		total, err = strconv.ParseInt(v, 10, 64)
		if err != nil {
			writeError(w, http.StatusBadRequest, reasonInvalid,
				"invalid X-Upload-Content-Length")
			return
		}
	}
	uploadID := store.CreateSession(projectID, body, total)
	w.Header().Set("Location", load.AbsoluteSessionLocation(
		requestEmulatorBaseURL(r), projectID, uploadID))
	w.WriteHeader(http.StatusOK)
}

func handleJobInsertUploadPut(deps Dependencies, store *load.UploadStore,
	w http.ResponseWriter, r *http.Request,
) {
	if r.URL.Query().Get("uploadType") != "resumable" {
		writeError(w, http.StatusBadRequest, reasonInvalid,
			"PUT upload requires uploadType=resumable")
		return
	}
	uploadID := r.URL.Query().Get("upload_id")
	if uploadID == "" {
		writeError(w, http.StatusBadRequest, reasonInvalid, "upload_id is required")
		return
	}
	sess := store.Get(uploadID)
	if sess == nil {
		writeError(w, http.StatusNotFound, reasonNotFound, "upload session not found")
		return
	}

	body, err := io.ReadAll(r.Body)
	if err != nil {
		writeError(w, http.StatusBadRequest, reasonInvalid,
			"Could not read upload chunk: "+err.Error())
		return
	}

	media, done, err := finalizeResumableChunk(store, uploadID, sess, r.Header.Get("Content-Range"), body)
	if err != nil {
		writeError(w, http.StatusBadRequest, reasonInvalid, err.Error())
		return
	}
	if !done {
		load.WriteResumeIncomplete(w, store.ReceivedBytes(uploadID))
		return
	}

	store.Delete(uploadID)
	runUploadedLoadJob(deps, w, r, sess.Metadata, media)
}

func finalizeResumableChunk(store *load.UploadStore, uploadID string, sess *load.UploadSession,
	contentRange string, body []byte,
) (media []byte, done bool, err error) {
	if contentRange != "" && len(body) == 0 {
		return nil, false, nil
	}
	switch {
	case contentRange != "":
		return appendResumableRange(store, uploadID, contentRange, body)
	case len(body) > 0:
		if aerr := store.AppendBytes(uploadID, body, 0); aerr != nil {
			return nil, false, aerr
		}
		sess = store.Get(uploadID)
		return sess.Data, true, nil
	default:
		received := store.ReceivedBytes(uploadID)
		if sess.Total > 0 && received < sess.Total {
			return nil, false, nil
		}
		return sess.Data, true, nil
	}
}

func appendResumableRange(store *load.UploadStore, uploadID, contentRange string, body []byte,
) ([]byte, bool, error) {
	start, end, total, ok := load.ParseContentRange(contentRange)
	if !ok {
		return nil, false, errInvalidContentRange
	}
	if int64(len(body)) != end-start+1 {
		return nil, false, errContentRangeLength
	}
	if err := store.AppendBytes(uploadID, body, start); err != nil {
		return nil, false, err
	}
	received := store.ReceivedBytes(uploadID)
	if total > 0 && received < total {
		return nil, false, nil
	}
	sess := store.Get(uploadID)
	return sess.Data, true, nil
}

var (
	errInvalidContentRange = errUpload("invalid Content-Range")
	errContentRangeLength  = errUpload("Content-Range length mismatch")
)

type errUpload string

func (e errUpload) Error() string { return string(e) }

func runUploadedLoadJob(deps Dependencies, w http.ResponseWriter, r *http.Request,
	metadata, media []byte,
) {
	var posted jobs.Job
	if len(metadata) > 0 {
		if err := json.Unmarshal(metadata, &posted); err != nil {
			writeError(w, http.StatusBadRequest, reasonInvalid,
				"Could not parse upload metadata as JSON: "+err.Error())
			return
		}
	}
	cfg := posted.Configuration
	if cfg == nil || cfg.Load == nil {
		writeError(w, http.StatusBadRequest, reasonInvalid,
			"upload metadata must include configuration.load")
		return
	}
	projectID := r.PathValue("projectId")
	job := newPendingJob(deps, projectID, &posted, cfg)
	start := time.Now().UTC()
	if deps.Catalog == nil {
		finalizeDeferredDataPlaneJob(job, cfg, start, "load")
		writeJSON(w, http.StatusOK, job)
		return
	}
	result, err := load.ExecuteFromBytes(r.Context(), deps.Catalog, cfg.Load, projectID, media)
	if err != nil {
		finalizeFailedDataPlaneJob(job, start, err)
		writeJSON(w, http.StatusOK, job)
		return
	}
	persistLoadTableMetadata(deps, cfg.Load, projectID)
	finalizeSuccessfulLoadJob(job, start, result)
	writeJSON(w, http.StatusOK, job)
}
