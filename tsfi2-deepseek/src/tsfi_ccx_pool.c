#define _GNU_SOURCE
#include "../inc/tsfi_ccx_pool.h"
#include <sched.h>
#include <stdlib.h>
#include <stdio.h>
#include <immintrin.h>


static void *ccx_worker_thread(void *arg) {
    TSFiCCXPool *pool = (TSFiCCXPool *)arg;
    
    while (1) {
        pthread_mutex_lock(&pool->lock);
        
        while (pool->queue_size == 0 && !pool->shutdown) {
            pthread_cond_wait(&pool->cond_empty, &pool->lock);
        }
        
        if (pool->shutdown && pool->queue_size == 0) {
            pthread_mutex_unlock(&pool->lock);
            break;
        }
        
        TSFiCCXTask task = pool->task_queue[pool->queue_head];
        pool->queue_head = (pool->queue_head + 1) % MAX_CCX_TASKS;
        pool->queue_size--;
        
        pthread_cond_signal(&pool->cond_full);
        pthread_mutex_unlock(&pool->lock);
        
        // Execute the task
        if (task.func) {
            task.func(task.arg);
        }
    }
    return NULL;
}

int tsfi_ccx_pool_init(TSFiCCXPool *pool, int ccx_id, int num_threads) {
    if (!pool || num_threads <= 0 || num_threads > MAX_CCX_THREADS) return -1;
    
    pool->num_threads = num_threads;
    pool->ccx_id = ccx_id;
    pool->queue_head = 0;
    pool->queue_tail = 0;
    pool->queue_size = 0;
    pool->shutdown = false;
    
    pthread_mutex_init(&pool->lock, NULL);
    pthread_cond_init(&pool->cond_empty, NULL);
    pthread_cond_init(&pool->cond_full, NULL);
    
    // Core allocation schema based on CCX:
    // CCX 0: Cores 0, 1, 2, 3
    // CCX 1: Cores 4, 5, 6, 7
    // etc.
    int cores_per_ccx = 4;
    int base_core = ccx_id * cores_per_ccx;
    
    for (int i = 0; i < num_threads; i++) {
        pthread_create(&pool->threads[i], NULL, ccx_worker_thread, pool);
        
        // Set CPU affinity for core pinning on a per-CCX basis
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        int target_core = base_core + (i % cores_per_ccx);
        CPU_SET(target_core, &cpuset);
        
        pthread_setaffinity_np(pool->threads[i], sizeof(cpu_set_t), &cpuset);
    }
    
    return 0;
}

int tsfi_ccx_pool_enqueue(TSFiCCXPool *pool, void (*func)(void *), void *arg) {
    if (!pool || !func) return -1;
    
    pthread_mutex_lock(&pool->lock);
    
    while (pool->queue_size == MAX_CCX_TASKS && !pool->shutdown) {
        pthread_cond_wait(&pool->cond_full, &pool->lock);
    }
    
    if (pool->shutdown) {
        pthread_mutex_unlock(&pool->lock);
        return -1;
    }
    
    pool->task_queue[pool->queue_tail].func = func;
    pool->task_queue[pool->queue_tail].arg = arg;
    pool->queue_tail = (pool->queue_tail + 1) % MAX_CCX_TASKS;
    pool->queue_size++;
    
    pthread_cond_signal(&pool->cond_empty);
    pthread_mutex_unlock(&pool->lock);
    
    return 0;
}

void tsfi_ccx_pool_wait(TSFiCCXPool *pool) {
    if (!pool) return;
    
    pthread_mutex_lock(&pool->lock);
    while (pool->queue_size > 0) {
        pthread_cond_wait(&pool->cond_full, &pool->lock);
    }
    pthread_mutex_unlock(&pool->lock);
}

void tsfi_ccx_pool_destroy(TSFiCCXPool *pool) {
    if (!pool) return;
    
    pthread_mutex_lock(&pool->lock);
    pool->shutdown = true;
    pthread_cond_broadcast(&pool->cond_empty);
    pthread_mutex_unlock(&pool->lock);
    
    for (int i = 0; i < pool->num_threads; i++) {
        pthread_join(pool->threads[i], NULL);
    }
    
    pthread_mutex_destroy(&pool->lock);
    pthread_cond_destroy(&pool->cond_empty);
    pthread_cond_destroy(&pool->cond_full);
}

typedef struct {
    const double *input;
    double *output;
    int width;
    int height;
    double nsr;
    int start_y;
    int end_y;
} TSFiDeconvWorkerArg;

static void tsfi_deconv_worker_func(void *arg) {
    TSFiDeconvWorkerArg *w = (TSFiDeconvWorkerArg *)arg;
    double k_center = 5.0 / (1.0 + w->nsr);
    double k_edge = -1.0 / (1.0 + w->nsr);
    
    for (int y = w->start_y; y < w->end_y; y++) {
        if (y == 0 || y == w->height - 1) {
            for (int x = 0; x < w->width; x++) {
                double sum = 0.0;
                for (int ky = -1; ky <= 1; ky++) {
                    for (int kx = -1; kx <= 1; kx++) {
                        int px = x + kx;
                        int py = y + ky;
                        if (px < 0) px = 0;
                        if (px >= w->width) px = w->width - 1;
                        if (py < 0) py = 0;
                        if (py >= w->height) py = w->height - 1;
                        
                        double weight = (kx == 0 && ky == 0) ? k_center : ((kx == 0 || ky == 0) ? k_edge : 0.0);
                        sum += w->input[py * w->width + px] * weight;
                    }
                }
                w->output[y * w->width + x] = sum;
            }
        }
    }
    
    int start_inner_y = (w->start_y < 1) ? 1 : w->start_y;
    int end_inner_y = (w->end_y > w->height - 1) ? w->height - 1 : w->end_y;
    
#if defined(__AVX2__) && defined(__FMA__)
    __m256d center_vec = _mm256_set1_pd(k_center);
    __m256d edge_vec = _mm256_set1_pd(k_edge);
    
    for (int y = start_inner_y; y < end_inner_y; y++) {
        int x = 1;
        {
            int idx = y * w->width + 0;
            double sum = 0.0;
            for (int ky = -1; ky <= 1; ky++) {
                for (int kx = -1; kx <= 1; kx++) {
                    int px = 0 + kx;
                    int py = y + ky;
                    if (px < 0) px = 0;
                    if (px >= w->width) px = w->width - 1;
                    if (py < 0) py = 0;
                    if (py >= w->height) py = w->height - 1;
                    double weight = (kx == 0 && ky == 0) ? k_center : ((kx == 0 || ky == 0) ? k_edge : 0.0);
                    sum += w->input[py * w->width + px] * weight;
                }
            }
            w->output[idx] = sum;
        }
        
        for (; x < w->width - 4; x += 4) {
            __m256d in_center = _mm256_loadu_pd(&w->input[y * w->width + x]);
            __m256d in_up     = _mm256_loadu_pd(&w->input[(y - 1) * w->width + x]);
            __m256d in_down   = _mm256_loadu_pd(&w->input[(y + 1) * w->width + x]);
            __m256d in_left   = _mm256_loadu_pd(&w->input[y * w->width + x - 1]);
            __m256d in_right  = _mm256_loadu_pd(&w->input[y * w->width + x + 1]);
            
            __m256d sum_edges = _mm256_add_pd(_mm256_add_pd(in_up, in_down), _mm256_add_pd(in_left, in_right));
            __m256d res = _mm256_fmadd_pd(sum_edges, edge_vec, _mm256_mul_pd(in_center, center_vec));
            
            _mm256_storeu_pd(&w->output[y * w->width + x], res);
        }
        
        for (; x < w->width - 1; x++) {
            int idx = y * w->width + x;
            w->output[idx] = w->input[idx] * k_center +
                             (w->input[idx - w->width] +
                              w->input[idx + w->width] +
                              w->input[idx - 1] +
                              w->input[idx + 1]) * k_edge;
        }
        
        {
            int idx = y * w->width + w->width - 1;
            double sum = 0.0;
            for (int ky = -1; ky <= 1; ky++) {
                for (int kx = -1; kx <= 1; kx++) {
                    int px = w->width - 1 + kx;
                    int py = y + ky;
                    if (px < 0) px = 0;
                    if (px >= w->width) px = w->width - 1;
                    if (py < 0) py = 0;
                    if (py >= w->height) py = w->height - 1;
                    double weight = (kx == 0 && ky == 0) ? k_center : ((kx == 0 || ky == 0) ? k_edge : 0.0);
                    sum += w->input[py * w->width + px] * weight;
                }
            }
            w->output[idx] = sum;
        }
    }
#else
    for (int y = start_inner_y; y < end_inner_y; y++) {
        for (int x = 0; x < w->width; x++) {
            int idx = y * w->width + x;
            if (x == 0 || x == w->width - 1) {
                double sum = 0.0;
                for (int ky = -1; ky <= 1; ky++) {
                    for (int kx = -1; kx <= 1; kx++) {
                        int px = x + kx;
                        int py = y + ky;
                        if (px < 0) px = 0;
                        if (px >= w->width) px = w->width - 1;
                        if (py < 0) py = 0;
                        if (py >= w->height) py = w->height - 1;
                        double weight = (kx == 0 && ky == 0) ? k_center : ((kx == 0 || ky == 0) ? k_edge : 0.0);
                        sum += w->input[py * w->width + px] * weight;
                    }
                }
                w->output[idx] = sum;
            } else {
                w->output[idx] = w->input[idx] * k_center +
                                 (w->input[idx - w->width] +
                                  w->input[idx + w->width] +
                                  w->input[idx - 1] +
                                  w->input[idx + 1]) * k_edge;
            }
        }
    }
#endif
}

void tsfi_ccx_deconvolve_parallel(TSFiCCXPool *pool, const double *input_image, double *output_image, int width, int height, double noise_signal_ratio) {
    if (!pool || !input_image || !output_image || width <= 0 || height <= 0) return;
    
    int num_threads = pool->num_threads;
    TSFiDeconvWorkerArg args[MAX_CCX_THREADS];
    int rows_per_thread = height / num_threads;
    
    for (int i = 0; i < num_threads; i++) {
        args[i].input = input_image;
        args[i].output = output_image;
        args[i].width = width;
        args[i].height = height;
        args[i].nsr = noise_signal_ratio;
        args[i].start_y = i * rows_per_thread;
        args[i].end_y = (i == num_threads - 1) ? height : (i + 1) * rows_per_thread;
        
        tsfi_ccx_pool_enqueue(pool, tsfi_deconv_worker_func, &args[i]);
    }
    
    tsfi_ccx_pool_wait(pool);
}

