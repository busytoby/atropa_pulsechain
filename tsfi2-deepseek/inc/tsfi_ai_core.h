#ifndef TSFI_AI_CORE_H
#define TSFI_AI_CORE_H

#include <stddef.h>

/**
 * Fetch a 256x256 photorealistic image from local Stable Diffusion (127.0.0.1:8080)
 * @param prompt Generation prompt
 * @param out_b64 Pointer to hold base64 image (must be freed by caller)
 * @param out_len Length of the returned base64 string
 * @return 0 on success, negative on error.
 */
int tsfi_ai_fetch_sd(const char *prompt, unsigned char **out_b64, size_t *out_len);

/**
 * Query local Ollama VLM (127.0.0.1:11434) to evaluate an image.
 * @param b64_img The base64 encoded image
 * @param prompt The prompt to ask the VLM
 * @param output Buffer to store the response
 * @param out_max Size of output buffer
 * @return 0 on success, negative on error.
 */
int tsfi_ai_evaluate_vlm(const char *b64_img, const char *prompt, char *output, size_t out_max);

// Native SD API (Thunked)
void* tsfi_sd_init(const char* model_path, const char* vae_path, const char* control_net_path);
int tsfi_sd_generate(void* ctx_ptr, const char* prompt, uint8_t* out_pixels, int width, int height, int steps, float cfg, int seed);
void tsfi_sd_free(void* ctx_ptr);

// Direct TAESD Latent Decoding (Stall-Free)
int tsfi_taesd_decode(void* sd_ctx_ptr, float* latents, uint8_t* out_pixels);

#endif // TSFI_AI_CORE_H
