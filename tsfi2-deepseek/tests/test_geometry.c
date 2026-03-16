#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_ottype.h"
#include "lau_memory.h"

// Helper to write big-endian values
static void write_u16(uint8_t *p, uint16_t v) {
    p[0] = (v >> 8) & 0xFF;
    p[1] = v & 0xFF;
}

static void write_u32(uint8_t *p, uint32_t v) {
    p[0] = (v >> 24) & 0xFF;
    p[1] = (v >> 16) & 0xFF;
    p[2] = (v >> 8) & 0xFF;
    p[3] = v & 0xFF;
}

static void write_i16(uint8_t *p, int16_t v) {
    write_u16(p, (uint16_t)v);
}

void test_geometry_simple() {
    printf("[TEST] Testing Geometry: Simple Glyph with implicit midpoints...\n");

    uint8_t buffer[1024];
    memset(buffer, 0, sizeof(buffer));

    // Simple glyph: 1 contour, 4 points
    // P0: (0,0) on-curve
    // P1: (100, 100) off-curve
    // P2: (200, 100) off-curve -> Implicit midpoint (150, 100) on-curve
    // P3: (300, 0) on-curve

    uint8_t *g = buffer + 32; 
    write_i16(g + 0, 1);   
    write_i16(g + 2, 0);   
    write_i16(g + 4, 0);   
    write_i16(g + 6, 300); 
    write_i16(g + 8, 100); 

    write_u16(g + 10, 3);  
    write_u16(g + 12, 0);  

    uint8_t *flags = g + 14;
    flags[0] = 0x01; 
    flags[1] = 0x00; 
    flags[2] = 0x00; 
    flags[3] = 0x01; 

    uint8_t *coords = g + 18;
    write_i16(coords + 0, 0);   
    write_i16(coords + 2, 100); 
    write_i16(coords + 4, 100); 
    write_i16(coords + 6, 100); 
    write_i16(coords + 8, 0);   
    write_i16(coords + 10, 100); 
    write_i16(coords + 12, 0);   
    write_i16(coords + 14, -100); 

    tsfi_ottype_t ot;
    memset(&ot, 0, sizeof(ot));
    ot.data = buffer;
    ot.size = sizeof(buffer);
    ot.glyf_secret = 32;
    ot.location_secret = 16;
    ot.location_format = 1; 
    ot.num_glyphs = 1;

    uint8_t *loca = buffer + 16;
    write_u32(loca + 0, 0); 
    write_u32(loca + 4, 100); 

    tsfi_glyph_geom_t geom;
    memset(&geom, 0, sizeof(geom));
    bool ok = tsfi_ottype_get_glyph_geom(&ot, 0, &geom);
    assert(ok);

    printf("  Found %d points, %d contours\n", geom.num_points, geom.num_contours);
    for (uint32_t i = 0; i < geom.num_points; i++) {
        printf("    P%d: (%.1f, %.1f) type=%d\n", i, geom.points[i].x, geom.points[i].y, geom.points[i].type);
    }

    // Midpoint expansion should result in 5 points
    assert(geom.num_points == 5);
    assert(geom.points[0].type == 1);
    assert(geom.points[1].type == 2);
    assert(geom.points[2].type == 1); // Expanded midpoint
    assert(geom.points[2].x == 150.0f);
    assert(geom.points[2].y == 100.0f);
    assert(geom.points[3].type == 2);
    assert(geom.points[4].type == 1);

    tsfi_ottype_free_geom(&geom);
}

void test_geometry_empty() {
    printf("[TEST] Testing Geometry: Empty Glyph...\n");
    uint8_t buffer[1024];
    memset(buffer, 0, sizeof(buffer));

    tsfi_ottype_t ot;
    memset(&ot, 0, sizeof(ot));
    ot.data = buffer;
    ot.size = sizeof(buffer);
    ot.glyf_secret = 32;
    ot.location_secret = 16;
    ot.location_format = 1;
    ot.num_glyphs = 1;
    
    uint8_t *loca = buffer + 16;
    write_u32(loca + 0, 0);
    write_u32(loca + 4, 0); 

    tsfi_glyph_geom_t geom;
    memset(&geom, 0, sizeof(geom));
    bool ok = tsfi_ottype_get_glyph_geom(&ot, 0, &geom);
    assert(ok);
    assert(geom.num_points == 0);
    assert(geom.num_contours == 0);
}

int main() {
    test_geometry_simple();
    test_geometry_empty();
    printf("[TEST] Geometry tests passed (verified midpoint expansion).\n");
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
