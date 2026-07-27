#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdint.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#pragma pack(push, 1)
typedef struct {
    float normal[3];
    float vertices[3][3];
    uint16_t attribute_byte_count;
} StlFacet;
#pragma pack(pop)

static void write_stl_header(FILE *f, uint32_t facet_count) {
    char header[80] = "Auncient Volumetric C Teddy Bear STL Generator";
    fwrite(header, 1, 80, f);
    fwrite(&facet_count, sizeof(uint32_t), 1, f);
}

static void add_ellipsoid(StlFacet **facets, uint32_t *count, uint32_t *capacity,
                          float rx, float ry, float rz, float cx, float cy, float cz,
                          int lats, int lons) {
    for (int i = 0; i < lats; i++) {
        float lat0 = M_PI * (-0.5f + (float)i / lats);
        float lat1 = M_PI * (-0.5f + (float)(i + 1) / lats);
        
        float z0 = rz * sinf(lat0);
        float r0 = cosf(lat0);
        
        float z1 = rz * sinf(lat1);
        float r1 = cosf(lat1);
        
        for (int j = 0; j < lons; j++) {
            float lon0 = 2.0f * M_PI * ((float)j / lons);
            float lon1 = 2.0f * M_PI * ((float)(j + 1) / lons);
            
            float x00 = cx + rx * r0 * cosf(lon0);
            float y00 = cy + ry * r0 * sinf(lon0);
            float z00 = cz + z0;
            
            float x10 = cx + rx * r1 * cosf(lon0);
            float y10 = cy + ry * r1 * sinf(lon0);
            float z10 = cz + z1;
            
            float x01 = cx + rx * r0 * cosf(lon1);
            float y01 = cy + ry * r0 * sinf(lon1);
            float z01 = cz + z0;
            
            float x11 = cx + rx * r1 * cosf(lon1);
            float y11 = cy + ry * r1 * sinf(lon1);
            float z11 = cz + z1;
            
            // Allocate more space if needed
            if (*count + 2 >= *capacity) {
                *capacity *= 2;
                *facets = realloc(*facets, sizeof(StlFacet) * (*capacity));
            }
            
            // Triangle 1
            StlFacet *f1 = &(*facets)[*count];
            memset(f1, 0, sizeof(StlFacet));
            f1->normal[2] = 1.0f;
            f1->vertices[0][0] = x00; f1->vertices[0][1] = y00; f1->vertices[0][2] = z00;
            f1->vertices[1][0] = x10; f1->vertices[1][1] = y10; f1->vertices[1][2] = z10;
            f1->vertices[2][0] = x01; f1->vertices[2][1] = y01; f1->vertices[2][2] = z01;
            (*count)++;
            
            // Triangle 2
            StlFacet *f2 = &(*facets)[*count];
            memset(f2, 0, sizeof(StlFacet));
            f2->normal[2] = 1.0f;
            f2->vertices[0][0] = x01; f2->vertices[0][1] = y01; f2->vertices[0][2] = z01;
            f2->vertices[1][0] = x10; f2->vertices[1][1] = y10; f2->vertices[1][2] = z10;
            f2->vertices[2][0] = x11; f2->vertices[2][1] = y11; f2->vertices[2][2] = z11;
            (*count)++;
        }
    }
}

int main(void) {
    // 1. Generate Head mesh with round ears and muzzle snout welded in C
    uint32_t head_cap = 2048;
    uint32_t head_count = 0;
    StlFacet *head_facets = malloc(sizeof(StlFacet) * head_cap);
    
    // Main Head
    add_ellipsoid(&head_facets, &head_count, &head_cap, 0.45f, 0.45f, 0.45f, 0.0f, 0.0f, 0.0f, 16, 16);
    // Snout
    add_ellipsoid(&head_facets, &head_count, &head_cap, 0.20f, 0.15f, 0.20f, 0.0f, -0.1f, 0.35f, 12, 12);
    // Left Ear
    add_ellipsoid(&head_facets, &head_count, &head_cap, 0.15f, 0.15f, 0.08f, -0.3f, 0.35f, 0.0f, 12, 12);
    // Right Ear
    add_ellipsoid(&head_facets, &head_count, &head_cap, 0.15f, 0.15f, 0.08f, 0.3f, 0.35f, 0.0f, 12, 12);
    
    FILE *fh = fopen("tsfi2-deepseek/assets/toy_bear_head.stl", "wb");
    if (fh) {
        write_stl_header(fh, head_count);
        fwrite(head_facets, sizeof(StlFacet), head_count, fh);
        fclose(fh);
    }
    free(head_facets);

    // 2. Generate Torso (pear body) mesh
    uint32_t torso_cap = 1024;
    uint32_t torso_count = 0;
    StlFacet *torso_facets = malloc(sizeof(StlFacet) * torso_cap);
    
    // Generate pear body: wider at bottom than top
    for (int i = 0; i < 16; i++) {
        float lat0 = M_PI * (-0.5f + (float)i / 16);
        float lat1 = M_PI * (-0.5f + (float)(i + 1) / 16);
        
        float y_pct0 = (-0.5f + (float)i / 16) * 1.2f;
        float y_pct1 = (-0.5f + (float)(i + 1) / 16) * 1.2f;
        
        float w0 = 0.55f * (1.0f - 0.3f * (y_pct0 + 0.6f));
        float w1 = 0.55f * (1.0f - 0.3f * (y_pct1 + 0.6f));
        
        float r0 = cosf(lat0);
        float r1 = cosf(lat1);
        
        for (int j = 0; j < 16; j++) {
            float lon0 = 2.0f * M_PI * ((float)j / 16);
            float lon1 = 2.0f * M_PI * ((float)(j + 1) / 16);
            
            float x00 = w0 * r0 * cosf(lon0);
            float y00 = y_pct0;
            float z00 = w0 * r0 * sinf(lon0);
            
            float x10 = w1 * r1 * cosf(lon0);
            float y10 = y_pct1;
            float z10 = w1 * r1 * sinf(lon0);
            
            float x01 = w0 * r0 * cosf(lon1);
            float y01 = y_pct0;
            float z01 = w0 * r0 * sinf(lon1);
            
            float x11 = w1 * r1 * cosf(lon1);
            float y11 = y_pct1;
            float z11 = w1 * r1 * sinf(lon1);
            
            if (torso_count + 2 >= torso_cap) {
                torso_cap *= 2;
                torso_facets = realloc(torso_facets, sizeof(StlFacet) * torso_cap);
            }
            
            StlFacet *f1 = &torso_facets[torso_count++];
            memset(f1, 0, sizeof(StlFacet));
            f1->normal[2] = 1.0f;
            f1->vertices[0][0] = x00; f1->vertices[0][1] = y00; f1->vertices[0][2] = z00;
            f1->vertices[1][0] = x10; f1->vertices[1][1] = y10; f1->vertices[1][2] = z10;
            f1->vertices[2][0] = x01; f1->vertices[2][1] = y01; f1->vertices[2][2] = z01;
            
            StlFacet *f2 = &torso_facets[torso_count++];
            memset(f2, 0, sizeof(StlFacet));
            f2->normal[2] = 1.0f;
            f2->vertices[0][0] = x01; f2->vertices[0][1] = y01; f2->vertices[0][2] = z01;
            f2->vertices[1][0] = x10; f2->vertices[1][1] = y10; f2->vertices[1][2] = z10;
            f2->vertices[2][0] = x11; f2->vertices[2][1] = y11; f2->vertices[2][2] = z11;
        }
    }
    
    FILE *ft = fopen("tsfi2-deepseek/assets/toy_bear_torso.stl", "wb");
    if (ft) {
        write_stl_header(ft, torso_count);
        fwrite(torso_facets, sizeof(StlFacet), torso_count, ft);
        fclose(ft);
    }
    free(torso_facets);

    // 3. Generate Chubby Limbs
    uint32_t limb_cap = 1024;
    uint32_t limb_count = 0;
    StlFacet *limb_facets = malloc(sizeof(StlFacet) * limb_cap);
    
    // Fat cylinder
    for (int i = 0; i < 16; i++) {
        float y0 = -0.5f + (float)i * 0.7f / 16;
        float y1 = -0.5f + (float)(i + 1) * 0.7f / 16;
        
        float r0 = 0.22f;
        float r1 = 0.22f;
        
        for (int j = 0; j < 16; j++) {
            float lon0 = 2.0f * M_PI * ((float)j / 16);
            float lon1 = 2.0f * M_PI * ((float)(j + 1) / 16);
            
            float x00 = r0 * cosf(lon0);
            float y00 = y0;
            float z00 = r0 * sinf(lon0);
            
            float x10 = r1 * cosf(lon0);
            float y10 = y1;
            float z10 = r1 * sinf(lon0);
            
            float x01 = r0 * cosf(lon1);
            float y01 = y0;
            float z01 = r0 * sinf(lon1);
            
            float x11 = r1 * cosf(lon1);
            float y11 = y1;
            float z11 = r1 * sinf(lon1);
            
            if (limb_count + 2 >= limb_cap) {
                limb_cap *= 2;
                limb_facets = realloc(limb_facets, sizeof(StlFacet) * limb_cap);
            }
            
            StlFacet *f1 = &limb_facets[limb_count++];
            memset(f1, 0, sizeof(StlFacet));
            f1->normal[2] = 1.0f;
            f1->vertices[0][0] = x00; f1->vertices[0][1] = y00; f1->vertices[0][2] = z00;
            f1->vertices[1][0] = x10; f1->vertices[1][1] = y10; f1->vertices[1][2] = z10;
            f1->vertices[2][0] = x01; f1->vertices[2][1] = y01; f1->vertices[2][2] = z01;
            
            StlFacet *f2 = &limb_facets[limb_count++];
            memset(f2, 0, sizeof(StlFacet));
            f2->normal[2] = 1.0f;
            f2->vertices[0][0] = x01; f2->vertices[0][1] = y01; f2->vertices[0][2] = z01;
            f2->vertices[1][0] = x10; f2->vertices[1][1] = y10; f2->vertices[1][2] = z10;
            f2->vertices[2][0] = x11; f2->vertices[2][1] = y11; f2->vertices[2][2] = z11;
        }
    }
    
    // Rounded paw cap
    add_ellipsoid(&limb_facets, &limb_count, &limb_cap, 0.22f, 0.15f, 0.22f, 0.0f, -0.5f, 0.0f, 12, 12);
    
    FILE *fl = fopen("tsfi2-deepseek/assets/toy_bear_joint.stl", "wb");
    if (fl) {
        write_stl_header(fl, limb_count);
        fwrite(limb_facets, sizeof(StlFacet), limb_count, fl);
        fclose(fl);
    }
    free(limb_facets);

    printf("[C_GEN SUCCESS] Volumetric STL files generated successfully.\n");
    return 0;
}
