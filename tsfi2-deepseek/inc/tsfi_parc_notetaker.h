#ifndef TSFI_PARC_NOTETAKER_H
#define TSFI_PARC_NOTETAKER_H

typedef struct {
    float pos_x;
    float pos_y;
    float prev_x;
    float prev_y;
    float mass;
    float charge;
} tsfi_parc_fet_node_t;

typedef struct {
    tsfi_parc_fet_node_t source_node;
    tsfi_parc_fet_node_t gate_node;
    tsfi_parc_fet_node_t drain_node;
    float depletion_factor;
} tsfi_parc_notetaker_fet_t;

/* Initialize NoteTaker FET nodes */
void tsfi_parc_notetaker_init(tsfi_parc_notetaker_fet_t *fet);

/* Perform low-level Verlet solver step on mass-spring dynamics during the FET discharge cycle */
int tsfi_parc_notetaker_step_discharge(tsfi_parc_notetaker_fet_t *fet, float dt);

#endif // TSFI_PARC_NOTETAKER_H
