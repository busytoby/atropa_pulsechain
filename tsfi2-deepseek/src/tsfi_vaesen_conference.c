#include "tsfi_vaesen_conference.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void tsfi_vaesen_conference_init(TsfiVaesenConferenceRoom *room) {
    if (!room) return;
    memset(room, 0, sizeof(TsfiVaesenConferenceRoom));
    room->rmsnorm_ctrl.mode = TSFI_RMSNORM_MODE_CONFERENCE;
    room->rmsnorm_ctrl.gain_gamma = 0.875f;
    room->rmsnorm_ctrl.epsilon_floor = 1e-6f;
    room->rmsnorm_ctrl.max_rail_amplitude = 2.0f;
    room->rmsnorm_ctrl.resuscitation_count = 0;
}

void tsfi_vaesen_conference_set_rmsnorm_mode(TsfiVaesenConferenceRoom *room, TsfiOperatorRmsNormMode mode) {
    if (!room) return;
    room->rmsnorm_ctrl.mode = (uint8_t)mode;
    if (mode == TSFI_RMSNORM_MODE_SANCTUARY) {
        room->rmsnorm_ctrl.gain_gamma = 0.500f;
        room->rmsnorm_ctrl.max_rail_amplitude = 1.000f;
    } else if (mode == TSFI_RMSNORM_MODE_CONFERENCE) {
        room->rmsnorm_ctrl.gain_gamma = 0.875f; /* BMRC Lyapunov limit */
        room->rmsnorm_ctrl.max_rail_amplitude = 2.000f;
    } else if (mode == TSFI_RMSNORM_MODE_CRISIS) {
        room->rmsnorm_ctrl.gain_gamma = 1.000f;
        room->rmsnorm_ctrl.max_rail_amplitude = 3.500f;
    }
}

float tsfi_vaesen_conference_apply_rmsnorm(TsfiVaesenConferenceRoom *room, float *samples, size_t count) {
    if (!room || !samples || count == 0) return 0.0f;

    float sum_sq = 0.0f;
    for (size_t i = 0; i < count; ++i) {
        sum_sq += samples[i] * samples[i];
    }
    float mean_sq = sum_sq / (float)count;

    /* Kouwenhoven Resuscitation Watchdog check for flatline */
    if (mean_sq < room->rmsnorm_ctrl.epsilon_floor) {
        room->rmsnorm_ctrl.resuscitation_count++;
        /* Inject defibrillation waveform */
        for (size_t i = 0; i < count; ++i) {
            samples[i] = sinf((float)i * 0.1f) * 0.5f;
        }
        mean_sq = 0.125f; /* Mean squared of 0.5 amplitude sine wave */
    }

    float rms = sqrtf(mean_sq + room->rmsnorm_ctrl.epsilon_floor);
    float scale = (1.0f / rms) * room->rmsnorm_ctrl.gain_gamma;
    for (size_t i = 0; i < count; ++i) {
        float normalized = samples[i] * scale;
        /* Dynamic range rail clamping */
        if (normalized > room->rmsnorm_ctrl.max_rail_amplitude) {
            normalized = room->rmsnorm_ctrl.max_rail_amplitude;
        } else if (normalized < -room->rmsnorm_ctrl.max_rail_amplitude) {
            normalized = -room->rmsnorm_ctrl.max_rail_amplitude;
        }
        samples[i] = normalized;
    }

    return rms;
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

int tsfi_vaesen_conference_populate_canonical_taxonomy(TsfiVaesenConferenceRoom *room) {
    if (!room) return -1;
    tsfi_vaesen_conference_init(room);

    static const TsfiVaesenEntity canonical_roster[] = {
        /* The Society Clan */
        { .name = "Linnea Elfvestam",   .clan_id = TSFI_CLAN_SOCIETY, .physique = 1, .precision = 2, .logic = 5, .fervour = 4, .dogma = 3, .caste = 5, .fear_level = 1, .edo22_freq = 8,  .mathieu_q = 850 },
        { .name = "Dr. Albert Victor",  .clan_id = TSFI_CLAN_SOCIETY, .physique = 2, .precision = 5, .logic = 4, .fervour = 2, .dogma = 4, .caste = 4, .fear_level = 1, .edo22_freq = 12, .mathieu_q = 920 },
        { .name = "Father Thomas",      .clan_id = TSFI_CLAN_SOCIETY, .physique = 2, .precision = 2, .logic = 3, .fervour = 5, .dogma = 5, .caste = 4, .fear_level = 2, .edo22_freq = 14, .mathieu_q = 1950 },
        { .name = "Astrid Lilja",       .clan_id = TSFI_CLAN_SOCIETY, .physique = 1, .precision = 3, .logic = 4, .fervour = 5, .dogma = 2, .caste = 3, .fear_level = 2, .edo22_freq = 17, .mathieu_q = 2100 },
        { .name = "Kasper Wallin",      .clan_id = TSFI_CLAN_SOCIETY, .physique = 4, .precision = 5, .logic = 2, .fervour = 3, .dogma = 2, .caste = 2, .fear_level = 1, .edo22_freq = 4,  .mathieu_q = 780 },
        { .name = "Baron von Essen",    .clan_id = TSFI_CLAN_SOCIETY, .physique = 2, .precision = 3, .logic = 3, .fervour = 3, .dogma = 5, .caste = 5, .fear_level = 1, .edo22_freq = 16, .mathieu_q = 890 },
        { .name = "Sally G. Larsen",    .clan_id = TSFI_CLAN_SOCIETY, .physique = 1, .precision = 4, .logic = 5, .fervour = 4, .dogma = 1, .caste = 3, .fear_level = 1, .edo22_freq = 6,  .mathieu_q = 600 },

        /* The Warden Clan */
        { .name = "Gårdstomte",         .clan_id = TSFI_CLAN_WARDEN,  .physique = 2, .precision = 5, .logic = 3, .fervour = 4, .dogma = 5, .caste = 2, .fear_level = 1, .edo22_freq = 2,  .mathieu_q = 650 },
        { .name = "Skogsrå",            .clan_id = TSFI_CLAN_WARDEN,  .physique = 2, .precision = 4, .logic = 3, .fervour = 4, .dogma = 2, .caste = 2, .fear_level = 2, .edo22_freq = 5,  .mathieu_q = 1450 },
        { .name = "Gruvrå",             .clan_id = TSFI_CLAN_WARDEN,  .physique = 4, .precision = 3, .logic = 4, .fervour = 3, .dogma = 4, .caste = 3, .fear_level = 2, .edo22_freq = 9,  .mathieu_q = 1800 },
        { .name = "Askefroa",           .clan_id = TSFI_CLAN_WARDEN,  .physique = 3, .precision = 2, .logic = 4, .fervour = 4, .dogma = 4, .caste = 3, .fear_level = 2, .edo22_freq = 7,  .mathieu_q = 1750 },
        { .name = "Vättar",             .clan_id = TSFI_CLAN_WARDEN,  .physique = 2, .precision = 4, .logic = 4, .fervour = 3, .dogma = 4, .caste = 2, .fear_level = 2, .edo22_freq = 1,  .mathieu_q = 1200 },

        /* The Teddy Bear Clan */
        { .name = "Teddy Bear Citizen", .clan_id = TSFI_CLAN_TEDDY_BEAR, .physique = 3, .precision = 4, .logic = 4, .fervour = 5, .dogma = 1, .caste = 3, .fear_level = 1, .edo22_freq = 10, .mathieu_q = 500 },

        /* The Undead Clan */
        { .name = "Myling",             .clan_id = TSFI_CLAN_UNDEAD,  .physique = 1, .precision = 2, .logic = 2, .fervour = 5, .dogma = 2, .caste = 1, .fear_level = 4, .edo22_freq = 19, .mathieu_q = 4500 },
        { .name = "Gengångare",         .clan_id = TSFI_CLAN_UNDEAD,  .physique = 4, .precision = 2, .logic = 1, .fervour = 5, .dogma = 3, .caste = 1, .fear_level = 3, .edo22_freq = 20, .mathieu_q = 3600 },
        { .name = "Nattramn",           .clan_id = TSFI_CLAN_UNDEAD,  .physique = 2, .precision = 4, .logic = 3, .fervour = 4, .dogma = 3, .caste = 2, .fear_level = 3, .edo22_freq = 13, .mathieu_q = 2800 },
        { .name = "Kyrkogrim",          .clan_id = TSFI_CLAN_UNDEAD,  .physique = 4, .precision = 3, .logic = 2, .fervour = 5, .dogma = 5, .caste = 4, .fear_level = 3, .edo22_freq = 15, .mathieu_q = 3100 },

        /* The Outcast / Mythic Clan */
        { .name = "Näcken",             .clan_id = TSFI_CLAN_OUTCAST, .physique = 3, .precision = 4, .logic = 3, .fervour = 5, .dogma = 1, .caste = 1, .fear_level = 3, .edo22_freq = 11, .mathieu_q = 3250 },
        { .name = "Troll",              .clan_id = TSFI_CLAN_OUTCAST, .physique = 5, .precision = 1, .logic = 1, .fervour = 3, .dogma = 4, .caste = 1, .fear_level = 2, .edo22_freq = 3,  .mathieu_q = 1100 },
        { .name = "Bäckahäst",          .clan_id = TSFI_CLAN_OUTCAST, .physique = 4, .precision = 3, .logic = 2, .fervour = 4, .dogma = 1, .caste = 1, .fear_level = 3, .edo22_freq = 18, .mathieu_q = 3400 },
        { .name = "Varulv",             .clan_id = TSFI_CLAN_OUTCAST, .physique = 5, .precision = 3, .logic = 1, .fervour = 5, .dogma = 1, .caste = 1, .fear_level = 4, .edo22_freq = 21, .mathieu_q = 4850 }
    };

    uint32_t count = sizeof(canonical_roster) / sizeof(canonical_roster[0]);
    for (uint32_t i = 0; i < count; ++i) {
        if (tsfi_vaesen_conference_add_entity(room, &canonical_roster[i]) < 0) {
            return -1;
        }
    }
    return 0;
}
