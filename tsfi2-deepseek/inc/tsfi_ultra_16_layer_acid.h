/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_ULTRA_16_LAYER_ACID_H
#define TSFI_ULTRA_16_LAYER_ACID_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	uint32_t session_id;
	uint32_t active_pasid;
	uint64_t acid_16_latch;
	bool acid_16_verified;
} tsfi_ultra_16_acid_ctx_t;

bool tsfi_ultra_16_layer_acid_init(tsfi_ultra_16_acid_ctx_t *ctx, uint32_t session_id, uint32_t pasid);
bool tsfi_ultra_16_layer_acid_eval(tsfi_ultra_16_acid_ctx_t *ctx, uint64_t *acid_16_latch_out);
bool tsfi_ultra_16_layer_acid_close(tsfi_ultra_16_acid_ctx_t *ctx);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_ULTRA_16_LAYER_ACID_H */
