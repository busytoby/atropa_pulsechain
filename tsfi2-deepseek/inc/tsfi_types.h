#ifndef TSFI_TYPES_H
#define TSFI_TYPES_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>
#include <stdatomic.h>

// ---------------------------------------------------------
struct TSFiBigInt;
// Meta-Thunk Dynamic Schema Definitions
// ---------------------------------------------------------
typedef enum {
    THUNK_BAKED,
    THUNK_FORWARDING,
    THUNK_ZMM,
    THUNK_IOCTL
} ThunkInjectionType;

typedef struct {
    size_t offset;
    ThunkInjectionType type;
    int arity;
    void* target_fn;
} ThunkSignature;

#define TSFI_VERSION "2.0.0"

#define LAU_TELEMETRY_MAGIC 0xCAFEBABE12345678
#define LAU_TELEM_FLAG_SEALED (1u << 0)
#define LAU_TELEM_RING_SIZE 1024

// --- Registry Database Definitions ---
#define LAU_REGISTRY_CAPACITY 65536

typedef struct {
    uint64_t timestamp;
    uint64_t ptr_addr;
    uint64_t size;
    uint32_t type;
    uint32_t flags;
} LauTelemetryEvent;

#define LAU_SEAL_NONE 0xFF

// Mapped Common Fields for Thunkable Objects
#define MAPPED_COMMON_FIELDS \
    int * const version; \
    char ** const resonance_as_status; \
    int * const counter; \
    void (*step_safety_epoch)(void); \
    void (*step_safety_state)(void); \
    void (*step_executor_directive)(char*); \
    void (*scramble)(void); \
    void (*provenance)(void); \
    void (*hilbert_eval)(float, float, float*); \
    void (*hilbert_batch)(void*, const float*, float*, int);

typedef struct LauMetadata {
    struct LauMetadata *next;
    struct LauMetadata *prev;
    const char *alloc_file;
    size_t alloc_size;
    void *actual_start;
    void *payload_start;
    uint32_t instruction_hash;
    uint32_t probe_latency;
    uint8_t current_prot;
    uint8_t physical_tier;
    uint8_t seal_level;
    uint8_t _pad[5]; // Pad 59 to 64
} LauMetadata;

typedef struct {
    uint64_t search_index[LAU_REGISTRY_CAPACITY] __attribute__((aligned(64)));
    LauMetadata slots[LAU_REGISTRY_CAPACITY];
    _Atomic uint32_t count;
    uint8_t _alignment_guard[508];
} LauRegistryManifold;

// --- TSFi System Primitives ---

typedef struct {
    uint32_t magic;
    _Atomic uint32_t sequence_number;
    uint32_t pid;
    _Atomic uint32_t header_crc;
    _Atomic uint32_t system_integrity_fault;
    _Atomic float current_intensity;
    
    _Atomic uint64_t total_allocs;
    _Atomic uint64_t total_frees;
    _Atomic uint64_t active_allocs;
    _Atomic uint64_t active_bytes;
    _Atomic uint64_t peak_bytes;
    
    _Atomic uint64_t exec_steps;
    _Atomic uint64_t exec_last_ts;
    _Atomic uint64_t throttle_count;
    _Atomic uint64_t total_stall_ns;
    
    // --- Little Petya Spider Coverage (Grid 8x5 = 40 cells) ---
    _Atomic uint64_t spider_grid;   // 40 bits used for occupancy
    _Atomic uint64_t covered_grid;  // 40 bits used for coverage
    _Atomic float    coverage_pct;
    _Atomic bool     xor_trap_active;
    
    char last_directive_str[64];
    _Atomic float recip_symmetry;
    char recip_dai[128];
    char zmm_msg[128];
    uint64_t zmm_val;
    char request_cmd[4096];
    _Atomic uint32_t request_id;
    struct { 
        uint64_t last_shutter_ts; 
        float shutter_fps; 
        float velocity_flux; 
        float jerk_metric; 
        float surface_quality; 
        uint64_t surface_hash; 
    } mouse_scope;
    
    uint32_t event_head;
    LauTelemetryEvent events[LAU_TELEM_RING_SIZE];

    struct {
        uint64_t frames_rendered;
        double avg_ms_per_frame;
        float current_intensity;
        uint32_t active_fibers;
        uint32_t svdag_nodes;
        float blue_secret_flux;
        uint32_t current_epoch;
        float blue_secret_jitter;
        float magnetic_yi_rigidity;
        float petal_irid_shift;
        uint32_t current_stage;
        float path_trace_m_evals;
    } render_telemetry;
} LauTelemetryState;
typedef struct MappedCommon { MAPPED_COMMON_FIELDS } MappedCommon;

#define DEFINE_MAPPED_STRUCT(name, ...) \
    typedef struct { \
        struct { \
            MAPPED_COMMON_FIELDS \
        }; \
        __VA_ARGS__ \
    } name; \
    extern const ThunkSignature name##_schema[]; \
    extern const int name##_schema_count;

DEFINE_MAPPED_STRUCT(WaveSystem,
    int system_id;
    char *current_directive;
    double current_intensity;
    char *provenance_sig;
    void *fw;
    void *active_sessions[4];
    void *active_hilbert_glyph;
    char shared_pty_buffer[4096];
)

DEFINE_MAPPED_STRUCT(TeddyBear, 
    struct MappedCommon *Psi;
    struct TSFiBigInt *Xi;
    struct TSFiBigInt *Ring;
    float norm_density; 
    float fur_roughness; 
    float eye_intensity; 
    float spectral_shift; 
    void *voxel_data; 
    size_t voxel_count; 
    void *heart_pulse_manifold; 
    void *limb_thunks[4]; 
)

DEFINE_MAPPED_STRUCT(WavefrontContext,
    float *input_x;
    float *input_y;
    float *input_z;
    float *output_data;
    int input_x_vgpr_offset;
    int input_y_vgpr_offset;
    int input_z_vgpr_offset;
    int output_vgpr_offset;
    int input_x_vgpr_secret;
    int input_y_vgpr_secret;
    int input_z_vgpr_secret;
    int output_vgpr_secret;
    void (*load_x_to_vgpr)(void);
    void (*load_y_to_vgpr)(void);
    void (*load_z_to_vgpr)(void);
    void (*store_output_from_vgpr)(void);
    void (*execute_vgpr_kernel)(void);
)

typedef struct {
    int id;
    int state;
    void (*fire_action)(void *ctx);
    void (*cycle_state)(void *ctx);
} HypervisorTestNode;

#endif
