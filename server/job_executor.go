package server

import (
	"context"
	"fmt"
	"sync"

	"github.com/goccy/go-json"
	bigqueryv2 "google.golang.org/api/bigquery/v2"

	"github.com/vantaboard/bigquery-emulator/internal/logger"
	"github.com/vantaboard/bigquery-emulator/internal/metadata"
)

const (
	defaultQueryWorkers     = 4
	defaultQueryQueueDepth  = 64
	defaultStorageReadSlots = 8
	// Connection pool sizing is handled by connection.PoolConfigFromEnv (elastic bounds from CPU/cgroup,
	// BQ_EMULATOR_POOL_*, or fixed BQ_EMULATOR_POOL_SIZE).
)

// JobExecutor runs async BigQuery query jobs with bounded concurrency.
type JobExecutor struct {
	server   *Server
	queue    chan *queryJobWork
	ctx      context.Context
	cancel   context.CancelFunc
	wg       sync.WaitGroup
	stopOnce sync.Once

	mu        sync.Mutex
	cancelFns map[string]context.CancelFunc // projectID + "\x00" + jobID
}

type queryJobWork struct {
	projectID string
	job       *bigqueryv2.Job
}

func jobCancelRegistryKey(projectID, jobID string) string {
	return projectID + "\x00" + jobID
}

func newJobExecutor(srv *Server, workers, queueDepth int) *JobExecutor {
	if workers <= 0 {
		workers = defaultQueryWorkers
	}
	if queueDepth <= 0 {
		queueDepth = defaultQueryQueueDepth
	}
	ctx, cancel := context.WithCancel(context.Background())
	e := &JobExecutor{
		server:    srv,
		queue:     make(chan *queryJobWork, queueDepth),
		ctx:       ctx,
		cancel:    cancel,
		cancelFns: make(map[string]context.CancelFunc),
	}
	for i := 0; i < workers; i++ {
		e.wg.Add(1)
		go e.workerLoop()
	}
	return e
}

func (e *JobExecutor) workerLoop() {
	defer e.wg.Done()
	for {
		select {
		case <-e.ctx.Done():
			return
		case w, ok := <-e.queue:
			if !ok || w == nil {
				if !ok {
					return
				}
				continue
			}
			e.runQueryJob(w)
		}
	}
}

// Submit enqueues async query execution. Returns an error if the queue is full or the server is stopping.
func (e *JobExecutor) Submit(projectID string, job *bigqueryv2.Job) error {
	if e == nil {
		return fmt.Errorf("job executor is not initialized")
	}
	jobCopy, err := deepCopyBigQueryJob(job)
	if err != nil {
		return fmt.Errorf("copy job: %w", err)
	}
	w := &queryJobWork{projectID: projectID, job: jobCopy}
	select {
	case <-e.ctx.Done():
		return fmt.Errorf("server is stopping")
	case e.queue <- w:
		return nil
	default:
		return fmt.Errorf("query job queue is full (max %d)", cap(e.queue))
	}
}

func (e *JobExecutor) Stop() {
	e.stopOnce.Do(func() {
		e.cancel()
		e.wg.Wait()
	})
}

func (e *JobExecutor) runQueryJob(w *queryJobWork) {
	workCtx, cancel := context.WithCancel(context.Background())
	workCtx = logger.WithLogger(workCtx, e.server.logger)
	key := jobCancelRegistryKey(w.projectID, w.job.JobReference.JobId)
	e.mu.Lock()
	e.cancelFns[key] = cancel
	e.mu.Unlock()
	defer func() {
		e.mu.Lock()
		delete(e.cancelFns, key)
		e.mu.Unlock()
		cancel()
	}()

	h := &jobsInsertHandler{}
	_ = h.executeAsyncQueryJob(workCtx, e.server, w.projectID, w.job)
}

// CancelQueryJob implements [metadata.JobCanceller].
func (e *JobExecutor) CancelQueryJob(projectID, jobID string) error {
	if e == nil {
		return fmt.Errorf("job executor is not initialized")
	}
	key := jobCancelRegistryKey(projectID, jobID)
	e.mu.Lock()
	fn, ok := e.cancelFns[key]
	e.mu.Unlock()
	if !ok || fn == nil {
		return nil
	}
	fn()
	return nil
}

func deepCopyBigQueryJob(j *bigqueryv2.Job) (*bigqueryv2.Job, error) {
	if j == nil {
		return nil, fmt.Errorf("job is nil")
	}
	b, err := json.Marshal(j)
	if err != nil {
		return nil, err
	}
	var out bigqueryv2.Job
	if err := json.Unmarshal(b, &out); err != nil {
		return nil, err
	}
	return &out, nil
}

func registerQueryJobCanceller(e *JobExecutor) {
	if e == nil {
		metadata.QueryJobCanceller = nil
		return
	}
	metadata.QueryJobCanceller = e
}
