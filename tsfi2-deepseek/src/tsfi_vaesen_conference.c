#include "tsfi_vaesen_conference.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void tsfi_vaesen_conference_init(TsfiVaesenConferenceRoom *room) {
    if (!room) return;
    memset(room, 0, sizeof(TsfiVaesenConferenceRoom));
}

int tsfi_vaesen_conference_add_entity(TsfiVaesenConferenceRoom *room, const TsfiVaesenEntity *entity) {
    if (!room || !entity) return -1;
    if (room->num_entities >= TSFI_VAESEN_MAX_ENTITIES) return -1;

    uint32_t idx = room->num_entities++;
    room->entities[idx] = *entity;
    room->entities[idx].id = (uint16_t)idx;
    return (int)idx;
}

void tsfi_vaesen_conference_introduce_pair(TsfiVaesenConferenceRoom *room, uint16_t id_a, uint16_t id_b) {
    if (!room || id_a >= room->num_entities || id_b >= room->num_entities || id_a == id_b) return;
    if (room->num_connections >= TSFI_VAESEN_MAX_CONNECTIONS) return;

    TsfiVaesenEntity *ea = &room->entities[id_a];
    TsfiVaesenEntity *eb = &room->entities[id_b];

    TsfiVaesenConnectionEdge *edge = &room->connections[room->num_connections++];
    edge->source_id = id_a;
    edge->target_id = id_b;
    edge->verlet_pos = 100.0f;     /* Initial standoff distance */
    edge->verlet_prev_pos = 100.0f;
    edge->rest_length_L0 = 10.0f;  /* Comfortable conversation distance */

    int caste_diff = (int)ea->caste - (int)eb->caste;
    edge->caste_delta = (uint8_t)(caste_diff < 0 ? -caste_diff : caste_diff);

    /* Dogma determines bond spring stiffness */
    edge->spring_k = 0.5f + ((float)(ea->dogma + eb->dogma) * 0.25f);
    edge->damping_c = 0.85f; /* Damping coefficient */

    /* Initial PLL phase error derived from EDO-22 frequency differential */
    int freq_diff = (int)ea->edo22_freq - (int)eb->edo22_freq;
    if (freq_diff < 0) freq_diff = -freq_diff;
    edge->pll_phase_deg = (uint16_t)((freq_diff * 360 / 22) % 360);
    edge->pll_lock_state = TSFI_PLL_STATE_TRACKING;
    edge->engram_weight_saat = 0;
}

void tsfi_vaesen_conference_step(TsfiVaesenConferenceRoom *room, float dt) {
    if (!room || dt <= 0.0f) return;

    for (uint32_t i = 0; i < room->num_connections; ++i) {
        TsfiVaesenConnectionEdge *edge = &room->connections[i];
        if (edge->pll_lock_state == TSFI_PLL_STATE_FRACTURED) continue;

        TsfiVaesenEntity *ea = &room->entities[edge->source_id];
        TsfiVaesenEntity *eb = &room->entities[edge->target_id];

        /* 1. PLL Phase Update & Synchronization */
        float phase_rad = (float)edge->pll_phase_deg * (3.14159265f / 180.0f);
        float phase_error_torque = sinf(phase_rad);

        /* Tracking gain is aided by logic and hindered by dogma */
        float track_rate = 15.0f * ((float)(ea->logic + eb->logic) / 10.0f) /
                           (1.0f + 0.2f * (float)(ea->dogma + eb->dogma));

        float delta_phase = phase_error_torque * track_rate * dt * (180.0f / 3.14159265f);
        float new_phase = (float)edge->pll_phase_deg - delta_phase;
        while (new_phase < 0.0f) new_phase += 360.0f;
        while (new_phase >= 360.0f) new_phase -= 360.0f;
        edge->pll_phase_deg = (uint16_t)new_phase;

        /* Check PLL Lock threshold (within 8 degrees of aligned phase) */
        bool is_aligned = (edge->pll_phase_deg <= 8 || edge->pll_phase_deg >= 352);
        if (is_aligned) {
            edge->pll_lock_state = TSFI_PLL_STATE_LOCKED;
        } else {
            edge->pll_lock_state = TSFI_PLL_STATE_TRACKING;
        }

        /* 2. Forces Acting on Social Distance */
        float displacement = edge->verlet_pos - edge->rest_length_L0;
        float f_spring = -edge->spring_k * displacement;

        /* Caste disparity exerts repulsive force if not locked */
        float f_caste_repulsion = 0.0f;
        if (!is_aligned && edge->caste_delta > 1) {
            f_caste_repulsion = (float)edge->caste_delta * 4.0f;
        }

        /* Fervour adds kinetic agitation */
        float f_fervour = ((float)ea->fervour + (float)eb->fervour) * 0.5f;

        float total_force = f_spring + f_caste_repulsion;
        if (displacement > 0) total_force -= f_fervour;

        /* 3. Verlet Integration Step */
        float velocity = (edge->verlet_pos - edge->verlet_prev_pos) / dt;
        float damping_force = -edge->damping_c * velocity;
        float accel = total_force + damping_force;

        float next_pos = 2.0f * edge->verlet_pos - edge->verlet_prev_pos + accel * dt * dt;
        edge->verlet_prev_pos = edge->verlet_pos;
        edge->verlet_pos = next_pos;

        /* Clamping physical limits */
        if (edge->verlet_pos < 1.0f) edge->verlet_pos = 1.0f;

        /* 4. Structural Fracture Check */
        if (edge->verlet_pos > 180.0f && !is_aligned) {
            edge->pll_lock_state = TSFI_PLL_STATE_FRACTURED;
            /* Record traumatic friction engram */
            edge->engram_weight_saat += (uint64_t)(edge->verlet_pos * 1000.0f * (float)(ea->fervour + eb->fervour));
        } else if (is_aligned && edge->verlet_pos <= 15.0f) {
            /* Constructive bond engram stabilization */
            edge->engram_weight_saat = 1000000ULL; /* Full default endowment harmony */
        }
    }
}

void tsfi_vaesen_conference_run_full(TsfiVaesenConferenceRoom *room, uint32_t num_steps, float dt) {
    if (!room) return;
    for (uint32_t s = 0; s < num_steps; ++s) {
        tsfi_vaesen_conference_step(room, dt);
    }
}

int tsfi_vaesen_conference_save_dat_bin(const TsfiVaesenConferenceRoom *room, const char *filepath) {
    if (!room || !filepath) return -1;
    FILE *fp = fopen(filepath, "wb");
    if (!fp) return -1;

    size_t w1 = fwrite(&room->num_entities, sizeof(uint32_t), 1, fp);
    size_t w2 = fwrite(&room->num_connections, sizeof(uint32_t), 1, fp);
    size_t w3 = fwrite(room->entities, sizeof(TsfiVaesenEntity), room->num_entities, fp);
    size_t w4 = fwrite(room->connections, sizeof(TsfiVaesenConnectionEdge), room->num_connections, fp);

    fclose(fp);
    return (w1 == 1 && w2 == 1 && w3 == room->num_entities && w4 == room->num_connections) ? 0 : -1;
}

int tsfi_vaesen_conference_load_dat_bin(TsfiVaesenConferenceRoom *room, const char *filepath) {
    if (!room || !filepath) return -1;
    FILE *fp = fopen(filepath, "rb");
    if (!fp) return -1;

    size_t r1 = fread(&room->num_entities, sizeof(uint32_t), 1, fp);
    size_t r2 = fread(&room->num_connections, sizeof(uint32_t), 1, fp);
    if (r1 != 1 || r2 != 1 || room->num_entities > TSFI_VAESEN_MAX_ENTITIES || room->num_connections > TSFI_VAESEN_MAX_CONNECTIONS) {
        fclose(fp);
        return -1;
    }

    size_t r3 = fread(room->entities, sizeof(TsfiVaesenEntity), room->num_entities, fp);
    size_t r4 = fread(room->connections, sizeof(TsfiVaesenConnectionEdge), room->num_connections, fp);

    fclose(fp);
    return (r3 == room->num_entities && r4 == room->num_connections) ? 0 : -1;
}

int tsfi_vaesen_conference_synthesize_stanag_frame(
    const TsfiVaesenConferenceRoom *room,
    uint32_t connection_idx,
    TsfiVaesenStanagFrame *out_frame
) {
    if (!room || !out_frame || connection_idx >= room->num_connections) return -1;

    const TsfiVaesenConnectionEdge *edge = &room->connections[connection_idx];
    const TsfiVaesenEntity *ea = &room->entities[edge->source_id];
    const TsfiVaesenEntity *eb = &room->entities[edge->target_id];

    out_frame->clan_id = ea->clan_id;
    out_frame->sender_id = ea->id;
    out_frame->target_id = eb->id;
    out_frame->phase_offset_deg = edge->pll_phase_deg;
    out_frame->fervour_drive = ea->fervour;

    if (edge->pll_lock_state == TSFI_PLL_STATE_LOCKED) {
        snprintf(out_frame->message, sizeof(out_frame->message),
                 "[STANAG-5066] %s affirms mutual covenant with %s across EDO-22 carrier %u.",
                 ea->name, eb->name, ea->edo22_freq);
    } else if (edge->pll_lock_state == TSFI_PLL_STATE_TRACKING) {
        snprintf(out_frame->message, sizeof(out_frame->message),
                 "[STANAG-5066] %s evaluates %s at distance %.1f: maybe mutual terms can harmonize.",
                 ea->name, eb->name, edge->verlet_pos);
    } else {
        snprintf(out_frame->message, sizeof(out_frame->message),
                 "[STANAG-5066] %s rejects %s: phase fractured, dogmatic boundaries unyielding.",
                 ea->name, eb->name);
    }
    return 0;
}
