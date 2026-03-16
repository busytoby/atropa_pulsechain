#ifndef TSFI_JPEG_ENCODER_H
#define TSFI_JPEG_ENCODER_H

#include <stdint.h>
#include <stddef.h>

/**
 * Encode RGB24 buffer to JPEG in memory.
 * Returns 0 on success, negative on failure.
 * @param out_buffer Output pointer (will be allocated by libjpeg, caller must free with free())
 * @param out_size Output size in bytes
 * @param in_pixels Input RGB24 pixels
 * @param width Image width
 * @param height Image height
 * @param quality JPEG quality (1-100)
 */
int tsfi_jpeg_encode(unsigned char **out_buffer, unsigned long *out_size, 
                     const uint8_t *in_pixels, int width, int height, int quality);

void tsfi_free_buffer(unsigned char *buffer);

#endif // TSFI_JPEG_ENCODER_H
