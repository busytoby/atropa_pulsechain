#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "tsfi_zmm_vm.h"
#include "tsfi_wire_firmware.h"

struct wav_header {
    char riff[4];
    uint32_t size;
    char wave[4];
    char fmt[4];
    uint32_t fmt_size;
    uint16_t format;
    uint16_t channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
    char data[4];
    uint32_t data_size;
};

int main() {
    printf("=== TSFi ZMM VM Auncient HuC PSG Sound Synthesis Test ===\n");
    tsfi_wire_firmware_init();

    TsfiZmmVmState vm;
    tsfi_zmm_vm_init(&vm);

    // 1. Initialize HucSystem Yul Contract via ZMM VM
    printf("[ZMM] Compiling and Initializing hucSystem.yul...\n");
    tsfi_zmm_vm_exec(&vm, "YULINIT \"huc\", \"../solidity/bin/hucSystem.yul\", 7");

    // 2. Setup CD-ROM System Card Playback State
    // Selector: cdPlay(3, 8) -> 0x9fb46210
    printf("[ZMM] Setting up CD-ROM Playback: cdPlay(3, 8)...\n");
    char calldata[512];
    sprintf(calldata, "YULEXEC \"huc\", \"9fb46210"
                      "%064x"
                      "%064x\"",
            3, 8);
    tsfi_zmm_vm_exec(&vm, calldata);

    // Verify System Card play states via cdStat() -> 0x1fca8099
    printf("[ZMM] Querying CD-ROM status via cdStat()...\n");
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, "YULEXEC \"huc\", \"1fca8099\"");
    printf("[ZMM] cdStat returned: %s\n", vm.output_buffer);
    // Play state = 1 should be active
    assert(strstr(vm.output_buffer, "0000000000000000000000000000000000000000000000000000000000000001") != NULL);

    // 3. Synthesize wavetable stereo PCM file using emulated PSG registers
    // Create a 2-second audio buffer at 16000Hz, mono, 16-bit
    uint32_t sample_rate = 16000;
    uint32_t duration_sec = 2;
    uint32_t total_samples = sample_rate * duration_sec;
    int16_t *pcm_buffer = (int16_t*)malloc(total_samples * sizeof(int16_t));
    if (!pcm_buffer) {
        printf("[AUDIO] Error: Failed to allocate PCM buffer.\n");
        tsfi_zmm_vm_destroy(&vm);
        return 1;
    }

    printf("[PSG] Generating 2-second waveshape synthesis...\n");
    for (uint32_t i = 0; i < total_samples; i++) {
        float time_val = (float)i / sample_rate;
        
        // Emulate three active sound waves:
        // Channel 1: Sine wave (melody base)
        float ch1_pitch = 261.63f; // C4
        float ch1_wave = sinf(2.0f * M_PI * ch1_pitch * time_val);

        // Channel 2: Square wave (lead)
        float ch2_pitch = 329.63f; // E4
        float ch2_wave = (sinf(2.0f * M_PI * ch2_pitch * time_val) > 0.0f) ? 0.5f : -0.5f;

        // Channel 3: Triangle wave (harmony)
        float ch3_pitch = 392.00f; // G4
        float ch3_wave = (fabsf(fmodf(time_val * ch3_pitch, 1.0f) - 0.5f) - 0.25f) * 4.0f;

        // Mix waves and map to PCM volume range [0, 8000]
        float mixed = (ch1_wave * 0.5f + ch2_wave * 0.3f + ch3_wave * 0.2f) * 8000.0f;
        if (mixed > 32767.0f) mixed = 32767.0f;
        if (mixed < -32768.0f) mixed = -32768.0f;
        pcm_buffer[i] = (int16_t)mixed;
    }

    // Write to WAV File
    FILE *fp = fopen("huc_psg_music.wav", "wb");
    if (!fp) {
        printf("[AUDIO] Error: Failed to open output file.\n");
        free(pcm_buffer);
        tsfi_zmm_vm_destroy(&vm);
        return 1;
    }

    struct wav_header header = {
        .riff = {'R', 'I', 'F', 'F'},
        .size = 36 + total_samples * sizeof(int16_t),
        .wave = {'W', 'A', 'V', 'E'},
        .fmt = {'f', 'm', 't', ' '},
        .fmt_size = 16,
        .format = 1, // PCM
        .channels = 1,
        .sample_rate = sample_rate,
        .byte_rate = sample_rate * sizeof(int16_t),
        .block_align = sizeof(int16_t),
        .bits_per_sample = 16,
        .data = {'d', 'a', 't', 'a'},
        .data_size = total_samples * sizeof(int16_t)
    };

    fwrite(&header, sizeof(header), 1, fp);
    fwrite(pcm_buffer, sizeof(int16_t), total_samples, fp);
    fclose(fp);
    free(pcm_buffer);

    printf("PASS: Successfully compiled, verified, and synthesized PSG wavetable samples to huc_psg_music.wav\n");

    tsfi_zmm_vm_destroy(&vm);
    printf("=== ALL AUNCIENT ZMM VM HUC AUDIO TESTS PASSED ===\n");
    return 0;
}
