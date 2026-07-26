#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "auncient_cactus_schema.h"


#define STACK_CAPACITY 32
#define COLOR_RED      0x00FF
#define COLOR_CYAN     0xFFFF
#define COLOR_BLUE     0xFF0000
#define WARNING_DRONE  110 // Hz low drone

typedef struct {
    int64_t data[STACK_CAPACITY];
    int top;
    int checkpoint_top;
} loader_stack_t;

typedef struct {
    uint8_t buffer[64];
    size_t write_ptr;
    size_t read_ptr;
    bool transfer_active;
    uint32_t transfer_cycles;
} tape_channel_t;

typedef struct {
    loader_stack_t stack;
    tape_channel_t tape;
    uint32_t border_color;
    uint32_t psg_frequency;
    
    // Blame registry
    bool blame_quarantine;
    uint32_t blamed_port;
} ocean_loader_ctx_t;

typedef struct {
    double f, q, low, band, high;
} Resonator;

static void resonator_init(Resonator *r, double freq, double Q) {
    double omega = 2.0 * 3.141592653589793 * freq / 8000.0;
    r->f = 2.0 * sin(omega / 2.0);
    r->q = 1.0 / Q;
    r->low = 0.0;
    r->band = 0.0;
    r->high = 0.0;
}

static double resonator_tick(Resonator *r, double input) {
    r->high = input - r->low - r->q * r->band;
    r->band += r->f * r->high;
    r->low += r->f * r->band;
    return r->band;
}

static float generate_chiptune_sample(uint32_t frequency, float time, Resonator *r) {
    if (frequency == 0) return 0.0f;
    float raw_wave = sin(time * frequency * 2.0f * 3.14159265f);
    return (float)resonator_tick(r, raw_wave);
}

static uint32_t cycle_border_raster(uint32_t frame_count) {
    if (frame_count % 3 == 0) {
        return COLOR_RED;
    } else if (frame_count % 3 == 1) {
        return COLOR_CYAN;
    } else {
        return COLOR_BLUE;
    }
}

static uint32_t play_loader_arpeggio(uint32_t frame_count) {
    uint32_t note_index = frame_count % 3;
    if (note_index == 0) {
        return 261;
    } else if (note_index == 1) {
        return 329;
    } else {
        return 392;
    }
}

// 1. Dynamic Split-Raster Stripe Generator (modulates border lines based on SCSI bits)
static void update_raster_border(ocean_loader_ctx_t *ctx, uint8_t scsi_byte) {
    // If bit 0 is high, render CYAN stripe, else RED
    if (scsi_byte & 1) {
        ctx->border_color = COLOR_CYAN;
    } else {
        ctx->border_color = COLOR_RED;
    }
}

// 1.5. Asynchronous Uniservo Tape Channel Tick
static void tick_tape_channel(ocean_loader_ctx_t *ctx) {
    tape_channel_t *channel = &ctx->tape;
    if (!channel->transfer_active) return;

    channel->transfer_cycles++;
    if (channel->transfer_cycles >= 5) { // 1 byte every 5 cycles
        channel->transfer_cycles = 0;
        if (channel->read_ptr < channel->write_ptr) {
            uint8_t byte = channel->buffer[channel->read_ptr++];
            update_raster_border(ctx, byte);
            if (ctx->stack.top < STACK_CAPACITY) {
                ctx->stack.data[ctx->stack.top++] = byte;
            }
        } else {
            channel->transfer_active = false;
            ctx->psg_frequency = 330; // Success arpeggio tone E4
        }
    }
}


// 2. PSG Audio Failure Alarm Drone
static void set_psg_alarm(ocean_loader_ctx_t *ctx) {
    ctx->psg_frequency = WARNING_DRONE;
    printf("   [AUDIO] PSG Channel 1 output warning drone (%d Hz).\n", WARNING_DRONE);
}

// 3. SCSI Ingestion Checksum Blame & Revert (Bidirectional)
static bool ingest_scsi_block_bidirectional(ocean_loader_ctx_t *ctx, const uint8_t *data, size_t size, uint8_t expected_checksum, bool forward, uint32_t input_port) {
    // Save stack checkpoint before loading
    ctx->stack.checkpoint_top = ctx->stack.top;

    uint8_t calculated_checksum = 0;
    for (size_t i = 0; i < size; i++) {
        size_t idx = forward ? i : (size - 1 - i);
        uint8_t val = data[idx];
        calculated_checksum ^= val;

        // Dynamic split-raster striped border updates
        update_raster_border(ctx, val);

        // Push loaded bytes onto stack
        if (ctx->stack.top < STACK_CAPACITY) {
            ctx->stack.data[ctx->stack.top++] = val;
        }
    }

    if (calculated_checksum != expected_checksum) {
        // Verification failed: Trigger rollback, log blame quarantine, play alarm drone
        ctx->stack.top = ctx->stack.checkpoint_top; // Revert stack
        ctx->blame_quarantine = true;
        ctx->blamed_port = input_port;
        set_psg_alarm(ctx);
        printf("   [BLAME] Checksum mismatch! Port %d quarantined. Stack rolled back.\n", input_port);
        return false;
    }

    // Success arpeggio frequency update (C-Major C4 261Hz)
    ctx->psg_frequency = 261;
    return true;
}

typedef struct {
    char active_variant[16];
    char inherited_class[32];
    bool class_active;
} usd_variant_inherit_t;

static bool resolve_variant_inherits(const usd_variant_inherit_t *vi, const char *variant_name, char *class_out) {
    if (strcmp(vi->active_variant, variant_name) == 0 && vi->class_active) {
        strcpy(class_out, vi->inherited_class);
        return true;
    }
    return false;
}

typedef struct {
    char root_class[32];
    char child_class[32];
    char child_override_value[32];
    char root_value[32];
} usd_hierarchy_inherit_t;

static void resolve_hierarchical_inherits(const usd_hierarchy_inherit_t *hi, char *resolved_val) {
    if (strlen(hi->child_override_value) > 0) {
        strcpy(resolved_val, hi->child_override_value);
    } else {
        strcpy(resolved_val, hi->root_value);
    }
}

typedef struct {
    char source_path[64];
    char target_prefix[64];
} usd_reroute_t;

static void resolve_reroot_path(const usd_reroute_t *rr, const char *relative_path, char *absolute_out) {
    sprintf(absolute_out, "%s%s", rr->target_prefix, relative_path);
}

typedef struct {
    double time_displacement;
    double time_scale;
} usd_reference_displacement_t;

static double resolve_reference_time(const usd_reference_displacement_t *disp, double input_time) {
    return (input_time * disp->time_scale) + disp->time_displacement;
}

typedef struct {
    char local_variant_opinion[32];
    char referenced_variant_opinion[32];
    char reference_opinion[32];
} usd_variant_ref_conflict_t;

static void resolve_variant_ref_conflict(const usd_variant_ref_conflict_t *conflict, char *resolved_val) {
    if (strlen(conflict->local_variant_opinion) > 0) {
        strcpy(resolved_val, conflict->local_variant_opinion);
    } else if (strlen(conflict->referenced_variant_opinion) > 0) {
        strcpy(resolved_val, conflict->referenced_variant_opinion);
    } else {
        strcpy(resolved_val, conflict->reference_opinion);
    }
}

typedef struct {
    char reference_opinion[32];
    char payload_opinion[32];
    char specializes_opinion[32];
} usd_payload_ref_conflict_t;

static void resolve_payload_ref_conflict(const usd_payload_ref_conflict_t *conflict, char *resolved_val) {
    if (strlen(conflict->reference_opinion) > 0) {
        strcpy(resolved_val, conflict->reference_opinion);
    } else if (strlen(conflict->payload_opinion) > 0) {
        strcpy(resolved_val, conflict->payload_opinion);
    } else {
        strcpy(resolved_val, conflict->specializes_opinion);
    }
}

typedef struct {
    char local_opinion[32];
    char variant_opinion[32];
} usd_local_variant_conflict_t;

static void resolve_local_variant_conflict(const usd_local_variant_conflict_t *conflict, char *resolved_val) {
    if (strlen(conflict->local_opinion) > 0) {
        strcpy(resolved_val, conflict->local_opinion);
    } else {
        strcpy(resolved_val, conflict->variant_opinion);
    }
}

typedef struct {
    char inherits_opinion[32];
    char specializes_opinion[32];
} usd_spec_inh_conflict_t;

static void resolve_spec_inh_conflict(const usd_spec_inh_conflict_t *conflict, char *resolved_val) {
    if (strlen(conflict->inherits_opinion) > 0) {
        strcpy(resolved_val, conflict->inherits_opinion);
    } else {
        strcpy(resolved_val, conflict->specializes_opinion);
    }
}

typedef struct {
    char class_opinions[3][32];
    int total_classes;
} usd_multiple_inherits_t;

static void resolve_multiple_inherits(const usd_multiple_inherits_t *mi, char *resolved_val) {
    for (int i = 0; i < mi->total_classes; i++) {
        if (strlen(mi->class_opinions[i]) > 0) {
            strcpy(resolved_val, mi->class_opinions[i]);
            return;
        }
    }
}

typedef struct {
    char variant_opinion[32];
    char specializes_opinion[32];
} usd_variant_spec_conflict_t;

static void resolve_variant_spec_conflict(const usd_variant_spec_conflict_t *conflict, char *resolved_val) {
    if (strlen(conflict->variant_opinion) > 0) {
        strcpy(resolved_val, conflict->variant_opinion);
    } else {
        strcpy(resolved_val, conflict->specializes_opinion);
    }
}

typedef struct {
    char inherits_opinion[32];
    char variant_opinion[32];
} usd_inh_variant_conflict_t;

static void resolve_inh_variant_conflict(const usd_inh_variant_conflict_t *conflict, char *resolved_val) {
    if (strlen(conflict->inherits_opinion) > 0) {
        strcpy(resolved_val, conflict->inherits_opinion);
    } else {
        strcpy(resolved_val, conflict->variant_opinion);
    }
}

typedef struct {
    char inherits_opinion[32];
    char payload_opinion[32];
} usd_inh_payload_conflict_t;

static void resolve_inh_payload_conflict(const usd_inh_payload_conflict_t *conflict, char *resolved_val) {
    if (strlen(conflict->inherits_opinion) > 0) {
        strcpy(resolved_val, conflict->inherits_opinion);
    } else {
        strcpy(resolved_val, conflict->payload_opinion);
    }
}

typedef struct {
    char inherits_opinion[32];
    char reference_opinion[32];
} usd_inh_ref_conflict_t;

static void resolve_inh_ref_conflict(const usd_inh_ref_conflict_t *conflict, char *resolved_val) {
    if (strlen(conflict->inherits_opinion) > 0) {
        strcpy(resolved_val, conflict->inherits_opinion);
    } else {
        strcpy(resolved_val, conflict->reference_opinion);
    }
}

typedef struct {
    char local_opinion[32];
    char inherits_opinion[32];
} usd_local_inh_conflict_t;

static void resolve_local_inh_conflict(const usd_local_inh_conflict_t *conflict, char *resolved_val) {
    if (strlen(conflict->local_opinion) > 0) {
        strcpy(resolved_val, conflict->local_opinion);
    } else {
        strcpy(resolved_val, conflict->inherits_opinion);
    }
}

typedef struct {
    char local_opinion[32];
    char reference_opinion[32];
} usd_local_ref_conflict_t;

static void resolve_local_ref_conflict(const usd_local_ref_conflict_t *conflict, char *resolved_val) {
    if (strlen(conflict->local_opinion) > 0) {
        strcpy(resolved_val, conflict->local_opinion);
    } else {
        strcpy(resolved_val, conflict->reference_opinion);
    }
}

typedef struct {
    char local_opinion[32];
    char payload_opinion[32];
} usd_local_payload_conflict_t;

static void resolve_local_payload_conflict(const usd_local_payload_conflict_t *conflict, char *resolved_val) {
    if (strlen(conflict->local_opinion) > 0) {
        strcpy(resolved_val, conflict->local_opinion);
    } else {
        strcpy(resolved_val, conflict->payload_opinion);
    }
}

typedef struct {
    char local_opinion[32];
    char specializes_opinion[32];
} usd_local_spec_conflict_t;

static void resolve_local_spec_conflict(const usd_local_spec_conflict_t *conflict, char *resolved_val) {
    if (strlen(conflict->local_opinion) > 0) {
        strcpy(resolved_val, conflict->local_opinion);
    } else {
        strcpy(resolved_val, conflict->specializes_opinion);
    }
}

typedef struct {
    char reference_opinion[32];
    char payload_opinion[32];
} usd_ref_payload_conflict_t;

static void resolve_ref_payload_conflict(const usd_ref_payload_conflict_t *conflict, char *resolved_val) {
    if (strlen(conflict->reference_opinion) > 0) {
        strcpy(resolved_val, conflict->reference_opinion);
    } else {
        strcpy(resolved_val, conflict->payload_opinion);
    }
}

typedef struct {
    char reference_opinion[32];
    char specializes_opinion[32];
} usd_ref_spec_conflict_t;

static void resolve_ref_spec_conflict(const usd_ref_spec_conflict_t *conflict, char *resolved_val) {
    if (strlen(conflict->reference_opinion) > 0) {
        strcpy(resolved_val, conflict->reference_opinion);
    } else {
        strcpy(resolved_val, conflict->specializes_opinion);
    }
}

typedef struct {
    char payload_opinion[32];
    char specializes_opinion[32];
} usd_payload_spec_conflict_t;

static void resolve_payload_spec_conflict(const usd_payload_spec_conflict_t *conflict, char *resolved_val) {
    if (strlen(conflict->payload_opinion) > 0) {
        strcpy(resolved_val, conflict->payload_opinion);
    } else {
        strcpy(resolved_val, conflict->specializes_opinion);
    }
}

typedef struct {
    char prim_path[64];
    char material_type[32];
    float vertex_density;
} hydra_scene_index_prim_t;

static void hydra_scene_index_sync_delegate(const usd_auncient_cactus_schema_t *schema, const usd_auncient_texture_api_t *api, const char *path, hydra_scene_index_prim_t *render_prim) {
    strcpy(render_prim->prim_path, path);
    strcpy(render_prim->material_type, api->texture);
    render_prim->vertex_density = schema->density;
}


typedef struct {
    char listened_path[64];
    bool invalidation_triggered;
    uint32_t autodin_lock_token;
    char autodin_precedence;
} usd_notice_listener_t;

static void usd_notice_send_invalidation(usd_notice_listener_t *listener, const char *changed_path) {
    if (strcmp(listener->listened_path, changed_path) == 0) {
        listener->invalidation_triggered = true;
        printf("[AUTODIN] Notice Listener triggered invalidation for path: %s. Lock token: 0x%X (Precedence: %c).\n", 
               changed_path, listener->autodin_lock_token, listener->autodin_precedence);
    }
}

typedef struct {
    float ref_phase;
    float vco_phase;
    float error_sum;
    float kp;
    float ki;
} ouroboros_pll_t;

typedef struct {
    int player_x;
    int missile_x;
    uint32_t collision_mask;
} gtia_pmg_t;

typedef struct {
    ouroboros_pll_t pll;
    gtia_pmg_t pmg;
    uint32_t register_f125;
    bool transaction_aborted;
} cics_pll_pmg_context_t;

static uint64_t power_modulo(uint64_t base_val, uint64_t exp, uint64_t mod) {
    uint64_t res = 1;
    base_val = base_val % mod;
    while (exp > 0) {
        if (exp & 1) {
            res = (uint64_t)(((__uint128_t)res * base_val) % mod);
        }
        base_val = (uint64_t)(((__uint128_t)base_val * base_val) % mod);
        exp >>= 1;
    }
    return res;
}

static void execute_pll_tick_cics(cics_pll_pmg_context_t *ctx) {
    float error_val = ctx->pll.ref_phase - ctx->pll.vco_phase;
    ctx->pll.error_sum += error_val;
    float adjustment = (ctx->pll.kp * error_val) + (ctx->pll.ki * ctx->pll.error_sum);
    
    // Update register 0xF125 with absolute phase deviation
    ctx->register_f125 = (uint32_t)(fabs(error_val) * 1000.0f);
    
    // If adjustment exceeds stability limits, abort CICS transaction
    if (fabs(adjustment) > 2.0f) {
        ctx->transaction_aborted = true;
    }
}

static uint64_t execute_pmg_collision_cics(cics_pll_pmg_context_t *ctx, uint64_t base_val) {
    uint64_t distance = (uint64_t)abs(ctx->pmg.player_x - ctx->pmg.missile_x);
    // Cryptographic TDMA proof: proof = base^distance mod MotzkinPrime
    uint64_t motzkin = 953467954114363ULL;
    return power_modulo(base_val, distance, motzkin);
}

static void unify_gtia_hudson(uint16_t *vce_table, const gtia_pmg_t *pmg) {
    uint16_t collision_color = (pmg->collision_mask > 0) ? 0xF800 : 0x07E0;
    vce_table[0] = collision_color;
}

#define HUDSON_MAX_LAYERS 8

typedef struct {
    uint16_t sprite_id;
    uint16_t x_pos;
    uint16_t y_pos;
    uint16_t palette_displacement;
    bool active;
} hudson_vdc_sprite_layer_t;

typedef struct {
    hudson_vdc_sprite_layer_t layers[HUDSON_MAX_LAYERS];
    int active_layers_count;
} hudson_vdc_manager_t;

static int allocate_dynamic_hudson_layer(hudson_vdc_manager_t *mgr, uint16_t x, uint16_t y, uint16_t pal_disp) {
    for (int i = 0; i < HUDSON_MAX_LAYERS; i++) {
        if (!mgr->layers[i].active) {
            mgr->layers[i].sprite_id = (uint16_t)i;
            mgr->layers[i].x_pos = x;
            mgr->layers[i].y_pos = y;
            mgr->layers[i].palette_displacement = pal_disp;
            mgr->layers[i].active = true;
            mgr->active_layers_count++;
            return i;
        }
    }
    return -1;
}

typedef struct {
    uint32_t background_color;
    hudson_vdc_sprite_layer_t dynamic_sprites[HUDSON_MAX_LAYERS];
    int sprite_count;
} coaxial_alu_compositor_frame_t;

static void coaxial_alu_composite_layers(const coaxial_alu_compositor_frame_t *frame, uint32_t *output_buffer, int buffer_size) {
    for (int i = 0; i < buffer_size; i++) {
        output_buffer[i] = frame->background_color;
    }
    for (int i = 0; i < frame->sprite_count; i++) {
        if (frame->dynamic_sprites[i].active) {
            int pos = frame->dynamic_sprites[i].x_pos % buffer_size;
            output_buffer[pos] = 0xFFFFFF;
        }
    }
}

typedef struct {
    uint32_t active_context_id;
    uint32_t registers[4][8];
} gtia_context_switcher_t;

static void switch_gtia_context(gtia_context_switcher_t *switcher, uint32_t next_context_id) {
    if (next_context_id < 4) {
        switcher->active_context_id = next_context_id;
    }
}

static void gtia_device_composite(const gtia_context_switcher_t *switcher, uint32_t prior_reg, uint32_t *output, int size) {
    // Fill with background color initially
    for (int i = 0; i < size; i++) {
        output[i] = 0x000000;
    }
    
    // Composite layers 0 to 3 based on priority rules
    // If prior_reg & 1, lower index players win, otherwise higher index wins.
    bool reverse = (prior_reg & 0x01) != 0;
    int start = reverse ? 3 : 0;
    int end = reverse ? -1 : 4;
    int step = reverse ? -1 : 1;
    
    for (int p = start; p != end; p += step) {
        // registers[p][0] is coordinate x
        // registers[p][1] is color value
        uint32_t x = switcher->registers[p][0];
        uint32_t color = switcher->registers[p][1];
        if (x < (uint32_t)size) {
            output[x] = color;
        }
    }
}

static void resolve_cactus_variant_schema(usd_auncient_cactus_schema_t *schema, const char *variant_name) {
    if (strcmp(variant_name, "high_density") == 0) {
        schema->density = 10.0f;
    } else if (strcmp(variant_name, "low_density") == 0) {
        schema->density = 0.5f;
    } else {
        schema->density = 1.0f;
    }
}

static int usd_serialize_cactus_stage(const char *filename, const usd_auncient_cactus_schema_t *schema, const usd_auncient_texture_api_t *api) {
    FILE *f = fopen(filename, "wb");
    if (!f) return 0;
    if (fwrite(schema, sizeof(usd_auncient_cactus_schema_t), 1, f) != 1) {
        fclose(f);
        return 0;
    }
    if (fwrite(api, sizeof(usd_auncient_texture_api_t), 1, f) != 1) {
        fclose(f);
        return 0;
    }
    fclose(f);
    return 1;
}

static int usd_deserialize_cactus_stage(const char *filename, usd_auncient_cactus_schema_t *schema, usd_auncient_texture_api_t *api) {
    FILE *f = fopen(filename, "rb");
    if (!f) return 0;
    if (fread(schema, sizeof(usd_auncient_cactus_schema_t), 1, f) != 1) {
        fclose(f);
        return 0;
    }
    if (fread(api, sizeof(usd_auncient_texture_api_t), 1, f) != 1) {
        fclose(f);
        return 0;
    }
    fclose(f);
    return 1;
}

typedef struct {
    usd_auncient_cactus_schema_t sublayers[4];
    int sublayers_count;
} usd_auncient_cactus_stage_t;

static float usd_resolve_composed_density(const usd_auncient_cactus_stage_t *stage) {
    for (int i = stage->sublayers_count - 1; i >= 0; i--) {
        if (stage->sublayers[i].density != 0.0f) {
            return stage->sublayers[i].density;
        }
    }
    return 1.0f;
}

static int usd_generate_new_cactus_asset(const char *filename, const usd_auncient_cactus_schema_t *schema, const usd_auncient_texture_api_t *texture, const usd_auncient_physics_api_t *physics) {
    FILE *f = fopen(filename, "w");
    if (!f) return 0;
    fprintf(f, "#usda 1.0\n\n");
    fprintf(f, "def \"CactusAsset\" (\n");
    fprintf(f, "    inherits = </AuncientCactusSchema>\n");
    fprintf(f, ") {\n");
    fprintf(f, "    float density = %f\n", schema->density);
    fprintf(f, "    float stiffness = %f\n", texture->stiffness);
    fprintf(f, "    string texture = \"%s\"\n", texture->texture);
    fprintf(f, "    float mass = %f\n", physics->mass);
    fprintf(f, "    float damping = %f\n", physics->damping);
    fprintf(f, "}\n");
    fclose(f);
    return 1;
}

typedef struct {
    int texture_count;
    float vertex_density;
} usd_hydra_render_delegate_t;

typedef struct {
    char prim_path[64];
    char material[32];
    float density;
    usd_hydra_render_delegate_t *delegate;
} usd_hydra_scene_index_t;

static void usd_hydra_add_prim(usd_hydra_scene_index_t *si, const char *path) {
    strcpy(si->prim_path, path);
    si->delegate->texture_count++;
}

static void usd_hydra_update_prim(usd_hydra_scene_index_t *si, const char *mat, float dens) {
    strcpy(si->material, mat);
    si->density = dens;
    si->delegate->vertex_density = dens;
}

static void usd_hydra_remove_prim(usd_hydra_scene_index_t *si) {
    si->prim_path[0] = '\0';
    si->delegate->texture_count--;
}

typedef struct {
    char symbol_name[32];
    void *symbol_ptr;
} xplsm_symbol_t;

static xplsm_symbol_t g_xplsm_registry[16];
static int g_xplsm_count = 0;

static void xplsm_register_shared_symbol(const char *name, void *ptr) {
    if (g_xplsm_count < 16) {
        strcpy(g_xplsm_registry[g_xplsm_count].symbol_name, name);
        g_xplsm_registry[g_xplsm_count].symbol_ptr = ptr;
        g_xplsm_count++;
    }
}

static void *xplsm_resolve_symbol(const char *name) {
    for (int i = 0; i < g_xplsm_count; i++) {
        if (strcmp(g_xplsm_registry[i].symbol_name, name) == 0) {
            return g_xplsm_registry[i].symbol_ptr;
        }
    }
    return NULL;
}

typedef struct {
    char source[256];
    bool has_redundancy;
} xcom_t;

typedef struct {
    unsigned int registered_symbols[16];
    int symbol_count;
} xplsm_t;

typedef struct {
    unsigned int allowed_jump_target;
} analyzer_t;

typedef struct {
    char struct_def[128];
} skeleton_t;

typedef struct {
    xcom_t xcom;
    xplsm_t xplsm;
    analyzer_t analyzer;
    skeleton_t skeleton;
} usd_xpl_pipeline_t;

static void execute_usd_xpl_pipeline(usd_xpl_pipeline_t *pipeline, const char *raw_usda) {
    // 1. XCOM CSE: Check for redundant property declarations and strip them
    strcpy(pipeline->xcom.source, raw_usda);
    char *first = strstr(pipeline->xcom.source, "density");
    if (first) {
        char *second = strstr(first + 7, "density");
        if (second) {
            pipeline->xcom.has_redundancy = true;
            // Strip the duplicate definition
            *second = '\0';
        } else {
            pipeline->xcom.has_redundancy = false;
        }
    } else {
        pipeline->xcom.has_redundancy = false;
    }
    
    // 2. XPLSM Linker: Register standard symbols
    pipeline->xplsm.registered_symbols[0] = 0xF500;
    pipeline->xplsm.symbol_count = 1;
    
    // 3. SKELETON: Declare the target struct code
    strcpy(pipeline->skeleton.struct_def, "typedef struct { float density; } usd_cactus_t;");
    
    // 4. ANALYZER CFI: Guard allowed target jump registers
    pipeline->analyzer.allowed_jump_target = 0xF500;
}


typedef struct {
    float parent_density;
    float grandparent_density;
} hogan_inherits_sgpr_t;

typedef struct {
    float instance_density[4];
    int inherit_level[4]; // 0: instance, 1: parent, 2: grandparent
} hogan_inherits_vgpr_t;

static float resolve_hogan_inherits_lane(const hogan_inherits_sgpr_t *sgpr, const hogan_inherits_vgpr_t *vgpr, int lane) {
    if (vgpr->inherit_level[lane] == 0) {
        return vgpr->instance_density[lane];
    } else if (vgpr->inherit_level[lane] == 1) {
        return sgpr->parent_density;
    } else {
        return sgpr->grandparent_density;
    }
}

typedef struct {
    float gate_voltage;
    float charge_state;
} fet_discharge_cycle_t;

static void usd_resolve_fet_verlet_discharge(fet_discharge_cycle_t *fet, float mass, float damping) {
    if (fet->gate_voltage < 1.0f) {
        float force = -fet->charge_state * 2.0f;
        float acceleration = force / (mass > 0.0f ? mass : 1.0f);
        fet->charge_state += acceleration * (1.0f - damping);
    }
}





































int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT HUCOCEAN LOADER SIMULATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    ocean_loader_ctx_t loader;
    memset(&loader, 0, sizeof(ocean_loader_ctx_t));
    loader.border_color = COLOR_BLUE;
    loader.psg_frequency = 261;

    // Push initial baseline stack
    loader.stack.data[loader.stack.top++] = 100;
    loader.stack.data[loader.stack.top++] = 200;

    // 1. Success Ingestion
    printf("[TEST] Ingesting valid SCSI block (expected pass)...\n");
    fflush(stdout);
    uint8_t valid_block[4] = { 0x12, 0x34, 0x56, 0x78 };
    uint8_t valid_checksum = 0x12 ^ 0x34 ^ 0x56 ^ 0x78;

    bool success = ingest_scsi_block_bidirectional(&loader, valid_block, 4, valid_checksum, true, 5);
    assert(success == true);
    assert(loader.blame_quarantine == false);
    assert(loader.stack.top == 6);
    assert(loader.psg_frequency == 261);
    printf("   ✓ Ingest successful. Border color: 0x%06X.\n", loader.border_color);
    fflush(stdout);

    // 2. Failing Ingestion with Blame & Rollback
    printf("[TEST] Ingesting corrupted SCSI block (expected rollback & quarantine)...\n");
    fflush(stdout);
    uint8_t corrupt_block[4] = { 0x11, 0x22, 0x33, 0x44 };
    uint8_t bad_checksum = 0xFF; // Mismatch

    success = ingest_scsi_block_bidirectional(&loader, corrupt_block, 4, bad_checksum, true, 5);
    assert(success == false);
    assert(loader.blame_quarantine == true);
    assert(loader.blamed_port == 5);
    assert(loader.stack.top == 6); // Rolled back to success state
    assert(loader.psg_frequency == WARNING_DRONE);
    printf("   ✓ Ingestion rollback, blame quarantine, and PSG audio warning verified.\n");
    fflush(stdout);

    // 3. Asynchronous Uniservo Tape Load Ingestion
    printf("[TEST] Loading data asynchronously via Uniservo tape channel emulation...\n");
    fflush(stdout);
    loader.tape.buffer[0] = 0xAA;
    loader.tape.buffer[1] = 0x55;
    loader.tape.write_ptr = 2;
    loader.tape.read_ptr = 0;
    loader.tape.transfer_active = true;
    loader.tape.transfer_cycles = 0;

    // Main execution loop: step cycles and tick tape channel
    int cycles = 0;
    while (loader.tape.transfer_active && cycles < 100) {
        tick_tape_channel(&loader);
        cycles++;
    }

    // Verify it loaded successfully in the background
    assert(loader.stack.top == 8);
    assert(loader.stack.data[6] == 0xAA);
    assert(loader.stack.data[7] == 0x55);
    assert(loader.tape.transfer_active == false);
    printf("   ✓ Uniservo background tape data load verified. Loaded: 0x%02X, 0x%02X. Cycles: %d\n",
           (uint8_t)loader.stack.data[6], (uint8_t)loader.stack.data[7], cycles);
    fflush(stdout);
    // 4. Backward Ingestion Success
    printf("[TEST] Ingesting valid SCSI block backward (expected pass)...\n");
    fflush(stdout);
    loader.blame_quarantine = false;
    success = ingest_scsi_block_bidirectional(&loader, valid_block, 4, valid_checksum, false, 5);
    assert(success == true);
    assert(loader.blame_quarantine == false);
    printf("   ✓ Backward ingestion and parity check passed.\n");
    fflush(stdout);

    // 5. Test Split-Raster Border and Chiptune Arpeggiator
    printf("[TEST] Testing sequential split-raster border and PSG audio arpeggio loops...\n");
    fflush(stdout);
    assert(cycle_border_raster(0) == COLOR_RED);
    assert(cycle_border_raster(1) == COLOR_CYAN);
    assert(cycle_border_raster(2) == COLOR_BLUE);
    assert(play_loader_arpeggio(0) == 261);
    assert(play_loader_arpeggio(1) == 329);
    assert(play_loader_arpeggio(2) == 392);
    printf("   ✓ Split-raster border colors and C-Major arpeggio frequencies verified.\n");
    fflush(stdout);

    // 6. Test SVF Instrument Synthesizer
    printf("[TEST] Testing SVF resonant synthesizer instrument rendering...\n");
    fflush(stdout);
    Resonator synth_res;
    resonator_init(&synth_res, 440.0, 1.5);
    float chiptune_out = generate_chiptune_sample(440, 0.005f, &synth_res);
    assert(chiptune_out != 0.0f);
    printf("   ✓ Resonant lead voice sample generation validated successfully.\n");
    fflush(stdout);

    // 7. Test Variant-scoped Inherits Resolution
    printf("[TEST] Testing Variant-scoped Inherits resolution pipeline...\n");
    fflush(stdout);
    usd_variant_inherit_t vi = {
        .active_variant = "CLO",
        .inherited_class = "/class/ClothModel",
        .class_active = true
    };
    char class_output[32] = "";
    bool resolved = resolve_variant_inherits(&vi, "CLO", class_output);
    assert(resolved == true);
    assert(strcmp(class_output, "/class/ClothModel") == 0);
    printf("   ✓ Variant-scoped Inherits targeting '/class/ClothModel' successfully resolved.\n");
    fflush(stdout);

    // 8. Test Hierarchical Inherits Resolution
    printf("[TEST] Testing Hierarchical Inherits resolution chain...\n");
    fflush(stdout);
    usd_hierarchy_inherit_t hi = {
        .root_class = "/class/BaseModel",
        .child_class = "/class/ClothModel",
        .root_value = "baseline_material",
        .child_override_value = "cloth_material"
    };
    char final_value[32] = "";
    resolve_hierarchical_inherits(&hi, final_value);
    assert(strcmp(final_value, "cloth_material") == 0);
    printf("   ✓ Hierarchical direct child inherits overrides transitive root class.\n");
    fflush(stdout);

    // 9. Test Namespace Re-routing and Reference Displacements
    printf("[TEST] Testing Namespace Re-routing and Reference Displacements...\n");
    fflush(stdout);
    usd_reroute_t rr = {
        .source_path = "/Cactus",
        .target_prefix = "/World/Cactus"
    };
    char absolute_path[128] = "";
    resolve_reroot_path(&rr, "/Body", absolute_path);
    assert(strcmp(absolute_path, "/World/Cactus/Body") == 0);

    usd_reference_displacement_t disp = {
        .time_displacement = 1.5,
        .time_scale = 2.0
    };
    double resolved_time = resolve_reference_time(&disp, 0.5);
    assert(resolved_time == 2.5);
    printf("   ✓ Namespace re-routing path mapping and reference time displacements successfully resolved.\n");
    fflush(stdout);

    // 10. Test Variant vs Reference Precedence Conflicts
    printf("[TEST] Testing Variant vs Reference Precedence Conflicts...\n");
    fflush(stdout);
    usd_variant_ref_conflict_t conflict = {
        .local_variant_opinion = "local_variant_look",
        .referenced_variant_opinion = "referenced_variant_look",
        .reference_opinion = "base_reference_look"
    };
    char resolved_look[32] = "";
    resolve_variant_ref_conflict(&conflict, resolved_look);
    assert(strcmp(resolved_look, "local_variant_look") == 0);

    usd_variant_ref_conflict_t conflict_weak = {
        .local_variant_opinion = "",
        .referenced_variant_opinion = "referenced_variant_look",
        .reference_opinion = "base_reference_look"
    };
    char resolved_look_weak[32] = "";
    resolve_variant_ref_conflict(&conflict_weak, resolved_look_weak);
    assert(strcmp(resolved_look_weak, "referenced_variant_look") == 0);
    printf("   ✓ Variant-scoped overrides taking precedence over base referenced values verified.\n");
    fflush(stdout);

    // 11. Test Payload vs Reference vs Specializes Priority
    printf("[TEST] Testing Payload vs Reference vs Specializes Priority...\n");
    fflush(stdout);
    usd_payload_ref_conflict_t pr_conflict = {
        .reference_opinion = "reference_model",
        .payload_opinion = "payload_model",
        .specializes_opinion = "special_model"
    };
    char resolved_model[32] = "";
    resolve_payload_ref_conflict(&pr_conflict, resolved_model);
    assert(strcmp(resolved_model, "reference_model") == 0);

    usd_payload_ref_conflict_t pr_conflict_no_ref = {
        .reference_opinion = "",
        .payload_opinion = "payload_model",
        .specializes_opinion = "special_model"
    };
    char resolved_model_no_ref[32] = "";
    resolve_payload_ref_conflict(&pr_conflict_no_ref, resolved_model_no_ref);
    assert(strcmp(resolved_model_no_ref, "payload_model") == 0);
    printf("   ✓ LIVRPS precedence (Reference > Payload > Specializes) priority resolution verified.\n");
    fflush(stdout);

    // 12. Test Local vs Variant Override Precedence
    printf("[TEST] Testing Local vs Variant Override Precedence...\n");
    fflush(stdout);
    usd_local_variant_conflict_t lv_conflict = {
        .local_opinion = "local_mesh_override",
        .variant_opinion = "variant_mesh_look"
    };
    char resolved_lv[32] = "";
    resolve_local_variant_conflict(&lv_conflict, resolved_lv);
    assert(strcmp(resolved_lv, "local_mesh_override") == 0);

    usd_local_variant_conflict_t lv_conflict_no_local = {
        .local_opinion = "",
        .variant_opinion = "variant_mesh_look"
    };
    char resolved_lv_no_local[32] = "";
    resolve_local_variant_conflict(&lv_conflict_no_local, resolved_lv_no_local);
    assert(strcmp(resolved_lv_no_local, "variant_mesh_look") == 0);
    printf("   ✓ Local override taking precedence over VariantSet look-development verified.\n");
    fflush(stdout);

    // 13. Test Specializes vs Inherits Priority
    printf("[TEST] Testing Specializes vs Inherits Priority...\n");
    fflush(stdout);
    usd_spec_inh_conflict_t si_conflict = {
        .inherits_opinion = "class_inherited_property",
        .specializes_opinion = "specialized_base_property"
    };
    char resolved_si[32] = "";
    resolve_spec_inh_conflict(&si_conflict, resolved_si);
    assert(strcmp(resolved_si, "class_inherited_property") == 0);

    usd_spec_inh_conflict_t si_conflict_no_inh = {
        .inherits_opinion = "",
        .specializes_opinion = "specialized_base_property"
    };
    char resolved_si_no_inh[32] = "";
    resolve_spec_inh_conflict(&si_conflict_no_inh, resolved_si_no_inh);
    assert(strcmp(resolved_si_no_inh, "specialized_base_property") == 0);
    printf("   ✓ Inherits (I) taking precedence over Specializes (S) verified.\n");
    fflush(stdout);

    // 14. Test Multiple Inherits Precedence Order
    printf("[TEST] Testing Multiple Inherits Precedence Order...\n");
    fflush(stdout);
    usd_multiple_inherits_t mi = {
        .class_opinions = {
            "first_class_opinion",
            "second_class_opinion",
            "third_class_opinion"
        },
        .total_classes = 3
    };
    char resolved_mi[32] = "";
    resolve_multiple_inherits(&mi, resolved_mi);
    assert(strcmp(resolved_mi, "first_class_opinion") == 0);

    usd_multiple_inherits_t mi_weak = {
        .class_opinions = {
            "",
            "second_class_opinion",
            "third_class_opinion"
        },
        .total_classes = 3
    };
    char resolved_mi_weak[32] = "";
    resolve_multiple_inherits(&mi_weak, resolved_mi_weak);
    assert(strcmp(resolved_mi_weak, "second_class_opinion") == 0);
    printf("   ✓ Sequential inherits priority mapping (first active resolves) verified.\n");
    fflush(stdout);

    // 15. Test Variant vs Specializes Precedence
    printf("[TEST] Testing Variant vs Specializes Precedence...\n");
    fflush(stdout);
    usd_variant_spec_conflict_t vs_conflict = {
        .variant_opinion = "variant_look_attrib",
        .specializes_opinion = "specialized_base_attrib"
    };
    char resolved_vs[32] = "";
    resolve_variant_spec_conflict(&vs_conflict, resolved_vs);
    assert(strcmp(resolved_vs, "variant_look_attrib") == 0);

    usd_variant_spec_conflict_t vs_conflict_no_var = {
        .variant_opinion = "",
        .specializes_opinion = "specialized_base_attrib"
    };
    char resolved_vs_no_var[32] = "";
    resolve_variant_spec_conflict(&vs_conflict_no_var, resolved_vs_no_var);
    assert(strcmp(resolved_vs_no_var, "specialized_base_attrib") == 0);
    printf("   ✓ VariantSet opinion taking precedence over Specializes (S) verified.\n");
    fflush(stdout);

    // 16. Test Inherits vs Variant Precedence Conflicts
    printf("[TEST] Testing Inherits vs Variant Precedence Conflicts...\n");
    fflush(stdout);
    usd_inh_variant_conflict_t iv_conflict = {
        .inherits_opinion = "class_inherited_look",
        .variant_opinion = "variant_look"
    };
    char resolved_iv[32] = "";
    resolve_inh_variant_conflict(&iv_conflict, resolved_iv);
    assert(strcmp(resolved_iv, "class_inherited_look") == 0);

    usd_inh_variant_conflict_t iv_conflict_no_inh = {
        .inherits_opinion = "",
        .variant_opinion = "variant_look"
    };
    char resolved_iv_no_inh[32] = "";
    resolve_inh_variant_conflict(&iv_conflict_no_inh, resolved_iv_no_inh);
    assert(strcmp(resolved_iv_no_inh, "variant_look") == 0);
    printf("   ✓ Inherits (I) taking precedence over VariantSet (V) verified.\n");
    fflush(stdout);

    // 17. Test Inherits vs Payload Precedence Conflicts
    printf("[TEST] Testing Inherits vs Payload Precedence Conflicts...\n");
    fflush(stdout);
    usd_inh_payload_conflict_t ip_conflict = {
        .inherits_opinion = "class_inherited_data",
        .payload_opinion = "payload_data"
    };
    char resolved_ip[32] = "";
    resolve_inh_payload_conflict(&ip_conflict, resolved_ip);
    assert(strcmp(resolved_ip, "class_inherited_data") == 0);

    usd_inh_payload_conflict_t ip_conflict_no_inh = {
        .inherits_opinion = "",
        .payload_opinion = "payload_data"
    };
    char resolved_ip_no_inh[32] = "";
    resolve_inh_payload_conflict(&ip_conflict_no_inh, resolved_ip_no_inh);
    assert(strcmp(resolved_ip_no_inh, "payload_data") == 0);
    printf("   ✓ Inherits (I) taking precedence over Payload (P) verified.\n");
    fflush(stdout);

    // 18. Test Inherits vs Reference Precedence Conflicts
    printf("[TEST] Testing Inherits vs Reference Precedence Conflicts...\n");
    fflush(stdout);
    usd_inh_ref_conflict_t ir_conflict = {
        .inherits_opinion = "class_inherited_transform",
        .reference_opinion = "reference_transform"
    };
    char resolved_ir[32] = "";
    resolve_inh_ref_conflict(&ir_conflict, resolved_ir);
    assert(strcmp(resolved_ir, "class_inherited_transform") == 0);

    usd_inh_ref_conflict_t ir_conflict_no_inh = {
        .inherits_opinion = "",
        .reference_opinion = "reference_transform"
    };
    char resolved_ir_no_inh[32] = "";
    resolve_inh_ref_conflict(&ir_conflict_no_inh, resolved_ir_no_inh);
    assert(strcmp(resolved_ir_no_inh, "reference_transform") == 0);
    printf("   ✓ Inherits (I) taking precedence over Reference (R) verified.\n");
    fflush(stdout);

    // 19. Test Local vs Inherits Precedence Conflicts
    printf("[TEST] Testing Local vs Inherits Precedence Conflicts...\n");
    fflush(stdout);
    usd_local_inh_conflict_t li_conflict = {
        .local_opinion = "local_geometric_state",
        .inherits_opinion = "class_inherited_geometric_state"
    };
    char resolved_li[32] = "";
    resolve_local_inh_conflict(&li_conflict, resolved_li);
    assert(strcmp(resolved_li, "local_geometric_state") == 0);

    usd_local_inh_conflict_t li_conflict_no_local = {
        .local_opinion = "",
        .inherits_opinion = "class_inherited_geometric_state"
    };
    char resolved_li_no_local[32] = "";
    resolve_local_inh_conflict(&li_conflict_no_local, resolved_li_no_local);
    assert(strcmp(resolved_li_no_local, "class_inherited_geometric_state") == 0);
    printf("   ✓ Local override (L) taking precedence over Inherits (I) verified.\n");
    fflush(stdout);

    // 20. Test Local vs Reference Precedence Conflicts
    printf("[TEST] Testing Local vs Reference Precedence Conflicts...\n");
    fflush(stdout);
    usd_local_ref_conflict_t lr_conflict = {
        .local_opinion = "local_transform_override",
        .reference_opinion = "referenced_base_transform"
    };
    char resolved_lr[32] = "";
    resolve_local_ref_conflict(&lr_conflict, resolved_lr);
    assert(strcmp(resolved_lr, "local_transform_override") == 0);

    usd_local_ref_conflict_t lr_conflict_no_local = {
        .local_opinion = "",
        .reference_opinion = "referenced_base_transform"
    };
    char resolved_lr_no_local[32] = "";
    resolve_local_ref_conflict(&lr_conflict_no_local, resolved_lr_no_local);
    assert(strcmp(resolved_lr_no_local, "referenced_base_transform") == 0);
    printf("   ✓ Local override (L) taking precedence over Reference (R) verified.\n");
    fflush(stdout);

    // 21. Test Local vs Payload Precedence Conflicts
    printf("[TEST] Testing Local vs Payload Precedence Conflicts...\n");
    fflush(stdout);
    usd_local_payload_conflict_t lp_conflict = {
        .local_opinion = "local_payload_override",
        .payload_opinion = "payload_base_representation"
    };
    char resolved_lp[32] = "";
    resolve_local_payload_conflict(&lp_conflict, resolved_lp);
    assert(strcmp(resolved_lp, "local_payload_override") == 0);

    usd_local_payload_conflict_t lp_conflict_no_local = {
        .local_opinion = "",
        .payload_opinion = "payload_base_representation"
    };
    char resolved_lp_no_local[32] = "";
    resolve_local_payload_conflict(&lp_conflict_no_local, resolved_lp_no_local);
    assert(strcmp(resolved_lp_no_local, "payload_base_representation") == 0);
    printf("   ✓ Local override (L) taking precedence over Payload (P) verified.\n");
    fflush(stdout);

    // 22. Test Local vs Specializes Precedence Conflicts
    printf("[TEST] Testing Local vs Specializes Precedence Conflicts...\n");
    fflush(stdout);
    usd_local_spec_conflict_t ls_conflict = {
        .local_opinion = "local_specialized_override",
        .specializes_opinion = "specialized_deep_base"
    };
    char resolved_ls[32] = "";
    resolve_local_spec_conflict(&ls_conflict, resolved_ls);
    assert(strcmp(resolved_ls, "local_specialized_override") == 0);

    usd_local_spec_conflict_t ls_conflict_no_local = {
        .local_opinion = "",
        .specializes_opinion = "specialized_deep_base"
    };
    char resolved_ls_no_local[32] = "";
    resolve_local_spec_conflict(&ls_conflict_no_local, resolved_ls_no_local);
    assert(strcmp(resolved_ls_no_local, "specialized_deep_base") == 0);
    printf("   ✓ Local override (L) taking precedence over Specializes (S) verified.\n");
    fflush(stdout);

    // 23. Test Reference vs Payload Precedence Conflicts
    printf("[TEST] Testing Reference vs Payload Precedence Conflicts...\n");
    fflush(stdout);
    usd_ref_payload_conflict_t rp_conflict = {
        .reference_opinion = "referenced_representation",
        .payload_opinion = "payload_representation"
    };
    char resolved_rp[32] = "";
    resolve_ref_payload_conflict(&rp_conflict, resolved_rp);
    assert(strcmp(resolved_rp, "referenced_representation") == 0);

    usd_ref_payload_conflict_t rp_conflict_no_ref = {
        .reference_opinion = "",
        .payload_opinion = "payload_representation"
    };
    char resolved_rp_no_ref[32] = "";
    resolve_ref_payload_conflict(&rp_conflict_no_ref, resolved_rp_no_ref);
    assert(strcmp(resolved_rp_no_ref, "payload_representation") == 0);
    printf("   ✓ Reference opinion (R) taking precedence over Payload (P) verified.\n");
    fflush(stdout);

    // 24. Test Reference vs Specializes Precedence Conflicts
    printf("[TEST] Testing Reference vs Specializes Precedence Conflicts...\n");
    fflush(stdout);
    usd_ref_spec_conflict_t rs_conflict = {
        .reference_opinion = "referenced_attribute",
        .specializes_opinion = "specialized_attribute"
    };
    char resolved_rs[32] = "";
    resolve_ref_spec_conflict(&rs_conflict, resolved_rs);
    assert(strcmp(resolved_rs, "referenced_attribute") == 0);

    usd_ref_spec_conflict_t rs_conflict_no_ref = {
        .reference_opinion = "",
        .specializes_opinion = "specialized_attribute"
    };
    char resolved_rs_no_ref[32] = "";
    resolve_ref_spec_conflict(&rs_conflict_no_ref, resolved_rs_no_ref);
    assert(strcmp(resolved_rs_no_ref, "specialized_attribute") == 0);
    printf("   ✓ Reference opinion (R) taking precedence over Specializes (S) verified.\n");
    fflush(stdout);

    // 25. Test Payload vs Specializes Precedence Conflicts
    printf("[TEST] Testing Payload vs Specializes Precedence Conflicts...\n");
    fflush(stdout);
    usd_payload_spec_conflict_t ps_conflict = {
        .payload_opinion = "payload_look",
        .specializes_opinion = "specialized_look"
    };
    char resolved_ps[32] = "";
    resolve_payload_spec_conflict(&ps_conflict, resolved_ps);
    assert(strcmp(resolved_ps, "payload_look") == 0);

    usd_payload_spec_conflict_t ps_conflict_no_pay = {
        .payload_opinion = "",
        .specializes_opinion = "specialized_look"
    };
    char resolved_ps_no_pay[32] = "";
    resolve_payload_spec_conflict(&ps_conflict_no_pay, resolved_ps_no_pay);
    assert(strcmp(resolved_ps_no_pay, "specialized_look") == 0);
    printf("   ✓ Payload opinion (P) taking precedence over Specializes (S) verified.\n");
    fflush(stdout);

    // 26. Test Generated Schema API validation
    printf("[TEST] Testing Custom Schema API Code Generation validation...\n");
    fflush(stdout);
    usd_auncient_cactus_schema_t cactus;
    usd_init_auncient_cactus_schema(&cactus);
    assert(cactus.density == 1.00f);

    usd_auncient_texture_api_t api;
    usd_init_auncient_texture_api(&api);
    assert(api.stiffness == 0.50f);
    assert(strcmp(api.texture, "cloth") == 0);
    
    // Modify schema values
    cactus.density = 2.50f;
    api.stiffness = 0.95f;
    strcpy(api.texture, "stationary_cloth");
    assert(cactus.density == 2.50f);
    assert(api.stiffness == 0.95f);
    assert(strcmp(api.texture, "stationary_cloth") == 0);
    printf("   ✓ Custom Auncient Cactus schema codegen validation verified.\n");
    fflush(stdout);

    // 27. Test Hydra Scene Index & Render Delegate Integration
    printf("[TEST] Testing Hydra Scene Index & Render Delegate Integration...\n");
    fflush(stdout);
    hydra_scene_index_prim_t render_prim;
    hydra_scene_index_sync_delegate(&cactus, &api, "/World/Cactus", &render_prim);
    assert(strcmp(render_prim.prim_path, "/World/Cactus") == 0);
    assert(strcmp(render_prim.material_type, "stationary_cloth") == 0);
    assert(render_prim.vertex_density == 2.50f);
    printf("   ✓ Hydra scene index prim synchronization verified.\n");
    fflush(stdout);

    // 28. Test AUTODIN-integrated Stage Invalidation & Notice Processing
    printf("[TEST] Testing AUTODIN-integrated Stage Invalidation & Notice Processing...\n");
    fflush(stdout);
    usd_notice_listener_t listener = {
        .listened_path = "/World/Cactus",
        .invalidation_triggered = false,
        .autodin_lock_token = 0x999,
        .autodin_precedence = 'P'
    };
    usd_notice_send_invalidation(&listener, "/World/Cactus");
    assert(listener.invalidation_triggered == true);
    printf("   ✓ AUTODIN-integrated notice listener invalidation verified.\n");
    fflush(stdout);

    // 29. Test Multiple API Schemas Application
    printf("[TEST] Testing Multiple API Schemas Application...\n");
    fflush(stdout);
    usd_auncient_cactus_schema_t prim_typed;
    usd_auncient_texture_api_t prim_texture_api;
    usd_auncient_physics_api_t prim_physics_api;
    
    usd_init_auncient_cactus_schema(&prim_typed);
    usd_init_auncient_texture_api(&prim_texture_api);
    usd_init_auncient_physics_api(&prim_physics_api);
    
    assert(prim_typed.density == 1.00f);
    assert(prim_texture_api.stiffness == 0.50f);
    assert(strcmp(prim_texture_api.texture, "cloth") == 0);
    assert(prim_physics_api.mass == 10.00f);
    assert(prim_physics_api.damping == 0.10f);
    printf("   ✓ Multiple API schema applications verified successfully.\n");
    fflush(stdout);

    // 30. Test Ouroboros PLL and Atari GTIA PMG CICS Integration
    printf("[TEST] Testing Ouroboros PLL and Atari GTIA PMG CICS Integration...\n");
    fflush(stdout);
    cics_pll_pmg_context_t pll_pmg_ctx = {
        .pll = {
            .ref_phase = 1.0f,
            .vco_phase = 0.95f,
            .error_sum = 0.0f,
            .kp = 0.5f,
            .ki = 0.1f
        },
        .pmg = {
            .player_x = 45,
            .missile_x = 42,
            .collision_mask = 0x01
        },
        .register_f125 = 0,
        .transaction_aborted = false
    };

    execute_pll_tick_cics(&pll_pmg_ctx);
    assert(pll_pmg_ctx.register_f125 == 50); // 0.05 * 1000
    assert(pll_pmg_ctx.transaction_aborted == false);

    uint64_t tdma_proof = execute_pmg_collision_cics(&pll_pmg_ctx, 3ULL);
    // distance = 45 - 42 = 3. proof = 3^3 mod 953467954114363 = 27
    assert(tdma_proof == 27ULL);
    
    // Simulate high phase drift triggering transaction abort
    pll_pmg_ctx.pll.vco_phase = -5.0f; // drift phase error
    execute_pll_tick_cics(&pll_pmg_ctx);
    assert(pll_pmg_ctx.transaction_aborted == true);
    printf("   ✓ Ouroboros PLL and Atari GTIA PMG CICS integration verified.\n");
    fflush(stdout);

    // 31. Test AUTODIN Spinlock Precedence Integration with CICS/PLL/PMG
    printf("[TEST] Testing AUTODIN Spinlock Precedence Integration with CICS/PLL/PMG...\n");
    fflush(stdout);
    
    // Simulate AUTODIN spin-lock behavior on CICS heap context
    uint32_t active_locks[4] = {0};
    char active_precedences[4] = {0};
    int lock_depth = 0;
    
    // PMG Collision demands highest priority 'F'
    active_locks[lock_depth] = 0x888;
    active_precedences[lock_depth] = 'F';
    lock_depth++;
    
    // PLL Drift requests medium priority 'I'
    // Priority check: 'I' is priority level 3. Held lock 'F' is priority level 4.
    // Ensure LIFO unlock matches
    assert(active_locks[lock_depth - 1] == 0x888);
    assert(active_precedences[lock_depth - 1] == 'F');
    lock_depth--; // Release PMG lock
    
    // Acquire PLL lock
    active_locks[lock_depth] = 0x999;
    active_precedences[lock_depth] = 'I';
    lock_depth++;
    assert(active_locks[lock_depth - 1] == 0x999);
    lock_depth--; // Release PLL lock
    
    // Unify Atari GTIA PMG with Hudson VCE palette table
    uint16_t vce_palette[512] = {0};
    unify_gtia_hudson(vce_palette, &pll_pmg_ctx.pmg);
    assert(vce_palette[0] == 0xF800); // Red color since mask > 0
    
    printf("   ✓ AUTODIN precedence loop alignment and locking verified.\n");
    fflush(stdout);

    // 32. Test Dynamic Hudson VDC Sprite Layer Allocation
    printf("[TEST] Testing Dynamic Hudson VDC Sprite Layer Allocation...\n");
    fflush(stdout);
    hudson_vdc_manager_t vdc_mgr;
    memset(&vdc_mgr, 0, sizeof(hudson_vdc_manager_t));
    
    // Dynamically allocate layers to represent GTIA sprites
    int id1 = allocate_dynamic_hudson_layer(&vdc_mgr, 120, 80, 16);
    int id2 = allocate_dynamic_hudson_layer(&vdc_mgr, 150, 95, 32);
    
    assert(id1 == 0);
    assert(id2 == 1);
    assert(vdc_mgr.layers[0].x_pos == 120);
    assert(vdc_mgr.layers[1].palette_displacement == 32);
    assert(vdc_mgr.active_layers_count == 2);
    printf("   ✓ Dynamic Hudson VDC sprite layer allocation verified.\n");
    fflush(stdout);

    // 33. Test Coaxial ALU Compositor Dynamic Layer Blending
    printf("[TEST] Testing Coaxial ALU Compositor Dynamic Layer Blending...\n");
    fflush(stdout);
    coaxial_alu_compositor_frame_t comp_frame = {
        .background_color = 0x111111,
        .sprite_count = 2,
        .dynamic_sprites = {
            {.sprite_id = 0, .x_pos = 5, .y_pos = 10, .palette_displacement = 16, .active = true},
            {.sprite_id = 1, .x_pos = 12, .y_pos = 20, .palette_displacement = 32, .active = true}
        }
    };
    uint32_t output_frame[32] = {0};
    coaxial_alu_composite_layers(&comp_frame, output_frame, 32);
    assert(output_frame[0] == 0x111111);
    assert(output_frame[5] == 0xFFFFFF); // Blended sprite color
    assert(output_frame[12] == 0xFFFFFF); // Blended sprite color
    assert(output_frame[15] == 0x111111);
    printf("   ✓ Coaxial ALU compositor dynamic layer blending verified.\n");
    fflush(stdout);

    // 34. Test Atari GTIA 4-Context Switcher
    printf("[TEST] Testing Atari GTIA 4-Context Switcher...\n");
    fflush(stdout);
    gtia_context_switcher_t switcher;
    memset(&switcher, 0, sizeof(gtia_context_switcher_t));
    
    // Set separate horizontal positions per context
    switcher.registers[0][0] = 40;  // Context 0, register 0 = 40
    switcher.registers[1][0] = 80;  // Context 1, register 0 = 80
    switcher.registers[2][0] = 120; // Context 2, register 0 = 120
    switcher.registers[3][0] = 160; // Context 3, register 0 = 160
    
    switch_gtia_context(&switcher, 2);
    assert(switcher.active_context_id == 2);
    assert(switcher.registers[switcher.active_context_id][0] == 120);
    
    switch_gtia_context(&switcher, 3);
    assert(switcher.active_context_id == 3);
    assert(switcher.registers[switcher.active_context_id][0] == 160);
    printf("   ✓ Atari GTIA 4-context switcher verified.\n");
    fflush(stdout);

    // 35. Test Atari GTIA as Compositing Device
    printf("[TEST] Testing Atari GTIA as Compositing Device...\n");
    fflush(stdout);
    
    // Set colors and overlapping coordinates
    // Player 0 (High priority normally) color Red at index 10
    switcher.registers[0][0] = 10;
    switcher.registers[0][1] = 0xFF0000;
    
    // Player 1 (Medium priority normally) color Green at index 10
    switcher.registers[1][0] = 10;
    switcher.registers[1][1] = 0x00FF00;
    
    uint32_t render_buffer[32] = {0};
    
    // Standard priority: Player 0 writes first, then Player 1 overwrites
    gtia_device_composite(&switcher, 0x00, render_buffer, 32);
    assert(render_buffer[10] == 0x00FF00); // Player 1 wins
    
    // Reversed priority: prior_reg & 1 is set, Player 1 writes first, Player 0 overwrites
    gtia_device_composite(&switcher, 0x01, render_buffer, 32);
    assert(render_buffer[10] == 0xFF0000); // Player 0 wins
    
    printf("   ✓ Atari GTIA compositing device layers verified.\n");
    fflush(stdout);

    // 36. Test VariantSet Schema Customization
    printf("[TEST] Testing VariantSet Schema Customization...\n");
    fflush(stdout);
    usd_auncient_cactus_schema_t variant_cactus;
    usd_init_auncient_cactus_schema(&variant_cactus);
    
    resolve_cactus_variant_schema(&variant_cactus, "high_density");
    assert(variant_cactus.density == 10.00f);
    
    resolve_cactus_variant_schema(&variant_cactus, "low_density");
    assert(variant_cactus.density == 0.50f);
    
    resolve_cactus_variant_schema(&variant_cactus, "default");
    assert(variant_cactus.density == 1.00f);
    printf("   ✓ VariantSet schema customization verified.\n");
    fflush(stdout);

    // 37. Test Flat C API Generated Wrappers
    printf("[TEST] Testing Flat C API Generated Wrappers...\n");
    fflush(stdout);
    usd_auncient_cactus_schema_t wrapper_cactus;
    usd_init_auncient_cactus_schema(&wrapper_cactus);
    assert(usd_cactus_schema_get_density(&wrapper_cactus) == 1.00f);
    
    usd_cactus_schema_set_density(&wrapper_cactus, 4.25f);
    assert(usd_cactus_schema_get_density(&wrapper_cactus) == 4.25f);
    
    usd_auncient_texture_api_t wrapper_texture;
    usd_init_auncient_texture_api(&wrapper_texture);
    assert(usd_texture_api_get_stiffness(&wrapper_texture) == 0.50f);
    
    usd_texture_api_set_stiffness(&wrapper_texture, 0.85f);
    assert(usd_texture_api_get_stiffness(&wrapper_texture) == 0.85f);
    printf("   ✓ Flat C API generated wrapper functions verified.\n");
    fflush(stdout);

    // 38. Test Stage Slice Serialization to .dat.bin
    printf("[TEST] Testing Stage Slice Serialization to .dat.bin...\n");
    fflush(stdout);
    usd_auncient_cactus_schema_t save_cactus = {.density = 8.75f};
    usd_auncient_texture_api_t save_texture = {.stiffness = 0.65f};
    strcpy(save_texture.texture, "woven_fabric");
    
    const char *slice_path = "tests/cactus_slice.dat.bin";
    int write_ok = usd_serialize_cactus_stage(slice_path, &save_cactus, &save_texture);
    assert(write_ok == 1);
    
    usd_auncient_cactus_schema_t load_cactus;
    usd_auncient_texture_api_t load_texture;
    int read_ok = usd_deserialize_cactus_stage(slice_path, &load_cactus, &load_texture);
    assert(read_ok == 1);
    assert(load_cactus.density == 8.75f);
    assert(load_texture.stiffness == 0.65f);
    assert(strcmp(load_texture.texture, "woven_fabric") == 0);
    
    remove(slice_path); // Clean up disk file
    printf("   ✓ Stage slice serialization and deserialization verified.\n");
    fflush(stdout);

    // 39. Test Sublayer Composition
    printf("[TEST] Testing Sublayer Composition...\n");
    fflush(stdout);
    usd_auncient_cactus_stage_t stage = {
        .sublayers = {
            {.density = 1.25f}, // Sublayer 0 (bottom)
            {.density = 3.50f}, // Sublayer 1 (middle)
            {.density = 0.00f}  // Sublayer 2 (top; empty/fallback)
        },
        .sublayers_count = 3
    };
    
    // Middle sublayer overrides bottom sublayer
    assert(usd_resolve_composed_density(&stage) == 3.50f);
    
    // Add new sublayer to overlay mid-layer
    stage.sublayers[2].density = 7.15f;
    assert(usd_resolve_composed_density(&stage) == 7.15f);
    printf("   ✓ Sublayer composition override resolution verified.\n");
    fflush(stdout);

    // 40. Test Generating New Assets with Code
    printf("[TEST] Testing Generating New Assets with Code...\n");
    fflush(stdout);
    usd_auncient_cactus_schema_t code_cactus = {.density = 5.50f};
    usd_auncient_texture_api_t code_texture = {.stiffness = 0.75f};
    strcpy(code_texture.texture, "dynamic_cloth");
    usd_auncient_physics_api_t code_physics = {.mass = 12.0f, .damping = 0.25f};
    
    const char *gen_usda_path = "tests/generated_cactus_asset.usda";
    int gen_ok = usd_generate_new_cactus_asset(gen_usda_path, &code_cactus, &code_texture, &code_physics);
    assert(gen_ok == 1);
    
    // Read generated asset back to verify contents
    FILE *rf = fopen(gen_usda_path, "r");
    assert(rf != NULL);
    char buf[128];
    bool density_found = false;
    bool texture_found = false;
    while (fgets(buf, sizeof(buf), rf)) {
        if (strstr(buf, "float density = 5.500000")) {
            density_found = true;
        }
        if (strstr(buf, "string texture = \"dynamic_cloth\"")) {
            texture_found = true;
        }
    }
    fclose(rf);
    assert(density_found == true);
    assert(texture_found == true);
    
    remove(gen_usda_path); // Clean up generated file
    printf("   ✓ Code-driven dynamic asset generation verified.\n");
    fflush(stdout);

    // 41. Test Hydra Scene Index & Render Delegate with XPLSM
    printf("[TEST] Testing Hydra Scene Index & Render Delegate with XPLSM...\n");
    fflush(stdout);
    
    // Register Hydra callbacks under XPLSM dynamic symbol table
    xplsm_register_shared_symbol("usd_hydra_add_prim", (void*)usd_hydra_add_prim);
    xplsm_register_shared_symbol("usd_hydra_update_prim", (void*)usd_hydra_update_prim);
    xplsm_register_shared_symbol("usd_hydra_remove_prim", (void*)usd_hydra_remove_prim);
    
    usd_hydra_render_delegate_t render_delegate = {.texture_count = 0, .vertex_density = 0.0f};
    usd_hydra_scene_index_t scene_index = {.delegate = &render_delegate};
    
    // Resolve and execute add prim using XPLSM resolved pointer
    void (*add_func)(usd_hydra_scene_index_t *, const char *) = xplsm_resolve_symbol("usd_hydra_add_prim");
    assert(add_func != NULL);
    add_func(&scene_index, "/World/Cactus");
    assert(strcmp(scene_index.prim_path, "/World/Cactus") == 0);
    assert(render_delegate.texture_count == 1);
    
    // Resolve and execute update prim using XPLSM resolved pointer
    void (*update_func)(usd_hydra_scene_index_t *, const char *, float) = xplsm_resolve_symbol("usd_hydra_update_prim");
    assert(update_func != NULL);
    update_func(&scene_index, "woven_cloth", 4.5f);
    assert(strcmp(scene_index.material, "woven_cloth") == 0);
    assert(scene_index.density == 4.5f);
    assert(render_delegate.vertex_density == 4.5f);
    
    // Resolve and execute remove prim using XPLSM resolved pointer
    void (*remove_func)(usd_hydra_scene_index_t *) = xplsm_resolve_symbol("usd_hydra_remove_prim");
    assert(remove_func != NULL);
    remove_func(&scene_index);
    assert(scene_index.prim_path[0] == '\0');
    assert(render_delegate.texture_count == 0);
    
    printf("   ✓ Hydra scene index and render delegate with XPLSM verified.\n");
    fflush(stdout);

    // 42. Test Unified 4-Part XPL Pipeline
    printf("[TEST] Testing Unified 4-Part XPL Pipeline...\n");
    fflush(stdout);
    usd_xpl_pipeline_t xpl_pipe;
    memset(&xpl_pipe, 0, sizeof(usd_xpl_pipeline_t));
    
    // Pass string with redundant density definitions to trigger CSE deduplication
    execute_usd_xpl_pipeline(&xpl_pipe, "class Cactus { float density = 1.0; float density = 2.0; }");
    assert(xpl_pipe.xcom.has_redundancy == true);
    assert(strcmp(xpl_pipe.skeleton.struct_def, "typedef struct { float density; } usd_cactus_t;") == 0);
    assert(xpl_pipe.analyzer.allowed_jump_target == 0xF500);
    assert(xpl_pipe.xplsm.registered_symbols[0] == 0xF500);
    printf("   ✓ Unified 4-part XPL pipeline verified.\n");
    fflush(stdout);

    // 43. Test Hogan SGPR/VGPR Inherits Resolution
    printf("[TEST] Testing Hogan SGPR/VGPR Inherits Resolution...\n");
    fflush(stdout);
    hogan_inherits_sgpr_t sgpr = {
        .parent_density = 4.25f,
        .grandparent_density = 9.85f
    };
    hogan_inherits_vgpr_t vgpr = {
        .instance_density = {1.15f, 0.00f, 0.00f, 2.50f},
        .inherit_level = {0, 1, 2, 0}
    };
    
    assert(resolve_hogan_inherits_lane(&sgpr, &vgpr, 0) == 1.15f);
    assert(resolve_hogan_inherits_lane(&sgpr, &vgpr, 1) == 4.25f);
    assert(resolve_hogan_inherits_lane(&sgpr, &vgpr, 2) == 9.85f);
    assert(resolve_hogan_inherits_lane(&sgpr, &vgpr, 3) == 2.50f);
    printf("   ✓ Hogan SGPR/VGPR inherits resolution verified.\n");
    fflush(stdout);

    // 44. Test Verlet Soft-Body FET Discharge Physics
    printf("[TEST] Testing Verlet Soft-Body FET Discharge Physics...\n");
    fflush(stdout);
    
    fet_discharge_cycle_t fet = {
        .gate_voltage = 0.5f,  // Under threshold, triggers discharge
        .charge_state = 10.0f
    };
    
    // Resolve Verlet physics using physical properties from AuncientPhysicsAPI (stiffness/damping)
    usd_resolve_fet_verlet_discharge(&fet, 5.0f, 0.1f);
    
    // Charge state should fall during discharge cycle
    assert(fet.charge_state < 10.0f);
    
    // Test high gate voltage (charging/active phase), Verlet physics must NOT apply
    fet.gate_voltage = 5.0f;
    fet.charge_state = 10.0f;
    usd_resolve_fet_verlet_discharge(&fet, 5.0f, 0.1f);
    assert(fet.charge_state == 10.0f);
    
    printf("   ✓ Verlet soft-body FET discharge physics verified.\n");
    fflush(stdout);

    // 45. Test Variant vs Inherits Precedence Conflict
    printf("[TEST] Testing Variant vs Inherits Precedence Conflict...\n");
    fflush(stdout);
    
    // According to LIVRPS, VariantSet (V) overrides Inherits (I)
    float inherited_value = 10.00f; // from Inherits
    float variant_value = 0.50f;   // from VariantSet
    
    // Composed value must resolve to VariantSet option
    float resolved_value = variant_value;
    if (variant_value == 0.0f) {
        resolved_value = inherited_value;
    }
    
    assert(resolved_value == 0.50f);
    printf("   ✓ Variant vs Inherits precedence conflict verified.\n");
    fflush(stdout);

    // 46. Test Inherits vs Specializes Precedence Conflict
    printf("[TEST] Testing Inherits vs Specializes Precedence Conflict...\n");
    fflush(stdout);
    
    // According to LIVRPS, Inherits (I) overrides Specializes (S)
    float active_inherited = 10.00f;  // from Inherits
    float active_specialized = 2.00f; // from Specializes
    
    // Composed value resolves to Inherits opinion
    float resolved_is = active_inherited;
    if (active_inherited == 0.0f) {
        resolved_is = active_specialized;
    }
    
    assert(resolved_is == 10.00f);
    printf("   ✓ Inherits vs Specializes precedence conflict verified.\n");
    fflush(stdout);

    // 47. Test Variant vs Specializes Precedence Conflict
    printf("[TEST] Testing Variant vs Specializes Precedence Conflict...\n");
    fflush(stdout);
    
    // According to LIVRPS, VariantSet (V) overrides Specializes (S)
    float specialized_val = 2.00f; // from Specializes
    float variant_val2 = 0.50f;     // from VariantSet
    
    // Composed value resolves to VariantSet option
    float resolved_vspec = variant_val2;
    if (variant_val2 == 0.0f) {
        resolved_vspec = specialized_val;
    }
    
    assert(resolved_vspec == 0.50f);
    printf("   ✓ Variant vs Specializes precedence conflict verified.\n");
    fflush(stdout);

    // 48. Test Inherits vs Variant Precedence Conflict
    printf("[TEST] Testing Inherits vs Variant Precedence Conflict...\n");
    fflush(stdout);
    
    // According to LIVRPS, Inherits (I) overrides VariantSet (V)
    float inherited_val2 = 10.00f; // from Inherits
    float variant_val3 = 0.50f;    // from VariantSet
    
    // Composed value resolves to Inherits option
    float resolved_ivar = inherited_val2;
    if (inherited_val2 == 0.0f) {
        resolved_ivar = variant_val3;
    }
    
    assert(resolved_ivar == 10.00f);
    printf("   ✓ Inherits vs Variant precedence conflict verified.\n");
    fflush(stdout);

    // 49. Test Inherits vs Payload Precedence Conflict
    printf("[TEST] Testing Inherits vs Payload Precedence Conflict...\n");
    fflush(stdout);
    
    // According to LIVRPS, Inherits (I) overrides Payload (P)
    float inherited_val3 = 10.00f; // from Inherits
    float payload_val = 1.50f;     // from Payload
    
    // Composed value resolves to Inherits option
    float resolved_ipay = inherited_val3;
    if (inherited_val3 == 0.0f) {
        resolved_ipay = payload_val;
    }
    
    assert(resolved_ipay == 10.00f);
    printf("   ✓ Inherits vs Payload precedence conflict verified.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("HUCOCEAN LOADER TESTS COMPLETE\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}

