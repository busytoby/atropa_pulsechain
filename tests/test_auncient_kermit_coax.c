#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <openssl/sha.h>

#define HASH_SIZE 32
#define SOH 0x01

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
static void sha256(const void *data, size_t len, uint8_t *out) {
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, data, len);
    SHA256_Final(out, &ctx);
}
#pragma GCC diagnostic pop

typedef enum {
    CUTOFF_STATE,
    CONDUC_STATE
} gate_state_t;

// 2-3 Merkle Tree Node (SRAM buffer storage)
typedef struct TwoThreeNode {
    bool is_leaf;
    int num_keys;
    uint32_t keys[2];
    char values[2][128];
    uint8_t node_hash[HASH_SIZE];
    struct TwoThreeNode *children[3];
} TwoThreeNode;

// Coaxial Kermit Packet Layout
typedef struct {
    uint8_t mark;
    uint8_t len;
    uint8_t seq;
    char type;
    char data[64];
    uint8_t checksum;
} kermit_packet_t;

// VDM context for Kermit transmission
typedef struct {
    uint32_t vdm_id;
    TwoThreeNode *file_buffer;
    gate_state_t write_gate;
    char backup_value[128];
    uint8_t backup_hash[HASH_SIZE];
} kermit_vdm_ctx_t;

#define TAG_KERNEL 0x01
#define TAG_USER   0x02

// Helper to create leaf node
static TwoThreeNode* create_leaf(uint32_t key1, const char *val1) {
    TwoThreeNode *node = (TwoThreeNode*)calloc(1, sizeof(TwoThreeNode));
    node->is_leaf = true;
    node->num_keys = 1;
    node->keys[0] = key1;
    strcpy(node->values[0], val1);
    
    uint8_t temp[256];
    int len = snprintf((char*)temp, sizeof(temp), "%u:%s", node->keys[0], node->values[0]);
    sha256(temp, len, node->node_hash);
    return node;
}

// Compute simple Kermit checksum
uint8_t kermit_calc_checksum(const kermit_packet_t *pkt) {
    uint32_t sum = pkt->len + pkt->seq + (uint8_t)pkt->type;
    for (int i = 0; i < pkt->len - 3; i++) {
        sum += (uint8_t)pkt->data[i];
    }
    return (uint8_t)((sum + ((sum & 0xC0) >> 6)) & 0x3F);
}

// -------------------------------------------------------------
// Coaxial Kermit Frame Receiver Gated by Fourier Implication
// -------------------------------------------------------------
bool kermit_receive_packet(kermit_vdm_ctx_t *ctx, const kermit_packet_t *pkt, 
                           double freq, double q_factor, uint8_t tag) {
    ctx->write_gate = CUTOFF_STATE;

    if (tag != TAG_KERNEL) return false;

    // 1. Log target buffer before transaction (for converse rollback)
    strcpy(ctx->backup_value, ctx->file_buffer->values[0]);
    memcpy(ctx->backup_hash, ctx->file_buffer->node_hash, HASH_SIZE);

    // 2. Precondition check: Fourier Frequency parameters
    bool phi_fourier = (freq == 440.0 && q_factor >= 0.5);
    if (!phi_fourier) return false;

    // 3. Postcondition check: Checksum and Mark verification
    uint8_t local_check = kermit_calc_checksum(pkt);
    bool psi_clearance = (pkt->mark == SOH && pkt->checksum == local_check);
    if (!psi_clearance) {
        // Rollback buffer using converse shadow values
        strcpy(ctx->file_buffer->values[0], ctx->backup_value);
        memcpy(ctx->file_buffer->node_hash, ctx->backup_hash, HASH_SIZE);
        return false;
    }

    // Pass: Deliver payload to target file buffer
    ctx->write_gate = CONDUC_STATE;
    strcpy(ctx->file_buffer->values[0], pkt->data);

    // Recompute target partition Merkle hash
    uint8_t temp[256];
    int len = snprintf((char*)temp, sizeof(temp), "%u:%s", ctx->file_buffer->keys[0], ctx->file_buffer->values[0]);
    sha256(temp, len, ctx->file_buffer->node_hash);

    return true;
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT COAXIAL KERMIT PROTOCOL VALIDATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    TwoThreeNode *file_node = create_leaf(500, "EOF");
    kermit_vdm_ctx_t ctx = {
        .vdm_id = 1,
        .file_buffer = file_node,
        .write_gate = CUTOFF_STATE
    };

    // 1. Build and send compliant Kermit packet -> Should succeed
    kermit_packet_t valid_packet = {
        .mark = SOH,
        .len = 16,
        .seq = 1,
        .type = 'D',
        .data = "KERMIT_DATA_BLOCK_01"
    };
    valid_packet.checksum = kermit_calc_checksum(&valid_packet);

    printf("[TEST] Dispatching compliant Kermit packet...\n");
    fflush(stdout);
    bool ok = kermit_receive_packet(&ctx, &valid_packet, 440.0, 0.8, TAG_KERNEL);
    assert(ok == true);
    assert(ctx.write_gate == CONDUC_STATE);
    assert(strcmp(file_node->values[0], "KERMIT_DATA_BLOCK_01") == 0);
    printf("   ✓ Packet accepted. Checksum and frequency verified.\n");
    fflush(stdout);

    // Save hash state
    uint8_t commit_hash[HASH_SIZE];
    memcpy(commit_hash, file_node->node_hash, HASH_SIZE);

    // 2. Send corrupted Kermit packet (checksum mismatch) -> Should rollback
    kermit_packet_t bad_packet = {
        .mark = SOH,
        .len = 16,
        .seq = 2,
        .type = 'D',
        .data = "CORRUPTED_PAYLOAD_BLOCK",
        .checksum = 99 // Bad checksum
    };

    printf("[TEST] Dispatching corrupted Kermit packet...\n");
    fflush(stdout);
    ok = kermit_receive_packet(&ctx, &bad_packet, 440.0, 0.8, TAG_KERNEL);
    assert(ok == false);
    assert(ctx.write_gate == CUTOFF_STATE);
    assert(strcmp(file_node->values[0], "KERMIT_DATA_BLOCK_01") == 0);
    assert(memcmp(commit_hash, file_node->node_hash, HASH_SIZE) == 0);
    printf("   ✓ Checksum mismatch caught. Buffer rolled back successfully.\n");
    fflush(stdout);

    free(file_node);
    printf("=============================================================\n");
    printf("COAXIAL KERMIT PROTOCOL TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
