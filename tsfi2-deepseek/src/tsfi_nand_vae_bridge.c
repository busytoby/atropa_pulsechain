#include "tsfi_nand_trap_firmware.h"
#include "stable-diffusion.h"
#include "tsfi_safetensors_cache.h"
#include <string.h>
#include <math.h>

/**
 * @brief Zero-Copy VAE Mount
 * 
 * Logic: We trick the Safetensors cache into thinking the VAE is already
 * loaded in SHM, but we point the data directly to the physical payload
 * inside the active NandTrapState.
 */
void tsfi_nand_mount_vae_memory(NandTrapState *nand) {
    if (!nand || nand->vae_actual_size == 0) return;
    
    // We register the payload with our global cache so that when sd_cpp
    // asks for "taesd.safetensors", it gets this pointer instantly.
    // This requires a new API in tsfi_safetensors_cache.h: tsfi_safetensors_cache_inject
    tsfi_safetensors_cache_inject("assets/models/taesd.safetensors", nand->vae_payload, nand->vae_actual_size);
}

/**
 * @brief Establishes the VAE upon the NAND Trap state.
 * 
 * Logic: Maps the 1024 NAND granules (fibers) to the 32x32 VAE latent space.
 * This effectively "traps" the VAE's imagination within the physical structural
 * constraints of the NAND trap.
 */
void tsfi_establish_vae_on_nand(sd_sample_params_t *params, const NandTrapState *nand) {
    if (!params || !nand || !params->latent_input) return;

    // We have 1024 fibers. VAE Latent space at 256x256 is 32x32 = 1024 pixels.
    // Each latent pixel has 4 channels.
    float *latent = params->latent_input;
    
    for (int i = 0; i < 1024; i++) {
        // Map fiber intensity/weight to the 4 latent channels
        // Fiber X/Y/Z provide structural "DNA"
        float weight = nand->fibers[i].weight;
        float x_bias = nand->fibers[i].x / 256.0f;
        float y_bias = nand->fibers[i].y / 256.0f;
        float z_bias = nand->fibers[i].z / 256.0f;

        // Channel 0: Structural Mass
        latent[i * 4 + 0] = weight * cosf(nand->epoch);
        // Channel 1: "Blue" Secret (Fur High-Frequency)
        latent[i * 4 + 1] = x_bias * sinf(nand->epoch);
        // Channel 2: Chromatic Balance
        latent[i * 4 + 2] = y_bias;
        // Channel 3: Depth/Persistence
        latent[i * 4 + 3] = z_bias;
    }
}

/**
 * @brief Injects NAND external params into VAE Secret path
 */
float tsfi_nand_get_vae_secret(const NandTrapState *nand) {
    if (!nand) return 1.0f;
    // Use the first external param as the "Blue" secret for the TeddyResnetBlock
    return nand->external_params[0];
}
