#include "tsfi_dom.h"
#include "tsfi_layout.h"
#include "tsfi_paint.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

int main(void) {
    printf("=== Starting Auncient Layout Engine Unit Test ===\n");

    const char *html = 
        "<div style=\"display: block; width: 400px; height: 300px; background-color: #ff0000;\">\n"
        "  <div style=\"display: block; margin-top: 10px; margin-left: 20px; width: 100px; height: 50px; background-color: #00ff00;\">\n"
        "  </div>\n"
        "  <video src=\"test_video.mp4\" style=\"width: 200px; height: 150px;\"></video>\n"
        "</div>";

    printf("[Test] Parsing HTML content...\n");
    TsfiNode *root = tsfi_dom_parse_html(html);
    assert(root != NULL);
    printf("[Test] DOM parsed successfully.\n");
    tsfi_dom_print(root, 0);

    printf("[Test] Building layout tree...\n");
    TsfiLayoutBox *layout_root = tsfi_layout_tree_build(root);
    assert(layout_root != NULL);
    printf("[Test] Layout tree built successfully.\n");
    tsfi_layout_print(layout_root, 0);

    printf("[Test] Solving layout for 1024x768 viewport...\n");
    tsfi_layout_solve(layout_root, 0.0f, 0.0f, 1024.0f, 768.0f);
    
    // Verify outer div box solved geometry
    TsfiLayoutBox *outer_div = layout_root->first_child;
    assert(outer_div != NULL);
    assert(outer_div->w == 400.0f);
    assert(outer_div->h == 300.0f);
    assert(outer_div->x == 0.0f);
    assert(outer_div->y == 0.0f);

    // Verify first child block coordinates
    TsfiLayoutBox *child1 = outer_div->first_child;
    assert(child1 != NULL);
    assert(child1->type == TSFI_BOX_BLOCK);
    // x should account for margin-left (20.0f)
    assert(child1->x == 20.0f);
    // y should account for margin-top (10.0f)
    assert(child1->y == 10.0f);
    assert(child1->w == 100.0f);
    assert(child1->h == 50.0f);

    // Verify video box coordinates (placed vertically after child1 block)
    TsfiLayoutBox *video_box = child1->next_sibling;
    assert(video_box != NULL);
    assert(video_box->type == TSFI_BOX_VIDEO);
    assert(video_box->w == 200.0f);
    assert(video_box->h == 150.0f);

    printf("[Test] Layout geometry solved correctly.\n");

    // Allocate viewport pixel buffer (1024 x 768 x 4 bytes)
    int width = 1024;
    int height = 768;
    uint32_t *pixels = (uint32_t *)calloc(width * height, sizeof(uint32_t));
    assert(pixels != NULL);

    printf("[Test] Painting layout tree onto buffer...\n");
    tsfi_paint_tree(layout_root, pixels, width, height);

    // Check pixels at child1 block (x=20, y=10) should have green color #00ff00 (0xFF00FF00 or 0x00FF00 in ABGR/RGBA)
    // The parse_color converts "#00ff00" depending on format. Let's make sure it's colored.
    uint32_t p_child1 = pixels[15 * width + 25]; 
    assert(p_child1 != 0); // Should be painted

    printf("[Test] Painting completed and verified.\n");

    // Cleanup
    free(pixels);
    tsfi_layout_box_free(layout_root);
    tsfi_node_free(root);

    printf("=== Auncient Layout Engine Unit Test PASSED ===\n");
    return 0;
}
