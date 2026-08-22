#ifndef AUNCIENT_NCSA_TELNET_CPMTOMIE_THEOREMS_1946_1950_H
#define AUNCIENT_NCSA_TELNET_CPMTOMIE_THEOREMS_1946_1950_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    TELNET_STATE_DATA = 0,
    TELNET_STATE_IAC = 1,
    TELNET_STATE_WILL = 2,
    TELNET_STATE_WONT = 3,
    TELNET_STATE_DO = 4,
    TELNET_STATE_DONT = 5,
    TELNET_STATE_SB = 6
} TelnetProtocolState;

typedef struct {
    uint32_t remote_ip;
    uint16_t remote_port;
    TelnetProtocolState proto_state;
    char terminal_type[32];
    uint16_t screen_cols;
    uint16_t screen_rows;
    uint32_t bytes_transferred;
    bool is_session_active;
} NcsaTelnetContext;

typedef struct {
    float in_silicon_telnet_fidelity;
    float telnet_strategy_datbin_merkle_ratio;
    float telnet_channel_latency_ns;
    uint64_t verified_telnet_saat_clearances;

    bool telnet_vt100_session_verified;
    bool telnet_strategy_merkle_verified;
    bool telnet_submicro_latency_verified;
    bool telnet_lossless_saat_verified;
    bool grand_1950_parity_closure_verified;
    uint32_t rule18_parity_checksum;
} NcsaTelnetBeyond1945State;

int cpm_tomie_telnet_init(NcsaTelnetContext *ctx, uint32_t remote_ip, uint16_t port);
int cpm_tomie_telnet_process_input(NcsaTelnetContext *ctx, const uint8_t *data, size_t len);
void auncient_ncsa_telnet_init(NcsaTelnetBeyond1945State *state);
bool auncient_ncsa_telnet_verify_theorems_1946_1950(NcsaTelnetBeyond1945State *state);
uint32_t auncient_ncsa_telnet_compute_rule18(const NcsaTelnetBeyond1945State *state);

#ifdef __cplusplus
}
#endif

#endif /* AUNCIENT_NCSA_TELNET_CPMTOMIE_THEOREMS_1946_1950_H */
