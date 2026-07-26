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

static void hydra_scene_index_sync_delegate(const usd_auncient_cactus_schema_t *schema, const char *path, hydra_scene_index_prim_t *render_prim) {
    strcpy(render_prim->prim_path, path);
    strcpy(render_prim->material_type, schema->texture);
    render_prim->vertex_density = schema->density;
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
    assert(cactus.stiffness == 0.50f);
    assert(strcmp(cactus.texture, "cloth") == 0);
    
    // Modify schema values
    cactus.density = 2.50f;
    cactus.stiffness = 0.95f;
    strcpy(cactus.texture, "stationary_cloth");
    assert(cactus.density == 2.50f);
    assert(cactus.stiffness == 0.95f);
    assert(strcmp(cactus.texture, "stationary_cloth") == 0);
    printf("   ✓ Custom Auncient Cactus schema codegen validation verified.\n");
    fflush(stdout);

    // 27. Test Hydra Scene Index & Render Delegate Integration
    printf("[TEST] Testing Hydra Scene Index & Render Delegate Integration...\n");
    fflush(stdout);
    hydra_scene_index_prim_t render_prim;
    hydra_scene_index_sync_delegate(&cactus, "/World/Cactus", &render_prim);
    assert(strcmp(render_prim.prim_path, "/World/Cactus") == 0);
    assert(strcmp(render_prim.material_type, "stationary_cloth") == 0);
    assert(render_prim.vertex_density == 2.50f);
    printf("   ✓ Hydra scene index prim synchronization verified.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("HUCOCEAN LOADER TESTS COMPLETE\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
