#ifndef TSFI_WIRE_FIRMWARE_H
#define TSFI_WIRE_FIRMWARE_H

#include "tsfi_types.h"
#include "tsfi_dysnomia.h"
#include "lau_thunk.h"
#include "lau_wire_mcp.h"
#include "lau_vram.h"
#include <sys/types.h>

// Include the generated RTL state structure
#include "../src/firmware/LauWireFirmware_rtl.h"

// Verilog-modeled Standard Cell Library for Bijective Wavefronts
DEFINE_MAPPED_STRUCT(LauWireFirmware,
    // --- Input Pins (Rx) ---
    void (*rx_pin_pty_out)(void *context, const char *data, size_t len);
    
    // --- Output Pins (Tx) ---
    ssize_t (*tx_pin_pty_in)(void *context, const char *data, size_t len);

    // --- Combinatorial Logic / Standard Cells ---
    void (*cell_update_base)(void *context, const char *data, size_t len);
    int (*cell_printf)(int session_id, const char *format, ...);
    
    // --- Wavefront I/O Cells ---
    ssize_t (*cell_inject_stdin)(int session_id, const char *data, size_t len);
    size_t (*cell_extract_stdout)(int session_id, char *buf, size_t max_len);

    // --- Physiological Boundary Cells ---
    void (*cell_brush_teeth)(int timeout_sec);
    void (*cell_mcp_execute)(LauMasterWavefront *mw);
    void (*cell_wave_exec)(uint32_t op, uint8_t dest, uint8_t src1, uint8_t src2);
    void (*cell_neural_observe)(int session_id, char *out_intent, char *out_insight);

    // --- Zhong Standard Cells ---
    void (*cell_zhong_sync_wait)(uint64_t handle, uint64_t val);
    void (*cell_zhong_sync_signal)(uint64_t handle, uint64_t val);
    void (*cell_zhong_atomic_wait)(uint64_t addr, uint64_t val);

    // --- Interrupt Controller Cells (Signals & Hardware I/O) ---
    int (*cell_hardware_poll)(int timeout_ms, char *out_stdin_buf, size_t max_len);

    // --- Seal Management Cells (Jurisdictional Boundary) ---
    void (*cell_seal_capture)(void *ptr);
    void (*cell_seal_release)(void *ptr);

    // --- Registry Database Cells (ZMM-Accelerated) ---
    void (*cell_reg_insert)(LauMetadata *m);
    void (*cell_reg_remove)(void *ptr);
    LauMetadata* (*cell_reg_lookup)(void *ptr);
    void (*cell_reg_strobe_zmm)(int zmm_idx, void *ptr);
    void* (*cell_mem_genesis)(size_t size, LauHeaderType type, const char *file, int line);
    void (*cell_mem_reclaim)(void *ptr, size_t size);

    // --- Registry Database (Zhong-Managed) ---
    LauRegistryManifold manifold __attribute__((aligned(512)));

    // --- Direct Manifolds ---
    LauMasterWavefront wavefront __attribute__((aligned(512)));

    // --- Hardware VRAM (Replaces stdout stream) ---
    LauVRAM vram __attribute__((aligned(512)));

    // --- The Synthesized Hardware State ---
    LauWireFirmware_State rtl __attribute__((aligned(512)));
    
    // --- The Firmware IS The YI ---
    struct YI core_yi __attribute__((aligned(512)));
) __attribute__((aligned(512)))

// Global singleton instantiation of the standard cell library
LauWireFirmware* tsfi_wire_firmware_get(void);
LauWireFirmware* tsfi_wire_firmware_get_no_init(void);

// Initialize the firmware and bind the zero-copy operators
void tsfi_wire_firmware_init(void);
void tsfi_wire_firmware_setup_signals(void);

// Standard Cell: Structure Loader (Provenance Bus)
void tsfi_wire_firmware_load_struct(LauWireFirmware *fw, void *ptr);
void tsfi_wire_firmware_load_manifest(LauWireFirmware *fw, void *manifest);
void tsfi_wire_firmware_load_waveform(LauWireFirmware *fw, int reg_idx, void *wave_ptr);
void tsfi_wire_firmware_strobe(LauWireFirmware *fw, uint8_t addr, uint64_t data);
void tsfi_wire_firmware_math_strobe(LauWireFirmware *fw);
void tsfi_wire_firmware_validate(LauWireFirmware *fw);

// Standard Cell: Logic Execution (Thunk Invoker)
void tsfi_wire_firmware_execute(LauWireFirmware *fw, void *handle, void *context);

// Standard Cell: Autonomous peripheral stepper
void tsfi_wire_firmware_step_peripheral(LauWireFirmware *fw, void *pty);

// Peripheral Utility: Get PTY Master FD for a session
int tsfi_wire_firmware_get_session_fd(int session_id);

#endif // TSFI_WIRE_FIRMWARE_H
