#ifndef TSFI_MICRO_FOCUS_H
#define TSFI_MICRO_FOCUS_H

#include <stdint.h>
#include <stddef.h>

// 1. COMP-5 Native Binary Format Encoders/Decoders
int tsfi_mf_comp5_encode(int64_t value, uint8_t *buffer, int size_bytes);
int64_t tsfi_mf_comp5_decode(const uint8_t *buffer, int size_bytes, int is_signed);

// 2. ISAM (Indexed Sequential Access Method) Reader/Writer (Supports only .dat.bin extension per Rule 13)
int tsfi_mf_isam_write_record(const char *filepath_dat_bin, uint32_t key, const uint8_t *record_data, int record_len);
int tsfi_mf_isam_read_record(const char *filepath_dat_bin, uint32_t key, uint8_t *record_out, int record_len);

// 3. SIF (Standard Interchange Format) CAD Reader
#include "tsfi_ray_tracer.h"
int tsfi_mf_sif_parse(const char *sif_line, tsfi_cgm_scene *scene);
int tsfi_mf_sif_serialize(const tsfi_cgm_scene *scene, char *out_buffer, size_t max_len);

// 4. Screen Section Terminal Layout Engine
int tsfi_mf_screen_section_render(const char *screen_def, char *terminal_buffer, int width, int height);

// 5. Michael Coughlan Stock & Sales Processing
int tsfi_mf_stock_inventory_process(const char *raw_record, char *report_line);
int tsfi_mf_sales_commission_process(const char *raw_record, char *report_line);
int tsfi_mf_calculate_diyat_tax(const char *event_code, uint32_t base_value, uint32_t *out_tax);
int tsfi_diyat_yul_excise_gas_taxes(uint64_t gas_amount, const char *user_address_hex);

#include "tsfi_ramac_layout.h"
int tsfi_s370_yul_exec_bridge(const tsfi_uncol_instruction *program, int program_size);

// 6. Winchester SCSI, DAT 2-3 Tree Node Write, and Accumulator Redirection
int tsfi_mf_scsi_winchester_handshake(uint32_t keycode, uint8_t *storage_registers);
int tsfi_mf_dat_write_23_node(const char *filepath_dat_bin, uint32_t node_id, const uint8_t *payload, int len);
int tsfi_mf_redirect_space_charge(uint64_t empirical_power, uint64_t *accumulator_state);
int tsfi_mf_wessler_dsa_link(uint32_t *current_save_area, uint32_t *next_save_area);
int tsfi_mf_cics_translate(const char *cics_statement, char *macro_call_out, int max_len);
int tsfi_mf_cics_getmain(int length, const char *suspend_type, uint8_t *storage_pool, uint32_t *allocated_offset);
int tsfi_mf_cics_bms_cad_render(const char *map_name, const tsfi_cgm_scene *scene, char *terminal_buffer);
int tsfi_mf_cics_bms_marauder_map(const char *map_name, const uint32_t *active_node_coordinates, int node_count, char *terminal_buffer);
int tsfi_mf_cics_bms_first_person_render(float player_x, float player_y, float player_angle, char *terminal_buffer);
int tsfi_mf_cics_bms_pmg_render(const uint8_t *pmg_base, int player_x, int player_y, char *terminal_buffer);
int tsfi_mf_cics_enq(const char *resource_name, uint32_t task_id, uint32_t *lock_table, int max_locks);
int tsfi_mf_cics_deq(const char *resource_name, uint32_t task_id, uint32_t *lock_table, int max_locks);
int tsfi_mf_zmachine_bms_room_render(uint32_t room_id, float player_angle, char *terminal_buffer);
int tsfi_mf_majordomo_process(const char *cmd_line, char *out_response, int max_len);
int tsfi_mf_majordomo_approve(uint32_t cookie, const char *action, uint32_t *pending_cookies, int max_pending);
int tsfi_mf_majordomo_digest(const char **posts, int post_count, char *digest_out, int max_len);

#endif // TSFI_MICRO_FOCUS_H
