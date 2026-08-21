#ifndef TSFI_VAESEN_DEVICE_SLICE_H
#define TSFI_VAESEN_DEVICE_SLICE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define TSFI_VAESEN_SLICE_MAGIC "VAESEN01"
#define TSFI_VAESEN_SLICE_DIR   "assets/vaesen"

typedef struct __attribute__((packed)) {
    char     magic[12];               /* "VAESEN01" */
    char     wmq_address[48];         /* "dynamic_wmq_0x0200000b" */
    char     name[64];                /* "Näcken (The Nök)" */
    uint8_t  clan_id;                 /* 0..4 */
    uint8_t  caste;                   /* 1..5 */
    uint8_t  dna_fnv1a_hash[32];      /* Cryptographic hash */

    /* Six Core Attributes & Fear */
    uint8_t  physique;                /* 1..5 */
    uint8_t  precision;               /* 1..5 */
    uint8_t  logic;                   /* 1..5 */
    uint8_t  fervour;                 /* 1..5 */
    uint8_t  dogma;                   /* 1..5 */
    uint8_t  fear_rating;             /* 1..5 */

    /* Mathieu Resonance Standards */
    uint8_t  edo22_carrier_freq;      /* 1..22 */
    uint8_t  padding[1];
    uint16_t mathieu_q0_milli;        /* milli-units */
    float    mathieu_a_param;
    float    acoustic_damping_c;

    /* Weyl Closure Standards */
    uint32_t weyl_closure_class_id;
    uint32_t singular_locus_f_s;
    uint32_t bernstein_sato_dim;

    /* Verlet Mass-Spring Standards */
    float    rest_length_L0;
    float    spring_stiffness_k;
    float    mass;
    float    fracture_threshold;

    /* Psychic Trauma & Engram Ledger */
    uint64_t engram_accumulator_saat;
    uint32_t phase_twist_phi_w;
    char     banishment_ritual_key[64];
} TsfiVaesenDeviceSliceBin;

/* Individual Slice API */
int  tsfi_vaesen_slice_save(const TsfiVaesenDeviceSliceBin *slice, const char *filename);
int  tsfi_vaesen_slice_load(const char *filepath, TsfiVaesenDeviceSliceBin *out_slice);
int  tsfi_vaesen_slice_build_canonical_all(void);

/* Dual-Slice [WMQ:WMQ] Interaction Engine */
int  tsfi_vaesen_slice_interact(
    const TsfiVaesenDeviceSliceBin *slice_a,
    const TsfiVaesenDeviceSliceBin *slice_b,
    float *out_equilibrium_distance,
    uint16_t *out_pll_phase_deg,
    bool *out_is_locked,
    char *out_stanag_dialogue,
    size_t dialogue_buf_size
);

#endif /* TSFI_VAESEN_DEVICE_SLICE_H */
