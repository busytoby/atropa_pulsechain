#ifndef TSFI_DRUM_LATENCY_H
#define TSFI_DRUM_LATENCY_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    double input_trigger_latency_us; // WinchesterMQ SCSI Keycode Interrupt Latency (us)
    double synth_render_latency_us;  // Logan Wavetable Resonator Render Latency (us)
    double pcm_buffer_latency_us;   // ALSA / PulseAudio Ring Buffer Latency (us)
    double total_drum_latency_us;   // Total Round-Trip Drum Latency (us)
    uint8_t latency_guard_passed;   // 1 if total latency < 1000 us (Rule 11 Guard Gate)
} tsfi_drum_latency_profile_t;

/* Initialize and profile real-time drum trigger latency across audio pipeline */
int tsfi_drum_latency_profile(
    uint32_t sample_rate_hz,
    uint32_t buffer_frame_size,
    tsfi_drum_latency_profile_t *out_profile
);

#endif // TSFI_DRUM_LATENCY_H
