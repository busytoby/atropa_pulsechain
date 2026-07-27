#define _GNU_SOURCE
#include "tsfi_dom.h"
#include "tsfi_layout.h"
#include "tsfi_paint.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define W 200
#define H 100

// Helper to count pixels matching a specific color
static int count_pixels(const uint32_t *pixels, int len, uint32_t target) {
    int count = 0;
    for (int i = 0; i < len; i++) {
        if (pixels[i] == target) count++;
    }
    return count;
}

static void test_color_parsing_rules(void) {
    printf("[Test] Running color parsing unit tests...\n");
    // Verify custom background name parsing & hex case sensitivity
    const char *html = 
        "<div style=\"width: 10px; height: 10px; background-color: #fF00Ff;\">"
        "  <div style=\"width: 5px; height: 5px; background: Purple;\"></div>"
        "</div>";

    TsfiNode *doc = tsfi_dom_parse_html(html);
    TsfiLayoutBox *root_box = tsfi_layout_tree_build(doc);
    assert(root_box != NULL);

    TsfiLayoutBox *div = root_box->first_child;
    tsfi_layout_solve(div, 0.0f, 0.0f, W, H);

    uint32_t *pixels = (uint32_t*)calloc(W * H, sizeof(uint32_t));
    tsfi_paint_tree(div, pixels, W, H);

    printf("div position: x=%.1f, y=%.1f, w=%.1f, h=%.1f\n", div->x, div->y, div->w, div->h);
    printf("pixels[0] = 0x%08X\n", pixels[0 * W + 0]);
    printf("pixels[1 * W + 1] = 0x%08X\n", pixels[1 * W + 1]);

    // Parent box should paint Fuchsia/Magenta (0xFFFF00FF) at (6, 0) since child overlaps (0,0)
    assert(pixels[0 * W + 6] == 0xFFFF00FF);

    // Child box should paint Purple (0xFF800080) at (1, 1)
    assert(pixels[1 * W + 1] == 0xFF800080);

    free(pixels);
    tsfi_layout_box_free(root_box);
    tsfi_node_free(doc);
    printf("[Test] Color parsing unit tests passed!\n");
}

static void test_bounds_clamping_safety(void) {
    printf("[Test] Running bounds clamping safety tests...\n");
    // Test a block box positioned half outside the target buffer width
    const char *html = "<div style=\"width: 300px; height: 50px; background-color: blue;\"></div>";
    TsfiNode *doc = tsfi_dom_parse_html(html);
    TsfiLayoutBox *root_box = tsfi_layout_tree_build(doc);

    TsfiLayoutBox *div = root_box->first_child;
    tsfi_layout_solve(div, 50.0f, 0.0f, W, H); // starting x at 50, w is 300 (extends to 350, buffer width is 200)

    uint32_t *pixels = (uint32_t*)calloc(W * H, sizeof(uint32_t));
    
    // This call should run safely without out-of-bounds segfaults
    tsfi_paint_tree(div, pixels, W, H);

    // Pixels inside buffer bounds (x from 50 to 199) should be blue
    assert(pixels[0 * W + 50] == 0xFF0000FF);
    assert(pixels[0 * W + 199] == 0xFF0000FF);

    free(pixels);
    tsfi_layout_box_free(root_box);
    tsfi_node_free(doc);
    printf("[Test] Bounds clamping safety tests passed!\n");
}

static void test_text_color_inheritance(void) {
    printf("[Test] Running text color inheritance tests...\n");
    const char *html = 
        "<div style=\"width: 100px; height: 50px; color: green;\">"
        "  <span>Nested Text</span>"
        "</div>";

    TsfiNode *doc = tsfi_dom_parse_html(html);
    TsfiLayoutBox *root_box = tsfi_layout_tree_build(doc);
    assert(root_box != NULL);

    TsfiLayoutBox *div = root_box->first_child;
    tsfi_layout_solve(div, 0.0f, 0.0f, W, H);

    uint32_t *pixels = (uint32_t*)calloc(W * H, sizeof(uint32_t));
    tsfi_paint_tree(div, pixels, W, H);

    // Count pixels that were drawn. They should inherit the green color (0xFF00FF00)
    int green_pixels = count_pixels(pixels, W * H, 0xFF00FF00);
    assert(green_pixels > 0);

    free(pixels);
    tsfi_layout_box_free(root_box);
    tsfi_node_free(doc);
    printf("[Test] Text color inheritance tests passed!\n");
}

int main(void) {
    printf("=== Starting Phase 3 Paint Compositor Unit Tests ===\n");
    test_color_parsing_rules();
    test_bounds_clamping_safety();
    test_text_color_inheritance();
    printf("=== All Phase 3 Paint Compositor Unit Tests Passed Successfully ===\n");
    return 0;
}
