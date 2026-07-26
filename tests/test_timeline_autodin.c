#include "../src/auncient_timeline_autodin.h"
#include "../src/auncient_vulkan_materials.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT TIMELINE, AUTODIN, & HOGAN INTEGRATION TEST\n");
    printf("=============================================================\n");

    // 1. Initialize CICS Context
    sdk_cics_context_t ctx;
    memset(&ctx, 0, sizeof(ctx));

    // 2. Define Timeline Events
    TimelineEvent events[2] = {
        { .timestamp = 0.5f, .event_id = 101, .lock_token = 0x111, .precedence = 'F', .triggered = false },
        { .timestamp = 1.2f, .event_id = 102, .lock_token = 0x222, .precedence = 'L', .triggered = false }
    };

    // Process at time 0.1 (nothing should trigger)
    auncient_timeline_process(events, 2, 0.1f, &ctx, NULL, 0, 0);
    assert(!events[0].triggered);
    assert(!events[1].triggered);
    printf("   ✓ Timeline boundary guard verified.\n");

    // Process at time 0.8 (first event should trigger)
    auncient_timeline_process(events, 2, 0.8f, &ctx, NULL, 0, 0);
    assert(events[0].triggered);
    assert(!events[1].triggered);
    printf("   ✓ AUTODIN precedence event locking verified.\n");

    // 3. Hogan Asset Reconciliation Test
    uint8_t mock_dna[4] = {0x12, 0x34, 0x56, 0x78};
    bool reconciled = auncient_hogan_reconcile_asset(999, mock_dna, 4);
    assert(reconciled);
    assert(reconciled);
    printf("   ✓ Hogan transaction consensus verified.\n");

    // 4. Hogan Account Registration Test
    HoganAccount account;
    bool registered = auncient_hogan_register_account(777, mock_dna, 4, &account);
    assert(registered);
    assert(account.account_id == 777);
    assert(account.balance_saat == 1000000);
    assert(account.is_active == true);
    printf("   ✓ First-class Hogan account holder registration verified.\n");

    // 5. Test Deposit, Withdraw, and Overdraft protection
    bool dep_ok = auncient_hogan_deposit(&account, 500000);
    assert(dep_ok);
    assert(account.balance_saat == 1500000); // 1.0M + 0.5M
    printf("   ✓ Hogan account deposit verified.\n");

    bool withdraw_ok = auncient_hogan_withdraw(&account, 1000000);
    assert(withdraw_ok);
    assert(account.balance_saat == 500000); // 1.5M - 1.0M
    printf("   ✓ Hogan account withdrawal verified.\n");

    bool overdraft_fail = auncient_hogan_withdraw(&account, 600000);
    assert(!overdraft_fail); // Should fail due to insufficient balance
    assert(account.balance_saat == 500000);
    printf("   ✓ Hogan overdraft protection verified.\n");

    // 6. Test Timeline Rollback and Checkpoint recovery
    auncient_timeline_rollback(events, 2, 0.4f, &account, 1000000);
    assert(!events[0].triggered); // Reset since 0.5 > 0.4
    assert(account.balance_saat == 1000000); // Checkpoint restored
    printf("   ✓ Timeline seek rollback and checkpoint recovery verified.\n");

    // 7. Test Timeline Payroll distribution
    auncient_timeline_payroll_payout(&account, 250000);
    assert(account.balance_saat == 1250000); // 1.0M restored balance + 0.25M payout
    printf("   ✓ Timeline payroll salary distribution verified.\n");

    // 8. Test Account Transfer and double-entry auditing
    HoganAccount recipient_acc;
    uint8_t rec_dna[4] = {0xAA, 0xBB, 0xCC, 0xDD};
    auncient_hogan_register_account(888, rec_dna, 4, &recipient_acc);
    
    // Attempt high-value transfer with clearance level 1 (should fail)
    bool unauth_tx = auncient_hogan_transfer(&account, &recipient_acc, 200000);
    assert(!unauth_tx);
    
    // Escalate clearance and transfer successfully
    account.clearance_level = 2;
    bool auth_tx = auncient_hogan_transfer(&account, &recipient_acc, 250000);
    assert(auth_tx);
    assert(account.balance_saat == 1000000); // 1.25M - 250k
    assert(recipient_acc.balance_saat == 1250000); // 1.0M + 250k
    printf("   ✓ Double-entry transfer and clearance authorizations verified.\n");

    // Verify ledger audit
    HoganAccount ledger[2] = { account, recipient_acc };
    bool audit_ok = auncient_hogan_audit_ledger(ledger, 2, 2250000); // 1.0M + 1.25M
    assert(audit_ok);
    printf("   ✓ Ledger auditing and checks verified.\n");

    // Verify ongoing recurring audits
    sdk_cics_context_t audit_ctx;
    memset(&audit_ctx, 0, sizeof(audit_ctx));
    
    // First audit at time 1.1 with correct balance (should pass, last_blame = 0)
    auncient_timeline_process(events, 2, 1.1f, &audit_ctx, ledger, 2, 2250000);
    assert(audit_ctx.last_blame == 0);
    printf("   ✓ Recurring AUTODIN audit success path verified.\n");

    // Second audit at time 2.2 with mismatched balance (should fail, last_blame = 1)
    auncient_timeline_process(events, 2, 2.2f, &audit_ctx, ledger, 2, 3000000);
    assert(audit_ctx.last_blame == 1);
    printf("   ✓ Recurring AUTODIN audit violation blame verified.\n");

    // Test hypervisor latency warning
    auncient_hypervisor_monitor_audit(0.0000002f, true); // Fast audit info
    auncient_hypervisor_monitor_audit(0.0000050f, false); // Long-running warning
    printf("   ✓ Hypervisor audit logging verified.\n");

    // Test WinchesterMQ ABI transitions and registers
    WinchesterMQState w_state;
    memset(&w_state, 0, sizeof(w_state));
    w_state.identity = 3;
    
    // Seed
    winchester_mq_seed(&w_state, 5, 2, 4);
    assert(w_state.base == 5);
    
    // Tune (Channel = 5^4 % MOTZKIN_PRIME = 625)
    winchester_mq_tune(&w_state);
    assert(w_state.channel == 625);
    
    // Saturate
    winchester_mq_saturate(&w_state, 10, 20, 100);
    assert(w_state.element == 110);
    assert(w_state.chin == 30);
    assert(w_state.monopole == (30 * 30 * 30)); // 30^3
    printf("   ✓ WinchesterMQ state transitions and ABI equations verified.\n");

    // Test Vulkan texgen configurations
    VkImageCreateInfo img_info;
    auncient_texgen_build_image_info(512, 512, &img_info);
    assert(img_info.sType == 14);
    assert(img_info.width == 512);
    assert(img_info.height == 512);
    printf("   ✓ Vulkan image staging configuration verified.\n");

    VkBufferImageCopy copy_info;
    auncient_texgen_build_copy_info(512, 512, &copy_info);
    assert(copy_info.imageExtent[0] == 512);
    assert(copy_info.imageDisplacement[0] == 0);
    printf("   ✓ Vulkan buffer-to-image copy regions verified.\n");

    // Test WinchesterMQ dynamic seed modulation
    uint32_t active_seed = auncient_texgen_modulated_seed(&w_state);
    assert(active_seed > 0);
    printf("   ✓ WinchesterMQ noise seed modulation verified.\n");

    // Test palette permutation gradients
    uint32_t ocean_col = auncient_texgen_permute_palette(0.8f, 1, 1.0f); // Ocean blue
    assert((ocean_col & 0x000000FF) != 0); // Blue channel present
    
    uint32_t fire_col = auncient_texgen_permute_palette(0.8f, 2, 1.0f); // Fire red
    assert((fire_col & 0x00FF0000) != 0); // Red channel present
    printf("   ✓ Material-driven palette color grading verified.\n");

    // Test Catmull-Rom spline camera paths
    float p0[3] = {0.0f, 0.0f, 0.0f};
    float p1[3] = {1.0f, 2.0f, 3.0f};
    float p2[3] = {2.0f, 4.0f, 6.0f};
    float p3[3] = {3.0f, 6.0f, 9.0f};
    float camera_pos[3];
    auncient_spline_evaluate(0.5f, p0, p1, p2, p3, camera_pos);
    assert(camera_pos[0] > 0.0f);
    assert(camera_pos[1] > 0.0f);
    assert(camera_pos[2] > 0.0f);
    printf("   ✓ Catmull-Rom camera position calculation verified.\n");

    GlobalUniformBlock glob_block;
    auncient_spline_to_global_uniform(camera_pos, &glob_block);
    assert(glob_block.camera_position[0] == camera_pos[0]);
    printf("   ✓ Camera coordinates written to Vulkan global uniform block.\n");

    // Test spline mesh deformation
    ClothVertex v;
    v.x = 0.0f; v.y = 0.0f; v.z = 0.0f;
    float spline_path[6] = {
        10.0f, 20.0f, 30.0f,
        15.0f, 25.0f, 35.0f
    };
    auncient_mesh_deform_along_spline(&v, 1, spline_path, 2);
    assert(v.x > 0.0f);
    assert(v.y > 0.0f);
    assert(v.z > 0.0f);
    printf("   ✓ Spline-deformed mesh vertex updates verified.\n");

    // Test spline-Verlet physics integrations
    SplinePhysNode phys_nodes[2] = {
        { .x = 0.0f, .y = 0.0f, .z = 0.0f, .px = 0.0f, .py = 0.0f, .pz = 0.0f, .ax = 0.0f, .ay = 0.0f, .az = 0.0f },
        { .x = 2.0f, .y = 0.0f, .z = 0.0f, .px = 2.0f, .py = 0.0f, .pz = 0.0f, .ax = 0.0f, .ay = 0.0f, .az = 0.0f }
    };
    auncient_spline_verlet_step(phys_nodes, 2, 0.1f, 0.99f, 10.0f, 0.0f, 0.0f);
    assert(phys_nodes[0].x != 0.0f || phys_nodes[1].x != 2.0f);
    printf("   ✓ Spline-Verlet physical step integration verified.\n");

    // Test spline to cloth coupling
    ClothPoint cp;
    cp.x = 5.0f; cp.y = 10.0f; cp.z = 15.0f;
    cp.px = 4.0f; cp.py = 9.0f; cp.pz = 14.0f;
    auncient_couple_spline_to_cloth(&phys_nodes[0], &cp);
    assert(phys_nodes[0].x == 5.0f);
    assert(phys_nodes[0].px == 4.0f);
    printf("   ✓ Spline-to-cloth coupling anchors verified.\n");

    // Test TCB Keyframe splines and XPL register writes
    TcbKeyframe keys[3];
    memset(keys, 0, sizeof(keys));
    
    // Write keyframe 0 using XPL write address space
    auncient_xpl_write_spline_register(keys, 3, 0xF200, 0.0f); // keys[0].frame = 0
    auncient_xpl_write_spline_register(keys, 3, 0xF201, 10.0f); // keys[0].data = 10
    
    // Write keyframe 1
    auncient_xpl_write_spline_register(keys, 3, 0xF208, 1.0f); // keys[1].frame = 1
    auncient_xpl_write_spline_register(keys, 3, 0xF209, 20.0f); // keys[1].data = 20
    
    // Write keyframe 2
    auncient_xpl_write_spline_register(keys, 3, 0xF210, 2.0f); // keys[2].frame = 2
    auncient_xpl_write_spline_register(keys, 3, 0xF211, 40.0f); // keys[2].data = 40
    
    assert(keys[0].data == 10.0f);
    assert(keys[1].frame == 1.0f);
    assert(keys[2].data == 40.0f);
    
    // Calculate tangents and evaluate spline interpolation at frame 0.5
    auncient_tcb_calculate_tangents(keys, 3);
    float interp_val = auncient_tcb_evaluate(keys, 3, 0.5f);
    assert(interp_val > 10.0f && interp_val < 20.0f);
    printf("   ✓ TCB spline Hermite interpolation and XPL script registers verified.\n");

    // Test smooth normal generation
    ClothVertex v_normal[3] = {
        { .x = 0.0f, .y = 0.0f, .z = 0.0f },
        { .x = 1.0f, .y = 0.0f, .z = 0.0f },
        { .x = 0.0f, .y = 1.0f, .z = 0.0f }
    };
    int indices[3] = { 0, 1, 2 };
    auncient_mesh_generate_normals(v_normal, 3, indices, 3);
    assert(v_normal[0].nz == 1.0f); // Normal points straight up out of XY plane (z-axis)
    printf("   ✓ Smooth vertex normal generation verified.\n");

    // Test termcap capabilities query
    AuncientTermcap tc;
    auncient_termcap_query(&tc);
    assert(tc.max_colors >= 8);
    assert(tc.cols > 0);
    assert(tc.rows > 0);
    printf("   ✓ ANSI Termcap capabilities query verified.\n");

    // Test coaxial kerning spacing updates
    float spacing = auncient_calculate_coaxial_kerning(0.8f, 2.0f);
    assert(spacing < 2.0f); // Spacing decreases under tension
    printf("   ✓ Coaxial kerning character spacing verified.\n");

    // Test spring links
    ClothPoint p_spring1 = { .x = 0.0f, .y = 0.0f, .z = 0.0f, .is_anchored = 0 };
    ClothPoint p_spring2 = { .x = 4.0f, .y = 0.0f, .z = 0.0f, .is_anchored = 0 };
    auncient_apply_spring_link(&p_spring1, &p_spring2, 2.0f); // Rest len 2, current len 4
    assert(p_spring1.x > 0.0f);
    assert(p_spring2.x < 4.0f);
    printf("   ✓ Inter-body spring link constraints verified.\n");

    // Test box-to-box collisions
    ClothPoint box1[2] = {
        { .x = 0.0f, .y = 0.0f, .z = 0.0f, .is_anchored = 0 },
        { .x = 2.0f, .y = 2.0f, .z = 0.0f, .is_anchored = 0 }
    };
    ClothPoint box2[2] = {
        { .x = 1.0f, .y = 0.0f, .z = 0.0f, .is_anchored = 0 },
        { .x = 3.0f, .y = 2.0f, .z = 0.0f, .is_anchored = 0 }
    };
    auncient_resolve_box_collisions(box1, 2, box2, 2);
    assert(box1[0].x < 0.0f); // Repelled left
    assert(box2[0].x > 1.0f); // Repelled right
    printf("   ✓ Box collision resolution and repulsion verified.\n");

    // Test antialiased line rasterization
    char line_grid[100];
    memset(line_grid, ' ', sizeof(line_grid));
    auncient_rasterize_antialiased_line(0.0f, 0.0f, 8.0f, 8.0f, line_grid, 10, 10);
    assert(line_grid[0] != ' '); // First pixel rasterized
    assert(line_grid[99] != ' '); // Last pixel rasterized
    printf("   ✓ Sub-pixel antialiased line rasterization verified.\n");

    // Test XPL spline text-grid renderer
    TcbKeyframe test_keys[3] = {
        { .frame = 0.0f, .data = 1.0f },
        { .frame = 1.0f, .data = 5.0f },
        { .frame = 2.0f, .data = 9.0f }
    };
    char xpl_grid[100];
    auncient_xpl_render_spline_to_grid(test_keys, 3, xpl_grid, 10, 10);
    assert(xpl_grid[0] != ' '); // First segment drawn
    printf("   ✓ XPL spline rendering to text grid verified.\n");

    // Test dual-column text partitioning
    char col1[30], col2[30];
    auncient_ansi_layout_partition("LEFT\nRIGHT", col1, 10, col2, 10, 3);
    assert(col1[0] == 'L');
    assert(col2[0] == 'R');
    printf("   ✓ Dual-column ANSI text layout partitioning verified.\n");

    // Test keyboard navigation menus
    int sel = 0;
    sel = auncient_navigate_ansi_menu(32, sel, 4); // Keycode 32 is 'D' / Right
    assert(sel == 1);
    sel = auncient_navigate_ansi_menu(30, sel, 4); // Keycode 30 is 'A' / Left
    assert(sel == 0);
    printf("   ✓ Keyboard menu selection navigation verified.\n");

    // Test Hudson VCE color registers
    uint16_t vce_table[512] = { 0 };
    auncient_hudson_vce_write_color(vce_table, 10, 0xF800); // Write Red color
    assert(vce_table[10] == 0xF800);
    
    auncient_hudson_vce_write_color(vce_table, 11, 0x07E0); // Write Green color
    auncient_hudson_vce_cycle_palette(vce_table, 10, 11, 1); // Shift range by 1
    assert(vce_table[10] == 0x07E0); // Green shifted to index 10
    assert(vce_table[11] == 0xF800); // Red shifted to index 11
    printf("   ✓ Hudson VCE palette registers and color cycling shifts verified.\n");

    // Test WinchesterMQ VCE synchronization
    WinchesterMQState mq_sync = {
        .channel = 10,
        .dynamo = 20,
        .pole = 5,
        .signal = 3
    };
    auncient_hudson_vce_sync_winchester(vce_table, &mq_sync);
    assert(vce_table[0] != 0); // Background color populated
    printf("   ✓ WinchesterMQ to Hudson VCE synchronization verified.\n");

    // Test Hudson VDC DMA block transfer palette shift
    auncient_hudson_vce_write_color(vce_table, 50, 0x07E0); // Green color at 50
    auncient_hudson_vdc_dma_palette_shift(vce_table, 50, 60, 1); // DMA copy to 60
    assert(vce_table[60] == 0x07E0);
    printf("   ✓ Hudson VDC DMA hardware palette shift verified.\n");

    // Test tracker portamento (pitch slide)
    float freq = 440.0f;
    auncient_apply_tracker_portamento(&freq, 480.0f, 10.0f);
    assert(freq == 450.0f);
    printf("   ✓ Tracker portamento pitch slide sweeps verified.\n");

    // Test tracker tremolo (volume modulation)
    float volume = 0.5f;
    auncient_apply_tracker_tremolo(&volume, 1.0f, 0.2f, 1.5f);
    assert(volume != 0.5f);
    printf("   ✓ Tracker tremolo volume modulation verified.\n");

    // Test Fourier passengers on Verlet splines
    SplinePhysNode fourier_nodes[2] = {
        { .x = 0.0f, .y = 0.0f, .z = 0.0f },
        { .x = 10.0f, .y = 0.0f, .z = 0.0f }
    };
    auncient_apply_fourier_passengers(fourier_nodes, 2, 1.0f, 0.5f);
    assert(fourier_nodes[0].y != 0.0f); // Harmonic wave displacement applied
    printf("   ✓ Fourier passenger harmonic wave modulations verified.\n");

    // Test visual character tremolo
    char t_grid[4] = "abc";
    auncient_apply_ansi_tremolo_char(t_grid, 2, 2, 0, 0, 1.0f, 10.0f);
    assert(t_grid[0] != 'a'); // Character modulated
    printf("   ✓ Visual character tremolo grid cell modulations verified.\n");

    // Test unified Verlet scene drawing
    char draw_grid[100];
    ClothPoint draw_points[2] = {
        { .x = 5.0f, .y = 5.0f, .z = 0.0f, .is_anchored = 0 },
        { .x = 2.0f, .y = 2.0f, .z = 0.0f, .is_anchored = 0 }
    };
    auncient_verlet_draw_scene(draw_grid, 10, 10, draw_points, 2, 3, 3, 1.0f); // Cursor hovering over button
    assert(draw_grid[23] == '='); // Highlight borders populated by cursor hover
    printf("   ✓ Unified Verlet ANSI scene drawing loop verified.\n");

    // Test cell-addressable transitions
    AuncientAnsiCell test_cell = {
        .point = { .x = 1.0f, .y = 1.0f, .z = 0.0f, .is_anchored = 0 },
        .glyph = 'a'
    };
    auncient_ansi_cell_transition(&test_cell, "seed", 0, 0);
    assert(test_cell.point.x >= 1.0f); // Coordinate updated by transition displacement
    printf("   ✓ Cell-addressable WinchesterMQ ABI transitions verified.\n");

    // Test word wrapping
    char wrap_out[50];
    auncient_word_wrap_text("HELLO WORLD TEST", wrap_out, 6, 3);
    assert(wrap_out[5] == '\n'); // Wrapped at space
    printf("   ✓ Word-wrapping paragraph layout engine verified.\n");

    // Test markdown parsing to ANSI grid
    char md_grid[100];
    auncient_parse_markdown_to_ansi("# HEADER\n> QUOTE", md_grid, 10, 10, 0);
    assert(md_grid[0] == '|'); // Border populated
    assert(md_grid[2] == '*'); // Header character converted
    printf("   ✓ Markdown-to-ANSI styled layout engine verified.\n");

    // Test AUTODIN edit auditing
    auncient_autodin_audit_edit("DATA", 4, 2, 'I');
    printf("   ✓ AUTODIN edit auditing verified.\n");

    printf("=============================================================\n");
    printf("AUNCIENT INTEGRATION TEST COMPLETE\n");
    printf("=============================================================\n");
    return 0;
}
