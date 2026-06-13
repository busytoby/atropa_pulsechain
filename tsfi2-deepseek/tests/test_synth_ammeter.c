#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "tsfi_zmm_vm.h"
#include "tsfi_ammeter.h"

// Helper to convert hex byte string to binary array
static size_t hex_to_bytes(const char *hex, uint8_t *buffer, size_t max_len) {
    if (strncmp(hex, "0x", 2) == 0) hex += 2;
    size_t len = strlen(hex) / 2;
    if (len > max_len) len = max_len;
    for (size_t i = 0; i < len; i++) {
        unsigned int val;
        sscanf(hex + i * 2, "%2x", &val);
        buffer[i] = (uint8_t)val;
    }
    return len;
}

int main() {
    printf("=== TSFi2 Folklore Yul CPU & Speech Synthesizer to Ammeter Test ===\n");

    TsfiZmmVmState vm;
    tsfi_zmm_vm_init(&vm);

    // 1. Initialize speechSynthesizer Yul contract (at standard address 0x8ff9a7885e7CB3C01D556e1bC4f9fb2A6e7Ac167)
    printf("[TEST] Loading speechSynthesizer Yul contract...\n");
    tsfi_zmm_vm_exec(&vm, "YULINIT \"speechSynthesizer\", \"../solidity/bin/speechSynthesizer.yul\", 0xc4f9fb2a6e7ac167");
    if (strstr(vm.output_buffer, "Error") != NULL) {
        printf("[TEST] Error loading speechSynthesizer: %s\n", vm.output_buffer);
        tsfi_zmm_vm_destroy(&vm);
        return 1;
    }

    // 1a. Upload neural weights to speechSynthesizer storage
    printf("[TEST] Uploading neural weights...\n");
    tsfi_zmm_vm_exec(&vm, "YULEXEC \"speechSynthesizer\", \"200c2ae0"
        "0000000000000000000000000000000000000000000000000000000000000000"
        "05000aec50f10ffb0ff6050ae246f61405fb0a000fec3cf6f60afb05000aec32\"");
    tsfi_zmm_vm_exec(&vm, "YULEXEC \"speechSynthesizer\", \"200c2ae0"
        "0000000000000000000000000000000000000000000000000000000000000001"
        "5af10fecf1140af6e755f60f05f6fb050afb4bf60a050af1ec14f141f60f000a\"");

    // 2. Initialize folklore (cpu6502) Yul contract
    printf("[TEST] Loading folklore cpu6502 Yul contract...\n");
    tsfi_zmm_vm_exec(&vm, "YULINIT \"cpu6502\", \"../solidity/bin/folklore.yul\", 0x12345");
    if (strstr(vm.output_buffer, "Error") != NULL) {
        printf("[TEST] Error loading folklore: %s\n", vm.output_buffer);
        tsfi_zmm_vm_destroy(&vm);
        return 1;
    }
    // 2a. Bind folklore CPU address (0x12345) to speechSynthesizer
    printf("[TEST] Binding CPU address to speechSynthesizer...\n");
    tsfi_zmm_vm_exec(&vm, "YULEXEC \"speechSynthesizer\", \"678575ea0000000000000000000000000000000000000000000000000000000000012345\"");

    // 3. Execute selector 0x51c726a8 (Kiai Speech Synth) with eventId = 1 (Jump sound)
    printf("[TEST] Executing folklore sound synthesis (eventId = 1)...\n");
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, "YULEXEC \"cpu6502\", \"51c726a8"
        "0000000000000000000000000000000000000000000000000000000000000001" // eventId = 1
        "\"");

    if (strlen(vm.output_buffer) < 128) {
        printf("[TEST] Error: Synthesis output too short or failed: %s\n", vm.output_buffer);
        tsfi_zmm_vm_destroy(&vm);
        return 1;
    }

    // Parse WAV data from execution output
    char len_hex[65];
    strncpy(len_hex, vm.output_buffer, 64);
    len_hex[64] = '\0';
    unsigned long wav_len = strtoul(len_hex, NULL, 16);
    printf("[TEST] Received synthesized WAV size: %lu bytes\n", wav_len);

    uint8_t *wav_bytes = malloc(wav_len);
    if (!wav_bytes) {
        tsfi_zmm_vm_destroy(&vm);
        return 1;
    }
    hex_to_bytes(vm.output_buffer + 64, wav_bytes, wav_len);

    // Parse PCM samples from WAV (WAV starts with 44-byte header, followed by data chunk)
    // Assuming 8-bit mono PCM samples at the end of the header
    size_t header_offset = 44;
    if (wav_len <= header_offset) {
        printf("[TEST] Error: WAV file too small to contain audio samples.\n");
        free(wav_bytes);
        tsfi_zmm_vm_destroy(&vm);
        return 1;
    }
    size_t num_samples = wav_len - header_offset;
    float *input_rf = malloc(num_samples * sizeof(float));
    float *out_theta = malloc(num_samples * sizeof(float));

    for (size_t i = 0; i < num_samples; i++) {
        // Convert uint8 PCM (0-255) to normalized float (-1.0 to 1.0) and scale to simulated Amps
        float normalized = ((float)wav_bytes[header_offset + i] - 128.0f) / 128.0f;
        input_rf[i] = normalized * 5.0f; // Scale to max 5 Amps
    }

    printf("[TEST] Successfully parsed %zu audio samples from Yul synthesizer.\n", num_samples);

    // 4. Initialize and run hot-wire ammeter on the synthesized speech waveform
    TsfiHotWireAmmeter ammeter;
    tsfi_ammeter_init(&ammeter);

    tsfi_ammeter_process(&ammeter, input_rf, out_theta, num_samples, 8000.0);

    // Verify ammeter needle deflection
    double max_theta = 0.0;
    size_t max_idx = 0;
    for (size_t i = 0; i < num_samples; i++) {
        if (out_theta[i] > max_theta) {
            max_theta = out_theta[i];
            max_idx = i;
        }
    }

    printf("[TEST] Peak Needle Deflection: %.4f rad achieved at sample %zu\n", max_theta, max_idx);
    printf("[TEST] Final Heater Temperature: %.2f K (ambient: %.2f K)\n", ammeter.T, ammeter.T_ambient);

    if (max_theta > 0.005) {
        printf("[SUCCESS] Folklore Yul CPU speech synthesis successfully verified on the ammeter!\n");
    } else {
        printf("[FAILURE] Ammeter failed to deflect from speech synthesis output.\n");
        free(wav_bytes); free(input_rf); free(out_theta);
        tsfi_zmm_vm_destroy(&vm);
        return 1;
    }

    free(wav_bytes); free(input_rf); free(out_theta);
    tsfi_zmm_vm_destroy(&vm);
    return 0;
}
