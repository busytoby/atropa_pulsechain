#ifndef TSFI_HELMK0RN_H
#define TSFI_HELMK0RN_H

#include <stdint.h>
#include <stddef.h>

// Builds the K0Rn Compute Waveform in memory
// Returns a pointer to the HelmK0Rn binary. Caller must free with lau_free().
uint32_t* tsfi_build_helmk0rn_waveform(size_t *out_size);

#endif
