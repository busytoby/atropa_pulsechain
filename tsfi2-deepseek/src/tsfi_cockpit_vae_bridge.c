#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <immintrin.h>
#include "tsfi_vae_firmware.h"
#include "tsfi_nand_trap_firmware.h"
#include "tsfi_vessel_anchor.h"
#include "tsfi_controlnet_shm.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s <output_bear.raw>\n", argv[0]);
        return 1;
    }

    const char *output_path = argv[1];

    // 1. Initialize Baseline Nand Trap (Sovereign/Atropa anchor)
    NandTrapState nand;
    memset(&nand, 0, sizeof(NandTrapState));
    tsfi_vessel_anchor_inject(&nand, 0x999); // 0.999 resonance (Atropa)

    // 2. Attach to Zero-Copy Latent SHM
    const TsfiLatentMap *shm_latent = tsfi_latent_shm_attach();
    if (!shm_latent) {
        fprintf(stderr, "[FRACTURE] Failed to attach to Latent SHM. Is SD Worker running?\n");
        return 1;
    }

    // 3. Inject Latents into Nand Fibers (Modulation)
    // We map the 1024 latents (32x32) to our 1024 fibers
    for (int i = 0; i < 1024; i++) {
        nand.fibers[i].weight = shm_latent->data[i * 4 + 0]; // Ch0: Mass
        nand.fibers[i].x = shm_latent->data[i * 4 + 1];      // Ch1: Blue Secret/Fur
        nand.fibers[i].y = shm_latent->data[i * 4 + 2];      // Ch2: Chromatic
        nand.fibers[i].z = shm_latent->data[i * 4 + 3];      // Ch3: Depth
    }

    tsfi_latent_shm_detach(shm_latent);

    // 4. Run Cockpit VAE Firmware
    LauVaeFirmware_State vae;
    tsfi_vae_firmware_boot(&vae, &nand);
    
    vae.decoding_active = true;
    printf("[INFO] Executing Cockpit VAE Bridge (sequential eval)...\n");
    
    for (int i = 0; i < 256; i++) {
        LauVaeFirmware_eval_sequential(&vae);
    }

    // 5. Export to Teddy Bear Planet (Raw RGB pixels)
    FILE *of = fopen(output_path, "wb");
    if (!of) { perror("fopen output"); return 1; }
    
    for (int i = 0; i < 256; i++) {
        float pixels[16];
        _mm512_storeu_ps(pixels, (__m512)vae.wrf[i]);
        
        // Convert the 4-pixel block to RGB
        for (int p = 0; p < 4; p++) {
            uint8_t rgb[3];
            rgb[0] = (uint8_t)(pixels[p*4 + 0] * 255.0f); // Mass -> R
            rgb[1] = (uint8_t)(pixels[p*4 + 1] * 255.0f); // Fur -> G
            rgb[2] = (uint8_t)(pixels[p*4 + 2] * 255.0f); // Chromatic -> B
            fwrite(rgb, 1, 3, of);
        }
    }
    fclose(of);

    printf("[INFO] Teddy Bear Planet Image Exported to %s\n", output_path);

    return 0;
}
