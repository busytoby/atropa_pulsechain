#define _GNU_SOURCE
#include "tsfi_dom.h"
#include "tsfi_layout.h"
#include "tsfi_paint.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define BUF_W 200
#define BUF_H 100

static void test_paint_red_box_white_border(void) {
    printf("[Test] Running red box and white border painting test...\n");
    const char *html = 
        "<div style=\"background-color: red; border-left: 2px; border-color: white; width: 50px; height: 30px;\">"
        "</div>";

    TsfiNode *doc = tsfi_dom_parse_html(html);
    TsfiLayoutBox *root_box = tsfi_layout_tree_build(doc);
    assert(root_box != NULL);

    TsfiLayoutBox *div_box = root_box->first_child;
    tsfi_layout_solve(div_box, 0.0f, 0.0f, (float)BUF_W, (float)BUF_H);

    // Allocate and clear pixel buffer to transparent black
    uint32_t *pixels = (uint32_t*)calloc(BUF_W * BUF_H, sizeof(uint32_t));
    assert(pixels != NULL);

    // Paint layout tree
    tsfi_paint_tree(div_box, pixels, BUF_W, BUF_H);

    // Left border pixel at (0, 0) should be white
    assert(pixels[0 * BUF_W + 0] == 0xFFFFFFFF);
    assert(pixels[10 * BUF_W + 1] == 0xFFFFFFFF);

    // Inner block pixel at (10, 10) should be red
    assert(pixels[10 * BUF_W + 10] == 0xFFFF0000);

    // Outer pixel at (60, 40) should be untouched (0x00000000)
    assert(pixels[40 * BUF_W + 60] == 0x00000000);

    free(pixels);
    tsfi_layout_box_free(root_box);
    tsfi_node_free(doc);
    printf("[Test] Red box and white border painting test passed!\n");
}

static void test_paint_text_color(void) {
    printf("[Test] Running text color painting test...\n");
    const char *html = 
        "<div style=\"width: 100px; height: 20px;\">"
        "  <span style=\"color: blue;\">H</span>"
        "</div>";

    TsfiNode *doc = tsfi_dom_parse_html(html);
    TsfiLayoutBox *root_box = tsfi_layout_tree_build(doc);
    assert(root_box != NULL);

    TsfiLayoutBox *div_box = root_box->first_child;
    tsfi_layout_solve(div_box, 0.0f, 0.0f, (float)BUF_W, (float)BUF_H);

    uint32_t *pixels = (uint32_t*)calloc(BUF_W * BUF_H, sizeof(uint32_t));
    assert(pixels != NULL);

    tsfi_paint_tree(div_box, pixels, BUF_W, BUF_H);

    // Verify text pixels of the letter 'H' are blue (0xFF0000FF)
    // Letter 'H' bitmap row is: 0x66 (binary: 01100110)
    // So columns 1, 2, 5, 6 should be colored.
    // Let's assert at least some text pixels are filled with blue
    bool has_blue_pixels = false;
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 20; x++) {
            if (pixels[y * BUF_W + x] == 0xFF0000FF) {
                has_blue_pixels = true;
                break;
            }
        }
    }
    assert(has_blue_pixels);

    free(pixels);
    tsfi_layout_box_free(root_box);
    tsfi_node_free(doc);
    printf("[Test] Text color painting test passed!\n");
}

int main(void) {
    printf("=== Starting Phase 3 Compositing & Paint Unit Tests ===\n");
    test_paint_red_box_white_border();
    test_paint_text_color();
    printf("=== All Phase 3 Compositing & Paint Unit Tests Passed Successfully ===\n");
    return 0;
}
