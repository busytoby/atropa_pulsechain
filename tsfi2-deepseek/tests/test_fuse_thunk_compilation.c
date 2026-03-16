#include "tsfi_wire_firmware.h"
#include "tsfi_reaction.h"
#include "tsfi_dys_math.h"
#include "tsfi_io.h"
#include "lau_memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <immintrin.h>

#define SHARED_SIZE (1024 * 1024)

typedef struct {
    LauWireFirmware_State rtl;
    _Atomic uint32_t turn_flag;
} SharedZMM;

int main() {
    tsfi_io_printf(stdout, "=== TSFi Firmware: Standard Cell JIT Transit Validation ===\n");
    
    SharedZMM* shared = (SharedZMM*)mmap(NULL, sizeof(SharedZMM), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (shared == MAP_FAILED) return 1;
    memset(shared, 0, sizeof(SharedZMM));

    TSFiBigInt* Prime = tsfi_bn_alloc(); tsfi_bn_set_u64(Prime, 953467954114363ULL);
    
    pid_t pid = fork();

    if (pid == 0) {
        // --- CHILD PROCESS: FIRMWARE (Eta) ---
        void* thunk_pool = mmap(NULL, 4096, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        
        while (atomic_load_explicit(&shared->turn_flag, memory_order_acquire) != 1) { }

        tsfi_io_printf(stdout, "[FIRMWARE] Detected incoming Standard Cell in ZMM buffer.\n");

        __m512i encrypted_thunk = _mm512_loadu_si512(&shared->rtl.wrf[100]);
        uint8_t thunk_bytes[64];
        _mm512_storeu_si512((__m512i*)thunk_bytes, encrypted_thunk);

        memcpy(thunk_pool, thunk_bytes, 16);

        void (*dynamic_cell)(LauRegisterBank*) = (void (*)(LauRegisterBank*))thunk_pool;

        LauRegisterBank test_regs;
        memset(&test_regs, 0, sizeof(test_regs));

        tsfi_io_printf(stdout, "[FIRMWARE] Executing dynamically mapped Standard Cell...\n");
        
        dynamic_cell(&test_regs);

        if (test_regs.sgpr[0] == 0x1337BEEF) {
            tsfi_io_printf(stdout, "[FIRMWARE] Standard Cell execution confirmed! SGPR[0] = 0x%X\n", test_regs.sgpr[0]);
            
            uint64_t ack[8] = {0};
            ack[0] = 0xFFFFFFFF;
            _mm512_storeu_si512(&shared->rtl.wrf[101], _mm512_loadu_si512((__m512i*)ack));
            atomic_store_explicit(&shared->turn_flag, 2, memory_order_release);
        } else {
            tsfi_io_printf(stderr, "[FIRMWARE] Execution failed. SGPR[0] is 0x%X\n", test_regs.sgpr[0]);
            exit(1);
        }

        munmap(thunk_pool, 4096);
        exit(0);

    } else {
        // --- PARENT PROCESS: COCKPIT (Mu) ---
        tsfi_io_printf(stdout, "[COCKPIT] Compiling Miniature Firmware (Standard Cell)...\n");

        uint8_t compiled_opcode[] = {
            0xc7, 0x07, 0xef, 0xbe, 0x37, 0x13, // mov DWORD PTR [rdi], 0x1337BEEF
            0xc3,                               // ret
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 // padding
        };

        uint64_t tx_payload[8] = {0};
        memcpy(tx_payload, compiled_opcode, sizeof(compiled_opcode));

        tsfi_io_printf(stdout, "[COCKPIT] Transmitting pure binary opcodes over ZMM mirror.\n");
        _mm512_storeu_si512(&shared->rtl.wrf[100], _mm512_loadu_si512((__m512i*)tx_payload));
        
        atomic_store_explicit(&shared->turn_flag, 1, memory_order_release);

        while (atomic_load_explicit(&shared->turn_flag, memory_order_acquire) != 2) { }

        tsfi_io_printf(stdout, "[SUCCESS] Firmware successfully received, mapped, and executed the dynamic Standard Cell.\n");

        int status;
        waitpid(pid, &status, 0);
    }

    tsfi_bn_free(Prime);
    munmap(shared, sizeof(SharedZMM));
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}