#ifndef TSFI_CCX_POOL_H
#define TSFI_CCX_POOL_H

#define _GNU_SOURCE
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>

#define MAX_CCX_THREADS 32
#define MAX_CCX_TASKS 1024

typedef struct {
    void (*func)(void *arg);
    void *arg;
} TSFiCCXTask;

typedef struct {
    pthread_t threads[MAX_CCX_THREADS];
    int num_threads;
    int ccx_id;
    
    TSFiCCXTask task_queue[MAX_CCX_TASKS];
    int queue_head;
    int queue_tail;
    int queue_size;
    
    pthread_mutex_t lock;
    pthread_cond_t cond_empty;
    pthread_cond_t cond_full;
    bool shutdown;
} TSFiCCXPool;

// Initialize a CCX pool for a specific ccx_id (using 4 cores per CCX mapping)
int tsfi_ccx_pool_init(TSFiCCXPool *pool, int ccx_id, int num_threads);

// Enqueue a task to the CCX pool
int tsfi_ccx_pool_enqueue(TSFiCCXPool *pool, void (*func)(void *), void *arg);

// Wait for all current tasks in the pool to finish
void tsfi_ccx_pool_wait(TSFiCCXPool *pool);

// Shutdown the pool and clean up resources
void tsfi_ccx_pool_destroy(TSFiCCXPool *pool);

// Parallel deconvolution using CCX thread pool
void tsfi_ccx_deconvolve_parallel(TSFiCCXPool *pool, const double *input_image, double *output_image, int width, int height, double noise_signal_ratio);


#endif /* TSFI_CCX_POOL_H */
