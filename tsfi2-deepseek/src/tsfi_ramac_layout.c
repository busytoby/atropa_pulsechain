#include "tsfi_ramac_layout.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

tsfi_ramac_chs tsfi_ramac_index_to_chs(int index) {
    tsfi_ramac_chs chs;
    int words_per_sector = RAMAC_WORDS;
    int words_per_track = RAMAC_SECTORS * words_per_sector;
    int words_per_cylinder = RAMAC_HEADS * words_per_track;

    chs.cylinder = index / words_per_cylinder;
    int rem = index % words_per_cylinder;
    chs.head = rem / words_per_track;
    rem = rem % words_per_track;
    chs.sector = rem / words_per_sector;
    chs.word_offset = rem % words_per_sector;

    return chs;
}

int tsfi_ramac_chs_to_index(tsfi_ramac_chs chs) {
    int words_per_sector = RAMAC_WORDS;
    int words_per_track = RAMAC_SECTORS * words_per_sector;
    int words_per_cylinder = RAMAC_HEADS * words_per_track;

    return chs.cylinder * words_per_cylinder +
           chs.head * words_per_track +
           chs.sector * words_per_sector +
           chs.word_offset;
}

double tsfi_ramac_calculate_seek(int from_index, int to_index) {
    tsfi_ramac_chs c1 = tsfi_ramac_index_to_chs(from_index);
    tsfi_ramac_chs c2 = tsfi_ramac_index_to_chs(to_index);

    // Physical model of IBM 305 RAMAC movement:
    // 1. Horizontal seek: Cylinder-to-cylinder arm movement.
    //    Average cylinder-to-cylinder seek: ~1.5 milliseconds per cylinder.
    double cylinder_seek = abs(c1.cylinder - c2.cylinder) * 1.5;

    // 2. Vertical seek: Disk surface head swap (if head index changes).
    //    Average head swap time: ~0.8 milliseconds.
    double head_swap = (c1.head != c2.head) ? 0.8 : 0.0;

    // 3. Rotational latency: Sector-to-sector delay.
    //    Assuming 600 RPM (100 ms per full rotation of 20 sectors).
    //    Rotational delay = 5.0 ms per sector distance.
    double rotational_delay = abs(c1.sector - c2.sector) * 5.0;

    return (cylinder_seek + head_swap + rotational_delay) * 1000.0; // Return in microseconds
}

int tsfi_ramac_layout_optimize(tsfi_dat *dat, const char *filepath) {
    if (!dat || !filepath) return -1;

    // Open target file for writing the RAMAC-optimized dat layout
    FILE *fp = fopen(filepath, "wb");
    if (!fp) return -1;

    // Write magic signature
    fwrite("RMAC", 1, 4, fp);

    // Calculate total layout words and size
    int capacity = dat->capacity;
    fwrite(&capacity, sizeof(int), 1, fp);

    // Write base and check arrays clustered as aligned structures
    fwrite(dat->base, sizeof(int), capacity, fp);
    fwrite(dat->check, sizeof(int), capacity, fp);

    fclose(fp);
    return 0;
}
