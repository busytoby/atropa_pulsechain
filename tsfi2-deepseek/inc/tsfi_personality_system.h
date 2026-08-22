#ifndef TSFI_PERSONALITY_SYSTEM_H
#define TSFI_PERSONALITY_SYSTEM_H

#include "tsfi_personality_core.h"

// Commits an izotope H-bridge flyback transaction under ACID safety constraints.
bool commit_izotope_flyback_transaction(evaluation_tx_t *tx, double switching_frequency, double max_safe_voltage);

evaluation_tx_t begin_evaluation_transaction(teddy_bear_geometry_t *target);

bool commit_evaluation_transaction(evaluation_tx_t *tx);

void rollback_evaluation_transaction(evaluation_tx_t *tx);


avatar_tx_t begin_avatar_transaction(agent_avatar_t *avatar);

bool commit_avatar_transaction(avatar_tx_t *tx, const char *bin_filepath);

void rollback_avatar_transaction(avatar_tx_t *tx);

// Engages systems via the SDK typestate and calculates parameters.
bool engage_system_boundary(agent_avatar_t *avatar, teddy_bear_personality_t personality);

// Authorizes the system boundary via GOST-encrypted state payloads when USD is not linked.
bool authorize_boundary_via_gost(agent_avatar_t *avatar, const uint32_t *key_8words, uint32_t token_left, uint32_t token_right);

// Validates the SDK typestate sequence
bool validate_sdk_typestate(const agent_avatar_t *avatar);

// Synchronizes vertex displacement math scales with WinchesterMQ register boundary constraints
void execute_displacement_shader_sync(const agent_avatar_t *avatar, double *vertex_offsets, uint32_t count);

// Emulates low-level WinchesterMQ SCSI register boundary handshake loop
bool emulate_winchester_scsi_loop(agent_avatar_t *avatar);

// Simulates Verlet soft-body dynamics of FET discharge cycles
void simulate_fet_verlet_discharge(agent_avatar_t *avatar, double time_step);

#endif // TSFI_PERSONALITY_SYSTEM_H
