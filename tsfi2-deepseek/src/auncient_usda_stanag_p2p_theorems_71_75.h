#ifndef AUNCIENT_USDA_STANAG_P2P_THEOREMS_71_75_H
#define AUNCIENT_USDA_STANAG_P2P_THEOREMS_71_75_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define STANAG_TPA_BASE 0x0100
#define STANAG_MAX_PDU_BYTES 128

typedef struct __attribute__((packed)) {
    uint8_t  stanag_magic;       /* 0x53 ('S') */
    uint8_t  stanag_type;        /* 0x46 (STANAG 4586/5066 Compliance PDU) */
    uint16_t payload_len;        /* Length of packet payload */
    uint32_t asset_dna_seed;     /* Unique USDA participant DNA */
    uint64_t theorems_bitmask_lo;/* Theorems 1-64 bitmask */
    uint16_t theorems_bitmask_hi;/* Theorems 65-75 bitmask */
    uint32_t master_seal_witness;/* Dysnomia VM Master Certification Seal */
    float    spatial_coords[3];  /* [X, Y, Z] zone position in meters */
    uint16_t rule18_crc16;       /* Non-preferential 3-term checksum */
} StanagCompliancePdu;

typedef struct {
    StanagCompliancePdu pdu;
    uint32_t serialized_bytes_count;
    bool self_certification_seal_verified;
    bool stanag_pdu_size_verified;
    bool p2p_mutual_auth_verified;
    bool mesh_anti_collision_verified;
    bool winchester_stanag_bridge_verified;
    uint32_t rule18_parity_checksum;
} UsdaStanagP2pState;

void auncient_usda_stanag_init(UsdaStanagP2pState *state, uint32_t dna_seed);
bool auncient_usda_stanag_serialize_pdu(UsdaStanagP2pState *state, uint8_t *buffer, size_t max_buf_len);
bool auncient_usda_stanag_verify_theorems_71_75(UsdaStanagP2pState *state);
uint32_t auncient_usda_stanag_compute_rule18(const UsdaStanagP2pState *state);

#endif /* AUNCIENT_USDA_STANAG_P2P_THEOREMS_71_75_H */
