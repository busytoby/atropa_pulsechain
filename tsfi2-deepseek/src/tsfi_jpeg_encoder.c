#include <stdio.h>
#include <stdlib.h>
#include <jpeglib.h>
#include "tsfi_jpeg_encoder.h"

int tsfi_jpeg_encode(unsigned char **out_buffer, unsigned long *out_size, 
                     const uint8_t *in_pixels, int width, int height, int quality) {
    if (!out_buffer || !out_size || !in_pixels) return -1;

    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);

    // Set destination to memory buffer (libjpeg-turbo allocates)
    jpeg_mem_dest(&cinfo, out_buffer, out_size);

    cinfo.image_width = width;
    cinfo.image_height = height;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;

    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, quality, TRUE);

    jpeg_start_compress(&cinfo, TRUE);

    JSAMPROW row_pointer[1];
    int row_stride = width * 3;

    while (cinfo.next_scanline < cinfo.image_height) {
        row_pointer[0] = (JSAMPROW)&in_pixels[cinfo.next_scanline * row_stride];
        jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }

    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);

    return 0;
}

void tsfi_free_buffer(unsigned char *buffer) {
    if (buffer) free(buffer);
}
