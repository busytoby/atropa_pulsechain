#ifndef TSFI_PARC_STCOMP_H
#define TSFI_PARC_STCOMP_H

#include <stdint.h>

/* Compile a simple Smalltalk method string (e.g. "load 0; const 5; add; return") into raw VM bytecodes */
int tsfi_parc_st_compile(const char *source, uint8_t *bytecodes_out, int max_len);

#endif // TSFI_PARC_STCOMP_H
