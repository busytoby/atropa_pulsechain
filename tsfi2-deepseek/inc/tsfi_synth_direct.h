#ifndef TSFI_SYNTH_DIRECT_H
#define TSFI_SYNTH_DIRECT_H

#include "tsfi_synth_akb.h"

// Read key value directly from the cache buffer, bypassing disk DAT lookups
const char* tsfi_synth_direct_read(const TSFiSynthAKB *synth, const char *key);

// Read graph edge weight directly from the AKB adjacency matrix
float tsfi_synth_direct_get_edge(const TSFiSynthAKB *synth, const char *src, const char *dest);

#endif // TSFI_SYNTH_DIRECT_H
