#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi2-deepseek/inc/tsfi_quadtree_ksds.h"
#include "tsfi2-deepseek/inc/tsfi_mainframe_computerworld.h"

#define MAX_STIMULI 54

typedef struct {
    char head_shape[16]; // narrow, round, wide
    char eye_shape[16];  // almond, rect, round
    char eye_size[16];   // small, medium, large
    char character[16];  // black, white
} stimuli_record_t;

// Procedurally generate all 54 stimuli combinations
int generate_stimuli_dataset(stimuli_record_t *dataset) {
    const char *heads[] = { "narrow", "round", "wide" };
    const char *eyes[] = { "almond", "rect", "round" };
    const char *sizes[] = { "small", "medium", "large" };
    const char *chars[] = { "black", "white" };
    
    int idx = 0;
    for (int h = 0; h < 3; h++) {
        for (int e = 0; e < 3; e++) {
            for (int s = 0; s < 3; s++) {
                for (int c = 0; c < 2; c++) {
                    strcpy(dataset[idx].head_shape, heads[h]);
                    strcpy(dataset[idx].eye_shape, eyes[e]);
                    strcpy(dataset[idx].eye_size, sizes[s]);
                    strcpy(dataset[idx].character, chars[c]);
                    idx++;
                }
            }
        }
    }
    return idx;
}

// Export a KSDS record directly to USD ASCII (.usda) schema format
void export_ksds_to_usd(const stimuli_record_t *rec, char *usd_out, size_t max_len) {
    double head_scale_x = strcmp(rec->head_shape, "narrow") == 0 ? 0.7 : (strcmp(rec->head_shape, "wide") == 0 ? 1.3 : 1.0);
    double eye_scale = strcmp(rec->eye_size, "large") == 0 ? 1.8 : (strcmp(rec->eye_size, "small") == 0 ? 0.6 : 1.0);
    
    snprintf(usd_out, max_len,
        "#usda 1.0\n\n"
        "def Xform \"teddy_bear_%s\" {\n"
        "    def Sphere \"head\" {\n"
        "        double3 xformOp:scale = (%0.2f, 1.0, 1.0)\n"
        "        uniform token[] xformOpOrder = [\"xformOp:scale\"]\n"
        "    }\n"
        "    def Sphere \"left_eye\" {\n"
        "        double3 xformOp:scale = (%0.2f, %0.2f, %0.2f)\n"
        "        double3 xformOp:translate = (-0.3, 0.4, 0.8)\n"
        "        uniform token[] xformOpOrder = [\"xformOp:translate\", \"xformOp:scale\"]\n"
        "    }\n"
        "}\n",
        rec->character, head_scale_x, eye_scale, eye_scale, eye_scale
    );
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT HATHITRUST MAINFRAME STIMULI RECORDING SUITE\n");
    printf("=============================================================\n");

    stimuli_record_t stimuli[MAX_STIMULI];
    int count = generate_stimuli_dataset(stimuli);
    assert(count == MAX_STIMULI);
    printf("[Stimuli] Procedurally generated %d combinations.\n", count);

    // Record the stimuli to the HathiTrust Primary KSDS database
    FILE *ht_pf = fopen("/tmp/ht_primary.dat.bin", "wb");
    assert(ht_pf != NULL);
    
    // Write copybook layout spacer header (128 bytes)
    uint8_t spacer[128] = {0};
    fwrite(spacer, 1, 128, ht_pf);
    
    uint32_t rec_offset = 128;
    for (int i = 0; i < count; i++) {
        char record_buf[256];
        int len = snprintf(record_buf, sizeof(record_buf), 
            "STIMULI_ID=%02d:HEAD=%s:EYE=%s:SIZE=%s:CHAR=%s\n", 
            i, stimuli[i].head_shape, stimuli[i].eye_shape, stimuli[i].eye_size, stimuli[i].character);
        fwrite(record_buf, 1, len, ht_pf);
    }
    fclose(ht_pf);
    printf("   ✓ Wrote %d stimuli records to HathiTrust KSDS primary database.\n", count);

    // Write index records (AIX ISBN quadtree mapping)
    InteropQuadNode aix_nodes[1] = {
        { 0, 0, 100, 100, rec_offset, { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF } }
    };
    bool aix_ok = tsfi_qt_ksds_aix_write(
        "/tmp/ht_aix_isbn.dat.bin",
        "AUNCIENT_AIX\nQuadtreeCount:\t1\nRecordCount:\t1",
        aix_nodes,
        1,
        &rec_offset,
        1
    );
    assert(aix_ok == true);
    printf("   ✓ Generated HathiTrust AIX quadtree index records.\n");

    // Test USD Dynamic Asset Generation from a collation index
    printf("[USD-GEN] Testing dynamic USD generation from KSDS Collation [Index 0]...\n");
    char usd_buffer[1024];
    export_ksds_to_usd(&stimuli[0], usd_buffer, sizeof(usd_buffer));
    assert(strstr(usd_buffer, "def Xform \"teddy_bear_black\"") != NULL);
    printf("   ✓ Compiled USDA metadata verified successfully:\n%s\n", usd_buffer);

    remove("/tmp/ht_primary.dat.bin");
    remove("/tmp/ht_aix_isbn.dat.bin");
    printf("=============================================================\n");
    printf("STIMULI RECORDING COMPLETED SUCCESSFULLY\n");
    printf("=============================================================\n");
    return 0;
}
