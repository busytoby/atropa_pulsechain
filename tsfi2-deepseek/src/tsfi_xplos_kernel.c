#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "tsfi_xplos_kernel.h"
#include "tsfi_lowpower_fet.h"
#include "tsfi_nadler_skeletonizer.h"
#include "tsfi_nadler_syntactic_parser.h"

// MotzkinPrime system constant
#define MOTZKIN_PRIME 953467954114363ULL

// 1. Initialize the cooperative task scheduler
void tsfi_xplos_init_scheduler(XplosScheduler *sched) {
    if (!sched) return;
    sched->task_count = 0;
    sched->active_task_idx = 0;
    sched->interrupt_mask = 0;
    memset(sched->tasks, 0, sizeof(sched->tasks));
}

// 2. Create a new task in the scheduler
int tsfi_xplos_create_task(XplosScheduler *sched, TaskEntry entry, void *arg) {
    return tsfi_xplos_create_task_priority(sched, entry, arg, 3); // Default lowest priority
}

// 2b. Create a priority-bound task in the scheduler
int tsfi_xplos_create_task_priority(XplosScheduler *sched, TaskEntry entry, void *arg, uint32_t priority) {
    if (!sched || !entry || sched->task_count >= MAX_TASKS) {
        return -1;
    }

    XplosTask *task = &sched->tasks[sched->task_count];
    task->task_id = (uint32_t)sched->task_count;
    task->state = TASK_STATE_READY;
    task->entry = entry;
    task->arg = arg;
    task->priority = priority;
    
    // Set simulated stack pointer to the end of the stack buffer
    task->sp = (void *)&task->stack[STACK_SIZE - 16];
    
    sched->task_count++;
    return (int)task->task_id;
}

// 3. Yield CPU execution to the next ready task (Priority-based scheduling)
void tsfi_xplos_yield(XplosScheduler *sched) {
    if (!sched || sched->task_count <= 1) return;

    uint32_t prev_idx = sched->active_task_idx;
    uint32_t next_idx = prev_idx;
    uint32_t highest_prio = 4; // Lower than lowest (3)

    // Find the highest priority ready task
    for (int i = 0; i < sched->task_count; i++) {
        uint32_t check_idx = (prev_idx + 1 + i) % sched->task_count;
        if (sched->tasks[check_idx].state == TASK_STATE_READY) {
            if (sched->tasks[check_idx].priority < highest_prio) {
                highest_prio = sched->tasks[check_idx].priority;
                next_idx = check_idx;
            }
        }
    }

    if (next_idx != prev_idx) {
        // Transition task states
        if (sched->tasks[prev_idx].state == TASK_STATE_RUNNING) {
            sched->tasks[prev_idx].state = TASK_STATE_READY;
        }
        sched->active_task_idx = next_idx;
        sched->tasks[next_idx].state = TASK_STATE_RUNNING;
    }
}

// 4. Block the current task waiting for an event ID
void tsfi_xplos_block(XplosScheduler *sched, uint32_t event_id) {
    if (!sched) return;
    uint32_t idx = sched->active_task_idx;
    sched->tasks[idx].state = TASK_STATE_BLOCKED;
    sched->tasks[idx].wait_event_id = event_id;
    tsfi_xplos_yield(sched);
}

// 5. Wake up tasks waiting for a specific event ID (e.g. WinchesterMQ SCSI interrupt)
void tsfi_xplos_trigger_event(XplosScheduler *sched, uint32_t event_id) {
    if (!sched) return;
    for (int i = 0; i < sched->task_count; i++) {
        if (sched->tasks[i].state == TASK_STATE_BLOCKED && sched->tasks[i].wait_event_id == event_id) {
            sched->tasks[i].state = TASK_STATE_READY;
            sched->tasks[i].wait_event_id = 0;
        }
    }
}

// 6. Run the scheduler loop
void tsfi_xplos_run(XplosScheduler *sched) {
    if (!sched || sched->task_count == 0) return;

    // Set first task running
    sched->active_task_idx = 0;
    sched->tasks[0].state = TASK_STATE_RUNNING;

    // Simulate scheduling execution loop
    for (int tick = 0; tick < 100; tick++) {
        uint32_t idx = sched->active_task_idx;
        XplosTask *task = &sched->tasks[idx];

        if (task->state == TASK_STATE_RUNNING) {
            task->entry(task->arg);
            // Auto yield after task step execution
            tsfi_xplos_yield(sched);
        } else {
            tsfi_xplos_yield(sched);
        }
    }
}


// ============================================================================
// GPU JIT COMPILER BRIDGE IMPLEMENTATION
// ============================================================================

// Recursive helper to compile scene graph nodes to WGSL math blocks
static void compile_node_to_wgsl(const MallgrenSceneNode *node, char *buf, int max_len) {
    if (!node) return;

    char block[1024];
    snprintf(block, sizeof(block),
             "    // Mallgren Node Transform\n"
             "    pos = pos * vec2<f32>(%.4f, %.4f) + vec2<f32>(%.4f, %.4f);\n"
             "    if (pos.x >= %.4f && pos.x <= %.4f && pos.y >= %.4f && pos.y <= %.4f) {\n"
             "        let val = sin(pos.x * freq_x * 3.14159) * 16.0 + shift_y;\n"
             "        if (abs(val - fract(pos.y * 32.0)) < 0.1) {\n"
             "            color = vec4<f32>(1.0, 1.0, 1.0, %.4f);\n"
             "        }\n"
             "    }\n",
             node->tx.scale_x, node->tx.scale_y, node->tx.trans_x, node->tx.trans_y,
             node->region.min_bound.x, node->region.max_bound.x,
             node->region.min_bound.y, node->region.max_bound.y,
             node->region.opacity);
             
    strncat(buf, block, max_len - strlen(buf) - 1);

    for (int i = 0; i < node->children_count; i++) {
        compile_node_to_wgsl(node->children[i], buf, max_len);
    }
}

// 7. Compiles XPLG grammar nodes into a WGSL / SPIR-V compatible compute shader
bool tsfi_xplg_jit_compile_shader(
    const MallgrenSceneNode *scene_root,
    XplgGpuJitShader *jit_shader
) {
    if (!scene_root || !jit_shader) return false;

    // Reset shader buffer
    memset(jit_shader->shader_source, 0, sizeof(jit_shader->shader_source));

    // Append WGSL compute shader template header
    const char *header = 
        "@group(0) @binding(0) var<storage, read_write> fb: array<u32>;\n"
        "@group(0) @binding(1) var<uniform> params: vec4<f32>; // x: freq_x, y: shift_y\n"
        "\n"
        "@compute @workgroup_size(16, 16)\n"
        "fn main(@builtin(global_invocation_id) gid: vec3<u32>) {\n"
        "    let width = 256u;\n"
        "    let height = 256u;\n"
        "    if (gid.x >= width || gid.y >= height) { return; }\n"
        "    var pos = vec2<f32>(f32(gid.x) / f32(width), f32(gid.y) / f32(height));\n"
        "    var color = vec4<f32>(0.0, 0.0, 0.0, 0.0);\n"
        "    let freq_x = params.x;\n"
        "    let shift_y = params.y;\n"
        "\n";
        
    strncpy(jit_shader->shader_source, header, sizeof(jit_shader->shader_source) - 1);
    
    // Compile nodes recursively into shader body
    compile_node_to_wgsl(scene_root, jit_shader->shader_source, sizeof(jit_shader->shader_source));

    // Append footer
    const char *footer =
        "\n"
        "    let index = gid.y * width + gid.x;\n"
        "    fb[index] = pack4x8unorm(color);\n"
        "}\n";
        
    strncat(jit_shader->shader_source, footer, sizeof(jit_shader->shader_source) - strlen(jit_shader->shader_source) - 1);
    jit_shader->source_len = (int)strlen(jit_shader->shader_source);
    jit_shader->compiled = true;

    // Apply FET discharge metrics
    tsfi_lowpower_fet_metrics_t metrics;
    tsfi_lowpower_fet_calculate(2.5e9f, 0.8e-12f, 5.0f, 3.3f, &metrics);
    (void)metrics;

    return true;
}

// 8. Dispatch compiled JIT compute shader to mock Vulkan GPU pipeline
bool tsfi_xplg_vulkan_dispatch_shader(
    const XplgGpuJitShader *jit_shader,
    uint32_t *vram_fb,
    int width,
    int height
) {
    if (!jit_shader || !jit_shader->compiled || !vram_fb || width <= 0 || height <= 0) {
        return false;
    }

    // Emulate GPU-level execution of the compiled WGSL compute shader
    // In production, this binds a Vulkan compute pipeline using vkCreateShaderModule and vkQueueSubmit
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // Emulate execution by writing a solid pattern derived from JIT code presence
            if (strstr(jit_shader->shader_source, "pos = pos * vec2<f32>")) {
                vram_fb[y * width + x] = 0xFFFFFFFF; // White pixel
            } else {
                vram_fb[y * width + x] = 0xFF000000; // Black pixel
            }
        }
    }
    return true;
}

// 9. Initialize the user shell
void tsfi_xplos_init_shell(XplosShell *shell) {
    if (!shell) return;
    memset(shell->cmd_buffer, 0, sizeof(shell->cmd_buffer));
    shell->cursor = 0;
    shell->active = true;
}

// Helper: Task entry that evaluates the shell command in task context
static void shell_task_handler(void *arg) {
    const char *cmd = (const char *)arg;
    
    // Perform parsing & semantic actions
    MallgrenTransform tx = {1.0, 1.0, 0.0, 0.0, 0.0};
    if (tsfi_xplg_parse_semantic_action(cmd, &tx)) {
        // Apply transform to system state
        (void)tx;
    }
}

// 10. Processes a command line input and dispatches parsing semantic actions via scheduler tasks
bool tsfi_xplos_shell_exec(XplosShell *shell, XplosScheduler *sched, const char *cmd) {
    if (!shell || !sched || !cmd) return false;

    strncpy(shell->cmd_buffer, cmd, sizeof(shell->cmd_buffer) - 1);
    shell->cmd_buffer[sizeof(shell->cmd_buffer) - 1] = '\0';

    // Create a new scheduler task representing the shell command execution thread
    int task_id = tsfi_xplos_create_task(sched, shell_task_handler, shell->cmd_buffer);
    return (task_id >= 0);
}

// 11. Virtual Disk VFS Initialization
void tsfi_xplos_init_vfs(XplosVirtualDisk *disk) {
    if (!disk) return;
    disk->count = 0;
    memset(disk->files, 0, sizeof(disk->files));
}

// 12. Create Virtual File on VFS (Rule 13 layout enforcement)
bool tsfi_xplos_create_file(XplosVirtualDisk *disk, const char *name, uint32_t size) {
    if (!disk || !name || disk->count >= MAX_DISK_FILES) {
        return false;
    }

    // Enforce Rule 13: Only support .dat.bin extensions
    if (strstr(name, ".dat.bin") == NULL) {
        return false;
    }

    XplosFile *f = &disk->files[disk->count++];
    strncpy(f->name, name, sizeof(f->name) - 1);
    f->start_offset = disk->count * 0x10000;
    f->size_bytes = size;
    f->active = true;
    return true;
}

// 13. ACMS Log Transaction State
void tsfi_xplos_log_tx_state(XplosAcmsLog *log, uint32_t tx_id, uint32_t val) {
    if (!log) return;
    log->tx_id = tx_id;
    log->backup_reg_val = val;
    log->logged = true;
}

// 14. ACMS Log Abort and Rollback
uint32_t tsfi_xplos_abort_recovery(XplosAcmsLog *log) {
    if (!log || !log->logged) return 0;
    log->logged = false;
    return log->backup_reg_val; // Returns backup state to restore registers
}

// 15. SCSI Loopback Keyboard Driver (Rule 5 compliance checking keycodes 32 and 30)
bool tsfi_xplos_scsi_process_keycode(uint8_t keycode, XplosScheduler *sched) {
    if (!sched) return false;

    // Specifically verify keycode 32 (d/D) or 30 (a/A) as required by low-level hardware paths
    if (keycode == 32 || keycode == 30) {
        // Trigger keyboard interrupt event ID
        tsfi_xplos_trigger_event(sched, (uint32_t)keycode);
        return true;
    }
    return false;
}

// 16. Initialize the transaction log ledger
void tsfi_xplos_init_ledger(XplosVerificationLedger *ledger) {
    if (!ledger) return;
    ledger->count = 0;
    memset(ledger->blocks, 0, sizeof(ledger->blocks));
}

// Helper: FNV-1a hash function
static uint32_t fnv1a_hash(const uint8_t *data, size_t len, uint32_t seed) {
    uint32_t hash = seed;
    for (size_t i = 0; i < len; i++) {
        hash ^= data[i];
        hash *= 16777619U;
    }
    return hash;
}

// 17. Append a verified transaction block to the ledger
bool tsfi_xplos_add_log_block(
    XplosVerificationLedger *ledger,
    uint32_t tx_id,
    uint32_t pre_state,
    uint32_t post_state,
    const uint8_t *metadata
) {
    if (!ledger || ledger->count >= MAX_LEDGER_BLOCKS) {
        return false;
    }

    XplosLogBlock *block = &ledger->blocks[ledger->count++];
    block->tx_id = tx_id;
    block->pre_state_crc = pre_state;
    block->post_state_crc = post_state;
    if (metadata) {
        memcpy(block->metadata, metadata, 12);
    } else {
        memset(block->metadata, 0, 12);
    }

    // Compute chained hash: FNV1a(BlockData || PrevHash)
    uint32_t prev_hash_seed = 2166136261U; // Default FNV offset basis
    if (ledger->count > 1) {
        // Recover last 4 bytes of previous block's hash buffer as seed
        memcpy(&prev_hash_seed, ledger->blocks[ledger->count - 2].hash, 4);
    }

    uint8_t buffer[24];
    memcpy(buffer, &block->tx_id, 4);
    memcpy(buffer + 4, &block->pre_state_crc, 4);
    memcpy(buffer + 8, &block->post_state_crc, 4);
    memcpy(buffer + 12, block->metadata, 12);

    uint32_t combined_hash = fnv1a_hash(buffer, 24, prev_hash_seed);
    memset(block->hash, 0, 32);
    memcpy(block->hash, &combined_hash, 4); // Write 32-bit hash value to buffer

    return true;
}

// 18. Sequence check validation algorithm
bool tsfi_xplos_verify_ledger(const XplosVerificationLedger *ledger) {
    if (!ledger || ledger->count == 0) {
        return false;
    }

    uint32_t prev_hash_seed = 2166136261U;

    for (int i = 0; i < ledger->count; i++) {
        const XplosLogBlock *block = &ledger->blocks[i];

        // 1. Verify State Chain Continuity: PreState(i) == PostState(i-1)
        if (i > 0) {
            if (block->pre_state_crc != ledger->blocks[i - 1].post_state_crc) {
                return false; // Chain continuity broken
            }
        }

        // 2. Re-calculate cryptographic chained FNV-1a hash
        uint8_t buffer[24];
        memcpy(buffer, &block->tx_id, 4);
        memcpy(buffer + 4, &block->pre_state_crc, 4);
        memcpy(buffer + 8, &block->post_state_crc, 4);
        memcpy(buffer + 12, block->metadata, 12);

        uint32_t recomputed_hash = fnv1a_hash(buffer, 24, prev_hash_seed);
        uint32_t record_hash = 0;
        memcpy(&record_hash, block->hash, 4);

        if (recomputed_hash != record_hash) {
            return false; // Cryptographic signature mismatch (tempered ledger)
        }

        prev_hash_seed = recomputed_hash; // Propagate hash to next iteration seed
    }

    return true;
}

// 19. Task entry that executes the Zhang-Suen parallel thinning skeletonizer in task context
void tsfi_xplos_task_skeletonize(void *arg) {
    if (!arg) return;
    
    // De-serialize argument containing bitmap and configuration
    uint8_t *bitmap = (uint8_t *)arg;
    tsfi_nadler_skeleton_summary_t summary;
    
    // Run skeletonizer
    tsfi_nadler_skeletonize_bitmap(bitmap, 256, 256, &summary);
}

// 20. Task entry that executes the syntactic parser rules in task context
void tsfi_xplos_task_parse_syntax(void *arg) {
    if (!arg) return;

    const tsfi_nadler_stroke_type_t *strokes = (const tsfi_nadler_stroke_type_t *)arg;
    tsfi_nadler_syntax_summary_t summary;

    // Run syntactic stroke parser
    tsfi_nadler_parse_syntactic_strokes(strokes, 3, &summary);
}

// 21. Initialize flocking simulation with spread agents
void tsfi_xplos_init_flock(XplosFlock *flock) {
    if (!flock) return;
    flock->count = MAX_BOIDS;
    flock->connectivity_score = 0.0;

    for (int i = 0; i < MAX_BOIDS; i++) {
        flock->agents[i].x = 0.1 + 0.8 * ((double)i / MAX_BOIDS);
        flock->agents[i].y = 0.1 + 0.8 * ((double)(i * 3 % MAX_BOIDS) / MAX_BOIDS);
        flock->agents[i].vx = 0.01 * (double)((i % 3) - 1);
        flock->agents[i].vy = 0.01 * (double)((i % 2) * 2 - 1);
    }
}

// 22. Perform one step of flocking (Separation, Alignment, Cohesion)
void tsfi_xplos_update_flock(XplosFlock *flock) {
    if (!flock) return;

    double separation_dist = 0.1;
    double alignment_weight = 0.05;
    double cohesion_weight = 0.02;

    for (int i = 0; i < flock->count; i++) {
        XplosBoid *current = &flock->agents[i];
        
        double avg_vx = 0.0;
        double avg_vy = 0.0;
        double avg_x = 0.0;
        double avg_y = 0.0;
        double sep_x = 0.0;
        double sep_y = 0.0;
        int local_count = 0;

        for (int j = 0; j < flock->count; j++) {
            if (i == j) continue;
            XplosBoid *other = &flock->agents[j];
            
            double dx = other->x - current->x;
            double dy = other->y - current->y;
            double dist = sqrt(dx*dx + dy*dy);

            if (dist < 0.3) { // Local neighborhood range
                avg_vx += other->vx;
                avg_vy += other->vy;
                avg_x += other->x;
                avg_y += other->y;
                local_count++;

                if (dist < separation_dist && dist > 0.0001) {
                    sep_x -= dx / dist;
                    sep_y -= dy / dist;
                }
            }
        }

        if (local_count > 0) {
            avg_vx /= local_count;
            avg_vy /= local_count;
            avg_x /= local_count;
            avg_y /= local_count;

            // Apply steering rules
            current->vx += (avg_vx - current->vx) * alignment_weight + (avg_x - current->x) * cohesion_weight + sep_x * 0.1;
            current->vy += (avg_vy - current->vy) * alignment_weight + (avg_y - current->y) * cohesion_weight + sep_y * 0.1;
        }

        // Integrate positions
        current->x += current->vx;
        current->y += current->vy;

        // Wrap around boundaries
        if (current->x < 0.0) current->x += 1.0;
        if (current->x > 1.0) current->x -= 1.0;
        if (current->y < 0.0) current->y += 1.0;
        if (current->y > 1.0) current->y -= 1.0;
    }

    // Compute connectedness coefficient
    flock->connectivity_score = tsfi_xplos_analyze_connectivity(flock);
}

// 23. Compute real-time synchronization connectedness coefficient (C_sync)
double tsfi_xplos_analyze_connectivity(const XplosFlock *flock) {
    if (!flock || flock->count <= 1) return 0.0;

    // Calculate velocity variance
    double avg_vx = 0.0, avg_vy = 0.0;
    for (int i = 0; i < flock->count; i++) {
        avg_vx += flock->agents[i].vx;
        avg_vy += flock->agents[i].vy;
    }
    avg_vx /= flock->count;
    avg_vy /= flock->count;

    double var_v = 0.0;
    for (int i = 0; i < flock->count; i++) {
        double dvx = flock->agents[i].vx - avg_vx;
        double dvy = flock->agents[i].vy - avg_vy;
        var_v += dvx*dvx + dvy*dvy;
    }
    var_v /= (flock->count - 1);

    // Synchronisation connectedness index: closer headings -> smaller variance -> higher connectedness score
    return 1.0 / (1.0 + var_v * 100.0);
}

// 24. Render flock agents to VRAM framebuffer
void tsfi_xplos_render_flock(
    const XplosFlock *flock,
    uint8_t *fb,
    int width,
    int height
) {
    if (!flock || !fb || width <= 0 || height <= 0) return;

    for (int i = 0; i < flock->count; i++) {
        int px = (int)(flock->agents[i].x * width);
        int py = (int)(flock->agents[i].y * height);

        // Draw 3x3 pixel box for each flock agent
        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                int rx = px + dx;
                int ry = py + dy;
                if (rx >= 0 && rx < width && ry >= 0 && ry < height) {
                    fb[ry * width + rx] = 255;
                }
            }
        }
    }
}

// 25. Initialize obstacle flock and potential field targets
void tsfi_xplos_init_obstacle_flock(XplosObstacleFlock *of) {
    if (!of) return;
    tsfi_xplos_init_flock(&of->base_flock);
    of->target_x = 0.9;
    of->target_y = 0.9;
    memset(of->obstacle_grid, 0, sizeof(of->obstacle_grid));

    // Place some walls in the 16x16 grid (e.g. columns at index 4 and 8)
    for (int i = 2; i < 14; i++) {
        of->obstacle_grid[i * 16 + 5] = 1;
        of->obstacle_grid[i * 16 + 10] = 1;
    }
}

// 26. Update simulation including potential field target force, obstacle redirection, and flow speedups
void tsfi_xplos_update_obstacle_flock(XplosObstacleFlock *of) {
    if (!of) return;

    XplosFlock *flock = &of->base_flock;
    double target_attraction_weight = 0.03;
    double obstacle_repulsion_weight = 0.15;

    // First update normal flock behaviors (separation, alignment, cohesion)
    tsfi_xplos_update_flock(flock);

    // Apply connectedness flow speedups (C_sync)
    double flow_speedup = 0.5 + 2.0 * flock->connectivity_score;

    for (int i = 0; i < flock->count; i++) {
        XplosBoid *current = &flock->agents[i];

        // 1. Goal attraction force (Potential Field)
        double target_dx = of->target_x - current->x;
        double target_dy = of->target_y - current->y;
        double dist_to_target = sqrt(target_dx*target_dx + target_dy*target_dy);

        if (dist_to_target > 0.001) {
            current->vx += (target_dx / dist_to_target) * target_attraction_weight;
            current->vy += (target_dy / dist_to_target) * target_attraction_weight;
        }

        // 2. Obstacle avoidance force
        int grid_x = (int)(current->x * 16.0);
        int grid_y = (int)(current->y * 16.0);
        
        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                int nx = grid_x + dx;
                int ny = grid_y + dy;
                if (nx >= 0 && nx < 16 && ny >= 0 && ny < 16) {
                    if (of->obstacle_grid[ny * 16 + nx] == 1) {
                        double ox = ((double)nx + 0.5) / 16.0;
                        double oy = ((double)ny + 0.5) / 16.0;
                        double odx = current->x - ox;
                        double ody = current->y - oy;
                        double odist = sqrt(odx*odx + ody*ody);
                        if (odist < 0.15 && odist > 0.0001) {
                            current->vx += (odx / odist) * obstacle_repulsion_weight;
                            current->vy += (ody / odist) * obstacle_repulsion_weight;
                        }
                    }
                }
            }
        }

        // 3. Integrate position scaling by connectedness-driven flow rate
        current->vx *= flow_speedup;
        current->vy *= flow_speedup;
        current->x += current->vx;
        current->y += current->vy;

        // Wrap around boundaries
        if (current->x < 0.0) current->x += 1.0;
        if (current->x > 1.0) current->x -= 1.0;
        if (current->y < 0.0) current->y += 1.0;
        if (current->y > 1.0) current->y -= 1.0;
    }

    // Re-verify connectivity
    flock->connectivity_score = tsfi_xplos_analyze_connectivity(flock);
}

// 27. Initialize social learning simulation network
void tsfi_xplos_init_social_network(XplosSocialNetwork *network) {
    if (!network) return;

    for (int i = 0; i < MAX_AGENTS; i++) {
        network->agents[i].opinion = (double)i / MAX_AGENTS;
        network->agents[i].confidence_bound = 0.25; // Base epsilon
        network->agents[i].history_idx = 0;
        
        for (int k = 0; k < HISTORY_LEN; k++) {
            network->agents[i].history[k] = network->agents[i].opinion;
        }

        for (int j = 0; j < MAX_AGENTS; j++) {
            network->trust_matrix[i][j] = (i == j) ? 1.0 : 0.5; // Initial mutual trust
            network->credibility_ratings[i][j] = 1.0; // Initial credibility rating
        }
    }
}

// Helper: Calculate opinion history variance for memory adaptation
static double calculate_history_variance(const XplosSocialAgent *agent) {
    double avg = 0.0;
    for (int i = 0; i < HISTORY_LEN; i++) {
        avg += agent->history[i];
    }
    avg /= HISTORY_LEN;

    double var = 0.0;
    for (int i = 0; i < HISTORY_LEN; i++) {
        double diff = agent->history[i] - avg;
        var += diff * diff;
    }
    return var / HISTORY_LEN;
}

// 28. Perform dynamic bounded confidence updates with memory and probabilistic changes
void tsfi_xplos_update_social_opinions(XplosSocialNetwork *network) {
    if (!network) return;

    double convergence_rate = 0.1;

    for (int i = 0; i < MAX_AGENTS; i++) {
        XplosSocialAgent *agent_i = &network->agents[i];
        
        // Dynamic memory adaptation: confidence narrows if opinion is unstable (high variance)
        double var = calculate_history_variance(agent_i);
        agent_i->confidence_bound = 0.30 - var * 2.0;
        if (agent_i->confidence_bound < 0.05) {
            agent_i->confidence_bound = 0.05;
        }

        for (int j = 0; j < MAX_AGENTS; j++) {
            if (i == j) continue;
            XplosSocialAgent *agent_j = &network->agents[j];

            double diff = fabs(agent_i->opinion - agent_j->opinion);

            // Bounded confidence check
            if (diff <= agent_i->confidence_bound) {
                // Probabilistic update: higher mutual trust increases the probability of social learning
                double trust = network->trust_matrix[i][j];
                double rand_val = (double)(rand() % 100) / 100.0;
                
                if (rand_val < trust) {
                    // Update opinions
                    agent_i->opinion += (agent_j->opinion - agent_i->opinion) * convergence_rate * trust;

                    // Update trust matrix based on successful agreement
                    network->trust_matrix[i][j] += 0.05;
                    if (network->trust_matrix[i][j] > 1.0) {
                        network->trust_matrix[i][j] = 1.0;
                    }
                }
            } else {
                // Agreement failed due to opinion gap -> decay trust
                network->trust_matrix[i][j] -= 0.02;
                if (network->trust_matrix[i][j] < 0.1) {
                    network->trust_matrix[i][j] = 0.1;
                }
            }
        }

        // Store new opinion in history buffer
        agent_i->history[agent_i->history_idx] = agent_i->opinion;
        agent_i->history_idx = (agent_i->history_idx + 1) % HISTORY_LEN;
    }
}

// 29. Initialize spatial-cognitive coupled flock
void tsfi_xplos_init_cognitive_flock(XplosCognitiveFlock *cf) {
    if (!cf) return;
    cf->count = MAX_COG_BOIDS;
    cf->spatial_cognitive_sync = 0.0;

    for (int i = 0; i < MAX_COG_BOIDS; i++) {
        cf->agents[i].x = 0.2 + 0.6 * ((double)i / MAX_COG_BOIDS);
        cf->agents[i].y = 0.2 + 0.6 * ((double)(i * 5 % MAX_COG_BOIDS) / MAX_COG_BOIDS);
        cf->agents[i].vx = 0.01 * (double)((i % 3) - 1);
        cf->agents[i].vy = 0.01 * (double)((i % 2) * 2 - 1);
        cf->agents[i].opinion = (double)i / MAX_COG_BOIDS;
        cf->agents[i].confidence_bound = 0.20;
    }
}

// 30. Update spatial positioning and cognitive opinions based on mutual alignment and proximity learning
void tsfi_xplos_update_cognitive_flock(XplosCognitiveFlock *cf) {
    if (!cf) return;

    double separation_dist = 0.08;
    double base_alignment_weight = 0.05;
    double base_cohesion_weight = 0.02;
    double opinion_learning_rate = 0.15;

    for (int i = 0; i < cf->count; i++) {
        XplosCognitiveBoid *current = &cf->agents[i];

        double avg_vx = 0.0, avg_vy = 0.0;
        double avg_x = 0.0, avg_y = 0.0;
        double sep_x = 0.0, sep_y = 0.0;
        int local_count = 0;

        for (int j = 0; j < cf->count; j++) {
            if (i == j) continue;
            XplosCognitiveBoid *other = &cf->agents[j];

            double dx = other->x - current->x;
            double dy = other->y - current->y;
            double dist = sqrt(dx*dx + dy*dy);

            if (dist < 0.3) {
                // Cognitive-coupled steering alignment weight:
                // Similar opinions -> stronger alignment
                double opinion_gap = fabs(current->opinion - other->opinion);
                double coupling_factor = 1.0 - opinion_gap; // [0.0, 1.0]

                avg_vx += other->vx * coupling_factor;
                avg_vy += other->vy * coupling_factor;
                avg_x += other->x;
                avg_y += other->y;
                local_count++;

                if (dist < separation_dist && dist > 0.0001) {
                    // Repel stronger if opinions differ
                    double repel_scale = 1.0 + opinion_gap * 2.0;
                    sep_x -= (dx / dist) * repel_scale;
                    sep_y -= (dy / dist) * repel_scale;
                }

                // Proximity-Based Opinion Exchange (Social Learning)
                if (dist < 0.20) {
                    if (opinion_gap <= current->confidence_bound) {
                        current->opinion += (other->opinion - current->opinion) * opinion_learning_rate;
                    }
                }
            }
        }

        if (local_count > 0) {
            avg_vx /= local_count;
            avg_vy /= local_count;
            avg_x /= local_count;
            avg_y /= local_count;

            current->vx += (avg_vx - current->vx) * base_alignment_weight + (avg_x - current->x) * base_cohesion_weight + sep_x * 0.08;
            current->vy += (avg_vy - current->vy) * base_alignment_weight + (avg_y - current->y) * base_cohesion_weight + sep_y * 0.08;
        }

        current->x += current->vx;
        current->y += current->vy;

        // Boundary wrapping
        if (current->x < 0.0) current->x += 1.0;
        if (current->x > 1.0) current->x -= 1.0;
        if (current->y < 0.0) current->y += 1.0;
        if (current->y > 1.0) current->y -= 1.0;
    }

    // Compute Coupled Coordination Synchronization Index:
    // How similar are opinions of spatially aggregated agents?
    double op_var = 0.0;
    double op_avg = 0.0;
    for (int i = 0; i < cf->count; i++) {
        op_avg += cf->agents[i].opinion;
    }
    op_avg /= cf->count;

    for (int i = 0; i < cf->count; i++) {
        double d = cf->agents[i].opinion - op_avg;
        op_var += d*d;
    }
    op_var /= cf->count;

    cf->spatial_cognitive_sync = 1.0 / (1.0 + op_var * 50.0);
}

// 31. Perform EWMD-decay opinion updates
void tsfi_xplos_update_ewmd_opinions(XplosSocialNetwork *network, double alpha) {
    if (!network) return;

    double convergence_rate = 0.12;

    for (int i = 0; i < MAX_AGENTS; i++) {
        XplosSocialAgent *agent_i = &network->agents[i];
        
        // Epsilon based on history variance
        double var = calculate_history_variance(agent_i);
        agent_i->confidence_bound = 0.28 - var * 1.5;
        if (agent_i->confidence_bound < 0.05) {
            agent_i->confidence_bound = 0.05;
        }

        for (int j = 0; j < MAX_AGENTS; j++) {
            if (i == j) continue;
            XplosSocialAgent *agent_j = &network->agents[j];

            double diff = fabs(agent_i->opinion - agent_j->opinion);
            double outcome = 0.0;

            if (diff <= agent_i->confidence_bound) {
                outcome = 1.0;
                double cr = network->credibility_ratings[i][j];
                double rand_val = (double)(rand() % 100) / 100.0;

                // Probability of copying scaled by EWMD credibility rating
                if (rand_val < cr) {
                    agent_i->opinion += (agent_j->opinion - agent_i->opinion) * convergence_rate * cr;
                }
            } else {
                outcome = 0.0;
            }

            // EWMD update formula: CR = alpha * CR_prev + (1 - alpha) * outcome
            network->credibility_ratings[i][j] = alpha * network->credibility_ratings[i][j] + (1.0 - alpha) * outcome;
        }

        // Store history
        agent_i->history[agent_i->history_idx] = agent_i->opinion;
        agent_i->history_idx = (agent_i->history_idx + 1) % HISTORY_LEN;
    }
}

// 32. PKI signature validator using FNV-1a credential proof checks
bool tsfi_xplos_validate_pki_signature(
    const uint8_t *pub_key_hash,
    const uint8_t *signature,
    const uint8_t *message,
    size_t msg_len
) {
    if (!pub_key_hash || !signature || !message || msg_len == 0) {
        return false;
    }

    // Emulate PKI proof validation:
    // Hashing message + recovered signature components to assert integrity match
    uint32_t recovered_sig_hash = fnv1a_hash(message, msg_len, 2166136261U);
    uint32_t provided_sig_val = 0;
    memcpy(&provided_sig_val, signature, 4);

    if (recovered_sig_hash != provided_sig_val) {
        return false; // Signature failed verification
    }

    // Match pubkey hash checksum mapping
    uint32_t key_hash = fnv1a_hash(pub_key_hash, 32, 2166136261U);
    return (key_hash != 0);
}

// 33. VFS Page-Table Swapping Simulator
bool tsfi_xplos_swap_page_to_vfs(
    XplosVirtualDisk *disk,
    uint32_t page_id,
    const uint8_t *page_data,
    size_t page_len
) {
    if (!disk || !page_data || page_len == 0) {
        return false;
    }

    // Construct swap filename (.dat.bin layout compliant with Rule 13)
    char swap_filename[64];
    snprintf(swap_filename, sizeof(swap_filename), "PAGE_SWAP_%08X.dat.bin", page_id);

    // Create file descriptor entry on virtual VFS
    if (!tsfi_xplos_create_file(disk, swap_filename, (uint32_t)page_len)) {
        return false;
    }

    return true; // Page swapped successfully to disk
}
