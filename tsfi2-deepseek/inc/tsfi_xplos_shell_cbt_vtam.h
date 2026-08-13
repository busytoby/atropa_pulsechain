#ifndef TSFI_XPLOS_SHELL_CBT_VTAM_H
#define TSFI_XPLOS_SHELL_CBT_VTAM_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

bool tsfi_xplos_shell_cbt_vtam(const char *cmd);
void tsfi_vtam_coax_write_buffer(const char *data);
void tsfi_vtam_coax_read_buffer(char *dest, int max_len);

// VTAM SNA LU6.2 2PC SyncPoint & Deadlock WFG APIs
bool tsfi_vsen_vtam_2pc_prepare(uint64_t tx_id, uint32_t lu_address_id);
bool tsfi_vsen_vtam_2pc_commit(uint64_t tx_id, uint32_t lu_address_id);
bool tsfi_vsen_vtam_2pc_rollback(uint64_t tx_id, uint32_t lu_address_id);
bool tsfi_vsen_vtam_deadlock_check_and_resolve(uint32_t lu_address_id, uint64_t tx_id);

#endif
