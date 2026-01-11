# threadpool

A simple and efficient thread pool implementation in Go for concurrent job execution.

## Features

- Fixed-size worker pool for controlled concurrency
- Non-blocking job submission
- Graceful shutdown with job completion
- Immediate shutdown option
- Thread-safe operations
- Lightweight and easy to use

## Installation

```bash
go get github.com/Jyarn/threadpool
```

## Usage

### Basic Example

```go
package main

import (
    "fmt"
    "sync"
    "github.com/Jyarn/threadpool"
)

func main() {
    // Create a pool with 5 workers
    pool := threadpool.New(5)
    pool.Start()
    defer pool.Shutdown()

    var wg sync.WaitGroup

    // Submit jobs
    for i := 0; i < 10; i++ {
        wg.Add(1)
        jobNum := i
        pool.Submit(func() {
            defer wg.Done()
            fmt.Printf("Processing job %d\n", jobNum)
        })
    }

    wg.Wait()
}
```

## API

### `New(workers int) *ThreadPool`

Creates a new thread pool with the specified number of workers. If workers is <= 0, defaults to 1.

### `Start()`

Starts the worker pool. Must be called before submitting jobs.

### `Submit(job Job) bool`

Submits a job (function) to be executed by the worker pool. Returns `true` if the job was successfully submitted, `false` if the pool is already shutdown.

### `Shutdown()`

Gracefully shuts down the pool, waiting for all queued jobs to complete.

### `ShutdownNow()`

Immediately shuts down the pool without waiting for jobs to complete.

## License

MIT