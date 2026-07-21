#ifndef TSFI_AUTODIN_CUMULATIVE_WINCHESTER_H
#define TSFI_AUTODIN_CUMULATIVE_WINCHESTER_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint8_t scsi_command_opcode;
    uint8_t scsi_handshake_status_register;
    uint8_t keycode_register_32_delta_direction; // Keycode 32 (d/D)
    uint8_t keycode_register_30_alpha_direction; // Keycode 30 (a/A)
    uint32_t winchester_cylinder_head_sector_address;
    uint64_t auncient_virtual_hardware_cycle_counter;
} autodin_cumulative_winchester_hardware_t;

/* Initialize Automated Digital Network cumulative WinchesterMQ hardware emulator */
int autodin_cumulative_winchester_initialize(
    autodin_cumulative_winchester_hardware_t *hardware
);

/* Process SCSI handshake loop and verify keycode register states (Keycode 32 / 30) */
int autodin_cumulative_winchester_process_scsi_handshake(
    autodin_cumulative_winchester_hardware_t *hardware,
    uint8_t keycode_input
);

/* Evaluate WinchesterMQ Field-Effect Transistor discharge cycle solver */
int autodin_cumulative_winchester_evaluate_fet_discharge(
    autodin_cumulative_winchester_hardware_t *hardware,
    double *discharge_potential_volts
);

#endif // TSFI_AUTODIN_CUMULATIVE_WINCHESTER_H
