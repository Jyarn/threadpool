package threadpool

import (
	"sync"
	"sync/atomic"
	"testing"
	"time"
)

func TestNew(t *testing.T) {
	tp := New(5)
	if tp.workers != 5 {
		t.Errorf("Expected 5 workers, got %d", tp.workers)
	}
}

func TestNewWithZeroWorkers(t *testing.T) {
	tp := New(0)
	if tp.workers != 1 {
		t.Errorf("Expected 1 worker for 0 input, got %d", tp.workers)
	}
}

func TestNewWithNegativeWorkers(t *testing.T) {
	tp := New(-5)
	if tp.workers != 1 {
		t.Errorf("Expected 1 worker for negative input, got %d", tp.workers)
	}
}

func TestSubmitAndExecute(t *testing.T) {
	tp := New(3)
	tp.Start()
	defer tp.Shutdown()

	var counter int32
	var wg sync.WaitGroup

	for i := 0; i < 10; i++ {
		wg.Add(1)
		tp.Submit(func() {
			atomic.AddInt32(&counter, 1)
			wg.Done()
		})
	}

	wg.Wait()

	if counter != 10 {
		t.Errorf("Expected counter to be 10, got %d", counter)
	}
}

func TestConcurrentExecution(t *testing.T) {
	tp := New(5)
	tp.Start()
	defer tp.Shutdown()

	var counter int32
	var wg sync.WaitGroup
	jobCount := 100

	for i := 0; i < jobCount; i++ {
		wg.Add(1)
		tp.Submit(func() {
			time.Sleep(10 * time.Millisecond)
			atomic.AddInt32(&counter, 1)
			wg.Done()
		})
	}

	wg.Wait()

	if counter != int32(jobCount) {
		t.Errorf("Expected counter to be %d, got %d", jobCount, counter)
	}
}

func TestShutdown(t *testing.T) {
	tp := New(3)
	tp.Start()

	var counter int32
	var wg sync.WaitGroup

	for i := 0; i < 5; i++ {
		wg.Add(1)
		tp.Submit(func() {
			atomic.AddInt32(&counter, 1)
			wg.Done()
		})
	}

	wg.Wait()
	tp.Shutdown()

	// After shutdown, counter should still be 5
	if counter != 5 {
		t.Errorf("Expected counter to be 5 after shutdown, got %d", counter)
	}
}

func TestMultipleShutdown(t *testing.T) {
	tp := New(2)
	tp.Start()

	tp.Shutdown()
	// Second shutdown should not panic
	tp.Shutdown()
}

func TestShutdownNow(t *testing.T) {
	tp := New(3)
	tp.Start()

	var counter int32

	for i := 0; i < 100; i++ {
		tp.Submit(func() {
			time.Sleep(100 * time.Millisecond)
			atomic.AddInt32(&counter, 1)
		})
	}

	time.Sleep(50 * time.Millisecond)
	tp.ShutdownNow()

	// Counter should be less than 100 since we shut down immediately
	finalCount := atomic.LoadInt32(&counter)
	if finalCount >= 100 {
		t.Errorf("Expected counter to be less than 100 after immediate shutdown, got %d", finalCount)
	}
}

func TestSubmitAfterShutdown(t *testing.T) {
	tp := New(2)
	tp.Start()

	tp.Shutdown()

	// Submit after shutdown should return false
	result := tp.Submit(func() {})
	if result {
		t.Error("Expected Submit to return false after shutdown")
	}
}

