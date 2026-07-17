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
int tsfi_mf_wessler_macro_expand(const char *macro_line, char *hlasm_out, int max_len);
int tsfi_mf_majordomo_restrict_post(const char *sender_email, const char **members, int member_count);
int tsfi_mf_majordomo_info(const char *list_name, char *info_out, int max_len);
int tsfi_mf_majordomo_help(char *help_out, int max_len);
int tsfi_mf_majordomo_lists(char *lists_out, int max_len);
int tsfi_mf_majordomo_intro(const char *list_name, char *intro_out, int max_len);
int tsfi_mf_majordomo_parse_config(const char *config_content, const char *key, char *val_out, int max_len);
int tsfi_mf_majordomo_unsubscribe(const char *list_name, const char *email, const char **members, int *member_count);
int tsfi_mf_majordomo_moderate(uint32_t cookie, const char *action, char *verdict_out, int max_len);
int tsfi_mf_majordomo_gen_cookie(const char *email, const char *list_name, uint32_t salt, char *cookie_out, int max_len);
int tsfi_mf_majordomo_queue_post(const char *list_name, const char *sender, const char *subject, uint32_t *pending_cookies, int *cookie_count, int max_cookies);
int tsfi_mf_majordomo_is_admin(const char *list_name, const char *email, const char **admin_registry, int admin_count);
int tsfi_mf_majordomo_check_password(const char *list_name, const char *password, const char *stored_hash_or_pass);
int tsfi_mf_majordomo_write_config(const char *key, const char *val, char *config_in_out, int max_len);
int tsfi_mf_cics_suspend(uint32_t task_id, uint32_t *suspend_log, int *log_count, int max_log);
int tsfi_mf_cics_resume(uint32_t task_id, uint32_t *suspend_log, int *log_count);
int tsfi_mf_majordomo_inject_mod_headers(const char *sender, const char *list_name, uint32_t cookie, char *msg_out, int max_len);
int tsfi_mf_majordomo_config_defaults(const char *list_name, char *defaults_out, int max_len);
int tsfi_mf_majordomo_welcome(const char *list_name, const char *owner_email, char *welcome_out, int max_len);
int tsfi_mf_majordomo_hash_password(const char *password, char *hash_out, int max_len);
int tsfi_mf_majordomo_discard(uint32_t cookie, uint32_t *pending_cookies, int *cookie_count);
int tsfi_mf_majordomo_validate_config(const char *config_content, int *errors_found);
int tsfi_mf_majordomo_update_info(const char *list_name, const char *new_info, char *info_store, int max_store);
int tsfi_mf_majordomo_archive_index(const char *list_name, const char **archive_files, int archive_count, char *index_out, int max_len);
int tsfi_mf_majordomo_archive_get(const char *list_name, const char *file_name, const char **archive_files, const char **archive_contents, int archive_count, char *content_out, int max_len);
int tsfi_mf_cics_delay(uint32_t seconds, uint32_t *delay_counter_log);
int tsfi_mf_cics_query_abend(int registry_active, int *is_active_out);
int tsfi_mf_cics_reset_abend(int *registry_active);
int tsfi_mf_cics_handle_abend_program(const char *program_name, char *abend_program_registry, int max_name);
int tsfi_mf_cics_handle_pmg_hit_abend(uint32_t player_id, int hit_type, int *health_in_out, float *x_out, float *y_out, char *action_log, int max_log);
int tsfi_mf_cics_handle_abend(uint64_t handler_addr, uint64_t *abend_handler_registry, int *registry_active);
int tsfi_mf_cics_cancel_abend(uint64_t *abend_handler_registry, int *registry_active);
int tsfi_mf_cics_release(const char *resource_name, uint8_t *storage_pool, uint32_t *allocated_offset);
int tsfi_mf_cics_load(const char *resource_name, uint32_t *loaded_address, uint8_t *storage_pool, uint32_t *allocated_offset);
int tsfi_mf_cics_abend(const char *abend_code, char *abend_log_buffer, int max_len);
int tsfi_mf_cics_return(const char *next_transaction_id, const uint8_t *commarea, int commarea_len, char *return_status_log, int max_log);
int tsfi_mf_majordomo_get_pending(const char *list_name, const uint32_t *pending_cookies, int cookie_count, uint32_t *out_cookies, int *out_count, int max_out);
int tsfi_mf_cics_bypass_security(const char *transaction_id, uint32_t auth_token, int *bypass_status);
int tsfi_mf_cics_getmain_shared(int length, uint8_t *storage_pool, uint32_t *allocated_offset, uint32_t *shared_registry, int *registry_count, int max_registry);
int tsfi_mf_cics_cwa_write(uint32_t offset, const uint8_t *data, int len, uint8_t *cwa_pool);
int tsfi_mf_cics_cwa_read(uint32_t offset, uint8_t *data_out, int len, const uint8_t *cwa_pool);
int tsfi_mf_cics_dump(const char *dump_id, uint32_t code, const uint8_t *payload, int len, char *dump_out, int max_len);
int tsfi_mf_cics_query_security(const char *user_id, const char *resource_name, const char *access_type, int *auth_verdict);
int tsfi_mf_majordomo_which(const char *email, const char **lists, int list_count, char *out_buffer, int max_len);
int tsfi_mf_cics_vsam_read(const char *dataset_name, const char *record_key, char *record_out, int max_len);
int tsfi_mf_cics_vsam_write(const char *dataset_name, const char *record_key, const char *record_data);
int tsfi_mf_cics_writeq_td(const char *queue_name, const char *log_message, char *queue_pool, int *queue_count, int max_entries);
int tsfi_mf_pmg_log_trajectory(uint8_t missile_id, float vx, float vy, char *queue_pool, int *queue_count, int max_entries);
int tsfi_mf_cics_isc_route(const char *target_system, const char *transaction_id, const uint8_t *payload, int payload_len, char *route_log_buffer, int max_log_len);
int tsfi_mf_gauntlet_btc_transaction(const char *player_id, uint32_t amount_sats, int32_t *health_in_out, uint32_t *keys_in_out, char *tx_out, int max_tx_len);
int tsfi_mf_vulkan_zmachine_dispatch_shader(uint32_t room_id, const float *vertex_coords, int vertex_count, char *shader_log, int max_log_len);
int tsfi_mf_cics_start_task(const char *trans_id, uint32_t delay_seconds, const uint8_t *data, int data_len, uint32_t *active_tasks_count);
int tsfi_mf_zmachine_vulkan_map_render(uint32_t active_room_id, const uint32_t *connections, int conn_count, char *terminal_buffer);
int tsfi_mf_cics_handle_condition(uint32_t condition_id, uint64_t handler_addr, uint64_t *registry_table, int max_conditions);
int tsfi_mf_cics_raise_condition(uint32_t condition_id, uint64_t *registry_table, int max_conditions, uint64_t *handler_out);
int tsfi_mf_set_agent_handle(const char *handle);
const char *tsfi_mf_get_agent_handle(void);
int tsfi_mf_cics_link(const char *program_name, const uint8_t *commarea, int commarea_len, uint8_t *storage_pool, uint32_t *allocated_offset);
int tsfi_mf_vulkan_appc_layout(uint32_t transaction_id, uint32_t state_flags, char *terminal_buffer);
int tsfi_mf_pmg_handle_collision(uint8_t player_id, uint8_t playfield_id, uint8_t *storage_pool, uint32_t *allocated_offset);
int tsfi_mf_cics_freemain(uint32_t offset, uint32_t length, uint8_t *storage_pool);

#endif // TSFI_MICRO_FOCUS_H
