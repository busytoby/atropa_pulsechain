# Plan: Wavefront Standard Cell & Provenance Loading

This plan replaces the legacy "shared memory" model (pointers between user structs and headers) with a hardware-mapped "Wavefront Standard Cell" in `LauWireFirmware`, populated via a strobe-based Provenance Bus.

## Phase 1: HDL Definition (Verilog)

### 1.1 Update `src/firmware/LauWireFirmware.v`
Add registers for the `LauWiredHeader` fields that are common to all `DEFINE_MAPPED_STRUCT`s.

- **Registers (The Cell):**
  - `reg [31:0] cell_version`
  - `reg [63:0] cell_resonance_status_ptr`
  - `reg        cell_ftw`
  - `reg [31:0] cell_counter`
  - `reg        cell_autonomous_excuse_active`
  - `reg [63:0] cell_logic_epoch_handle`
  - `reg [63:0] cell_logic_state_handle`
  - `reg [63:0] cell_logic_directive_handle`
  - `reg [63:0] cell_logic_scramble_handle`
  - `reg [63:0] cell_logic_provenance_handle`
  - `reg [63:0] cell_logic_hilbert_handle`
  - `reg [63:0] cell_logic_hilbert_batch_handle`
  - `reg [63:0] cell_logic_classify_handle`

- **Provenance Loading Bus:**
  - `input wire [63:0] prov_data`
  - `input wire [7:0]  prov_addr`
  - `input wire        prov_strobe`

- **Logic:**
  - Update `always @(posedge epoch_strobe)` block to handle `prov_strobe`. On strobe, write `prov_data` to the register indexed by `prov_addr`.

## Phase 2: Transpilation & C Model

### 2.1 Build and Run `tsfi_v2c`
Ensure `tools/tsfi_v2c.c` correctly generates the `__m512i` types for the Wave512 cells and basic types for the new registers.
Run: `bin/tsfi_v2c src/firmware/LauWireFirmware.v`

## Phase 3: Firmware API & Integration

### 3.1 Update `inc/tsfi_wire_firmware.h`
Add function:
`void tsfi_wire_firmware_load_struct(LauWireFirmware *fw, void *ptr);`

### 3.2 Update `src/tsfi_wire_firmware.c`
Implement `tsfi_wire_firmware_load_struct`:
- Extract `LauWiredHeader` from `ptr`.
- Sequentially strobe each field (version, handles, etc.) into the RTL module via `pty_rx_data` (or the new `prov_*` pins if we expose them directly in the C state).
- *Note:* Since `tsfi_v2c` maps pins to struct fields in `LauWireFirmware_State`, we can set `rtl.prov_data` etc. directly and call `eval_sequential`.

### 3.3 Modify `src/tsfi_wiring.c`
Update `lau_wire_system`:
- Instead of setting pointers in `ws` to addresses in `h`, call `tsfi_wire_firmware_load_struct(tsfi_wire_firmware_get(), ws)`.
- This "synchronizes" the hardware standard cell with the software structure.

## Phase 4: Verification

### 4.1 Create `tests/test_wavefront_cell.c`
- Allocate a `WaveSystem` (or any `DEFINE_MAPPED_STRUCT`).
- Initialize its header fields.
- Call `lau_wire_system`.
- Verify via `LauWireFirmware_get()->rtl` that the registers now match the header values.
- Verify that changes to the software header are *not* reflected in the cell until a new provenance strobe occurs (Absolute Determinism).

## Commands
```bash
# Build transpiler
gcc tools/tsfi_v2c.c -o bin/tsfi_v2c
# Synthesize
bin/tsfi_v2c src/firmware/LauWireFirmware.v
# Build project
make
# Run tests
./test_all.sh
```
