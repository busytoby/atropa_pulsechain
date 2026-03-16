#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "lau_memory.h"
#include "tsfi_helmk0rn.h"
#include "tsfi_io.h"

// Verified Minimal Compute SPIR-V (local_size 16,16,1)
static const uint32_t verified_k0rn_blob[] = {
    0x07230203, 0x00010000, 0x000d000b, 0x00000006, 0x00000000, 0x0003000e, 
    0x00000000, 0x00000001, 0x0003000f, 0x00000000, 0x00000001, 0x00060010, 
    0x00000004, 0x00000011, 0x00000010, 0x00000010, 0x00000001, 0x00030003, 
    0x00000002, 0x000001c2, 0x00040005, 0x00000004, 0x6e69616d, 0x00000000, 
    0x00020013, 0x00000002, 0x00030021, 0x00000003, 0x00000002, 0x00050036, 
    0x00000002, 0x00000004, 0x00000000, 0x00000003, 0x000200f8, 0x00000005, 
    0x000100fd, 0x00010038
};

uint32_t* tsfi_build_helmk0rn_waveform(size_t *out_size) {
    size_t sz = sizeof(verified_k0rn_blob);
    uint32_t *wave = (uint32_t*)lau_malloc(sz);
    memcpy(wave, verified_k0rn_blob, sz);
    *out_size = sz;
    tsfi_io_printf(stdout, "[HelmK0Rn] Generated Resident Waveform (%zu bytes)\n", *out_size);
    return wave;
}
