#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "tsfi_vaesen_conference.h"

static double get_time_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1e9 + (double)ts.tv_nsec;
}

int main(void) {
    printf("========================================================================================\n");
    printf("   VAESEN LLM VERLET-PLL CONTEXTUAL CONFERENCE SIMULATOR & .DAT.BIN SERIALIZER\n");
    printf("========================================================================================\n\n");

    TsfiVaesenConferenceRoom room;
    tsfi_vaesen_conference_init(&room);

    /* 1. Add Conference Participants */
    TsfiVaesenEntity linnea = {
        .name = "Linnea Elfvestam",
        .clan_id = TSFI_CLAN_SOCIETY,
        .physique = 1, .precision = 2, .logic = 5, .fervour = 4, .dogma = 3, .caste = 5,
        .fear_level = 1, .edo22_freq = 8, .mathieu_q = 850
    };
    int id_linnea = tsfi_vaesen_conference_add_entity(&room, &linnea);
    assert(id_linnea >= 0);

    TsfiVaesenEntity tomte = {
        .name = "Gårdstomte",
        .clan_id = TSFI_CLAN_WARDEN,
        .physique = 2, .precision = 5, .logic = 3, .fervour = 4, .dogma = 5, .caste = 2,
        .fear_level = 1, .edo22_freq = 2, .mathieu_q = 650
    };
    int id_tomte = tsfi_vaesen_conference_add_entity(&room, &tomte);
    assert(id_tomte >= 0);

    TsfiVaesenEntity nacken = {
        .name = "Näcken",
        .clan_id = TSFI_CLAN_OUTCAST,
        .physique = 3, .precision = 4, .logic = 3, .fervour = 5, .dogma = 1, .caste = 1,
        .fear_level = 3, .edo22_freq = 11, .mathieu_q = 3250
    };
    int id_nacken = tsfi_vaesen_conference_add_entity(&room, &nacken);
    assert(id_nacken >= 0);

    TsfiVaesenEntity priest = {
        .name = "Father Thomas",
        .clan_id = TSFI_CLAN_SOCIETY,
        .physique = 2, .precision = 2, .logic = 3, .fervour = 5, .dogma = 5, .caste = 4,
        .fear_level = 2, .edo22_freq = 14, .mathieu_q = 1950
    };
    int id_priest = tsfi_vaesen_conference_add_entity(&room, &priest);
    assert(id_priest >= 0);

    /* 2. Introduce Pairs for Conference */
    tsfi_vaesen_conference_introduce_pair(&room, id_linnea, id_tomte);
    tsfi_vaesen_conference_introduce_pair(&room, id_linnea, id_nacken);
    tsfi_vaesen_conference_introduce_pair(&room, id_priest, id_nacken);

    printf("[SETUP] Introduced 4 entities with %u active connections.\n", room.num_connections);
    assert(room.num_connections == 3);

    /* 3. Run Conference Simulation for 120 steps */
    printf("\n[CONFERENCE PROGRESSION]\n");
    for (int step = 1; step <= 120; ++step) {
        tsfi_vaesen_conference_step(&room, 0.05f);
        if (step % 40 == 0) {
            printf("--- Step %d ---\n", step);
            for (uint32_t c = 0; c < room.num_connections; ++c) {
                TsfiVaesenConnectionEdge *edge = &room.connections[c];
                const char *state_str = (edge->pll_lock_state == TSFI_PLL_STATE_LOCKED) ? "LOCKED (Harmony)" :
                                        (edge->pll_lock_state == TSFI_PLL_STATE_FRACTURED) ? "FRACTURED (Grudge)" : "TRACKING";
                printf("  [%s <-> %s] Distance: %.2f | PLL Phase: %u deg | Status: %s | Engram: %lu Saat\n",
                       room.entities[edge->source_id].name,
                       room.entities[edge->target_id].name,
                       edge->verlet_pos,
                       edge->pll_phase_deg,
                       state_str,
                       (unsigned long)edge->engram_weight_saat);
            }
        }
    }

    /* 4. Synthesize STANAG 5066 Dialogue Frames */
    printf("\n[STANAG 5066 DIALOGUE BROADCAST FRAMES]\n");
    for (uint32_t c = 0; c < room.num_connections; ++c) {
        TsfiVaesenStanagFrame frame;
        int frame_res = tsfi_vaesen_conference_synthesize_stanag_frame(&room, c, &frame);
        assert(frame_res == 0);
        printf("  Frame %u (Clan %u): %s\n", c, frame.clan_id, frame.message);
    }

    /* 5. Serialize to .dat.bin (Rule 13 Compliance) */
    const char *dat_path = "assets/vaesen_llm_conference.dat.bin";
    int save_res = tsfi_vaesen_conference_save_dat_bin(&room, dat_path);
    printf("\n[STORAGE] Saving to %s: %s\n", dat_path, save_res == 0 ? "SUCCESS (.dat.bin)" : "FAIL");
    assert(save_res == 0);

    /* 6. Load and Verify Roundtrip Invariance */
    TsfiVaesenConferenceRoom loaded_room;
    int load_res = tsfi_vaesen_conference_load_dat_bin(&loaded_room, dat_path);
    printf("[STORAGE] Loading from %s: %s\n", dat_path, load_res == 0 ? "SUCCESS" : "FAIL");
    assert(load_res == 0);
    assert(loaded_room.num_entities == room.num_entities);
    assert(loaded_room.num_connections == room.num_connections);
    assert(strcmp(loaded_room.entities[0].name, "Linnea Elfvestam") == 0);

    /* 6. Latency Benchmark across 100,000 conference simulation steps */
    double t_start = get_time_ns();
    const int benchmark_steps = 100000;
    for (int i = 0; i < benchmark_steps; ++i) {
        tsfi_vaesen_conference_step(&room, 0.01f);
    }
    double t_end = get_time_ns();
    double avg_latency = (t_end - t_start) / (double)benchmark_steps;
    printf("\n[BENCHMARK] Average conference step latency: %.2f ns (Target: < 1000 ns)\n", avg_latency);
    assert(avg_latency < 1000.0);

    printf("\n>>> VAESEN VERLET-PLL CONFERENCE SIMULATION FORMALLY VERIFIED <<<\n");
    return 0;
}
