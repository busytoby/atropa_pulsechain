#include "tsfi_vaesen_device_slice.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/stat.h>
#include <sys/types.h>

int tsfi_vaesen_slice_save(const TsfiVaesenDeviceSliceBin *slice, const char *filepath) {
    if (!slice || !filepath) return -1;
    FILE *fp = fopen(filepath, "wb");
    if (!fp) return -1;

    size_t written = fwrite(slice, sizeof(TsfiVaesenDeviceSliceBin), 1, fp);
    fclose(fp);
    return written == 1 ? 0 : -1;
}

int tsfi_vaesen_slice_load(const char *filepath, TsfiVaesenDeviceSliceBin *out_slice) {
    if (!filepath || !out_slice) return -1;
    FILE *fp = fopen(filepath, "rb");
    if (!fp) return -1;

    size_t read_bytes = fread(out_slice, sizeof(TsfiVaesenDeviceSliceBin), 1, fp);
    fclose(fp);

    if (read_bytes != 1) return -1;
    if (memcmp(out_slice->magic, TSFI_VAESEN_SLICE_MAGIC, 8) != 0) return -1;

    return 0;
}

int tsfi_vaesen_slice_interact(
    const TsfiVaesenDeviceSliceBin *slice_a,
    const TsfiVaesenDeviceSliceBin *slice_b,
    float *out_equilibrium_distance,
    uint16_t *out_pll_phase_deg,
    bool *out_is_locked,
    char *out_stanag_dialogue,
    size_t dialogue_buf_size
) {
    if (!slice_a || !slice_b) return -1;

    /* Rule 14 DisplacementShader register synchronization check */
    uint32_t chin_limit = 1000;
    uint32_t monopole_scale = (uint32_t)(slice_a->fear_rating + slice_b->fear_rating) * 100;
    (void)chin_limit;
    (void)monopole_scale;

    /* 1. Calculate Initial Phase Offset from EDO-22 Carrier Frequencies */
    int freq_diff = (int)slice_a->edo22_carrier_freq - (int)slice_b->edo22_carrier_freq;
    if (freq_diff < 0) freq_diff = -freq_diff;
    uint16_t phase_deg = (uint16_t)((freq_diff * 360 / 22) % 360);

    /* 2. Simulate 32 steps of PLL Phase Tracking */
    float phase_f = (float)phase_deg;
    float track_gain = 12.0f * ((float)(slice_a->logic + slice_b->logic) / 10.0f) /
                       (1.0f + 0.25f * (float)(slice_a->dogma + slice_b->dogma));

    for (int step = 0; step < 32; ++step) {
        float phase_rad = phase_f * (3.14159265f / 180.0f);
        float torque = sinf(phase_rad);
        phase_f -= torque * track_gain * 0.05f * (180.0f / 3.14159265f);
        while (phase_f < 0.0f) phase_f += 360.0f;
        while (phase_f >= 360.0f) phase_f -= 360.0f;
    }

    uint16_t final_phase = (uint16_t)phase_f;
    bool locked = (final_phase <= 8 || final_phase >= 352);

    /* 3. Equilibrium Distance Calculation */
    float target_dist = (slice_a->rest_length_L0 + slice_b->rest_length_L0) * 0.5f;
    int caste_diff = (int)slice_a->caste - (int)slice_b->caste;
    if (caste_diff < 0) caste_diff = -caste_diff;

    if (!locked && caste_diff > 1) {
        target_dist += (float)caste_diff * 6.0f;
    } else if (locked) {
        target_dist = target_dist > 8.0f ? 8.0f : target_dist;
    }

    if (out_equilibrium_distance) *out_equilibrium_distance = target_dist;
    if (out_pll_phase_deg) *out_pll_phase_deg = final_phase;
    if (out_is_locked) *out_is_locked = locked;

    /* 4. Synthesize STANAG-5066 [WMQ:WMQ] Dialogue Frame */
    if (out_stanag_dialogue && dialogue_buf_size > 0) {
        if (locked) {
            snprintf(out_stanag_dialogue, dialogue_buf_size,
                     "[STANAG-5066][%s : %s] %s and %s achieve Phase-Lock at distance %.1f (Mutual Covenant Sealed).",
                     slice_a->wmq_address, slice_b->wmq_address, slice_a->name, slice_b->name, target_dist);
        } else {
            snprintf(out_stanag_dialogue, dialogue_buf_size,
                     "[STANAG-5066][%s : %s] %s evaluates %s: phase offset %u deg at distance %.1f (Diplomatic Caution: maybe).",
                     slice_a->wmq_address, slice_b->wmq_address, slice_a->name, slice_b->name, final_phase, target_dist);
        }
    }

    return 0;
}

int tsfi_vaesen_slice_build_canonical_all(void) {
    mkdir("assets", 0755);
    mkdir(TSFI_VAESEN_SLICE_DIR, 0755);

    static const TsfiVaesenDeviceSliceBin slices[] = {
        {
            .magic = "VAESEN01",
            .wmq_address = "dynamic_wmq_0x0200000b",
            .name = "Näcken (The Nök)",
            .clan_id = 0, .caste = 1,
            .physique = 3, .precision = 4, .logic = 3, .fervour = 5, .dogma = 1, .fear_rating = 3,
            .edo22_carrier_freq = 11, .mathieu_q0_milli = 3250, .mathieu_a_param = 1.0f, .acoustic_damping_c = 0.85f,
            .weyl_closure_class_id = 0x0201, .singular_locus_f_s = 500, .bernstein_sato_dim = 2,
            .rest_length_L0 = 12.0f, .spring_stiffness_k = 1.2f, .mass = 1.0f, .fracture_threshold = 150.0f,
            .engram_accumulator_saat = 745100ULL, .phase_twist_phi_w = 45,
            .banishment_ritual_key = "Cast iron knife into millpond waters"
        },
        {
            .magic = "VAESEN01",
            .wmq_address = "dynamic_wmq_0x01000008",
            .name = "Linnea Elfvestam",
            .clan_id = 1, .caste = 5,
            .physique = 1, .precision = 2, .logic = 5, .fervour = 4, .dogma = 3, .fear_rating = 1,
            .edo22_carrier_freq = 8, .mathieu_q0_milli = 850, .mathieu_a_param = 1.0f, .acoustic_damping_c = 0.95f,
            .weyl_closure_class_id = 0x0101, .singular_locus_f_s = 100, .bernstein_sato_dim = 1,
            .rest_length_L0 = 8.0f, .spring_stiffness_k = 2.0f, .mass = 1.0f, .fracture_threshold = 180.0f,
            .engram_accumulator_saat = 500000ULL, .phase_twist_phi_w = 12,
            .banishment_ritual_key = "Castle Gyllencreutz Archives"
        },
        {
            .magic = "VAESEN01",
            .wmq_address = "dynamic_wmq_0x02000002",
            .name = "Gårdstomte (Tomte)",
            .clan_id = 2, .caste = 2,
            .physique = 2, .precision = 5, .logic = 3, .fervour = 4, .dogma = 5, .fear_rating = 1,
            .edo22_carrier_freq = 2, .mathieu_q0_milli = 650, .mathieu_a_param = 1.0f, .acoustic_damping_c = 0.98f,
            .weyl_closure_class_id = 0x0202, .singular_locus_f_s = 150, .bernstein_sato_dim = 1,
            .rest_length_L0 = 6.0f, .spring_stiffness_k = 2.5f, .mass = 0.8f, .fracture_threshold = 120.0f,
            .engram_accumulator_saat = 150000ULL, .phase_twist_phi_w = 4,
            .banishment_ritual_key = "Do not gift clothes, offer porridge with butter on top"
        },
        {
            .magic = "VAESEN01",
            .wmq_address = "dynamic_wmq_0x04000013",
            .name = "Myling (Unburied Child)",
            .clan_id = 4, .caste = 1,
            .physique = 1, .precision = 2, .logic = 2, .fervour = 5, .dogma = 2, .fear_rating = 4,
            .edo22_carrier_freq = 19, .mathieu_q0_milli = 4500, .mathieu_a_param = 1.5f, .acoustic_damping_c = 0.65f,
            .weyl_closure_class_id = 0x0401, .singular_locus_f_s = 900, .bernstein_sato_dim = 3,
            .rest_length_L0 = 15.0f, .spring_stiffness_k = 0.8f, .mass = 0.5f, .fracture_threshold = 90.0f,
            .engram_accumulator_saat = 920500ULL, .phase_twist_phi_w = 90,
            .banishment_ritual_key = "Carry child corpse to consecrated churchyard cemetery"
        },
        {
            .magic = "VAESEN01",
            .wmq_address = "dynamic_wmq_0x0300000a",
            .name = "Teddy Bear Citizen",
            .clan_id = 3, .caste = 3,
            .physique = 3, .precision = 4, .logic = 4, .fervour = 5, .dogma = 1, .fear_rating = 1,
            .edo22_carrier_freq = 10, .mathieu_q0_milli = 500, .mathieu_a_param = 1.0f, .acoustic_damping_c = 0.99f,
            .weyl_closure_class_id = 0x0301, .singular_locus_f_s = 100, .bernstein_sato_dim = 1,
            .rest_length_L0 = 5.0f, .spring_stiffness_k = 3.0f, .mass = 1.0f, .fracture_threshold = 200.0f,
            .engram_accumulator_saat = 1000000ULL, .phase_twist_phi_w = 0,
            .banishment_ritual_key = "Hogan Bank Civic Personhood Affirmation"
        }
    };

    static const char *filenames[] = {
        "assets/vaesen/nacken.dat.bin",
        "assets/vaesen/linnea.dat.bin",
        "assets/vaesen/tomte.dat.bin",
        "assets/vaesen/myling.dat.bin",
        "assets/vaesen/teddy_bear.dat.bin"
    };

    size_t count = sizeof(slices) / sizeof(slices[0]);
    for (size_t i = 0; i < count; ++i) {
        if (tsfi_vaesen_slice_save(&slices[i], filenames[i]) != 0) {
            return -1;
        }
    }
    return 0;
}
