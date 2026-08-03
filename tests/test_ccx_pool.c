#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "../tsfi2-deepseek/inc/tsfi_ccx_pool.h"
#include "../tsfi2-deepseek/inc/tsfi_depthoffield.h"

typedef struct {
    const double *input;
    double *output;
    int width;
    int height;
    double nsr;
} DeconvTaskArg;

void deconv_task_func(void *arg) {
    DeconvTaskArg *task = (DeconvTaskArg *)arg;
    tsfi_depthoffield_wiener_deconvolve(task->input, task->output, task->width, task->height, task->nsr);
}

int main(void) {
    printf("=== RUNNING AUNCIENT CCX POOL TESTS ===\n");
    
    TSFiCCXPool pool0, pool1;
    int ret0 = tsfi_ccx_pool_init(&pool0, 0, 4);
    int ret1 = tsfi_ccx_pool_init(&pool1, 1, 4);
    
    assert(ret0 == 0);
    assert(ret1 == 0);
    printf("   ✓ CCX Pool 0 and 1 initialized successfully.\n");
    
    // Allocate dummy images
    int width = 128;
    int height = 128;
    double *img_in = malloc(width * height * sizeof(double));
    double *img_out0 = malloc(width * height * sizeof(double));
    double *img_out1 = malloc(width * height * sizeof(double));
    
    for (int i = 0; i < width * height; i++) {
        img_in[i] = (double)(i % 100);
    }
    
    DeconvTaskArg arg0 = { img_in, img_out0, width, height, 0.01 };
    DeconvTaskArg arg1 = { img_in, img_out1, width, height, 0.02 };
    
    // Enqueue deconvolution tasks to separate CCXs in parallel
    tsfi_ccx_pool_enqueue(&pool0, deconv_task_func, &arg0);
    tsfi_ccx_pool_enqueue(&pool1, deconv_task_func, &arg1);
    
    // Wait for all tasks to complete
    tsfi_ccx_pool_wait(&pool0);
    tsfi_ccx_pool_wait(&pool1);
    printf("   ✓ CCX parallel tasks dispatched and completed successfully.\n");
    
    // Test direct CCX-parallel deconvolution driver
    double *img_out_parallel = malloc(width * height * sizeof(double));
    tsfi_ccx_deconvolve_parallel(&pool0, img_in, img_out_parallel, width, height, 0.01);
    assert(img_out_parallel[100] == img_out0[100]);
    printf("   ✓ CCX direct parallel deconvolution driver output verified.\n");
    
    // Test multi-CCX parallel deconvolution scheduler
    double *img_out_multi = malloc(width * height * sizeof(double));
    TSFiCCXPool *pools[2] = { &pool0, &pool1 };
    tsfi_multi_ccx_deconvolve_parallel(pools, 2, img_in, img_out_multi, width, height, 0.01);

    // Since region boundaries might differ slightly in convolution edge clamping, compare center pixels
    assert(img_out_multi[width * 64 + 64] == img_out0[width * 64 + 64]);
    printf("   ✓ Multi-CCX parallel deconvolution output verified.\n");
    free(img_out_multi);
    
    // Validate output is non-zero
    assert(img_out0[100] != 0.0);
    assert(img_out1[100] != 0.0);
    printf("   ✓ Deconvolution results processed by CCX pool threads verified.\n");
    
    tsfi_ccx_pool_destroy(&pool0);
    tsfi_ccx_pool_destroy(&pool1);
    printf("   ✓ CCX pools shut down and destroyed cleanly.\n");
    free(img_out_parallel);


    
    free(img_in);
    free(img_out0);
    free(img_out1);
    
    printf("=== AUNCIENT CCX POOL TESTS COMPLETE (PASS) ===\n");
    return 0;
}
