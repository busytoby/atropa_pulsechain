#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "tsfi_autodin_cumulative_winchester.h"

int autodin_cumulative_winchester_initialize(
    autodin_cumulative_winchester_hardware_t *hardware
) {
    if (!hardware) return -1;

    memset(hardware, 0, sizeof(autodin_cumulative_winchester_hardware_t));
    hardware->scsi_command_opcode = 0x28; // SCSI Read(10)
    hardware->scsi_handshake_status_register = 0x00; // Ready
    hardware->winchester_cylinder_head_sector_address = 0x00010001;

    return 0;
}

int autodin_cumulative_winchester_process_scsi_handshake(
    autodin_cumulative_winchester_hardware_t *hardware,
    uint8_t keycode_input
) {
    if (!hardware) return -1;

    hardware->auncient_virtual_hardware_cycle_counter++;

    // Keycode 32 (d/D) for WinchesterMQ SCSI Delta Direction register
    if (keycode_input == 32) {
        hardware->keycode_register_32_delta_direction = 1;
        hardware->scsi_handshake_status_register = 0x01; // Handshake Active
    }
    // Keycode 30 (a/A) for WinchesterMQ SCSI Alpha Direction register
    else if (keycode_input == 30) {
        hardware->keycode_register_30_alpha_direction = 1;
        hardware->scsi_handshake_status_register = 0x02; // Handshake Acknowledged
    } else {
        hardware->scsi_handshake_status_register = 0x00; // Idle
    }

    return 0;
}

int autodin_cumulative_winchester_evaluate_fet_discharge(
    autodin_cumulative_winchester_hardware_t *hardware,
    double *discharge_potential_volts
) {
    if (!hardware || !discharge_potential_volts) return -1;

    // Rule 10: Soft body physics (Verlet solver) applies strictly to FET discharge cycles
    // Rule 12: Non-preferential accumulator model (Child-Langmuir law is strictly banned)
    double time_step = 0.001;
    double damping_factor = 0.995;
    double current_potential = *discharge_potential_volts;

    // Exponential decay non-preferential accumulator
    current_potential = current_potential * exp(-time_step * 10.0) * damping_factor;
    *discharge_potential_volts = current_potential;

    return 0;
}
