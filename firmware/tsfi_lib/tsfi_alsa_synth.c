#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <pthread.h>
#include <alsa/asoundlib.h>

#define SAMPLE_RATE 22050
#define PI 3.14159265358979323846

typedef struct {
    snd_pcm_t *pcm_handle;
    double current_frequency;
    bool is_running;
    pthread_t thread;
    pthread_mutex_t mutex;
} ALSASynth;

static void* alsa_synth_thread(void *arg) {
    ALSASynth *synth = (ALSASynth*)arg;
    int16_t buffer[1024];
    double phase = 0.0;
    
    while (true) {
        pthread_mutex_lock(&synth->mutex);
        if (!synth->is_running) {
            pthread_mutex_unlock(&synth->mutex);
            break;
        }
        double freq = synth->current_frequency;
        pthread_mutex_unlock(&synth->mutex);

        // Generate sine wave block
        for (int i = 0; i < 1024; i++) {
            buffer[i] = (int16_t)(32767.0 * sin(phase));
            // Calculate phase step based on current frequency
            phase += (2.0 * PI * freq) / SAMPLE_RATE;
            if (phase >= 2.0 * PI) {
                phase -= 2.0 * PI;
            }
        }

        if (synth->pcm_handle) {
            snd_pcm_sframes_t written = snd_pcm_writei(synth->pcm_handle, buffer, 1024);
            if (written < 0) {
                snd_pcm_recover(synth->pcm_handle, written, 0);
            }
        } else {
            // Emulate device output rate timing if running headlessly without ALSA hardware
            usleep((1024 * 1000000) / SAMPLE_RATE);
        }
    }
    return NULL;
}

bool alsa_synth_init(ALSASynth *synth, double initial_freq) {
    synth->current_frequency = initial_freq;
    synth->is_running = false;
    pthread_mutex_init(&synth->mutex, NULL);
    
    // Open PCM device (falling back to NULL for emulated execution if device is busy/unavailable)
    int err = snd_pcm_open(&synth->pcm_handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
    if (err < 0) {
        fprintf(stderr, "[ALSA SYNTH] Warning: Cannot open sound hardware (%s). Running in emulator mode.\n", snd_strerror(err));
        synth->pcm_handle = NULL;
    } else {
        err = snd_pcm_set_params(synth->pcm_handle, SND_PCM_FORMAT_S16_LE, SND_PCM_ACCESS_RW_INTERLEAVED,
                                 1, SAMPLE_RATE, 1, 100000);
        if (err < 0) {
            fprintf(stderr, "[ALSA SYNTH] Error: Cannot set sound parameters (%s)\n", snd_strerror(err));
            snd_pcm_close(synth->pcm_handle);
            synth->pcm_handle = NULL;
            return false;
        }
    }
    
    synth->is_running = true;
    pthread_create(&synth->thread, NULL, alsa_synth_thread, synth);
    return true;
}

void alsa_synth_set_frequency(ALSASynth *synth, double freq) {
    pthread_mutex_lock(&synth->mutex);
    synth->current_frequency = freq;
    pthread_mutex_unlock(&synth->mutex);
}

void alsa_synth_stop(ALSASynth *synth) {
    pthread_mutex_lock(&synth->mutex);
    synth->is_running = false;
    pthread_mutex_unlock(&synth->mutex);
    
    pthread_join(synth->thread, NULL);
    
    if (synth->pcm_handle) {
        snd_pcm_drain(synth->pcm_handle);
        snd_pcm_close(synth->pcm_handle);
        synth->pcm_handle = NULL;
    }
    pthread_mutex_destroy(&synth->mutex);
}
