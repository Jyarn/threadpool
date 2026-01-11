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
	mu         sync.RWMutex
	isShutdown bool
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
// Returns false if the pool is already shutdown, true otherwise
func (tp *ThreadPool) Submit(job Job) bool {
	tp.mu.RLock()
	defer tp.mu.RUnlock()
	
	if tp.isShutdown {
		return false
	}
	
	select {
	case tp.jobQueue <- job:
		return true
	default:
		// Queue is full, try with blocking send
		tp.jobQueue <- job
		return true
	}
}

// Shutdown gracefully shuts down the threadpool, waiting for all jobs to complete
func (tp *ThreadPool) Shutdown() {
	tp.once.Do(func() {
		tp.mu.Lock()
		tp.isShutdown = true
		tp.mu.Unlock()
		
		close(tp.jobQueue)
		tp.wg.Wait()
		close(tp.shutdownCh)
	})
}

// ShutdownNow immediately shuts down the threadpool without waiting for jobs
func (tp *ThreadPool) ShutdownNow() {
	tp.once.Do(func() {
		tp.mu.Lock()
		tp.isShutdown = true
		tp.mu.Unlock()
		
		// Close jobQueue first so workers exit cleanly
		close(tp.jobQueue)
		close(tp.shutdownCh)
	})
}
