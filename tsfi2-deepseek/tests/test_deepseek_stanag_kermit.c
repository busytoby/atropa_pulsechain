#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_deepseek_stanag_kermit.h"

int main(void) {
    printf("=== TESTING COAXIAL KERMIT & STANAG 5066/5516 LINK LAYER ===\n");

    // 1. Test Kermit Packet Encoding & Arithmetic Checksum
    tsfi_kermit_packet_t pkt_tx;
    assert(tsfi_kermit_packet_init(&pkt_tx, 1, KERMIT_TYPE_TELEMETRY));
    const char *telemetry_msg = "ZORSE_CORDIS:turn_id=101;ast_nodes=14;zmm_vector_ok=1";
    pkt_tx.data_len = strlen(telemetry_msg);
    memcpy(pkt_tx.data, telemetry_msg, pkt_tx.data_len);

    uint8_t raw_kermit[STANAG_KERMIT_MAX_FRAME_LEN] = {0};
    size_t kermit_len = 0;
    assert(tsfi_kermit_encode_frame(&pkt_tx, raw_kermit, &kermit_len));
    printf("[1/3] Kermit packet encoded (%zu bytes, Checksum: 0x%02X)\n", kermit_len, raw_kermit[kermit_len - 1]);

    // Test Kermit Packet Decoding & Parity Verification
    tsfi_kermit_packet_t pkt_rx;
    assert(tsfi_kermit_decode_frame(raw_kermit, kermit_len, &pkt_rx));
    assert(pkt_rx.type == KERMIT_TYPE_TELEMETRY);
    assert(pkt_rx.data_len == pkt_tx.data_len);
    assert(memcmp(pkt_rx.data, telemetry_msg, pkt_rx.data_len) == 0);
    printf("[1/3] Kermit packet decoded and byte-verified with perfect parity\n");

    // 2. Test STANAG 5066 Encapsulation
    tsfi_stanag_coaxial_frame_t stanag_frame;
    assert(tsfi_stanag_coaxial_encapsulate(
        STANAG_SAP_CORDIS_TELEMETRY,
        14, // High Priority
        0x57A1, // Node ID
        &pkt_tx,
        &stanag_frame
    ));
    assert(stanag_frame.sap_address == STANAG_SAP_CORDIS_TELEMETRY);
    assert(stanag_frame.priority == 14);
    printf("[2/3] STANAG 5066 frame encapsulated (SAP=0x%02X, Node=0x%04X, CRC32=0x%08X)\n",
           stanag_frame.sap_address, stanag_frame.node_address, stanag_frame.crc32);

    // 3. Test Coaxial Bus Physical Wire Transmission
    tsfi_stanag_coaxial_bus_state_t bus;
    assert(tsfi_stanag_coaxial_bus_init(&bus));

    uint8_t wire_buf[2048] = {0};
    size_t wire_len = 0;
    assert(tsfi_stanag_coaxial_transmit(&bus, &stanag_frame, wire_buf, &wire_len));
    assert(wire_buf[0] == 0x57 && wire_buf[1] == 0xA1); // Non-volatile 0x57A1 sync header
    assert(bus.total_frames_sent == 1);
    printf("[3/3] Coaxial wire transmission verified (%zu bytes dispatched, wire latency: %.2f ns)\n",
           wire_len, bus.coaxial_wire_latency_ns);

    printf("=== ALL COAXIAL KERMIT & STANAG 5066 TESTS PASSED ===\n");
    return 0;
}
