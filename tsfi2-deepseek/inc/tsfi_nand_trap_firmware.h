#ifndef TSFI_NAND_TRAP_FIRMWARE_H
#define TSFI_NAND_TRAP_FIRMWARE_H

#include <stdint.h>

#define WIDTH 512
#define HEIGHT 512
#define GRANS 1024

#define NAND_TRAP_MAGIC 0x54534649 // "TSFI"
#define NAND_TRAP_VERSION 2
#define NAND_TRAP_VAE_SIZE (5 * 1024 * 1024) // 5MB reserved for VAE Weights

typedef struct {
    float x, y, z;
    float weight;
} TsfiGran;

typedef struct {
    uint32_t magic;
    uint32_t version;
    TsfiGran fibers[GRANS];
    float epoch;
    int genome; 
    float external_params[16]; // VLM-driven mutations
    
    // The Hardware VAE: Physically embedded into the firmware state
    uint32_t vae_actual_size;
    uint8_t vae_payload[NAND_TRAP_VAE_SIZE] __attribute__((aligned(512)));
} NandTrapState;

// Rendering API
void generate_nand_trap_frame(uint8_t* out_pixels, int frame, int genome, const uint8_t* masterpiece_rgb);
void extract_silhouette(const uint8_t* in_pixels, uint8_t* out_silhouette, int w, int h);

// Ricci Discovery API (Cornu Spiral)
float tsfi_nand_trap_cornu_c(float u);
float tsfi_nand_trap_cornu_s(float u);

// Ipomoea Lifecycle
void tsfi_nand_ipomoea_lifecycle(NandTrapState *nand, float t);

// Causal Discovery API (FROM -> TO Mapping)
void tsfi_nand_trap_causal_discovery(const NandTrapState *from_state, NandTrapState *to_state, float lti_factor);

// Autonomous Session API
int tsfi_nand_trap_autonomous_step(const char *storage_path, float lti_factor);

// Persistence API
int tsfi_nand_trap_save(const char *path, const NandTrapState *state);
int tsfi_nand_trap_restore(const char *path, NandTrapState *state);

typedef struct {
    uint64_t wavelet_id;
    uint64_t monopole_identity; // Acoustic proof anchor
    
    // Genie's Witness
    char genie_poetic_witness[256];
    float puppet_fitness;
    uint32_t classification_id;
    
    // Spider's Witness
    uint64_t trait_signature;
    uint32_t kinematic_joint_hash;
    bool xor_solidified;
    
    uint64_t witness_ts_ns;
} TSFiVisualWitness;

// Wavelet-NAND Association
typedef struct TsfiWavelet TsfiWavelet;
void tsfi_wavelet_nand_signal(void *manifold, const TsfiWavelet *W_Prov, const TsfiWavelet *W_Pot, uint64_t resonance_k);
void tsfi_wavelet_xor_signal(void *manifold, TsfiWavelet *W_Target, const TsfiWavelet *W_G, const TsfiWavelet *W_D, const uint8_t *seed_512b);

// Witness Generation
void tsfi_witness_establish(TSFiVisualWitness *out, const TsfiWavelet *W, const char *observation, float fitness, uint32_t class_id);

// Spider Taste Audit (LLM Integration)
bool tsfi_alligator_audit_spider_taste(const TsfiWavelet *W_Target);

// Firmware Level VAE Embedding
int tsfi_nand_trap_embed_vae(NandTrapState *state, const char *vae_path);

// Image Morphing API (AVX-512 Optimized)
void tsfi_image_blend(uint8_t* out, const uint8_t* a, const uint8_t* b, int w, int h, float alpha);
void tsfi_image_alpha_composite(uint8_t* out, const uint8_t* bg, const uint8_t* fg, const uint8_t* alpha_mask, int w, int h);

#endif // TSFI_NAND_TRAP_FIRMWARE_H
