package main

import (
	"fmt"
	"sync"
	"time"

	"github.com/Jyarn/threadpool"
)

func main() {
	// Create a threadpool with 5 workers
	pool := threadpool.New(5)
	pool.Start()
	defer pool.Shutdown()

	var wg sync.WaitGroup
	var mu sync.Mutex
	results := make([]string, 0)

	// Submit 10 jobs
	for i := 1; i <= 10; i++ {
		wg.Add(1)
		jobNum := i
		pool.Submit(func() {
			defer wg.Done()
			// Simulate work
			time.Sleep(100 * time.Millisecond)
			result := fmt.Sprintf("Job %d completed", jobNum)
			mu.Lock()
			results = append(results, result)
			mu.Unlock()
		})
	}

	// Wait for all jobs to complete
	wg.Wait()

	// Print results
	fmt.Println("All jobs completed:")
	for _, result := range results {
		fmt.Println(result)
	}
}
