#include "tsfi_wire_firmware.h"
#include "tsfi_reaction.h"
#include "tsfi_dys_math.h"
#include "tsfi_io.h"
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
    tsfi_io_printf(stdout, "=== TSFi Encrypted Buffer Security: Bidirectional IPC Validation ===\n");
    
    SharedZMM* shared = (SharedZMM*)mmap(NULL, sizeof(SharedZMM), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (shared == MAP_FAILED) return 1;
    memset(shared, 0, sizeof(SharedZMM));

    TSFiBigInt* Prime = tsfi_bn_alloc(); tsfi_bn_set_u64(Prime, 953467954114363ULL);
    TSFiBigInt* Xi = tsfi_bn_alloc(); tsfi_bn_randomize(Xi);
    TSFiBigInt* Rod_Identity = tsfi_bn_alloc(); tsfi_bn_randomize(Rod_Identity);
    TSFiBigInt* Cone_Identity = tsfi_bn_alloc(); tsfi_bn_randomize(Cone_Identity);

    pid_t pid = fork();

    if (pid == 0) {
        TSFiHelmholtzAdductState rod_state = {0};
        rod_state.Prime = Prime; rod_state.Xi = Xi; rod_state.Beta = Rod_Identity;
        rod_state.current_epoch = HELMHOLTZ_EPOCH_INIT;
        (void)rod_state;
        
        struct YI* root_yi = tsfi_reaction_shoot(Prime);
        TSFiBigInt* Pi = root_yi->Ring ? root_yi->Ring : root_yi->Xi;
        struct Dai* rod_dai = tsfi_reaction_compute_reciprocity(root_yi->Psi, Pi, NULL);

        while (atomic_load_explicit(&shared->turn_flag, memory_order_acquire) != 1) { }

        __m512i encrypted_rx = _mm512_loadu_si512(&shared->rtl.wrf[125]);
        uint64_t rx_buffer[8];
        _mm512_storeu_si512((__m512i*)rx_buffer, encrypted_rx);

        uint64_t decrypted_val = rx_buffer[0] ^ rod_dai->Ichidai->limbs[0];
        
        if (decrypted_val == 0xDEADBEEFCAFEBABE) {
            tsfi_io_printf(stdout, "[FIRMWARE] Successfully decrypted incoming ZMM Command from Cockpit: 0x%lX\n", decrypted_val);
            
            uint64_t ack_payload[8] = {0};
            ack_payload[0] = 0x9999999999999999 ^ rod_dai->Daiichi->limbs[0];
            
            _mm512_storeu_si512(&shared->rtl.wrf[126], _mm512_loadu_si512((__m512i*)ack_payload));
            atomic_store_explicit(&shared->turn_flag, 2, memory_order_release);
        } else {
            tsfi_io_printf(stderr, "[FIRMWARE] Decryption failed. Got 0x%lX\n", decrypted_val);
            exit(1);
        }

        freeDAI(rod_dai); freeYI(root_yi);
        exit(0);

    } else {
        struct YI* root_yi = tsfi_reaction_shoot(Prime); 
        TSFiBigInt* Pi = root_yi->Ring ? root_yi->Ring : root_yi->Xi;
        struct Dai* cone_dai = tsfi_reaction_compute_reciprocity(root_yi->Psi, Pi, NULL);

        uint64_t tx_payload[8] = {0};
        uint64_t raw_command = 0xDEADBEEFCAFEBABE;
        tx_payload[0] = raw_command ^ cone_dai->Ichidai->limbs[0]; 

        tsfi_io_printf(stdout, "[COCKPIT] Transmitting encrypted ZMM Command (0x%lX) to Firmware...\n", raw_command);
        
        _mm512_storeu_si512(&shared->rtl.wrf[125], _mm512_loadu_si512((__m512i*)tx_payload));
        atomic_store_explicit(&shared->turn_flag, 1, memory_order_release);

        while (atomic_load_explicit(&shared->turn_flag, memory_order_acquire) != 2) { }

        __m512i encrypted_ack = _mm512_loadu_si512(&shared->rtl.wrf[126]);
        uint64_t ack_buffer[8];
        _mm512_storeu_si512((__m512i*)ack_buffer, encrypted_ack);

        uint64_t decrypted_ack = ack_buffer[0] ^ cone_dai->Daiichi->limbs[0];
        
        if (decrypted_ack == 0x9999999999999999) {
            tsfi_io_printf(stdout, "[COCKPIT] Successfully received encrypted acknowledgment from Firmware: 0x%lX\n", decrypted_ack);
            tsfi_io_printf(stdout, "[SUCCESS] Bidirectional IPC stream cipher mapping is fully operational.\n");
        } else {
            tsfi_io_printf(stderr, "[COCKPIT] Failed to decrypt acknowledgment. Got: 0x%lX\n", decrypted_ack);
        }

        int status;
        waitpid(pid, &status, 0);

        freeDAI(cone_dai); freeYI(root_yi);
    }

    tsfi_bn_free(Rod_Identity); tsfi_bn_free(Cone_Identity);
    tsfi_bn_free(Prime); tsfi_bn_free(Xi);
    munmap(shared, sizeof(SharedZMM));
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}