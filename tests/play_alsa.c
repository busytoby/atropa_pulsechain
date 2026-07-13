#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <alsa/asoundlib.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT ZMM VM: PHYSICAL ALSA SPEAKER PLAYBACK\n");
    printf("=============================================================\n");

    snd_pcm_t *handle;
    int err;

    // 1. Open PCM device for playback
    printf("1. Opening ALSA default PCM playback device...\n");
    if ((err = snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
        fprintf(stderr, "Playback open error: %s\n", snd_strerror(err));
        return 1;
    }

    // 2. Set hardware parameters (Mono, 44100Hz, 16-bit PCM)
    printf("2. Configuring ALSA hardware parameters...\n");
    if ((err = snd_pcm_set_params(handle,
                                  SND_PCM_FORMAT_S16_LE,
                                  SND_PCM_ACCESS_RW_INTERLEAVED,
                                  1,              // 1 channel (Mono)
                                  44100,          // Sample rate (Hz)
                                  1,              // Soft resample allowed
                                  500000)) < 0) { // Latency (us)
        fprintf(stderr, "Playback parameters error: %s\n", snd_strerror(err));
        snd_pcm_close(handle);
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
        snd_pcm_close(handle);
        return 1;
    }

    printf("3. Streaming audio frames directly to physical speakers...\n");
    for (int note = 0; note < 4; note++) {
        double freq = score[note];
        printf("   [Speaker] Playing Note %d: %.2f Hz\n", note + 1, freq);
        
        // Generate sine wave samples
        for (uint32_t i = 0; i < samples_per_note; i++) {
            double t = (double)i / sample_rate;
            double raw_val = sin(2.0 * M_PI * freq * t);
            
            // Fade-in/out to avoid clicking
            double envelope = 1.0;
            if (i < 800) {
                envelope = (double)i / 800.0;
            } else if (i > samples_per_note - 800) {
                envelope = (double)(samples_per_note - i) / 800.0;
            }
            
            buffer[i] = (int16_t)(raw_val * envelope * 16384.0);
        }
        
        // Write PCM data to device (play sound)
        snd_pcm_sframes_t frames = snd_pcm_writei(handle, buffer, samples_per_note);
        if (frames < 0) {
            // Attempt to recover from underrun
            frames = snd_pcm_recover(handle, frames, 0);
        }
        if (frames < 0) {
            fprintf(stderr, "snd_pcm_writei failed: %s\n", snd_strerror(frames));
            break;
        }
    }

    // Clean up
    free(buffer);
    snd_pcm_drain(handle);
    snd_pcm_close(handle);
    
    printf("\nPlayback completed successfully.\n");
    printf("=============================================================\n");
    return 0;
}
