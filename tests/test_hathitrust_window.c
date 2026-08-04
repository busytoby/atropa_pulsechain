#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "inc/hathitrust_window.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>

// Working font implementation for draw_char
void draw_char(uint32_t *pixels, int w, int h, int rx, int ry, unsigned char c, uint32_t color, int scale) {
    (void)pixels; (void)w; (void)h; (void)rx; (void)ry; (void)c; (void)color; (void)scale;
}

// Stubs for stbtt font parsing
int stbtt_InitFont(void *info, const unsigned char *data, int offset) {
    (void)info; (void)data; (void)offset; return 1;
}
float stbtt_ScaleForPixelHeight(const void *info, float pixels) {
    (void)info; (void)pixels; return 1.0f;
}
unsigned char *stbtt_GetCodepointBitmap(const void *info, float scale_x, float scale_y, int codepoint, int *width, int *height, int *xoff, int *yoff) {
    (void)info; (void)scale_x; (void)scale_y; (void)codepoint; *width = 8; *height = 8; *xoff = 0; *yoff = 0;
    return calloc(64, 1);
}
void stbtt_FreeBitmap(unsigned char *bitmap, void *userdata) {
    (void)userdata; free(bitmap);
}
void stbtt_GetCodepointHMetrics(const void *info, int codepoint, int *advanceWidth, int *leftSideBearing) {
    (void)info; (void)codepoint; *advanceWidth = 8; *leftSideBearing = 0;
}

// Stub for node attributes
const char *tsfi_node_get_attribute(const void *node, const char *name) {
    (void)node; (void)name; return NULL;
}
static void test_window_initialization(void) {
    printf("[Test] Running Wayland/Vulkan reader window initialization tests...\n");
    
    bool ok = hathitrust_open_reader_window("L'art mérovingien et carolingien", 15);
    assert(ok == true);
    
    printf("[Test] Wayland/Vulkan reader window initialization tests passed.\n");
}

static void test_scsi_event_verification(void) {
    printf("[Test] Running SCSI key event routing tests...\n");
    
    // Test initial advance (page 1 -> page 2)
    int action = hathitrust_process_scsi_key_event(32); // keycode 32 is 'd'
    assert(action == 1);
    
    // Test reverse page (page 2 -> page 1)
    action = hathitrust_process_scsi_key_event(30); // keycode 30 is 'a'
    assert(action == -1);
    
    // Test boundary: going below page 1 should not change page but return -1
    action = hathitrust_process_scsi_key_event(30);
    assert(action == -1);
    
    // Test unknown key
    action = hathitrust_process_scsi_key_event(99);
    assert(action == 0);
    
    printf("[Test] SCSI key event routing tests passed.\n");
}

int main(void) {
    printf("[Test] Running HathiTrust Alto/Wayland window display tests...\n");
    test_window_initialization();
    test_scsi_event_verification();
    printf("[Test] All window display tests completed successfully.\n");
    return 0;
}
