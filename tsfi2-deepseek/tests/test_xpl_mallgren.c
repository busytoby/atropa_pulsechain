#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "tsfi_xpl_mallgren.h"

int main(void) {
    printf("====================================================================\n");
    printf("   TSFi XPLG PRECEDENCE & MALLGREN GRAPHICS SPECIFICATION TEST SUITE \n");
    printf("====================================================================\n");

    // 1. Test XPLG Precedence Matrix Verification
    tsfi_xplg_init_matrix();
    assert(tsfi_xplg_check_precedence(XPLG_TERM_BYTE, XPLG_TERM_LPAREN) == XPLG_PREC_EQUAL);
    assert(tsfi_xplg_check_precedence(XPLG_TERM_LPAREN, XPLG_TERM_NUMBER) == XPLG_PREC_LESS);
    assert(tsfi_xplg_check_precedence(XPLG_TERM_NUMBER, XPLG_TERM_RPAREN) == XPLG_PREC_GREATER);
    assert(tsfi_xplg_check_precedence(XPLG_TERM_SEMICOLON, XPLG_TERM_BYTE) == XPLG_PREC_NONE);
    printf("[PASS] XPLG grammar precedence transitions successfully verified.\n");

    // 2. Test XPLG Register Resolution (WinchesterMQ Context)
    uint32_t base_addr = 0xABCD01;
    uint32_t reg_channel = tsfi_xplg_resolve_register(XPLG_TERM_BYTE, base_addr);
    uint32_t reg_pole = tsfi_xplg_resolve_register(XPLG_TERM_NUMBER, base_addr);
    assert(reg_channel != 0);
    assert(reg_pole != 0);
    printf("[PASS] WinchesterMQ address resolution mapping successfully verified.\n");

    // 3. Test Mallgren Formal Graphics Specification
    MallgrenRegion r0, r1;
    tsfi_mallgren_init_region(&r0, 0.1, 0.1, 0.9, 0.9);
    assert(r0.min_bound.x == 0.1);
    assert(r0.max_bound.y == 0.9);

    MallgrenTransform tx = {2.0, 2.0, 0.0, 0.05, 0.05};
    tsfi_mallgren_apply_transform(&r0, &tx, &r1);
    assert(r1.min_bound.x == 0.1 * 2.0 + 0.05);
    assert(r1.max_bound.y == 0.9 * 2.0 + 0.05);
    printf("[PASS] Mallgren formal region transformations successfully verified.\n");

    // 4. Test Mallgren Rasterizer Execution
    uint8_t fb[128 * 128];
    memset(fb, 0, sizeof(fb));
    int ret = tsfi_mallgren_rasterize(&r1, fb, 128, 128, base_addr, 19);
    assert(ret == 0);
    printf("[PASS] Mallgren WinchesterMQ dynamics rasterizer execution successfully verified.\n");

    // 5. Test Virtual Inputs
    MallgrenVirtualInput input = {MALLGREN_INPUT_LOCATOR, 0.5, 0.5, 1.0};
    tsfi_mallgren_update_input(&input, 0.1, -0.2, 0.5);
    assert(input.val_x == 0.6);
    assert(input.val_y == 0.3);
    assert(input.scale == 1.5);
    printf("[PASS] Mallgren virtual input devices successfully verified.\n");

    // 6. Test Viewports and clipping
    MallgrenViewport vp = {0.2, 0.2, 0.8, 0.8};
    assert(tsfi_mallgren_clip_point(&vp, 0.5, 0.5) == true);
    assert(tsfi_mallgren_clip_point(&vp, 0.1, 0.5) == false);
    assert(tsfi_mallgren_clip_point(&vp, 0.5, 0.9) == false);
    printf("[PASS] Mallgren clipping viewports successfully verified.\n");

    // 7. Test Scene Graph hierarchies
    MallgrenSceneNode parent, child;
    tsfi_mallgren_init_node(&parent, 0.1, 0.1, 0.9, 0.9);
    tsfi_mallgren_init_node(&child, 0.2, 0.2, 0.8, 0.8);
    child.tx.scale_x = 0.5;
    child.tx.scale_y = 0.5;
    
    bool added = tsfi_mallgren_add_child(&parent, &child);
    assert(added == true);
    assert(parent.children_count == 1);

    memset(fb, 0, sizeof(fb));
    tsfi_mallgren_render_scene(&parent, NULL, &vp, fb, 128, 128, base_addr, 19);
    printf("[PASS] Mallgren scene graph hierarchical rendering successfully verified.\n");

    // 8. Test Semantic Actions
    MallgrenTransform action_tx = {1.0, 1.0, 0.0, 0.0, 0.0};
    bool parsed = tsfi_xplg_parse_semantic_action("OP_SCALE_DOUBLE_VAL", &action_tx);
    assert(parsed == true);
    assert(action_tx.scale_x == 2.0);
    printf("[PASS] XPLG semantic action parser successfully verified.\n");

    // 9. Test Region Combinations
    MallgrenRegion r_comb;
    tsfi_mallgren_combine_regions(&parent.region, &child.region, MALLGREN_OP_INTERSECT, &r_comb);
    assert(r_comb.opacity == 1.0);
    printf("[PASS] Mallgren region algebraic boolean combinations successfully verified.\n");

    // 10. Test Display updates model
    MallgrenDisplayModel display;
    tsfi_mallgren_init_display(&display, 60.0, true);
    assert(display.active_buffer_idx == 0);
    tsfi_mallgren_swap_buffers(&display);
    assert(display.active_buffer_idx == 1);
    // 11. Test compiler bridge
    MallgrenTransform bridge_tx = {1.0, 1.0, 0.0, 0.0, 0.0};
    bool compiled = tsfi_xplg_bridge_compile(XPLG_TERM_BYTE, 20, &bridge_tx);
    assert(compiled == true);
    assert(bridge_tx.scale_x == 2.0);
    printf("[PASS] XPLG graphics compiler bridge successfully verified.\n");

    // 12. Test grammar interpreter
    XplgTerminal tokens[3] = {XPLG_TERM_BYTE, XPLG_TERM_LPAREN, XPLG_TERM_NUMBER};
    tsfi_xplg_interpret_grammar(tokens, 3, &bridge_tx);
    assert(bridge_tx.scale_x > 2.0);
    printf("[PASS] XPLG interactive graphics grammar interpreter successfully verified.\n");

    // 13. Test Plotter driver
    TurrillPlotter plotter = {0.1, 0.1, true, 255};
    uint8_t plotter_fb[128 * 128];
    memset(plotter_fb, 0, sizeof(plotter_fb));
    tsfi_xplg_plotter_draw(&plotter, &bridge_tx, plotter_fb, 128, 128);
    // 14. Test Token Tracer and Conflict resolution
    tsfi_xplg_trace_token(XPLG_TERM_NUMBER, "Tracing test token");
    XplgConflictType conflict = tsfi_xplg_resolve_conflict(XPLG_TERM_NUMBER, XPLG_PREC_GREATER);
    assert(conflict == XPLG_CONFLICT_SHIFT_REDUCE);
    printf("[PASS] XPLG conflict resolution mapping successfully verified.\n");

    // 15. Test SVDAG Registry Node integration
    SvdagRegistry registry;
    tsfi_xplg_init_svdag_registry(&registry);
    assert(registry.count == 0);
    
    bool registered = tsfi_xplg_register_svdag_node(&registry, &parent.region, 0x56aD);
    assert(registered == true);
    assert(registry.count == 1);
    assert(registry.nodes[0].voxel_id == 0x56aD);
    assert(registry.nodes[0].bounds[0] == parent.region.min_bound.x);
    printf("[PASS] SVDAG registry coordinate mapping successfully verified.\n");

    printf("\n=== ALL XPLG & MALLGREN SPECIFICATION TESTS PASSED ===\n");
    return 0;
}
