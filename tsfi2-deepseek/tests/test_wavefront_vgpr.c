#include "tsfi_wiring.h"
#include "tsfi_types.h"
#include "lau_memory.h"
#include <stdio.h>
#include <string.h>
#include <immintrin.h> // For __m512
#include <unistd.h> // For tsfi_raw_usleep
#include "tsfi_raw.h"

#define BATCH_SIZE 16 // Number of floats per ZMM register

// --- AVX-512 Kernel for Vector Addition ---
__attribute__((naked)) void vgpr_add_kernel(void *context) {
    (void)context;
    __asm__ volatile (
        "vaddps %%zmm1, %%zmm0, %%zmm3\n\t"
        "ret\n\t"
        : : : "zmm3", "zmm0", "zmm1"
    );
}

int main() {
    printf("[TEST] Starting Wavefront VGPR Test...\n");

    // 1. Initialize WaveSystem
    WaveSystem *ws = tsfi_create_system();
    if (!ws) {
        fprintf(stderr, "[ERROR] Failed to create WaveSystem.\n");
        return 1;
    }

    // 2. Create WavefrontContext
    WavefrontContext *wf = tsfi_create_wavefront(ws);
    if (!wf) {
        fprintf(stderr, "[ERROR] Failed to create WavefrontContext.\n");
        lau_free(ws);
        return 1;
    }

    // 3. Prepare Input/Output Data
    float *input_x = (float*)lau_memalign(64, BATCH_SIZE * sizeof(float));
    float *input_y = (float*)lau_memalign(64, BATCH_SIZE * sizeof(float));
    float *output_data = (float*)lau_memalign(64, BATCH_SIZE * sizeof(float));
    
    for (int i = 0; i < BATCH_SIZE; i++) {
        input_x[i] = 100.0f + (float)i;
        input_y[i] = 200.0f + (float)i;
        output_data[i] = 0.0f; // Clear output
    }

    wf->input_x = input_x;
    wf->input_y = input_y;
    wf->output_data = output_data; // Output will go to ZMM3, then stored here

    printf("[DEBUG] input_x ptr: %p, val[0]: %.2f\n", (void*)input_x, input_x[0]);
    printf("[DEBUG] input_y ptr: %p, val[0]: %.2f\n", (void*)input_y, input_y[0]);
    printf("[DEBUG] output_data ptr: %p\n", (void*)output_data);
    printf("[DEBUG] wf->input_x field addr: %p, field val: %p\n", (void*)&wf->input_x, (void*)wf->input_x);

    // 4. Rebind Kernel
    printf("[TEST] Rebinding execute_vgpr_kernel to vgpr_add_kernel.\n");
    ThunkProxy_rebind((void*)wf->execute_vgpr_kernel, (void*)vgpr_add_kernel);

    // 5. Execute Pipeline
    printf("[TEST] Executing VGPR pipeline: Load X -> Load Y -> Add -> Store Output.\n");
    wf->load_x_to_vgpr();      // Loads input_x to ZMM0
    wf->load_y_to_vgpr();      // Loads input_y to ZMM1
    wf->execute_vgpr_kernel(NULL); // Executes ZMM3 = ZMM0 + ZMM1
    wf->store_output_from_vgpr(); // Stores ZMM3 to output_data

    // 6. Verify Results
    int failures = 0;
    for (int i = 0; i < BATCH_SIZE; i++) {
        float expected = input_x[i] + input_y[i];
        if (output_data[i] != expected) {
            fprintf(stderr, "[ERROR] Verification failed at index %d: Expected %.2f, got %.2f\n", i, expected, output_data[i]);
            failures++;
        }
    }

    if (failures == 0) {
        printf("[TEST] VGPR Vector Add: PASSED\n");
    } else {
        fprintf(stderr, "[TEST] VGPR Vector Add: FAILED (%d failures)\n", failures);
    }
    
    // 7. Cleanup
    printf("[TEST] Cleaning up WavefrontContext and WaveSystem.\n");
    lau_free(input_x);
    lau_free(input_y);
    lau_free(output_data);
    lau_free(wf);
    lau_free(ws);

    printf("[TEST] Wavefront VGPR Test Finished.\n");

    return failures > 0 ? 1 : 0;


    extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
}
