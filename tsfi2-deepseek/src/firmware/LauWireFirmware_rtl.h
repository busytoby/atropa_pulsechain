#ifndef RTL_H
#define RTL_H
#include <stdint.h>
#include <stdbool.h>
#include <immintrin.h>
#include "tsfi_math.h"
typedef struct {
  bool epoch_strobe, reset_strobe, directive_strobe, state_ready, prov_strobe, exec_strobe, exec_done, wave_instr_strobe, wave_instr_done, pty_rx_valid, pty_rx_ready, pty_tx_valid, pty_tx_ready, periph_directive_strobe, periph_epoch_strobe, periph_state_ready, spawn_gemini_strobe, cell_ftw, cell_autonomous_excuse_active, val_strobe, epoch_error, zhong_strobe, zhong_done;
  uint32_t directive_cmd, sys_version, sys_counter, periph_directive_cmd, cell_version, cell_counter, last_directive, log_stdout_head, log_stdout_tail, log_stdout_valid_head, log_stdout_lock, host_epoch, plugin_epoch, last_host_epoch, cell_status, log_stdin_head, log_stdin_tail, log_stdin_lock, session_id, wave_instr_op, zhong_status, zhong_op;
  uint8_t wave_instr_dest, wave_instr_src1, wave_instr_src2, wave_instr_src3;
  uint64_t prov_data, prov_addr, exec_handle, exec_context, provenance_hash, cell_resonance_status_ptr, periph_wave_handle, log_stdin_ptr, log_stdout_ptr, bunch_session_mask, zhong_rebar_ptr, zhong_rebar_size, zhong_timeline_handle, zhong_timeline_wait_val, zhong_timeline_sig_val;
  uint64_t cell_logic_epoch_handle, cell_logic_state_handle, cell_logic_directive_handle, cell_logic_scramble_handle, cell_logic_provenance_handle, cell_logic_hilbert_handle, cell_logic_hilbert_batch_handle, cell_logic_classify_handle;
  uint64_t cell_slot_ptr[32];
  __m512i wrf[128];
  __m512i pty_rx_wave, pty_tx_wave, wave_in_a, wave_in_b, wave_out;
  // Qing Standard Cells Cache
  uint8_t dys_ctx_mod_slot;
  bool dys_ctx_valid;
  TSFiModContextBatch8 dys_ctx;
} LauWireFirmware_State;
void LauWireFirmware_eval_combinatorial(LauWireFirmware_State *s);
void LauWireFirmware_eval_sequential(LauWireFirmware_State *s);
#endif
