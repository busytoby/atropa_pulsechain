#ifndef TSFI_VAESEN_CONFERENCE_H
#define TSFI_VAESEN_CONFERENCE_H

#include <stdint.h>
#include <stdbool.h>

#define TSFI_VAESEN_MAX_ENTITIES 32
#define TSFI_VAESEN_MAX_CONNECTIONS (TSFI_VAESEN_MAX_ENTITIES * TSFI_VAESEN_MAX_ENTITIES)
#define TSFI_VAESEN_CONFERENCE_STEPS 64

typedef enum {
    TSFI_PLL_STATE_UNCOUPLED = 0,
    TSFI_PLL_STATE_TRACKING  = 1,
    TSFI_PLL_STATE_LOCKED    = 2,
    TSFI_PLL_STATE_FRACTURED = 3
} TsfiPllLockState;

typedef struct {
    uint16_t id;
    char     name[32];
    uint8_t  physique;   /* [1..5] */
    uint8_t  precision;  /* [1..5] */
    uint8_t  logic;      /* [1..5] */
    uint8_t  fervour;    /* [1..5] */
    uint8_t  dogma;      /* [1..5] */
    uint8_t  caste;      /* [1..5] */
    uint8_t  fear_level; /* [1..5] */
    uint8_t  edo22_freq; /* [1..22] */
    uint16_t mathieu_q;  /* milli-units */
} TsfiVaesenEntity;

typedef struct {
    uint16_t source_id;
    uint16_t target_id;
    float    verlet_pos;
    float    verlet_prev_pos;
    float    spring_k;
    float    rest_length_L0;
    float    damping_c;
    uint16_t pll_phase_deg;      /* [0..359] */
    uint8_t  pll_lock_state;     /* TsfiPllLockState */
    uint8_t  caste_delta;
    uint64_t engram_weight_saat;
} __attribute__((packed)) TsfiVaesenConnectionEdge;

typedef struct {
    uint32_t num_entities;
    uint32_t num_connections;
    TsfiVaesenEntity entities[TSFI_VAESEN_MAX_ENTITIES];
    TsfiVaesenConnectionEdge connections[TSFI_VAESEN_MAX_CONNECTIONS];
} TsfiVaesenConferenceRoom;

/* Conference API */
void tsfi_vaesen_conference_init(TsfiVaesenConferenceRoom *room);
int  tsfi_vaesen_conference_add_entity(TsfiVaesenConferenceRoom *room, const TsfiVaesenEntity *entity);
void tsfi_vaesen_conference_introduce_pair(TsfiVaesenConferenceRoom *room, uint16_t id_a, uint16_t id_b);
void tsfi_vaesen_conference_step(TsfiVaesenConferenceRoom *room, float dt);
void tsfi_vaesen_conference_run_full(TsfiVaesenConferenceRoom *room, uint32_t num_steps, float dt);

int  tsfi_vaesen_conference_save_dat_bin(const TsfiVaesenConferenceRoom *room, const char *filepath);
int  tsfi_vaesen_conference_load_dat_bin(TsfiVaesenConferenceRoom *room, const char *filepath);

#endif /* TSFI_VAESEN_CONFERENCE_H */
