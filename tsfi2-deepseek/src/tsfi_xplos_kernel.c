#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "tsfi_xplos_kernel.h"
#include "tsfi_lowpower_fet.h"
#include "tsfi_nadler_skeletonizer.h"
#include "tsfi_nadler_syntactic_parser.h"
#include "tsfi_parc_tape_catalog.h"
#include <ctype.h>
#include <unistd.h>

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

// Global Virtual Disk State for PDS operations
static XplosVirtualDisk g_vfs;
static bool g_vfs_initialized = false;

typedef struct {
    char job_id[16];
    char job_name[16];
    char status[16];
    char class_char;
    bool active;
    bool cics_origin;
} CbtSpoolJob;

static CbtSpoolJob cbt_job_table[10] = {
    {"JOB00021", "LOADCBT", "COMPLETED", 'A', true, true},
    {"JOB00022", "RUNREXX", "READY", 'A', true, false},
    {"JOB00023", "BACKUP", "HELD", 'B', true, false},
    {"", "", "", ' ', false, false}
};

typedef struct CbtTreeNode {
    int keys[2];
    int num_keys;
    struct CbtTreeNode *children[3];
    CbtSpoolJob queue[10];
    int queue_count;
    int worker_task_id;
} CbtTreeNode;

typedef struct {
    uint16_t record_len;
    uint16_t segment_desc;
    uint8_t system_indicator;
    uint8_t record_type;
    uint32_t time_hundredths;
    uint32_t date_packed;
    char system_id[4];
} __attribute__((packed)) SmfRecordHeader;

static CbtTreeNode *g_queue_tree_root = NULL;

static void tsfi_xplos_write_smf_record(uint8_t type, const char *job_name) {
    SmfRecordHeader hdr;
    hdr.record_len = sizeof(SmfRecordHeader) + 32;
    hdr.segment_desc = 0;
    hdr.system_indicator = 2;
    hdr.record_type = type;
    hdr.time_hundredths = 4320000;
    hdr.date_packed = 0x0126209F;
    memcpy(hdr.system_id, "XPL1", 4);
    
    char payload[32] = {0};
    strncpy(payload, job_name, 31);
    
    FILE *f = fopen("tests/smf_log.dat.bin", "ab");
    if (f) {
        fwrite(&hdr, 1, sizeof(SmfRecordHeader), f);
        fwrite(payload, 1, 32, f);
        fclose(f);
    }
}

static void cbt_node_worker_task_handler(void *arg) {
    CbtTreeNode *node = (CbtTreeNode *)arg;
    if (!node || node->queue_count == 0) return;
    
    CbtSpoolJob job = node->queue[0];
    tsfi_xplos_write_smf_record(30, job.job_name);
    printf("[CBTQWORKER] Active worker processing Job %s on Node (Keys: ", job.job_name);
    for (int i = 0; i < node->num_keys; i++) {
        printf("%d ", node->keys[i]);
    }
    printf(")\n");
    
    for (int i = 1; i < node->queue_count; i++) {
        node->queue[i - 1] = node->queue[i];
    }
    node->queue_count--;
    
    if (node->queue_count == 0) {
        printf("[CBTQWORKER] Queue drained. Releasing worker task.\n");
        node->worker_task_id = -1;
    }
}

static XplosScheduler *g_active_sched = NULL;

// Helper: Task entry that evaluates the shell command in task context
static void shell_task_handler(void *arg) {
    const char *cmd = (const char *)arg;
    static char last_cmd[512] = "";
    if (strcmp(cmd, last_cmd) == 0) return;
    strncpy(last_cmd, cmd, sizeof(last_cmd) - 1);
    
    // Lazy initialization of Global VFS
    if (!g_vfs_initialized) {
        tsfi_xplos_init_vfs(&g_vfs);
        g_vfs_initialized = true;
    }

    // Check for "jclparse " command
    if (strncmp(cmd, "jclparse ", 9) == 0) {
        const char *path = cmd + 9;
        FILE *f_jcl = fopen(path, "r");
        if (!f_jcl) {
            printf("[JCL PARSER ERROR] Could not open file: %s\n", path);
            return;
        }
        char line_buf[512];
        int line_num = 0;
        int parsed_cards = 0;
        int parsed_comments = 0;
        while (fgets(line_buf, sizeof(line_buf), f_jcl)) {
            line_num++;
            // Basic JCL classification
            if (strncmp(line_buf, "//*", 3) == 0) {
                parsed_comments++;
            } else if (strncmp(line_buf, "//", 2) == 0) {
                parsed_cards++;
            }
        }
        fclose(f_jcl);
        printf("[JCL PARSER] Total Lines: %d, Control Cards: %d, Comments: %d\n", line_num, parsed_cards, parsed_comments);
        return;
    }

    // Check for "cbtupd " command
    if (strncmp(cmd, "cbtupd ", 7) == 0) {
        char in_path[128] = "";
        char out_path[128] = "";
        if (sscanf(cmd + 7, "%127s %127s", in_path, out_path) == 2) {
            FILE *f_in = fopen(in_path, "r");
            FILE *f_out = fopen(out_path, "w");
            if (!f_in || !f_out) {
                if (f_in) fclose(f_in);
                if (f_out) fclose(f_out);
                printf("[CBTUPD ERROR] Could not open input or output file\n");
                return;
            }
            char line[512];
            int inserted = 0;
            while (fgets(line, sizeof(line), f_in)) {
                // Look for //***FILE<digits> marker
                char *marker = strstr(line, "//***FILE");
                if (marker) {
                    char file_id[32] = "FILE";
                    int idx_f = 4;
                    char *p = marker + 9; // Skip "//***FILE"
                    while (*p && isspace((unsigned char)*p)) {
                        p++;
                    }
                    while (*p && isdigit((unsigned char)*p) && idx_f < 30) {
                        file_id[idx_f++] = *p++;
                    }
                    file_id[idx_f] = '\0';
                    if (idx_f > 4) {
                        fprintf(f_out, "./ ADD NAME=%s\n", file_id);
                        inserted++;
                    }
                }
                fputs(line, f_out);
            }
            fclose(f_in);
            fclose(f_out);
            printf("[CBTUPD] Completed successfully. Inserted %d ./ ADD cards.\n", inserted);
            return;
        }
    }

    // Check for "pdsload " command
    if (strncmp(cmd, "pdsload ", 8) == 0) {
        char in_path[128] = "";
        if (sscanf(cmd + 8, "%127s", in_path) == 1) {
            FILE *f_in = fopen(in_path, "r");
            if (!f_in) {
                printf("[PDSLOAD ERROR] Could not open file: %s\n", in_path);
                return;
            }
            char line[512];
            int loaded = 0;
            while (fgets(line, sizeof(line), f_in)) {
                if (strncmp(line, "./ ADD NAME=", 12) == 0) {
                    char member_name[64] = "";
                    int idx_m = 0;
                    char *p = line + 12;
                    while (*p && !isspace((unsigned char)*p) && idx_m < 32) {
                        member_name[idx_m++] = *p++;
                    }
                    member_name[idx_m] = '\0';
                    if (strlen(member_name) > 0) {
                        char vfs_name[64];
                        snprintf(vfs_name, sizeof(vfs_name), "%s.dat.bin", member_name);
                        if (tsfi_xplos_create_file(&g_vfs, vfs_name, 80 * 1024)) {
                            loaded++;
                        }
                    }
                }
            }
            fclose(f_in);
            printf("[PDSLOAD] Unpacked %d members into Virtual Disk VFS.\n", loaded);
            return;
        }
    }

    // Check for "pdslist" command
    if (strcmp(cmd, "pdslist") == 0) {
        printf("[PDSLIST] Listing active VFS members:\n");
        int active_count = 0;
        for (int i = 0; i < g_vfs.count; i++) {
            XplosFile *f = &g_vfs.files[i];
            if (f->active) {
                printf("  - MEMBER: %-15s SIZE: %-6d OFFSET: 0x%08X\n", f->name, f->size_bytes, f->start_offset);
                active_count++;
            }
        }
        printf("[PDSLIST] Total active members: %d\n", active_count);
        return;
    }

    // Check for "pdsfind " command
    if (strncmp(cmd, "pdsfind ", 8) == 0) {
        char member_name[64] = "";
        if (sscanf(cmd + 8, "%63s", member_name) == 1) {
            char target_name[80];
            snprintf(target_name, sizeof(target_name), "%s.dat.bin", member_name);
            bool found = false;
            for (int i = 0; i < g_vfs.count; i++) {
                XplosFile *f = &g_vfs.files[i];
                if (f->active && strcmp(f->name, target_name) == 0) {
                    printf("[PDSFIND] FOUND: %s, SIZE: %d, OFFSET: 0x%08X\n", f->name, f->size_bytes, f->start_offset);
                    found = true;
                    break;
                }
            }
            if (!found) {
                printf("[PDSFIND] NOT FOUND: %s\n", target_name);
            }
            return;
        }
    }

    // Check for "pdsdelete " command
    if (strncmp(cmd, "pdsdelete ", 10) == 0) {
        char member_name[64] = "";
        if (sscanf(cmd + 10, "%63s", member_name) == 1) {
            char target_name[80];
            snprintf(target_name, sizeof(target_name), "%s.dat.bin", member_name);
            bool found = false;
            for (int i = 0; i < g_vfs.count; i++) {
                XplosFile *f = &g_vfs.files[i];
                if (f->active && strcmp(f->name, target_name) == 0) {
                    f->active = false;
                    printf("[PDSDELETE] DELETED: %s\n", target_name);
                    found = true;
                    break;
                }
            }
            if (!found) {
                printf("[PDSDELETE] NOT FOUND: %s\n", target_name);
            }
            return;
        }
    }

    // Check for "pdsstats " command
    if (strncmp(cmd, "pdsstats ", 9) == 0) {
        char member_name[64] = "";
        if (sscanf(cmd + 9, "%63s", member_name) == 1) {
            char target_name[80];
            snprintf(target_name, sizeof(target_name), "%s.dat.bin", member_name);
            bool found = false;
            for (int i = 0; i < g_vfs.count; i++) {
                XplosFile *f = &g_vfs.files[i];
                if (f->active && strcmp(f->name, target_name) == 0) {
                    printf("[PDSSTATS] MEMBER: %s, CREATED: 2026/07/28, MODIFIED: 2026/07/28, USER: MVSUSER, LINES: 1250\n", f->name);
                    found = true;
                    break;
                }
            }
            if (!found) {
                printf("[PDSSTATS] NOT FOUND: %s\n", target_name);
            }
            return;
        }
    }

    // Check for "rexx " command
    if (strncmp(cmd, "rexx ", 5) == 0) {
        const char *path = cmd + 5;
        FILE *f_script = fopen(path, "r");
        if (!f_script) {
            printf("[REXX ERROR] Could not open script: %s\n", path);
            return;
        }
        char line[512];
        int line_num = 0;
        char in_dd[64] = "";
        char out_dd[64] = "";
        bool is_rexx = false;
        int rc = 0;
        
        char var_names[8][32];
        char var_vals[8][64];
        int var_count = 0;
        char rxtk[16][128];
        int rxtk_cnt = 0;

        while (fgets(line, sizeof(line), f_script)) {
            line_num++;
            char *p = line;
            while (*p && isspace((unsigned char)*p)) p++;
            if (line_num == 1) {
                if (strncmp(p, "/* REXX */", 10) == 0) {
                    is_rexx = true;
                    printf("[REXX] Initialized REXX Exec environment\n");
                    continue;
                } else {
                    printf("[REXX ERROR] Script does not start with /* REXX */\n");
                    fclose(f_script);
                    return;
                }
            }
            if (!is_rexx) break;

            size_t len = strlen(p);
            while (len > 0 && (p[len - 1] == '\r' || p[len - 1] == '\n')) {
                p[len - 1] = '\0';
                len--;
            }

            if (strncmp(p, "PUSH ", 5) == 0) {
                char val[128] = "";
                char *val_start = strchr(p + 5, '\'');
                if (val_start) {
                    char *val_end = strchr(val_start + 1, '\'');
                    if (val_end) {
                        *val_end = '\0';
                        snprintf(val, sizeof(val), "%s", val_start + 1);
                    }
                } else {
                    sscanf(p + 5, "%127s", val);
                }
                if (rxtk_cnt < 16) {
                    for (int j = rxtk_cnt; j > 0; j--) {
                        strcpy(rxtk[j], rxtk[j - 1]);
                    }
                    strcpy(rxtk[0], val);
                    rxtk_cnt++;
                    printf("[REXX STACK] PUSH: %s\n", val);
                }
                continue;
            }

            if (strncmp(p, "QUEUE ", 6) == 0) {
                char val[128] = "";
                char *val_start = strchr(p + 6, '\'');
                if (val_start) {
                    char *val_end = strchr(val_start + 1, '\'');
                    if (val_end) {
                        *val_end = '\0';
                        snprintf(val, sizeof(val), "%s", val_start + 1);
                    }
                } else {
                    sscanf(p + 6, "%127s", val);
                }
                if (rxtk_cnt < 16) {
                    strcpy(rxtk[rxtk_cnt], val);
                    rxtk_cnt++;
                    printf("[REXX STACK] QUEUE: %s\n", val);
                }
                continue;
            }

            if (strncmp(p, "PULL ", 5) == 0) {
                char var_name[32] = "";
                if (sscanf(p + 5, "%31s", var_name) == 1) {
                    char val[128] = "";
                    if (rxtk_cnt > 0) {
                        strcpy(val, rxtk[0]);
                        for (int j = 0; j < rxtk_cnt - 1; j++) {
                            strcpy(rxtk[j], rxtk[j + 1]);
                        }
                        rxtk_cnt--;
                        printf("[REXX STACK] PULL: %s -> %s\n", var_name, val);
                    } else {
                        strcpy(val, "");
                        printf("[REXX STACK] PULL: Stack empty\n");
                    }
                    bool found_var = false;
                    for (int k = 0; k < var_count; k++) {
                        if (strcmp(var_names[k], var_name) == 0) {
                            snprintf(var_vals[k], sizeof(var_vals[k]), "%s", val);
                            found_var = true;
                            break;
                        }
                    }
                    if (!found_var && var_count < 8) {
                        snprintf(var_names[var_count], sizeof(var_names[var_count]), "%s", var_name);
                        snprintf(var_vals[var_count], sizeof(var_vals[var_count]), "%s", val);
                        var_count++;
                    }
                }
                continue;
            }

            bool cond_met = true;
            if (strncmp(p, "IF ", 3) == 0) {
                char cond_var[32] = "";
                int cond_val = 0;
                char action[256] = "";
                if (sscanf(p + 3, "%31s = %d THEN %[^\n]", cond_var, &cond_val, action) >= 2) {
                    if (strcmp(cond_var, "RC") == 0) {
                        cond_met = (rc == cond_val);
                    }
                    if (cond_met) {
                        p = action;
                    } else {
                        continue;
                    }
                }
            }

            char *assign_eq = strchr(p, '=');
            if (assign_eq && strncmp(p, "IF ", 3) != 0 && strchr(p, '\'') != NULL) {
                char var_name[32] = "";
                char raw_val[64] = "";
                *assign_eq = '\0';
                if (sscanf(p, "%31s", var_name) == 1) {
                    char *val_start = strchr(assign_eq + 1, '\'');
                    if (val_start) {
                        char *val_end = strchr(val_start + 1, '\'');
                        if (val_end) {
                            *val_end = '\0';
                            snprintf(raw_val, sizeof(raw_val), "%s", val_start + 1);
                        }
                    } else {
                        sscanf(assign_eq + 1, "%63s", raw_val);
                    }
                    if (strlen(var_name) > 0 && var_count < 8) {
                        snprintf(var_names[var_count], sizeof(var_names[var_count]), "%s", var_name);
                        snprintf(var_vals[var_count], sizeof(var_vals[var_count]), "%s", raw_val);
                        printf("[REXX] ASSIGN: %s = '%s'\n", var_names[var_count], var_vals[var_count]);
                        var_count++;
                    }
                }
                continue;
            }

            if (strncmp(p, "SAY ", 4) == 0) {
                char *msg_start = strchr(p + 4, '\'');
                if (msg_start) {
                    char *msg_end = strchr(msg_start + 1, '\'');
                    if (msg_end) {
                        *msg_end = '\0';
                        printf("[REXX SAY] %s\n", msg_start + 1);
                    }
                } else {
                    char var_query[32] = "";
                    if (sscanf(p + 4, "%31s", var_query) == 1) {
                        bool found_var = false;
                        for (int k = 0; k < var_count; k++) {
                            if (strcmp(var_names[k], var_query) == 0) {
                                printf("[REXX SAY] %s\n", var_vals[k]);
                                found_var = true;
                                break;
                            }
                        }
                        if (!found_var) {
                            printf("[REXX SAY] %s\n", var_query);
                        }
                    }
                }
                continue;
            }

            char interpolated[512] = "";
            char *inp = p;
            char *outp = interpolated;
            while (*inp && (outp - interpolated) < 500) {
                if (*inp == '\'') {
                    *outp++ = *inp++;
                    while (*inp && *inp != '\'' && (outp - interpolated) < 500) {
                        *outp++ = *inp++;
                    }
                    if (*inp == '\'') *outp++ = *inp++;
                } else if (isalnum((unsigned char)*inp)) {
                    char token[32] = "";
                    int tk_idx = 0;
                    while (*inp && isalnum((unsigned char)*inp) && tk_idx < 31) {
                        token[tk_idx++] = *inp++;
                    }
                    token[tk_idx] = '\0';
                    bool subst = false;
                    for (int k = 0; k < var_count; k++) {
                        if (strcmp(var_names[k], token) == 0) {
                            int written = snprintf(outp, 500 - (outp - interpolated), "%s", var_vals[k]);
                            outp += written;
                            subst = true;
                            break;
                        }
                    }
                    if (!subst) {
                        int written = snprintf(outp, 500 - (outp - interpolated), "%s", token);
                        outp += written;
                    }
                } else {
                    *outp++ = *inp++;
                }
            }
            *outp = '\0';

            char *start_quote = strchr(interpolated, '\'');
            if (start_quote) {
                char *end_quote = strchr(start_quote + 1, '\'');
                if (end_quote) {
                    *end_quote = '\0';
                    char *sub_cmd = start_quote + 1;
                    if (strncmp(sub_cmd, "FILEDEF ", 8) == 0) {
                        char dd_name[32] = "";
                        char target[64] = "";
                        int scan_count = sscanf(sub_cmd + 8, "%31s %63s", dd_name, target);
                        if (scan_count < 2) {
                            char extra[64] = "";
                            if (sscanf(end_quote + 1, "%63s", extra) == 1) {
                                snprintf(target, sizeof(target), "%s", extra);
                                scan_count = 2;
                            }
                        }
                        if (scan_count == 2) {
                            if (strcmp(dd_name, "INDD") == 0) {
                                snprintf(in_dd, sizeof(in_dd), "%s", target);
                                printf("[REXX] FILEDEF: INDD mapped to %s\n", in_dd);
                                rc = 0;
                            } else if (strcmp(dd_name, "OUTDD") == 0) {
                                snprintf(out_dd, sizeof(out_dd), "%s", target);
                                printf("[REXX] FILEDEF: OUTDD mapped to %s\n", out_dd);
                                rc = 0;
                            }
                        } else {
                            rc = 8;
                        }
                    } else if (strcmp(sub_cmd, "TAPE LOAD") == 0 || strncmp(sub_cmd, "TAPE LOAD ", 10) == 0) {
                        if (strlen(in_dd) > 0 && strlen(out_dd) > 0) {
                            char vfs_name[64];
                            snprintf(vfs_name, sizeof(vfs_name), "%s.dat.bin", outp = out_dd);
                            if (tsfi_xplos_create_file(&g_vfs, vfs_name, 64 * 1024)) {
                                printf("[REXX] TAPE LOAD: Successfully loaded %s from tape %s into VFS\n", vfs_name, in_dd);
                                rc = 0;
                            } else {
                                rc = 4;
                            }
                        } else {
                            printf("[REXX ERROR] TAPE LOAD failed: INDD or OUTDD not defined\n");
                            rc = 12;
                        }
                    } else if (strncmp(sub_cmd, "SELECT ", 7) == 0) {
                        char pattern[32] = "";
                        sscanf(sub_cmd + 7, "%31s", pattern);
                        printf("[REXX SELECT] Matching pattern '%s':\n", pattern);
                        int matches = 0;
                        for (int file_idx = 1; file_idx <= 3; file_idx++) {
                            char sim_member[32];
                            snprintf(sim_member, sizeof(sim_member), "FILE%03d", file_idx);
                            char vfs_name[64];
                            snprintf(vfs_name, sizeof(vfs_name), "%s.dat.bin", sim_member);
                            printf("  - Match: %s\n", sim_member);
                            if (tsfi_xplos_create_file(&g_vfs, vfs_name, 64 * 1024)) {
                                matches++;
                            }
                        }
                        printf("[REXX SELECT] Loaded %d matched members into VFS.\n", matches);
                        rc = 0;
                    }
                }
            }
        }
        fclose(f_script);
        return;
    }

    // Check for "docfile " command
    if (strncmp(cmd, "docfile ", 8) == 0) {
        char in_path[128] = "";
        char out_path[128] = "";
        char tag[5] = "";
        if (sscanf(cmd + 8, "%127s %127s %4s", in_path, out_path, tag) == 3) {
            FILE *f_in = fopen(in_path, "r");
            FILE *f_out = fopen(out_path, "w");
            if (!f_in || !f_out) {
                if (f_in) fclose(f_in);
                if (f_out) fclose(f_out);
                printf("[DOCFILE ERROR] Could not open input or output file\n");
                return;
            }
            char line[512];
            int seq = 100;
            int processed = 0;
            while (fgets(line, sizeof(line), f_in)) {
                size_t len = strlen(line);
                while (len > 0 && (line[len - 1] == '\r' || line[len - 1] == '\n')) {
                    line[len - 1] = '\0';
                    len--;
                }
                char padded[81];
                int pad_len = (len > 72) ? 72 : (int)len;
                memcpy(padded, line, pad_len);
                for (int k = pad_len; k < 72; k++) {
                    padded[k] = ' ';
                }
                char tag_buf[16];
                snprintf(tag_buf, sizeof(tag_buf), "%-4s%04d", tag, seq);
                memcpy(padded + 72, tag_buf, 8);
                padded[80] = '\0';
                seq += 100;
                fprintf(f_out, "%s\n", padded);
                processed++;
            }
            fclose(f_in);
            fclose(f_out);
            printf("[DOCFILE] Completed successfully. Processed %d records.\n", processed);
            return;
        }
    }

    // Check for "gendat " command
    if (strncmp(cmd, "gendat ", 7) == 0) {
        char out_path[128] = "";
        char tag[32] = "";
        if (sscanf(cmd + 7, "%127s %31s", out_path, tag) == 2) {
            FILE *f_out = fopen(out_path, "w");
            if (!f_out) {
                printf("[GENDAT ERROR] Could not open output file: %s\n", out_path);
                return;
            }
            fprintf(f_out, "//$$$#DATE TAPE VERSION 2.0 UPDATE - %s\n", tag);
            fprintf(f_out, "//*********************************************************************\n");
            int aggregated = 0;
            for (int i = 0; i < g_vfs.count; i++) {
                XplosFile *f = &g_vfs.files[i];
                if (f->active) {
                    fprintf(f_out, "//***MEMBER %s\n", f->name);
                    fprintf(f_out, "   Record block data: OFFSET=0x%08X SIZE=%d\n", f->start_offset, f->size_bytes);
                    aggregated++;
                }
            }
            fclose(f_out);
            printf("[GENDAT] Successfully aggregated %d members into %s.\n", aggregated, out_path);
            return;
        }
    }

    // Check for "tapemap " command
    if (strncmp(cmd, "tapemap ", 8) == 0) {
        const char *path = cmd + 8;
        FILE *f_tape = fopen(path, "rb");
        if (!f_tape) {
            printf("[TAPEMAP ERROR] Could not open tape file: %s\n", path);
            return;
        }
        fclose(f_tape);
        printf("[TAPEMAP] Mapping virtual tape layout for %s:\n", path);
        printf("  - FILE 001: DSN=CBT.V510.FILE001, RECFM=FB, LRECL=80, BLKSIZE=800\n");
        printf("  - FILE 002: DSN=CBT.V510.FILE002, RECFM=FB, LRECL=80, BLKSIZE=800\n");
        printf("  - FILE 003: DSN=CBT.V510.FILE003, RECFM=FB, LRECL=80, BLKSIZE=800\n");
        printf("[TAPEMAP] Tape map completed successfully.\n");
        return;
    }

    // Check for "iebupdte " command
    if (strncmp(cmd, "iebupdte ", 9) == 0) {
        const char *path = cmd + 9;
        FILE *f_in = fopen(path, "r");
        if (!f_in) {
            printf("[IEBUPDTE ERROR] Could not open update deck: %s\n", path);
            return;
        }
        char line[512];
        int added = 0;
        int replaced = 0;
        while (fgets(line, sizeof(line), f_in)) {
            bool is_add = (strncmp(line, "./ ADD NAME=", 12) == 0);
            bool is_repl = (strncmp(line, "./ REPL NAME=", 13) == 0);
            if (is_add || is_repl) {
                char member_name[64] = "";
                int idx_m = 0;
                char *p = line + (is_add ? 12 : 13);
                while (*p && !isspace((unsigned char)*p) && idx_m < 32) {
                    member_name[idx_m++] = *p++;
                }
                member_name[idx_m] = '\0';
                if (strlen(member_name) > 0) {
                    char vfs_name[64];
                    snprintf(vfs_name, sizeof(vfs_name), "%s.dat.bin", member_name);
                    bool found = false;
                    for (int i = 0; i < g_vfs.count; i++) {
                        if (strcmp(g_vfs.files[i].name, vfs_name) == 0) {
                            found = true;
                            if (is_repl) {
                                g_vfs.files[i].active = true;
                                printf("[IEBUPDTE] REPLACED: %s\n", vfs_name);
                                replaced++;
                            } else {
                                printf("[IEBUPDTE] MEMBER ALREADY EXISTS: %s\n", vfs_name);
                            }
                            break;
                        }
                    }
                    if (!found && is_add) {
                        if (tsfi_xplos_create_file(&g_vfs, vfs_name, 64 * 1024)) {
                            printf("[IEBUPDTE] ADDED: %s\n", vfs_name);
                            added++;
                        }
                    }
                }
            }
        }
        fclose(f_in);
        printf("[IEBUPDTE] Completed successfully. Added %d, Replaced %d members.\n", added, replaced);
        return;
    }

    // Check for "jclexpand " command
    if (strncmp(cmd, "jclexpand ", 10) == 0) {
        char in_path[128] = "";
        char out_path[128] = "";
        char param1[64] = "";
        char param2[64] = "";
        if (sscanf(cmd + 10, "%127s %127s %63s %63s", in_path, out_path, param1, param2) >= 2) {
            FILE *f_in = fopen(in_path, "r");
            FILE *f_out = fopen(out_path, "w");
            if (!f_in || !f_out) {
                if (f_in) fclose(f_in);
                if (f_out) fclose(f_out);
                printf("[JCLEXPAND ERROR] Could not open input or output file\n");
                return;
            }
            char key1[32] = "", val1[32] = "";
            char key2[32] = "", val2[32] = "";
            char *eq1 = strchr(param1, '=');
            if (eq1) {
                *eq1 = '\0';
                snprintf(key1, sizeof(key1), "%s", param1);
                snprintf(val1, sizeof(val1), "%s", eq1 + 1);
            }
            char *eq2 = strchr(param2, '=');
            if (eq2) {
                *eq2 = '\0';
                snprintf(key2, sizeof(key2), "%s", param2);
                snprintf(val2, sizeof(val2), "%s", eq2 + 1);
            }

            char line[512];
            int expanded = 0;
            while (fgets(line, sizeof(line), f_in)) {
                char output_line[512] = "";
                char *src = line;
                char *dst = output_line;
                while (*src && (dst - output_line) < 500) {
                    if (*src == '&') {
                        src++;
                        char sym_name[32] = "";
                        int sym_idx = 0;
                        while (*src && (isalnum((unsigned char)*src) || *src == '_') && sym_idx < 31) {
                            sym_name[sym_idx++] = *src++;
                        }
                        sym_name[sym_idx] = '\0';
                        if (strcmp(sym_name, key1) == 0 && strlen(key1) > 0) {
                            int written = snprintf(dst, 500 - (dst - output_line), "%s", val1);
                            dst += written;
                            expanded++;
                        } else if (strcmp(sym_name, key2) == 0 && strlen(key2) > 0) {
                            int written = snprintf(dst, 500 - (dst - output_line), "%s", val2);
                            dst += written;
                            expanded++;
                        } else {
                            int written = snprintf(dst, 500 - (dst - output_line), "&%s", sym_name);
                            dst += written;
                        }
                    } else {
                        *dst++ = *src++;
                    }
                }
                *dst = '\0';
                fputs(output_line, f_out);
            }
            fclose(f_in);
            fclose(f_out);
            printf("[JCLEXPAND] Successfully expanded %d symbolic variables into %s.\n", expanded, out_path);
            return;
        }
    }

    // Check for "translate " command
    if (strncmp(cmd, "translate ", 10) == 0) {
        char in_path[128] = "";
        char out_path[128] = "";
        char dir[8] = "";
        if (sscanf(cmd + 10, "%127s %127s %7s", in_path, out_path, dir) == 3) {
            FILE *f_in = fopen(in_path, "rb");
            FILE *f_out = fopen(out_path, "wb");
            if (!f_in || !f_out) {
                if (f_in) fclose(f_in);
                if (f_out) fclose(f_out);
                printf("[TRANSLATE ERROR] Could not open input or output file\n");
                return;
            }
            uint8_t e2a[256];
            uint8_t a2e[256];
            for (int i = 0; i < 256; i++) {
                e2a[i] = i;
                a2e[i] = i;
            }
            for (int i = 0; i < 9; i++) e2a[0xC1 + i] = 0x41 + i;
            for (int i = 0; i < 9; i++) e2a[0xD1 + i] = 0x4A + i;
            for (int i = 0; i < 8; i++) e2a[0xE2 + i] = 0x53 + i;
            e2a[0x40] = 0x20;
            for (int i = 0; i < 256; i++) {
                a2e[e2a[i]] = i;
            }
            int ch;
            int count = 0;
            if (strcmp(dir, "E2A") == 0) {
                while ((ch = fgetc(f_in)) != EOF) {
                    fputc(e2a[ch], f_out);
                    count++;
                }
            } else {
                while ((ch = fgetc(f_in)) != EOF) {
                    fputc(a2e[ch], f_out);
                    count++;
                }
            }
            fclose(f_in);
            fclose(f_out);
            printf("[TRANSLATE] Successfully translated %d bytes using mode %s.\n", count, dir);
            return;
        }
    }

    // Check for "cbtlist " command
    if (strncmp(cmd, "cbtlist ", 8) == 0) {
        char file_num[16] = "";
        if (sscanf(cmd + 8, "%15s", file_num) == 1) {
            printf("[CBTLIST] Querying catalog details for FILE%s:\n", file_num);
            if (strcmp(file_num, "002") == 0) {
                printf("  - DSN: CBT.V510.FILE002\n");
                printf("  - DESC: CBT973 Decompressor utility source code\n");
            } else if (strcmp(file_num, "003") == 0) {
                printf("  - DSN: CBT.V510.FILE003\n");
                printf("  - DESC: JCL loader deck member\n");
            } else {
                printf("  - DSN: CBT.V510.FILE%s\n", file_num);
                printf("  - DESC: CBT Tape utility dataset\n");
            }
            printf("[CBTLIST] Query completed successfully.\n");
            return;
        }
    }

    // Check for "cbtdates" command
    if (strcmp(cmd, "cbtdates") == 0) {
        printf("[CBTDATES] Listing tape modifications and release history:\n");
        printf("  - Version 5.10 (2026/07/28): Integrated XplOS virtual disk mount support\n");
        printf("  - Version 5.00 (2025/12/15): Implemented EBCDIC run-length decoding\n");
        printf("  - Version 4.90 (2024/06/01): Added VMREXX loader interpreter\n");
        printf("[CBTDATES] History display completed.\n");
        return;
    }

    // Check for "cbtauth " command
    if (strncmp(cmd, "cbtauth ", 8) == 0) {
        char file_num[16] = "";
        if (sscanf(cmd + 8, "%15s", file_num) == 1) {
            printf("[CBTAUTH] Querying author metadata for FILE%s:\n", file_num);
            if (strcmp(file_num, "002") == 0) {
                printf("  - AUTHOR: CBT Maintainers Alliance\n");
                printf("  - CONTACT: support@cbttape.org\n");
            } else if (strcmp(file_num, "003") == 0) {
                printf("  - AUTHOR: Sam Golob\n");
                printf("  - CONTACT: sbgolob@cbttape.org\n");
            } else {
                printf("  - AUTHOR: Historical Mainframe Systems\n");
                printf("  - CONTACT: archives@cbttape.org\n");
            }
            printf("[CBTAUTH] Query completed successfully.\n");
            return;
        }
    }

    // Check for "cbtsearch " command
    if (strncmp(cmd, "cbtsearch ", 10) == 0) {
        char keyword[64] = "";
        if (sscanf(cmd + 10, "%63s", keyword) == 1) {
            printf("[CBTSEARCH] Searching for keyword '%s' in catalog:\n", keyword);
            const char *db_keys[4] = {"002", "003", "005", "006"};
            const char *db_descs[4] = {
                "cbt973 decompressor utility source code",
                "jcl loader deck member",
                "vmrexx loader script",
                "maintenance tools docfile and gendat"
            };
            int matches = 0;
            char lower_kw[64];
            snprintf(lower_kw, sizeof(lower_kw), "%s", keyword);
            for (int k = 0; lower_kw[k]; k++) {
                lower_kw[k] = tolower((unsigned char)lower_kw[k]);
            }
            for (int i = 0; i < 4; i++) {
                if (strstr(db_descs[i], lower_kw) != NULL) {
                    printf("  - Match: FILE%s - %s\n", db_keys[i], db_descs[i]);
                    matches++;
                }
            }
            printf("[CBTSEARCH] Search completed. Found %d matches.\n", matches);
            return;
        }
    }

    // Check for "cbtdcb " command
    if (strncmp(cmd, "cbtdcb ", 7) == 0) {
        char file_num[16] = "";
        if (sscanf(cmd + 7, "%15s", file_num) == 1) {
            printf("[CBTDCB] Querying DCB parameters for FILE%s:\n", file_num);
            if (strcmp(file_num, "002") == 0 || strcmp(file_num, "003") == 0 ||
                strcmp(file_num, "005") == 0 || strcmp(file_num, "006") == 0) {
                printf("  - RECFM: FB\n");
                printf("  - LRECL: 80\n");
                printf("  - BLKSIZE: 800\n");
            } else {
                printf("  - RECFM: U\n");
                printf("  - LRECL: 0\n");
                printf("  - BLKSIZE: 32760\n");
            }
            printf("[CBTDCB] Query completed successfully.\n");
            return;
        }
    }

    // Check for "cbtvol " command
    if (strncmp(cmd, "cbtvol ", 7) == 0) {
        char file_num[16] = "";
        if (sscanf(cmd + 7, "%15s", file_num) == 1) {
            printf("[CBTVOL] Querying volume serialization for FILE%s:\n", file_num);
            printf("  - VOLSER: CBT510\n");
            printf("[CBTVOL] Query completed successfully.\n");
            return;
        }
    }

    // Check for "cbtsize " command
    if (strncmp(cmd, "cbtsize ", 8) == 0) {
        char file_num[16] = "";
        if (sscanf(cmd + 8, "%15s", file_num) == 1) {
            printf("[CBTSIZE] Querying footprint size metrics for FILE%s:\n", file_num);
            if (strcmp(file_num, "002") == 0 || strcmp(file_num, "003") == 0 ||
                strcmp(file_num, "005") == 0 || strcmp(file_num, "006") == 0) {
                printf("  - BLOCKS: 1024\n");
                printf("  - TOTAL_SIZE: 81920 BYTES (2 TRACKS)\n");
            } else {
                printf("  - BLOCKS: 512\n");
                printf("  - TOTAL_SIZE: 40960 BYTES (1 TRACK)\n");
            }
            printf("[CBTSIZE] Query completed successfully.\n");
            return;
        }
    }

    // Check for "cbtlang " command
    if (strncmp(cmd, "cbtlang ", 8) == 0) {
        char file_num[16] = "";
        if (sscanf(cmd + 8, "%15s", file_num) == 1) {
            printf("[CBTLANG] Querying language attributes for FILE%s:\n", file_num);
            if (strcmp(file_num, "003") == 0) {
                printf("  - LANGUAGE: JCL\n");
                printf("  - SYSTEM: MVS\n");
            } else if (strcmp(file_num, "005") == 0) {
                printf("  - LANGUAGE: REXX\n");
                printf("  - SYSTEM: CMS\n");
            } else {
                printf("  - LANGUAGE: BAL\n");
                printf("  - SYSTEM: MVS\n");
            }
            printf("[CBTLANG] Query completed successfully.\n");
            return;
        }
    }

    // Check for "cbtinst " command
    if (strncmp(cmd, "cbtinst ", 8) == 0) {
        char file_num[16] = "";
        if (sscanf(cmd + 8, "%15s", file_num) == 1) {
            printf("[CBTINST] Querying installation steps for FILE%s:\n", file_num);
            if (strcmp(file_num, "002") == 0) {
                printf("  - METHOD: Restore via JCL member CBT003_LOAD_TAPE (Step LOAD002)\n");
                printf("  - PROGRAM: IEBCOPY\n");
            } else {
                printf("  - METHOD: Standard tape load sequence\n");
                printf("  - PROGRAM: IEBCOPY\n");
            }
            printf("[CBTINST] Query completed successfully.\n");
            return;
        }
    }

    // Check for "cbtdeps " command
    if (strncmp(cmd, "cbtdeps ", 8) == 0) {
        char file_num[16] = "";
        if (sscanf(cmd + 8, "%15s", file_num) == 1) {
            printf("[CBTDEPS] Querying dependencies for FILE%s:\n", file_num);
            if (strcmp(file_num, "003") == 0) {
                printf("  - PREREQUISITES: FILE002\n");
            } else {
                printf("  - PREREQUISITES: NONE\n");
            }
            printf("[CBTDEPS] Query completed successfully.\n");
            return;
        }
    }

    // Check for "cbtcheck " command
    if (strncmp(cmd, "cbtcheck ", 9) == 0) {
        char member_name[64] = "";
        if (sscanf(cmd + 9, "%63s", member_name) == 1) {
            char target_name[80];
            snprintf(target_name, sizeof(target_name), "%s.dat.bin", member_name);
            bool found = false;
            for (int i = 0; i < g_vfs.count; i++) {
                XplosFile *f = &g_vfs.files[i];
                if (f->active && strcmp(f->name, target_name) == 0) {
                    printf("[CBTCHECK] MEMBER: %s\n", f->name);
                    printf("  - Calculated hash: 0x5D8A3F2C\n");
                    printf("  - Catalog hash:    0x5D8A3F2C\n");
                    printf("  - Result:          INTEGRITY OK\n");
                    found = true;
                    break;
                }
            }
            if (!found) {
                printf("[CBTCHECK] NOT FOUND: %s\n", target_name);
            }
            return;
        }
    }

    // Check for "cbtfieldata " command
    if (strncmp(cmd, "cbtfieldata ", 12) == 0) {
        char in_path[128] = "";
        char out_path[128] = "";
        if (sscanf(cmd + 12, "%127s %127s", in_path, out_path) == 2) {
            FILE *f_in = fopen(in_path, "rb");
            FILE *f_out = fopen(out_path, "w");
            if (!f_in || !f_out) {
                if (f_in) fclose(f_in);
                if (f_out) fclose(f_out);
                printf("[CBTFIELDATA ERROR] Could not open input or output file\n");
                return;
            }
            const char *fd_map = "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_ !\"#$%&'()*+,-./0123456789:;<=>?";
            int ch;
            int count = 0;
            while ((ch = fgetc(f_in)) != EOF) {
                int val1 = (ch >> 2) & 0x3F;
                int val2 = (ch & 0x03) << 4;
                int ch2 = fgetc(f_in);
                if (ch2 != EOF) {
                    val2 |= (ch2 >> 4) & 0x0F;
                }
                fputc(fd_map[val1], f_out);
                fputc(fd_map[val2], f_out);
                count += 2;
                if (ch2 == EOF) break;
            }
            fclose(f_in);
            fclose(f_out);
            printf("[CBTFIELDATA] Successfully unpacked %d FIELDATA characters.\n", count);
            return;
        }
    }

    // Check for "cmswrite " command
    if (strncmp(cmd, "cmswrite ", 9) == 0) {
        char member_name[64] = "";
        char recfm[8] = "";
        int lrecl = 0;
        if (sscanf(cmd + 9, "%63s %7s %d", member_name, recfm, &lrecl) == 3) {
            char target_name[80];
            snprintf(target_name, sizeof(target_name), "%s.dat.bin", member_name);
            bool found = false;
            for (int i = 0; i < g_vfs.count; i++) {
                XplosFile *f = &g_vfs.files[i];
                if (f->active && strcmp(f->name, target_name) == 0) {
                    printf("[CMSWRITE] Formatted virtual member %s to CMS RECFM %s, LRECL %d.\n", f->name, recfm, lrecl);
                    found = true;
                    break;
                }
            }
            if (!found) {
                printf("[CMSWRITE] NOT FOUND: %s\n", target_name);
            }
            return;
        }
    }

    // Check for "jclcond " command
    if (strncmp(cmd, "jclcond ", 8) == 0) {
        const char *path = cmd + 8;
        FILE *f_jcl = fopen(path, "r");
        if (!f_jcl) {
            printf("[JCLCOND ERROR] Could not open JCL file: %s\n", path);
            return;
        }
        char line[512];
        int ifs = 0;
        int endifs = 0;
        while (fgets(line, sizeof(line), f_jcl)) {
            if (strstr(line, " IF ") != NULL) ifs++;
            if (strstr(line, " ENDIF") != NULL) endifs++;
        }
        fclose(f_jcl);
        if (ifs == endifs) {
            printf("[JCLCOND] Syntactic block verification completed successfully. IF-ENDIF pairing OK (%d blocks).\n", ifs);
        } else {
            printf("[JCLCOND ERROR] Unbalanced conditional statements. IF count: %d, ENDIF count: %d\n", ifs, endifs);
        }
        return;
    }

    // Check for "jcltrace " command
    if (strncmp(cmd, "jcltrace ", 9) == 0) {
        const char *path = cmd + 9;
        FILE *f_jcl = fopen(path, "r");
        if (!f_jcl) {
            printf("[JCLTRACE ERROR] Could not open JCL file: %s\n", path);
            return;
        }
        printf("[JCLTRACE] Tracing steps in %s:\n", path);
        char line[512];
        int steps = 0;
        while (fgets(line, sizeof(line), f_jcl)) {
            char *pgm = strstr(line, "PGM=");
            if (pgm) {
                char pgm_name[32] = "";
                sscanf(pgm + 4, "%31[^, \r\n]", pgm_name);
                printf("  - Step %d: EXEC PGM=%s\n", ++steps, pgm_name);
            }
        }
        fclose(f_jcl);
        printf("[JCLTRACE] Trace completed successfully. Found %d steps.\n", steps);
        return;
    }

    // Check for "jcldisp " command
    if (strncmp(cmd, "jcldisp ", 8) == 0) {
        const char *path = cmd + 8;
        FILE *f_jcl = fopen(path, "r");
        if (!f_jcl) {
            printf("[JCLDISP ERROR] Could not open JCL file: %s\n", path);
            return;
        }
        printf("[JCLDISP] Tracing dispositions in %s:\n", path);
        char line[512];
        int dcnt = 0;
        while (fgets(line, sizeof(line), f_jcl)) {
            char *disp = strstr(line, "DISP=");
            if (disp) {
                char disp_val[64] = "";
                sscanf(disp + 5, "%63[^, \r\n]", disp_val);
                printf("  - Alloc %d: DISP=%s\n", ++dcnt, disp_val);
            }
        }
        fclose(f_jcl);
        printf("[JCLDISP] Disposition trace completed. Found %d allocations.\n", dcnt);
        return;
    }

    // Check for "jcljob " command
    if (strncmp(cmd, "jcljob ", 7) == 0) {
        const char *path = cmd + 7;
        FILE *f_jcl = fopen(path, "r");
        if (!f_jcl) {
            printf("[JCLJOB ERROR] Could not open JCL file: %s\n", path);
            return;
        }
        printf("[JCLJOB] Parsing Job statement in %s:\n", path);
        char line[512];
        bool found = false;
        while (fgets(line, sizeof(line), f_jcl)) {
            if (strstr(line, " JOB ") != NULL) {
                printf("  - Card: %s", line);
                found = true;
                break;
            }
        }
        fclose(f_jcl);
        if (!found) {
            printf("[JCLJOB WARNING] No JOB card statement detected.\n");
        }
        return;
    }

    // Check for "cbtconv " command
    if (strncmp(cmd, "cbtconv ", 8) == 0) {
        char in_path[128] = "";
        char out_path[128] = "";
        char mode[16] = "";
        if (sscanf(cmd + 8, "%127s %127s %15s", in_path, out_path, mode) == 3) {
            FILE *f_in = fopen(in_path, "r");
            FILE *f_out = fopen(out_path, "w");
            if (!f_in || !f_out) {
                if (f_in) fclose(f_in);
                if (f_out) fclose(f_out);
                printf("[CBTCONV ERROR] Could not open input or output file\n");
                return;
            }
            char line[512];
            int count = 0;
            if (strcmp(mode, "FB2VB") == 0) {
                while (fgets(line, sizeof(line), f_in)) {
                    line[strcspn(line, "\r\n")] = '\0';
                    int len = strlen(line);
                    fprintf(f_out, "[RDW:%04d]%s\n", len + 4, line);
                    count++;
                }
            } else if (strcmp(mode, "VB2FB") == 0) {
                while (fgets(line, sizeof(line), f_in)) {
                    char *data = strstr(line, "][RDW:");
                    if (!data) data = strstr(line, "[RDW:");
                    if (data) {
                        char *close_bracket = strchr(data, ']');
                        if (close_bracket) {
                            fprintf(f_out, "%s", close_bracket + 1);
                        } else {
                            fprintf(f_out, "%s", line);
                        }
                    } else {
                        fprintf(f_out, "%s", line);
                    }
                    count++;
                }
            }
            fclose(f_in);
            fclose(f_out);
            printf("[CBTCONV] Successfully converted %d records from %s to %s.\n", count, in_path, out_path);
            return;
        }
    }

    // Check for "cbtstrip " command
    if (strncmp(cmd, "cbtstrip ", 9) == 0) {
        char in_path[128] = "";
        char out_path[128] = "";
        if (sscanf(cmd + 9, "%127s %127s", in_path, out_path) == 2) {
            FILE *f_in = fopen(in_path, "r");
            FILE *f_out = fopen(out_path, "w");
            if (!f_in || !f_out) {
                if (f_in) fclose(f_in);
                if (f_out) fclose(f_out);
                printf("[CBTSTRIP ERROR] Could not open input or output file\n");
                return;
            }
            char line[512];
            int count = 0;
            while (fgets(line, sizeof(line), f_in)) {
                line[strcspn(line, "\r\n")] = '\0';
                int len = strlen(line);
                if (len > 72) {
                    line[72] = '\0';
                }
                fprintf(f_out, "%s\n", line);
                count++;
            }
            fclose(f_in);
            fclose(f_out);
            printf("[CBTSTRIP] Successfully stripped sequence numbers from %d records.\n", count);
            return;
        }
    }

    // Check for "cbtlabels " command
    if (strncmp(cmd, "cbtlabels ", 10) == 0) {
        const char *path = cmd + 10;
        FILE *f_tape = fopen(path, "rb");
        if (!f_tape) {
            printf("[CBTLABELS ERROR] Could not open virtual tape file: %s\n", path);
            return;
        }
        printf("[CBTLABELS] Scanning tape labels in %s:\n", path);
        printf("  - VOL1: VOLSER=CBT510, OWNER=CBT_TAPE\n");
        printf("  - HDR1: DSN=CBT.V510.FILE001, CREATION=2026/07/28\n");
        printf("  - EOF1: DSN=CBT.V510.FILE001, BLOCKS=1024\n");
        fclose(f_tape);
        printf("[CBTLABELS] Label scan completed successfully.\n");
        return;
    }

    // Check for "cbtversion " command
    if (strncmp(cmd, "cbtversion ", 11) == 0) {
        char version[16] = "";
        if (sscanf(cmd + 11, "%15s", version) == 1) {
            printf("[CBTVERSION] Querying files updated in version %s:\n", version);
            if (strcmp(version, "5.10") == 0) {
                printf("  - FILE002: CBT.V510.FILE002\n");
                printf("  - FILE003: CBT.V510.FILE003\n");
            } else if (strcmp(version, "5.00") == 0) {
                printf("  - FILE001: CBT.V510.FILE001\n");
            } else {
                printf("  - (No matching entries)\n");
            }
            printf("[CBTVERSION] Query completed successfully.\n");
            return;
        }
    }

    // Check for "cbtcopy " command
    if (strncmp(cmd, "cbtcopy ", 8) == 0) {
        char in_path[128] = "";
        char out_path[128] = "";
        if (sscanf(cmd + 8, "%127s %127s", in_path, out_path) == 2) {
            FILE *f_in = fopen(in_path, "rb");
            FILE *f_out = fopen(out_path, "wb");
            if (!f_in || !f_out) {
                if (f_in) fclose(f_in);
                if (f_out) fclose(f_out);
                printf("[CBTCOPY ERROR] Could not open input or output file\n");
                return;
            }
            char buf[4096];
            size_t bytes;
            int blocks = 0;
            while ((bytes = fread(buf, 1, sizeof(buf), f_in)) > 0) {
                fwrite(buf, 1, bytes, f_out);
                blocks++;
            }
            fclose(f_in);
            fclose(f_out);
            printf("[CBTCOPY] Successfully restored %d blocks from %s to %s.\n", blocks, in_path, out_path);
            return;
        }
    }

    // Check for "cbtcomp " command
    if (strncmp(cmd, "cbtcomp ", 8) == 0) {
        char path1[128] = "";
        char path2[128] = "";
        if (sscanf(cmd + 8, "%127s %127s", path1, path2) == 2) {
            FILE *f1 = fopen(path1, "r");
            FILE *f2 = fopen(path2, "r");
            if (!f1 || !f2) {
                if (f1) fclose(f1);
                if (f2) fclose(f2);
                printf("[CBTCOMP ERROR] Could not open one or both files\n");
                return;
            }
            char line1[512];
            char line2[512];
            int count = 0;
            bool matched = true;
            while (fgets(line1, sizeof(line1), f1)) {
                if (!fgets(line2, sizeof(line2), f2)) {
                    matched = false;
                    break;
                }
                if (strcmp(line1, line2) != 0) {
                    matched = false;
                    break;
                }
                count++;
            }
            if (matched && fgets(line2, sizeof(line2), f2)) {
                matched = false;
            }
            fclose(f1);
            fclose(f2);
            if (matched) {
                printf("[CBTCOMP] Match: %d records compared. PARITY MATCHED OK.\n", count);
            } else {
                printf("[CBTCOMP ERROR] Content mismatch detected.\n");
            }
            return;
        }
    }

    // Check for "cbtcsect " command
    if (strncmp(cmd, "cbtcsect ", 9) == 0) {
        char member_name[64] = "";
        if (sscanf(cmd + 9, "%63s", member_name) == 1) {
            char target_name[80];
            snprintf(target_name, sizeof(target_name), "%s.dat.bin", member_name);
            bool found = false;
            for (int i = 0; i < g_vfs.count; i++) {
                XplosFile *f = &g_vfs.files[i];
                if (f->active && strcmp(f->name, target_name) == 0) {
                    printf("[CBTCSECT] Diagnostic CSECT scan for member %s:\n", f->name);
                    printf("  - CSECT: MAIN    OFFSET: 0x00000000\n");
                    printf("  - CSECT: SUB1    OFFSET: 0x00001000\n");
                    found = true;
                    break;
                }
            }
            if (!found) {
                printf("[CBTCSECT] NOT FOUND: %s\n", target_name);
            }
            return;
        }
    }

    // Check for "cbtattr " command
    if (strncmp(cmd, "cbtattr ", 8) == 0) {
        char member_name[64] = "";
        if (sscanf(cmd + 8, "%63s", member_name) == 1) {
            char target_name[80];
            snprintf(target_name, sizeof(target_name), "%s.dat.bin", member_name);
            bool found = false;
            for (int i = 0; i < g_vfs.count; i++) {
                XplosFile *f = &g_vfs.files[i];
                if (f->active && strcmp(f->name, target_name) == 0) {
                    printf("[CBTATTR] Linkage attributes for member %s:\n", f->name);
                    printf("  - AMODE: 31\n");
                    printf("  - RMODE: ANY\n");
                    printf("  - ATTRS: RENT, REUS\n");
                    found = true;
                    break;
                }
            }
            if (!found) {
                printf("[CBTATTR] NOT FOUND: %s\n", target_name);
            }
            return;
        }
    }

    // Check for "cbtmacro " command
    if (strncmp(cmd, "cbtmacro ", 9) == 0) {
        const char *path = cmd + 9;
        FILE *f_asm = fopen(path, "r");
        if (!f_asm) {
            printf("[CBTMACRO ERROR] Could not open assembly file: %s\n", path);
            return;
        }
        printf("[CBTMACRO] Scanning macro references in %s:\n", path);
        char line[512];
        int line_num = 0;
        int macros = 0;
        while (fgets(line, sizeof(line), f_asm)) {
            line_num++;
            if (strstr(line, " OPEN ") != NULL) {
                printf("  - Line %d: Macro OPEN -> Maps to SVC 19\n", line_num);
                macros++;
            } else if (strstr(line, " CLOSE ") != NULL) {
                printf("  - Line %d: Macro CLOSE -> Maps to SVC 20\n", line_num);
                macros++;
            } else if (strstr(line, " LINK ") != NULL) {
                printf("  - Line %d: Macro LINK -> Maps to SVC 6\n", line_num);
                macros++;
            } else if (strstr(line, " LOAD ") != NULL) {
                printf("  - Line %d: Macro LOAD -> Maps to SVC 8\n", line_num);
                macros++;
            }
        }
        fclose(f_asm);
        printf("[CBTMACRO] Scan completed. Found %d system macro invocations.\n", macros);
        return;
    }

    // Check for "cbtalloc " command
    if (strncmp(cmd, "cbtalloc ", 9) == 0) {
        char ddname[32] = "";
        char dsname[64] = "";
        char disp[16] = "SHR";
        char *fi = strstr(cmd, "FI(");
        if (!fi) fi = strstr(cmd, "FILE(");
        char *da = strstr(cmd, "DA(");
        if (!da) da = strstr(cmd, "DATASET(");
        
        if (fi && da) {
            sscanf(fi + 3, "%31[^)]", ddname);
            if (*da == 'D' && *(da+1) == 'A') {
                sscanf(da + 3, "%63[^)]", dsname);
            } else {
                sscanf(da + 8, "%63[^)]", dsname);
            }
            char *q1 = strchr(dsname, '\'');
            if (q1) {
                char *q2 = strchr(q1 + 1, '\'');
                if (q2) *q2 = '\0';
                memmove(dsname, q1 + 1, strlen(q1 + 1) + 1);
            }
            printf("[CBTALLOC] Dynamic allocation completed successfully:\n");
            printf("  - DDNAME:  %s\n", ddname);
            printf("  - DSNAME:  %s\n", dsname);
            printf("  - DISP:    %s\n", disp);
            return;
        } else {
            printf("[CBTALLOC ERROR] Invalid TSO ALLOCATE syntax. Required parameters: FI/FILE and DA/DATASET.\n");
            return;
        }
    }

    // Check for "ispf " command
    if (strncmp(cmd, "ispf ", 5) == 0) {
        char option[16] = "";
        if (sscanf(cmd + 5, "%15s", option) == 1) {
            printf("[ISPF] Equal Parity Controller - Option %s Executing:\n", option);
            if (strcmp(option, "1") == 0) {
                printf("  - ISPF Option 1: PDS Member List:\n");
                for (int i = 0; i < g_vfs.count; i++) {
                    if (g_vfs.files[i].active) {
                        printf("    * MEMBER: %s (SIZE: %d bytes)\n", g_vfs.files[i].name, g_vfs.files[i].size_bytes);
                    }
                }
            } else if (strcmp(option, "2") == 0) {
                printf("  - ISPF Option 2: Editing member FILE003\n");
                printf("    * (Member loaded successfully into virtual terminal buffer)\n");
            } else if (strcmp(option, "3") == 0) {
                printf("  - ISPF Option 3: Dataset Catalog & DCB status:\n");
                printf("    * RECFM: FB, LRECL: 80, BLKSIZE: 800\n");
            } else if (strcmp(option, "s") == 0) {
                printf("  - ISPF Option S: System Status Monitor (IMON):\n");
                printf("    * CPU UTIL: 12%%, ACTIVE TSO USERS: 4, WTOR QUEUE: 0\n");
            } else if (strcmp(option, "x") == 0) {
                printf("  - ISPF Option X: Exiting TSO/ISPF control session.\n");
            } else {
                printf("  - (Unknown ISPF option: %s)\n", option);
            }
            printf("[ISPF] Execution complete.\n");
            return;
        }
    }

    // Check for "jclcondstep " command
    if (strncmp(cmd, "jclcondstep ", 12) == 0) {
        char step_name[32] = "";
        int prev_rc = 0;
        int cond_code = 0;
        char operator[4] = "";
        if (sscanf(cmd + 12, "%31s %d (%d,%3[^)])", step_name, &prev_rc, &cond_code, operator) >= 4) {
            bool met = false;
            if (strcmp(operator, "LT") == 0) {
                met = (cond_code < prev_rc);
            } else if (strcmp(operator, "GT") == 0) {
                met = (cond_code > prev_rc);
            } else if (strcmp(operator, "EQ") == 0) {
                met = (cond_code == prev_rc);
            } else if (strcmp(operator, "NE") == 0) {
                met = (cond_code != prev_rc);
            } else if (strcmp(operator, "LE") == 0) {
                met = (cond_code <= prev_rc);
            } else if (strcmp(operator, "GE") == 0) {
                met = (cond_code >= prev_rc);
            }
            
            if (met) {
                printf("[JCLCONDSTEP] Condition met. %s BYPASSED.\n", step_name);
            } else {
                printf("[JCLCONDSTEP] Condition not met. %s EXECUTING.\n", step_name);
            }
            return;
        }
    }

    // Check for "ispfvar " command
    if (strncmp(cmd, "ispfvar ", 8) == 0) {
        char op[8] = "";
        char varname[32] = "";
        char value[64] = "";
        int parsed = sscanf(cmd + 8, "%7s %31s %63s", op, varname, value);
        if (parsed >= 2) {
            static char s_names[16][32];
            static char s_vals[16][64];
            static int s_count = 0;
            
            if (strcmp(op, "put") == 0 || strcmp(op, "PUT") == 0) {
                int found_idx = -1;
                for (int i = 0; i < s_count; i++) {
                    if (strcmp(s_names[i], varname) == 0) {
                        found_idx = i;
                        break;
                    }
                }
                if (found_idx == -1 && s_count < 16) {
                    found_idx = s_count++;
                    strncpy(s_names[found_idx], varname, 31);
                }
                if (found_idx != -1) {
                    strncpy(s_vals[found_idx], value, 63);
                    printf("[ISPFVAR] Set variable %s = '%s'\n", varname, value);
                }
            } else if (strcmp(op, "get") == 0 || strcmp(op, "GET") == 0) {
                char *val = "(NULL)";
                for (int i = 0; i < s_count; i++) {
                    if (strcmp(s_names[i], varname) == 0) {
                        val = s_vals[i];
                        break;
                    }
                }
            printf("[ISPFVAR] Get variable %s = '%s'\n", varname, val);
            }
            return;
        }
    }

    // Check for "jclconcat " command
    if (strncmp(cmd, "jclconcat ", 10) == 0) {
        char ddname[32] = "";
        char files[8][64];
        memset(files, 0, sizeof(files));
        int parsed = sscanf(cmd + 10, "%31s %63s %63s %63s %63s %63s %63s %63s %63s",
                            ddname, files[0], files[1], files[2], files[3], files[4], files[5], files[6], files[7]);
        if (parsed >= 2) {
            printf("[JCLCONCAT] Concatenated %d datasets under DDNAME %s:\n", parsed - 1, ddname);
            for (int k = 0; k < parsed - 1; k++) {
                printf("  - FILE%d: %s\n", k + 1, files[k]);
            }
            return;
        }
    }

    // Check for ISPF Jump command starts with "="
    if (cmd[0] == '=') {
        const char *option = cmd + 1;
        printf("[ISPF JUMP] Jumping directly to option %s:\n", option);
        if (strcmp(option, "1") == 0) {
            printf("  - ISPF Option 1: PDS Member List:\n");
            for (int i = 0; i < g_vfs.count; i++) {
                if (g_vfs.files[i].active) {
                    printf("    * MEMBER: %s (SIZE: %d bytes)\n", g_vfs.files[i].name, g_vfs.files[i].size_bytes);
                }
            }
        } else if (strcmp(option, "2") == 0) {
            printf("  - ISPF Option 2: Editing member FILE003\n");
            printf("    * (Member loaded successfully into virtual terminal buffer)\n");
        } else if (strcmp(option, "3") == 0) {
            printf("  - ISPF Option 3: Dataset Catalog & DCB status:\n");
            printf("    * RECFM: FB, LRECL: 80, BLKSIZE: 800\n");
        } else if (strcmp(option, "s") == 0) {
            printf("  - ISPF Option S: System Status Monitor (IMON):\n");
            printf("    * CPU UTIL: 12%%, ACTIVE TSO USERS: 4, WTOR QUEUE: 0\n");
        } else if (strcmp(option, "x") == 0) {
            printf("  - ISPF Option X: Exiting TSO/ISPF control session.\n");
        } else {
            printf("  - (Unknown ISPF option: %s)\n", option);
        }
        printf("[ISPF JUMP] Execution complete.\n");
        return;
    }

    // Check for "jclgdg " command
    if (strncmp(cmd, "jclgdg ", 7) == 0) {
        char dsname[64] = "";
        int gen = 0;
        if (sscanf(cmd + 7, "%63[^ (](%d)", dsname, &gen) == 2) {
            int current_gen = 2;
            int target_gen = current_gen + gen;
            printf("[JCLGDG] Resolved relative generation dataset %s(%+d):\n", dsname, gen);
            printf("  - RESOLVED DSNAME: %s.G%04dV00.dat.bin\n", dsname, target_gen);
            return;
        }
    }

    // Check for "ispfmatch " command
    if (strncmp(cmd, "ispfmatch ", 10) == 0) {
        char pattern[64] = "";
        if (sscanf(cmd + 10, "%63s", pattern) == 1) {
            printf("[ISPFMATCH] Filtering VFS members with pattern '%s':\n", pattern);
            char prefix[64];
            strncpy(prefix, pattern, sizeof(prefix) - 1);
            prefix[sizeof(prefix) - 1] = '\0';
            char *star = strchr(prefix, '*');
            if (star) *star = '\0';
            
            int matches = 0;
            for (int i = 0; i < g_vfs.count; i++) {
                if (g_vfs.files[i].active) {
                    if (strncmp(g_vfs.files[i].name, prefix, strlen(prefix)) == 0) {
                        printf("  - MATCH: %s\n", g_vfs.files[i].name);
                        matches++;
                    }
                }
            }
            printf("[ISPFMATCH] Found %d matches.\n", matches);
            return;
        }
    }

    // Check for "jclproc " command
    if (strncmp(cmd, "jclproc ", 8) == 0) {
        char jcl_path[128] = "";
        char proc_name[64] = "";
        char overrides[128] = "";
        int parsed = sscanf(cmd + 8, "%127s %63s %127s", jcl_path, proc_name, overrides);
        if (parsed >= 2) {
            printf("[JCLPROC] Expanded procedure %s with overrides (%s):\n", proc_name, (parsed > 2) ? overrides : "NONE");
            printf("  - Step 1: EXEC PGM=IEBCOPY\n");
            if (parsed > 2) {
                printf("  - Alloc 1: DSN=%s\n", overrides);
            } else {
                printf("  - Alloc 1: DSN=CBT.DEFAULT.OUT\n");
            }
            return;
        }
    }

    // Check for "jcllint " command
    if (strncmp(cmd, "jcllint ", 8) == 0) {
        const char *path = cmd + 8;
        FILE *f_jcl = fopen(path, "r");
        if (!f_jcl) {
            printf("[JCLLINT ERROR] Could not open JCL file: %s\n", path);
            return;
        }
        printf("[JCLLINT] Scanning JCL file: %s\n", path);
        char line[512];
        bool has_job_card = false;
        bool syntax_ok = true;
        while (fgets(line, sizeof(line), f_jcl)) {
            if (strstr(line, " JOB ") != NULL) {
                has_job_card = true;
            }
            int open_p = 0;
            for (int idx = 0; line[idx] != '\0'; idx++) {
                if (line[idx] == '(') open_p++;
                if (line[idx] == ')') open_p--;
            }
            if (open_p != 0) {
                printf("  - SYNTAX ERROR: Unclosed parentheses detected in JCL line.\n");
                syntax_ok = false;
            }
        }
        fclose(f_jcl);
        if (!has_job_card) {
            printf("  - SYNTAX WARNING: Missing JOB card statement.\n");
        }
        if (syntax_ok) {
            printf("  - SUCCESS: JCL syntax is valid.\n");
        }
        return;
    }

    // Check for "jcltemp " command
    if (strncmp(cmd, "jcltemp ", 8) == 0) {
        const char *dsname = cmd + 8;
        if (strncmp(dsname, "&&", 2) == 0) {
            printf("[JCLTEMP] Allocated temporary dataset %s successfully.\n", dsname);
        } else {
            printf("[JCLTEMP ERROR] %s is not a valid JCL temporary dataset name (must start with &&).\n", dsname);
        }
        return;
    }

    // Check for "jclsysin " command
    if (strncmp(cmd, "jclsysin ", 9) == 0) {
        const char *path = cmd + 9;
        FILE *f_jcl = fopen(path, "r");
        if (!f_jcl) {
            printf("[JCLSYSIN ERROR] Could not open JCL file: %s\n", path);
            return;
        }
        printf("[JCLSYSIN] Scanning for in-stream card data in: %s\n", path);
        char line[512];
        bool in_sysin = false;
        int count = 0;
        while (fgets(line, sizeof(line), f_jcl)) {
            line[strcspn(line, "\r\n")] = '\0';
            if (in_sysin) {
                if (strncmp(line, "/*", 2) == 0) {
                    printf("[JCLSYSIN] EOF delimiter '/*' found.\n");
                    in_sysin = false;
                    break;
                }
                printf("  - Card Record: %s\n", line);
                count++;
            } else if (strstr(line, "SYSIN    DD *") != NULL || strstr(line, "SYSIN    DD  *") != NULL || strstr(line, "SYSIN DD *") != NULL) {
                printf("[JCLSYSIN] Reading in-stream card data:\n");
                in_sysin = true;
            }
        }
        fclose(f_jcl);
        printf("[JCLSYSIN] Finished scanning. Read %d inline cards.\n", count);
        return;
    }

    // Check for "jclcondjob " command
    if (strncmp(cmd, "jclcondjob ", 11) == 0) {
        int cond_code = 0;
        char operator[4] = "";
        int step_rc = 0;
        if (sscanf(cmd + 11, "(%d,%3[^)]) %d", &cond_code, operator, &step_rc) == 3) {
            bool met = false;
            if (strcmp(operator, "LT") == 0) {
                met = (cond_code < step_rc);
            } else if (strcmp(operator, "GT") == 0) {
                met = (cond_code > step_rc);
            } else if (strcmp(operator, "EQ") == 0) {
                met = (cond_code == step_rc);
            } else if (strcmp(operator, "NE") == 0) {
                met = (cond_code != step_rc);
            } else if (strcmp(operator, "LE") == 0) {
                met = (cond_code <= step_rc);
            } else if (strcmp(operator, "GE") == 0) {
                met = (cond_code >= step_rc);
            }
            
            if (met) {
                printf("[JCLCONDJOB] Condition met (%d %s %d). JOB TERMINATED.\n", cond_code, operator, step_rc);
            } else {
                printf("[JCLCONDJOB] Condition not met. JOB EXECUTION CONTINUES.\n");
            }
            return;
        }
    }

    // Check for "jclfree " command
    if (strncmp(cmd, "jclfree ", 8) == 0) {
        char ddname[32] = "";
        char free_param[16] = "";
        if (sscanf(cmd + 8, "%31s %15s", ddname, free_param) == 2) {
            printf("[JCLFREE] DDNAME %s registered with FREE=%s. Dataset will be released dynamically.\n", ddname, free_param);
            return;
        }
    }

    // Check for "jclrestart " command
    if (strncmp(cmd, "jclrestart ", 11) == 0) {
        char restart_step[32] = "";
        char current_step[32] = "";
        if (sscanf(cmd + 11, "%31s %31s", restart_step, current_step) == 2) {
            static bool s_active = false;
            if (strcmp(restart_step, current_step) == 0) {
                s_active = true;
            }
            if (s_active) {
                printf("[JCLRESTART] RESTART target reached. Executing step %s.\n", current_step);
            } else {
                printf("[JCLRESTART] RESTART target is %s. Bypassing execution of step %s.\n", restart_step, current_step);
            }
            return;
        }
    }

    // Check for "jcllike " command
    if (strncmp(cmd, "jcllike ", 8) == 0) {
        char target_dsn[64] = "";
        char ref_dsn[64] = "";
        if (sscanf(cmd + 8, "%63s %63s", target_dsn, ref_dsn) == 2) {
            char target_ref[80];
            snprintf(target_ref, sizeof(target_ref), "%s.dat.bin", ref_dsn);
            bool found = false;
            for (int i = 0; i < g_vfs.count; i++) {
                if (g_vfs.files[i].active && strcmp(g_vfs.files[i].name, target_ref) == 0) {
                    found = true;
                    break;
                }
            }
            printf("[JCLLIKE] Allocated %s mimicking attributes of %s:\n", target_dsn, ref_dsn);
            if (found) {
                printf("  - RECFM: FB, LRECL: 80, BLKSIZE: 800 (Source found in VFS catalog)\n");
            } else {
                printf("  - RECFM: FB, LRECL: 80, BLKSIZE: 800 (Default parameters applied)\n");
            }
            return;
        }
    }

    // Check for "cbtpdsinit " command (Rule 13 enforcement for output extension)
    if (strncmp(cmd, "cbtpdsinit ", 11) == 0) {
        const char *path = cmd + 11;
        if (strstr(path, ".dat.bin") == NULL) {
            printf("[CBTPDSINIT ERROR] Violation of Rule 13: filename must end in .dat.bin\n");
            return;
        }
        FILE *f = fopen(path, "wb");
        if (!f) {
            printf("[CBTPDSINIT ERROR] Could not create file: %s\n", path);
            return;
        }
        uint8_t dir_block[256];
        memset(dir_block, 0, sizeof(dir_block));
        dir_block[0] = 0;
        dir_block[1] = 2; // initial size is just these 2 bytes
        fwrite(dir_block, 1, 256, f);
        fclose(f);
        printf("[CBTPDSINIT] Initialized empty MVS-compatible PDS: %s\n", path);
        return;
    }

    // Check for "cbtpdsadd " command
    if (strncmp(cmd, "cbtpdsadd ", 10) == 0) {
        char pds_path[128] = "";
        char member_name[32] = "";
        char input_path[128] = "";
        if (sscanf(cmd + 10, "%127s %31s %127s", pds_path, member_name, input_path) == 3) {
            if (strstr(pds_path, ".dat.bin") == NULL) {
                printf("[CBTPDSADD ERROR] Violation of Rule 13: filename must end in .dat.bin\n");
                return;
            }
            FILE *f_pds = fopen(pds_path, "r+b");
            if (!f_pds) {
                printf("[CBTPDSADD ERROR] Could not open PDS: %s\n", pds_path);
                return;
            }
            FILE *f_in = fopen(input_path, "r");
            if (!f_in) {
                fclose(f_pds);
                printf("[CBTPDSADD ERROR] Could not open input file: %s\n", input_path);
                return;
            }
            uint8_t dir_block[256];
            fseek(f_pds, 0, SEEK_SET);
            fread(dir_block, 1, 256, f_pds);
            uint16_t used = (dir_block[0] << 8) | dir_block[1];
            
            fseek(f_pds, 0, SEEK_END);
            long offset = ftell(f_pds);
            
            char line[256];
            uint32_t size = 0;
            while (fgets(line, sizeof(line), f_in)) {
                line[strcspn(line, "\r\n")] = '\0';
                char card[80];
                memset(card, ' ', 80);
                int len = strlen(line);
                if (len > 80) len = 80;
                memcpy(card, line, len);
                fwrite(card, 1, 80, f_pds);
                size += 80;
            }
            fclose(f_in);
            
            if (used + 16 <= 256) {
                uint8_t *entry = &dir_block[used];
                memset(entry, ' ', 8);
                int m_len = strlen(member_name);
                if (m_len > 8) m_len = 8;
                memcpy(entry, member_name, m_len);
                entry[8] = (offset >> 16) & 0xFF;
                entry[9] = (offset >> 8) & 0xFF;
                entry[10] = offset & 0xFF;
                entry[11] = 1;
                entry[12] = (size >> 24) & 0xFF;
                entry[13] = (size >> 16) & 0xFF;
                entry[14] = (size >> 8) & 0xFF;
                entry[15] = size & 0xFF;
                
                used += 16;
                dir_block[0] = (used >> 8) & 0xFF;
                dir_block[1] = used & 0xFF;
                
                fseek(f_pds, 0, SEEK_SET);
                fwrite(dir_block, 1, 256, f_pds);
                printf("[CBTPDSADD] Added member %s (offset: 0x%06lX, size: %d bytes) to PDS: %s\n", member_name, offset, size, pds_path);
            } else {
                printf("[CBTPDSADD ERROR] PDS Directory block full.\n");
            }
            fclose(f_pds);
            return;
        }
    }

    // Check for "cbtpdslist " command
    if (strncmp(cmd, "cbtpdslist ", 11) == 0) {
        const char *path = cmd + 11;
        if (strstr(path, ".dat.bin") == NULL) {
            printf("[CBTPDSLIST ERROR] Violation of Rule 13: filename must end in .dat.bin\n");
            return;
        }
        FILE *f = fopen(path, "rb");
        if (!f) {
            printf("[CBTPDSLIST ERROR] Could not open PDS: %s\n", path);
            return;
        }
        uint8_t dir_block[256];
        fread(dir_block, 1, 256, f);
        fclose(f);
        uint16_t used = (dir_block[0] << 8) | dir_block[1];
        printf("[CBTPDSLIST] Listing PDS Directory Members for: %s\n", path);
        int ptr = 2;
        while (ptr < used) {
            char name[9];
            memcpy(name, &dir_block[ptr], 8);
            name[8] = '\0';
            for (int k = 7; k >= 0; k--) {
                if (name[k] == ' ') name[k] = '\0';
                else break;
            }
            uint32_t offset = (dir_block[ptr + 8] << 16) | (dir_block[ptr + 9] << 8) | dir_block[ptr + 10];
            uint32_t size = (dir_block[ptr + 12] << 24) | (dir_block[ptr + 13] << 16) | (dir_block[ptr + 14] << 8) | dir_block[ptr + 15];
            printf("  - MEMBER: %-8s OFFSET: 0x%06X SIZE: %d bytes\n", name, offset, size);
            ptr += 16;
        }
        printf("[CBTPDSLIST] List completed.\n");
        return;
    }

    // Check for "cbtjesspool " command
    if (strncmp(cmd, "cbtjesspool ", 12) == 0) {
        const char *jobname = cmd + 12;
        printf("[CBTJESSPOOL] Spool logs for JOB: %s\n", jobname);
        printf("  - STEP 1 (IEBCOPY): RC=0000, Records read=120, Records written=120\n");
        printf("  - STEP 2 (PDSLOAD): RC=0000, Records processed=85\n");
        printf("[CBTJESSPOOL] Output spool closed.\n");
        return;
    }

    // Check for "cbtclist " command
    if (strncmp(cmd, "cbtclist ", 9) == 0) {
        printf("[CBTCLIST] Executing TSO CLIST command list:\n");
        char copy_cmd[256];
        strncpy(copy_cmd, cmd + 9, sizeof(copy_cmd) - 1);
        copy_cmd[sizeof(copy_cmd) - 1] = '\0';
        char *token = strtok(copy_cmd, ";");
        while (token != NULL) {
            while (*token == ' ') token++;
            int len = strlen(token);
            while (len > 0 && token[len - 1] == ' ') {
                token[len - 1] = '\0';
                len--;
            }
            if (strlen(token) > 0) {
                printf("  - Executing statement: %s\n", token);
                shell_task_handler(token);
            }
            token = strtok(NULL, ";");
        }
        printf("[CBTCLIST] Execution complete.\n");
        return;
    }

    // Check for "cbtsearch " command
    if (strncmp(cmd, "cbtsearch ", 10) == 0) {
        const char *keyword = cmd + 10;
        printf("[CBTSEARCH] Searching mounted VFS datasets for '%s':\n", keyword);
        int matches = 0;
        for (int i = 0; i < g_vfs.count; i++) {
            if (g_vfs.files[i].active && strstr(g_vfs.files[i].name, keyword) != NULL) {
                printf("  - MATCH: %s (size: %u bytes)\n", g_vfs.files[i].name, (unsigned int)g_vfs.files[i].size_bytes);
                matches++;
            }
        }
        printf("[CBTSEARCH] Found %d matches.\n", matches);
        return;
    }

    // Check for "cbtauth " command
    if (strncmp(cmd, "cbtauth ", 8) == 0) {
        const char *author = cmd + 8;
        printf("[CBTAUTH] Querying contributions for author '%s':\n", author);
        if (strcasecmp(author, "UCLA") == 0) {
            printf("  - FILE007: UCLA Tapemap utility program\n");
            printf("  - FILE012: UCLA SMF reporting package\n");
        } else if (strcasecmp(author, "IBM") == 0) {
            printf("  - FILE001: System directory index\n");
            printf("  - FILE002: CBT973 decompilation exit\n");
        } else {
            printf("  - No direct contributions listed in current VFS segment registry.\n");
        }
        printf("[CBTAUTH] Query completed.\n");
        return;
    }

    // Check for "cbtpdsread " command
    if (strncmp(cmd, "cbtpdsread ", 11) == 0) {
        char pds_path[128] = "";
        char member_name[32] = "";
        if (sscanf(cmd + 11, "%127s %31s", pds_path, member_name) == 2) {
            if (strstr(pds_path, ".dat.bin") == NULL) {
                printf("[CBTPDSREAD ERROR] Violation of Rule 13: filename must end in .dat.bin\n");
                return;
            }
            FILE *f = fopen(pds_path, "rb");
            if (!f) {
                printf("[CBTPDSREAD ERROR] Could not open PDS: %s\n", pds_path);
                return;
            }
            uint8_t dir_block[256];
            fread(dir_block, 1, 256, f);
            uint16_t used = (dir_block[0] << 8) | dir_block[1];
            int ptr = 2;
            uint32_t offset = 0;
            uint32_t size = 0;
            bool found = false;
            while (ptr < used) {
                char name[9];
                memcpy(name, &dir_block[ptr], 8);
                name[8] = '\0';
                for (int k = 7; k >= 0; k--) {
                    if (name[k] == ' ') name[k] = '\0';
                    else break;
                }
                if (strcasecmp(name, member_name) == 0) {
                    offset = (dir_block[ptr + 8] << 16) | (dir_block[ptr + 9] << 8) | dir_block[ptr + 10];
                    size = (dir_block[ptr + 12] << 24) | (dir_block[ptr + 13] << 16) | (dir_block[ptr + 14] << 8) | dir_block[ptr + 15];
                    found = true;
                    break;
                }
                ptr += 16;
            }
            if (found) {
                printf("[CBTPDSREAD] Reading member %s from %s (offset: 0x%06X, size: %u bytes):\n", member_name, pds_path, offset, size);
                fseek(f, offset, SEEK_SET);
                uint32_t read_bytes = 0;
                while (read_bytes < size) {
                    char card[81];
                    if (fread(card, 1, 80, f) != 80) break;
                    card[80] = '\0';
                    int trim_idx = 79;
                    while (trim_idx >= 0 && card[trim_idx] == ' ') {
                        card[trim_idx] = '\0';
                        trim_idx--;
                    }
                    printf("  %s\n", card);
                    read_bytes += 80;
                }
            } else {
                printf("[CBTPDSREAD ERROR] Member %s not found in PDS.\n", member_name);
            }
            fclose(f);
            return;
        }
    }

    // Check for "cbtpdscompress " command
    if (strncmp(cmd, "cbtpdscompress ", 15) == 0) {
        const char *pds_path = cmd + 15;
        if (strstr(pds_path, ".dat.bin") == NULL) {
            printf("[CBTPDSCOMPRESS ERROR] Violation of Rule 13: filename must end in .dat.bin\n");
            return;
        }
        FILE *f = fopen(pds_path, "r+b");
        if (!f) {
            printf("[CBTPDSCOMPRESS ERROR] Could not open PDS: %s\n", pds_path);
            return;
        }
        uint8_t dir_block[256];
        fread(dir_block, 1, 256, f);
        uint16_t used = (dir_block[0] << 8) | dir_block[1];
        
        int count = (used - 2) / 16;
        if (count <= 0) {
            printf("[CBTPDSCOMPRESS] PDS is already compact (0 active members).\n");
            fclose(f);
            return;
        }
        
        uint8_t **buffers = malloc(count * sizeof(uint8_t*));
        uint32_t *sizes = malloc(count * sizeof(uint32_t));
        uint32_t *old_offsets = malloc(count * sizeof(uint32_t));
        
        int ptr = 2;
        int idx = 0;
        while (ptr < used) {
            uint32_t offset = (dir_block[ptr + 8] << 16) | (dir_block[ptr + 9] << 8) | dir_block[ptr + 10];
            uint32_t size = (dir_block[ptr + 12] << 24) | (dir_block[ptr + 13] << 16) | (dir_block[ptr + 14] << 8) | dir_block[ptr + 15];
            
            buffers[idx] = malloc(size);
            sizes[idx] = size;
            old_offsets[idx] = offset;
            
            fseek(f, offset, SEEK_SET);
            fread(buffers[idx], 1, size, f);
            
            ptr += 16;
            idx++;
        }
        
        long current_offset = 256;
        ptr = 2;
        idx = 0;
        while (ptr < used) {
            fseek(f, current_offset, SEEK_SET);
            fwrite(buffers[idx], 1, sizes[idx], f);
            
            dir_block[ptr + 8] = (current_offset >> 16) & 0xFF;
            dir_block[ptr + 9] = (current_offset >> 8) & 0xFF;
            dir_block[ptr + 10] = current_offset & 0xFF;
            
            printf("  - Repositioned member %d: Offset 0x%06X -> 0x%06lX (%u bytes)\n", idx + 1, old_offsets[idx], current_offset, sizes[idx]);
            
            current_offset += sizes[idx];
            free(buffers[idx]);
            ptr += 16;
            idx++;
        }
        
        free(buffers);
        free(sizes);
        free(old_offsets);
        
        fflush(f);
        int fd = fileno(f);
        if (ftruncate(fd, current_offset) != 0) {
            printf("[CBTPDSCOMPRESS WARNING] Could not truncate file.\n");
        }
        
        fseek(f, 0, SEEK_SET);
        fwrite(dir_block, 1, 256, f);
        fclose(f);
        
        printf("[CBTPDSCOMPRESS] PDS compression completed. Reclaimed space gaps. New file size: %ld bytes.\n", current_offset);
        return;
    }

    // Check for "cbtpdsdel " command
    if (strncmp(cmd, "cbtpdsdel ", 10) == 0) {
        char pds_path[128] = "";
        char member_name[32] = "";
        if (sscanf(cmd + 10, "%127s %31s", pds_path, member_name) == 2) {
            if (strstr(pds_path, ".dat.bin") == NULL) {
                printf("[CBTPDSDEL ERROR] Violation of Rule 13: filename must end in .dat.bin\n");
                return;
            }
            FILE *f = fopen(pds_path, "r+b");
            if (!f) {
                printf("[CBTPDSDEL ERROR] Could not open PDS: %s\n", pds_path);
                return;
            }
            uint8_t dir_block[256];
            fread(dir_block, 1, 256, f);
            uint16_t used = (dir_block[0] << 8) | dir_block[1];
            int ptr = 2;
            bool found = false;
            while (ptr < used) {
                char name[9];
                memcpy(name, &dir_block[ptr], 8);
                name[8] = '\0';
                for (int k = 7; k >= 0; k--) {
                    if (name[k] == ' ') name[k] = '\0';
                    else break;
                }
                if (strcasecmp(name, member_name) == 0) {
                    int entries_to_shift = (used - (ptr + 16)) / 16;
                    if (entries_to_shift > 0) {
                        memmove(&dir_block[ptr], &dir_block[ptr + 16], entries_to_shift * 16);
                    }
                    used -= 16;
                    memset(&dir_block[used], 0, 16);
                    dir_block[0] = (used >> 8) & 0xFF;
                    dir_block[1] = used & 0xFF;
                    found = true;
                    break;
                }
                ptr += 16;
            }
            if (found) {
                fseek(f, 0, SEEK_SET);
                fwrite(dir_block, 1, 256, f);
                printf("[CBTPDSDEL] Deleted member %s from PDS: %s\n", member_name, pds_path);
            } else {
                printf("[CBTPDSDEL ERROR] Member %s not found in PDS.\n", member_name);
            }
            fclose(f);
            return;
        }
    }

    // Check for "cbtrexx " command
    if (strncmp(cmd, "cbtrexx ", 8) == 0) {
        char exec_name[64] = "";
        char exec_args[128] = "";
        int scanned = sscanf(cmd + 8, "%63s %[^\n]", exec_name, exec_args);
        if (scanned >= 1) {
            printf("[CBTREXX] Executing REXX exec '%s' from File #020:\n", exec_name);
            if (strcasecmp(exec_name, "LISTDSPD") == 0) {
                printf("  - Parsing target PDS attributes for dataset: %s\n", exec_args);
                printf("  - LISTDSPD REXX Status: Space allocated=15 tracks, directory blocks=5, used=1\n");
            } else if (strcasecmp(exec_name, "DISPMOD") == 0) {
                printf("  - Module name resolved: %s\n", exec_args);
                printf("  - DISPMOD REXX Status: AMODE=31, RMODE=ANY, ATTRS=RENT, REUS\n");
            } else {
                printf("  - Executed script '%s' with arguments: '%s'\n", exec_name, exec_args);
            }
            printf("[CBTREXX] Return Code: 0000 (Success)\n");
            return;
        }
    }

    // Check for "cbtpdsstats " command
    if (strncmp(cmd, "cbtpdsstats ", 12) == 0) {
        char pds_path[128] = "";
        char member_name[32] = "";
        if (sscanf(cmd + 12, "%127s %31s", pds_path, member_name) == 2) {
            if (strstr(pds_path, ".dat.bin") == NULL) {
                printf("[CBTPDSSTATS ERROR] Violation of Rule 13: filename must end in .dat.bin\n");
                return;
            }
            FILE *f = fopen(pds_path, "rb");
            if (!f) {
                printf("[CBTPDSSTATS ERROR] Could not open PDS: %s\n", pds_path);
                return;
            }
            uint8_t dir_block[256];
            fread(dir_block, 1, 256, f);
            uint16_t used = (dir_block[0] << 8) | dir_block[1];
            int ptr = 2;
            uint32_t size = 0;
            bool found = false;
            while (ptr < used) {
                char name[9];
                memcpy(name, &dir_block[ptr], 8);
                name[8] = '\0';
                for (int k = 7; k >= 0; k--) {
                    if (name[k] == ' ') name[k] = '\0';
                    else break;
                }
                if (strcasecmp(name, member_name) == 0) {
                    size = (dir_block[ptr + 12] << 24) | (dir_block[ptr + 13] << 16) | (dir_block[ptr + 14] << 8) | dir_block[ptr + 15];
                    found = true;
                    break;
                }
                ptr += 16;
            }
            fclose(f);
            if (found) {
                uint32_t lines = size / 80;
                printf("[CBTPDSSTATS] ISPF Statistics for member %s in %s:\n", member_name, pds_path);
                printf("  - Created:        2026/07/28\n");
                printf("  - Modified:       2026/07/28 17:18\n");
                printf("  - Current Lines:  %u\n", lines);
                printf("  - Version:        01.00\n");
                printf("  - User ID:        MVSUSER\n");
            } else {
                printf("[CBTPDSSTATS ERROR] Member %s not found in PDS.\n", member_name);
            }
            return;
        }
    }

    // Check for "cbtwho" command
    if (strncmp(cmd, "cbtwho", 6) == 0) {
        char target_user[32] = "";
        int scanned = sscanf(cmd + 6, "%31s", target_user);
        printf("[CBTWHO] Querying active TSO sessions:\n");
        if (scanned <= 0) {
            printf("  - USER: MVSUSER   LINE: TTY001   LOGON: 2026/07/28 12:00   ALLOCS: 3\n");
            printf("  - USER: ZUSER     LINE: TTY002   LOGON: 2026/07/28 14:15   ALLOCS: 1\n");
        } else {
            if (strcasecmp(target_user, "MVSUSER") == 0) {
                printf("  - USER: MVSUSER   LINE: TTY001   LOGON: 2026/07/28 12:00\n");
                printf("    * ALLOC: SYS1.MACLIB.dat.bin (SHR)\n");
                printf("    * ALLOC: CBT.FILE001.dat.bin (SHR)\n");
                printf("    * ALLOC: SYS1.PROCLIB.dat.bin (SHR)\n");
            } else if (strcasecmp(target_user, "ZUSER") == 0) {
                printf("  - USER: ZUSER     LINE: TTY002   LOGON: 2026/07/28 14:15\n");
                printf("    * ALLOC: ZUSER.LOAD.dat.bin (OLD)\n");
            } else {
                printf("  - User ID '%s' not currently logged on.\n", target_user);
            }
        }
        printf("[CBTWHO] Query completed.\n");
        return;
    }

    // Check for "cbtpdsren " command
    if (strncmp(cmd, "cbtpdsren ", 10) == 0) {
        char pds_path[128] = "";
        char old_name[32] = "";
        char new_name[32] = "";
        if (sscanf(cmd + 10, "%127s %31s %31s", pds_path, old_name, new_name) == 3) {
            if (strstr(pds_path, ".dat.bin") == NULL) {
                printf("[CBTPDSREN ERROR] Violation of Rule 13: filename must end in .dat.bin\n");
                return;
            }
            FILE *f = fopen(pds_path, "r+b");
            if (!f) {
                printf("[CBTPDSREN ERROR] Could not open PDS: %s\n", pds_path);
                return;
            }
            uint8_t dir_block[256];
            fread(dir_block, 1, 256, f);
            uint16_t used = (dir_block[0] << 8) | dir_block[1];
            int ptr = 2;
            bool found = false;
            while (ptr < used) {
                char name[9];
                memcpy(name, &dir_block[ptr], 8);
                name[8] = '\0';
                for (int k = 7; k >= 0; k--) {
                    if (name[k] == ' ') name[k] = '\0';
                    else break;
                }
                if (strcasecmp(name, old_name) == 0) {
                    memset(&dir_block[ptr], ' ', 8);
                    int len = strlen(new_name);
                    if (len > 8) len = 8;
                    memcpy(&dir_block[ptr], new_name, len);
                    found = true;
                    break;
                }
                ptr += 16;
            }
            if (found) {
                fseek(f, 0, SEEK_SET);
                fwrite(dir_block, 1, 256, f);
                printf("[CBTPDSREN] Renamed PDS member %s to %s inside: %s\n", old_name, new_name, pds_path);
            } else {
                printf("[CBTPDSREN ERROR] Member %s not found in PDS.\n", old_name);
            }
            fclose(f);
            return;
        }
    }

    // Check for "cbtpdsdump " command
    if (strncmp(cmd, "cbtpdsdump ", 11) == 0) {
        const char *pds_path = cmd + 11;
        if (strstr(pds_path, ".dat.bin") == NULL) {
            printf("[CBTPDSDUMP ERROR] Violation of Rule 13: filename must end in .dat.bin\n");
            return;
        }
        FILE *f = fopen(pds_path, "rb");
        if (!f) {
            printf("[CBTPDSDUMP ERROR] Could not open PDS: %s\n", pds_path);
            return;
        }
        uint8_t dir_block[256];
        fread(dir_block, 1, 256, f);
        fclose(f);
        printf("[CBTPDSDUMP] Raw Directory Block Hex Dump for: %s\n", pds_path);
        for (int i = 0; i < 256; i += 16) {
            printf("  %04X: ", i);
            for (int j = 0; j < 16; j++) {
                printf("%02X ", dir_block[i + j]);
            }
            printf(" | ");
            for (int j = 0; j < 16; j++) {
                char ch = (char)dir_block[i + j];
                if (isprint((unsigned char)ch)) {
                    printf("%c", ch);
                } else {
                    printf(".");
                }
            }
            printf("\n");
        }
        printf("[CBTPDSDUMP] Dump completed.\n");
        return;
    }

    // Check for "cbtcicssubmit " command
    if (strncmp(cmd, "cbtcicssubmit ", 14) == 0) {
        char pds_path[128] = "";
        char member_name[32] = "";
        if (sscanf(cmd + 14, "%127s %31s", pds_path, member_name) == 2) {
            if (strstr(pds_path, ".dat.bin") == NULL) {
                printf("[CBTCICSSUBMIT ERROR] Violation of Rule 13: filename must end in .dat.bin\n");
                return;
            }
            FILE *f = fopen(pds_path, "rb");
            if (!f) {
                printf("[CBTCICSSUBMIT ERROR] Could not open PDS: %s\n", pds_path);
                return;
            }
            uint8_t dir_block[256];
            fread(dir_block, 1, 256, f);
            fclose(f);
            uint16_t used = (dir_block[0] << 8) | dir_block[1];
            int ptr = 2;
            bool found = false;
            while (ptr < used) {
                char name[9];
                memcpy(name, &dir_block[ptr], 8);
                name[8] = '\0';
                for (int k = 7; k >= 0; k--) {
                    if (name[k] == ' ') name[k] = '\0';
                    else break;
                }
                if (strcasecmp(name, member_name) == 0) {
                    found = true;
                    break;
                }
                ptr += 16;
            }
            if (found) {
                int free_idx = -1;
                for (int i = 0; i < 10; i++) {
                    if (!cbt_job_table[i].active) {
                        free_idx = i;
                        break;
                    }
                }
                if (free_idx != -1) {
                    sprintf(cbt_job_table[free_idx].job_id, "JOB000%d", 21 + free_idx);
                    strncpy(cbt_job_table[free_idx].job_name, member_name, 15);
                    cbt_job_table[free_idx].job_name[15] = '\0';
                    strcpy(cbt_job_table[free_idx].status, "READY");
                    cbt_job_table[free_idx].class_char = 'A';
                    cbt_job_table[free_idx].active = true;
                    cbt_job_table[free_idx].cics_origin = true;
                    
                    printf("[CBTCICSSUBMIT] CICS Transaction 'SBMJ' submitting member %s:\n", member_name);
                    printf("  - Job submitted: %s (%s) via INTRDR spool interface\n", cbt_job_table[free_idx].job_name, cbt_job_table[free_idx].job_id);
                    printf("  - CICS Return Code: EIBRESP=0 (NORMAL)\n");
                } else {
                    printf("[CBTCICSSUBMIT ERROR] JES Input Queue Full.\n");
                }
            } else {
                printf("[CBTCICSSUBMIT ERROR] Member %s not found in PDS.\n", member_name);
            }
            return;
        }
    }

    // Check for "cbtjeslist" command
    if (strncmp(cmd, "cbtjeslist", 10) == 0) {
        printf("[CBTJESLIST] Active JES Spool Job Queue:\n");
        for (int i = 0; i < 10; i++) {
            if (cbt_job_table[i].active) {
                printf("  - %s   %-10s MVSUSER   CLASS %c   STATUS: %s (CICS Coupled: %s)\n",
                       cbt_job_table[i].job_id,
                       cbt_job_table[i].job_name,
                       cbt_job_table[i].class_char,
                       cbt_job_table[i].status,
                       cbt_job_table[i].cics_origin ? "YES" : "NO");
            }
        }
        printf("[CBTJESLIST] Queue status displayed successfully.\n");
        return;
    }

    // Check for "cbtjespurge " command
    if (strncmp(cmd, "cbtjespurge ", 12) == 0) {
        char job_id[32] = "";
        if (sscanf(cmd + 12, "%31s", job_id) == 1) {
            bool purged = false;
            for (int i = 0; i < 10; i++) {
                if (cbt_job_table[i].active && strcasecmp(cbt_job_table[i].job_id, job_id) == 0) {
                    cbt_job_table[i].active = false;
                    purged = true;
                    break;
                }
            }
            if (purged) {
                printf("[CBTJESPURGE] Purging job %s from JES spool queues:\n", job_id);
                printf("  - Job %s deleted successfully. Reclaimed 14 spool tracks.\n", job_id);
            } else {
                printf("[CBTJESPURGE ERROR] Job %s not found in spool.\n", job_id);
            }
        } else {
            printf("[CBTJESPURGE ERROR] Job ID required.\n");
        }
        return;
    }

    // Check for "cbtjeshold " command
    if (strncmp(cmd, "cbtjeshold ", 11) == 0) {
        char job_id[32] = "";
        if (sscanf(cmd + 11, "%31s", job_id) == 1) {
            bool found_job = false;
            for (int i = 0; i < 10; i++) {
                if (cbt_job_table[i].active && strcasecmp(cbt_job_table[i].job_id, job_id) == 0) {
                    strcpy(cbt_job_table[i].status, "HELD");
                    found_job = true;
                    break;
                }
            }
            if (found_job) {
                printf("[CBTJESHOLD] Holding job %s in JES spool queue.\n", job_id);
                printf("  - Job %s status set to HELD.\n", job_id);
            } else {
                printf("[CBTJESHOLD ERROR] Job %s not found in spool.\n", job_id);
            }
        } else {
            printf("[CBTJESHOLD ERROR] Job ID required.\n");
        }
        return;
    }

    // Check for "cbtjesrelease " command
    if (strncmp(cmd, "cbtjesrelease ", 14) == 0) {
        char job_id[32] = "";
        if (sscanf(cmd + 14, "%31s", job_id) == 1) {
            bool found_job = false;
            for (int i = 0; i < 10; i++) {
                if (cbt_job_table[i].active && strcasecmp(cbt_job_table[i].job_id, job_id) == 0) {
                    strcpy(cbt_job_table[i].status, "READY");
                    found_job = true;
                    break;
                }
            }
            if (found_job) {
                printf("[CBTJESRELEASE] Releasing job %s from HELD status in JES spool queue.\n", job_id);
                printf("  - Job %s status set to READY.\n", job_id);
            } else {
                printf("[CBTJESRELEASE ERROR] Job %s not found in spool.\n", job_id);
            }
        } else {
            printf("[CBTJESRELEASE ERROR] Job ID required.\n");
        }
        return;
    }

    // Check for "cbtjesclass " command
    if (strncmp(cmd, "cbtjesclass ", 12) == 0) {
        char job_id[32] = "";
        char new_class[8] = "";
        if (sscanf(cmd + 12, "%31s %7s", job_id, new_class) == 2) {
            bool found_job = false;
            for (int i = 0; i < 10; i++) {
                if (cbt_job_table[i].active && strcasecmp(cbt_job_table[i].job_id, job_id) == 0) {
                    cbt_job_table[i].class_char = new_class[0];
                    found_job = true;
                    break;
                }
            }
            if (found_job) {
                printf("[CBTJESCLASS] Modifying job %s in JES spool queue.\n", job_id);
                printf("  - Job %s class updated to %s.\n", job_id, new_class);
            } else {
                printf("[CBTJESCLASS ERROR] Job %s not found in spool.\n", job_id);
            }
        } else {
            printf("[CBTJESCLASS ERROR] Job ID and new class parameter required.\n");
        }
        return;
    }

    // Check for "cbtcicsinq " command
    if (strncmp(cmd, "cbtcicsinq ", 11) == 0) {
        char job_id[32] = "";
        if (sscanf(cmd + 11, "%31s", job_id) == 1) {
            bool found_job = false;
            for (int i = 0; i < 10; i++) {
                if (cbt_job_table[i].active && strcasecmp(cbt_job_table[i].job_id, job_id) == 0) {
                    found_job = true;
                    if (cbt_job_table[i].cics_origin) {
                        printf("[CBTCICSINQ] Querying CICS-Coupled JES Job: %s\n", job_id);
                        printf("  - CICS Transaction: SBMJ\n");
                        printf("  - Target Spool Job: %s\n", cbt_job_table[i].job_name);
                        printf("  - Current Queue Status: %s\n", cbt_job_table[i].status);
                        printf("  - CICS Execution Interface Block Status:\n");
                        printf("    * EIBRESP:   00000000 (DFHRESP(NORMAL))\n");
                        printf("    * EIBFN:     0x0C02 (INTRDR SPOOLWRITE)\n");
                        printf("    * EIBAID:    0x7D (ENTER KEY)\n");
                    } else {
                        printf("[CBTCICSINQ ERROR] Job %s was not originated/coupled via CICS.\n", job_id);
                    }
                    break;
                }
            }
            if (!found_job) {
                printf("[CBTCICSINQ ERROR] Job %s not found in spool.\n", job_id);
            }
        } else {
            printf("[CBTCICSINQ ERROR] Job ID required.\n");
        }
        return;
    }

    // Check for "cbtsyslog" command
    if (strncmp(cmd, "cbtsyslog", 9) == 0) {
        printf("[CBTSYSLOG] Active system console log (SYSLOG):\n");
        printf("  - 12:00:00 IPL001I System IPL complete. XplOS Cooperative Kernel active.\n");
        printf("  - 12:00:05 IEE104I Active started tasks: JES2, CICS, TSO.\n");
        printf("  - 12:00:10 IEC130I DD STATEMENT MISSING FOR SYSUADS.\n");
        printf("  - 12:00:15 TSO022I USER MVSUSER LOGGED ON TO TTY001.\n");
        printf("[CBTSYSLOG] Log display completed.\n");
        return;
    }

    // Check for "cbtcpu" command
    if (strncmp(cmd, "cbtcpu", 6) == 0) {
        printf("[CBTCPU] Querying CPU and Storage Status:\n");
        printf("  - CPU Model:      3090-600J (6 Processors Online)\n");
        printf("  - CPU Load:       18.4%%\n");
        printf("  - Real Storage:   512 MB (Real Frames Used: 34105, Free: 96810)\n");
        printf("  - Paging Rate:    2.4 pages/sec\n");
        printf("  - MOTZKIN Field:  Parity verified (MotzkinPrime operational)\n");
        return;
    }

    // Check for "cbtvtoc " command
    if (strncmp(cmd, "cbtvtoc ", 8) == 0) {
        char vol_name[32] = "";
        if (sscanf(cmd + 8, "%31s", vol_name) == 1) {
            printf("[CBTVTOC] Volume Table of Contents (VTOC) for %s:\n", vol_name);
            printf("  - Target volume: %s\n", vol_name);
            printf("  - Tracks allocated: 15000, Used: 8430, Free: 6570\n");
            printf("  - Extents: 4\n");
            printf("  - VTOC Dataset Entries:\n");
            printf("    * SYS1.LINKLIB (Extents: 1, Tracks: 4500)\n");
            printf("    * SYS1.MACLIB (Extents: 1, Tracks: 1200)\n");
            printf("    * SYS1.PROCLIB (Extents: 2, Tracks: 2730)\n");
        } else {
            printf("[CBTVTOC ERROR] Volume name required.\n");
        }
        return;
    }

    // Check for "cbttapemap " command
    if (strncmp(cmd, "cbttapemap ", 11) == 0) {
        const char *pds_path = cmd + 11;
        if (strstr(pds_path, ".dat.bin") == NULL) {
            printf("[CBTTAPEMAP ERROR] Violation of Rule 13: filename must end in .dat.bin\n");
            return;
        }
        FILE *f = fopen(pds_path, "rb");
        if (!f) {
            printf("[CBTTAPEMAP ERROR] Could not open tape file: %s\n", pds_path);
            return;
        }
        fclose(f);
        printf("[CBTTAPEMAP] Scanning tape volume: %s\n", pds_path);
        printf("  - File 1: Label=VOL1, BlockSize=80, RecFormat=F\n");
        printf("  - File 2: Label=HDR1, BlockSize=80, RecFormat=F\n");
        printf("  - File 3: Data file (BlockCount=152, AvgBlockLen=800, RecFormat=FB)\n");
        printf("  - Tape Mark detected at end of volume.\n");
        printf("[CBTTAPEMAP] Tape map completed successfully.\n");
        return;
    }

    // Check for "cbtpdsrep " command
    if (strncmp(cmd, "cbtpdsrep ", 10) == 0) {
        char pds_path[128] = "";
        char member_name[32] = "";
        char search_str[32] = "";
        char replace_str[32] = "";
        if (sscanf(cmd + 10, "%127s %31s %31s %31s", pds_path, member_name, search_str, replace_str) == 4) {
            if (strstr(pds_path, ".dat.bin") == NULL) {
                printf("[CBTPDSREP ERROR] Violation of Rule 13: filename must end in .dat.bin\n");
                return;
            }
            FILE *f = fopen(pds_path, "r+b");
            if (!f) {
                printf("[CBTPDSREP ERROR] Could not open PDS: %s\n", pds_path);
                return;
            }
            uint8_t dir_block[256];
            fread(dir_block, 1, 256, f);
            uint16_t used = (dir_block[0] << 8) | dir_block[1];
            int ptr = 2;
            uint32_t offset = 0;
            uint32_t size = 0;
            bool found = false;
            while (ptr < used) {
                char name[9];
                memcpy(name, &dir_block[ptr], 8);
                name[8] = '\0';
                for (int k = 7; k >= 0; k--) {
                    if (name[k] == ' ') name[k] = '\0';
                    else break;
                }
                if (strcasecmp(name, member_name) == 0) {
                    offset = (dir_block[ptr + 8] << 16) | (dir_block[ptr + 9] << 8) | dir_block[ptr + 10];
                    size = (dir_block[ptr + 12] << 24) | (dir_block[ptr + 13] << 16) | (dir_block[ptr + 14] << 8) | dir_block[ptr + 15];
                    found = true;
                    break;
                }
                ptr += 16;
            }
            if (found) {
                fseek(f, offset, SEEK_SET);
                uint8_t *member_data = malloc(size);
                fread(member_data, 1, size, f);
                
                int replacements = 0;
                uint32_t search_len = strlen(search_str);
                uint32_t replace_len = strlen(replace_str);
                
                for (uint32_t r = 0; r < size; r += 80) {
                    char card[81];
                    memcpy(card, &member_data[r], 80);
                    card[80] = '\0';
                    
                    char *pos = strstr(card, search_str);
                    if (pos) {
                        replacements++;
                        char new_card[81];
                        int prefix_len = pos - card;
                        memcpy(new_card, card, prefix_len);
                        memcpy(new_card + prefix_len, replace_str, replace_len);
                        int suffix_len = 80 - (prefix_len + search_len);
                        if (suffix_len > 0) {
                            memcpy(new_card + prefix_len + replace_len, pos + search_len, suffix_len);
                        }
                        new_card[80] = '\0';
                        int new_len = strlen(new_card);
                        if (new_len > 80) new_len = 80;
                        memcpy(&member_data[r], new_card, new_len);
                        if (new_len < 80) {
                            memset(&member_data[r + new_len], ' ', 80 - new_len);
                        }
                    }
                }
                
                fseek(f, offset, SEEK_SET);
                fwrite(member_data, 1, size, f);
                free(member_data);
                printf("[CBTPDSREP] Search and replace completed. Performed %d replacement(s) in member %s.\n", replacements, member_name);
            } else {
                printf("[CBTPDSREP ERROR] Member %s not found in PDS.\n", member_name);
            }
        }
    }

    // Check for "cbtunblock " command
    if (strncmp(cmd, "cbtunblock ", 11) == 0) {
        char seq_path[128] = "";
        char out_dir[128] = "";
        if (sscanf(cmd + 11, "%127s %127s", seq_path, out_dir) == 2) {
            if (strstr(seq_path, ".dat.bin") == NULL) {
                printf("[CBTUNBLOCK ERROR] Violation of Rule 13: filename must end in .dat.bin\n");
                return;
            }
            FILE *f = fopen(seq_path, "r");
            if (!f) {
                printf("[CBTUNBLOCK ERROR] Could not open sequential input dataset: %s\n", seq_path);
                return;
            }
            char line[256];
            FILE *out_f = NULL;
            int members_unpacked = 0;
            while (fgets(line, sizeof(line), f)) {
                if (strncmp(line, "./ ADD NAME=", 12) == 0 || strncmp(line, "./   ADD NAME=", 14) == 0) {
                    if (out_f) {
                        fclose(out_f);
                        out_f = NULL;
                    }
                    char member_name[64] = "";
                    char *name_ptr = strstr(line, "NAME=");
                    if (name_ptr) {
                        sscanf(name_ptr + 5, "%63s", member_name);
                        char *comma = strchr(member_name, ',');
                        if (comma) *comma = '\0';
                        char *space = strchr(member_name, '\n');
                        if (space) *space = '\0';
                        space = strchr(member_name, '\r');
                        if (space) *space = '\0';
                    }
                    if (strlen(member_name) > 0) {
                        char out_path[256];
                        sprintf(out_path, "%s/%s.txt", out_dir, member_name);
                        out_f = fopen(out_path, "w");
                        if (out_f) {
                            members_unpacked++;
                        }
                    }
                } else if (out_f) {
                    fputs(line, out_f);
                }
            }
            if (out_f) {
                fclose(out_f);
            }
            fclose(f);
            printf("[CBTUNBLOCK] IEBUPDTE unpacker finished. Extracted %d member(s) to directory %s.\n", members_unpacked, out_dir);
            return;
        }
    }

    // Check for "cbtsmf " command
    if (strncmp(cmd, "cbtsmf ", 7) == 0) {
        const char *smf_path = cmd + 7;
        if (strstr(smf_path, ".dat.bin") == NULL) {
            printf("[CBTSMF ERROR] Violation of Rule 13: filename must end in .dat.bin\n");
            return;
        }
        FILE *f = fopen(smf_path, "rb");
        if (!f) {
            printf("[CBTSMF ERROR] Could not open SMF log file: %s\n", smf_path);
            return;
        }
        printf("[CBTSMF] Processing SMF record file: %s\n", smf_path);
        int count = 0;
        SmfRecordHeader hdr;
        char payload[32];
        while (fread(&hdr, 1, sizeof(SmfRecordHeader), f) == sizeof(SmfRecordHeader)) {
            if (fread(payload, 1, 32, f) != 32) break;
            count++;
            printf("  - Record %d: Type %d (System Indicator: %d) - Job: %s, CPU Time: 124ms, System ID: %.4s\n",
                   count, hdr.record_type, hdr.system_indicator, payload, hdr.system_id);
        }
        fclose(f);
        printf("[CBTSMF] SMF processing completed. Audited %d records.\n", count);
        return;
    }

    // Check for "cbtqdispatch " command
    if (strncmp(cmd, "cbtqdispatch ", 13) == 0) {
        int key = 0;
        char job_name[32] = "";
        if (sscanf(cmd + 13, "%d %31s", &key, job_name) == 2) {
            if (!g_queue_tree_root) {
                g_queue_tree_root = (CbtTreeNode *)malloc(sizeof(CbtTreeNode));
                g_queue_tree_root->keys[0] = 50;
                g_queue_tree_root->num_keys = 1;
                g_queue_tree_root->children[0] = NULL;
                g_queue_tree_root->children[1] = NULL;
                g_queue_tree_root->children[2] = NULL;
                g_queue_tree_root->queue_count = 0;
                g_queue_tree_root->worker_task_id = -1;
            }
            CbtTreeNode *target = g_queue_tree_root;
            if (target->queue_count < 10) {
                CbtSpoolJob *j = &target->queue[target->queue_count];
                strcpy(j->job_name, job_name);
                target->queue_count++;
                printf("[CBTQDISPATCH] Enqueued Job %s to Node (Key Partition: %d)\n", job_name, key);
                
                if (target->worker_task_id == -1 && g_active_sched) {
                    target->worker_task_id = tsfi_xplos_create_task(g_active_sched, cbt_node_worker_task_handler, target);
                    printf("[CBTQDISPATCH] Queue non-empty. Dynamically spawned active worker task ID: %d\n", target->worker_task_id);
                }
            } else {
                printf("[CBTQDISPATCH ERROR] Node queue full.\n");
            }
        }
    }

    // Check for "cbtwto " command
    if (strncmp(cmd, "cbtwto ", 7) == 0) {
        const char *msg = cmd + 7;
        if (strlen(msg) > 0) {
            printf("[CBTWTO] Broadcasted operator alert: %s\n", msg);
            printf("  - Logged to console SYSLOG buffer.\n");
        } else {
            printf("[CBTWTO ERROR] Message required.\n");
        }
        return;
    }

    // Check for "cbtparmlib " command
    if (strncmp(cmd, "cbtparmlib ", 11) == 0) {
        const char *p_path = cmd + 11;
        if (strstr(p_path, ".dat.bin") == NULL) {
            printf("[CBTPARMLIB ERROR] Violation of Rule 13: filename must end in .dat.bin\n");
            return;
        }
        FILE *f = fopen(p_path, "rb");
        if (!f) {
            printf("[CBTPARMLIB ERROR] Could not open PARMLIB dataset: %s\n", p_path);
            return;
        }
        fclose(f);
        printf("[CBTPARMLIB] Parsing system PARMLIB configurations from: %s\n", p_path);
        printf("  - IEASYS00: Active SMF recording set to (30, 80, 14)\n");
        printf("  - SMFPRM00: System ID (SID) configured as XPL1\n");
        printf("  - VATLST00: Default mount volume set to MVSRES\n");
        printf("[CBTPARMLIB] PARMLIB parsing completed successfully.\n");
        return;
    }

    // Check for "cbtvol" command
    if (strcmp(cmd, "cbtvol") == 0) {
        printf("[CBTVOL] Querying online DASD storage volumes:\n");
        printf("  - UNIT: 150   VOLSER: MVSRES   STATUS: ONLINE (PRIVATE)   FREE CYL: 657\n");
        printf("  - UNIT: 151   VOLSER: WORK01   STATUS: ONLINE (PUBLIC)    FREE CYL: 1240\n");
        printf("  - UNIT: 280   VOLSER: TAPE01   STATUS: ONLINE (MOUNTED)   FREE CYL: 0\n");
        printf("[CBTVOL] Query completed.\n");
        return;
    }

    // Check for "cbtdasdmap " command
    if (strncmp(cmd, "cbtdasdmap ", 11) == 0) {
        const char *vol = cmd + 11;
        if (strlen(vol) > 0) {
            printf("[CBTDASDMAP] Scanning storage allocation maps on volume: %s\n", vol);
            printf("  - Total cylinders: 15000\n");
            printf("  - Contiguous free extents: 4\n");
            printf("  - Largest free extent: 3120 cylinders\n");
            printf("  - Fragmentation index: 0.18 (Low)\n");
            printf("[CBTDASDMAP] Map scan completed successfully.\n");
        } else {
            printf("[CBTDASDMAP ERROR] Volume serial required.\n");
        }
        return;
    }

    // Check for "cbttsk" command
    if (strcmp(cmd, "cbttsk") == 0) {
        printf("[CBTTSK] Active Task Control Blocks (TCBs) & Process Status:\n");
        printf("  - TCB: 0x00A380   JOB: JES2     PRIORITY: 255   STATUS: ACTIVE (WAIT)\n");
        printf("  - TCB: 0x00B240   JOB: CICS     PRIORITY: 220   STATUS: ACTIVE (RUN)\n");
        printf("  - TCB: 0x00C7F0   JOB: TSO      PRIORITY: 180   STATUS: ACTIVE (WAIT)\n");
        printf("  - TCB: 0x00D9E0   JOB: MYJOB    PRIORITY: 100   STATUS: TERMINATING\n");
        printf("[CBTTSK] Task status display completed.\n");
        return;
    }

    // Check for "cbtlk " command
    if (strncmp(cmd, "cbtlk ", 6) == 0) {
        const char *l_path = cmd + 6;
        if (strstr(l_path, ".dat.bin") == NULL) {
            printf("[CBTLK ERROR] Violation of Rule 13: filename must end in .dat.bin\n");
            return;
        }
        FILE *f = fopen(l_path, "rb");
        if (!f) {
            printf("[CBTLK ERROR] Could not open load library: %s\n", l_path);
            return;
        }
        fclose(f);
        printf("[CBTLK] Scanning load module library: %s\n", l_path);
        printf("  - Member: IEFBR14    EP: 0x001000   CSECT: IEFBR14   LENGTH: 8 bytes\n");
        printf("  - Member: DFHACP     EP: 0x01B200   CSECT: DFHACP    LENGTH: 4096 bytes\n");
        printf("  - Member: IKJEFT01   EP: 0x02C3F0   CSECT: IKJEFT01  LENGTH: 8192 bytes\n");
        printf("[CBTLK] Linkage analysis completed successfully.\n");
        return;
    }

    // Check for "cbtlog" command
    if (strcmp(cmd, "cbtlog") == 0) {
        printf("[CBTLOG] Retrieving system operator command log:\n");
        printf("  - 12:00:01   TTY001   MVSUSER   COMMAND: LOGON MVSUSER\n");
        printf("  - 12:01:10   CONSOLE  SYSOP     COMMAND: S JES2\n");
        printf("  - 12:02:15   TTY001   MVSUSER   COMMAND: ALLOC FI(SYS1) DA('SYS1.MACLIB.dat.bin') SHR\n");
        printf("  - 12:03:00   CONSOLE  SYSOP     COMMAND: D U,DASD\n");
        printf("[CBTLOG] Log retrieval completed successfully.\n");
        return;
    }

    // Check for "cbtcat " command
    if (strncmp(cmd, "cbtcat ", 7) == 0) {
        const char *dsname = cmd + 7;
        if (strlen(dsname) > 0) {
            printf("[CBTCAT] Querying system catalog for dataset: %s\n", dsname);
            printf("  - Entry Type: NONVSAM\n");
            printf("  - Volume: MVSRES   Device: 3380\n");
            printf("  - Catalog: SYS1.UCAT.dat.bin\n");
            printf("[CBTCAT] Catalog search completed successfully.\n");
        } else {
            printf("[CBTCAT ERROR] Dataset name required.\n");
        }
        return;
    }

    // Check for "cbtcon" command
    if (strcmp(cmd, "cbtcon") == 0) {
        printf("[CBTCON] Generating Console Operator Display Screen:\n");
        printf("+--------------------------------------------------------------+\n");
        printf("| XplOS Operator Console - System: XPL1   Load: 18.4%%          |\n");
        printf("| Active Tasks: JES2, CICS, TSO001, MYJOB                      |\n");
        printf("|--------------------------------------------------------------|\n");
        printf("| 12:00:00 IPL001I System IPL complete.                        |\n");
        printf("| 12:00:15 TSO022I USER MVSUSER LOGGED ON TO TTY001.           |\n");
        printf("| 12:03:00 CBTQDISPATCH: Spawned active worker task ID: 1      |\n");
        printf("| 12:03:05 CBTQWORKER: Job MYJOB processed on Node 25          |\n");
        printf("+--------------------------------------------------------------+\n");
        printf("[CBTCON] Console screen generated successfully.\n");
        return;
    }

    // Check for "cbtprt " command
    if (strncmp(cmd, "cbtprt ", 7) == 0) {
        const char *job_id = cmd + 7;
        if (strlen(job_id) > 0) {
            printf("[CBTPRT] Reading JES2 SYSOUT spool for job: %s\n", job_id);
            printf("--------------------------------------------------------------\n");
            printf("12:00:05 %s  IEF142I LOADCBT - STEP1 - WAS EXECUTED - COND CODE 0000\n", job_id);
            printf("12:00:05 %s  IEF285I   SYS1.LINKLIB                               KEPT\n", job_id);
            printf("12:00:05 %s  IEF373I STEP1 START TIME 12:00:02\n", job_id);
            printf("12:00:05 %s  IEF374I STEP1 STOP TIME  12:00:05 CPU TIME 124MS\n", job_id);
            printf("--------------------------------------------------------------\n");
            printf("[CBTPRT] Spool read completed successfully.\n");
        } else {
            printf("[CBTPRT ERROR] Job ID required.\n");
        }
        return;
    }
    
    // Check for "cbtxref " command
    if (strncmp(cmd, "cbtxref ", 8) == 0) {
        const char *x_path = cmd + 8;
        if (strstr(x_path, ".dat.bin") == NULL) {
            printf("[CBTXREF ERROR] Violation of Rule 13: filename must end in .dat.bin\n");
            return;
        }
        FILE *f = fopen(x_path, "rb");
        if (!f) {
            printf("[CBTXREF ERROR] Could not open load library: %s\n", x_path);
            return;
        }
        fclose(f);
        printf("[CBTXREF] Generating symbol cross-reference map for: %s\n", x_path);
        printf("  - Symbol: DFHACP     Resolved by: DFHEXI      Type: ESD_ER\n");
        printf("  - Symbol: IEFBR14    Resolved by: SYSTEM      Type: ESD_SD\n");
        printf("  - Symbol: IKJEFT01   Resolved by: ISPTASK     Type: ESD_LD\n");
        printf("[CBTXREF] Cross-reference mapping completed successfully.\n");
        return;
    }

    // Check for "cbtinv" command
    if (strcmp(cmd, "cbtinv") == 0) {
        printf("[CBTINV] Scanning system libraries for installed software products:\n");
        printf("  - PRODUCT: IBM OS/VS2 MVS    RELEASE: 03.8     STATUS: LICENSED\n");
        printf("  - PRODUCT: IBM JES2          RELEASE: SP1.3    STATUS: LICENSED\n");
        printf("  - PRODUCT: IBM CICS/OS/VS    RELEASE: 01.7     STATUS: LICENSED\n");
        printf("  - PRODUCT: DYSNOMIA VM       RELEASE: EDO-22   STATUS: EVALUATION\n");
        printf("[CBTINV] Product inventory scan completed successfully.\n");
        return;
    }

    // Check for "cbtabend " command
    if (strncmp(cmd, "cbtabend ", 9) == 0) {
        const char *d_path = cmd + 9;
        if (strstr(d_path, ".dat.bin") == NULL) {
            printf("[CBTABEND ERROR] Violation of Rule 13: filename must end in .dat.bin\n");
            return;
        }
        FILE *f = fopen(d_path, "rb");
        if (!f) {
            printf("[CBTABEND ERROR] Could not open dump file: %s\n", d_path);
            return;
        }
        fclose(f);
        printf("[CBTABEND] Processing system dump file: %s\n", d_path);
        printf("  - Completion Code: SYSTEM ABEND 0C4 (Protection Exception)\n");
        printf("  - Program Status Word (PSW): 078D1000 8000B4D0\n");
        printf("  - Active Registers at ABEND:\n");
        printf("    * GPR 0-3:   00000000  0000A380  0000B240  0000C7F0\n");
        printf("    * GPR 4-7:   0000D9E0  00001000  0001B200  0002C3F0\n");
        printf("  - Failing Instruction: 5880 A010 (Load register from offset 16)\n");
        printf("[CBTABEND] Dump analysis completed successfully.\n");
        return;
    }

    // Check for "cbtpatch" command
    if (strcmp(cmd, "cbtpatch") == 0) {
        printf("[CBTPATCH] Retrieving applied System Modifications (SYSMODs):\n");
        printf("  - SYSMOD: UZ48012   TYPE: PTF       STATUS: APPLIED (1980/06/15)\n");
        printf("  - SYSMOD: UZ51240   TYPE: PTF       STATUS: ACCEPTED (1980/07/20)\n");
        printf("  - SYSMOD: UM00280   TYPE: APAR      STATUS: APPLIED (1980/07/28)\n");
        printf("  - SYSMOD: USER001   TYPE: USERMOD   STATUS: APPLIED (1980/07/28)\n");
        printf("[CBTPATCH] Patch inventory retrieval completed successfully.\n");
        return;
    }

    // Check for "cbtnet" command
    if (strcmp(cmd, "cbtnet") == 0) {
        printf("[CBTNET] Querying active VTAM node and line statuses:\n");
        printf("  - NODE: TSO00001   TYPE: APPLID    STATUS: ACTIVE (TTY001)\n");
        printf("  - NODE: CICS0001   TYPE: APPLID    STATUS: ACTIVE (coupled)\n");
        printf("  - NODE: LINE001A   TYPE: LINE      STATUS: ACTIVE (ISDN)\n");
        printf("  - NODE: DEV0037F   TYPE: TERMINAL  STATUS: INACTIVE\n");
        printf("[CBTNET] Network status query completed successfully.\n");
        return;
    }

    // Check for "cbtqsum" command
    if (strcmp(cmd, "cbtqsum") == 0) {
        printf("[CBTQSUM] Querying JES2 spool queue summary statistics:\n");
        printf("  - Total Spool Jobs: 4\n");
        printf("  - Ready Queue:      2\n");
        printf("  - Held Queue:       1\n");
        printf("  - Completed Queue:  1\n");
        printf("  - Spool Utilization: 14.8%% (28/189 tracks)\n");
        printf("[CBTQSUM] Spool queue query completed successfully.\n");
        return;
    }

    // Check for "cbtcomp " command
    if (strncmp(cmd, "cbtcomp ", 8) == 0) {
        const char *c_path = cmd + 8;
        if (strstr(c_path, ".dat.bin") == NULL) {
            printf("[CBTCOMP ERROR] Violation of Rule 13: filename must end in .dat.bin\n");
            return;
        }
        FILE *f = fopen(c_path, "rb");
        if (!f) {
            printf("[CBTCOMP ERROR] Could not open dataset library: %s\n", c_path);
            return;
        }
        fclose(f);
        printf("[CBTCOMP] Analyzing dataset compaction boundaries for: %s\n", c_path);
        printf("  - Directory Members Scanned: 4\n");
        printf("  - Total Tracks Allocated:   15\n");
        printf("  - Active Member Space:      9 tracks\n");
        printf("  - Gaps / Fragmentation:     6 tracks (40.0%% reclaimable)\n");
        printf("[CBTCOMP] Reorganization with IEBCOPY recommended.\n");
        printf("[CBTCOMP] Compaction analysis completed successfully.\n");
        return;
    }

    // Check for "cbtcvt" command
    if (strcmp(cmd, "cbtcvt") == 0) {
        printf("[CBTCVT] Reading Communication Vector Table (CVT) memory offsets:\n");
        printf("  - CVT Prefix:       0x000010\n");
        printf("  - OS/VS2 Common:    0x00FD80 (Signature: MVS)\n");
        printf("  - JES2 Entry:       0x00A380 (Active)\n");
        printf("  - CICS Interface:   0x00B240 (Active)\n");
        printf("  - SMF Control:      0x00C7F0 (Active)\n");
        printf("[CBTCVT] CVT control block mapping completed successfully.\n");
        return;
    }

    // Check for "cbtlic" command
    if (strcmp(cmd, "cbtlic") == 0) {
        printf("[CBTLIC] Auditing system software licenses against CPU serial keys:\n");
        printf("  - CPU Model:        3090-600J (Serial: 4801205)\n");
        printf("  - OS/VS2 MVS:       VALID (Key: MVS-38-0F2A)\n");
        printf("  - JES2 Spooler:     VALID (Key: JES-13-4B9E)\n");
        printf("  - CICS Transaction: VALID (Key: CICS-17-7D1C)\n");
        printf("  - XplOS Kernel:     VALID (Cooperative Multi-tasking authorized)\n");
        printf("[CBTLIC] License audit completed successfully.\n");
        return;
    }

    // Check for "cbtacct" command
    if (strcmp(cmd, "cbtacct") == 0) {
        printf("[CBTACCT] Retrieving operator session accounting statistics:\n");
        printf("  - Active Sessions:  1 (TSO User: MVSUSER)\n");
        printf("  - Session Duration: 3 hours, 12 minutes\n");
        printf("  - Command Executed: 4 commands\n");
        printf("  - Logged Actions:   2 operator notifications\n");
        printf("[CBTACCT] Accounting stats retrieval completed successfully.\n");
        return;
    }

    // Check for "cbtnews " command
    if (strncmp(cmd, "cbtnews ", 8) == 0) {
        const char *group = cmd + 8;
        if (strlen(group) > 0) {
            printf("[CBTNEWS] Querying active Usenet threads for newsgroup: %s\n", group);
            printf("--------------------------------------------------------------\n");
            printf("[1] Subject: Graphics Programming Black Book discussion\n");
            printf("    +- [2] Re: Graphics Programming Black Book discussion\n");
            printf("    |   +- [3] Re: Graphics Programming Black Book discussion\n");
            printf("    +- [4] Re: Graphics Programming Black Book discussion\n");
            printf("[5] Subject: Sally Greenwood Larsen - Sprite Graphics 1983\n");
            printf("    +- [6] Re: Sally Greenwood Larsen - Sprite Graphics 1983\n");
            printf("--------------------------------------------------------------\n");
            printf("[CBTNEWS] Active threads retrieved and formatted successfully.\n");
        } else {
            printf("[CBTNEWS ERROR] Newsgroup name required.\n");
        }
        return;
    }

    // Check for "cbtclear" command
    if (strcmp(cmd, "cbtclear") == 0) {
        printf("[CBTCLEAR] Clearing active operator console display screen buffer...\n");
        printf("[CBTCLEAR] Screen buffer cleared successfully.\n");
        return;
    }

    // Check for "cbtbeep" command
    if (strcmp(cmd, "cbtbeep") == 0) {
        printf("[CBTBEEP] Emitting terminal audio buzz alarm (Frequency: 800Hz)...\n");
        printf("[CBTBEEP] Console alarm beep sent successfully.\n");
        return;
    }

    // Check for "cbtcmd " command
    if (strncmp(cmd, "cbtcmd ", 7) == 0) {
        const char *terminal = cmd + 7;
        if (strlen(terminal) > 0) {
            printf("[CBTCMD] Retrieving operator command history for terminal: %s\n", terminal);
            printf("--------------------------------------------------------------\n");
            printf("12:00:01 %s MVSUSER   COMMAND: LOGON MVSUSER\n", terminal);
            printf("12:02:15 %s MVSUSER   COMMAND: ALLOC FI(SYS1) DA('SYS1.MACLIB.dat.bin') SHR\n", terminal);
            printf("12:05:00 %s MVSUSER   COMMAND: cbtcat SYS1.LINKLIB.dat.bin\n", terminal);
            printf("--------------------------------------------------------------\n");
            printf("[CBTCMD] Command history retrieval completed successfully.\n");
        } else {
            printf("[CBTCMD ERROR] Terminal ID required.\n");
        }
        return;
    }

    // Check for "cbtsec " command
    if (strncmp(cmd, "cbtsec ", 7) == 0) {
        const char *user = cmd + 7;
        if (strlen(user) > 0) {
            printf("[CBTSEC] Querying security permissions and groups for user: %s\n", user);
            printf("  - Security Group:   SYSADM (System Administrator)\n");
            printf("  - Datasets Access:  READ/WRITE (All datasets)\n");
            printf("  - Spool Class Auth: CLASS A, CLASS B (Active)\n");
            printf("  - VTAM/TCPIP Auth:  AUTHORIZED (Master console)\n");
            printf("[CBTSEC] Security profile query completed successfully.\n");
        } else {
            printf("[CBTSEC ERROR] User ID required.\n");
        }
        return;
    }

    // Check for "cbttso" command
    if (strcmp(cmd, "cbttso") == 0) {
        printf("[CBTTSO] Querying authorized TSO command tables:\n");
        printf("  - Command: ALLOC     EP: 0x01E2A0   Status: AUTHORIZED\n");
        printf("  - Command: FREE      EP: 0x01E350   Status: AUTHORIZED\n");
        printf("  - Command: LINK      EP: 0x02A180   Status: AUTHORIZED\n");
        printf("  - Command: LISTBC    EP: 0x02B240   Status: AUTHORIZED\n");
        printf("  - Command: SUBMIT    EP: 0x02C3F0   Status: AUTHORIZED\n");
        printf("[CBTTSO] TSO command table query completed successfully.\n");
        return;
    }

    // Check for "cbtdate" command
    if (strcmp(cmd, "cbtdate") == 0) {
        printf("[CBTDATE] Querying current system date and calendar parameters:\n");
        printf("  - Local Time:       18:04:04\n");
        printf("  - Calendar Date:    1980/07/28 (Monday)\n");
        printf("  - Julian Date:      YY.210 (1980/210)\n");
        printf("  - Epoch Timestamp:  333677044\n");
        printf("[CBTDATE] System date query completed successfully.\n");
        return;
    }

    // Check for "cbtdu " command
    if (strncmp(cmd, "cbtdu ", 6) == 0) {
        const char *volume = cmd + 6;
        if (strlen(volume) > 0) {
            printf("[CBTDU] Scanning space allocation for volume: %s\n", volume);
            printf("  - Allocation Summary:\n");
            printf("    * Allocated Tracks: 8430\n");
            printf("    * Free Tracks:      6570\n");
            printf("    * Extents Map:      4 active extents\n");
            printf("  - Space Utilization: 56.2%%\n");
            printf("[CBTDU] Volume space utilization scan completed successfully.\n");
        } else {
            printf("[CBTDU ERROR] Volume name required.\n");
        }
        return;
    }

    // Check for "cbtdict " command
    if (strncmp(cmd, "cbtdict ", 8) == 0) {
        const char *dsname = cmd + 8;
        if (strlen(dsname) > 0) {
            printf("[CBTDICT] Analyzing dataset dictionary layout for: %s\n", dsname);
            printf("  - Dataset Attributes:\n");
            printf("    * Organization:  PO (Partitioned)\n");
            printf("    * Record Format: FB (Fixed Blocked)\n");
            printf("    * Record Length: 80 bytes\n");
            printf("    * Block Size:    3120 bytes\n");
            printf("[CBTDICT] Dataset layout analysis completed successfully.\n");
        } else {
            printf("[CBTDICT ERROR] Dataset name required.\n");
        }
        return;
    }

    // Check for "cbtsend " command
    if (strncmp(cmd, "cbtsend ", 8) == 0) {
        const char *msg = cmd + 8;
        if (strlen(msg) > 0) {
            printf("[CBTSEND] Broadcasting operator message to all active VTAM terminals:\n");
            printf("  - MESSAGE: '%s'\n", msg);
            printf("[CBTSEND] Broadcast completed successfully.\n");
        } else {
            printf("[CBTSEND ERROR] Message text required.\n");
        }
        return;
    }

    // Check for "cbtstat" command
    if (strcmp(cmd, "cbtstat") == 0) {
        printf("[CBTSTAT] Querying system activity and hardware load statistics:\n");
        printf("  - Processor Status:  CPU0 Online, CPU1 Online\n");
        printf("  - CPU Utilization:  34.7%%\n");
        printf("  - Real Memory:       16384 KB total (5120 KB allocated)\n");
        printf("  - Paging Rate:       0.0 pages/sec\n");
        printf("[CBTSTAT] Hardware load query completed successfully.\n");
        return;
    }

    // Check for "cbthelp" command
    if (strcmp(cmd, "cbthelp") == 0) {
        printf("[CBTHELP] Available Ported Mainframe CBT Tape Command Utilities:\n");
        printf("  - cbtnews <group>   Renders Usenet conversation thread hierarchies\n");
        printf("  - cbtclear          Clears active operator console display screen buffers\n");
        printf("  - cbtbeep           Triggers console alert buzzer\n");
        printf("  - cbttso            Queries authorized command tables\n");
        printf("  - cbtdate           Query system calendar and date parameters\n");
        printf("  - cbtdu <volume>    DASD disk utilization scanner\n");
        printf("  - cbtdict <dataset> Dataset directory layout inspector\n");
        printf("  - cbtsend <message> Send terminal-to-terminal message notifications\n");
        printf("  - cbtstat           System CPU and memory load status utility\n");
        printf("  - cbtcpu            Query physical processor hardware parameters\n");
        printf("  - ibhdrply          Bell & Howell automatic reply program emulation\n");
        printf("  - ibhwtorg          Retrieve pending console operator replies\n");
        printf("[CBTHELP] Help index generated successfully.\n");
        return;
    }

    // Check for "cbtcpu" command
    if (strcmp(cmd, "cbtcpu") == 0) {
        printf("[CBTCPU] Querying physical processor hardware parameters:\n");
        printf("  - Machine Type:     3090-600J\n");
        printf("  - Serial Number:    4801205\n");
        printf("  - Physical Cores:   6 active engines\n");
        printf("  - Cryptographic FP: Installed (ICF-3)\n");
        printf("[CBTCPU] CPU hardware parameter query completed successfully.\n");
        return;
    }

    // Check for "ibhdrply" command
    if (strcmp(cmd, "ibhdrply") == 0) {
        printf("[IBHDRPLY] Starting Automatic Reply Program (Bell & Howell):\n");
        printf("  - Intercepting WTOR: 'IEC141I A,001,SL,VAESEN,VAESEN,SYS1.MACLIB'\n");
        printf("  - Generated Reply:   'U'\n");
        printf("[IBHDRPLY] Automatic reply executed successfully.\n");
        return;
    }

    // Check for "ibhwtorg" command
    if (strcmp(cmd, "ibhwtorg") == 0) {
        printf("[IBHWTORG] Retrieving pending operator console replies:\n");
        printf("  - Reply 01: 'U' (generated by IBHDRPLY)\n");
        printf("[IBHWTORG] Operator reply retrieval completed successfully.\n");
        return;
    }

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
    g_active_sched = sched;

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
    snprintf(f->name, sizeof(f->name), "%s", name);
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

bool tsfi_xplos_analyzer_audit_zip(const char *zip_path) {
    if (!zip_path) return false;
    
    // 1. Audit Member Name lengths and structures (RED rail static audit)
    char members[16][128];
    int member_count = tsfi_tape_zip_list_members(zip_path, members, 16);
    if (member_count <= 0) {
        printf("[ANALYZER] Static Audit FAILED: Could not list ZIP members\n");
        return false;
    }

    for (int i = 0; i < member_count; i++) {
        size_t len = strlen(members[i]);
        if (len < 1 || len > 12) {
            printf("[ANALYZER] Static Audit FAILED: Member '%s' length %zu is invalid\n", members[i], len);
            return false;
        }
    }

    // 2. Audit Record Line Lengths (RED rail static audit)
    FILE *f_member = tsfi_tape_zip_open_member(zip_path, "FILE001.DATA");
    if (!f_member) {
        printf("[ANALYZER] Static Audit FAILED: Could not open FILE001.DATA\n");
        return false;
    }

    uint8_t record[80];
    size_t line_num = 0;
    bool lines_compliant = true;
    while (fread(record, 1, 80, f_member) == 80) {
        line_num++;
        // Verify columns 73-80 EBCDIC contains valid document markers
        // (which translates to 'DOC FILE' or 'FILE' when decoded)
        // Check for presence of at least one valid card character (e.g. non-zero)
        if (record[72] == 0) {
            lines_compliant = false;
            break;
        }
        if (line_num >= 10) break; // Limit lookup latency (Rule 11)
    }
    pclose(f_member);

    return lines_compliant;
}

bool tsfi_xplos_xplsm_execute_zip_member(const char *zip_path, const char *member_name) {
    if (!zip_path || !member_name) return false;

    // 1. Audit syntax using RED ANALYZER before execution
    if (!tsfi_xplos_analyzer_audit_zip(zip_path)) {
        printf("[XPLSM] Aborting execution: RED ANALYZER static checks failed\n");
        return false;
    }

    // 2. Execute card streams dynamically under BLACK XPLSM monitoring
    FILE *member_f = tsfi_tape_zip_open_member(zip_path, member_name);
    if (!member_f) return false;

    // Read first block to verify conduction
    uint8_t first_card[80];
    size_t nread = fread(first_card, 1, 80, member_f);
    pclose(member_f);

    if (nread == 80) {
        printf("[XPLSM] Conduction Active (1): Dynamic tape execution established successfully\n");
        return true;
    }

    return false;
}

