#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "tsfi_zmm_rpc.h"
#include "tsfi_wiring.h"
#include "lau_memory.h"

int main() {
    printf("=== TSFi Dynamic Availment Verification (Epoch 1) ===\n");
    
    // 1. Initialize System
    lau_memory_init_gpu(NULL);
    (void)tsfi_create_system();
    TsfiZmmVmState vm;
    tsfi_zmm_vm_init(&vm);
    
    // Clear legacy files
    unlink("/tmp/tsfi_llm_in");
    unlink("/tmp/tsfi_llm_out.txt");

    pid_t pid = fork();
    if (pid == 0) {
        // --- CHILD: DeepSeek Responder Simulation ---
        printf("[CHILD] DeepSeek Responder Active. Waiting for Pulse...\n");
        
        char pulse_hex[65] = {0};
        while (1) {
            FILE *fin = fopen("/tmp/tsfi_llm_in", "r");
            if (fin) {
                char line[4096];
                while (fgets(line, sizeof(line), fin)) {
                    char *p = strstr(line, "Availment Pulse: ");
                    if (p) {
                        strncpy(pulse_hex, p + 17, 64);
                        pulse_hex[64] = 0;
                        break;
                    }
                }
                fclose(fin);
                if (pulse_hex[0] != 0) break;
            }
            usleep(100000); // 100ms
        }
        
        printf("[CHILD] Pulse Inhaled: %s\n", pulse_hex);
        
        // Perform Reciprocal Shift (XOR 0x89)
        char shifted_hex[65];
        for (int i = 0; i < 32; i++) {
            unsigned int val;
            sscanf(&pulse_hex[i*2], "%02x", &val);
            sprintf(&shifted_hex[i*2], "%02x", (val ^ 0x89) & 0xFF);
        }
        shifted_hex[64] = 0;
        
        printf("[CHILD] Exhaling Shifted Proof: %s\n", shifted_hex);
        FILE *fout = fopen("/tmp/tsfi_llm_out.txt", "w");
        fprintf(fout, "AVAILED: %s", shifted_hex);
        fclose(fout);
        
        exit(0);
    } else {
        // --- PARENT: RPC Caller ---
        printf("[PARENT] Triggering Manifold Handshake...\n");
        char output[4096];
        const char *json_start = "{\"jsonrpc\": \"2.0\", \"method\": \"manifold.start_handshake\", \"id\": 1}";
        
        tsfi_zmm_rpc_dispatch(&vm, json_start, output, sizeof(output));
        printf("RPC Response: %s", output);
        
        if (strstr(output, "Verified Perfectly") || strstr(output, "Verified (Epoch 1)")) {
            printf("[PASS] Dynamic Availment Proof Synchronized Successfully.\n");
        } else {
            printf("[FAIL] Handshake fracture detected!\n");
            kill(pid, SIGKILL);
            return 1;
        }
        
        wait(NULL);
    }

    tsfi_zmm_vm_destroy(&vm);
    lau_free_all_active();
    
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    extern void lau_assert_zero_unsealed_leaks(const char *context, void *teardown_ptr);
    lau_assert_zero_unsealed_leaks("test", NULL);
    return 0;
}
