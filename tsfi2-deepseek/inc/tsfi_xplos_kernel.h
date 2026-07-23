#ifndef TSFI_XPLOS_KERNEL_H
#define TSFI_XPLOS_KERNEL_H

#include <stdint.h>
#include <stdbool.h>
#include "tsfi_xpl_mallgren.h"

#define MAX_TASKS 16
#define STACK_SIZE 2048

typedef enum {
    TASK_STATE_READY,
    TASK_STATE_RUNNING,
    TASK_STATE_BLOCKED,
    TASK_STATE_TERMINATED
} TaskState;

typedef void (*TaskEntry)(void *arg);

typedef struct {
    uint32_t task_id;
    TaskState state;
    TaskEntry entry;
    void *arg;
    uint8_t stack[STACK_SIZE];
    void *sp;
    uint32_t wait_event_id;
    uint32_t priority; // Priority: 0 (highest) to 3 (lowest)
} XplosTask;

typedef struct {
    XplosTask tasks[MAX_TASKS];
    int task_count;
    uint32_t active_task_idx;
    uint32_t interrupt_mask;
} XplosScheduler;

// Initialize the cooperative task scheduler
void tsfi_xplos_init_scheduler(XplosScheduler *sched);

// Create a new task in the scheduler
int tsfi_xplos_create_task(XplosScheduler *sched, TaskEntry entry, void *arg);

// Create a new priority task in the scheduler
int tsfi_xplos_create_task_priority(XplosScheduler *sched, TaskEntry entry, void *arg, uint32_t priority);

// Yield CPU execution to the next ready task (Priority-aware QoS)
void tsfi_xplos_yield(XplosScheduler *sched);

// Block the current task waiting for an event ID
void tsfi_xplos_block(XplosScheduler *sched, uint32_t event_id);

// Wake up tasks waiting for a specific event ID (e.g. WinchesterMQ SCSI interrupt)
void tsfi_xplos_trigger_event(XplosScheduler *sched, uint32_t event_id);

// PKI signature validator using FNV-1a credential proof checks
bool tsfi_xplos_validate_pki_signature(
    const uint8_t *pub_key_hash,
    const uint8_t *signature,
    const uint8_t *message,
    size_t msg_len
);

// Run the scheduler tick/loop
void tsfi_xplos_run(XplosScheduler *sched);


// ============================================================================
// GPU JIT COMPILER BRIDGE INTERFACE
// ============================================================================

typedef struct {
    char shader_source[8192];
    int source_len;
    bool compiled;
} XplgGpuJitShader;

bool tsfi_xplg_jit_compile_shader(
    const MallgrenSceneNode *scene_root,
    XplgGpuJitShader *jit_shader
);

// Dispatch compiled JIT compute shader to mock Vulkan GPU pipeline
bool tsfi_xplg_vulkan_dispatch_shader(
    const XplgGpuJitShader *jit_shader,
    uint32_t *vram_fb,
    int width,
    int height
);

// XplOS Shell state structures
typedef struct {
    char cmd_buffer[128];
    int cursor;
    bool active;
} XplosShell;

// Initialize the user shell
void tsfi_xplos_init_shell(XplosShell *shell);

// Processes a command line input and dispatches parsing semantic actions via scheduler tasks
bool tsfi_xplos_shell_exec(XplosShell *shell, XplosScheduler *sched, const char *cmd);


// ============================================================================
// 11. XplOS SYSTEM UTILITIES: VFS, ACMS RECOVERY, INTERRUPT KEYBOARD
// ============================================================================

typedef struct {
    char name[32];
    uint32_t start_offset;
    uint32_t size_bytes;
    bool active;
} XplosFile;

#define MAX_DISK_FILES 8

typedef struct {
    XplosFile files[MAX_DISK_FILES];
    int count;
} XplosVirtualDisk;

typedef struct {
    uint32_t tx_id;
    uint32_t backup_reg_val;
    bool logged;
} XplosAcmsLog;

// Virtual VFS operations
void tsfi_xplos_init_vfs(XplosVirtualDisk *disk);
bool tsfi_xplos_create_file(XplosVirtualDisk *disk, const char *name, uint32_t size);

// VFS Page-Table Swapping Simulator
bool tsfi_xplos_swap_page_to_vfs(
    XplosVirtualDisk *disk,
    uint32_t page_id,
    const uint8_t *page_data,
    size_t page_len
);

// ACMS recovery operations
void tsfi_xplos_log_tx_state(XplosAcmsLog *log, uint32_t tx_id, uint32_t val);
uint32_t tsfi_xplos_abort_recovery(XplosAcmsLog *log);

// SCSI loopback keyboard driver (Rule 5 compliance checking keycodes 32 and 30)
bool tsfi_xplos_scsi_process_keycode(uint8_t keycode, XplosScheduler *sched);


// ============================================================================
// 12. CRYPTOGRAPHIC TRANSACTION LOG VALIDATION & VERIFICATION
// ============================================================================

typedef struct {
    uint32_t tx_id;
    uint32_t pre_state_crc;
    uint32_t post_state_crc;
    uint8_t metadata[12];
    uint8_t hash[32]; // Chained SHA-256 / CRC hash
} XplosLogBlock;

#define MAX_LEDGER_BLOCKS 16

typedef struct {
    XplosLogBlock blocks[MAX_LEDGER_BLOCKS];
    int count;
} XplosVerificationLedger;

// Initialize log ledger
void tsfi_xplos_init_ledger(XplosVerificationLedger *ledger);

// Append a verified transaction block to the ledger
bool tsfi_xplos_add_log_block(
    XplosVerificationLedger *ledger,
    uint32_t tx_id,
    uint32_t pre_state,
    uint32_t post_state,
    const uint8_t *metadata
);

// Sequence check validation algorithm
bool tsfi_xplos_verify_ledger(const XplosVerificationLedger *ledger);

// Cooperative task entries for skeletonization and syntactic analysis
void tsfi_xplos_task_skeletonize(void *arg);
void tsfi_xplos_task_parse_syntax(void *arg);


// ============================================================================
// 13. SYNCH.LIVE: COOPERATIVE FLOCKING ENGINE & CONNECTIVITY ANALYZER
// ============================================================================

typedef struct {
    double x;
    double y;
    double vx;
    double vy;
} XplosBoid;

#define MAX_BOIDS 16

typedef struct {
    XplosBoid agents[MAX_BOIDS];
    int count;
    double connectivity_score; // Real-time C_sync connectedness coefficient
} XplosFlock;

// Initialize flocking simulation
void tsfi_xplos_init_flock(XplosFlock *flock);

// Perform one step of flocking (separation, alignment, cohesion)
void tsfi_xplos_update_flock(XplosFlock *flock);

// Compute real-time synchronization connectedness coefficient
double tsfi_xplos_analyze_connectivity(const XplosFlock *flock);

// Render flock agents to VRAM framebuffer
void tsfi_xplos_render_flock(
    const XplosFlock *flock,
    uint8_t *fb,
    int width,
    int height
);


// ============================================================================
// 14. SYNCH.LIVE EXTENSIONS: OBSTACLES, PATH SEEKING, VELOCITY FLOWS
// ============================================================================

typedef struct {
    XplosFlock base_flock;
    double target_x;
    double target_y;
    uint8_t obstacle_grid[256]; // 16x16 maze grid
} XplosObstacleFlock;

// Initialize obstacle flock and potential field targets
void tsfi_xplos_init_obstacle_flock(XplosObstacleFlock *of);

// Update simulation including potential field target force, obstacle redirection, and flow speedups
void tsfi_xplos_update_obstacle_flock(XplosObstacleFlock *of);


// ============================================================================
// 15. BOUNDED CONFIDENCE PROBABILISTIC SOCIAL LEARNING WITH MEMORY
// ============================================================================

#define HISTORY_LEN 4
#define MAX_AGENTS 8

typedef struct {
    double opinion;
    double confidence_bound; // epsilon
    double history[HISTORY_LEN];
    int history_idx;
} XplosSocialAgent;

typedef struct {
    XplosSocialAgent agents[MAX_AGENTS];
    double trust_matrix[MAX_AGENTS][MAX_AGENTS];
    double credibility_ratings[MAX_AGENTS][MAX_AGENTS]; // Credibility Memory Ledger
} XplosSocialNetwork;

// Initialize social learning simulation network
void tsfi_xplos_init_social_network(XplosSocialNetwork *network);

// Perform dynamic bounded confidence updates with memory and probabilistic changes
void tsfi_xplos_update_social_opinions(XplosSocialNetwork *network);

// Perform EWMD-decay opinion updates
void tsfi_xplos_update_ewmd_opinions(XplosSocialNetwork *network, double alpha);


// ============================================================================
// 16. SYNCH.LIVE: SPATIAL-COGNITIVE COUPLED FLOCKING SYNCHRONIZER
// ============================================================================

typedef struct {
    double x;
    double y;
    double vx;
    double vy;
    double opinion; // Cognitive opinion mapping [0.0, 1.0]
    double confidence_bound; // epsilon
} XplosCognitiveBoid;

#define MAX_COG_BOIDS 12

typedef struct {
    XplosCognitiveBoid agents[MAX_COG_BOIDS];
    int count;
    double spatial_cognitive_sync; // Coupled coordination index
} XplosCognitiveFlock;

// Initialize spatial-cognitive coupled flock
void tsfi_xplos_init_cognitive_flock(XplosCognitiveFlock *cf);

// Update spatial positioning and cognitive opinions based on mutual alignment and proximity learning
void tsfi_xplos_update_cognitive_flock(XplosCognitiveFlock *cf);

#endif // TSFI_XPLOS_KERNEL_H
