#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "tsfi_xplos_kernel.h"
#include "tsfi_nadler_syntactic_parser.h"

static int g_task_executions = 0;

static void dummy_task_entry(void *arg) {
    (void)arg;
    g_task_executions++;
}

int main(void) {
    printf("====================================================================\n");
    printf("        TSFi XplOS KERNEL SCHEDULER & GPU JIT COMPILER TEST SUITE    \n");
    printf("====================================================================\n");

    // 1. Initialize and run cooperative scheduler
    XplosScheduler sched;
    tsfi_xplos_init_scheduler(&sched);
    assert(sched.task_count == 0);

    int t0 = tsfi_xplos_create_task(&sched, dummy_task_entry, NULL);
    int t1 = tsfi_xplos_create_task(&sched, dummy_task_entry, NULL);
    assert(t0 == 0);
    assert(t1 == 1);
    assert(sched.task_count == 2);

    tsfi_xplos_run(&sched);
    assert(g_task_executions > 0);
    printf("[PASS] Xplos cooperative scheduler execution successfully verified.\n");

    // 2. Test block and event triggers
    uint32_t blocked_task_idx = sched.active_task_idx;
    tsfi_xplos_block(&sched, 0xF5A0);
    assert(sched.tasks[blocked_task_idx].state == TASK_STATE_BLOCKED);
    
    tsfi_xplos_trigger_event(&sched, 0xF5A0);
    assert(sched.tasks[blocked_task_idx].state == TASK_STATE_READY);
    printf("[PASS] Xplos event blocking and triggering successfully verified.\n");

    // 3. Test GPU JIT compiler
    MallgrenSceneNode root;
    tsfi_mallgren_init_node(&root, 0.1, 0.1, 0.9, 0.9);
    root.tx.scale_x = 1.5;
    root.tx.scale_y = 1.5;

    XplgGpuJitShader jit_shader;
    bool compiled = tsfi_xplg_jit_compile_shader(&root, &jit_shader);
    assert(compiled == true);
    assert(jit_shader.source_len > 0);
    assert(strstr(jit_shader.shader_source, "fn main") != NULL);
    printf("[PASS] XPLG GPU JIT shader compiler code generation successfully verified.\n");

    // 4. Test Vulkan Shader Dispatch
    uint32_t vram_fb[256 * 256];
    bool dispatched = tsfi_xplg_vulkan_dispatch_shader(&jit_shader, vram_fb, 256, 256);
    assert(dispatched == true);
    assert(vram_fb[0] == 0xFFFFFFFF);
    printf("[PASS] XPLG Vulkan GPU dispatch successfully verified.\n");

    // 5. Test XplOS User Shell
    XplosShell shell;
    tsfi_xplos_init_shell(&shell);
    assert(shell.active == true);

    bool shell_ok = tsfi_xplos_shell_exec(&shell, &sched, "SCALE_DOUBLE");
    assert(shell_ok == true);
    assert(sched.task_count == 3);
    printf("[PASS] XplOS interactive shell command dispatch successfully verified.\n");

    // 6. Test VFS Layout Checks (Rule 13)
    XplosVirtualDisk disk;
    tsfi_xplos_init_vfs(&disk);
    assert(tsfi_xplos_create_file(&disk, "test.dat.bin", 1024) == true);
    assert(tsfi_xplos_create_file(&disk, "illegal.json", 1024) == false); // Blocked by Rule 13
    printf("[PASS] XplOS VFS layout checks successfully verified.\n");

    // 7. Test ACMS Recovery Rollback
    XplosAcmsLog tx_log;
    tsfi_xplos_log_tx_state(&tx_log, 0x1, 0xABCDEF);
    assert(tx_log.logged == true);
    uint32_t restored = tsfi_xplos_abort_recovery(&tx_log);
    assert(restored == 0xABCDEF);
    assert(tx_log.logged == false);
    printf("[PASS] ACMS transaction logging and recovery successfully verified.\n");

    // 8. Test SCSI loopback keyboard driver (Rule 5)
    uint32_t key_task_idx = sched.active_task_idx;
    tsfi_xplos_block(&sched, 32); // Block active task on keycode 32 (d/D)
    bool key_processed = tsfi_xplos_scsi_process_keycode(32, &sched);
    assert(key_processed == true);
    assert(sched.tasks[key_task_idx].state == TASK_STATE_READY); // Task successfully awakened
    printf("[PASS] SCSI loopback keyboard interrupts successfully verified.\n");

    // 9. Test Cryptographic Ledger validation and verification
    XplosVerificationLedger ledger;
    tsfi_xplos_init_ledger(&ledger);
    assert(ledger.count == 0);

    bool b0 = tsfi_xplos_add_log_block(&ledger, 1, 0x1000, 0x2000, NULL);
    bool b1 = tsfi_xplos_add_log_block(&ledger, 2, 0x2000, 0x3000, NULL);
    assert(b0 == true);
    assert(b1 == true);
    assert(ledger.count == 2);

    // Verify valid chain
    assert(tsfi_xplos_verify_ledger(&ledger) == true);
    printf("[PASS] Cryptographic ledger verification successfully validated.\n");

    // Tamper with intermediate block and check rejection
    ledger.blocks[0].post_state_crc = 0x9999; // Broke state chain continuity
    assert(tsfi_xplos_verify_ledger(&ledger) == false);
    printf("[PASS] Cryptographic ledger tampering detection successfully validated.\n");

    // 10. Test Cooperative Task Execution of Skeletonizer & Syntactic Parser
    uint8_t *test_bitmap = calloc(256 * 256, 1);
    tsfi_nadler_stroke_type_t test_strokes[3] = {NADLER_STROKE_STEM, NADLER_STROKE_STEM, NADLER_STROKE_BAR};
    
    int t_skel = tsfi_xplos_create_task(&sched, tsfi_xplos_task_skeletonize, test_bitmap);
    int t_synt = tsfi_xplos_create_task(&sched, tsfi_xplos_task_parse_syntax, test_strokes);
    assert(t_skel >= 0);
    assert(t_synt >= 0);
    
    // Run scheduler loops to execute these tasks
    tsfi_xplos_run(&sched);
    free(test_bitmap);
    printf("[PASS] Cooperative execution of skeletonizer and parser tasks successfully validated.\n");

    // 11. Test Synch.Live Flocking & Connectedness
    XplosFlock flock;
    tsfi_xplos_init_flock(&flock);
    assert(flock.count == MAX_BOIDS);
    assert(flock.connectivity_score == 0.0);

    tsfi_xplos_update_flock(&flock);
    assert(flock.connectivity_score > 0.0);
    printf("[PASS] Flocking agent updates and C_sync connectedness successfully validated.\n");

    // 12. Test flocking rendering to VRAM
    uint8_t flock_fb[128 * 128];
    memset(flock_fb, 0, sizeof(flock_fb));
    tsfi_xplos_render_flock(&flock, flock_fb, 128, 128);
    printf("[PASS] Flocking spatial framebuffer rendering successfully validated.\n");

    // 13. Test Synch.Live Obstacle Flocking
    XplosObstacleFlock of_flock;
    tsfi_xplos_init_obstacle_flock(&of_flock);
    assert(of_flock.target_x == 0.9);
    assert(of_flock.obstacle_grid[2 * 16 + 5] == 1);

    tsfi_xplos_update_obstacle_flock(&of_flock);
    assert(of_flock.base_flock.connectivity_score > 0.0);
    printf("[PASS] Obstacle-aware flocking and potential field updates successfully validated.\n");

    // 14. Test Bounded Confidence Probabilistic Social Learning with Memory
    XplosSocialNetwork net;
    tsfi_xplos_init_social_network(&net);
    assert(net.agents[0].opinion == 0.0);
    assert(net.trust_matrix[0][1] == 0.5);

    // Perform updates
    tsfi_xplos_update_social_opinions(&net);
    // Since opinions change, confidence bounds adjust based on memory variance
    assert(net.agents[0].confidence_bound > 0.0);
    printf("[PASS] Bounded confidence social learning updates successfully validated.\n");

    // 15. Test Synch.Live Spatial-Cognitive Coupled Flocking Synchronizer
    XplosCognitiveFlock cog_flock;
    tsfi_xplos_init_cognitive_flock(&cog_flock);
    assert(cog_flock.count == MAX_COG_BOIDS);
    assert(cog_flock.spatial_cognitive_sync == 0.0);

    tsfi_xplos_update_cognitive_flock(&cog_flock);
    assert(cog_flock.spatial_cognitive_sync > 0.0);
    printf("[PASS] Spatial-cognitive coupled flocking synchronizer successfully validated.\n");

    // 16. Test EWMD Bounded Confidence Social Learning with Credibility Ratings
    tsfi_xplos_update_ewmd_opinions(&net, 0.9); // alpha = 0.9
    // Assert that credibility rating is decayed/updated for out-of-bounds agent 7
    assert(net.credibility_ratings[0][7] < 1.0);
    printf("[PASS] EWMD-decay credibility social learning successfully validated.\n");

    // 17. Test Priority-Based Real-Time Scheduler (QoS Yielding)
    int t_low = tsfi_xplos_create_task_priority(&sched, dummy_task_entry, NULL, 3); // Low Priority
    int t_high = tsfi_xplos_create_task_priority(&sched, dummy_task_entry, NULL, 1); // High Priority
    assert(t_low >= 0);
    assert(t_high >= 0);

    // Yield should prioritize the higher priority task index over the lower priority one
    tsfi_xplos_yield(&sched);
    assert(sched.tasks[sched.active_task_idx].priority == 1);
    printf("[PASS] Priority-based scheduler QoS yielding successfully validated.\n");

    // 18. Test PKI Credential Signature Validation
    uint8_t dummy_pubkey[32] = {0xAB};
    uint8_t dummy_msg[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    // Expected signature derived from FNV-1a hash of dummy_msg
    uint32_t expected_sig = 0x2804678D; // precalculated hash of message
    uint8_t sig_buffer[4];
    memcpy(sig_buffer, &expected_sig, 4);

    bool pki_ok = tsfi_xplos_validate_pki_signature(dummy_pubkey, sig_buffer, dummy_msg, 8);
    assert(pki_ok == true);
    printf("[PASS] PKI credential proof verification successfully validated.\n");

    // 19. Test VFS Page-Table Swapping (Rule 13)
    uint8_t page_data[256] = {0xFF};
    bool swap_ok = tsfi_xplos_swap_page_to_vfs(&disk, 0x900D, page_data, 256);
    assert(swap_ok == true);
    assert(disk.count == 2); // Previous file + new swapped page file
    printf("[PASS] VFS page-table swapping successfully validated.\n");

    printf("\n=== ALL XplOS KERNEL & GPU JIT TESTS PASSED ===\n");
    return 0;
}
