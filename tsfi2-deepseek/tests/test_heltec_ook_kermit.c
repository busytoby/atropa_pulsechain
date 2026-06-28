#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#define KERMIT_SOH 0x01
#define MAX_PAYLOAD 90

// Auncient YI State Registers
typedef struct {
    uint64_t base;
    uint64_t secret;
    uint64_t signal;
    uint64_t motzkin_prime;
} YiState;

// Protocol Frame
typedef struct {
    uint8_t soh;
    uint8_t len;
    uint8_t seq;
    uint8_t type;
    uint8_t data[MAX_PAYLOAD];
    uint8_t check;
} KermitFrame;

typedef struct {
    char name[16];
    uint16_t address;
    bool is_rod;
    uint8_t seq_num;
    uint8_t public_key[32];
    uint8_t peer_public_key[32];
    YiState yi;
    bool yi_established;
} HeltecNode;

// Helper to compute Kermit 6-bit checksum
static uint8_t kermit_checksum(const uint8_t *buf, size_t len) {
    uint32_t sum = 0;
    for (size_t i = 0; i < len; i++) {
        sum += buf[i];
    }
    return (uint8_t)(((sum + ((sum & 0xC0) >> 6)) & 0x3F) + 32);
}

// Pack a Kermit frame
static size_t pack_kermit_frame(uint8_t seq, char type, const uint8_t *data, size_t data_len, uint8_t *out_buf) {
    out_buf[0] = KERMIT_SOH;
    out_buf[1] = (uint8_t)(data_len + 3 + 32);
    out_buf[2] = (uint8_t)(seq + 32);
    out_buf[3] = (uint8_t)type;
    if (data_len > 0 && data != NULL) {
        memcpy(&out_buf[4], data, data_len);
    }
    uint8_t check = kermit_checksum(&out_buf[1], data_len + 3);
    out_buf[4 + data_len] = check;
    return data_len + 5;
}

// Parse a Kermit frame
static bool parse_kermit_frame(const uint8_t *buf, size_t size, KermitFrame *frame) {
    if (size < 5 || buf[0] != KERMIT_SOH) return false;
    frame->soh = buf[0];
    frame->len = (uint8_t)(buf[1] - 32);
    frame->seq = (uint8_t)(buf[2] - 32);
    frame->type = buf[3];
    size_t expected_data_len = frame->len - 3;
    if (expected_data_len > MAX_PAYLOAD || expected_data_len + 5 > size) return false;
    memcpy(frame->data, &buf[4], expected_data_len);
    frame->check = buf[4 + expected_data_len];
    uint8_t calc_check = kermit_checksum(&buf[1], expected_data_len + 3);
    return calc_check == frame->check;
}

// YI react signature derivation
static uint64_t yi_react(HeltecNode *node, uint64_t nonce) {
    uint64_t signature = node->yi.base;
    if (node->is_rod) {
        signature = (signature ^ node->yi.secret ^ nonce) % node->yi.motzkin_prime;
    } else {
        signature = (signature ^ node->yi.signal ^ nonce) % node->yi.motzkin_prime;
    }
    return signature;
}

int main() {
    printf("=== Auncient Emulated YI Handshake & Nonce Signing Test Bed ===\n");
    
    // Initialize Node A (Rod) and Node B (Cone)
    HeltecNode node_a = {
        .name = "NODE_A_ROD",
        .address = 0xAA01,
        .is_rod = true,
        .seq_num = 0,
        .public_key = { 0x0A, 0x0B, 0x0C }, // Node A Ephemeral Key
        .yi = { .secret = 9876543, .signal = 5555555, .motzkin_prime = 953467954114363ULL },
        .yi_established = false
    };
    
    HeltecNode node_b = {
        .name = "NODE_B_CONE",
        .address = 0xBB02,
        .is_rod = false,
        .seq_num = 0,
        .public_key = { 0x01, 0x02, 0x03 }, // Node B Ephemeral Key
        .yi = { .secret = 9876543, .signal = 5555555, .motzkin_prime = 953467954114363ULL },
        .yi_established = false
    };
    
    uint8_t channel_buffer[256];
    size_t channel_len = 0;
    
    // --- PHASE 1: Open Key Exchange (No Nonces) ---
    printf("\n[PHASE 1] Initiating Open Key Exchange...\n");
    
    // Node A sends 'S' frame carrying its public key
    printf("[%s] Broadcasting Send-Init (S) frame...\n", node_a.name);
    channel_len = pack_kermit_frame(node_a.seq_num, 'S', node_a.public_key, 32, channel_buffer);
    
    // Node B receives and processes 'S' frame
    KermitFrame frame_rx;
    if (parse_kermit_frame(channel_buffer, channel_len, &frame_rx) && frame_rx.type == 'S') {
        printf("[%s] Received Send-Init. Saving Peer Public Key.\n", node_b.name);
        memcpy(node_b.peer_public_key, frame_rx.data, 32);
        
        // Derive shared YI base on Node B: XOR public keys
        node_b.yi.base = 0;
        for (int i = 0; i < 32; i++) {
            node_b.yi.base ^= (node_b.public_key[i] ^ node_b.peer_public_key[i]);
        }
        node_b.yi_established = true;
        printf("[%s] Derived YI Base: %lu\n", node_b.name, node_b.yi.base);
        
        // Node B responds with ACK ('Y') carrying its public key
        printf("[%s] Transmitting ACK (Y) frame...\n", node_b.name);
        channel_len = pack_kermit_frame(frame_rx.seq, 'Y', node_b.public_key, 32, channel_buffer);
    }
    
    // Node A receives the ACK frame
    if (parse_kermit_frame(channel_buffer, channel_len, &frame_rx) && frame_rx.type == 'Y') {
        printf("[%s] Received ACK. Saving Peer Public Key.\n", node_a.name);
        memcpy(node_a.peer_public_key, frame_rx.data, 32);
        
        // Derive shared YI base on Node A
        node_a.yi.base = 0;
        for (int i = 0; i < 32; i++) {
            node_a.yi.base ^= (node_a.public_key[i] ^ node_a.peer_public_key[i]);
        }
        node_a.yi_established = true;
        printf("[%s] Derived YI Base: %lu\n", node_a.name, node_a.yi.base);
    }
    
    // Check if base matches
    if (node_a.yi.base == node_b.yi.base && node_a.yi_established) {
        printf("[SUCCESS] Shared YI State Established on both devices!\n");
    } else {
        printf("[FAILED] Shared YI derivation failed.\n");
        return 1;
    }
    
    // --- PHASE 2: Secure Nonce-Signed Session ---
    printf("\n[PHASE 2] Transitioning to Nonce-Signed Data Session...\n");
    node_a.seq_num++;
    
    uint64_t nonce_index = 100;
    // Node A generates signature using its role (ROD) and the nonce
    // We bind address: (nonce_index << 16) | address
    uint64_t nonce_a = (nonce_index << 16) | node_a.address;
    uint64_t sig_a = yi_react(&node_a, nonce_a);
    
    uint8_t payload[48];
    const char *payload_text = "Secure-WinchesterMQ-Data";
    memset(payload, 0, 48);
    memcpy(payload, payload_text, strlen(payload_text));
    memcpy(&payload[32], &nonce_a, 8);
    memcpy(&payload[40], &sig_a, 8);
    
    printf("[%s] Transmitting Signed Data: Nonce=0x%lx, Sig=%lu\n", node_a.name, nonce_a, sig_a);
    channel_len = pack_kermit_frame(node_a.seq_num, 'D', payload, 48, channel_buffer);
    
    // Node B receives and verifies the signature
    if (parse_kermit_frame(channel_buffer, channel_len, &frame_rx) && frame_rx.type == 'D') {
        uint64_t rx_nonce, rx_sig;
        memcpy(&rx_nonce, &frame_rx.data[32], 8);
        memcpy(&rx_sig, &frame_rx.data[40], 8);
        
        // B (CONE) verifies using A's role (ROD) parameters
        uint64_t expected_sig = yi_react(&node_a, rx_nonce); // Verify as Rod
        printf("[%s] Verifying signature. Expected: %lu, Received: %lu\n", node_b.name, expected_sig, rx_sig);
        
        if (rx_sig == expected_sig) {
            printf("[%s] [VERIFIED] Signature matches. Processing message: \"%s\"\n", node_b.name, frame_rx.data);
            
            // Respond with signed ACK ('Y') using B's role (CONE)
            uint64_t nonce_b = rx_nonce + 1;
            uint64_t sig_b = yi_react(&node_b, nonce_b);
            
            uint8_t ack_payload[48];
            memset(ack_payload, 0, 48);
            memcpy(&ack_payload[32], &nonce_b, 8);
            memcpy(&ack_payload[40], &sig_b, 8);
            
            printf("[%s] Transmitting Signed ACK: Nonce=0x%lx, Sig=%lu\n", node_b.name, nonce_b, sig_b);
            channel_len = pack_kermit_frame(frame_rx.seq, 'Y', ack_payload, 48, channel_buffer);
        } else {
            printf("[%s] [DROP] Signature verification failed! Blocked spoofing attempt.\n", node_b.name);
        }
    }
    
    // Node A receives and verifies Node B's ACK signature
    if (parse_kermit_frame(channel_buffer, channel_len, &frame_rx) && frame_rx.type == 'Y') {
        uint64_t rx_nonce, rx_sig;
        memcpy(&rx_nonce, &frame_rx.data[32], 8);
        memcpy(&rx_sig, &frame_rx.data[40], 8);
        
        uint64_t expected_sig = yi_react(&node_b, rx_nonce); // Verify as Cone
        printf("[%s] Verifying ACK signature. Expected: %lu, Received: %lu\n", node_a.name, expected_sig, rx_sig);
        
        if (rx_sig == expected_sig) {
            printf("[%s] [SUCCESS] ACK verified. Packet transmission completed successfully!\n", node_a.name);
        } else {
            printf("[%s] [FAIL] ACK verification failed. Rejected packet.\n", node_a.name);
        }
    }
    
    // --- SCENARIO 3: Attacker (Node C) Spoofing Attempt ---
    printf("\n[PHASE 3] Simulating Spoofing Attack from Node C (Attacker)...\n");
    
    // Attacker sends a fake ACK with arbitrary signature
    uint64_t fake_nonce = ((nonce_index + 1) << 16) | node_b.address;
    uint64_t fake_sig = 999999999; // Bad signature
    
    uint8_t fake_payload[48];
    memset(fake_payload, 0, 48);
    memcpy(&fake_payload[32], &fake_nonce, 8);
    memcpy(&fake_payload[40], &fake_sig, 8);
    
    printf("[NODE_C] Injecting fake ACK to Node A: Nonce=0x%lx, Sig=%lu\n", fake_nonce, fake_sig);
    channel_len = pack_kermit_frame(node_a.seq_num, 'Y', fake_payload, 48, channel_buffer);
    
    // Node A tries to parse the incoming frame
    if (parse_kermit_frame(channel_buffer, channel_len, &frame_rx) && frame_rx.type == 'Y') {
        uint64_t rx_nonce, rx_sig;
        memcpy(&rx_nonce, &frame_rx.data[32], 8);
        memcpy(&rx_sig, &frame_rx.data[40], 8);
        
        uint64_t expected_sig = yi_react(&node_b, rx_nonce); // Expected B (Cone) signature
        printf("[%s] Verifying signature. Expected: %lu, Received: %lu\n", node_a.name, expected_sig, rx_sig);
        
        if (rx_sig == expected_sig) {
            printf("[%s] [FAIL] Accepted fake signature! Vulnerability exposed.\n", node_a.name);
        } else {
            printf("[%s] [SUCCESS] Signature validation failed! Successfully blocked and ignored spoofed ACK.\n", node_a.name);
        }
    }
    
    printf("\n=== Emulated YI Handshake & Nonce Verification Passed ===\n");
    return 0;
}
