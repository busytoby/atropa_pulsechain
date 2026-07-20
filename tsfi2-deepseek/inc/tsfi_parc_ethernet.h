#ifndef TSFI_PARC_ETHERNET_H
#define TSFI_PARC_ETHERNET_H

#include <stdint.h>

#define PUP_TYPE_DATA 0x01
#define PUP_TYPE_ACK  0x02
#define PUP_TYPE_ERROR 0x03

typedef struct {
    uint8_t dest_host;   // 8-bit Xerox host address
    uint8_t src_host;    // 8-bit source host address
    uint16_t pup_type;   // PARC Universal Packet type
    uint32_t pup_id;     // Packet sequence ID
    uint16_t data_len;   // Byte size of payload
    uint8_t payload[128];
    uint16_t checksum;   // 16-bit block checksum
} tsfi_parc_pup_packet_t;

/* Encode a PARC Universal Packet into a raw byte stream */
int tsfi_parc_pup_encode(const tsfi_parc_pup_packet_t *pkt, uint8_t *stream_out, int max_len);

/* Decode a raw byte stream back into a packet structure */
int tsfi_parc_pup_decode(const uint8_t *stream_in, int stream_len, tsfi_parc_pup_packet_t *pkt_out);

#endif // TSFI_PARC_ETHERNET_H
