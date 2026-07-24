#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <openssl/sha.h>

#define HASH_SIZE 32

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

// 2-3 Merkle Tree Node (TTY Device Registry Control Block)
typedef struct TwoThreeNode {
    bool is_leaf;
    int num_keys;
    uint32_t keys[2];
    char values[2][128];
    uint8_t node_hash[HASH_SIZE];
    struct TwoThreeNode *children[3];
} TwoThreeNode;

// TTY Device Context
typedef struct {
    uint32_t device_id;
    char rx_buffer[64];
    uint32_t rx_ptr;
    uint8_t translate_table[256];
    TwoThreeNode *control_block;
    bool interrupt_asserted;
    gate_state_t write_gate;
} xplos_tty_device_t;

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

// -------------------------------------------------------------
// TTY Character Write & Translate Gated Loop
// -------------------------------------------------------------
bool xplos_tty_write_character(xplos_tty_device_t *tty, char c, uint8_t tag) {
    tty->interrupt_asserted = false;
    tty->write_gate = CUTOFF_STATE;

    // 1. Privilege Tag Check
    if (tag != TAG_KERNEL) return false;

    // 2. Constant-Time character translation
    uint8_t translated_char = tty->translate_table[(uint8_t)c];

    // 3. Buffer Bounds Check
    if (tty->rx_ptr >= sizeof(tty->rx_buffer) - 1) {
        return false;
    }

    // Write character and assert interrupt event
    tty->write_gate = CONDUC_STATE;
    tty->rx_buffer[tty->rx_ptr++] = (char)translated_char;
    tty->rx_buffer[tty->rx_ptr] = '\0';
    tty->interrupt_asserted = true;

    // Update TTY Device Control Block node values in the 2-3 tree DAT
    snprintf(tty->control_block->values[0], sizeof(tty->control_block->values[0]), "BUF:%s", tty->rx_buffer);

    // Recompute Merkle root hash of the control block
    uint8_t temp[256];
    int len = snprintf((char*)temp, sizeof(temp), "%u:%s", tty->control_block->keys[0], tty->control_block->values[0]);
    sha256(temp, len, tty->control_block->node_hash);

    return true;
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT XPLOS TTY DEVICE VALIDATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    TwoThreeNode *cb = create_leaf(800, "BUF:");
    xplos_tty_device_t tty = {
        .device_id = 1,
        .rx_buffer = "",
        .rx_ptr = 0,
        .translate_table = {0},
        .control_block = cb,
        .interrupt_asserted = false,
        .write_gate = CUTOFF_STATE
    };

    // Initialize translation table with identity mapping
    for (int i = 0; i < 256; i++) {
        tty.translate_table[i] = i;
    }
    // Simple EBCDIC-to-ASCII-style uppercase letter transposition (ROT13)
    for (int i = 'A'; i <= 'Z'; i++) {
        tty.translate_table[i] = 'A' + ((i - 'A' + 13) % 26);
    }

    uint8_t initial_hash[HASH_SIZE];
    memcpy(initial_hash, cb->node_hash, HASH_SIZE);

    // 1. Authorized TTY Write (TAG_KERNEL) -> Should translate and update control block hash
    printf("[TEST] Writing character 'A' (TAG_KERNEL)...\n");
    fflush(stdout);
    bool ok = xplos_tty_write_character(&tty, 'A', TAG_KERNEL);
    assert(ok == true);
    assert(tty.write_gate == CONDUC_STATE);
    assert(tty.interrupt_asserted == true);
    assert(strcmp(tty.rx_buffer, "N") == 0); // Translated to N (ROT13)
    assert(strcmp(cb->values[0], "BUF:N") == 0);
    assert(memcmp(initial_hash, cb->node_hash, HASH_SIZE) != 0);
    printf("   ✓ Character written and translated successfully: %s\n", tty.rx_buffer);
    fflush(stdout);

    // Save hash state
    uint8_t step1_hash[HASH_SIZE];
    memcpy(step1_hash, cb->node_hash, HASH_SIZE);

    // 2. Unauthorized TTY Write (TAG_USER) -> Should block and cutoff
    printf("[TEST] Writing character 'B' (TAG_USER)...\n");
    fflush(stdout);
    ok = xplos_tty_write_character(&tty, 'B', TAG_USER);
    assert(ok == false);
    assert(tty.write_gate == CUTOFF_STATE); // Cutoff active
    assert(strcmp(tty.rx_buffer, "N") == 0); // Unaltered
    assert(memcmp(step1_hash, cb->node_hash, HASH_SIZE) == 0); // Hash unchanged
    printf("   ✓ Write blocked successfully by privilege check.\n");
    fflush(stdout);

    free(cb);
    printf("=============================================================\n");
    printf("XPLOS TTY DEVICE TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
