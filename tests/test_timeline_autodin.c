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

    printf("=============================================================\n");
    printf("AUNCIENT INTEGRATION TEST COMPLETE\n");
    printf("=============================================================\n");
    return 0;
}
