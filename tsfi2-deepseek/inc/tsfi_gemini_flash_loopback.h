/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_GEMINI_FLASH_LOOPBACK_H
#define TSFI_GEMINI_FLASH_LOOPBACK_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	uint32_t client_id;
	uint32_t active_pasid;
	char endpoint_uri[128];
	bool is_connected;
} tsfi_gemini_flash_client_t;

bool tsfi_gemini_flash_loopback_init(tsfi_gemini_flash_client_t *client, uint32_t client_id, uint32_t pasid, const char *endpoint_uri);
bool tsfi_gemini_flash_loopback_query(tsfi_gemini_flash_client_t *client, const char *prompt, char *response_buf, size_t max_len);
bool tsfi_gemini_flash_loopback_close(tsfi_gemini_flash_client_t *client);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_GEMINI_FLASH_LOOPBACK_H */
