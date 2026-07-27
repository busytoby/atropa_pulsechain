#define _GNU_SOURCE
#include "tsfi_dom.h"
#include "tsfi_layout.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#define EPSILON 0.001f
static bool approx_equal(float a, float b) {
    return fabsf(a - b) < EPSILON;
}

static void test_box_size_invariance(void) {
    printf("[Test] Running box size invariance test...\n");
    const char *html = "<div style=\"width: 100px; height: 50px; margin-left: 10px; margin-top: 5px; padding-left: 20px; border-left: 2px;\"></div>";
    TsfiNode *doc = tsfi_dom_parse_html(html);
    TsfiLayoutBox *root_box = tsfi_layout_tree_build(doc);
    assert(root_box != NULL);

    TsfiLayoutBox *div_box = root_box->first_child;
    assert(div_box != NULL);

    // Solve with infinite containing bounds
    tsfi_layout_solve(div_box, 0.0f, 0.0f, 800.0f, 600.0f);

    // Bounding dimensions should match explicit style width and height
    assert(approx_equal(div_box->w, 100.0f));
    assert(approx_equal(div_box->h, 50.0f));

    // Absolute position x, y must respect margin-left and margin-top offset
    assert(approx_equal(div_box->x, 10.0f));
    assert(approx_equal(div_box->y, 5.0f));

    tsfi_layout_box_free(root_box);
    tsfi_node_free(doc);
    printf("[Test] Box size invariance test passed!\n");
}

static void test_block_layout_flow(void) {
    printf("[Test] Running block layout flow test...\n");
    const char *html = 
        "<div>"
        "  <div style=\"height: 40px; margin-bottom: 10px;\"></div>"
        "  <div style=\"height: 60px;\"></div>"
        "</div>";
    TsfiNode *doc = tsfi_dom_parse_html(html);
    TsfiLayoutBox *root_box = tsfi_layout_tree_build(doc);
    assert(root_box != NULL);

    TsfiLayoutBox *container_box = root_box->first_child;
    assert(container_box != NULL);

    tsfi_layout_solve(container_box, 0.0f, 0.0f, 200.0f, 600.0f);

    TsfiLayoutBox *child1 = container_box->first_child;
    TsfiLayoutBox *child2 = child1->next_sibling;

    // Check vertical flow stacking coordinates
    assert(approx_equal(child1->y, 0.0f));
    assert(approx_equal(child1->h, 40.0f));

    // child2 should stack below child1 (including child1's margin-bottom)
    assert(approx_equal(child2->y, 50.0f));
    assert(approx_equal(child2->h, 60.0f));

    // Parent height should encapsulate both children outer heights (40 + 10 + 60 = 110)
    assert(approx_equal(container_box->h, 110.0f));

    tsfi_layout_box_free(root_box);
    tsfi_node_free(doc);
    printf("[Test] Block layout flow test passed!\n");
}

static void test_flex_layout_flow(void) {
    printf("[Test] Running flex layout flow test...\n");
    const char *html = 
        "<div style=\"display: flex;\">"
        "  <div style=\"width: 50px; height: 30px; margin-right: 5px;\"></div>"
        "  <div style=\"width: 70px; height: 40px;\"></div>"
        "</div>";
    TsfiNode *doc = tsfi_dom_parse_html(html);
    TsfiLayoutBox *root_box = tsfi_layout_tree_build(doc);
    assert(root_box != NULL);

    TsfiLayoutBox *flex_container = root_box->first_child;
    assert(flex_container != NULL && flex_container->type == TSFI_BOX_FLEX);

    tsfi_layout_solve(flex_container, 0.0f, 0.0f, 500.0f, 500.0f);

    TsfiLayoutBox *child1 = flex_container->first_child;
    TsfiLayoutBox *child2 = child1->next_sibling;

    // Check horizontal flow stacking coordinates
    assert(approx_equal(child1->x, 0.0f));
    assert(approx_equal(child1->w, 50.0f));

    // child2 should stack next to child1 (including child1's margin-right)
    assert(approx_equal(child2->x, 55.0f));
    assert(approx_equal(child2->w, 70.0f));

    // Flex container height should match maximum height of children (40px)
    assert(approx_equal(flex_container->h, 40.0f));

    tsfi_layout_box_free(root_box);
    tsfi_node_free(doc);
    printf("[Test] Flex layout flow test passed!\n");
}

static void test_display_none_exclusion(void) {
    printf("[Test] Running display: none exclusion test...\n");
    const char *html = "<div><div style=\"display: none;\"></div><div style=\"height: 20px;\"></div></div>";
    TsfiNode *doc = tsfi_dom_parse_html(html);
    TsfiLayoutBox *root_box = tsfi_layout_tree_build(doc);
    assert(root_box != NULL);

    TsfiLayoutBox *container_box = root_box->first_child;
    tsfi_layout_solve(container_box, 0.0f, 0.0f, 200.0f, 600.0f);
    tsfi_layout_print(root_box, 0);

    // The display: none element should have been excluded from box generation.
    // So the container should only have 1 active child layout box.
    TsfiLayoutBox *child = container_box->first_child;
    assert(child != NULL);
    assert(child->next_sibling == NULL);
    assert(approx_equal(child->h, 20.0f));

    tsfi_layout_box_free(root_box);
    tsfi_node_free(doc);
    printf("[Test] display: none exclusion test passed!\n");
}

static void test_inline_text_wrapping(void) {
    printf("[Test] Running inline text wrapping test...\n");
    // Container width of 100px. Three spans of length 10 (80px width each) should wrap onto 3 separate lines of height 16px.
    const char *html = 
        "<div style=\"width: 100px;\">"
        "  <span>0123456789</span>"
        "  <span>0123456789</span>"
        "  <span>0123456789</span>"
        "</div>";
    TsfiNode *doc = tsfi_dom_parse_html(html);
    TsfiLayoutBox *root_box = tsfi_layout_tree_build(doc);
    assert(root_box != NULL);

    TsfiLayoutBox *container = root_box->first_child;
    tsfi_layout_solve(container, 0.0f, 0.0f, 100.0f, 600.0f);
    tsfi_layout_print(root_box, 0);

    TsfiLayoutBox *span1 = container->first_child;
    TsfiLayoutBox *span2 = span1->next_sibling;
    TsfiLayoutBox *span3 = span2->next_sibling;

    // span1 should be on line 1 (y = 0)
    assert(approx_equal(span1->y, 0.0f));
    // span2 should wrap to line 2 (y = 16.0)
    assert(approx_equal(span2->y, 16.0f));
    // span3 should wrap to line 3 (y = 32.0)
    assert(approx_equal(span3->y, 32.0f));

    // Container height should be 3 * 16 = 48px
    assert(approx_equal(container->h, 48.0f));

    tsfi_layout_box_free(root_box);
    tsfi_node_free(doc);
    printf("[Test] Inline text wrapping test passed!\n");
}

int main(void) {
    printf("=== Starting Phase 2 Layout Unit Tests ===\n");
    test_box_size_invariance();
    test_block_layout_flow();
    test_flex_layout_flow();
    test_display_none_exclusion();
    test_inline_text_wrapping();
    printf("=== All Phase 2 Layout Unit Tests Passed Successfully ===\n");
    return 0;
}
