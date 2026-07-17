#ifndef TSFI_MAINFRAME_DECNET_H
#define TSFI_MAINFRAME_DECNET_H

#include <stdint.h>
#include <stddef.h>
#include <string.h>

// DECnet Phase II Distributed Routing
typedef struct {
    uint16_t node_id;
    uint16_t next_hop;
    uint16_t cost;
    int age;
} tsfi_decnet_route;

typedef struct {
    uint16_t local_node_id;
    tsfi_decnet_route routing_table[32];
    int route_count;
} tsfi_decnet_router;

void tsfi_decnet_init(tsfi_decnet_router *router, uint16_t local_id);
int tsfi_decnet_add_neighbor(tsfi_decnet_router *router, uint16_t neighbor_id, uint16_t cost);
int tsfi_decnet_update_route(tsfi_decnet_router *router, uint16_t dest_id, uint16_t next_hop, uint16_t cost);
int tsfi_decnet_route_packet(tsfi_decnet_router *router, uint16_t dest_id, uint16_t *next_hop_out);

// IBM SDLC Frame Transceiver
typedef struct {
    uint8_t address;
    uint8_t control;
    uint8_t payload[256];
    size_t payload_len;
    uint16_t fcs;
} tsfi_sdlc_frame;

uint16_t tsfi_sdlc_crc16(const uint8_t *data, size_t len);
int tsfi_sdlc_serialize(const tsfi_sdlc_frame *frame, uint8_t *buf_out, size_t *len_out);
int tsfi_sdlc_deserialize(const uint8_t *buf, size_t len, tsfi_sdlc_frame *frame_out);

#define SDLC_CMD_POLL   0x13
#define SDLC_CMD_SELECT 0x23
#define SDLC_CMD_ACK    0x43
#define SDLC_CMD_SNRM   0x83
#define SDLC_CMD_DISC   0x53
#define SDLC_CMD_UA     0x63
#define SDLC_CMD_FRMR   0x97

// Virtual Coaxial Line Hub
typedef struct {
    uint8_t connected_addresses[8];
    int connected_count;
} tsfi_sdlc_hub;

void tsfi_sdlc_hub_init(tsfi_sdlc_hub *hub);
int tsfi_sdlc_hub_connect(tsfi_sdlc_hub *hub, uint8_t addr);
int tsfi_sdlc_hub_broadcast(tsfi_sdlc_hub *hub, const uint8_t *frame_data, size_t len, uint8_t rx_addr, uint8_t *buf_out, size_t *len_out);

// DECnet-over-SDLC Bridge
int tsfi_decnet_bridge_send_update(tsfi_decnet_router *router, uint16_t dest_node, tsfi_sdlc_hub *hub, uint8_t sdlc_addr);
int tsfi_decnet_bridge_receive_update(tsfi_decnet_router *router, const uint8_t *sdlc_frame_data, size_t len);

// SDLC Link State
typedef struct {
    uint8_t ns;
    uint8_t nr;
    int link_established;
} tsfi_sdlc_link_state;

// Primary Polling Interrogation Scheduler
typedef struct {
    uint8_t stations[8];
    int station_count;
    int current_index;
} tsfi_sdlc_scheduler;

void tsfi_sdlc_link_init(tsfi_sdlc_link_state *link);
int tsfi_sdlc_link_handshake(tsfi_sdlc_link_state *primary, tsfi_sdlc_link_state *secondary, uint8_t command);
int tsfi_sdlc_verify_sequence(tsfi_sdlc_link_state *link, uint8_t ns_received, uint8_t nr_received);

void tsfi_sdlc_scheduler_init(tsfi_sdlc_scheduler *sched);
int tsfi_sdlc_scheduler_add(tsfi_sdlc_scheduler *sched, uint8_t addr);
uint8_t tsfi_sdlc_scheduler_next(tsfi_sdlc_scheduler *sched);

// SDLC Frame Reject, T1 Timer, and Half-Duplex Turnaround
typedef struct {
    int timer_active;
    int timeout_count;
} tsfi_sdlc_t1_timer;

typedef struct {
    int tx_enabled;
    int rx_enabled;
    int turnaround_requested;
} tsfi_sdlc_line_mode;

int tsfi_sdlc_generate_frmr(uint8_t invalid_ctrl, uint8_t expected_nr, uint8_t actual_nr, uint8_t *frmr_data_out);
void tsfi_sdlc_timer_start(tsfi_sdlc_t1_timer *timer);
int tsfi_sdlc_timer_tick(tsfi_sdlc_t1_timer *timer, int elapsed_ms, int max_wait_ms);
void tsfi_sdlc_line_init(tsfi_sdlc_line_mode *line);
int tsfi_sdlc_line_turnaround(tsfi_sdlc_line_mode *line);

// DECnet NSP Logical Links
typedef struct {
    uint16_t local_link_id;
    uint16_t remote_link_id;
    int state;
} tsfi_decnet_nsp_link;

void tsfi_decnet_nsp_init(tsfi_decnet_nsp_link *link, uint16_t local_id);
int tsfi_decnet_nsp_connect(tsfi_decnet_nsp_link *local, tsfi_decnet_nsp_link *remote);

// SDLC Group Broadcast
int tsfi_sdlc_hub_broadcast_group(tsfi_sdlc_hub *hub, const uint8_t *frame_data, size_t len, uint8_t rx_addr, uint8_t *buf_out, size_t *len_out);

// DECnet Node Name Directory Resolver
typedef struct {
    char name[16];
    uint16_t node_id;
} tsfi_decnet_node_entry;

typedef struct {
    tsfi_decnet_node_entry entries[16];
    int entry_count;
} tsfi_decnet_directory;

void tsfi_decnet_dir_init(tsfi_decnet_directory *dir);
int tsfi_decnet_dir_add(tsfi_decnet_directory *dir, const char *name, uint16_t node_id);
int tsfi_decnet_dir_resolve(tsfi_decnet_directory *dir, const char *name, uint16_t *node_id_out);

#define SDLC_CMD_RR  0x11
#define SDLC_CMD_RNR 0x15

int tsfi_sdlc_update_buffer_status(tsfi_sdlc_link_state *link, int buffer_busy);
int tsfi_decnet_forward_packet(tsfi_decnet_router *router, uint16_t src, uint16_t dest, uint16_t *next_hop_out);
int tsfi_decnet_exchange_hello(tsfi_decnet_router *local, tsfi_decnet_router *remote, uint16_t link_cost);

// DECnet Routing Header
typedef struct {
    uint8_t flags;
    uint16_t dst_node;
    uint16_t src_node;
    uint8_t forward_count;
} tsfi_decnet_header;

int tsfi_decnet_serialize_header(const tsfi_decnet_header *hdr, uint8_t *buf, size_t *len_out);
int tsfi_decnet_deserialize_header(const uint8_t *buf, size_t len, tsfi_decnet_header *hdr_out);

// SDLC Frame Retransmission Queue
typedef struct {
    tsfi_sdlc_frame frames[4];
    int count;
} tsfi_sdlc_tx_queue;

void tsfi_sdlc_queue_init(tsfi_sdlc_tx_queue *q);
int tsfi_sdlc_queue_push(tsfi_sdlc_tx_queue *q, const tsfi_sdlc_frame *f);
int tsfi_sdlc_queue_ack(tsfi_sdlc_tx_queue *q, uint8_t nr);

// DECnet Routing Age
int tsfi_decnet_age_routes(tsfi_decnet_router *router);

// SDLC Bit-stuffing, abort sequences, and hop count checks
int tsfi_sdlc_bit_stuff(const uint8_t *in, size_t in_len, uint8_t *out, size_t *out_len);
int tsfi_sdlc_bit_unstuff(const uint8_t *in, size_t in_len, uint8_t *out, size_t *out_len);
int tsfi_sdlc_detect_abort(const uint8_t *bit_stream, size_t bit_len);
int tsfi_decnet_hop_check(tsfi_decnet_header *hdr);

// DECnet NSP Segment Credit Flow Control
typedef struct {
    int tx_credits;
    int rx_credits;
} tsfi_decnet_nsp_flow;

void tsfi_decnet_nsp_flow_init(tsfi_decnet_nsp_flow *flow, int initial_credits);
int tsfi_decnet_nsp_flow_consume(tsfi_decnet_nsp_flow *flow);
void tsfi_decnet_nsp_flow_grant(tsfi_decnet_nsp_flow *flow, int credits);

// SDLC Secondary Station Multiplexing
typedef struct {
    uint8_t secondary_addresses[4];
    int count;
} tsfi_sdlc_multiplexer;

void tsfi_sdlc_mux_init(tsfi_sdlc_multiplexer *mux);
int tsfi_sdlc_mux_add(tsfi_sdlc_multiplexer *mux, uint8_t addr);
int tsfi_sdlc_mux_route(tsfi_sdlc_multiplexer *mux, uint8_t addr);

// DECnet NSP Message Segmentation and Reassembly
typedef struct {
    uint8_t buffer[1024];
    size_t len;
    int expected_segments;
    int current_segments;
} tsfi_decnet_reassembler;

void tsfi_decnet_reassembler_init(tsfi_decnet_reassembler *r, int expected);
int tsfi_decnet_reassembler_add(tsfi_decnet_reassembler *r, const uint8_t *segment, size_t seg_len);

#define NSP_MSG_DATA      0x01
#define NSP_MSG_INTERRUPT 0x02
#define NSP_MSG_PROBE     0x03

// NSP Out-of-Order Segment Reordering Queue
typedef struct {
    uint8_t seq_num;
    uint8_t data[256];
    size_t len;
    int valid;
} tsfi_nsp_queue_item;

typedef struct {
    tsfi_nsp_queue_item items[8];
    int count;
} tsfi_nsp_reorder_queue;

int tsfi_decnet_nsp_send_interrupt(tsfi_decnet_nsp_link *link, const uint8_t *data, size_t len, uint8_t *pkt_out, size_t *pkt_len);
void tsfi_nsp_reorder_init(tsfi_nsp_reorder_queue *q);
int tsfi_nsp_reorder_add(tsfi_nsp_reorder_queue *q, uint8_t seq, const uint8_t *data, size_t len);
int tsfi_decnet_nsp_probe(tsfi_decnet_nsp_link *link, int ticks_elapsed, int timeout_ticks);

#define NSP_CMD_CI 0x18
#define NSP_CMD_CC 0x28

// NSP Control Message (CI/CC)
typedef struct {
    uint8_t command;
    uint16_t src_link;
    uint16_t dst_link;
    uint8_t services;
} tsfi_nsp_control_msg;

// NSP Subchannel Allocation
typedef struct {
    uint16_t normal_subchannel;
    uint16_t interrupt_subchannel;
} tsfi_nsp_subchannel;

// NSP Transport Retransmit Timer
typedef struct {
    int active;
    int elapsed;
    int retries;
} tsfi_nsp_retransmit_timer;

int tsfi_nsp_serialize_control(const tsfi_nsp_control_msg *msg, uint8_t *buf, size_t *len_out);
int tsfi_nsp_deserialize_control(const uint8_t *buf, size_t len, tsfi_nsp_control_msg *msg_out);
void tsfi_nsp_retransmit_start(tsfi_nsp_retransmit_timer *timer);
int tsfi_nsp_retransmit_tick(tsfi_nsp_retransmit_timer *timer, int elapsed_ms, int timeout_ms, int max_retries);
void tsfi_nsp_subchannel_init(tsfi_nsp_subchannel *sub, uint16_t link_id);

// IBM 3705 Communications Controller CTC
typedef struct {
    uint8_t command_register;
    uint8_t status_register;
    int interrupt_pending;
} tsfi_ibm3705_ctc;

void tsfi_ibm3705_init(tsfi_ibm3705_ctc *ctc);
int tsfi_ibm3705_write_cmd(tsfi_ibm3705_ctc *ctc, uint8_t cmd);
uint8_t tsfi_ibm3705_read_status(tsfi_ibm3705_ctc *ctc);

// IBM 3270 Visual Terminal Cluster Controller
typedef struct {
    uint8_t screen_buffer[1920];
    int cursor_position;
    int buffer_updated;
} tsfi_ibm3270_terminal;

void tsfi_ibm3270_init(tsfi_ibm3270_terminal *term);
int tsfi_ibm3270_write_char(tsfi_ibm3270_terminal *term, char c, int pos);

// SNA/VTAM Path Information Unit (PIU) Header
typedef struct {
    uint8_t th_fid;
    uint8_t th_daf;
    uint8_t th_saf;
    uint16_t th_snf;
    uint8_t rh_type;
} tsfi_vtam_piu_header;

int tsfi_vtam_serialize_piu(const tsfi_vtam_piu_header *piu, const uint8_t *payload, size_t pay_len, uint8_t *buf_out, size_t *len_out);
int tsfi_vtam_deserialize_piu(const uint8_t *buf, size_t len, tsfi_vtam_piu_header *piu_out, uint8_t *pay_out, size_t *pay_len_out);

// IBM SDLC Loop Mode Station
typedef struct {
    uint8_t loop_address;
    int active_repeater;
    int has_token;
} tsfi_sdlc_loop_station;

void tsfi_sdlc_loop_init(tsfi_sdlc_loop_station *station, uint8_t addr);
int tsfi_sdlc_loop_forward(tsfi_sdlc_loop_station *station, uint8_t *frame_bits, size_t bit_len);

// IBM 3705 Scanner Bit-Sampling Machine
typedef struct {
    int clock_phase;
    int last_bit;
    int bit_synced;
} tsfi_ibm3705_scanner;

void tsfi_ibm3705_scanner_init(tsfi_ibm3705_scanner *scanner);
int tsfi_ibm3705_scanner_sample(tsfi_ibm3705_scanner *scanner, int pin_value);

// SNA Virtual Route and Explicit Route Path engine
typedef struct {
    uint8_t virtual_route_id;
    uint8_t explicit_route_id;
    int transmission_priority;
} tsfi_sna_route;

void tsfi_sna_route_init(tsfi_sna_route *route, uint8_t vr, uint8_t er, int prio);
int tsfi_sna_route_prioritize(const tsfi_sna_route *route, int load_factor);

// IBM 3705 EP/NCP Partition State Machine
typedef struct {
    int active_partition;
    int partition_busy[4];
} tsfi_ibm3705_ncp;

void tsfi_ibm3705_ncp_init(tsfi_ibm3705_ncp *ncp);
int tsfi_ibm3705_ncp_switch(tsfi_ibm3705_ncp *ncp, int partition_id);

// SNA Session BIND/SDT Handshake Engine
typedef struct {
    int session_active;
    int traffic_started;
} tsfi_sna_session;

void tsfi_sna_session_init(tsfi_sna_session *sess);
int tsfi_sna_session_handshake(tsfi_sna_session *sess, uint8_t cmd);

#define SNA_CMD_BIND   0x31
#define SNA_CMD_UNBIND 0x32
#define SNA_CMD_SDT    0x33

// S/370 Channel Status & Sense Byte Registers
typedef struct {
    uint8_t status_byte;
    uint8_t sense_byte;
} tsfi_s370_channel_status;

void tsfi_s370_channel_status_init(tsfi_s370_channel_status *chan);
void tsfi_s370_channel_set_error(tsfi_s370_channel_status *chan, uint8_t sense);

// SNA MSNF CDRM Cross-Domain Session broker (linking btc-rails VM domains)
typedef struct {
    uint16_t local_domain_id;
    uint16_t remote_domain_id;
    int session_state;
} tsfi_msnf_cdrm;

void tsfi_msnf_init(tsfi_msnf_cdrm *cdrm, uint16_t local_id);
int tsfi_msnf_establish_session(tsfi_msnf_cdrm *cdrm, uint16_t remote_id);

// X.25 Packet Switching
typedef struct {
    uint8_t gfi_lci;
    uint8_t packet_type;
    uint8_t payload[256];
    size_t payload_len;
} tsfi_x25_packet;

int tsfi_x25_encapsulate_sdlc(const uint8_t *sdlc_buf, size_t sdlc_len, uint8_t lci, tsfi_x25_packet *x25_out);
int tsfi_x25_decapsulate_sdlc(const tsfi_x25_packet *x25, uint8_t *sdlc_buf_out, size_t *sdlc_len_out);

// IBM 8100 Distributed Information System Terminal
typedef struct {
    uint16_t terminal_id;
    int local_accumulator;
    int sync_pending;
} tsfi_ibm8100_dpcx;

void tsfi_ibm8100_init(tsfi_ibm8100_dpcx *node, uint16_t term_id);
int tsfi_ibm8100_process_local(tsfi_ibm8100_dpcx *node, int val);
int tsfi_ibm8100_sync_host(tsfi_ibm8100_dpcx *node, uint8_t *sync_frame_out, size_t *len_out);

// 2-3 Tree message packet replication over TCP/IP
typedef struct {
    uint32_t key;
    uint32_t value;
    int command_type;
} tsfi_23tree_msg;

typedef struct {
    int socket_fd;
    int connected;
} tsfi_tcp_connection;

int tsfi_msnf_send_tree_op(tsfi_msnf_cdrm *cdrm, tsfi_tcp_connection *conn, const tsfi_23tree_msg *msg, uint8_t *pkt_out, size_t *len_out);
int tsfi_msnf_recv_tree_op(tsfi_msnf_cdrm *cdrm, tsfi_tcp_connection *conn, const uint8_t *pkt, size_t len, tsfi_23tree_msg *msg_out);

// SNA Cryptographic Session (SNA-DES)
typedef struct {
    uint8_t session_key[8];
    int encryption_enabled;
} tsfi_sna_crypto;

void tsfi_sna_crypto_init(tsfi_sna_crypto *crypto, const uint8_t *key);
int tsfi_sna_encrypt(tsfi_sna_crypto *crypto, const uint8_t *plain, size_t len, uint8_t *cipher);
int tsfi_sna_decrypt(tsfi_sna_crypto *crypto, const uint8_t *cipher, size_t len, uint8_t *plain);

// VTAM Network Terminal Option (NTO) teletype translator
typedef struct {
    int active;
} tsfi_vtam_nto;

void tsfi_vtam_nto_init(tsfi_vtam_nto *nto);
int tsfi_vtam_nto_translate(tsfi_vtam_nto *nto, const char *tty_in, size_t len, uint8_t *lu_out, size_t *out_len);

// SNA Explicit Route TG pacing controller
typedef struct {
    int window_size;
    int max_window;
    int congestion_detected;
} tsfi_sna_pacing;

void tsfi_sna_pacing_init(tsfi_sna_pacing *pacing, int initial_window);
int tsfi_sna_pacing_adjust(tsfi_sna_pacing *pacing, int congestion_flag);

#define LU_TYPE_FILE     0x01
#define LU_TYPE_SOCKET   0x02
#define LU_TYPE_TERMINAL 0x03
#define LU_TYPE_DISK     0x04

// Unified SNA Logical Unit (LU)
typedef struct {
    uint8_t lu_type;
    uint16_t lu_address;
    char resource_name[32];
    int active;
} tsfi_vtam_lu;

// SNA LU Registry
typedef struct {
    tsfi_vtam_lu lus[16];
    int count;
} tsfi_vtam_lu_registry;

void tsfi_vtam_lu_registry_init(tsfi_vtam_lu_registry *reg);
int tsfi_vtam_lu_registry_add(tsfi_vtam_lu_registry *reg, uint16_t addr, uint8_t type, const char *name);
int tsfi_vtam_lu_registry_route(tsfi_vtam_lu_registry *reg, uint16_t addr, uint8_t *data, size_t len);
int tsfi_vtam_lu_bridge_winchester(tsfi_vtam_lu_registry *reg, uint16_t addr, uint8_t *scsi_status, uint8_t *data_reg, uint8_t *keycode_reg);
int tsfi_vtam_lu_bridge_coaxial(tsfi_vtam_lu_registry *reg, uint16_t addr, int *coax_phase, int *coax_signal);

// SNA Network Addressable Unit (NAU) Session Manager
typedef struct {
    uint16_t sscp_id;
    uint16_t pu_id;
    uint16_t lu_id;
    int session_active;
} tsfi_nau_session;

void tsfi_nau_session_init(tsfi_nau_session *sess);
int tsfi_nau_session_bind(tsfi_nau_session *sess, uint16_t sscp, uint16_t pu, uint16_t lu);

// IBM 3705 Transmission Group (TG) Sequence Reordering
typedef struct {
    uint16_t expected_tg_seq;
    uint16_t received_tg_seq;
    int out_of_sequence_count;
} tsfi_3705_tg_reorder;

void tsfi_3705_tg_init(tsfi_3705_tg_reorder *tg);
int tsfi_3705_tg_process(tsfi_3705_tg_reorder *tg, uint16_t seq);

// SNA Path Control Explicit Route Route State
typedef struct {
    uint8_t route_number;
    int active;
} tsfi_sna_er_route;

void tsfi_sna_er_init(tsfi_sna_er_route *er, uint8_t route_num);
int tsfi_sna_er_activate(tsfi_sna_er_route *er);
int tsfi_sna_er_deactivate(tsfi_sna_er_route *er);

// Mainframe Connection Visibility status reporter
void tsfi_mainframe_connection_status(const tsfi_msnf_cdrm *cdrm, const tsfi_nau_session *nau, const tsfi_sna_er_route *er, char *report_out, size_t max_len);

// SNA LU-LU Stage Pacing
typedef struct {
    int stage_window;
    int credits_left;
} tsfi_sna_stage_pacing;

void tsfi_sna_stage_pacing_init(tsfi_sna_stage_pacing *pacing, int window);
int tsfi_sna_stage_pacing_consume(tsfi_sna_stage_pacing *pacing);
void tsfi_sna_stage_pacing_response(tsfi_sna_stage_pacing *pacing);

// SNA Function Management Header (FMH) Codec
typedef struct {
    uint8_t fmh_type;
    uint8_t fmh_len;
    uint16_t destination_id;
} tsfi_sna_fmh;

int tsfi_sna_serialize_fmh(const tsfi_sna_fmh *fmh, uint8_t *buf, size_t *len_out);
int tsfi_sna_deserialize_fmh(const uint8_t *buf, size_t len, tsfi_sna_fmh *fmh_out);

// SNA SSCP-LU Service Control Sessions
typedef struct {
    int lu_active;
} tsfi_sscp_lu_session;

void tsfi_sscp_lu_init(tsfi_sscp_lu_session *sess);
int tsfi_sscp_lu_control(tsfi_sscp_lu_session *sess, uint8_t cmd);

#define SNA_CMD_ACTLU  0x0E
#define SNA_CMD_DACTLU 0x0F

#endif // TSFI_MAINFRAME_DECNET_H
