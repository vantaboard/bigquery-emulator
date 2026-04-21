package metadata

import (
	"context"
	"database/sql"
	"errors"
	"fmt"
	"sync"
	"time"

	internaltypes "github.com/vantaboard/bigquery-emulator/internal/types"
	bigqueryv2 "google.golang.org/api/bigquery/v2"
)

type Job struct {
	ID        string
	ProjectID string
	content   *bigqueryv2.Job
	response  *internaltypes.QueryResponse
	err       error
	completed bool
	mu        sync.RWMutex
	repo      *Repository
}

func (j *Job) Query() string {
	return j.content.Configuration.Query.Query
}

func (j *Job) QueryParameters() []*bigqueryv2.QueryParameter {
	return j.content.Configuration.Query.QueryParameters
}

func (j *Job) SetResult(ctx context.Context, tx *sql.Tx, response *internaltypes.QueryResponse, err error) error {
	j.response = response
	j.err = err
	if err := j.repo.UpdateJob(ctx, tx, j); err != nil {
		return fmt.Errorf("failed to update job: %w", err)
	}
	return nil
}

// IsTerminal reports whether the job has finished (success, failure, or cancel).
func (j *Job) IsTerminal() bool {
	if j == nil {
		return false
	}
	if j.err != nil {
		return true
	}
	if j.content != nil && j.content.Status != nil {
		switch j.content.Status.State {
		case "DONE":
			return true
		}
	}
	return false
}

// Response returns the query response when the job completed successfully.
func (j *Job) Response() *internaltypes.QueryResponse {
	if j == nil {
		return nil
	}
	return j.response
}

// Err returns a terminal error for the job, if any.
func (j *Job) Err() error {
	if j == nil {
		return nil
	}
	return j.err
}

func (j *Job) Content() *bigqueryv2.Job {
	return j.content
}

func (j *Job) Wait(ctx context.Context) (*internaltypes.QueryResponse, error) {
	ticker := time.NewTicker(100 * time.Millisecond)
	defer ticker.Stop()
	for {
		if err := ctx.Err(); err != nil {
			return nil, err
		}
		foundJob, err := j.repo.FindJob(ctx, j.ProjectID, j.ID)
		if err != nil {
			return nil, err
		}
		if foundJob == nil {
			return nil, fmt.Errorf("job %s not found in project %s", j.ID, j.ProjectID)
		}
		if foundJob.IsTerminal() {
			return foundJob.response, foundJob.err
		}
		select {
		case <-ticker.C:
		case <-ctx.Done():
			return nil, ctx.Err()
		}
	}
}

// JobCanceller cancels in-flight async query jobs (registered by the server).
type JobCanceller interface {
	CancelQueryJob(projectID, jobID string) error
}

// QueryJobCanceller is set during server startup.
var QueryJobCanceller JobCanceller

func (j *Job) Cancel(ctx context.Context) error {
	_ = ctx
	if QueryJobCanceller == nil {
		return errors.New("job cancellation is not available")
	}
	return QueryJobCanceller.CancelQueryJob(j.ProjectID, j.ID)
}

func (j *Job) Insert(ctx context.Context, tx *sql.Tx) error {
	return j.repo.AddJob(ctx, tx, j)
}

func (j *Job) Delete(ctx context.Context, tx *sql.Tx) error {
	return j.repo.DeleteJob(ctx, tx, j)
}

func NewJob(repo *Repository, projectID, jobID string, content *bigqueryv2.Job, response *internaltypes.QueryResponse, err error) *Job {
	return &Job{
		ID:        jobID,
		ProjectID: projectID,
		content:   content,
		response:  response,
		err:       err,
		repo:      repo,
	}
}
