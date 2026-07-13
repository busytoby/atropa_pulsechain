#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <pulse/simple.h>
#include <pulse/error.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT ZMM VM: PHYSICAL PULSEAUDIO SPEAKER PLAYBACK\n");
    printf("=============================================================\n");

    // 1. Configure PulseAudio sample specification
    pa_sample_spec ss;
    ss.format = PA_SAMPLE_S16LE;
    ss.rate = 44100;
    ss.channels = 1;

    int error;
    pa_simple *s = NULL;

    // 2. Open PulseAudio playback connection
    printf("1. Connecting to PulseAudio server...\n");
    s = pa_simple_new(NULL,               // Use default server
                      "ZMM_MCP_Server",   // Application name
                      PA_STREAM_PLAYBACK,
                      NULL,               // Use default device
                      "Synthesizer",      // Stream description
                      &ss,                // Sample format spec
                      NULL,               // Channel map
                      NULL,               // Buffering attributes
                      &error);            // Error code output

    if (!s) {
        fprintf(stderr, "pa_simple_new() failed: %s\n", pa_strerror(error));
        return 1;
    }

    // Am7 arpeggio note sequence
    double score[4] = { 440.0, 523.25, 659.25, 783.99 };
    uint32_t sample_rate = 44100;
    double duration = 0.4; // 0.4 seconds per note
    uint32_t samples_per_note = (uint32_t)(sample_rate * duration);

    int16_t *buffer = malloc(samples_per_note * sizeof(int16_t));
    if (!buffer) {
        perror("Failed to allocate buffer");
        pa_simple_free(s);
        return 1;
    }

    printf("2. Streaming audio frames to PulseAudio server (Multi-App Mixing)...\n");
    for (int note = 0; note < 4; note++) {
        double freq = score[note];
        printf("   [PulseAudio] Playing Note %d: %.2f Hz\n", note + 1, freq);
        
        // Generate sine wave samples with envelope to prevent clicking
        for (uint32_t i = 0; i < samples_per_note; i++) {
            double t = (double)i / sample_rate;
            double raw_val = sin(2.0 * M_PI * freq * t);
            
            double envelope = 1.0;
            if (i < 800) {
                envelope = (double)i / 800.0;
            } else if (i > samples_per_note - 800) {
                envelope = (double)(samples_per_note - i) / 800.0;
            }
            
            buffer[i] = (int16_t)(raw_val * envelope * 16384.0);
        }
        
        // Write PCM data to PulseAudio stream
        if (pa_simple_write(s, buffer, samples_per_note * sizeof(int16_t), &error) < 0) {
            fprintf(stderr, "pa_simple_write() failed: %s\n", pa_strerror(error));
            break;
        }
    }

    // Flush and Clean up
    if (pa_simple_drain(s, &error) < 0) {
        fprintf(stderr, "pa_simple_drain() failed: %s\n", pa_strerror(error));
    }

    free(buffer);
    pa_simple_free(s);
    
    printf("\nPlayback completed successfully.\n");
    printf("=============================================================\n");
    return 0;
}
