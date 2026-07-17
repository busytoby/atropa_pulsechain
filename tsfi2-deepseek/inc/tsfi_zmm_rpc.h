#ifndef TSFI_ZMM_RPC_H
#define TSFI_ZMM_RPC_H

#include "tsfi_zmm_vm.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include <pulse/simple.h>

#define MAX_DILEMMA_LOGS 128
typedef struct {
    char event[64];
    char source[64];
    char details[256];
    uint64_t timestamp;
} DilemmaLogEntry;

#define MAX_STORE_KEYS 1024
typedef struct {
    char key[128];
    char value[4096];
    uint64_t timestamp;
} StoreEntry;

typedef struct {
    bool lead_mounted;
    bool bass_mounted;
    bool growl_mounted;
    bool drums_mounted;
    bool audio_stream_control_mounted;
} RpcSynthMounts;

#define MAX_ARRANGEMENT 64
#define MAX_STEPS 32

typedef struct {
    char pattern_name[16];
    char lead_notes[MAX_STEPS][8];
    char bass_notes[MAX_STEPS][8];
    char growl_notes[MAX_STEPS][8];
    double growl_gain[MAX_STEPS];
    double growl_mod[MAX_STEPS];
    uint8_t drum_kick[MAX_STEPS];
    uint8_t drum_snare[MAX_STEPS];
    int lead_count;
    int bass_count;
    int growl_count;
    int gain_count;
    int mod_count;
    int kick_count;
    int snare_count;
} RpcPatternData;

typedef enum {
    FAULT_NONE = 0,
    FAULT_OPEN,   // Unmapped memory region
    FAULT_SHORT,  // Memory overlapping/cross-talk
    FAULT_GROUND  // Completely zeroed/grounded register state
} FortellMemoryFault;

#define MAX_REGISTERED_OPERATORS 8
typedef struct {
    int project;
    int programmer;
    int key_id;
    int acl_level;
    bool is_active;
} ZmmOperatorEntry;

// Shared Globals
extern DilemmaLogEntry g_dilemma_logs[MAX_DILEMMA_LOGS];
extern int g_dilemma_log_count;
extern StoreEntry g_seq_store[MAX_STORE_KEYS];
extern int g_seq_store_count;
extern pthread_mutex_t g_seq_store_mutex;
extern bool g_seq_store_loaded;
extern RpcSynthMounts g_rpc_mounts;
extern pa_simple *g_rpc_pulse_stream;
extern int g_consecutive_verification_failures;
extern bool g_is_session_locked_out;
extern int g_telemetry_23_log_count;
extern ZmmOperatorEntry g_operator_registry[MAX_REGISTERED_OPERATORS];
extern int g_operator_count;

// Shared Helper Functions
int extract_json_string(const char *json, const char *key, char *out, size_t max);
int extract_json_int(const char *json, const char *key, int default_val);
size_t decode_hex(const char *hex, uint8_t *out, size_t max);
double rpc_note_to_frequency(const char *note);
void rpc_play_polyphonic_step(double f_lead, double f_bass, double f_growl, double growl_gain, double growl_mod, bool has_kick, bool has_snare, double duration);
int verify_23_tree_traversal_acl(int project, int programmer, int key_id);
int register_zmm_operator_via_23_tree(int project, int programmer, int key_id);
bool rpc_play_bio_arrangement(const char *file_path, const char **out_err);
void save_seq_store(void);
void load_seq_store(void);

// Dispatch a JSON-RPC command
int tsfi_zmm_rpc_dispatch(TsfiZmmVmState *state, const char *json_in, char *output_buf, size_t out_max);

#endif // TSFI_ZMM_RPC_H
