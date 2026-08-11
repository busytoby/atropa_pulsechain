/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_XDC_CPM_BAAI_INSPECTOR_H
#define TSFI_XDC_CPM_BAAI_INSPECTOR_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_xdc_cpm_baai_inspect_registers(uint32_t pasid, uint64_t *chin_out, uint64_t *monopole_out, uint64_t *identity_out);
bool tsfi_xdc_cpm_baai_modulate_trail(uint64_t chin, uint64_t monopole, float *opacity_out);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_XDC_CPM_BAAI_INSPECTOR_H */
