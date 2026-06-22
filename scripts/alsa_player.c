#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>

int main(int argc, char **argv) {
    snd_pcm_t *pcm_handle = NULL;
    snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;
    unsigned int channels = 1;
    unsigned int sample_rate = 22050;
    const char *device_name = "default";
    
    // Parse sample rate optionally
    if (argc > 1) {
        sample_rate = atoi(argv[1]);
    }
    // Parse device name optionally
    if (argc > 2) {
        device_name = argv[2];
    }
    
    fprintf(stderr, "[ALSA PLAYER] Initializing: device=%s, sample_rate=%u\n", device_name, sample_rate);
    
    // Open PCM device
    int err = snd_pcm_open(&pcm_handle, device_name, SND_PCM_STREAM_PLAYBACK, 0);
    if (err < 0) {
        fprintf(stderr, "[ALSA PLAYER] Error: Cannot open ALSA PCM device %s (%s)\n", device_name, snd_strerror(err));
        return 1;
    }
    
    // Set params
    err = snd_pcm_set_params(pcm_handle, format, SND_PCM_ACCESS_RW_INTERLEAVED,
                             channels, sample_rate, 1, 100000);
    if (err < 0) {
        fprintf(stderr, "[ALSA PLAYER] Error: Cannot set ALSA parameters (%s)\n", snd_strerror(err));
        snd_pcm_close(pcm_handle);
        return 1;
    }
    
    char buffer[2048];
    int bytes_per_frame = 2; // S16_LE mono is 2 bytes/frame
    
    while (1) {
        size_t bytes_read = fread(buffer, 1, sizeof(buffer), stdin);
        if (bytes_read <= 0) {
            break; // EOF or error
        }
        
        snd_pcm_sframes_t frames = bytes_read / bytes_per_frame;
        snd_pcm_sframes_t written = snd_pcm_writei(pcm_handle, buffer, frames);
        if (written < 0) {
            written = snd_pcm_recover(pcm_handle, written, 0);
            if (written < 0) {
                fprintf(stderr, "[ALSA PLAYER] Recover failed: %s\n", snd_strerror(written));
                break;
            }
        }
    }
    
    snd_pcm_drain(pcm_handle);
    snd_pcm_close(pcm_handle);
    return 0;
}
