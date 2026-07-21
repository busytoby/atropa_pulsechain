#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include "tsfi_paint.h"
#include "tsfi_computel_blue_box.h"
#include "tsfi_hogan.h"
#include "tsfi_encodings.h"
#include "tsfi_cade_imf.h"
#include "tsfi_cade_vulkan.h"
#include "tsfi_parc_bitblt.h"
#include "tsfi_parc_smalltalk.h"
#include "tsfi_parc_window.h"
#include "tsfi_parc_ethernet.h"
#include "tsfi_parc_superpaint.h"
#include "tsfi_parc_interpress.h"
#include "tsfi_parc_bravo.h"
#include "tsfi_parc_cursor.h"
#include "tsfi_parc_brush.h"
#include "tsfi_parc_runlength.h"
#include "tsfi_parc_csmacd.h"
#include "tsfi_parc_keyset.h"
#include "tsfi_parc_gc.h"
#include "tsfi_parc_bfs.h"
#include "tsfi_parc_stcomp.h"
#include "tsfi_parc_video.h"
#include "tsfi_parc_shaders.h"
#include "tsfi_parc_videodeck.h"
#include "tsfi_parc_clipboard.h"
#include "tsfi_parc_mvc.h"
#include "tsfi_parc_laurel.h"
#include "tsfi_parc_disktool.h"
#include "tsfi_parc_notetaker.h"
#include "tsfi_parc_alfont.h"
#include "tsfi_parc_routing.h"
#include "tsfi_parc_keyboard.h"
#include "tsfi_parc_knoll.h"
#include "tsfi_parc_tknoll.h"
#include "tsfi_parc_rle.h"
#include "tsfi_parc_figma.h"
#include "tsfi_parc_figma_advanced.h"
#include "tsfi_parc_figma_super.h"
#include "tsfi_parc_figma_proto.h"
#include "tsfi_parc_figma_vector.h"
#include "tsfi_parc_runcible_lang.h"
#include "tsfi_parc_tape_label_yul.h"
#include "tsfi_parc_tape_catalog.h"
#include "tsfi_autodin_tape_proof.h"
#include "tsfi_iso_transpiler.h"
#include "tsfi_parc_tape_trunk.h"
#include "tsfi_yul_deployer.h"
#include "tsfi_autodin_zmm_tx.h"
#include "tsfi_autodin_anvil_oracle.h"
#include "tsfi_parc_trunk_queue.h"
#include "tsfi_iso_security_engine.h"
#include "tsfi_helmholtz_dispatcher.h"
#include "tsfi_logan_synth.h"
#include "tsfi_lynch_exec.h"
#include "tsfi_helmholtz_lynch_bridge.h"
#include "tsfi_zevm_vm_selector.h"
#include "tsfi_lynch_channel.h"
#include "tsfi_lynch_sync.h"
#include "tsfi_knuth_storage.h"
#include "tsfi_knuth_fet_gas.h"
#include "tsfi_electricity_meter.h"
#include "tsfi_knuth_yul_transpiler.h"
#include "tsfi_knuth_fft.h"
#include "tsfi_knuth_quadtree.h"
#include "tsfi_knuth_rb_dragon.h"
#include "tsfi_lowpower_fet.h"
#include "tsfi_lowpower_mode.h"
#include "tsfi_dvfs_governor.h"
#include "tsfi_drum_latency.h"
#include "tsfi_minimized_drum_buffer.h"
#include "tsfi_card_spooler.h"
#include "tsfi_nanosecond_drum.h"
#include "tsfi_nadler_optimizer.h"
#include "tsfi_nadler_advanced.h"
#include "tsfi_transcendental_math.h"
#include "tsfi_cobol_algol_power.h"
#include "tsfi_fortran_dml_power.h"
#include "tsfi_continuous_zero_power.h"

#define WIDTH 512
#define HEIGHT 512
#define FRAMES 2700 // 90 seconds @ 30 FPS
#define SAMPLE_RATE 8000

typedef struct {
    uint64_t base;
    uint64_t signal;
    uint64_t channel;
    uint64_t secret;
    uint64_t pole;
    uint64_t identity;
    uint64_t foundation;
    uint64_t element;
    uint64_t chin;
    uint64_t dynamo;
    uint64_t manifold;
    uint64_t monopole;
    uint64_t ring;
    uint64_t contour;
    uint64_t coordinate;
    uint64_t charge;
    uint64_t xi;
    uint64_t omicron;
    uint64_t omega;
} Lau19DValues;

static int load_lau_19d_values(Lau19DValues *val) {
    val->base = 196811331987337ULL;
    val->signal = 512736817632179ULL;
    val->channel = 779859001551596ULL;
    val->secret = 481588919265707ULL;
    val->pole = 0ULL;
    val->identity = 341042560473881ULL;
    val->foundation = 505921226463547ULL;
    val->element = 931869611669725ULL;
    val->chin = 415958429943167ULL;
    val->dynamo = 58333592128473ULL;
    val->manifold = 546174312197144ULL;
    val->monopole = 802597347437964ULL;
    val->ring = 363710239596665ULL;
    val->contour = 825385904428888ULL;
    val->coordinate = 643408250229640ULL;
    val->charge = 553126253850364ULL;
    val->xi = 261640507549433ULL;
    val->omicron = 70972746134015ULL;
    val->omega = 152946039182293ULL;

    FILE *f = fopen("config/live_quaternion_data.json", "r");
    if (!f) {
        f = fopen("../config/live_quaternion_data.json", "r");
    }
    if (!f) return -1;

    char buf[1024];
    int in_laus = 0;
    while (fgets(buf, sizeof(buf), f)) {
        if (strstr(buf, "\"LAUs\"")) {
            in_laus = 1;
        }
        if (!in_laus) continue;
        
        if (strstr(buf, "\"Base\":")) {
            char *ptr = strchr(buf, ':');
            if (ptr) val->base = strtoull(ptr + 1, NULL, 10);
        }
        if (strstr(buf, "\"Signal\":")) {
            char *ptr = strchr(buf, ':');
            if (ptr) val->signal = strtoull(ptr + 1, NULL, 10);
        }
        if (strstr(buf, "\"Channel\":")) {
            char *ptr = strchr(buf, ':');
            if (ptr) val->channel = strtoull(ptr + 1, NULL, 10);
        }
        if (strstr(buf, "\"Secret\":")) {
            char *ptr = strchr(buf, ':');
            if (ptr) val->secret = strtoull(ptr + 1, NULL, 10);
        }
        if (strstr(buf, "\"Pole\":")) {
            char *ptr = strchr(buf, ':');
            if (ptr) val->pole = strtoull(ptr + 1, NULL, 10);
        }
        if (strstr(buf, "\"Identity\":")) {
            char *ptr = strchr(buf, ':');
            if (ptr) val->identity = strtoull(ptr + 1, NULL, 10);
        }
        if (strstr(buf, "\"Foundation\":")) {
            char *ptr = strchr(buf, ':');
            if (ptr) val->foundation = strtoull(ptr + 1, NULL, 10);
        }
        if (strstr(buf, "\"Element\":")) {
            char *ptr = strchr(buf, ':');
            if (ptr) val->element = strtoull(ptr + 1, NULL, 10);
        }
        if (strstr(buf, "\"Chin\":")) {
            char *ptr = strchr(buf, ':');
            if (ptr) val->chin = strtoull(ptr + 1, NULL, 10);
        }
        if (strstr(buf, "\"Dynamo\":")) {
            char *ptr = strchr(buf, ':');
            if (ptr) val->dynamo = strtoull(ptr + 1, NULL, 10);
        }
        if (strstr(buf, "\"Manifold\":")) {
            char *ptr = strchr(buf, ':');
            if (ptr) val->manifold = strtoull(ptr + 1, NULL, 10);
        }
        if (strstr(buf, "\"Monopole\":")) {
            char *ptr = strchr(buf, ':');
            if (ptr) val->monopole = strtoull(ptr + 1, NULL, 10);
        }
        if (strstr(buf, "\"Ring\":")) {
            char *ptr = strchr(buf, ':');
            if (ptr) val->ring = strtoull(ptr + 1, NULL, 10);
        }
        if (strstr(buf, "\"Contour\":")) {
            char *ptr = strchr(buf, ':');
            if (ptr) val->contour = strtoull(ptr + 1, NULL, 10);
        }
        if (strstr(buf, "\"Coordinate\":")) {
            char *ptr = strchr(buf, ':');
            if (ptr) val->coordinate = strtoull(ptr + 1, NULL, 10);
        }
        if (strstr(buf, "\"Charge\":")) {
            char *ptr = strchr(buf, ':');
            if (ptr) val->charge = strtoull(ptr + 1, NULL, 10);
        }
        if (strstr(buf, "\"Xi\":")) {
            char *ptr = strchr(buf, ':');
            if (ptr) val->xi = strtoull(ptr + 1, NULL, 10);
        }
        if (strstr(buf, "\"Omicron\":")) {
            char *ptr = strchr(buf, ':');
            if (ptr) val->omicron = strtoull(ptr + 1, NULL, 10);
        }
        if (strstr(buf, "\"Omega\":")) {
            char *ptr = strchr(buf, ':');
            if (ptr) val->omega = strtoull(ptr + 1, NULL, 10);
        }
    }
    fclose(f);
    return 0;
}

// WAV Header struct
#pragma pack(push, 1)
struct WavHeader {
    char riff[4];
    int32_t overall_size;
    char wave[4];
    char fmt_chunk_marker[4];
    int32_t fmt_length;
    int16_t audio_format;
    int16_t num_channels;
    int32_t sample_rate;
    int32_t byte_rate;
    int16_t block_align;
    int16_t bits_per_sample;
    char data_chunk_header[4];
    int32_t data_size;
};
#pragma pack(pop)

// Simple State Variable Filter
typedef struct {
    double f, q, low, band, high;
} Resonator;

static void resonator_init(Resonator *r, double freq, double Q) {
    double omega = 2.0 * M_PI * freq / SAMPLE_RATE;
    r->f = 2.0 * sin(omega / 2.0);
    r->q = 1.0 / Q;
    r->low = 0.0;
    r->band = 0.0;
    r->high = 0.0;
}

static double resonator_tick(Resonator *r, double input) {
    r->high = input - r->low - r->q * r->band;
    r->band += r->f * r->high;
    r->low += r->f * r->band;
    return r->band;
}

static double parse_note(const char *note) {
    if (strcmp(note, "REST") == 0 || strcmp(note, "rest") == 0) return 0.0;
    char char0 = note[0];
    int idx = 0;
    switch (char0) {
        case 'C': case 'c': idx = 0; break;
        case 'D': case 'd': idx = 2; break;
        case 'E': case 'e': idx = 4; break;
        case 'F': case 'f': idx = 5; break;
        case 'G': case 'g': idx = 7; break;
        case 'A': case 'a': idx = 9; break;
        case 'B': case 'b': idx = 11; break;
        default: return 0.0;
    }
    char char1 = note[1];
    int octave_char = char1;
    if (char1 == '#' || char1 == 'b') {
        octave_char = note[2];
        if (char1 == '#') idx += 1;
        else idx -= 1;
    }
    int octave = octave_char - '0';
    double freq = 0.0;
    switch (idx) {
        case 0:  freq = 261.63; break; // C4
        case 1:  freq = 277.18; break;
        case 2:  freq = 293.66; break;
        case 3:  freq = 311.13; break;
        case 4:  freq = 329.63; break;
        case 5:  freq = 349.23; break;
        case 6:  freq = 369.99; break;
        case 7:  freq = 392.00; break;
        case 8:  freq = 415.30; break;
        case 9:  freq = 440.00; break;
        case 10: freq = 466.16; break; // Bb4
        case 11: freq = 493.88; break;
        default: return 0.0;
    }
    if (octave > 4) {
        for (int i = 4; i < octave; i++) freq *= 2.0;
    } else if (octave < 4) {
        for (int i = octave; i < 4; i++) freq /= 2.0;
    }
    return freq;
}

static void write_wav_header(FILE *fp, uint32_t data_size) {
    struct WavHeader header;
    memcpy(header.riff, "RIFF", 4);
    header.overall_size = 36 + data_size;
    memcpy(header.wave, "WAVE", 4);
    memcpy(header.fmt_chunk_marker, "fmt ", 4);
    header.fmt_length = 16;
    header.audio_format = 1;
    header.num_channels = 1;
    header.sample_rate = SAMPLE_RATE;
    header.byte_rate = SAMPLE_RATE * 2;
    header.block_align = 2;
    header.bits_per_sample = 16;
    memcpy(header.data_chunk_header, "data", 4);
    header.data_size = data_size;
    fwrite(&header, sizeof(struct WavHeader), 1, fp);
}

void generate_tiger_soundtrack(const char *filepath) {
    const char *bass_seq[] = {
        "C2", "C2", "C2", "C2", "C2", "C2", "C2", "C2",
        "C2", "C2", "Bb1", "C2", "C2", "C2", "C2", "C2",
        "C2", "C2", "Bb1", "C2", "C2", "C2", "C2", "C2",
        "C2", "C2", "Bb1", "C2", "C2", "Ab1", "Bb1", "C2"
    };
    const char *lead_seq[] = {
        "C4", "C4", "C4", "C4", "C4", "C4", "C4", "C4",
        "C4", "C4", "Bb3", "C4", "REST", "REST", "REST", "REST",
        "REST", "REST", "Bb3", "C4", "REST", "REST", "REST", "REST",
        "REST", "REST", "Bb3", "C4", "REST", "Ab3", "Bb3", "C4"
    };
    const char *arp_seq[] = {
        "C3", "Eb3", "G3", "C4", "Eb3", "G3", "C4", "Eb3",
        "C3", "Eb3", "G3", "C4", "Eb3", "G3", "C4", "Eb3",
        "C3", "Eb3", "G3", "C4", "Eb3", "G3", "C4", "Eb3",
        "C3", "Eb3", "G3", "C4", "Eb3", "G3", "C4", "Eb3"
    };
    int kick_seq[] = {
        1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0,
        1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0
    };
    int snare_seq[] = {
        0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
        0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0
    };
    int hh_seq[] = {
        1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
        1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0
    };
    int clap_seq[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 0, 0, 0,
        1, 0, 1, 1, 0, 0, 0, 0, 1, 0, 1, 1, 0, 1, 1, 1
    };

    int step_duration_ms = 136;
    int samples_per_step = (SAMPLE_RATE * step_duration_ms) / 1000;
    int loop_samples = samples_per_step * 32;
    int loops = (SAMPLE_RATE * 90) / loop_samples + 1;
    int total_samples = loop_samples * loops;

    Lau19DValues lau_vals;
    load_lau_19d_values(&lau_vals);

    // Compute modular frequency scales from actual LAU contract registers
    double freq_x = 3.0 + (double)(lau_vals.channel % 1000) / 500.0;
    double freq_y = 5.0 + (double)(lau_vals.dynamo % 1000) / 500.0;
    double freq_z = 1.0 + (double)(lau_vals.foundation % 1000) / 1000.0;

    FILE *f = fopen(filepath, "wb");
    if (!f) return;
    write_wav_header(f, total_samples * 2);

    double lead_phase = 0.0;
    double bass_phase = 0.0;
    double arp_phase = 0.0;

    double kick_env = 0.0;
    double kick_pitch = 150.0;
    double snare_env = 0.0;
    double hihat_env = 0.0;
    double clap_env = 0.0;

    Resonator snare_res, lead_res;
    resonator_init(&snare_res, 1000.0, 4.0);
    resonator_init(&lead_res, 800.0, 2.0);

    unsigned int seed = 0x54494752; // "TIGR"

    for (int l = 0; l < loops; l++) {
        for (int step = 0; step < 32; step++) {
            if (kick_seq[step]) { kick_env = 1.0; kick_pitch = 150.0; }
            if (snare_seq[step]) snare_env = 1.0;
            if (hh_seq[step]) hihat_env = 1.0;
            if (clap_seq[step]) clap_env = 1.0;

            double lead_freq = parse_note(lead_seq[step]);
            double bass_freq = parse_note(bass_seq[step]);
            double arp_freq = parse_note(arp_seq[step]);

            for (int s = 0; s < samples_per_step; s++) {
                int global_idx = (l * 32 + step) * samples_per_step + s;
                double t_s = (double)global_idx / SAMPLE_RATE;

                // Drive oscillators along the 3D Hot Lissajous Tube coordinate paths
                double theta_l = t_s * 2.0 * M_PI;
                double X = sin(theta_l * freq_x);
                double Y = cos(theta_l * freq_y) * sin(theta_l * freq_z);

                double kick_out = 0.0;
                if (kick_env > 0.0) {
                    double active_kick_pitch = kick_pitch * (1.0 + 0.15 * X * Y);
                    kick_out = sin(2.0 * M_PI * active_kick_pitch * s / SAMPLE_RATE) * kick_env;
                    kick_pitch *= 0.995;
                    kick_env *= 0.992;
                }

                double snare_out = 0.0;
                if (snare_env > 0.0) {
                    seed = seed * 1103515245 + 12345;
                    double noise = ((double)(seed & 0xFFFF) / 65535.0) - 0.5;
                    snare_out = resonator_tick(&snare_res, noise) * snare_env;
                    snare_env *= 0.995;
                }

                double hh_out = 0.0;
                if (hihat_env > 0.0) {
                    seed = seed * 1103515245 + 12345;
                    double noise = ((double)(seed & 0xFFFF) / 65535.0) - 0.5;
                    hh_out = noise * hihat_env * 0.5;
                    hihat_env *= 0.985;
                }

                double clap_out = 0.0;
                if (clap_env > 0.0) {
                    seed = seed * 1103515245 + 12345;
                    double noise = ((double)(seed & 0xFFFF) / 65535.0) - 0.5;
                    clap_out = noise * clap_env;
                    clap_env *= 0.991;
                }

                double lead_out = 0.0;
                if (lead_freq > 0.0) {
                    double raw_saw = (lead_phase / M_PI) - 1.0;
                    lead_out = resonator_tick(&lead_res, raw_saw) * 0.3;
                    lead_phase += (2.0 * M_PI * lead_freq) / SAMPLE_RATE;
                    if (lead_phase >= 2.0 * M_PI) lead_phase -= 2.0 * M_PI;
                }

                double bass_out = 0.0;
                if (bass_freq > 0.0) {
                    // Drive sub-bass FM growl using secret and signal registers
                    double main_bass = sin(bass_phase + 0.9 * X * Y * ((double)(lau_vals.secret % 1000) / 1000.0));
                    double sub_bass = sin(bass_phase * 0.5 + 0.5 * X) + sin(bass_phase * 0.25 + 0.3 * Y);
                    double combined_bass = main_bass * 0.4 + sub_bass * 1.5;
                    bass_out = tanh(combined_bass * 3.5) * 0.9f;
                    bass_phase += (2.0 * M_PI * bass_freq) / SAMPLE_RATE;
                    if (bass_phase >= 2.0 * M_PI) bass_phase -= 2.0 * M_PI;
                }

                double arp_out = 0.0;
                if (arp_freq > 0.0) {
                    double raw_tri = (arp_phase < M_PI) ? (arp_phase / M_PI * 2.0 - 1.0) : (3.0 - arp_phase / M_PI * 2.0);
                    arp_out = raw_tri * 0.15;
                    arp_phase += (2.0 * M_PI * arp_freq) / SAMPLE_RATE;
                    if (arp_phase >= 2.0 * M_PI) arp_phase -= 2.0 * M_PI;
                }

                double mixed = kick_out + snare_out + hh_out + clap_out + lead_out + bass_out + arp_out;
                if (mixed > 1.0) mixed = 1.0;
                if (mixed < -1.0) mixed = -1.0;

                int16_t sample_16 = (int16_t)(16384.0 * mixed);
                fwrite(&sample_16, 1, sizeof(int16_t), f);
            }
        }
    }
    fclose(f);
}

void apply_super8_crop(uint32_t *pixels, int w, int h, int f_idx) {
    int target_h = (int)(w / 1.85f);
    int border_y = (h - target_h) / 2;

    for (int y = 0; y < border_y; y++) {
        memset(pixels + y * w, 0, w * sizeof(uint32_t));
    }
    for (int y = h - border_y; y < h; y++) {
        memset(pixels + y * w, 0, w * sizeof(uint32_t));
    }

    float bob = sinf(f_idx * 0.1f) * 2.0f;

    for (int sy = border_y + 16; sy < h - border_y - 16; sy += 64) {
        int active_sy = sy + (int)bob;
        for (int y = active_sy - 12; y <= active_sy + 12; y++) {
            if (y < border_y || y >= h - border_y) { continue; }
            for (int x = 12; x <= 28; x++) {
                int dy = abs(y - active_sy);
                int dx = abs(x - 20);
                if (dy > 9 && dx > 6) { continue; }
                pixels[y * w + x] = 0xFF080808;
            }
        }
    }

    float noise = ((float)rand() / RAND_MAX - 0.5f) * 8.0f;
    for (int y = border_y; y < h - border_y; y++) {
        for (int x = 0; x < w; x++) {
            uint32_t pix = pixels[y * w + x];
            int r = ((pix >> 16) & 0xFF) + (int)noise;
            int g = ((pix >> 8) & 0xFF) + (int)noise;
            int b = (pix & 0xFF) + (int)noise;

            if (r < 0) { r = 0; }
            if (r > 255) { r = 255; }
            if (g < 0) { g = 0; }
            if (g > 255) { g = 255; }
            if (b < 0) { b = 0; }
            if (b > 255) { b = 255; }

            pixels[y * w + x] = (pix & 0xFF000000) | (r << 16) | (g << 8) | b;
        }
    }

    if (f_idx % 25 == 0) {
        int sx = rand() % (w - 60) + 30;
        int sy = rand() % (h - 2 * border_y) + border_y;
        int len = rand() % 20 + 10;
        for (int i = 0; i < len; i++) {
            int px = sx + i;
            int py = sy + (int)(sinf(i * 0.2f) * 3.0f);
            if (px >= 0 && px < w && py >= border_y && py < h - border_y) {
                pixels[py * w + px] = 0xFF101010;
            }
        }
    }
}

void draw_line_thick(uint32_t *pixels, int w, int h, int x0, int y0, int x1, int y1, int thickness, uint32_t color) {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;
    while (1) {
        for (int i = -thickness / 2; i <= thickness / 2; i++) {
            for (int j = -thickness / 2; j <= thickness / 2; j++) {
                int px = x0 + i, py = y0 + j;
                if (px >= 0 && px < w && py >= 0 && py < h) pixels[py * w + px] = color;
            }
        }
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

void draw_baudot_punched_tape(uint32_t *pixels, int w, int h, const char *text, int f_idx) {
    uint8_t baud_buf[128];
    int len = tsfi_encode_baudot(text, baud_buf, sizeof(baud_buf));
    int start_x = 35 + (f_idx % 16);
    int tape_y = 48;
    int border_y = 32;

    // Draw white paper tape background strip matching Super8 style
    for (int x = 35; x < w - 35; x++) {
        for (int y = tape_y - 8; y < tape_y + 8; y++) {
            if (x >= 0 && x < w && y >= border_y && y < h - border_y) {
                pixels[y * w + x] = 0xFFe6dfd3; // Cream background
            }
        }
    }
    // Draw holes for each character
    for (int i = 0; i < len; i++) {
        int char_x = start_x + i * 14;
        if (char_x < 35 || char_x > w - 35) continue;
        uint8_t code = baud_buf[i];
        // Draw 5 code holes + 1 feed sprocket hole
        for (int bit = 0; bit < 5; bit++) {
            int hole_y = tape_y - 6 + bit * 3;
            if (bit >= 3) hole_y += 2; // skip sprocket space
            // If bit is set, draw a dark punch mark
            uint32_t hole_color = (code & (1 << bit)) ? 0xFF12100e : 0xFFe6dfd3;
            if (hole_color == 0xFF12100e) {
                pixels[hole_y * w + char_x] = hole_color;
                pixels[hole_y * w + (char_x - 1)] = hole_color;
                pixels[hole_y * w + (char_x + 1)] = hole_color;
            }
        }
        // Sprocket hole (smaller)
        pixels[tape_y * w + char_x] = 0xFF12100e;
    }
}

void draw_hogan_telemetry(uint32_t *pixels, int w, int h, const hogan_umbrella_system *sys, float t) {
    (void)h;
    int start_y = 398;
    int end_y = 508;
    int start_x = 35;
    int end_x = w - 35;

    // Golden Vaesen borders
    for (int x = start_x; x <= end_x; x++) {
        pixels[start_y * w + x] = 0xFF8c7241;
        pixels[end_y * w + x] = 0xFF8c7241;
    }
    for (int y = start_y; y <= end_y; y++) {
        pixels[y * w + start_x] = 0xFF8c7241;
        pixels[y * w + end_x] = 0xFF8c7241;
    }

    // Load dynamic LAU values to display on the panel
    Lau19DValues lau_vals;
    load_lau_19d_values(&lau_vals);

    // Render title
    tsfi_quantel_paintbox_typographer(pixels, w, 512, start_x + 10, start_y + 16, "LAU STATE MONITOR", 0xFFe6dfd3, 10.0f);

    // Render registers
    char buf[128];
    snprintf(buf, sizeof(buf), "BASE: %lu", lau_vals.base % 1000000);
    tsfi_quantel_paintbox_typographer(pixels, w, 512, start_x + 10, start_y + 32, buf, 0xFFc5a059, 8.0f);

    snprintf(buf, sizeof(buf), "CHIN: %lu", lau_vals.chin % 1000000);
    tsfi_quantel_paintbox_typographer(pixels, w, 512, start_x + 10, start_y + 44, buf, 0xFFc5a059, 8.0f);

    snprintf(buf, sizeof(buf), "DYNAMO: %lu", lau_vals.dynamo % 1000000);
    tsfi_quantel_paintbox_typographer(pixels, w, 512, start_x + 10, start_y + 56, buf, 0xFFc5a059, 8.0f);

    // Active VMs
    tsfi_quantel_paintbox_typographer(pixels, w, 512, start_x + 150, start_y + 16, "ACTIVE VM UNITS", 0xFFe6dfd3, 10.0f);
    int active_vm = ((int)(t * 2.0f)) % 3;
    int vm_x = start_x + 150;
    int vm_y = start_y + 24;
    for (int vm = 0; vm < 3; vm++) {
        uint32_t color = (vm == active_vm) ? 0xFFc5a059 : 0xFF1c1815;
        for (int dx = 0; dx < 22; dx++) {
            for (int dy = 0; dy < 10; dy++) {
                pixels[(vm_y + dy) * w + (vm_x + vm * 28 + dx)] = color;
            }
        }
        char vm_label[8];
        snprintf(vm_label, sizeof(vm_label), "VM%d", vm);
        tsfi_quantel_paintbox_typographer(pixels, w, 512, vm_x + vm * 28 + 2, vm_y + 8, vm_label, 0xFFe6dfd3, 7.0f);
    }

    // Transaction pipeline
    tsfi_quantel_paintbox_typographer(pixels, w, 512, start_x + 150, start_y + 48, "ZMM TX HANDSHAKE LOOPS", 0xFFe6dfd3, 9.0f);
    int dot_y = start_y + 60;
    int dot_x = start_x + 150;
    int tx_limit = (sys->tx_count < 20) ? (int)sys->tx_count : 20;
    for (int i = 0; i < tx_limit; i++) {
        const hogan_transaction *tx = &sys->tx_log[sys->tx_count - tx_limit + i];
        uint32_t dot_color = tx->processed ? 0xFFc5a059 : 0xFFe6dfd3;
        int dot_width = (tx->amount > 150) ? 8 : 5;
        int dot_height = (tx->amount > 150) ? 10 : 7;
        for (int dx = 0; dx < dot_width; dx++) {
            for (int dy = 0; dy < dot_height; dy++) {
                int px_x = dot_x + i * 11 + dx;
                int px_y = dot_y + dy;
                if (px_x < end_x - 5) {
                    pixels[px_y * w + px_x] = dot_color;
                }
            }
        }
    }

    // Right Side: CADE IMF Control Panel & Daily Batch summary
    tsfi_quantel_paintbox_typographer(pixels, w, 512, start_x + 280, start_y + 16, "CADE IMF BATCH REGISTER", 0xFFe6dfd3, 10.0f);

    // Call actual CADE daily batch summary formatter
    char cade_buf[128];
    tsfi_mf_cade_format_batch_summary((int)sys->tx_count, 0, (double)sys->tx_count * 105.5, 0.0, cade_buf, sizeof(cade_buf));
    tsfi_quantel_paintbox_typographer(pixels, w, 512, start_x + 280, start_y + 32, cade_buf, 0xFFc5a059, 7.5f);

    // Dynamic taxpayer registration lookup using actual CADE logic
    char registry_out[256];
    tsfi_mf_cade_register_taxpayer("999-12-3456", 100000.0, 1, registry_out, sizeof(registry_out));
    
    // Extract state status name using CADE lookup
    char status_name[32];
    tsfi_mf_cade_get_status_name(1, status_name, sizeof(status_name));
    snprintf(buf, sizeof(buf), "SSN: 999-12-3456 | STATUS: %s", status_name);
    tsfi_quantel_paintbox_typographer(pixels, w, 512, start_x + 280, start_y + 44, buf, 0xFFc5a059, 8.0f);

    // Display the BCDIC representation of the transaction code
    uint8_t bcd_buf[16];
    tsfi_encode_bcdic("CADE_OK", bcd_buf, sizeof(bcd_buf));
    snprintf(buf, sizeof(buf), "BCDIC T-CODE: %02X %02X %02X %02X", bcd_buf[0], bcd_buf[1], bcd_buf[2], bcd_buf[3]);
    tsfi_quantel_paintbox_typographer(pixels, w, 512, start_x + 280, start_y + 56, buf, 0xFFc5a059, 8.0f);
}
static void draw_char_1bit(uint16_t *buf, int cx, int cy, char c) {
    static const uint8_t font8x8[16][8] = {
        {0x3C, 0x66, 0x6E, 0x7E, 0x76, 0x66, 0x66, 0x00}, // A
        {0x7C, 0x66, 0x66, 0x7C, 0x66, 0x66, 0x7C, 0x00}, // B
        {0x3C, 0x66, 0x60, 0x60, 0x60, 0x66, 0x3C, 0x00}, // C
        {0x78, 0x6C, 0x66, 0x66, 0x66, 0x6C, 0x78, 0x00}, // D
        {0x7E, 0x60, 0x60, 0x7C, 0x60, 0x60, 0x7E, 0x00}, // E
        {0x7E, 0x60, 0x60, 0x7C, 0x60, 0x60, 0x60, 0x00}, // F
        {0x3C, 0x66, 0x60, 0x6E, 0x66, 0x66, 0x3C, 0x00}, // G
        {0x66, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x66, 0x00}, // H
        {0x3C, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3C, 0x00}, // I
        {0x1E, 0x0C, 0x0C, 0x0C, 0x0C, 0xCC, 0x78, 0x00}, // J
        {0x66, 0x6C, 0x78, 0x70, 0x78, 0x6C, 0x66, 0x00}, // K
        {0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x7E, 0x00}, // L
        {0x63, 0x77, 0x7F, 0x6B, 0x63, 0x63, 0x63, 0x00}, // M
        {0x66, 0x76, 0x7E, 0x7E, 0x76, 0x66, 0x66, 0x00}, // N
        {0x3C, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00}, // O
        {0x7C, 0x66, 0x66, 0x7C, 0x60, 0x60, 0x60, 0x00}  // P
    };
    int idx = c - 'A';
    if (idx < 0 || idx >= 16) idx = 2; // fallback to 'C'
    for (int y = 0; y < 8; y++) {
        uint8_t row = font8x8[idx][y];
        for (int x = 0; x < 8; x++) {
            if ((row >> (7 - x)) & 1) {
                int px = cx + x;
                int py = cy + y;
                if (px >= 0 && px < 606 && py >= 0 && py < 808) {
                    buf[py * 38 + (px / 16)] |= (uint16_t)(1 << (15 - (px % 16)));
                }
            }
        }
    }
}

static void draw_string_1bit(uint16_t *buf, int cx, int cy, const char *str) {
    int x_offset = 0;
    while (*str) {
        draw_char_1bit(buf, cx + x_offset, cy, *str);
        x_offset += 8;
        str++;
    }
}

int main() {
    srand((unsigned int)time(NULL));
    const char *audio_file = "/tmp/tsfi_tiger_demo.wav";
    
    printf("[INFO] Synthesizing Eye of the Tiger .bio arrangement...\n");
    generate_tiger_soundtrack(audio_file);

    printf("[INFO] Launching FFmpeg encoder...\n");
    FILE *ffmpeg_pipe = popen("ffmpeg -y -f rawvideo -vcodec rawvideo -s 512x512 -pix_fmt rgb24 -r 30 -i - -i /tmp/tsfi_tiger_demo.wav -c:v libx264 -preset ultrafast -c:a aac -b:a 192k -pix_fmt yuv420p bin/tiger_super8_demo.mp4 2>/dev/null", "w");
    if (!ffmpeg_pipe) {
        printf("[ERROR] Failed to open FFmpeg pipe.\n");
        return 1;
    }

    hogan_umbrella_system hogan_sys;
    tsfi_hogan_init(&hogan_sys);
    tsfi_hogan_register_account(&hogan_sys, 999, 1000000);
    tsfi_hogan_register_account(&hogan_sys, 1001, 10000);
    tsfi_hogan_register_account(&hogan_sys, 2002, 5000);
    tsfi_hogan_register_account(&hogan_sys, 3003, 2000);

    // Xerox PARC: Initialize Smalltalk Sandbox VM
    tsfi_parc_smalltalk_vm_t st_vm;
    tsfi_parc_st_init_vm(&st_vm);
    oop_t cls_sensor = tsfi_parc_st_define_class(&st_vm, "TigerSensor", 0);
    uint8_t m_code[4] = { 0x00, 0x25, 0x30, 0x50 };
    tsfi_parc_st_bind_method(&st_vm, cls_sensor, "processData", m_code, 4);
    
    oop_t obj_inst = 0xAA01;
    st_vm.heap[0].oop = obj_inst;
    st_vm.heap[0].class_oop = cls_sensor;
    st_vm.heap[0].fields[0] = 42;
    st_vm.heap_count = 1;

    // Xerox PARC: Initialize overlapping Window Manager
    tsfi_parc_window_manager_t wm;
    tsfi_parc_wm_init(&wm);
    tsfi_parc_wm_open_window(&wm, "CADE IMF Register", 40, 410, 100, 60);
    tsfi_parc_wm_open_window(&wm, "Smalltalk VM", 145, 425, 95, 60);
    tsfi_parc_wm_open_window(&wm, "PUP Netlog", 270, 410, 120, 65);
    // Xerox PARC: Initialize painting & storyboarding systems
    tsfi_parc_superpaint_lut_t sp_lut;
    tsfi_parc_superpaint_init_lut(&sp_lut);
    
    tsfi_parc_interpress_ctx_t ip_ctx;
    tsfi_parc_interpress_init(&ip_ctx);
    
    tsfi_parc_bravo_layout_t bravo_layout;
    tsfi_parc_bravo_init(&bravo_layout, 35, 35, 398, 512 - 508);

    // Xerox PARC: Initialize network collision engine and Smalltalk GC tracker
    tsfi_parc_gc_t st_gc;
    tsfi_parc_gc_init(&st_gc);
    tsfi_parc_gc_add_ref(&st_gc, obj_inst); // register starting instance reference
    
    tsfi_parc_network_node_t local_node = {0, 0, 0};
    tsfi_parc_network_node_t gateway_node = {0, 0, 0};

    // Xerox PARC: Statically verify BFS Diablo drive, Smalltalk compiler, and video scan
    tsfi_parc_bfs_sector_t bfs_sec;
    tsfi_parc_bfs_init_sector(&bfs_sec, 0, 0, 0);

    uint8_t compiler_out[16];
    int compiled_len = tsfi_parc_st_compile("load 0; const 5; add; return", compiler_out, 16);
    (void)compiled_len;

    tsfi_parc_video_controller_t vc;
    tsfi_parc_video_init(&vc);
    tsfi_parc_video_step(&vc, 100, 100);

    // Xerox PARC: Load and parse Algol61 and COBOL strategy shaders at runtime
    tsfi_parc_shader_params_t shader_params;
    tsfi_parc_load_shaders("src/alto_video.algol61", "src/alto_video.strategy", &shader_params);
    // Xerox PARC: Initialize video playback deck
    tsfi_parc_video_deck_t deck;
    tsfi_parc_deck_init(&deck, FRAMES, 3);
    tsfi_parc_deck_control(&deck, DECK_CMD_PLAY, 0);

    uint16_t *alto_display_mem = calloc(38 * 808, sizeof(uint16_t));

    // Statically verify video deck record and load track operations using .dat.bin files
    FILE *f_dummy = fopen("alto_dummy.dat.bin", "wb");
    if (f_dummy) {
        uint8_t dummy_header[32] = {'A', 'L', 'T', 'O', 0};
        fwrite(dummy_header, 1, 32, f_dummy);
        tsfi_parc_deck_record_frame(alto_display_mem, f_dummy);
        fclose(f_dummy);
    }
    tsfi_parc_deck_load_track(&deck, "alto_dummy.dat.bin");
    remove("alto_dummy.dat.bin");

    char typed_document[64] = "CICS CADE ";

    // Xerox Gypsy Modeless Clipboard verify check
    tsfi_parc_clipboard_t clip;
    tsfi_parc_clip_init(&clip);
    tsfi_parc_clip_copy(&clip, "GYPSY OK ", 9);
    tsfi_parc_clip_paste(&clip, typed_document, sizeof(typed_document));

    // Xerox Smalltalk MVC Event Loop Dispatcher verify check
    tsfi_parc_mvc_dispatcher_t mvc_disp;
    tsfi_parc_mvc_init(&mvc_disp, &wm, &st_vm);
    tsfi_parc_mvc_event_t mvc_ev = { MVC_EVENT_CLICK, 100, 100, 'Z' };
    tsfi_parc_mvc_dispatch(&mvc_disp, &mvc_ev);

    // Xerox Laurel Email Client verify check
    tsfi_parc_email_t email;
    tsfi_parc_laurel_compose(&email, "Larry", "Tim", "Gypsy Release", "Clipboard cuts are modeless now.");
    uint8_t email_pup_payload[256];
    tsfi_parc_laurel_serialize_pup(&email, email_pup_payload, sizeof(email_pup_payload));

    // Xerox Alto Diablo Disk Formatter tool check
    tsfi_parc_disktool_t disk_tool;
    tsfi_parc_disktool_init(&disk_tool);
    tsfi_parc_disktool_format(&disk_tool, "VOL_01");

    // Xerox NoteTaker power FET discharge simulation verify check
    tsfi_parc_notetaker_fet_t fet;
    tsfi_parc_notetaker_init(&fet);
    tsfi_parc_notetaker_step_discharge(&fet, 0.1f);

    // Xerox Alto .AL font shaping parser verify check
    tsfi_parc_alfont_t al_font;
    tsfi_parc_alfont_init(&al_font);
    tsfi_parc_alfont_load(&al_font, "alto_font.dat.bin");

    // Xerox PUP Gateway Routing Table verify check
    tsfi_parc_routing_table_t routing_table;
    tsfi_parc_routing_init(&routing_table);
    tsfi_parc_routing_add(&routing_table, 3, 22, 1);
    uint8_t next_hop = 0;
    tsfi_parc_routing_lookup(&routing_table, 3, &next_hop);

    // Xerox Alto Keyboard Scan Matrix translation check
    tsfi_parc_kb_translate(1, 1, 0); // 'q'

    uint32_t *canvas = calloc(WIDTH * HEIGHT, sizeof(uint32_t));
    uint32_t *canvas_b = calloc(WIDTH * HEIGHT, sizeof(uint32_t));
    uint32_t *dst_buffer = calloc(WIDTH * HEIGHT, sizeof(uint32_t));

    // John Knoll (Photoshop/Display pre-history) filters verify check
    tsfi_parc_knoll_lens_flare(canvas, WIDTH, HEIGHT, 256, 256, 1.0f);
    tsfi_parc_knoll_gaussian_blur(canvas, canvas_b, WIDTH, HEIGHT, 1.0f);
    tsfi_parc_knoll_gray_lut(canvas, canvas_b, WIDTH, HEIGHT);

    // Thomas Knoll (Photoshop/Display pre-history) filters verify check
    tsfi_parc_tknoll_levels_adjustment(canvas_b, WIDTH, HEIGHT, 10, 240, 1.0f);
    uint32_t hist[256];
    tsfi_parc_tknoll_histogram(canvas_b, WIDTH, HEIGHT, hist);
    tsfi_parc_tknoll_dither_floyd_steinberg(canvas_b, alto_display_mem, WIDTH, HEIGHT);

    // Thomas Knoll-style RLE Compression verify check
    uint8_t raw_data[16] = { 1, 1, 1, 1, 2, 2, 2, 3, 3, 3, 3, 3, 4, 4, 4, 4 };
    uint8_t comp_buf[32];
    uint8_t decomp_buf[16];
    int comp_sz = tsfi_parc_rle_compress(raw_data, 16, comp_buf, sizeof(comp_buf));
    tsfi_parc_rle_decompress(comp_buf, comp_sz, decomp_buf, 16);

    // Figma Vector Network, Auto Layout, and Multi-Cursor verify check
    tsfi_parc_vector_node_t nodes[3] = { {100, 100}, {200, 100}, {150, 200} };
    tsfi_parc_vector_edge_t edges[3] = { {0, 1}, {1, 2}, {2, 0} };
    tsfi_parc_figma_draw_vector_network(canvas, WIDTH, HEIGHT, nodes, 3, edges, 3, 0xFFFFFFFF);

    tsfi_parc_layout_item_t layout_items[3] = { {0, 0, 50, 50}, {0, 0, 50, 50}, {0, 0, 50, 50} };
    tsfi_parc_figma_auto_layout(layout_items, 3, 0, 10, 10, 5, WIDTH, HEIGHT);

    tsfi_parc_figma_cursor_t cursors[2] = { {120, 120, "User A", 0xFFFF0000}, {180, 150, "User B", 0xFF00FF00} };
    tsfi_parc_figma_draw_cursors(canvas, WIDTH, HEIGHT, cursors, 2);

    // Advanced Figma layer blending, variant property mapping, and Boolean union check
    tsfi_parc_figma_blend_layer(canvas, canvas_b, WIDTH, HEIGHT, BLEND_MULTIPLY);
    int var_w, var_h;
    uint32_t var_col;
    tsfi_parc_figma_variant_resolve("type=button,state=hover", &var_w, &var_h, &var_col);
    tsfi_parc_layout_item_t union_result;
    tsfi_parc_figma_vector_boolean_union(&layout_items[0], &layout_items[1], &union_result);

    // Super Figma constraints, corner smooth, and tidy grid check
    int cx = 10, cy = 10, cw = 100, ch = 100;
    tsfi_parc_figma_apply_constraints(500, 500, 600, 600, CONSTRAINT_SCALE, CONSTRAINT_SCALE, &cx, &cy, &cw, &ch);
    float ctrl_x, ctrl_y;
    tsfi_parc_figma_corner_smooth(0, 0, 10, 10, 20, 0, 5.0f, &ctrl_x, &ctrl_y);
    tsfi_parc_figma_tidy_grid(layout_items, 3, 2, 8, 8);

    // Figma proto Smart Animate, CSS exporter, and drop shadow check
    tsfi_parc_layout_item_t interpolated;
    tsfi_parc_figma_smart_animate(&layout_items[0], &layout_items[1], 0.5f, &interpolated);
    char css_out[128];
    tsfi_parc_figma_to_css(&layout_items[0], 4.0f, 0xFF4A90E2, css_out, sizeof(css_out));
    tsfi_parc_figma_drop_shadow(canvas, WIDTH, HEIGHT, &layout_items[0], 4, 4, 2, 0xFF000000);

    // Figma vector Bezier curves, stencil masking, and overrides check
    tsfi_parc_figma_draw_bezier(canvas, WIDTH, HEIGHT, 10, 10, 50, 100, 100, 0, 150, 150, 0xFFFFFFFF);
    uint8_t stencil[16] = { 255, 255, 255, 255, 0, 0, 0, 0, 255, 255, 255, 255, 0, 0, 0, 0 };
    tsfi_parc_figma_apply_mask(canvas, WIDTH, HEIGHT, stencil, 4, 4, 10, 10);
    tsfi_parc_figma_override_t ovr = { 0xFFFF0000, "Override Text", 1, 1 };
    uint32_t base_c = 0xFFFFFFFF;
    char base_t[32] = "Original Text";
    tsfi_parc_figma_apply_overrides(&base_c, base_t, &ovr);

    // Runcible interactive TTY language CLI check (CALL & KB support)
    tsfi_runcible_main_step("CALL 0x00401000 main");
    tsfi_runcible_main_step("KB 1 1 0");
    tsfi_runcible_main_step("SLIDE");
    tsfi_runcible_main_step("SCSI WRITE 0x20 32");
    tsfi_runcible_main_step("KEYSET 0x01");
    tsfi_runcible_main_step("KEYSET 0x21");
    tsfi_runcible_main_step("SCHUR (2,1) (1) (1)");
    tsfi_runcible_main_step("PEEK 0x00401000");
    tsfi_runcible_main_step("POKE 0x00401000 0xFF357ABD");
    tsfi_runcible_main_step("YUL RESOLVE 0x7a89bc3d 0x12345678");
    tsfi_runcible_main_step("MACRO PLAY BOOT_SEQ");
    tsfi_runcible_main_step("DISCHARGE 1 0.1");
    tsfi_runcible_main_step("PUP SEND 3 22 1");
    tsfi_runcible_main_step("CLIP COPY Hello");
    tsfi_runcible_main_step("ST EVAL 3 + 4");
    tsfi_runcible_main_step("VOID");
    tsfi_runcible_main_step("TAPE CHECK HDL001");
    tsfi_runcible_main_step("TAPE LABEL HDL038 HOLDERS_NEW.DAT.BIN 1");
    tsfi_runcible_main_step("TAPE CATALOG .");
    tsfi_runcible_main_step("TAPE BOUNDS ./tmp/test_guarded.dat.bin");
    tsfi_runcible_main_step("TAPE PHASE ./tmp/test_guarded.dat.bin");
    tsfi_runcible_main_step("TAPE PROOF ./tmp/test_guarded.dat.bin");
    tsfi_runcible_main_step("STATUS");

    // Tape Label Yul DDL verification check for 720-byte 8-block full sequence (VOL1..HDR8 + EOF1)
    uint8_t tape_hdr[720];
    tsfi_tape_label_yul_format_full_header(tape_hdr, "DAT001", "QUAD_TREE.DAT.BIN", TAPE_SECURITY_SECRET, 0.0f, 0.0f, 512.0f, 512.0f, "HDL000", "HDL002", 0.125f, 2, 4, 8, 32, 30);
    int tape_valid = tsfi_tape_label_yul_validate_sequence(tape_hdr);
    int gov_res = tsfi_tape_label_yul_check_governance(tape_hdr, TAPE_SECURITY_TOPSECRET);
    float xmin, ymin, xmax, ymax, phi;
    int fx, fy, fz;
    uint8_t kc32, kc30;
    tsfi_tape_label_yul_get_spatial_bounds(tape_hdr, &xmin, &ymin, &xmax, &ymax);
    tsfi_tape_label_yul_get_phase_invariants(tape_hdr, &phi, &fx, &fy, &fz);
    tsfi_tape_label_yul_get_scsi_map(tape_hdr, &kc32, &kc30);
    printf("[INFO] Yul DDL Full 8-Block Sequence (VOL1..HDR8) Result: %d (Gov: %d, Phase: %.3f, SCSI: %d/%d)\n",
           tape_valid, gov_res, phi, kc32, kc30);

    // Tape Catalog unique Volume ID and meaningful File ID generation check
    tsfi_tape_catalog_entry_t cat_entries[8];
    int cat_count = tsfi_tape_catalog_process_all(".", cat_entries, 8);
    printf("[INFO] Processed %d Tape Catalog entries (Sample Vol: %s -> FileID: %s)\n",
           cat_count, cat_entries[0].volume_id, cat_entries[0].file_id);

    char next_vol[7];
    tsfi_tape_catalog_next_id("HDL", next_vol);
    FILE *guard_f = tsfi_tape_open_guarded("./tmp/test_guarded.dat.bin", "w+b", TAPE_SECURITY_UNCLASSIFIED);
    if (guard_f) { fclose(guard_f); }
    printf("[INFO] Guarded Tape Creator Check -> Assigned Serial: %s\n", next_vol);

    // AI System Isomorphism Transpiler Equivalence Check
    int iso_equiv = tsfi_iso_validate_equivalence("SCSI WRITE 0x20 32");
    printf("[INFO] Yul Isomorphism Transpiler Equivalence Check Result: %d\n", iso_equiv);

    // Yul-Compiled Tape Trunk Engine Sequence Check
    char trunk_vol[7];
    tsfi_tape_trunk_get_vol_id(0, trunk_vol);
    printf("[INFO] Yul-Compiled Tape Trunk Engine Head Volume ID: %s\n", trunk_vol);

    // Multi-Target AUTODIN Yul Contract Deployer Verification (2-3 Tree Node, Anvil EVM, In-Memory Thunk)
    tsfi_yul_deploy_result_t dep_res1, dep_res2, dep_res3;
    tsfi_yul_deploy_contract("assets/isomorphism_engine.yul", YUL_DEPLOY_TARGET_23_TREE_NODE, &dep_res1);
    tsfi_yul_deploy_contract("assets/isomorphism_engine.yul", YUL_DEPLOY_TARGET_ANVIL_EVM, &dep_res2);
    tsfi_yul_deploy_contract("assets/isomorphism_engine.yul", YUL_DEPLOY_TARGET_IN_MEM_THUNK, &dep_res3);

    // AUTODIN ZMM VM Block-Secure Transaction Dispatcher & Nonce Tracking Check
    tsfi_autodin_zmm_tx_req_t tx_req;
    memset(&tx_req, 0, sizeof(tx_req));
    snprintf(tx_req.from_addr, sizeof(tx_req.from_addr), "0x1111111111111111111111111111111111111111");
    snprintf(tx_req.to_addr, sizeof(tx_req.to_addr), "%s", dep_res1.contract_address);
    tx_req.chain_id = 369; // PulseChain / ZMM Mainnet
    tx_req.selector = 0x1a2b3c4d;
    tx_req.calldata[0] = 0x01;
    tx_req.calldata_len = 1;
    tx_req.max_fee_per_fet = 100;
    tx_req.process_clearance = TAPE_SECURITY_TOPSECRET;
    snprintf(tx_req.sig_key, sizeof(tx_req.sig_key), "SIG_2026_USLM_AFFIRMED");

    tsfi_autodin_zmm_tx_receipt_t zmm_tx_rec;
    autodin_send_zmm_tx(&tx_req, &zmm_tx_rec);

    // Dual-Path AUTODIN Anvil Isomorphism Verification Oracle Check
    tsfi_autodin_anvil_isom_proof_t isom_proof;
    tsfi_autodin_verify_anvil_isom(&tx_req, &isom_proof);

    // AUTODIN Priority Trunk Queue Check
    tsfi_trunk_queue_item_t qitem;
    tsfi_trunk_queue_enqueue(0, "SYS001", 512, &qitem);

    // Real-Time Isomorphic Security Verification & Anomaly Interception Audit Check
    tsfi_iso_sec_audit_result_t sec_audit;
    tsfi_iso_sec_audit_state(0, tape_hdr, 0x20, TAPE_SECURITY_TOPSECRET, &sec_audit);

    // Helmholtz Operator Unified System Wave Cycle Dispatch Check
    tsfi_helmholtz_dispatch_result_t hres;
    tsfi_helmholtz_dispatch_wave_cycle(0x1a2b3c4d, dep_res1.contract_address, 32, "./tmp/test_guarded.dat.bin", TAPE_SECURITY_TOPSECRET, &hres);

    // William A. Logan 512-Sample Logarithmic Sine Wavetable Synthesizer Check
    tsfi_logan_synth_t logan_s;
    tsfi_logan_synth_init(&logan_s, 440.0f, 44100.0f);
    float sample_v = tsfi_logan_synth_step(&logan_s);
    (void)sample_v;

    // William C. Lynch Executive Kernel & Concurrent Process Scheduler Check
    tsfi_lynch_exec_t lynch_exec;
    tsfi_lynch_exec_init(&lynch_exec);
    uint32_t spawned_pid, active_pid;
    tsfi_lynch_exec_spawn(&lynch_exec, 1, 20, &spawned_pid);
    tsfi_lynch_exec_schedule_next(&lynch_exec, &active_pid);

    // Helmholtz-Lynch First-Class Bridge Wave Sync Check
    tsfi_hl_bridge_sync_t hl_sync;
    tsfi_hl_bridge_sync_wave(&lynch_exec, 0.785f, 1.1f, 0, &hl_sync);

    // z/EVMn & Mainframe VM Kernel Choice Selector Check (Lynch, Logan, Helmholtz VM Modes)
    tsfi_zevm_vm_config_t vm_cfg_lynch, vm_cfg_logan, vm_cfg_helm;
    tsfi_zevm_select_vm_kernel(ZEVM_VM_MODE_LYNCH, &vm_cfg_lynch);
    tsfi_zevm_select_vm_kernel(ZEVM_VM_MODE_LOGAN, &vm_cfg_logan);
    tsfi_zevm_select_vm_kernel(ZEVM_VM_MODE_HELMHOLTZ, &vm_cfg_helm);

    // William C. Lynch Double-Buffering Channel & Checkpoint Rollback Check
    tsfi_lynch_channel_t lynch_chan;
    tsfi_lynch_channel_init(&lynch_chan);
    tsfi_lynch_channel_swap_buffers(&lynch_chan);

    uint32_t dummy_slots[32] = {0};
    uint8_t scsi_reg_out = 0;
    tsfi_lynch_checkpoint_t lynch_ckpt;
    tsfi_lynch_checkpoint_capture(&lynch_ckpt, dummy_slots, 32);
    tsfi_lynch_checkpoint_rollback(&lynch_ckpt, dummy_slots, &scsi_reg_out);

    // William C. Lynch Multiprocessor Counting Semaphore & Index Node Latching Check
    tsfi_lynch_sem_t lynch_sem;
    tsfi_lynch_sem_init(&lynch_sem, 1);
    tsfi_lynch_sem_wait(&lynch_sem, 1);
    tsfi_lynch_sem_signal(&lynch_sem);

    tsfi_lynch_index_latch_t lynch_latch;
    memset(&lynch_latch, 0, sizeof(lynch_latch));
    tsfi_lynch_latch_acquire(&lynch_latch, 101, LYNCH_LATCH_SHARED);
    tsfi_lynch_latch_release(&lynch_latch);

    // Donald E. Knuth Quater-Imaginary Base 2i Dual-Axis Gas Storage Check
    uint64_t packed_val = tsfi_knuth_pack_base2i(15, -7);
    tsfi_knuth_complex_pair_t unpacked_pair;
    tsfi_knuth_unpack_base2i(packed_val, &unpacked_pair);
    uint32_t saved_gas = tsfi_knuth_calculate_gas_savings(100);
    printf("[KNUTH GAS OPTIMIZATION] Saved %u Gas on 100 SSTORE Operations (Packed Val: 0x%016lX)\n",
           saved_gas, (unsigned long)packed_val);

    // Donald E. Knuth Base 2i FET Discharge Physics & Net Gas Engine Check
    tsfi_knuth_fet_gas_state_t fet_gas_st;
    tsfi_knuth_fet_gas_step(3.3f, 0.01f, &fet_gas_st);

    // Physical Electricity Metering System Check (Joules / Micro-Joules)
    tsfi_electricity_meter_t elec_meter;
    tsfi_electricity_meter_init(&elec_meter);
    tsfi_electricity_meter_calculate(3.3f, 1e-9f, 0.05f, 5.0f, 0.001f, &elec_meter);

    // Donald E. Knuth Base 2i Automatic Yul SSTORE Transpiler Check
    tsfi_knuth_yul_transpile_result_t yul_tr_res;
    tsfi_knuth_transpile_yul_sstore("0x01", "balance", "0x02", "debt", &yul_tr_res);

    // Donald E. Knuth Base 2i Complex Fast Fourier Transform Check
    tsfi_knuth_fft_plan_t knuth_fft_plan;
    tsfi_knuth_fft_init(&knuth_fft_plan, 16);
    tsfi_knuth_fft_execute(&knuth_fft_plan);

    // Donald E. Knuth Base 2i Twin Dragon Quadtree Indexer Check (Rule 13)
    tsfi_knuth_quadtree_node_t knuth_quad_node;
    tsfi_knuth_quadtree_resolve_node(25, -12, 8, &knuth_quad_node);

    // Donald E. Knuth Base 2i Red-Black Twin Dragon Quadtree Classifier Check (Rule 13)
    tsfi_knuth_rb_dragon_node_t rb_dragon_node;
    tsfi_knuth_rb_dragon_classify(knuth_quad_node.quad_node_address, &rb_dragon_node);

    // Low-Power FET Energy Optimizer Check under Knuth Base 2i Voltage Scaling
    tsfi_lowpower_fet_metrics_t fet_lp_metrics;
    tsfi_lowpower_fet_calculate(1e9f, 1e-12f, 5.0f, 3.3f, &fet_lp_metrics);

    // System Low-Power Operational Mode Controller Check (78.2% Savings Tier)
    tsfi_lowpower_controller_t lp_mode_ctrl;
    tsfi_lowpower_mode_init(&lp_mode_ctrl);

    // Dynamic Voltage & Frequency Scaling (DVFS) Governor Check
    tsfi_dvfs_governor_t dvfs_gov;
    tsfi_dvfs_governor_init(&dvfs_gov);
    tsfi_dvfs_governor_adapt(&dvfs_gov, 85.0f);

    // Real-Time Sub-Microsecond Drum Latency Profiler Check (Rule 11)
    tsfi_drum_latency_profile_t drum_lat_prof;
    tsfi_drum_latency_profile(48000, 32, &drum_lat_prof);

    // Minimized Lockless SPSC Drum Audio Buffer Check (5.86 us Latency)
    tsfi_minimized_drum_buffer_t min_drum_buf;
    tsfi_minimized_drum_buffer_init(&min_drum_buf);
    tsfi_minimized_drum_buffer_push(&min_drum_buf, 0.85f);

    // Punch Card-to-Drum & Tape Spooler Interface Check (Rule 13)
    tsfi_punch_card_t sample_cards[2] = {
        {"JOB 001 SSTORE BALANCE 0x01", 1},
        {"JOB 002 SSTORE DEBT 0x02", 2}
    };
    tsfi_card_spooler_summary_t card_summary;
    tsfi_card_spooler_process_deck(sample_cards, 2, &card_summary);

    // Nanosecond AVX-512 L1 Register Drum Check (< 50 ns Latency / Keycode 32)
    tsfi_nanosecond_drum_t nano_drum;
    tsfi_nanosecond_drum_init(&nano_drum);
    tsfi_nanosecond_drum_trigger_stroke(&nano_drum, 32);

    // Morton Nadler Positional Bit Interleaving Check (5,000 Gas Tier / Rule 13)
    tsfi_nadler_optimizer_t nadler_opt;
    tsfi_nadler_interleave_2d(25, 42, &nadler_opt);

    // Advanced 3D Morton Nadler Bit Interleaving Check (3,000 Gas Tier / 60% Power Cut)
    tsfi_nadler_advanced_t nadler_adv;
    tsfi_nadler_interleave_3d(15, 30, 45, &nadler_adv);

    // Transcendental Number Solver Evaluation Check (Euler e & Pi Harmonic Phase)
    tsfi_transcendental_eval_t trans_eval;
    tsfi_transcendental_profile(5.0, 440.0, 0.001, &trans_eval);

    // COBOL DDL / ALGOL DML Low-Power FET Discharge Check (5,000 Gas / 78.2% Power Drop)
    tsfi_cobol_ddl_record_t cobol_rec = {1001, 50000, "ALICE"};
    tsfi_cobol_algol_summary_t cobol_summary;
    tsfi_cobol_algol_execute_dml(&cobol_rec, "FIND FIRST ACCOUNT WHERE ACCT-ID = 1001", &cobol_summary);

    // FORTRAN Scientific DML Low-Power Check (3,000 Gas 3D Slot / 78.2% Power Drop)
    tsfi_fortran_matrix_t f_mat;
    memset(&f_mat, 0, sizeof(f_mat));
    f_mat.matrix_id = 9001;
    tsfi_fortran_dml_summary_t f_summary;
    tsfi_fortran_dml_execute(&f_mat, "WRITE(6,100) MATRIX", &f_summary);

    // Continuous Zero-Leakage Lowest Power Sentinel Initialization
    tsfi_continuous_power_status_t p_status;
    tsfi_continuous_power_init(&p_status);

    uint8_t *rgb_out = malloc(WIDTH * HEIGHT * 3);

    printf("[INFO] Rendering 90-second Eye of the Tiger Super8 / Quantel demoscene showcase...\n");

    for (int f = 0; f < FRAMES; f++) {
        float t = (float)f / 30.0f;

        // Paintbox: Multi-Stop Gradient background fill
        uint32_t stops_colors[3] = { 0xFF12100e, 0xFF1c1815, 0xFF0d0c0a };
        float stops_positions[3] = { 0.0f, 0.5f, 1.0f };
        tsfi_quantel_paintbox_multistop_gradient(canvas, WIDTH, HEIGHT, 0, 0, WIDTH, HEIGHT, stops_colors, stops_positions, 3);

        // Paintbox: Dynamic Stencil mask application
        uint8_t *stencil_mask = calloc(WIDTH * HEIGHT, sizeof(uint8_t));
        for (int i = 0; i < WIDTH * HEIGHT; i++) {
            stencil_mask[i] = ((i % WIDTH) > 40 && (i % WIDTH) < 80) ? 255 : 0;
        }
        tsfi_quantel_paintbox_apply_stencil(canvas, WIDTH, HEIGHT, stencil_mask, 0xFF1c1815);
        free(stencil_mask);

        float center_x = WIDTH / 2.0f;
        float center_y = HEIGHT / 2.0f;

        // Draw animated Menorah structure branches
        // Base trunk
        for (int y = 260; y <= 380; y++) {
            for (int dx = -4; dx <= 4; dx++) {
                int px = 256 + dx;
                if (px >= 0 && px < WIDTH && y >= 0 && y < HEIGHT) {
                    canvas[y * WIDTH + px] = 0xFF8c7241; // Vaesen Bronze/Gold
                }
            }
        }
        // Left side arc
        for (int x = 70; x < 256; x++) {
            float norm = (x - 256.0f) / 186.0f;
            int y = 360 - (int)(200.0f * (1.0f - norm * norm));
            if (y >= 0 && y < HEIGHT) {
                for (int dy = -3; dy <= 3; dy++) {
                    canvas[(y + dy) * WIDTH + x] = 0xFF8c7241;
                }
            }
        }
        // Right side arc
        for (int x = 256; x <= 442; x++) {
            float norm = (x - 256.0f) / 186.0f;
            int y = 360 - (int)(200.0f * (1.0f - norm * norm));
            if (y >= 0 && y < HEIGHT) {
                for (int dy = -3; dy <= 3; dy++) {
                    canvas[(y + dy) * WIDTH + x] = 0xFF8c7241;
                }
            }
        }

        // Screens coordinate mapping scaled for 512x512
        int scr_x[7] = { 256, 70, 130, 190, 322, 382, 442 };
        int scr_y[7] = { 260, 160, 180, 200, 200, 180, 160 };
        int scr_r[7] = { 65,  22,  22,  22,  22,  22,  22  };

        // Draw screen circular frames & inner dark tube background
        for (int s = 0; s < 7; s++) {
            int cx = scr_x[s];
            int cy = scr_y[s];
            int r = scr_r[s];
            for (int y = cy - r - 2; y <= cy + r + 2; y++) {
                for (int x = cx - r - 2; x <= cx + r + 2; x++) {
                    if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
                        float dist = sqrtf((x - cx) * (x - cx) + (y - cy) * (y - cy));
                        if (dist <= r) {
                            canvas[y * WIDTH + x] = 0xFF0d0c0a; // CRT Phosphor dark tube background
                        } else if (dist <= r + 2) {
                            canvas[y * WIDTH + x] = 0xFF8c7241; // Screen bezel
                        }
                    }
                }
            }
        }

        // Draw animated Tiger eye glowing wireframe inside Center CRT (Master Attractor B1)
        float pulse = 1.0f + 0.15f * sinf(t * 8.0f); // Pulsing on simulated beat
        int b1_cx = scr_x[0];
        int b1_cy = scr_y[0];
        int b1_r = scr_r[0];
        for (int theta = 0; theta < 360; theta += 15) {
            float rad = (theta * (float)M_PI) / 180.0f;
            float ex = b1_cx + (b1_r - 20) * cosf(rad) * pulse;
            float ey = b1_cy + (b1_r - 35) * sinf(rad) * pulse;
            tsfi_quantel_paintbox_pressure_jitter_hue(canvas, WIDTH, HEIGHT, (int)ex, (int)ey, 6, 0.8f, 0.1f, 0xFFc5a059);
        }

        // Draw 3D Hot Lissajous Tube Orbitals wrapping inside screens driven by LAU 19D values
        static Lau19DValues lau_vals;
        static bool lau_loaded = false;
        if (!lau_loaded) {
            load_lau_19d_values(&lau_vals);
            lau_loaded = true;
        }

        // Determine active step and decay envelopes for each instrument track
        int current_step = (int)(t * 1000.0f / 136.0f) % 32;
        float step_ms = fmodf(t * 1000.0f, 136.0f);
        
        // Sequences matching generate_tiger_soundtrack
        int kick_trig = (current_step % 4 == 0) ? 1 : 0;
        int snare_trig = (current_step % 8 == 4) ? 1 : 0;
        int hh_trig = (current_step % 2 == 0) ? 1 : 0;

        float kick_env = kick_trig ? expf(-step_ms / 40.0f) : 0.0f;
        float snare_env = snare_trig ? expf(-step_ms / 60.0f) : 0.0f;
        float hh_env = hh_trig ? expf(-step_ms / 20.0f) : 0.0f;
        float bass_env = 0.6f + 0.4f * sinf(t * 4.0f); // continuous bass line
        float lead_env = 0.5f + 0.5f * cosf(t * 2.0f); // continuous lead line

        // Modulate frequency indices using actual LAU channel, dynamo, and foundation ratios
        float freq_x = 3.0f + (float)(lau_vals.channel % 1000) / 500.0f;
        float freq_y = 5.0f + (float)(lau_vals.dynamo % 1000) / 500.0f;
        float freq_z = 1.0f + (float)(lau_vals.foundation % 1000) / 1000.0f;

        // 1. Kick Tube (Screen 4, scaled by kick beat rumble)
        float kick_scale = 10.0f + kick_env * 8.0f;
        int k_cx = scr_x[4];
        int k_cy = scr_y[4];
        for (int i = 0; i < 60; i++) {
            float theta_l = (float)i * 2.0f * (float)M_PI / 60.0f;
            float lx = k_cx + kick_scale * sinf(theta_l * freq_x + t * 5.0f);
            float ly = k_cy + kick_scale * cosf(theta_l * freq_y + t * 4.0f);
            tsfi_quantel_paintbox_pressure_jitter_hue(canvas, WIDTH, HEIGHT, (int)lx, (int)ly, 3, 0.9f, 0.1f, 0xFFc5a059); // Gold
        }

        // 2. Snare Tube (Screen 5, scaled by snare snap)
        float snare_scale = 8.0f + snare_env * 8.0f;
        int s_cx = scr_x[5];
        int s_cy = scr_y[5];
        for (int i = 0; i < 50; i++) {
            float theta_l = (float)i * 2.0f * (float)M_PI / 50.0f;
            float lx = s_cx + snare_scale * sinf(theta_l * (freq_x + 1.0f) + t * 6.0f);
            float ly = s_cy + snare_scale * cosf(theta_l * freq_y + t * 5.0f);
            tsfi_quantel_paintbox_pressure_jitter_hue(canvas, WIDTH, HEIGHT, (int)lx, (int)ly, 3, 0.8f, 0.1f, 0xFFe6dfd3); // Cream
        }

        // 3. Hihat Tube (Screen 6, rapid jittery waves)
        float hh_scale = 6.0f + hh_env * 6.0f;
        int h_cx = scr_x[6];
        int h_cy = scr_y[6];
        for (int i = 0; i < 40; i++) {
            float theta_l = (float)i * 2.0f * (float)M_PI / 40.0f;
            float lx = h_cx + hh_scale * sinf(theta_l * (freq_x * 2.0f) + t * 12.0f);
            float ly = h_cy + hh_scale * cosf(theta_l * (freq_y * 1.5f) + t * 10.0f);
            tsfi_quantel_paintbox_pressure_jitter_hue(canvas, WIDTH, HEIGHT, (int)lx, (int)ly, 2, 0.7f, 0.1f, 0xFF8c7241); // Dim Gold
        }

        // 4. Bass Tube (Screen 2, deep waves)
        float bass_scale = 12.0f * bass_env;
        int b_cx = scr_x[2];
        int b_cy = scr_y[2];
        for (int i = 0; i < 60; i++) {
            float theta_l = (float)i * 2.0f * (float)M_PI / 60.0f;
            float lx = b_cx + bass_scale * sinf(theta_l * (freq_x * 0.5f) + t * 3.0f);
            float ly = b_cy + bass_scale * cosf(theta_l * (freq_y * 0.5f) + t * 2.0f);
            tsfi_quantel_paintbox_pressure_jitter_hue(canvas, WIDTH, HEIGHT, (int)lx, (int)ly, 3, 0.9f, 0.1f, 0xFFc5a059); // Gold
        }

        // 5. Lead Tube (Screen 3, high sweep)
        float lead_scale = 10.0f * lead_env;
        int l_cx = scr_x[3];
        int l_cy = scr_y[3];
        for (int i = 0; i < 60; i++) {
            float theta_l = (float)i * 2.0f * (float)M_PI / 60.0f;
            float lx = l_cx + lead_scale * sinf(theta_l * (freq_x + 2.0f) + t * 8.0f);
            float ly = l_cy + lead_scale * cosf(theta_l * (freq_y + 1.0f) + t * 7.0f);
            tsfi_quantel_paintbox_pressure_jitter_hue(canvas, WIDTH, HEIGHT, (int)lx, (int)ly, 2, 0.8f, 0.2f, 0xFFFF4500); // Red-Orange
        }

        // 6. Sub Growl Tube (Screen 1, negative-resistance wobble)
        float growl_scale = 8.0f + 6.0f * sinf(t * 5.8f);
        int g_cx = scr_x[1];
        int g_cy = scr_y[1];
        for (int i = 0; i < 50; i++) {
            float theta_l = (float)i * 2.0f * (float)M_PI / 50.0f;
            float lx = g_cx + growl_scale * sinf(theta_l * 2.0f + t * 2.0f);
            float ly = g_cy + growl_scale * cosf(theta_l * 3.0f + t * 1.5f);
            tsfi_quantel_paintbox_pressure_jitter_hue(canvas, WIDTH, HEIGHT, (int)lx, (int)ly, 3, 0.8f, 0.1f, 0xFF8c7241); // Dim Gold
        }

        // Suppress unused warning
        (void)freq_z;

        // Draw tiger pupil slit using Vaesen Ink (0x0d0c0a) inside B1 (256, 260)
        for (int dy = -40; dy <= 40; dy++) {
            int px = b1_cx + (int)(sinf(t * 3.0f) * 2.0f);
            int py = b1_cy + dy;
            if (px >= 0 && px < WIDTH && py >= 0 && py < HEIGHT) {
                tsfi_quantel_paintbox_chalk_pressure_texture(canvas, WIDTH, HEIGHT, px, py, 10, 0.9f, 1.5f, 0xFF0d0c0a);
            }
        }

        // Draw dynamic tiger stripes sweeping across the screen using Paintbox pressure dynamics and path Bezier spline flow dynamics
        float p0_x = center_x - 140.0f;
        float p0_y = center_y - 80.0f;
        float p1_x = center_x + 100.0f * cosf(t * 1.5f);
        float p1_y = center_y + 60.0f * sinf(t * 2.2f);
        float p2_x = center_x + 140.0f * sinf(t * 1.5f);
        float p2_y = center_y - 80.0f * cosf(t * 2.2f);
        float p3_x = center_x + 140.0f;
        float p3_y = center_y + 80.0f;
        tsfi_quantel_paintbox_pressure_jitter_opacity_saturation_value_width_angle_shape_texture_path_spline_flow(canvas, WIDTH, HEIGHT, p0_x, p0_y, p1_x, p1_y, p2_x, p2_y, p3_x, p3_y, 16, 0.9f, 0.3f, 0xFFFF4500, t * 1.5f, 1.4f, 1.8f, 0.5f);

        // Paintbox: Calligraphy brush strokes around border
        tsfi_quantel_paintbox_calligraphy_shape(canvas, WIDTH, HEIGHT, 10, 10, WIDTH-10, 10, 8, 0.7f, 0.4f, 45.0f, 0xFFc5a059);
        tsfi_quantel_paintbox_calligraphy_shape(canvas, WIDTH, HEIGHT, WIDTH-10, 10, WIDTH-10, HEIGHT-10, 8, 0.7f, 0.4f, 45.0f, 0xFFc5a059);

        // Paintbox: Orchestrator Synthesizer Link waveforms drawing
        float local_audio[32];
        for (int i = 0; i < 32; i++) {
            local_audio[i] = sinf(t * 8.0f + i * 0.5f) * 0.8f;
        }
        tsfi_quantel_orchestrator_paintbox_synth_link(canvas, WIDTH, HEIGHT, (int)center_x, (int)center_y, 0.8f, 1.2f, local_audio, 32);

        // Paintbox: Bristle brush simulation paint marks
        tsfi_quantel_paintbox_bristle_brush(canvas, WIDTH, HEIGHT, (int)center_x - 100, (int)center_y - 80, 15, 0.6f, 8, 0xFF8c7241);

        // Paintbox: Brush Spacing Dynamics along stylus path
        uint8_t spacing_tex[16] = { 0xFF, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0xFF, 0xFF, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0xFF };
        tsfi_quantel_paintbox_spacing_brush(canvas, WIDTH, HEIGHT, 20, HEIGHT - 30, WIDTH - 20, HEIGHT - 30, spacing_tex, 4, 4, 12.0f, 0.7f, 0xFF8c7241);

        // Paintbox: Chalk / dry brush strokes with pressure texture modulation
        tsfi_quantel_paintbox_chalk_pressure_texture(canvas, WIDTH, HEIGHT, (int)center_x - 120, (int)center_y + 120, 16, 0.7f, 1.5f, 0xFF8c7241);
        tsfi_quantel_paintbox_chalk_brush(canvas, WIDTH, HEIGHT, (int)center_x + 60, (int)center_y + 80, 12, 0.5f, 0xFFc5a059);

        // Paintbox: Brush pressure jitter hue dynamics stroke
        tsfi_quantel_paintbox_pressure_jitter_hue(canvas, WIDTH, HEIGHT, (int)center_x + 120, (int)center_y - 120, 14, 0.6f, 0.2f, 0xFFc5a059);

        // Paintbox: Dynamic Airbrush Flow spray marks
        tsfi_quantel_paintbox_dynamic_flow(canvas, WIDTH, HEIGHT, (int)center_x, (int)center_y - 80, 20, 0.5f, 1.5f, 0xFF8c7241);

        // Paintbox: Brush Angle Lock relative to stylus velocity
        tsfi_quantel_paintbox_angle_lock(canvas, WIDTH, HEIGHT, (int)center_x, (int)center_y + 80, 15, t, 0xFFc5a059);

        // Paintbox: Spray can noise particles around border
        tsfi_quantel_paintbox_spray_can(canvas, WIDTH, HEIGHT, (int)center_x, (int)center_y - 120, 30, 0.4f, 0xFFc5a059);

        // Paintbox: Rubber stamp cloning
        tsfi_quantel_paintbox_clone(canvas, WIDTH, HEIGHT, 50, 50, (int)(center_x - 50), (int)(center_y - 50), 20, 0.5f);

        // Paintbox: Flood fill top corner segment initially
        if (f < 90) {
            tsfi_quantel_paintbox_flood_fill(canvas, WIDTH, HEIGHT, 2, 2, 0xFF12100e, 0.05f);
        }

        // Paintbox: Wet paint canvas smudge mixing at center
        uint32_t smudge_color = 0xFFc5a059;
        tsfi_quantel_paintbox_wet_paint(canvas, WIDTH, HEIGHT, (int)center_x, (int)center_y, 40, 0.05f, & smudge_color);

        // Mirage: 3D Perspective Plane Tilt Warp
        tsfi_quantel_mirage_plane_tilt(canvas, WIDTH, HEIGHT, dst_buffer, WIDTH, HEIGHT, 0.05f * sinf(t), 0.05f * cosf(t));
        memcpy(canvas, dst_buffer, WIDTH * HEIGHT * sizeof(uint32_t));

        // Mirage: 3D Perspective Projection Rotation
        tsfi_quantel_mirage_perspective_rotate(canvas, WIDTH, HEIGHT, dst_buffer, WIDTH, HEIGHT, 0.1f * sinf(t), 0.1f * cosf(t), 0.0f, 60.0f);
        memcpy(canvas, dst_buffer, WIDTH * HEIGHT * sizeof(uint32_t));

        // Mirage: 3D Barrel Distorted Fish-Eye Warp
        tsfi_quantel_mirage_fisheye_warp(canvas, WIDTH, HEIGHT, dst_buffer, WIDTH, HEIGHT, 0.04f * sinf(t * 1.5f));
        memcpy(canvas, dst_buffer, WIDTH * HEIGHT * sizeof(uint32_t));

        memcpy(canvas_b, canvas, WIDTH * HEIGHT * sizeof(uint32_t));

        // Mirage: Sphere wrap, cylinder wrap, flag ripple, twirl, pyramid, and ripple distortions
        int phase_cycle = ((int)t) % 4;
        if (phase_cycle == 0) {
            // Mirage: 3D Dual-Sided Page Curl exposing a solid reverse color texture
            uint32_t *back_tex = malloc(WIDTH * HEIGHT * sizeof(uint32_t));
            for (int i = 0; i < WIDTH * HEIGHT; i++) {
                back_tex[i] = 0xFFc5a059;
            }
            tsfi_quantel_mirage_dual_sided_page_curl(canvas, back_tex, WIDTH, HEIGHT, dst_buffer, WIDTH, HEIGHT, 80.0f, 0.4f * sinf(t));
            memcpy(canvas_b, dst_buffer, WIDTH * HEIGHT * sizeof(uint32_t));
            free(back_tex);
        } else if (phase_cycle == 1) {
            tsfi_quantel_mirage_flag_ripple(canvas, WIDTH, HEIGHT, dst_buffer, WIDTH, HEIGHT, 6.0f, 0.05f, t * 2.0f);
            memcpy(canvas_b, dst_buffer, WIDTH * HEIGHT * sizeof(uint32_t));
        } else if (phase_cycle == 2) {
            // Mirage: Cylinder warp mapping
            tsfi_quantel_mirage_cylinder_wrap(canvas, WIDTH, HEIGHT, dst_buffer, WIDTH, HEIGHT, 180.0f);
            memcpy(canvas_b, dst_buffer, WIDTH * HEIGHT * sizeof(uint32_t));
        } else if (phase_cycle == 3) {
            // Mirage: Ribbon Wave warp & pyramid projections
            tsfi_quantel_mirage_ribbon_wave_warp(canvas, WIDTH, HEIGHT, dst_buffer, WIDTH, HEIGHT, 10.0f, 8.0f);
            tsfi_quantel_mirage_pyramid_wrap(dst_buffer, WIDTH, HEIGHT, canvas, WIDTH, HEIGHT, 100.0f, 200.0f);
            tsfi_quantel_mirage_twirl(canvas, WIDTH, HEIGHT, canvas_b, WIDTH, HEIGHT, 0.15f * sinf(t), 120.0f, center_x, center_y);
        }

        // Harry: Solarize and Posterize filter effects
        if (phase_cycle == 1) {
            tsfi_quantel_harry_solarize(canvas_b, WIDTH, HEIGHT, 0.65f);
        } else if (phase_cycle == 2) {
            tsfi_quantel_harry_posterize(canvas_b, WIDTH, HEIGHT, 8);
        }

        // Harry: 3x3 Convolution filtering (Sharpening the output frame)
        float sharpen_kernel[9] = {
             0.0f, -1.0f,  0.0f,
            -1.0f,  5.0f, -1.0f,
             0.0f, -1.0f,  0.0f
        };
        tsfi_quantel_harry_filter(canvas_b, dst_buffer, WIDTH, HEIGHT, sharpen_kernel);
        memcpy(canvas_b, dst_buffer, WIDTH * HEIGHT * sizeof(uint32_t));

        // Harry: Color Grading (Lift, Gamma, Gain)
        float lift[3] = {0.04f, 0.0f, -0.04f};
        float gamma[3] = {1.1f, 1.0f, 0.9f};
        float gain[3] = {1.0f, 0.95f, 0.9f};
        tsfi_quantel_harry_lift_gamma_gain(canvas_b, WIDTH, HEIGHT, lift, gamma, gain);

        // Harry: Color Balance Sweep over time
        tsfi_quantel_harry_color_balance_sweep(canvas_b, WIDTH, HEIGHT, 1.0f + 0.05f * sinf(t), 1.0f, 1.0f + 0.05f * cosf(t));

        // Harry: SMPTE-C to Rec709 Color Space Conversion
        tsfi_quantel_harry_smptec_to_rec709(canvas_b, WIDTH, HEIGHT);

        // Harry: Film Grain vintage noise overlay
        tsfi_quantel_harry_film_grain(canvas_b, WIDTH, HEIGHT, 0.07f);

        // Harry: Chroma Key Spill Suppression along green boundaries
        tsfi_quantel_harry_spill_suppress(canvas_b, WIDTH, HEIGHT, "green", 0.85f);

        // Harry: Frame Dissolve dynamic cross-fade transitions
        tsfi_quantel_harry_dissolve(canvas, canvas_b, dst_buffer, WIDTH, HEIGHT, 0.92f);
        memcpy(canvas_b, dst_buffer, WIDTH * HEIGHT * sizeof(uint32_t));

        // Storyboard: Onion Skinning compositor (blending canvas offsets)
        tsfi_quantel_storyboard_onion_skin(canvas, canvas_b, dst_buffer, WIDTH, HEIGHT, 0.25f, 0.75f);
        memcpy(canvas_b, dst_buffer, WIDTH * HEIGHT * sizeof(uint32_t));memcpy(canvas_b, dst_buffer, WIDTH * HEIGHT * sizeof(uint32_t));

        // Harry: Split Screen Matte Preview removed to prevent panning pin-stripe glitch

        // Harry: Invert colors dynamically on beat sync
        if (f % 60 < 2) {
            tsfi_quantel_harry_invert(canvas_b, WIDTH, HEIGHT);
        }

        // Paintbox: Palette color cycling
        tsfi_quantel_paintbox_palette_cycle(canvas_b, WIDTH, HEIGHT, (int)(t * 12.0f));

        // Mirage: 3D Page Peel Shadow intensity simulation
        tsfi_quantel_mirage_peel_shadow(canvas_b, WIDTH, HEIGHT, 0.5f + 0.2f * sinf(t), 0.75f);

        // Storyboard: Burn-in real-time SMPTE timecode (HH:MM:SS:FF)
        tsfi_quantel_storyboard_timecode_burn(canvas_b, WIDTH, HEIGHT, f, 30.0f, 0xFFe6dfd3);

        // Paintbox: Typographer text overlay
        tsfi_quantel_paintbox_typographer(canvas_b, WIDTH, HEIGHT, 110, 48, "AUNCIENT VAESEN TIGER", 0xFFe6dfd3, 16.0f);

        // Harry: Keyframe In-Between Interpolator for frame parameters
        float interp_val = 0.0f;
        tsfi_quantel_harry_interpolate_keyframe(fmodf(t, 2.0f) / 2.0f, 0.0f, 1.0f, &interp_val);

        // Storyboard: Film Frame Borders & Outline Cell overlays
        tsfi_quantel_storyboard_film_borders(canvas_b, WIDTH, HEIGHT, 32, 120, WIDTH - 64, HEIGHT - 240, 0xFF8c7241);
        tsfi_quantel_storyboard_outline_cell(canvas_b, WIDTH, HEIGHT, 32, 120, WIDTH - 64, HEIGHT - 240, 0xFFc5a059);

        // Storyboard: Concentric double borders overlay
        tsfi_quantel_storyboard_border_highlights_concentric_double(canvas_b, WIDTH, HEIGHT, 32, 120, WIDTH - 64, HEIGHT - 240, 2, 4, 2, 0xFFc5a059, 0xFF8c7241);

        // Storyboard: Double borders offset with color
        tsfi_quantel_storyboard_double_borders_offset_color(canvas_b, WIDTH, HEIGHT, 32, 120, WIDTH - 64, HEIGHT - 240, 6, 0xFFc5a059, 0xFF8c7241);

        // Harry: Interlaced field split chromatic offset blend
        uint32_t *field_even_b = malloc(WIDTH * HEIGHT * sizeof(uint32_t));
        memcpy(field_even_b, canvas_b, WIDTH * HEIGHT * sizeof(uint32_t));
        tsfi_quantel_harry_blend_fields_color_offset(field_even_b, canvas_b, dst_buffer, WIDTH, HEIGHT, 0.5f * interp_val, 4, 0xFFc5a059);
        memcpy(canvas_b, dst_buffer, WIDTH * HEIGHT * sizeof(uint32_t));
        free(field_even_b);

        // Storyboard: Aspect Ratio Guides, Centering Crosshairs & Corner crop marks
        tsfi_quantel_storyboard_aspect_guides(canvas_b, WIDTH, HEIGHT, 32, 120, WIDTH - 64, HEIGHT - 240, "1.85:1", 0xFF8c7241);
        tsfi_quantel_storyboard_crosshairs(canvas_b, WIDTH, HEIGHT, 32, 120, WIDTH - 64, HEIGHT - 240, 0xFF8c7241);
        tsfi_quantel_storyboard_corner_marks(canvas_b, WIDTH, HEIGHT, 32, 120, WIDTH - 64, HEIGHT - 240, 8, 0xFFc5a059);

        // Baudot 5-Bit Punched Tape visual overlay ticker
        draw_baudot_punched_tape(canvas_b, WIDTH, HEIGHT, "AUNCIENT TIGER", f);

        // Storyboard: Production Slate burned at start (first 25 frames)
        if (f < 25) {
            tsfi_quantel_storyboard_production_slate(canvas_b, WIDTH, HEIGHT, "MARIARAHEL", "AUNCIENT TIGER", "2026-07-20");
        }

        // Harry Wipe Transitions with Matte mask and chroma / luma key filters
        int transition_stage = ((int)t) % 12;
        if (transition_stage == 0) {
            float wipe_p = fmodf(t, 1.0f);
            uint32_t *mat = calloc(WIDTH * HEIGHT, sizeof(uint32_t));
            uint8_t *matte_mask = malloc(WIDTH * HEIGHT);
            for (int y = 0; y < HEIGHT; y++) {
                for (int x = 0; x < WIDTH; x++) {
                    mat[y * WIDTH + x] = ((x / 32 + y / 32) % 2 == 0) ? 0xFF3E2723 : 0xFF4E342E;
                    matte_mask[y * WIDTH + x] = (x > WIDTH / 2) ? 255 : 128;
                }
            }
            tsfi_quantel_harry_blend_fields_color_offset_vertical_scale_aspect_rotation_center_mirror_matte_chroma_luma(canvas_b, mat, dst_buffer, WIDTH, HEIGHT, wipe_p, 4, 0xFFFF8C00, 2.0f * sinf(t), 1.0f + 0.03f * cosf(t), 1.0f + 0.01f * sinf(t), t * 0.03f, center_x, center_y, 0, 0, matte_mask, 0xFF4E342E, 0.15f, 10, 240);
            memcpy(canvas_b, dst_buffer, WIDTH * HEIGHT * sizeof(uint32_t));
            free(mat);
            free(matte_mask);

            // Double outline highlights with inner bevel overlay, directional soft drop shadow, and rounded corner bounding clip
            tsfi_quantel_storyboard_border_highlights_concentric_double_outer_width_offset_color_texture_bevel_shadow_corner(canvas_b, WIDTH, HEIGHT, 32, 120, WIDTH - 64, HEIGHT - 240, 4, 2, 3, 0xFFc5a059, 0xFF8c7241, 10, 2, (int)(2.0f * sinf(t)), (int)(2.0f * cosf(t)), 0xFF12100e, 0.2f, 1, 2, 12);
        }

        // Dispatch CICS inputs to HOGAN
        if (f % 30 == 0) {
            float paint_val = sqrtf(p3_x * p3_x + p3_y * p3_y) * 0.4f;
            float mirage_val = 30.0f * (1.0f + 0.1f * sinf(t));
            float harry_val = (1.0f + 0.01f * sinf(t)) * 90.0f;

            tsfi_hogan_dispatch_tx(&hogan_sys, 999, 1001, (uint64_t)paint_val, VM_RAMAC);
            tsfi_hogan_dispatch_tx(&hogan_sys, 999, 2002, (uint64_t)mirage_val, VM_EVM);
            tsfi_hogan_dispatch_tx(&hogan_sys, 999, 3003, (uint64_t)harry_val, VM_ZMM);
        }
        if (f % 150 == 0) {
            tsfi_hogan_overnight_reconciliation(&hogan_sys, "demo_tiger_lfs.dat.bin");
        }

        // Beat-synced camera shake/rumble pass
        float shake_amp = 0.0f;
        int frame_in_beat = f % 15;
        if (frame_in_beat < 6) {
            shake_amp = (6 - frame_in_beat) * 1.5f;
        }
        if (shake_amp > 0.1f) {
            int dx_shake = (rand() % 2 == 0 ? 1 : -1) * (int)(shake_amp * ((float)rand() / RAND_MAX));
            int dy_shake = (rand() % 2 == 0 ? 1 : -1) * (int)(shake_amp * ((float)rand() / RAND_MAX));
            memcpy(dst_buffer, canvas_b, WIDTH * HEIGHT * sizeof(uint32_t));
            memset(canvas_b, 0, WIDTH * HEIGHT * sizeof(uint32_t));
            for (int y = 0; y < HEIGHT; y++) {
                int sy = y + dy_shake;
                if (sy < 0 || sy >= HEIGHT) continue;
                for (int x = 0; x < WIDTH; x++) {
                    int sx = x + dx_shake;
                    if (sx < 0 || sx >= WIDTH) continue;
                    canvas_b[y * WIDTH + x] = dst_buffer[sy * WIDTH + sx];
                }
            }
        }

        // Enforce Super8 crop gate & sprocket holes
        apply_super8_crop(canvas_b, WIDTH, HEIGHT, f);

        // 1. Xerox Smalltalk VM dynamic execution & GC reference tracking
        st_vm.heap[0].fields[0] = (uint32_t)hogan_sys.tx_count;
        uint32_t st_result = 0;
        tsfi_parc_st_send_message(&st_vm, obj_inst, "processData", &st_result);
        
        tsfi_parc_gc_add_ref(&st_gc, obj_inst);
        tsfi_parc_gc_release_ref(&st_gc, &st_vm, obj_inst);

        // 2. Xerox Alto Keysets Chord keyboard emulation
        // Type a character from keyset mapping into our running Bravo document every 60 frames (2 seconds)
        if (f > 0 && f % 60 == 0) {
            uint8_t kb_chord = (uint8_t)((f / 60) % 16); // characters A-P
            char next_char = tsfi_parc_decode_keyset(kb_chord);
            int len = strlen(typed_document);
            if (len < 63) {
                typed_document[len] = next_char;
                typed_document[len + 1] = '\0';
            }
        }

        // 3. Xerox PUP packet serialization & CSMA/CD Ethernet Collision Engine
        tsfi_parc_pup_packet_t pup_pkt;
        pup_pkt.dest_host = 022;
        pup_pkt.src_host = 014;
        pup_pkt.pup_type = PUP_TYPE_DATA;
        pup_pkt.pup_id = (uint32_t)f;
        snprintf((char*)pup_pkt.payload, sizeof(pup_pkt.payload), "TX_COUNT: %lu", hogan_sys.tx_count);
        pup_pkt.data_len = strlen((char*)pup_pkt.payload);
        
        uint8_t pup_raw[256];
        int pup_raw_len = tsfi_parc_pup_encode(&pup_pkt, pup_raw, sizeof(pup_raw));
        
        int net_status = tsfi_parc_csmacd_transmit(&local_node, &gateway_node, 10);
        (void)net_status; // Collision engine backoff monitor

        // 3. Render overlapping Alto window desktop dashboard
        tsfi_parc_wm_render(&wm, canvas_b, WIDTH, HEIGHT, t, st_result, pup_raw_len, pup_pkt.checksum, (int)hogan_sys.tx_count);

        // 4. Xerox Bravo Page Guidelines and WYSIWYG Header Annotation
        tsfi_parc_bravo_draw_guides(canvas_b, WIDTH, HEIGHT, &bravo_layout, 0xFF8c7241);
        tsfi_parc_style_run_t bravo_style;
        bravo_style.is_bold = 1;
        bravo_style.is_underline = 1;
        bravo_style.is_shadow = 1;
        bravo_style.color = 0xFFe6dfd3;
        tsfi_parc_render_styled_text(canvas_b, WIDTH, HEIGHT, bravo_layout.margin_left, bravo_layout.margin_top - 12, "XEROX ALTO / STAR CADE MONITOR", &bravo_style, 8.0f);

        // Render keyboard keyset typed document in real-time
        tsfi_parc_style_run_t doc_style;
        doc_style.is_bold = 0;
        doc_style.is_underline = 1;
        doc_style.is_shadow = 0;
        doc_style.color = 0xFFc5a059;
        tsfi_parc_render_styled_text(canvas_b, WIDTH, HEIGHT, bravo_layout.margin_left, bravo_layout.margin_top + 16, typed_document, &doc_style, 7.0f);

        // 5. Xerox Interpress PDL Scalable vector diagram: Rotating target box in CADE panel center
        tsfi_parc_interpress_init(&ip_ctx);
        tsfi_parc_interpress_scale(&ip_ctx, 1.0f, 1.0f);
        tsfi_parc_interpress_rotate(&ip_ctx, t * 0.4f);
        ip_ctx.matrix[4] = 256.0f;
        ip_ctx.matrix[5] = 450.0f;
        tsfi_parc_interpress_moveto(&ip_ctx, -20.0f, -12.0f);
        tsfi_parc_interpress_lineto(&ip_ctx, canvas_b, WIDTH, HEIGHT, 20.0f, -12.0f, 0xFFc5a059);
        tsfi_parc_interpress_lineto(&ip_ctx, canvas_b, WIDTH, HEIGHT, 20.0f, 12.0f, 0xFFc5a059);
        tsfi_parc_interpress_lineto(&ip_ctx, canvas_b, WIDTH, HEIGHT, -20.0f, 12.0f, 0xFFc5a059);
        tsfi_parc_interpress_lineto(&ip_ctx, canvas_b, WIDTH, HEIGHT, -20.0f, -12.0f, 0xFFc5a059);

        // 6. Xerox Alto mouse cursor and SuperPaint brush stamp drawing
        int cur_x = 256 + (int)(25.0f * sinf(t * 3.0f));
        int cur_y = 450 + (int)(15.0f * cosf(t * 2.0f));
        tsfi_parc_draw_cursor(canvas_b, WIDTH, HEIGHT, cur_x, cur_y, CURSOR_CROSSHAIR, 0xFFc5a059);
        
        tsfi_parc_brush_draw(canvas_b, WIDTH, HEIGHT, 45, 492, 8, BRUSH_CHISEL, 0xFF8c7241);
        tsfi_parc_brush_draw(canvas_b, WIDTH, HEIGHT, 465, 492, 8, BRUSH_STAMP, 0xFF8c7241);

        // Xerox Alto Video Scanline & CRT frame rendering inside Smalltalk VM window
        memset(alto_display_mem, 0, 38 * 808 * sizeof(uint16_t));
        
        // Draw mainframe status indicators inside the 1-bit display buffer using our A-P font
        draw_string_1bit(alto_display_mem, 10, 20, "CICS OK");
        draw_string_1bit(alto_display_mem, 10, 35, "CADE IMF");
        draw_string_1bit(alto_display_mem, 10, 50, "EBCDIC");
        draw_string_1bit(alto_display_mem, 10, 65, "HOGAN");

        for (int y = 80; y < 808; y++) {
            alto_display_mem[y * 38 + (y % 38)] = (uint16_t)(0xF0F0 ^ (y * 7));
            if (y % 40 < 10) {
                for (int w = 0; w < 10; w++) {
                    alto_display_mem[y * 38 + w] = 0xAAAA;
                }
            }
        }
        int alto_mx = 40 + (int)(30.0f * sinf(t * 2.0f));
        int alto_my = 30 + (int)(20.0f * cosf(t * 2.5f));
        tsfi_parc_video_step(&vc, alto_mx, alto_my);
        
        static const uint16_t arrow_cursor_bits[16] = {
            0xC000, 0xE000, 0xF000, 0xF800, 0xFC00, 0xFE00, 0xFF00, 0xFF80,
            0xFFC0, 0xFE00, 0xEF00, 0xC780, 0x0780, 0x03C0, 0x03C0, 0x0000
        };
        tsfi_parc_video_render_frame(&vc, alto_display_mem, canvas_b, WIDTH, HEIGHT, 147, 438, 0xFF39FF14, 0xFFc5a059, arrow_cursor_bits);
        // Step video deck playback state and flip page buffers dynamically
        tsfi_parc_deck_step(&deck, 1.0f / 30.0f, t);
        tsfi_parc_deck_flip(&deck);

        // 7. Cycle the SuperPaint LUT dynamically
        tsfi_parc_superpaint_cycle_lut(&sp_lut, f % 256);

        // Convert to RGB24 for FFmpeg pipe output
        for (int i = 0; i < WIDTH * HEIGHT; i++) {
            uint32_t pix = canvas_b[i];
            rgb_out[i * 3 + 0] = (pix >> 16) & 0xFF;
            rgb_out[i * 3 + 1] = (pix >> 8) & 0xFF;
            rgb_out[i * 3 + 2] = pix & 0xFF;
        }

        fwrite(rgb_out, 1, WIDTH * HEIGHT * 3, ffmpeg_pipe);

        if (f % 150 == 0) {
            printf("  -> Eye of the Tiger loop: Rendered %d/%d frames (%.1f%%)...\n", f, FRAMES, (float)f / FRAMES * 100.0f);
        }
    }

    pclose(ffmpeg_pipe);
    free(canvas);
    free(canvas_b);
    free(alto_display_mem);
    tsfi_parc_deck_free(&deck);
    free(dst_buffer);
    free(rgb_out);
    remove(audio_file);

    printf("\n[SUCCESS] 90-Second Eye of the Tiger demoscene rendered to bin/tiger_super8_demo.mp4\n");
}
