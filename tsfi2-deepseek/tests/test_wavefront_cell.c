#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdatomic.h>
#include "tsfi_wire_firmware.h"
#include "tsfi_wiring.h"
#include "tsfi_wire_pty.h"
#include "lau_wire_log.h"
#include "lau_memory.h"
#include "tsfi_raw.h"

#define TEST_LOG(name, status, msg) printf("  [%-4s] %-40s | %s\n", status, name, msg)

void strobe_prov(LauWireFirmware *fw, uint8_t addr, uint64_t data);

int main() {
    printf("\n=== TSFi Wavefront Standard Cell: Bi-Directional Interactive Loop Proof ===\n\n");
    
    tsfi_wire_firmware_init();
    LauWireFirmware *fw = tsfi_wire_firmware_get();
    
    // --- Setup Two Logs ---
    LauWireLog *rx_log = (LauWireLog*)lau_malloc_wired(sizeof(LauWireLog));
    lau_wire_log(rx_log);
    
    char *tx_buf = (char*)lau_malloc_wired(4096);
    memset(tx_buf, 0, 4096);
    // Wire TX Log via Provenance
    strobe_prov(fw, 0x23, (uintptr_t)tx_buf);
    strobe_prov(fw, 0x20, 0); // head
    strobe_prov(fw, 0x21, 0); // tail
    
    // 1. Authorize Many Epochs
    strobe_prov(fw, 0x0D, 10000); 
    
    // 2. Launch 'cat' (to test echo)
    alignas(64) char cmd_wave[64] = "cat";
    tsfi_wire_firmware_load_waveform(fw, 0, cmd_wave);
    printf("[INFO] Strobing 'GEMINI' directive (cat)...\n");
    fw->step_executor_directive("GEMINI");
    
    // Step driver to spawn
    tsfi_wire_firmware_step_peripheral(fw, NULL);
    
    // 3. Send Input through tx_log
    const char *msg = "HELLO_GEMINI_INTERACTIVE\n";
    memcpy(tx_buf, msg, strlen(msg));
    strobe_prov(fw, 0x20, strlen(msg)); // Set tx_head
    printf("[DEBUG] tx_buf=%p, msg=%s", (void*)tx_buf, msg);
    printf("[DEBUG] fw->rtl.log_stdin_head=%u, tail=%u, ptr=%lx\n", fw->rtl.log_stdin_head, fw->rtl.log_stdin_tail, (unsigned long)fw->rtl.log_stdin_ptr);
    TEST_LOG("tx_write", "PASS", "Interactive message written to hardware TX log");

    // 4. Drive epochs to process TX and capture RX
    printf("[INFO] Driving Epoch Handshake...\n");
    for(int i=0; i<1000; i++) {
        tsfi_wire_firmware_strobe(fw, 0x0D, 10001 + (uint64_t)i); // increment host_epoch
        fw->rtl.periph_state_ready = true;
        tsfi_wire_firmware_step_peripheral(fw, NULL);
        
        // Polling read more aggressively
        if (i > 10) {
            if (*rx_log->head > 0 && strstr(rx_log->buffer, "INTERACTIVE") != NULL) break;
        }
        tsfi_raw_usleep(1000);
    }
    
    // 5. Verify Echo in rx_log
    tsfi_wire_firmware_validate(fw);
    printf("[DEBUG] rx_log head=%u, valid_head=%u\n", *rx_log->head, *rx_log->valid_head);
    printf("[DEBUG] RAW RX BUFFER HEX: ");
    for(int i=0; i<128; i++) printf("%02x ", (unsigned char)rx_log->buffer[i]);
    printf("\n");
    if (strstr(rx_log->buffer, "INTERACTIVE") != NULL) {
        TEST_LOG("interactive_echo", "PASS", "Successfully captured Bi-Directional zero-copy echo");
        printf("[INFO] RX Log Buffer: %s", rx_log->buffer);
    } else {
        printf("  [FAIL] Echo failed! RX Buffer: %.64s\n", rx_log->buffer);
        return 1;
    }

    printf("--------------------------------------------------------------------------------\n");
    printf("RESULT: Bi-Directional Interactive Loop verification SUCCESSFUL.\n");
    printf("STATUS: Full-Duplex zero-copy operational channel secured.\n\n");

    lau_free(rx_log);
    lau_free(tx_buf);
    lau_free(fw); 
        extern void tsfi_wire_firmware_teardown(void);
    tsfi_wire_firmware_teardown();
    extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}

void strobe_prov(LauWireFirmware *fw, uint8_t addr, uint64_t data) {
    fw->rtl.prov_data = data; fw->rtl.prov_addr = addr; fw->rtl.prov_strobe = true; fw->rtl.epoch_strobe = true;
    LauWireFirmware_eval_sequential(&fw->rtl);
    fw->rtl.epoch_strobe = false; fw->rtl.prov_strobe = false; 
    LauWireFirmware_eval_combinatorial(&fw->rtl);
}
