#ifndef TSFI_MIDAS_C_H
#define TSFI_MIDAS_C_H

#include <stdint.h>
#include <stddef.h>

// A conceptual C interface for a lightweight C/C++ port of MiDaS.
// This defines the necessary bindings if we were to compile the
// ONNX or NCNN version of MiDaS directly into the TSFi binary.

#ifdef __cplusplus
extern "C" {
#endif

// Opaque handle for the MiDaS inference context
typedef struct TsfiMidasContext TsfiMidasContext;

// Initialize the MiDaS context using an ONNX/NCNN model file.
// Returns NULL on failure.
TsfiMidasContext* tsfi_midas_init(const char* model_path, int use_vulkan);

// Perform monocular depth estimation on a raw RGB frame.
// input_rgb: Pointer to a flat 24-bit RGB array (Width * Height * 3 bytes)
// width: Image width (typically 512)
// height: Image height (typically 512)
// out_depth: Pre-allocated buffer for the 8-bit depth map (Width * Height bytes)
// Returns 0 on success, non-zero on error.
int tsfi_midas_estimate_depth(TsfiMidasContext* ctx, const uint8_t* input_rgb, int width, int height, uint8_t* out_depth);

// Free the MiDaS context
void tsfi_midas_free(TsfiMidasContext* ctx);

#ifdef __cplusplus
}
#endif

#endif // TSFI_MIDAS_C_H
