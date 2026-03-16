#include "lau_thunk.h"
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

// Helper function called by assembly
// Writes raw RGBA frame to stdout (fd 1)
void tsfi_video_write_frame(void *pixels, size_t size) {
    // We write to stdout. The user is expected to pipe this to ffmpeg.
    // ffmpeg -f rawvideo -pixel_format rgba -video_size 800x600 -i - ...
    if (write(1, pixels, size) < 0) {
        perror("tsfi_video_write_frame: write failed");
    }
}
