package threadpool

import (
	"sync"
)

// Job represents a unit of work to be executed by the threadpool
type Job func()

// ThreadPool represents a pool of workers that execute jobs concurrently
type ThreadPool struct {
	workers    int
	jobQueue   chan Job
	wg         sync.WaitGroup
	once       sync.Once
	shutdownCh chan struct{}
}

// New creates a new ThreadPool with the specified number of workers
func New(workers int) *ThreadPool {
	if workers <= 0 {
		workers = 1
	}
	return &ThreadPool{
		workers:    workers,
		jobQueue:   make(chan Job, workers*2),
		shutdownCh: make(chan struct{}),
	}
}

// Start begins processing jobs with the worker pool
func (tp *ThreadPool) Start() {
	for i := 0; i < tp.workers; i++ {
		tp.wg.Add(1)
		go tp.worker()
	}
}

// worker processes jobs from the job queue
func (tp *ThreadPool) worker() {
	defer tp.wg.Done()
	for {
		select {
		case job, ok := <-tp.jobQueue:
			if !ok {
				return
			}
			job()
		case <-tp.shutdownCh:
			return
		}
	}
}

// Submit adds a job to the queue for execution
func (tp *ThreadPool) Submit(job Job) {
	tp.jobQueue <- job
}

// Shutdown gracefully shuts down the threadpool, waiting for all jobs to complete
func (tp *ThreadPool) Shutdown() {
	tp.once.Do(func() {
		close(tp.jobQueue)
		tp.wg.Wait()
		close(tp.shutdownCh)
	})
}

// ShutdownNow immediately shuts down the threadpool without waiting for jobs
func (tp *ThreadPool) ShutdownNow() {
	tp.once.Do(func() {
		close(tp.shutdownCh)
		close(tp.jobQueue)
	})
}
