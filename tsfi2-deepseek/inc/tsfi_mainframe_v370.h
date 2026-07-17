#ifndef TSFI_MAINFRAME_V370_H
#define TSFI_MAINFRAME_V370_H

#include <stdint.h>
#include <stddef.h>
#include "tsfi_ramac_layout.h"



// VM/370 CP Terminal Options Configurator
typedef struct {
    char chardel_char;
    char linedel_char;
    int is_chardel_enabled;
    int is_linedel_enabled;
} tsfi_cp_terminal_options;

void tsfi_cp_term_opts_init(tsfi_cp_terminal_options *opts);
int tsfi_cp_term_opts_config(tsfi_cp_terminal_options *opts, const char *param, int enable);
void tsfi_cp_term_opts_process(const tsfi_cp_terminal_options *opts, const char *in_buf, char *out_buf, int out_max);

// VM/370 CP Minidisk Link Manager
typedef struct {
    char owner_uid[16];
    uint32_t owner_vdev;
    char read_write_pwd[16];
} tsfi_cp_minidisk;

typedef struct {
    char target_uid[16];
    uint32_t my_vdev;
    int has_write_access;
} tsfi_cp_minidisk_link;

#define MAX_MINIDISKS 8
#define MAX_MINIDISK_LINKS 8
typedef struct {
    tsfi_cp_minidisk disks[MAX_MINIDISKS];
    int disk_count;
    tsfi_cp_minidisk_link links[MAX_MINIDISK_LINKS];
    int link_count;
} tsfi_cp_link_manager;

void tsfi_cp_link_init(tsfi_cp_link_manager *mgr);
int tsfi_cp_link_register(tsfi_cp_link_manager *mgr, const char *owner, uint32_t vdev, const char *pwd);
int tsfi_cp_link_execute(tsfi_cp_link_manager *mgr, const char *requester, const char *owner, uint32_t owner_vdev, uint32_t my_vdev, const char *mode, const char *provided_pwd);

int tsfi_cp_query_disk(const tsfi_cp_link_manager *mgr, char *out_buf, int out_max);

// VM/370 CP Dynamic Device Definer
typedef struct {
    uint32_t vdev;
    char dev_type[8];
} tsfi_cp_virtual_device;

#define MAX_DYN_DEVICES 8
typedef struct {
    tsfi_cp_virtual_device devices[MAX_DYN_DEVICES];
    int count;
} tsfi_cp_device_table;

void tsfi_cp_device_table_init(tsfi_cp_device_table *tbl);
int tsfi_cp_device_define(tsfi_cp_device_table *tbl, const char *type, uint32_t vdev);

int tsfi_cp_query_virtual(const tsfi_cp_device_table *tbl, char *out_buf, int out_max);

void tsfi_cp_logout_execute(tsfi_cp_device_table *tbl, tsfi_cp_link_manager *link_mgr, tsfi_cp_terminal_sleep *sleep_state);

// VM/370 CP Spool Card Punch Queue
typedef struct {
    char data[80];
} tsfi_cp_punch_card;

#define MAX_PUNCH_QUEUE 8
typedef struct {
    tsfi_cp_punch_card cards[MAX_PUNCH_QUEUE];
    int card_count;
    int is_held;
} tsfi_cp_punch_spooler;

void tsfi_cp_punch_spooler_init(tsfi_cp_punch_spooler *spl);
int tsfi_cp_punch_write(tsfi_cp_punch_spooler *spl, const char *card_data);
int tsfi_cp_punch_set_hold(tsfi_cp_punch_spooler *spl, int hold);
int tsfi_cp_punch_flush(tsfi_cp_punch_spooler *spl, int *cards_flushed);

// CMS JCL FILEDEF Dataset Binder
typedef struct {
    char ddname[16];
    char dsname[64];
    char device_class[8];
} tsfi_cms_filedef;

#define MAX_FILEDEFS 8
typedef struct {
    tsfi_cms_filedef entries[MAX_FILEDEFS];
    int count;
} tsfi_cms_filedef_table;

void tsfi_cms_filedef_init(tsfi_cms_filedef_table *tbl);
int tsfi_cms_filedef_bind(tsfi_cms_filedef_table *tbl, const char *ddname, const char *dsname, const char *device_class);
int tsfi_cms_filedef_resolve(const tsfi_cms_filedef_table *tbl, const char *ddname, char *out_dsname, int max_len);

// VM/370 RSCS Network Node Spool Linker
typedef struct {
    char node_name[16];
    int is_active;
    int routed_files;
} tsfi_rscs_node;

#define MAX_RSCS_NODES 8
typedef struct {
    tsfi_rscs_node nodes[MAX_RSCS_NODES];
    int node_count;
} tsfi_rscs_manager;

void tsfi_rscs_init(tsfi_rscs_manager *mgr);
int tsfi_rscs_add_node(tsfi_rscs_manager *mgr, const char *name);
int tsfi_rscs_route_spool(tsfi_rscs_manager *mgr, const char *target_node, int spool_file_id);
int tsfi_rscs_deactivate_node(tsfi_rscs_manager *mgr, const char *name);

// VM/370 CP Forms Control Buffer (FCB) Spool Filter
typedef struct {
    char fcb_name[16];
    int page_length_lines;
    int channel_stops[12];
} tsfi_cp_fcb;

void tsfi_cp_fcb_init(tsfi_cp_fcb *fcb);
int tsfi_cp_fcb_load(tsfi_cp_fcb *fcb, const char *name, int page_len);
int tsfi_cp_fcb_set_channel(tsfi_cp_fcb *fcb, int channel, int line);

// VM/370 CP Spool Class Router
typedef struct {
    char device_name[16];
    char active_class;
} tsfi_cp_spool_device_class;

#define MAX_SPOOL_CLASSES 8
typedef struct {
    tsfi_cp_spool_device_class filters[MAX_SPOOL_CLASSES];
    int count;
} tsfi_cp_spool_class_router;

void tsfi_cp_spool_router_init(tsfi_cp_spool_class_router *router);
int tsfi_cp_spool_router_set(tsfi_cp_spool_class_router *router, const char *dev_name, char class_char);
int tsfi_cp_spool_router_match(const tsfi_cp_spool_class_router *router, const char *dev_name, char file_class);

int tsfi_cp_spool_transfer(tsfi_cp_spool_queue_v3 *src_q, tsfi_cp_spool_queue_v3 *dst_q, int file_id);

// VM/370 Release 3 Discontiguous Shared Segment (DCSS) Manager
typedef struct {
    char segment_name[16];
    uint32_t start_address;
    uint32_t end_address;
    int is_loaded;
} tsfi_cp_dcss_segment;

#define MAX_DCSS_SEGMENTS 8
typedef struct {
    tsfi_cp_dcss_segment segments[MAX_DCSS_SEGMENTS];
    int count;
} tsfi_cp_dcss_manager;

void tsfi_cp_dcss_init(tsfi_cp_dcss_manager *mgr);
int tsfi_cp_dcss_register(tsfi_cp_dcss_manager *mgr, const char *name, uint32_t start_addr, uint32_t end_addr);
int tsfi_cp_dcss_diagnose_find(const tsfi_cp_dcss_manager *mgr, const char *name);
int tsfi_cp_dcss_diagnose_load(tsfi_cp_dcss_manager *mgr, const char *name);
int tsfi_cp_dcss_diagnose_purge(tsfi_cp_dcss_manager *mgr, const char *name);

// CMS VSAM Indexed Access Simulator
typedef struct {
    char key[32];
    char val[64];
} tsfi_vsam_record;

#define MAX_VSAM_RECORDS 16
typedef struct {
    tsfi_vsam_record records[MAX_VSAM_RECORDS];
    int count;
    int is_opened;
} tsfi_vsam_file;

void tsfi_vsam_init(tsfi_vsam_file *file);
int tsfi_vsam_open(tsfi_vsam_file *file);
int tsfi_vsam_close(tsfi_vsam_file *file);
int tsfi_vsam_put(tsfi_vsam_file *file, const char *key, const char *val);
int tsfi_vsam_get(const tsfi_vsam_file *file, const char *key, char *out_val, int max_len);

// VM/370 Release 3 Virtual Machine Assist (VMA) Intercept Controller
typedef struct {
    int assist_enabled;
    int software_intercepts;
    int assisted_instructions;
} tsfi_cp_vma_controller;

void tsfi_cp_vma_init(tsfi_cp_vma_controller *ctrl);
int tsfi_cp_vma_set(tsfi_cp_vma_controller *ctrl, int enable);
int tsfi_cp_vma_execute(tsfi_cp_vma_controller *ctrl, const char *instr_type);

// VM/370 Release 3 APL 3270 Copy Screen Printer Formatter
#define APL_SCREEN_ROWS 24
#define APL_SCREEN_COLS 80
typedef struct {
    char screen_buffer[APL_SCREEN_ROWS][APL_SCREEN_COLS];
} tsfi_cp_apl_screen;

void tsfi_cp_apl_screen_init(tsfi_cp_apl_screen *scr);
int tsfi_cp_apl_screen_write(tsfi_cp_apl_screen *scr, int row, int col, const char *data);
int tsfi_cp_apl_copy_to_printer(const tsfi_cp_apl_screen *scr, tsfi_cp_spool_printer *prt);

// VM/370 Release 3 CP Scheduler Share Configurator
typedef struct {
    char userid[16];
    int is_absolute;
    int value;
} tsfi_cp_share_entry;

#define MAX_SHARE_ENTRIES 8
typedef struct {
    tsfi_cp_share_entry entries[MAX_SHARE_ENTRIES];
    int count;
} tsfi_cp_share_table;

void tsfi_cp_share_init(tsfi_cp_share_table *tbl);
int tsfi_cp_share_set(tsfi_cp_share_table *tbl, const char *uid, int absolute, int val);
int tsfi_cp_share_calculate_slice(const tsfi_cp_share_table *tbl, const char *uid, int total_weight_sum, int *slice_ms);

// VM/370 Release 4 IBM 3850 Mass Storage System (MSS) Sharing Linker
typedef struct {
    char group_name[16];
    char owner_vm[16];
    int is_mounted;
} tsfi_cp_mss_group;

#define MAX_MSS_GROUPS 4
typedef struct {
    tsfi_cp_mss_group groups[MAX_MSS_GROUPS];
    int count;
} tsfi_cp_mss_manager;

void tsfi_cp_mss_init(tsfi_cp_mss_manager *mgr);
int tsfi_cp_mss_register_group(tsfi_cp_mss_manager *mgr, const char *group_name);
int tsfi_cp_mss_mount(tsfi_cp_mss_manager *mgr, const char *group_name, const char *vm_name);
int tsfi_cp_mss_unmount(tsfi_cp_mss_manager *mgr, const char *group_name, const char *vm_name);
int tsfi_cp_mss_query(const tsfi_cp_mss_manager *mgr, const char *group_name, char *out_owner, int max_len);

// VM/370 Release 4 CP IUCV Multi-Path Connector
typedef struct {
    int path_id;
    char target_userid[16];
    int is_active;
} tsfi_cp_iucv_path;

#define MAX_IUCV_PATHS 8
typedef struct {
    tsfi_cp_iucv_path paths[MAX_IUCV_PATHS];
    int count;
} tsfi_cp_iucv_multipath;

void tsfi_cp_iucv_mp_init(tsfi_cp_iucv_multipath *mp);
int tsfi_cp_iucv_mp_connect(tsfi_cp_iucv_multipath *mp, int path_id, const char *target);
int tsfi_cp_iucv_mp_send(const tsfi_cp_iucv_multipath *mp, int path_id, const char *msg, char *out_target);
int tsfi_cp_iucv_mp_disconnect(tsfi_cp_iucv_multipath *mp, int path_id);

// VM/370 Release 5 CMS Command Resolution Engine
#define MAX_NUCLEUS_EXTENSIONS 4
typedef struct {
    char cmd_name[16];
    uint32_t entry_point;
} tsfi_cp_nucleus_ext;

typedef struct {
    tsfi_cp_nucleus_ext extensions[MAX_NUCLEUS_EXTENSIONS];
    int count;
} tsfi_cp_nucleus_table;

void tsfi_cp_nucleus_init(tsfi_cp_nucleus_table *tbl);
int tsfi_cp_nucleus_register(tsfi_cp_nucleus_table *tbl, const char *name, uint32_t entry_addr);
int tsfi_cp_resolve_command(const tsfi_cp_nucleus_table *nuc_tbl, const tsfi_cp_dcss_manager *dcss_mgr, const char *cmd, char *out_loc, uint32_t *out_addr);

// VM/370 Release 5 CMS Command Parameter List (PLIST) Parser
#define MAX_PLIST_WORDS 8
typedef struct {
    char words[MAX_PLIST_WORDS][8];
    int count;
    uint64_t sentinel;
} tsfi_cms_plist;

void tsfi_cms_plist_init(tsfi_cms_plist *plist);
int tsfi_cms_plist_parse(tsfi_cms_plist *plist, const char *cmd_line);

// VM/370 Release 5 CMS Virtual Tape Utility Manager
#define MAX_TAPE_BLOCKS 32
typedef struct {
    char data[80];
    int is_tape_mark;
} tsfi_cms_tape_block;

typedef struct {
    tsfi_cms_tape_block blocks[MAX_TAPE_BLOCKS];
    int block_count;
    int current_pointer;
} tsfi_cms_tape_drive;

void tsfi_cms_tape_init(tsfi_cms_tape_drive *tape);
int tsfi_cms_tape_write_record(tsfi_cms_tape_drive *tape, const char *record);
int tsfi_cms_tape_write_mark(tsfi_cms_tape_drive *tape);
int tsfi_cms_tape_read_record(tsfi_cms_tape_drive *tape, char *out_record, int max_len, int *out_is_mark);
int tsfi_cms_tape_rewind(tsfi_cms_tape_drive *tape);
int tsfi_cms_tape_skip_file(tsfi_cms_tape_drive *tape, int count);

// z/VM Virtual Switch (VSwitch) Controller
#define MAX_VSWITCH_PORTS 8
typedef struct {
    char userid[16];
    uint32_t vdev;
    int is_coupled;
} tsfi_zvm_vswitch_port;

typedef struct {
    char name[16];
    uint32_t rdev;
    tsfi_zvm_vswitch_port ports[MAX_VSWITCH_PORTS];
    int port_count;
    int packet_count;
} tsfi_zvm_vswitch;

#define MAX_VSWITCHES 4
typedef struct {
    tsfi_zvm_vswitch switches[MAX_VSWITCHES];
    int count;
} tsfi_zvm_vswitch_manager;

void tsfi_zvm_vswitch_init(tsfi_zvm_vswitch_manager *mgr);
int tsfi_zvm_vswitch_define(tsfi_zvm_vswitch_manager *mgr, const char *name, uint32_t rdev);
int tsfi_zvm_vswitch_couple(tsfi_zvm_vswitch_manager *mgr, const char *name, const char *userid, uint32_t vdev);
int tsfi_zvm_vswitch_transmit(tsfi_zvm_vswitch_manager *mgr, const char *name, int packets);
int tsfi_zvm_vswitch_query(const tsfi_zvm_vswitch_manager *mgr, const char *name, int *out_ports, int *out_packets);

// Scenario 130: IBM 3031/3032 SIPL Controller
#define MAX_CPUS 4
typedef struct {
    uint32_t cpu_id;
    int pending_interrupts;
    uint32_t last_signal_code;
} tsfi_zvm_cpu_state;

typedef struct {
    tsfi_zvm_cpu_state cpus[MAX_CPUS];
    int cpu_count;
} tsfi_zvm_sipl_controller;

void tsfi_zvm_sipl_init(tsfi_zvm_sipl_controller *ctrl);
int tsfi_zvm_sipl_register_cpu(tsfi_zvm_sipl_controller *ctrl, uint32_t cpu_id);
int tsfi_zvm_sipl_send(tsfi_zvm_sipl_controller *ctrl, uint32_t source_id, uint32_t target_id, uint32_t signal_code);
int tsfi_zvm_sipl_receive(tsfi_zvm_sipl_controller *ctrl, uint32_t cpu_id, uint32_t *out_signal_code);

// Scenario 131: CODASYL DDL Schema Analyzer
#define MAX_DDL_RECORDS 8
#define MAX_DDL_SETS 8

typedef struct {
    char record_name[32];
    int record_len;
} tsfi_codasyl_ddl_record;

typedef struct {
    char set_name[32];
    char owner_record[32];
    char member_record[32];
} tsfi_codasyl_ddl_set;

typedef struct {
    char schema_name[32];
    char area_name[32];
    tsfi_codasyl_ddl_record records[MAX_DDL_RECORDS];
    int record_count;
    tsfi_codasyl_ddl_set sets[MAX_DDL_SETS];
    int set_count;
} tsfi_codasyl_schema;

void tsfi_codasyl_schema_init(tsfi_codasyl_schema *schema);
int tsfi_codasyl_schema_parse(tsfi_codasyl_schema *schema, const char *ddl_statement);
int tsfi_codasyl_schema_validate(const tsfi_codasyl_schema *schema, char *out_error, int max_err_len);

// Scenario 132: IBM Mainframe-to-Minicomputer FEP Red/Black Audit Channel Linker
typedef struct {
    int parity_errors;
    int retry_count;
    uint32_t timing_sector;
    int black_rail_valid;
    int red_rail_valid;
} tsfi_fep_audit_state;

typedef struct {
    char device_id[16];
    tsfi_fep_audit_state audit;
    int total_transactions;
} tsfi_fep_channel;

void tsfi_fep_init(tsfi_fep_channel *chan, const char *device_id);
int tsfi_fep_process_red_rail(tsfi_fep_channel *chan, uint32_t telemetry_data, int parity_bit);
int tsfi_fep_process_black_rail(tsfi_fep_channel *chan, uint32_t timing_sector_input);
int tsfi_fep_query_audit(const tsfi_fep_channel *chan, int *out_transactions, int *out_errors, int *out_timing);

// Scenario 133: Model Context Protocol (MCP) Client Channel Multiplexer
#define MAX_MCP_CHANNELS 4
typedef struct {
    int channel_id;
    char client_name[16];
    int is_active;
    int request_count;
    int security_violations;
} tsfi_mcp_channel_state;

typedef struct {
    tsfi_mcp_channel_state channels[MAX_MCP_CHANNELS];
    int channel_count;
} tsfi_mcp_multiplexer;

void tsfi_mcp_mux_init(tsfi_mcp_multiplexer *mux);
int tsfi_mcp_mux_register(tsfi_mcp_multiplexer *mux, int channel_id, const char *client_name);
int tsfi_mcp_mux_send_request(tsfi_mcp_multiplexer *mux, int channel_id, const char *method, int is_secure_token_valid);
int tsfi_mcp_mux_query(const tsfi_mcp_multiplexer *mux, int channel_id, int *out_requests, int *out_violations);

// Scenario 134: CODASYL DML Execution Engine
typedef struct {
    tsfi_codasyl_schema *schema;
    tsfi_dbtg_currency *currency;
    int mock_records_stored;
} tsfi_codasyl_dml_runtime;

void tsfi_codasyl_dml_init(tsfi_codasyl_dml_runtime *rt, tsfi_codasyl_schema *schema, tsfi_dbtg_currency *currency);
int tsfi_codasyl_dml_execute(tsfi_codasyl_dml_runtime *rt, const char *dml_statement, int *out_db_status);

// Scenario 135: CODASYL 2-3 Tree Distributed Database Controller
#define MAX_TREE_NODES 16
typedef struct {
    int key;
    char dat_bin_path[64];
    int is_mounted;
    char mounted_client_ip[32];
} tsfi_zvm_23_node;

typedef struct {
    tsfi_zvm_23_node nodes[MAX_TREE_NODES];
    int node_count;
} tsfi_zvm_23_tree;

void tsfi_zvm_23_tree_init(tsfi_zvm_23_tree *tree);
int tsfi_zvm_23_tree_add_node(tsfi_zvm_23_tree *tree, int key, const char *dat_bin_path);
int tsfi_zvm_23_tree_mount(tsfi_zvm_23_tree *tree, int key, const char *client_ip);
int tsfi_zvm_23_tree_call(tsfi_zvm_23_tree *tree, int key, const char *client_ip, const char *dml_statement, int *out_db_status);

int tsfi_codasyl_to_relational_translate(const tsfi_codasyl_schema *schema, char *out_sql, int max_len);

// Scenario 137: CODASYL Schema Administration Audit Trail Manager
#define MAX_AUDIT_LOGS 16
typedef struct {
    char op[16];
    char elem_name[32];
    uint32_t hash_before;
    uint32_t hash_after;
} tsfi_schema_audit_entry;

typedef struct {
    tsfi_schema_audit_entry entries[MAX_AUDIT_LOGS];
    int count;
    uint32_t running_checksum;
} tsfi_schema_audit_tracker;

void tsfi_schema_audit_init(tsfi_schema_audit_tracker *tracker);
int tsfi_schema_audit_log(tsfi_schema_audit_tracker *tracker, const char *op, const char *elem_name, uint32_t hash_before);
uint32_t tsfi_schema_audit_checksum(const tsfi_schema_audit_tracker *tracker);

// Scenario 138: CODASYL Subschema Authorization and Area Privilege Auditor
#define MAX_PRIVILEGE_RULES 8
#define PRIV_READ   0x01
#define PRIV_WRITE  0x02

typedef struct {
    char subschema_name[32];
    char target_element[32];
    int allowed_mask;
} tsfi_subschema_rule;

typedef struct {
    tsfi_subschema_rule rules[MAX_PRIVILEGE_RULES];
    int rule_count;
    int denied_attempts;
    int authorized_attempts;
} tsfi_subschema_auditor;

void tsfi_subschema_audit_init(tsfi_subschema_auditor *auditor);
int tsfi_subschema_add_rule(tsfi_subschema_auditor *auditor, const char *subschema, const char *element, int allowed_mask);
int tsfi_subschema_authorize(tsfi_subschema_auditor *auditor, const char *subschema, const char *element, int priv_mask, int *out_authorized);

// Scenario 139: CODASYL Database Transaction Recovery and Rollback Auditor
typedef struct {
    char record_name[32];
    char before_image[128];
    char after_image[128];
    int is_active;
    int rollback_count;
    int commit_count;
} tsfi_db_tx_manager;

void tsfi_db_tx_init(tsfi_db_tx_manager *mgr);
int tsfi_db_tx_begin(tsfi_db_tx_manager *mgr, const char *record_name, const char *initial_data);
int tsfi_db_tx_update(tsfi_db_tx_manager *mgr, const char *new_data);
int tsfi_db_tx_rollback(tsfi_db_tx_manager *mgr, char *out_restored_data, int *out_db_status);
int tsfi_db_tx_commit(tsfi_db_tx_manager *mgr, int *out_db_status);

// Scenario 140: Two-Phase Commit (2PC) Protocol Coordinator
#define MAX_2PC_PARTICIPANTS 4
typedef struct {
    int node_key;
    int prepared;
} tsfi_2pc_participant;

typedef struct {
    tsfi_2pc_participant participants[MAX_2PC_PARTICIPANTS];
    int participant_count;
    int state;
} tsfi_2pc_coordinator;

void tsfi_2pc_init(tsfi_2pc_coordinator *coord);
int tsfi_2pc_join(tsfi_2pc_coordinator *coord, int node_key);
int tsfi_2pc_prepare(tsfi_2pc_coordinator *coord);
int tsfi_2pc_commit(tsfi_2pc_coordinator *coord, int *out_db_status);
int tsfi_2pc_abort(tsfi_2pc_coordinator *coord);

// Scenario 141: IBM System/38 Single-Level Store and Logical Access Path Manager
#define MAX_S38_OBJECTS 16
#define MAX_S38_RECORDS 32

typedef struct {
    uint64_t object_address;
    char object_name[32];
    int is_logical;
    char parent_pf_name[32];
    char key_field[16];
} tsfi_s38_object;

typedef struct {
    char data[64];
    int key_val;
} tsfi_s38_record;

typedef struct {
    tsfi_s38_object objects[MAX_S38_OBJECTS];
    int object_count;
    tsfi_s38_record physical_records[MAX_S38_RECORDS];
    int pf_record_count;
} tsfi_s38_store;

void tsfi_s38_store_init(tsfi_s38_store *store);
int tsfi_s38_create_object(tsfi_s38_store *store, uint64_t addr, const char *name, int is_logical, const char *parent_pf, const char *key_field);
int tsfi_s38_insert_physical(tsfi_s38_store *store, const char *pf_name, const char *data, int key_val);
int tsfi_s38_query_logical_path(const tsfi_s38_store *store, const char *lf_name, int *out_keys, int max_keys);

// Scenario 142: IBM 3848 Cryptographic Subsystem Simulation
typedef struct {
    uint64_t master_key;
    int is_key_loaded;
} tsfi_crypto_subsystem;

void tsfi_crypto_init(tsfi_crypto_subsystem *crypto);
int tsfi_crypto_load_master_key(tsfi_crypto_subsystem *crypto, uint64_t master_key);
int tsfi_crypto_encrypt(tsfi_crypto_subsystem *crypto, const uint8_t *plain, uint8_t *cipher, int supervisor_state);
int tsfi_crypto_decrypt(tsfi_crypto_subsystem *crypto, const uint8_t *cipher, uint8_t *plain, int supervisor_state);

// Scenario 143: NBS FIPS PUB 60 Standard I/O Channel Interface
typedef struct tsfi_fips60_interface {
    uint8_t bus_out_command;
    uint8_t bus_in_status;
    int command_pending;
    int status_updated;
} tsfi_fips60_interface;

void tsfi_fips60_init(tsfi_fips60_interface *fips);
int tsfi_fips60_bus_out(tsfi_fips60_interface *fips, uint8_t command_byte, const uint8_t *data, uint16_t data_len);
int tsfi_fips60_status_in(const tsfi_fips60_interface *fips, uint8_t *out_status);

// Scenario 144: NBS FIPS PUB 54 Computer Output Microform (COM) Spool Formatter
#define MAX_COM_INDEX_ENTRIES 32
typedef struct {
    char key[16];
    int frame_number;
} tsfi_com_index_entry;

typedef struct {
    int current_frame;
    int line_count;
    tsfi_com_index_entry index_table[MAX_COM_INDEX_ENTRIES];
    int index_count;
} tsfi_com_formatter;

void tsfi_com_init(tsfi_com_formatter *fmt);
int tsfi_com_format_record(tsfi_com_formatter *fmt, const char *record_text, uint8_t *out_frame, uint16_t *out_len);
int tsfi_com_generate_index_frame(const tsfi_com_formatter *fmt, char *out_index_data, int max_len);

#endif // TSFI_MAINFRAME_V370_H
