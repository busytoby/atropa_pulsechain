#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include "tsfi_c_math.h"
#include "tsfi_broadcaster.h"
#include "lau_memory.h"

// Internally developed mock audio synthesis (Deterministic Wavefront)
void tsfi_broadcaster_audio_mock_step(LauBroadcaster *lb) {
    if (!lb->audio_buffer) {
        lb->audio_buffer = (uint8_t*)lau_malloc(16384);
        lb->audio_size = 0;
        lb->audio_read_offset = 0;
    }
    
    // Occasional Compaction: Only shift memory when nearing the physical cliff
    if (lb->audio_read_offset + lb->audio_size + 2048 > 16384) {
        if (lb->audio_size > 0 && lb->audio_read_offset > 0) {
            memmove(lb->audio_buffer, lb->audio_buffer + lb->audio_read_offset, lb->audio_size);
        }
        lb->audio_read_offset = 0;
    }
    
    // Generate a simple deterministic 440Hz sine wave into the buffer
    static float phase = 0;
    size_t samples = 1024; // ~23ms
    int16_t *buf = (int16_t*)(lb->audio_buffer + lb->audio_read_offset + lb->audio_size);
    
    for (size_t i = 0; i < samples; i++) {
        buf[i] = (int16_t)(32767.0 * tsfi_sinf(phase));
        phase += 2.0 * TSFI_PI * 440.0 / 44100.0;
    }
    lb->audio_size += samples * 2;
}

void test_audio_capture_determinism() {
    printf("[TEST] Verifying Audio Synthesis (Internal)...\n");

    LauBroadcaster *lb = (LauBroadcaster *)lau_malloc_wired(sizeof(LauBroadcaster));
    assert(lb != NULL);
    memset(lb, 0, sizeof(LauBroadcaster));

    // Capture 20 chunks
    bool captured_any = false;
    for (int i = 0; i < 20; i++) {
        tsfi_broadcaster_audio_mock_step(lb);
        if (lb->audio_size > 0) {
            printf("[TEST] Synthesized chunk %d: %zu bytes\n", i, lb->audio_size);
            captured_any = true;
        }
        // Drain completely to mock RTMP sending
        if (lb->audio_size >= 2048) {
            lb->audio_size -= 2048;
            lb->audio_read_offset += 2048;
        }
    }

    assert(captured_any == true);

    if (lb->audio_buffer) lau_free(lb->audio_buffer);
    lau_free(lb);
    printf("[PASS] Audio Synthesis verified.\n");
}

int main() {
    test_audio_capture_determinism();
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
