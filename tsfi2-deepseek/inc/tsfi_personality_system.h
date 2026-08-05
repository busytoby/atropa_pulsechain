#ifndef TSFI_PERSONALITY_SYSTEM_H
#define TSFI_PERSONALITY_SYSTEM_H

#include "tsfi_personality_core.h"

// Transaction handlers
evaluation_tx_t begin_evaluation_transaction(teddy_geometry_t *target);
bool commit_evaluation_transaction(evaluation_tx_t *tx);
void rollback_evaluation_transaction(evaluation_tx_t *tx);

avatar_tx_t begin_avatar_transaction(agent_avatar_t *avatar);
bool commit_avatar_transaction(avatar_tx_t *tx, const char *bin_filepath);
void rollback_avatar_transaction(avatar_tx_t *tx);

// System boundaries and auth checks
bool engage_system_boundary(agent_avatar_t *avatar, teddy_personality_t personality);
bool authorize_boundary_via_gost(agent_avatar_t *avatar, const uint32_t *key_8words, uint32_t token_left, uint32_t token_right);
bool validate_sdk_typestate(const agent_avatar_t *avatar);

// WinchesterMQ hardware integrations
void execute_displacement_shader_sync(const agent_avatar_t *avatar, double *vertex_offsets, uint32_t count);
bool emulate_winchester_scsi_loop(agent_avatar_t *avatar);

// Physics loops
void simulate_fet_verlet_discharge(agent_avatar_t *avatar, double time_step);

#endif // TSFI_PERSONALITY_SYSTEM_H
