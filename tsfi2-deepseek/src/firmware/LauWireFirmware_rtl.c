#include "LauWireFirmware_rtl.h"
#include "tsfi_math.h"
#include <string.h>
void LauWireFirmware_eval_combinatorial(LauWireFirmware_State *state) {
  state->pty_rx_ready=1; state->pty_tx_ready=1; state->state_ready=1; state->provenance_hash=state->cell_logic_epoch_handle;
  state->periph_epoch_strobe = state->periph_state_ready && (state->host_epoch > state->plugin_epoch) && !state->epoch_error;
  state->pty_tx_wave = state->pty_rx_wave; state->pty_tx_valid = state->pty_rx_valid; }
void LauWireFirmware_eval_sequential(LauWireFirmware_State *state) {
  if (state->reset_strobe) {
    state->cell_version=0; state->cell_counter=0; state->last_directive=0; state->exec_done=0; state->spawn_gemini_strobe=0; state->log_stdout_head=0; state->log_stdout_tail=0; state->log_stdout_valid_head=0; state->log_stdout_lock=0; state->host_epoch=0; state->plugin_epoch=0; state->last_host_epoch=0; state->epoch_error=0; state->cell_status=0; state->log_stdin_head=0; state->log_stdin_tail=0; state->log_stdin_lock=0; state->log_stdin_ptr=0; state->log_stdout_ptr=0;
    state->wave_instr_done=0; state->zhong_rebar_ptr=0; state->zhong_rebar_size=0; state->zhong_timeline_handle=0; state->zhong_timeline_wait_val=0; state->zhong_timeline_sig_val=0; state->zhong_status=0; state->zhong_done=0; for(int i=0; i<128; i++) state->wrf[i]=_mm512_setzero_si512();
    state->dys_ctx_valid=false;
  } else {
    if (state->prov_strobe) {
      if (state->prov_addr < 0x10) {
        if(state->prov_addr==0x00) state->cell_version=(uint32_t)state->prov_data;
        if(state->prov_addr==0x01) state->cell_resonance_status_ptr=state->prov_data;
        if(state->prov_addr==0x03) state->cell_counter=(uint32_t)state->prov_data;
        if(state->prov_addr==0x0F) state->cell_status=(uint32_t)state->prov_data;
        if(state->prov_addr==0x0D) {
          if((uint32_t)state->prov_data >= state->last_host_epoch) { state->host_epoch=(uint32_t)state->prov_data; state->last_host_epoch=(uint32_t)state->prov_data; state->epoch_error=0; }
          else state->epoch_error=1;
        }
        if(state->prov_addr==0x0E) state->plugin_epoch=(uint32_t)state->prov_data;
        if(state->prov_addr>=0x05 && state->prov_addr<=0x0C) {
           uint64_t *l = (uint64_t*)&state->cell_logic_epoch_handle; l[state->prov_addr-0x05] = state->prov_data;
        }
      } else if (state->prov_addr >= 0x20 && state->prov_addr < 0x30) {
        if(state->prov_addr==0x20) state->log_stdin_head=(uint32_t)state->prov_data;
        if(state->prov_addr==0x21) state->log_stdin_tail=(uint32_t)state->prov_data;
        if(state->prov_addr==0x22) state->log_stdin_lock=(uint32_t)state->prov_data;
        if(state->prov_addr==0x23) state->log_stdin_ptr=state->prov_data;
        if(state->prov_addr==0x24) state->log_stdout_head=(uint32_t)state->prov_data;
        if(state->prov_addr==0x25) state->log_stdout_tail=(uint32_t)state->prov_data;
        if(state->prov_addr==0x26) state->log_stdout_lock=(uint32_t)state->prov_data;
        if(state->prov_addr==0x27) state->log_stdout_ptr=state->prov_data;
      } else if (state->prov_addr >= 0x80) {
        int r = state->prov_addr & 0x7F; int s = (state->prov_data >> 56) & 7;
        ((uint64_t*)&state->wrf[r])[s] = state->prov_data & 0x00FFFFFFFFFFFFFFULL;
        if (r == state->dys_ctx_mod_slot) state->dys_ctx_valid = false;
      }
    }
    if (state->val_strobe) state->log_stdout_valid_head = state->log_stdout_head;
    if (state->wave_instr_strobe) {
      if (state->wave_instr_op == 1) { // VADDPS
        for(int i=0; i<8; i++) state->wrf[state->wave_instr_dest*8+i] = (__m512i)_mm512_add_ps((__m512)state->wrf[state->wave_instr_src1*8+i], (__m512)state->wrf[state->wave_instr_src2*8+i]);
      } else if (state->wave_instr_op == 2) { // VMULPS
        for(int i=0; i<8; i++) state->wrf[state->wave_instr_dest*8+i] = (__m512i)_mm512_mul_ps((__m512)state->wrf[state->wave_instr_src1*8+i], (__m512)state->wrf[state->wave_instr_src2*8+i]);
      } else if (state->wave_instr_op == 0x11) { // VDYS_MODPOW
        TSFiBigInt *res_ptrs[8], *base_ptrs[8], *exp_ptrs[8], *mod_ptrs[8];
        TSFiBigInt b_objs[8], e_objs[8], m_objs[8], r_objs[8];
        for(int i=0; i<8; i++) {
          res_ptrs[i]=&r_objs[i]; base_ptrs[i]=&b_objs[i]; exp_ptrs[i]=&e_objs[i]; mod_ptrs[i]=&m_objs[i];
          memset(&b_objs[i], 0, sizeof(TSFiBigInt)); b_objs[i].limbs[0]=((uint64_t*)&state->wrf[state->wave_instr_src1])[i]; b_objs[i].active_limbs=1;
          memset(&e_objs[i], 0, sizeof(TSFiBigInt)); e_objs[i].limbs[0]=((uint64_t*)&state->wrf[state->wave_instr_src2])[i]; e_objs[i].active_limbs=1;
          memset(&m_objs[i], 0, sizeof(TSFiBigInt)); m_objs[i].limbs[0]=((uint64_t*)&state->wrf[state->wave_instr_src3])[i]; m_objs[i].active_limbs=1;
        }
        if (!state->dys_ctx_valid || state->dys_ctx_mod_slot != state->wave_instr_src3) {
          tsfi_bn_mod_setup_batch8(&state->dys_ctx, mod_ptrs);
          state->dys_ctx_mod_slot = state->wave_instr_src3; state->dys_ctx_valid = true;
        }
        tsfi_bn_modpow_batch8_ctx(res_ptrs, base_ptrs, exp_ptrs, mod_ptrs, &state->dys_ctx);
        for(int i=0; i<8; i++) ((uint64_t*)&state->wrf[state->wave_instr_dest])[i] = r_objs[i].limbs[0];
      }
      state->wave_instr_done = 1;
    } else state->wave_instr_done = 0;
    if (state->directive_strobe) {
      state->cell_counter++; state->last_directive = state->directive_cmd;
      if (state->directive_cmd == 0x47454D49 || state->directive_cmd == 0x48454C4D) {
        state->periph_directive_cmd = state->directive_cmd; state->periph_directive_strobe = 1;
        if (state->directive_cmd == 0x47454D49) state->spawn_gemini_strobe = 1;
      } else { state->periph_directive_strobe = 0; state->spawn_gemini_strobe = 0; }
    } else { state->periph_directive_strobe = 0; state->spawn_gemini_strobe = 0; }
    if (state->exec_strobe && state->exec_handle) { ((void(*)(void*))state->exec_handle)((void*)state->exec_context); state->exec_done=1; } else state->exec_done=0;
    state->cell_status = (state->cell_status & ~1u) | (state->periph_state_ready ? 1u : 0u);
    state->sys_version = state->cell_version; state->sys_counter = state->cell_counter;
  }
}
