#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "tsfi_nand_trap_firmware.h"

int main() {
    uint8_t *pixels = (uint8_t*)malloc(256 * 256 * 3);
    if (!pixels) return 1;

    for (int frame = 0; frame < 5; frame++) {
        generate_nand_trap_frame(pixels, frame * 10, 0);
        
        char filename[64];
        sprintf(filename, "tmp/tesseract_frame_%d.ppm", frame);
        FILE *f = fopen(filename, "wb");
        if (!f) continue;
        
        fprintf(f, "P6\n256 256\n255\n");
        fwrite(pixels, 1, 256 * 256 * 3, f);
        fclose(f);
        printf("Rendered 4D slice to %s\n", filename);
    }

    free(pixels);
    return 0;
}
