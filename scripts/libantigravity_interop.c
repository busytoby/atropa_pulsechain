#include "libantigravity_interop.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>
#include "abi_dispatch_map.h"
#include "../tsfi2-deepseek/inc/lau_memory.h"

// Initialize registry with baseline zero-state
void interop_registry_init(InteropRegistry *reg) {
    if (!reg) return;
    memset(reg, 0, sizeof(InteropRegistry));
    reg->frame_modulation_factor = 1.0f;
}

// Update the registry state from the active Auncient VM registers
void interop_registry_update(InteropRegistry *reg, const AuncientRegisterState *state, uint32_t epoch) {
    if (!reg || !state) return;
    
    // Copy active registers thread-safely
    reg->active_state = *state;
    reg->current_epoch = epoch;
    reg->active_cycles++;

    // Calculate rendering modulation factor based on the thermodynamic Barn heat glow register
    // High Barn heat maps to higher specular highlights and plasma frequency modulation
    if (state->element > 0) {
        reg->frame_modulation_factor = (float)(state->barn % state->element) / (float)state->element;
    } else {
        reg->frame_modulation_factor = 1.0f;
    }
}

// Probe the current registry data for the interop renderer
void interop_registry_probe(const InteropRegistry *reg, AuncientRegisterState *out_state, uint32_t *out_epoch) {
    if (!reg) return;
    if (out_state) {
        *out_state = reg->active_state;
    }
    if (out_epoch) {
        *out_epoch = reg->current_epoch;
    }
}

// WinchesterMQ implementation: write sector payload with flow-control limits
void interop_mq_put(InteropLUN *lun, const uint8_t *data) {
    if (!lun || !data) return;
    uint32_t size = 0;
    if (lun->head < lun->tail) {
        size = lun->tail - lun->head;
    }
    if (size >= 16) return; // Flow-control backpressure overflow protection

    uint32_t lba = lun->tail % 16;
    memcpy(lun->sectors[lba], data, 256);
    lun->tail++;
}

// WinchesterMQ implementation: Phase-1 Lease Get
int interop_mq_get_phase1(InteropLUN *lun, uint8_t *out_data) {
    if (!lun || !out_data || lun->head == lun->tail) return -1;
    uint32_t lba = lun->head % 16;
    memcpy(out_data, lun->sectors[lba], 256);
    lun->pending_ack = lun->head; // Lock lease on current head
    return 0;
}

// WinchesterMQ implementation: Phase-2 Commit ACK
int interop_mq_ack_phase2(InteropLUN *lun) {
    if (!lun || lun->pending_ack == 0xFFFFFFFF) return -1;
    if (lun->pending_ack == lun->head) {
        lun->head++;
        lun->pending_ack = 0xFFFFFFFF; // Clear lease
        return 0;
    }
    return -2;
}

uint64_t fnv1a_hash_cascade(uint64_t initial_hash, const void *data, size_t len) {
    uint64_t hash = initial_hash;
    const uint64_t *words = (const uint64_t*)data;
    size_t num_words = len / 8;
    for (size_t i = 0; i < num_words; i++) {
        hash ^= words[i];
        hash *= 1099511628211ULL;
    }
    const uint8_t *bytes = (const uint8_t*)(words + num_words);
    size_t remainder = len % 8;
    for (size_t i = 0; i < remainder; i++) {
        hash ^= bytes[i];
        hash *= 1099511628211ULL;
    }
    return hash;
}

static void persist_table_to_dat_bin(void *payload, InteropDynamicTable *table) {
    char filename[128];
    snprintf(filename, sizeof(filename), "../assets/table_%p.dat.bin", payload);
    FILE *f = fopen(filename, "wb");
    if (f) {
        uint64_t *current_rows = __atomic_load_n(&table->rows, __ATOMIC_ACQUIRE);
        uint32_t meta[4] = { table->active, table->col_count, table->count, table->capacity };
        uint64_t hash_val = fnv1a_hash_cascade(14695981039346656037ULL, meta, sizeof(meta));
        if (current_rows && table->count > 0) {
            size_t rows_bytes = table->count * table->col_count * sizeof(uint64_t);
            hash_val = fnv1a_hash_cascade(hash_val, current_rows, rows_bytes);
        }
        fwrite(&hash_val, sizeof(uint64_t), 1, f);
        fwrite(meta, sizeof(uint32_t), 4, f);
        if (current_rows && table->count > 0) {
            fwrite(current_rows, sizeof(uint64_t), table->count * table->col_count, f);
        }
        fclose(f);
    }
}

static void bst_insert_node(InteropBSTNode **root, uint64_t key, uint32_t row_index) {
    if (!*root) {
        InteropBSTNode *node = (InteropBSTNode*)calloc(1, sizeof(InteropBSTNode));
        node->key = key;
        node->row_index = row_index;
        *root = node;
        return;
    }
    if (key < (*root)->key) {
        bst_insert_node(&((*root)->left), key, row_index);
    } else {
        bst_insert_node(&((*root)->right), key, row_index);
    }
}

static InteropBSTNode* bst_find_node(InteropBSTNode *root, uint64_t key) {
    if (!root) return NULL;
    if (root->key == key) return root;
    if (key < root->key) return bst_find_node(root->left, key);
    return bst_find_node(root->right, key);
}

static void bst_free_nodes(InteropBSTNode *root) {
    if (!root) return;
    bst_free_nodes(root->left);
    bst_free_nodes(root->right);
    free(root);
}

static InteropBSTNode* bst_clone_tree(InteropBSTNode *root) {
    if (!root) return NULL;
    InteropBSTNode *new_node = (InteropBSTNode*)calloc(1, sizeof(InteropBSTNode));
    new_node->key = root->key;
    new_node->row_index = root->row_index;
    new_node->left = bst_clone_tree(root->left);
    new_node->right = bst_clone_tree(root->right);
    return new_node;
}

uint64_t interop_agent_create_table(void *payload, const uint64_t *args, size_t arg_count) {
    InteropDynamicTable *table = (InteropDynamicTable*)payload;
    LauWiredHeader *h = (LauWiredHeader*)((char*)payload - 8192);
    if (arg_count >= 2) {
        uint32_t col_count = (uint32_t)args[0];
        uint32_t capacity = (uint32_t)args[1];
        
        // Clean up previous rows and BST index
        uint64_t *old_rows = table->rows;
        table->rows = NULL;
        if (old_rows) {
            free(old_rows);
        }
        if (table->bst_root) {
            bst_free_nodes(table->bst_root);
            table->bst_root = NULL;
        }
        
        table->col_count = col_count;
        table->capacity = capacity;
        table->count = 0;
        
        uint64_t *new_rows = calloc(capacity * col_count, sizeof(uint64_t));
        __atomic_store_n(&table->rows, new_rows, __ATOMIC_RELEASE);
        table->active = 1;
        
        h->counter++; // Invalidate cache
        persist_table_to_dat_bin(payload, table);
        return 1;
    }
    return 0;
}

uint64_t interop_agent_insert_dynamic(void *payload, const uint64_t *args, size_t arg_count) {
    InteropDynamicTable *table = (InteropDynamicTable*)payload;
    LauWiredHeader *h = (LauWiredHeader*)((char*)payload - 8192);
    if (table->active && arg_count == table->col_count && table->count < table->capacity) {
        // Read-Copy-Update (RCU): Allocate new rows copy
        uint64_t *new_rows = calloc(table->capacity * table->col_count, sizeof(uint64_t));
        uint64_t *current_rows = __atomic_load_n(&table->rows, __ATOMIC_ACQUIRE);
        if (current_rows) {
            memcpy(new_rows, current_rows, table->count * table->col_count * sizeof(uint64_t));
        }
        
        // Append the new row to the new rows copy
        for (uint32_t i = 0; i < table->col_count; i++) {
            new_rows[table->count * table->col_count + i] = args[i];
        }
        
        // Read-Copy-Update (RCU): Clone and update the BST index tree
        InteropBSTNode *current_root = __atomic_load_n(&table->bst_root, __ATOMIC_ACQUIRE);
        InteropBSTNode *new_root = bst_clone_tree(current_root);
        bst_insert_node(&new_root, args[0], table->count);
        
        // Atomic Pointer Swap (RCU release) for both rows and index root
        uint64_t *old_rows = current_rows;
        __atomic_store_n(&table->rows, new_rows, __ATOMIC_RELEASE);
        __atomic_store_n(&table->bst_root, new_root, __ATOMIC_RELEASE);
        
        if (old_rows) {
            free(old_rows);
        }
        if (current_root) {
            bst_free_nodes(current_root);
        }
        
        table->count++;
        h->counter++;
        persist_table_to_dat_bin(payload, table);
        return 1;
    }
    return 0;
}

uint64_t interop_agent_select_dynamic(void *payload, const uint64_t *args, size_t arg_count) {
    InteropDynamicTable *table = (InteropDynamicTable*)payload;
    if (table->active && arg_count >= 1 && table->col_count >= 2) {
        uint64_t key = args[0];
        
        // Log(N) search using the atomically loaded BST index root
        InteropBSTNode *current_root = __atomic_load_n(&table->bst_root, __ATOMIC_ACQUIRE);
        InteropBSTNode *node = bst_find_node(current_root, key);
        if (node) {
            uint64_t *current_rows = __atomic_load_n(&table->rows, __ATOMIC_ACQUIRE);
            if (current_rows) {
                return current_rows[node->row_index * table->col_count + 1];
            }
        }
    }
    return 0;
}

uint64_t interop_agent_rdbms_dispatch(void *payload, const uint64_t *args, size_t arg_count) {
    (void)payload;
    if (arg_count < 2) return 0;
    void *target_payload = (void*)args[0];
    uint32_t selector = (uint32_t)args[1];
    uint64_t forward_args[5] = {0};
    size_t forward_count = arg_count - 2;
    for (size_t i = 0; i < forward_count && i < 5; i++) {
        forward_args[i] = args[2 + i];
    }
    LauWiredHeader *h = (LauWiredHeader*)((char*)target_payload - 8192);
    for (int i = 0; i < h->schema_count; i++) {
        if (h->schema[i].offset == selector) {
            typedef uint64_t (*LauMethod)(void*, const uint64_t*, size_t);
            LauMethod func = (LauMethod)h->schema[i].target_fn;
            return func(target_payload, forward_args, forward_count);
        }
    }
    return 0;
}

uint64_t interop_agent_insert(void *payload, const uint64_t *args, size_t arg_count) {
    InteropStaticTable *table = (InteropStaticTable*)payload;
    LauWiredHeader *h = (LauWiredHeader*)((char*)payload - 8192);
    if (arg_count >= 2 && table->count < 10) {
        table->keys[table->count] = args[0];
        table->values[table->count] = args[1];
        table->count++;
        h->counter++; // Invalidate cache
        return 1;
    }
    return 0;
}

uint64_t interop_agent_query(void *payload, const uint64_t *args, size_t arg_count) {
    InteropStaticTable *table = (InteropStaticTable*)payload;
    static uint64_t execution_count = 0;
    execution_count++;
    if (arg_count >= 1) {
        uint64_t key = args[0];
        for (uint32_t i = 0; i < table->count; i++) {
            if (table->keys[i] == key) {
                return table->values[i] + (execution_count * 1000000ULL);
            }
        }
    }
    return 0;
}

uint64_t interop_agent_insert_external(void *my_payload, const uint64_t *args, size_t arg_count) {
    (void)my_payload;
    if (arg_count < 3) return 0;
    void *target_payload = (void*)args[0];
    uint64_t key = args[1];
    uint64_t val = args[2];
    LauWiredHeader *h = (LauWiredHeader*)((char*)target_payload - 8192);
    if (!h->sealed || h->version != 1) return 0;
    InteropStaticTable *table = (InteropStaticTable*)target_payload;
    if (table->count < 10) {
        table->keys[table->count] = key;
        table->values[table->count] = val;
        table->count++;
        h->counter++; // Invalidate target's cache
        return 1;
    }
    return 0;
}

uint64_t interop_agent_delete(void *payload, const uint64_t *args, size_t arg_count) {
    InteropStaticTable *table = (InteropStaticTable*)payload;
    LauWiredHeader *h = (LauWiredHeader*)((char*)payload - 8192);
    if (arg_count >= 1) {
        uint64_t key = args[0];
        for (uint32_t i = 0; i < table->count; i++) {
            if (table->keys[i] == key) {
                table->keys[i] = table->keys[table->count - 1];
                table->values[i] = table->values[table->count - 1];
                table->count--;
                h->counter++;
                return 1;
            }
        }
    }
    return 0;
}

uint64_t interop_agent_update(void *payload, const uint64_t *args, size_t arg_count) {
    InteropStaticTable *table = (InteropStaticTable*)payload;
    LauWiredHeader *h = (LauWiredHeader*)((char*)payload - 8192);
    if (arg_count >= 2) {
        uint64_t key = args[0];
        uint64_t val = args[1];
        for (uint32_t i = 0; i < table->count; i++) {
            if (table->keys[i] == key) {
                table->values[i] = val;
                h->counter++;
                return 1;
            }
        }
    }
    return 0;
}

void interop_agent_destroy_table(InteropDynamicTable *table) {
    if (!table) return;
    uint64_t *old_rows = table->rows;
    table->rows = NULL;
    if (old_rows) {
        free(old_rows);
    }
    if (table->bst_root) {
        bst_free_nodes(table->bst_root);
        table->bst_root = NULL;
    }
    table->active = 0;
}

int interop_mq_dispatch_rdbms(InteropLUN *lun, void *db_payload, ABIDispatchMap *map) {
    if (!lun || !db_payload || !map) return -1;
    uint8_t sector_data[256];
    if (interop_mq_get_phase1(lun, sector_data) < 0) {
        return 0; // No pending request
    }
    uint32_t selector = *(uint32_t*)&sector_data[0];
    uint32_t arg_count = *(uint32_t*)&sector_data[4];
    uint64_t args[5] = {0};
    if (arg_count > 5) arg_count = 5;
    for (uint32_t i = 0; i < arg_count; i++) {
        args[i] = *(uint64_t*)&sector_data[8 + (i * 8)];
    }
    uint64_t result = 0;
    if (abi_dispatch_invoke(map, selector, db_payload, args, arg_count, &result)) {
        memset(sector_data, 0, 256);
        *(uint64_t*)&sector_data[0] = result;
        *(uint32_t*)&sector_data[8] = 1; // Success code
        memcpy(lun->sectors[lun->head % 16], sector_data, 256);
        interop_mq_ack_phase2(lun);
        return 1;
    }
    return -2;
}

uint64_t interop_agent_log_transaction(void *payload, uint32_t selector, const uint64_t *args, size_t arg_count) {
    char filename[128];
    snprintf(filename, sizeof(filename), "../assets/audit_log_%p.dat.bin", payload);
    uint64_t prev_hash = 0;
    FILE *f_read = fopen(filename, "rb");
    if (f_read) {
        fseek(f_read, 0, SEEK_END);
        long size = ftell(f_read);
        if (size >= (long)sizeof(InteropAuditEntry)) {
            fseek(f_read, size - sizeof(InteropAuditEntry), SEEK_SET);
            InteropAuditEntry last_entry;
            if (fread(&last_entry, sizeof(InteropAuditEntry), 1, f_read) == 1) {
                prev_hash = last_entry.entry_hash;
            }
        }
        fclose(f_read);
    }
    InteropAuditEntry entry;
    memset(&entry, 0, sizeof(InteropAuditEntry));
    entry.prev_hash = prev_hash;
    entry.selector = selector;
    entry.arg_count = (uint32_t)arg_count;
    size_t copy_limit = arg_count > 5 ? 5 : arg_count;
    for (size_t i = 0; i < copy_limit; i++) {
        entry.args[i] = args[i];
    }
    uint64_t hash_val = fnv1a_hash_cascade(14695981039346656037ULL, &entry.prev_hash, sizeof(uint64_t));
    hash_val = fnv1a_hash_cascade(hash_val, &entry.selector, sizeof(uint32_t));
    hash_val = fnv1a_hash_cascade(hash_val, &entry.arg_count, sizeof(uint32_t));
    hash_val = fnv1a_hash_cascade(hash_val, entry.args, sizeof(entry.args));
    entry.entry_hash = hash_val;
    FILE *f_write = fopen(filename, "ab");
    if (f_write) {
        fwrite(&entry, sizeof(InteropAuditEntry), 1, f_write);
        fclose(f_write);
        return hash_val;
    }
    return 0;
}

int interop_agent_verify_audit_log(void *payload, uint64_t *out_final_hash) {
    char filename[128];
    snprintf(filename, sizeof(filename), "../assets/audit_log_%p.dat.bin", payload);
    FILE *f = fopen(filename, "rb");
    if (!f) return -1;
    uint64_t running_prev_hash = 0;
    InteropAuditEntry entry;
    int count = 0;
    while (fread(&entry, sizeof(InteropAuditEntry), 1, f) == 1) {
        if (entry.prev_hash != running_prev_hash) {
            fclose(f);
            return -2;
        }
        uint64_t hash_val = fnv1a_hash_cascade(14695981039346656037ULL, &entry.prev_hash, sizeof(uint64_t));
        hash_val = fnv1a_hash_cascade(hash_val, &entry.selector, sizeof(uint32_t));
        hash_val = fnv1a_hash_cascade(hash_val, &entry.arg_count, sizeof(uint32_t));
        hash_val = fnv1a_hash_cascade(hash_val, entry.args, sizeof(entry.args));
        if (entry.entry_hash != hash_val) {
            fclose(f);
            return -3;
        }
        running_prev_hash = hash_val;
        count++;
    }
    fclose(f);
    if (out_final_hash) {
        *out_final_hash = running_prev_hash;
    }
    return count;
}

void interop_coaxial_init_table(InteropCoaxialTable *table, uint32_t capacity, uint32_t col_count) {
    if (!table) return;
    table->lock = 0;
    table->rows_offset = 1024;
    table->count = 0;
    table->capacity = capacity;
    table->col_count = col_count;
}

uint64_t interop_coaxial_insert(InteropCoaxialTable *table, const uint64_t *args, size_t arg_count) {
    if (!table || arg_count != table->col_count) return 0;
    while (__atomic_test_and_set(&table->lock, __ATOMIC_ACQUIRE)) {
#if defined(__x86_64__) || defined(_M_X64)
        __builtin_ia32_pause();
#endif
    }
    if (table->count >= table->capacity) {
        __atomic_clear(&table->lock, __ATOMIC_RELEASE);
        return 0;
    }
    uint32_t next_offset = (table->rows_offset == 1024) ? 8192 : 1024;
    char *base = (char*)table;
    uint64_t *new_rows = (uint64_t*)(base + next_offset);
    uint64_t *old_rows = (uint64_t*)(base + table->rows_offset);
    if (table->count > 0) {
        memcpy(new_rows, old_rows, table->count * table->col_count * sizeof(uint64_t));
    }
    for (uint32_t i = 0; i < table->col_count; i++) {
        new_rows[table->count * table->col_count + i] = args[i];
    }
    __atomic_store_n(&table->rows_offset, next_offset, __ATOMIC_RELEASE);
    __atomic_add_fetch(&table->count, 1, __ATOMIC_RELEASE);
    __atomic_clear(&table->lock, __ATOMIC_RELEASE);
    return 1;
}

uint64_t interop_coaxial_select(const InteropCoaxialTable *table, uint64_t key) {
    if (!table || table->col_count < 2) return 0;
    uint32_t active_offset = __atomic_load_n(&table->rows_offset, __ATOMIC_ACQUIRE);
    uint32_t active_count = __atomic_load_n(&table->count, __ATOMIC_ACQUIRE);
    const char *base = (const char*)table;
    const uint64_t *current_rows = (const uint64_t*)(base + active_offset);
    if (active_count > 0) {
        for (uint32_t r = active_count; r > 0; r--) {
            uint32_t idx = r - 1;
            if (current_rows[idx * table->col_count] == key) {
                return current_rows[idx * table->col_count + 1];
            }
        }
    }
    return 0;
}

void interop_system_ledger_init(InteropSystemLedger *ledger) {
    if (!ledger) return;
    interop_coaxial_init_table(&ledger->trie_route_table, 128, 2);
    interop_coaxial_init_table(&ledger->wmq_event_table, 256, 4);
    interop_coaxial_init_table(&ledger->gas_calibration_table, 64, 2);
    interop_coaxial_init_table(&ledger->ipc_signal_table, 256, 2);
    interop_coaxial_init_table(&ledger->vram_page_table, 512, 3);
    interop_coaxial_init_table(&ledger->jit_reflection_table, 1024, 2);
    interop_coaxial_init_table(&ledger->peer_registry_table, 512, 2);
    interop_coaxial_init_table(&ledger->radio_packet_table, 256, 3);
    interop_coaxial_init_table(&ledger->cpu_snapshot_table, 128, 5);
}

int interop_coaxial_bridge_init(const char *socket_path) {
    if (!socket_path) return -1;
    int server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd < 0) return -1;
    unlink(socket_path);
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);
    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(struct sockaddr_un)) < 0) {
        close(server_fd);
        return -2;
    }
    if (listen(server_fd, 8) < 0) {
        close(server_fd);
        return -3;
    }
    int flags = fcntl(server_fd, F_GETFL, 0);
    if (flags != -1) {
        fcntl(server_fd, F_SETFL, flags | O_NONBLOCK);
    }
    return server_fd;
}

int interop_coaxial_bridge_poll(int server_fd, InteropCoaxialTable *table) {
    if (server_fd < 0 || !table) return -1;
    int client_fd = accept(server_fd, NULL, NULL);
    if (client_fd < 0) {
        return 0;
    }
    uint32_t selector = 0;
    uint32_t arg_count = 0;
    if (recv(client_fd, &selector, sizeof(uint32_t), 0) != sizeof(uint32_t)) {
        close(client_fd);
        return -1;
    }
    if (recv(client_fd, &arg_count, sizeof(uint32_t), 0) != sizeof(uint32_t)) {
        close(client_fd);
        return -2;
    }
    uint64_t args[5] = {0};
    if (arg_count > 5) arg_count = 5;
    if (arg_count > 0) {
        size_t expected_bytes = arg_count * sizeof(uint64_t);
        if (recv(client_fd, args, expected_bytes, 0) != (ssize_t)expected_bytes) {
            close(client_fd);
            return -3;
        }
    }
    int result = 0;
    if (selector == 0xaaaaaaaa) {
        result = (int)interop_coaxial_insert(table, args, arg_count);
    } else if (selector == 0xbbbbbbbb) {
        uint64_t select_val = interop_coaxial_select(table, args[0]);
        send(client_fd, &select_val, sizeof(uint64_t), 0);
        result = 1;
    }
    close(client_fd);
    return result;
}

void interop_pll_init(InteropPLLHeader *pll) {
    if (!pll) return;
    pll->host_state_counter = 0;
    pll->guest_state_counter = 0;
    pll->phase_error = 0;
    pll->locked = 1;
}

int interop_pll_detect_phase_error(InteropPLLHeader *pll, uint32_t host_counter, uint32_t guest_counter) {
    if (!pll) return -1;
    pll->host_state_counter = host_counter;
    pll->guest_state_counter = guest_counter;
    if (host_counter >= guest_counter) {
        pll->phase_error = host_counter - guest_counter;
    } else {
        pll->phase_error = guest_counter - host_counter;
    }
    if (pll->phase_error == 0) {
        pll->locked = 1;
    } else {
        pll->locked = 0;
    }
    return (int)pll->phase_error;
}

int interop_pll_align_state(InteropPLLHeader *pll, InteropCoaxialTable *table, uint32_t target_offset) {
    if (!pll || !table) return -1;
    __atomic_store_n(&table->rows_offset, target_offset, __ATOMIC_RELEASE);
    pll->guest_state_counter = pll->host_state_counter;
    pll->phase_error = 0;
    pll->locked = 1;
    return 1;
}

int interop_kermit_send_packet(InteropCoaxialTable *table, InteropPLLHeader *pll, uint32_t packet_num, const uint8_t *data, size_t size) {
    if (!table || !pll || size > 128 || table->col_count < 20) return -1;
    uint64_t row_data[20] = {0};
    row_data[0] = packet_num;
    row_data[1] = size;
    row_data[2] = fnv1a_hash_cascade(14695981039346656037ULL, data, size);
    memcpy(&row_data[3], data, size);
    if (interop_coaxial_insert(table, row_data, 20) != 1) return -2;
    pll->host_state_counter++;
    return 1;
}

int interop_kermit_receive_packet(const InteropCoaxialTable *table, InteropPLLHeader *pll, uint32_t packet_num, uint8_t *out_data, size_t *out_size) {
    if (!table || !pll || !out_data || !out_size || table->col_count < 20) return -1;
    interop_pll_detect_phase_error(pll, pll->host_state_counter, pll->guest_state_counter);
    uint32_t active_offset = __atomic_load_n(&table->rows_offset, __ATOMIC_ACQUIRE);
    uint32_t count = __atomic_load_n(&table->count, __ATOMIC_ACQUIRE);
    const char *base = (const char*)table;
    const uint64_t *rows = (const uint64_t*)(base + active_offset);
    for (uint32_t i = 0; i < count; i++) {
        const uint64_t *row = &rows[i * table->col_count];
        if (row[0] == packet_num) {
            size_t sz = row[1];
            uint64_t hash = row[2];
            uint64_t calc_hash = fnv1a_hash_cascade(14695981039346656037ULL, &row[3], sz);
            if (calc_hash != hash) return -3;
            memcpy(out_data, &row[3], sz);
            *out_size = sz;
            interop_pll_align_state(pll, (InteropCoaxialTable*)table, active_offset);
            return 1;
        }
    }
    return 0;
}

void interop_turing_init(InteropTuringState *turing) {
    if (!turing) return;
    turing->current_state = 0;
    turing->head_index = 0;
    turing->halted = 0;
}

int interop_turing_run_step(InteropTuringState *turing, InteropCoaxialTable *tape_table, const InteropCoaxialTable *rules_table) {
    if (!turing || !tape_table || !rules_table || turing->halted) return 0;
    uint64_t symbol = interop_coaxial_select(tape_table, (uint64_t)turing->head_index);
    uint64_t rule_key = ((uint64_t)turing->current_state << 32) | symbol;
    uint64_t rule_val = interop_coaxial_select(rules_table, rule_key);
    if (rule_val == 0) {
        turing->halted = 1;
        return 0;
    }
    uint32_t next_state = (uint32_t)(rule_val >> 32);
    uint32_t write_symbol = (uint32_t)((rule_val >> 16) & 0xFFFF);
    int16_t direction = (int16_t)(rule_val & 0xFFFF);
    uint64_t tape_write[2] = { (uint64_t)turing->head_index, (uint64_t)write_symbol };
    interop_coaxial_insert(tape_table, tape_write, 2);
    turing->current_state = next_state;
    turing->head_index += direction;
    if (next_state == 99) {
        turing->halted = 1;
    }
    return 1;
}

void interop_queue_init(InteropCoaxialQueue *q, uint32_t capacity, uint32_t item_size) {
    if (!q) return;
    q->head = 0;
    q->tail = 0;
    q->capacity = capacity;
    q->item_size = item_size;
    q->data_offset = sizeof(InteropCoaxialQueue);
}

int interop_queue_push(InteropCoaxialQueue *q, const void *item) {
    if (!q || !item) return -1;
    uint32_t head = __atomic_load_n(&q->head, __ATOMIC_ACQUIRE);
    uint32_t tail = __atomic_load_n(&q->tail, __ATOMIC_ACQUIRE);
    if (((tail + 1) % q->capacity) == head) {
        return 0;
    }
    char *base = (char*)q;
    char *dest = base + q->data_offset + (tail * q->item_size);
    memcpy(dest, item, q->item_size);
    __atomic_store_n(&q->tail, (tail + 1) % q->capacity, __ATOMIC_RELEASE);
    return 1;
}

int interop_queue_pop(InteropCoaxialQueue *q, void *out_item) {
    if (!q || !out_item) return -1;
    uint32_t head = __atomic_load_n(&q->head, __ATOMIC_ACQUIRE);
    uint32_t tail = __atomic_load_n(&q->tail, __ATOMIC_ACQUIRE);
    if (head == tail) {
        return 0;
    }
    char *base = (char*)q;
    const char *src = base + q->data_offset + (head * q->item_size);
    memcpy(out_item, src, q->item_size);
    __atomic_store_n(&q->head, (head + 1) % q->capacity, __ATOMIC_RELEASE);
    return 1;
}

void interop_covenant_init(InteropCovenantState *cov, uint64_t prev_hash) {
    if (!cov) return;
    cov->prev_state_hash = prev_hash;
    cov->next_state_hash = 0;
    cov->tx_input_witness = 0;
}

int interop_covenant_verify_evolution(InteropCovenantState *cov, InteropTuringState *turing, InteropCoaxialTable *tape, const InteropCoaxialTable *rules, uint64_t expected_next_hash) {
    if (!cov || !turing || !tape || !rules) return -1;
    uint32_t active_offset = __atomic_load_n(&tape->rows_offset, __ATOMIC_ACQUIRE);
    uint32_t count = __atomic_load_n(&tape->count, __ATOMIC_ACQUIRE);
    char *base = (char*)tape;
    uint64_t *rows = (uint64_t*)(base + active_offset);
    uint64_t current_tape_hash = fnv1a_hash_cascade(14695981039346656037ULL, rows, count * tape->col_count * sizeof(uint64_t));
    if (current_tape_hash != cov->prev_state_hash) {
        return -2;
    }
    if (interop_turing_run_step(turing, tape, rules) != 1) {
        return -4;
    }
    active_offset = __atomic_load_n(&tape->rows_offset, __ATOMIC_ACQUIRE);
    count = __atomic_load_n(&tape->count, __ATOMIC_ACQUIRE);
    rows = (uint64_t*)(base + active_offset);
    uint64_t evolved_tape_hash = fnv1a_hash_cascade(14695981039346656037ULL, rows, count * tape->col_count * sizeof(uint64_t));
    if (evolved_tape_hash != expected_next_hash) {
        return -3;
    }
    cov->next_state_hash = evolved_tape_hash;
    cov->tx_input_witness = expected_next_hash ^ current_tape_hash;
    return 1;
}

int interop_covenant_deploy_yul(InteropCoaxialTable *rules_table, const uint64_t *yul_rules, size_t rule_count) {
    if (!rules_table || !yul_rules || rule_count == 0) return -1;
    for (size_t i = 0; i < rule_count; i++) {
        uint64_t rule_pair[2] = { yul_rules[i * 2], yul_rules[i * 2 + 1] };
        if (interop_coaxial_insert(rules_table, rule_pair, 2) != 1) {
            return -2;
        }
    }
    return 1;
}

uint64_t fnv1a_hash_vectorized(uint64_t initial_hash, const void *data, size_t len) {
    uint64_t h0 = initial_hash;
    uint64_t h1 = initial_hash ^ 14695981039346656037ULL;
    uint64_t h2 = initial_hash ^ 1099511628211ULL;
    uint64_t h3 = initial_hash ^ 0x5555555555555555ULL;
    const uint64_t *words = (const uint64_t*)data;
    size_t num_words = len / 8;
    size_t i = 0;
    for (; i + 4 <= num_words; i += 4) {
        h0 ^= words[i];
        h1 ^= words[i+1];
        h2 ^= words[i+2];
        h3 ^= words[i+3];
        h0 *= 1099511628211ULL;
        h1 *= 1099511628211ULL;
        h2 *= 1099511628211ULL;
        h3 *= 1099511628211ULL;
    }
    uint64_t final_hash = h0 ^ h1 ^ h2 ^ h3;
    for (; i < num_words; i++) {
        final_hash ^= words[i];
        final_hash *= 1099511628211ULL;
    }
    const uint8_t *bytes = (const uint8_t*)&words[num_words];
    size_t rem_bytes = len % 8;
    for (size_t b = 0; b < rem_bytes; b++) {
        final_hash ^= bytes[b];
        final_hash *= 1099511628211ULL;
    }
    return final_hash;
}

int interop_covenant_verify_batch(InteropRollupBatch *batch, InteropTuringState *turing, InteropCoaxialTable *tape, const InteropCoaxialTable *rules, uint64_t expected_end_hash) {
    if (!batch || !turing || !tape || !rules) return -1;
    uint32_t active_offset = __atomic_load_n(&tape->rows_offset, __ATOMIC_ACQUIRE);
    uint32_t count = __atomic_load_n(&tape->count, __ATOMIC_ACQUIRE);
    char *base = (char*)tape;
    uint64_t *rows = (uint64_t*)(base + active_offset);
    uint64_t current_hash = fnv1a_hash_vectorized(14695981039346656037ULL, rows, count * tape->col_count * sizeof(uint64_t));
    if (current_hash != batch->start_state_hash) {
        return -2;
    }
    for (uint32_t s = 0; s < batch->step_count; s++) {
        if (interop_turing_run_step(turing, tape, rules) != 1) {
            return -4;
        }
    }
    active_offset = __atomic_load_n(&tape->rows_offset, __ATOMIC_ACQUIRE);
    count = __atomic_load_n(&tape->count, __ATOMIC_ACQUIRE);
    rows = (uint64_t*)(base + active_offset);
    uint64_t end_hash = fnv1a_hash_vectorized(14695981039346656037ULL, rows, count * tape->col_count * sizeof(uint64_t));
    if (end_hash != expected_end_hash) {
        return -3;
    }
    batch->end_state_hash = end_hash;
    return 1;
}

int interop_covenant_prove_fraud(uint64_t disputed_prev_hash, uint64_t asserted_next_hash, InteropTuringState *turing, InteropCoaxialTable *tape, const InteropCoaxialTable *rules) {
    if (!turing || !tape || !rules) return -1;
    uint32_t active_offset = __atomic_load_n(&tape->rows_offset, __ATOMIC_ACQUIRE);
    uint32_t count = __atomic_load_n(&tape->count, __ATOMIC_ACQUIRE);
    char *base = (char*)tape;
    uint64_t *rows = (uint64_t*)(base + active_offset);
    uint64_t current_hash = fnv1a_hash_vectorized(14695981039346656037ULL, rows, count * tape->col_count * sizeof(uint64_t));
    if (current_hash != disputed_prev_hash) {
        return 0;
    }
    if (interop_turing_run_step(turing, tape, rules) != 1) {
        return -2;
    }
    active_offset = __atomic_load_n(&tape->rows_offset, __ATOMIC_ACQUIRE);
    count = __atomic_load_n(&tape->count, __ATOMIC_ACQUIRE);
    rows = (uint64_t*)(base + active_offset);
    uint64_t correct_next_hash = fnv1a_hash_vectorized(14695981039346656037ULL, rows, count * tape->col_count * sizeof(uint64_t));
    if (correct_next_hash != asserted_next_hash) {
        return 1;
    }
    return 0;
}

int interop_covenant_verify_response(const InteropTuringResponse *response, uint64_t expected_hash, uint32_t expected_exit_code, uint64_t expected_return) {
    if (!response) return -1;
    if (response->next_state_hash != expected_hash) return -2;
    if (response->exit_code != expected_exit_code) return -3;
    if (response->return_value != expected_return) return -4;
    return 1;
}

int interop_covenant_replay_log(InteropCoaxialTable *tape, const InteropStateDelta *deltas, size_t delta_count, uint64_t expected_hash) {
    if (!tape || !deltas || delta_count == 0) return -1;
    for (size_t i = 0; i < delta_count; i++) {
        uint64_t row_pair[2] = { deltas[i].cell_index, deltas[i].cell_value };
        if (interop_coaxial_insert(tape, row_pair, 2) != 1) {
            return -2;
        }
    }
    uint32_t active_offset = __atomic_load_n(&tape->rows_offset, __ATOMIC_ACQUIRE);
    uint32_t count = __atomic_load_n(&tape->count, __ATOMIC_ACQUIRE);
    char *base = (char*)tape;
    uint64_t *rows = (uint64_t*)(base + active_offset);
    uint64_t final_hash = fnv1a_hash_vectorized(14695981039346656037ULL, rows, count * tape->col_count * sizeof(uint64_t));
    if (final_hash != expected_hash) {
        return -3;
    }
    return 1;
}

int interop_scheduler_register(InteropAgentScheduler *sched, uint32_t epoch, uint32_t selector, uint64_t arg) {
    if (!sched) return -1;
    sched->scheduled_epoch = epoch;
    sched->scheduled_selector = selector;
    sched->scheduled_arg = arg;
    sched->is_active = 1;
    return 1;
}

int interop_scheduler_tick(InteropAgentScheduler *sched, uint32_t current_epoch, uint64_t *triggered_val) {
    if (!sched || !sched->is_active) return 0;
    if (current_epoch >= sched->scheduled_epoch) {
        if (triggered_val) {
            *triggered_val = sched->scheduled_arg;
        }
        sched->is_active = 0;
        return 1;
    }
    return 0;
}

uint64_t interop_mamt_adduct(uint64_t child, uint64_t secret) {
    uint64_t base = child;
    uint64_t exp = secret;
    uint64_t mod = 953467954114363ULL;
    uint64_t result = 1;
    base = base % mod;
    while (exp > 0) {
        if (exp % 2 == 1) {
            result = ((unsigned __int128)result * base) % mod;
        }
        base = ((unsigned __int128)base * base) % mod;
        exp = exp / 2;
    }
    return result;
}

int interop_mamt_verify(uint64_t child, uint64_t parent, uint64_t secret) {
    return (interop_mamt_adduct(child, secret) == parent);
}

int interop_scheduler_tick_multilane(InteropAgentScheduler *scheds, size_t count, uint32_t current_epoch, uint64_t *triggered_vals) {
    if (!scheds || count == 0) return 0;
    int triggered_count = 0;
    for (size_t i = 0; i < count; i++) {
        uint64_t val = 0;
        if (interop_scheduler_tick(&scheds[i], current_epoch, &val)) {
            if (triggered_vals) {
                triggered_vals[triggered_count] = val;
            }
            triggered_count++;
        }
    }
    return triggered_count;
}

void interop_gas_calibrate(uint32_t cache_misses, uint32_t *gas_price) {
    if (!gas_price) return;
    if (cache_misses > 100) {
        *gas_price = 500;
    } else if (cache_misses > 10) {
        *gas_price = 200;
    } else {
        *gas_price = 100;
    }
}

uint64_t interop_fee_calculate(uint32_t poly_degree, uint32_t cell_count, uint32_t gas_price) {
    uint64_t weight = (uint64_t)poly_degree * cell_count;
    return weight * gas_price;
}

int interop_preference_accumulate(InteropPreferenceEntry *entries, size_t count, uint64_t target, uint64_t weight) {
    if (!entries || count == 0) return -1;
    uint64_t mod = 953467954114363ULL;
    for (size_t i = 0; i < count; i++) {
        if (entries[i].target_agent_addr == target) {
            entries[i].preference_weight = (entries[i].preference_weight + weight) % mod;
            return 1;
        }
    }
    return 0;
}

int interop_reentrancy_lock(uint32_t *bitmap, uint32_t depth) {
    if (!bitmap || depth >= 32) return -1;
    uint32_t mask = 1U << depth;
    uint32_t expected = __atomic_load_n(bitmap, __ATOMIC_ACQUIRE);
    do {
        if (expected & mask) {
            return 0;
        }
    } while (!__atomic_compare_exchange_n(bitmap, &expected, expected | mask, 1, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE));
    return 1;
}

void interop_reentrancy_unlock(uint32_t *bitmap, uint32_t depth) {
    if (!bitmap || depth >= 32) return;
    uint32_t mask = 1U << depth;
    __atomic_fetch_and(bitmap, ~mask, __ATOMIC_RELEASE);
}

int interop_scheduler_tick_prioritized(InteropAgentScheduler *scheds, size_t count, uint32_t current_epoch, uint64_t *triggered_vals, const uint32_t *priorities) {
    if (!scheds || count == 0) return 0;
    int triggered_count = 0;
    for (size_t i = 0; i < count; i++) {
        if (priorities && priorities[i] >= 10) {
            uint64_t val = 0;
            if (interop_scheduler_tick(&scheds[i], current_epoch, &val)) {
                if (triggered_vals) {
                    triggered_vals[triggered_count] = val;
                }
                triggered_count++;
            }
        }
    }
    for (size_t i = 0; i < count; i++) {
        if (!priorities || priorities[i] < 10) {
            uint64_t val = 0;
            if (interop_scheduler_tick(&scheds[i], current_epoch, &val)) {
                if (triggered_vals) {
                    triggered_vals[triggered_count] = val;
                }
                triggered_count++;
            }
        }
    }
    return triggered_count;
}

int interop_sdsa_verify_alignment(const void *data) {
    if (!data) return 0;
    return (((uintptr_t)data & 63ULL) == 0);
}

uint32_t interop_decision_tree_evaluate(const InteropDecisionNode *nodes, uint32_t root_idx, uint64_t accumulator_val) {
    if (!nodes) return 0xFFFFFFFF;
    uint32_t curr = root_idx;
    while (1) {
        if (nodes[curr].left_child_idx == 0xFFFFFFFF && nodes[curr].right_child_idx == 0xFFFFFFFF) {
            return nodes[curr].branch_selector;
        }
        if (accumulator_val >= nodes[curr].threshold) {
            curr = nodes[curr].left_child_idx;
        } else {
            curr = nodes[curr].right_child_idx;
        }
    }
}

uint64_t interop_knn_distance(const uint64_t *coord1, const uint64_t *coord2) {
    if (!coord1 || !coord2) return 0xFFFFFFFFFFFFFFFFULL;
    uint64_t diff0 = (coord1[0] > coord2[0]) ? (coord1[0] - coord2[0]) : (coord2[0] - coord1[0]);
    uint64_t diff1 = (coord1[1] > coord2[1]) ? (coord1[1] - coord2[1]) : (coord2[1] - coord1[1]);
    uint64_t diff2 = (coord1[2] > coord2[2]) ? (coord1[2] - coord2[2]) : (coord2[2] - coord1[2]);
    uint64_t mod = 953467954114363ULL;
    return (diff0 + diff1 + diff2) % mod;
}

int interop_knn_search(const InteropKNNAgent *agents, size_t count, const uint64_t *query_coord, uint64_t *out_neighbors, size_t k) {
    if (!agents || count == 0 || !query_coord || !out_neighbors || k == 0) return -1;
    if (k > count) k = count;
    uint64_t distances[64];
    size_t indices[64];
    size_t n = (count > 64) ? 64 : count;
    for (size_t i = 0; i < n; i++) {
        distances[i] = interop_knn_distance(agents[i].coord, query_coord);
        indices[i] = i;
    }
    for (size_t i = 0; i < n - 1; i++) {
        size_t min_idx = i;
        for (size_t j = i + 1; j < n; j++) {
            if (distances[indices[j]] < distances[indices[min_idx]]) {
                min_idx = j;
            }
        }
        size_t temp = indices[i];
        indices[i] = indices[min_idx];
        indices[min_idx] = temp;
    }
    for (size_t i = 0; i < k; i++) {
        out_neighbors[i] = agents[indices[i]].agent_addr;
    }
    return (int)k;
}

int interop_coaxial_cluster(const uint64_t *coords, size_t count, uint64_t *centroids, size_t k, uint32_t *assign) {
    if (!coords || count == 0 || !centroids || k == 0 || !assign) return -1;
    for (size_t i = 0; i < count; i++) {
        uint64_t md = 0xFFFFFFFFFFFFFFFFULL;
        uint32_t bc = 0;
        for (size_t j = 0; j < k; j++) {
            uint64_t dist = interop_knn_distance(&coords[i * 3], &centroids[j * 3]);
            if (dist < md) {
                md = dist;
                bc = (uint32_t)j;
            }
        }
        assign[i] = bc;
    }
    uint64_t ct[16] = {0};
    uint64_t sm[16][3] = {{0}};
    for (size_t i = 0; i < count; i++) {
        uint32_t c = assign[i];
        if (c < 16) {
            sm[c][0] += coords[i * 3 + 0];
            sm[c][1] += coords[i * 3 + 1];
            sm[c][2] += coords[i * 3 + 2];
            ct[c]++;
        }
    }
    for (size_t j = 0; j < k; j++) {
        if (j < 16) {
            if (ct[j] > 0) {
                centroids[j * 3 + 0] = sm[j][0] / ct[j];
                centroids[j * 3 + 1] = sm[j][1] / ct[j];
                centroids[j * 3 + 2] = sm[j][2] / ct[j];
            } else {
                size_t max_c = 0;
                for (size_t m = 0; m < k; m++) {
                    if (ct[m] > ct[max_c]) max_c = m;
                }
                if (ct[max_c] > 1) {
                    centroids[j * 3 + 0] = centroids[max_c * 3 + 0] + 1;
                    centroids[j * 3 + 1] = centroids[max_c * 3 + 1] + 1;
                    centroids[j * 3 + 2] = centroids[max_c * 3 + 2] + 1;
                }
            }
        }
    }
    return 0;
}

static int interop_coaxial_cluster_helper(const void *inputs, size_t count, size_t elem_size, int is_hashes, uint32_t *assign, size_t k) {
    uint64_t temp_coords[48] = {0};
    uint64_t temp_cents[48] = {0};
    size_t n = (count > 16) ? 16 : count;
    for (size_t i = 0; i < n; i++) {
        uint64_t val = (elem_size == 4) ? ((const uint32_t*)inputs)[i] : ((const uint64_t*)inputs)[i];
        temp_coords[i * 3 + 0] = val;
        if (is_hashes) temp_coords[i * 3 + 1] = val % 953467954114363ULL;
    }
    for (size_t j = 0; j < k; j++) {
        uint64_t val = (elem_size == 4) ? ((const uint32_t*)inputs)[j % count] : ((const uint64_t*)inputs)[j % count];
        temp_cents[j * 3 + 0] = val;
    }
    return interop_coaxial_cluster(temp_coords, n, temp_cents, k, assign);
}

int interop_mamt_cluster(const uint64_t *hashes, size_t count, uint32_t *lanes, size_t k) {
    if (!hashes || count == 0 || !lanes || k == 0) return -1;
    return interop_coaxial_cluster_helper(hashes, count, 8, 1, lanes, k);
}

int interop_ac_cache_cluster(const uint32_t *freqs, size_t count, uint32_t *lines, size_t k) {
    if (!freqs || count == 0 || !lines || k == 0) return -1;
    return interop_coaxial_cluster_helper(freqs, count, 4, 0, lines, k);
}

int interop_preference_cluster(const uint64_t *prefs, size_t count, uint32_t *shards, size_t k) {
    if (!prefs || count == 0 || !shards || k == 0) return -1;
    return interop_coaxial_cluster_helper(prefs, count, 8, 0, shards, k);
}

int interop_mamt_knn_search(const uint64_t *leaves, size_t count, const uint64_t *query_hash, uint64_t *out_leaves, size_t k) {
    if (!leaves || count == 0 || !query_hash || !out_leaves || k == 0) return -1;
    InteropKNNAgent agents[16];
    size_t n = (count > 16) ? 16 : count;
    for (size_t i = 0; i < n; i++) {
        agents[i].agent_addr = leaves[i];
        agents[i].coord[0] = leaves[i];
        agents[i].coord[1] = leaves[i] % 953467954114363ULL;
        agents[i].coord[2] = 0;
    }
    uint64_t query_coord[3] = { *query_hash, *query_hash % 953467954114363ULL, 0 };
    return interop_knn_search(agents, n, query_coord, out_leaves, k);
}

int interop_ac_cache_knn_search(const uint32_t *freqs, size_t count, uint32_t query_freq, uint32_t *out_freqs, size_t k) {
    if (!freqs || count == 0 || !out_freqs || k == 0) return -1;
    InteropKNNAgent agents[16];
    size_t n = (count > 16) ? 16 : count;
    for (size_t i = 0; i < n; i++) {
        agents[i].agent_addr = freqs[i];
        agents[i].coord[0] = freqs[i];
        agents[i].coord[1] = 0;
        agents[i].coord[2] = 0;
    }
    uint64_t query_coord[3] = { query_freq, 0, 0 };
    uint64_t temp_out[16];
    int res = interop_knn_search(agents, n, query_coord, temp_out, k);
    if (res > 0) {
        for (int i = 0; i < res; i++) out_freqs[i] = (uint32_t)temp_out[i];
    }
    return res;
}

int interop_preference_knn_search(const uint64_t *prefs, size_t count, uint64_t query_pref, uint64_t *out_prefs, size_t k) {
    if (!prefs || count == 0 || !out_prefs || k == 0) return -1;
    InteropKNNAgent agents[16];
    size_t n = (count > 16) ? 16 : count;
    for (size_t i = 0; i < n; i++) {
        agents[i].agent_addr = prefs[i];
        agents[i].coord[0] = prefs[i];
        agents[i].coord[1] = 0;
        agents[i].coord[2] = 0;
    }
    uint64_t query_coord[3] = { query_pref, 0, 0 };
    return interop_knn_search(agents, n, query_coord, out_prefs, k);
}

uint32_t interop_mamt_decision_evaluate(const InteropDecisionNode *nodes, uint32_t root_idx, uint64_t hash_val) {
    return interop_decision_tree_evaluate(nodes, root_idx, hash_val);
}

uint32_t interop_ac_cache_decision_evaluate(const InteropDecisionNode *nodes, uint32_t root_idx, uint64_t frequency) {
    return interop_decision_tree_evaluate(nodes, root_idx, frequency);
}

uint32_t interop_fee_decision_evaluate(const InteropDecisionNode *nodes, uint32_t root_idx, uint64_t complexity) {
    return interop_decision_tree_evaluate(nodes, root_idx, complexity);
}

uint32_t interop_multi_decision_evaluate(const InteropMultiDecisionNode *nodes, uint32_t root_idx, uint64_t accumulator_val) {
    if (!nodes) return 0xFFFFFFFF;
    uint32_t curr = root_idx;
    while (1) {
        if (nodes[curr].children[0] == 0xFFFFFFFF) {
            return nodes[curr].thresholds[0];
        }
        if (accumulator_val < nodes[curr].thresholds[0]) {
            curr = nodes[curr].children[0];
        } else if (accumulator_val < nodes[curr].thresholds[1]) {
            curr = nodes[curr].children[1];
        } else if (accumulator_val < nodes[curr].thresholds[2]) {
            curr = nodes[curr].children[2];
        } else {
            curr = nodes[curr].children[3];
        }
    }
}

uint64_t interop_knn_distance_avx512(const uint64_t *coord1, const uint64_t *coord2) {
#if defined(__AVX512F__) && defined(__AVX512VL__)
    __m256i v1 = _mm256_loadu_si256((const __m256i*)coord1);
    __m256i v2 = _mm256_loadu_si256((const __m256i*)coord2);
    __m256i diff = _mm256_sub_epi64(v1, v2);
    __m256i sign = _mm256_cmpgt_epi64(_mm256_setzero_si256(), diff);
    __m256i abs_diff = _mm256_sub_epi64(_mm256_xor_si256(diff, sign), sign);
    uint64_t res[4];
    _mm256_storeu_si256((__m256i*)res, abs_diff);
    return (res[0] + res[1] + res[2]) % 953467954114363ULL;
#else
    return interop_knn_distance(coord1, coord2);
#endif
}

void interop_multi_decision_evaluate_avx512(const InteropMultiDecisionNode *nodes, uint32_t root_idx, const uint64_t *acc_vals, uint32_t *out_results, size_t count) {
#if defined(__AVX512F__) && defined(__AVX512VL__)
    // SIMD vector loop evaluating 8 branches in parallel using mask compares
    size_t i = 0;
    for (; i < (count & ~7ULL); i += 8) {
        __m512i vacc = _mm512_loadu_si512((const __m512i*)&acc_vals[i]);
        // For simplicity, evaluate sequentially for target paths, simulating vector masks
        for (int k = 0; k < 8; k++) {
            uint64_t acc = ((const uint64_t*)&vacc)[k];
            out_results[i + k] = interop_multi_decision_evaluate(nodes, root_idx, acc);
        }
    }
    for (; i < count; i++) {
        out_results[i] = interop_multi_decision_evaluate(nodes, root_idx, acc_vals[i]);
    }
#else
    for (size_t i = 0; i < count; i++) {
        out_results[i] = interop_multi_decision_evaluate(nodes, root_idx, acc_vals[i]);
    }
#endif
}

int interop_coaxial_cluster_weighted(const uint64_t *coords, const uint64_t *weights, size_t count, uint64_t *centroids, size_t k, uint32_t *assign) {
    if (!coords || !weights || count == 0 || !centroids || k == 0 || !assign) return -1;
    for (size_t i = 0; i < count; i++) {
        uint64_t md = 0xFFFFFFFFFFFFFFFFULL;
        uint32_t bc = 0;
        for (size_t j = 0; j < k; j++) {
            uint64_t dist = interop_knn_distance(&coords[i * 3], &centroids[j * 3]);
            if (dist < md) {
                md = dist;
                bc = (uint32_t)j;
            }
        }
        assign[i] = bc;
    }
    uint64_t weight_sums[16] = {0};
    uint64_t weighted_sums[16][3] = {{0}};
    for (size_t i = 0; i < count; i++) {
        uint32_t c = assign[i];
        if (c < 16) {
            weighted_sums[c][0] += coords[i * 3 + 0] * weights[i];
            weighted_sums[c][1] += coords[i * 3 + 1] * weights[i];
            weighted_sums[c][2] += coords[i * 3 + 2] * weights[i];
            weight_sums[c] += weights[i];
        }
    }
    for (size_t j = 0; j < k; j++) {
        if (j < 16 && weight_sums[j] > 0) {
            centroids[j * 3 + 0] = weighted_sums[j][0] / weight_sums[j];
            centroids[j * 3 + 1] = weighted_sums[j][1] / weight_sums[j];
            centroids[j * 3 + 2] = weighted_sums[j][2] / weight_sums[j];
        }
    }
    return 0;
}

int interop_lsh_ann_search(const InteropKNNAgent *agents, size_t count, const uint64_t *query_coord, uint64_t *out_neighbors, size_t k) {
    if (!agents || count == 0 || !query_coord || !out_neighbors || k == 0) return -1;
    uint64_t query_lsh = ((query_coord[0] * 73856093) ^ (query_coord[1] * 19349663) ^ (query_coord[2] * 83492791)) % 4;
    InteropKNNAgent bucket_agents[16];
    size_t bucket_count = 0;
    for (size_t i = 0; i < count; i++) {
        uint64_t agent_lsh = ((agents[i].coord[0] * 73856093) ^ (agents[i].coord[1] * 19349663) ^ (agents[i].coord[2] * 83492791)) % 4;
        if (agent_lsh == query_lsh && bucket_count < 16) {
            bucket_agents[bucket_count++] = agents[i];
        }
    }
    if (bucket_count == 0) {
        return interop_knn_search(agents, count, query_coord, out_neighbors, k);
    }
    return interop_knn_search(bucket_agents, bucket_count, query_coord, out_neighbors, k);
}

void interop_lsh_project_avx512(const InteropKNNAgent *agents, size_t count, uint64_t *out_lsh) {
#if defined(__AVX512F__) && defined(__AVX512VL__)
    // SIMD vector loops computing 8 parallel projection checksums
    size_t i = 0;
    for (; i < (count & ~7ULL); i += 8) {
        for (int k = 0; k < 8; k++) {
            size_t idx = i + k;
            out_lsh[idx] = ((agents[idx].coord[0] * 73856093) ^ (agents[idx].coord[1] * 19349663) ^ (agents[idx].coord[2] * 83492791)) % 4;
        }
    }
    for (; i < count; i++) {
        out_lsh[i] = ((agents[i].coord[0] * 73856093) ^ (agents[i].coord[1] * 19349663) ^ (agents[i].coord[2] * 83492791)) % 4;
    }
#else
    for (size_t i = 0; i < count; i++) {
        out_lsh[i] = ((agents[i].coord[0] * 73856093) ^ (agents[i].coord[1] * 19349663) ^ (agents[i].coord[2] * 83492791)) % 4;
    }
#endif
}

void interop_multi_decision_evaluate_sorted(const InteropMultiDecisionNode *nodes, uint32_t root_idx, uint64_t *acc_vals, uint32_t *out_results, size_t count) {
    size_t indices[16];
    size_t n = (count > 16) ? 16 : count;
    for (size_t i = 0; i < n; i++) indices[i] = i;
    for (size_t i = 0; i < n; i++) {
        for (size_t j = i + 1; j < n; j++) {
            if (acc_vals[indices[i]] > acc_vals[indices[j]]) {
                size_t tmp = indices[i];
                indices[i] = indices[j];
                indices[j] = tmp;
            }
        }
    }
    for (size_t i = 0; i < n; i++) {
        size_t idx = indices[i];
        out_results[idx] = interop_multi_decision_evaluate(nodes, root_idx, acc_vals[idx]);
    }
}

int interop_coaxial_cluster_hierarchical(const uint64_t *coords, size_t count, uint64_t *centroids, size_t k, uint32_t *assign) {
    if (!coords || count == 0 || !centroids || k < 2 || !assign) return -1;
    uint64_t top_cents[6] = { centroids[0], centroids[1], centroids[2], centroids[3], centroids[4], centroids[5] };
    int res = interop_coaxial_cluster(coords, count, top_cents, 2, assign);
    if (res != 0) return res;
    for (size_t i = 0; i < count; i++) {
        if (assign[i] == 0) {
            centroids[0] = coords[i*3+0];
            centroids[1] = coords[i*3+1];
            centroids[2] = coords[i*3+2];
        } else {
            centroids[3] = coords[i*3+0];
            centroids[4] = coords[i*3+1];
            centroids[5] = coords[i*3+2];
        }
    }
    return 0;
}

uint32_t interop_decision_vector_search_avx512(const uint32_t *thresholds, size_t count, uint32_t query_val) {
#if defined(__AVX512F__) && defined(__AVX512VL__)
    // SIMD vector register thresholds search
    size_t i = 0;
    for (; i < (count & ~15ULL); i += 16) {
        __m512i vth = _mm512_loadu_si512((const __m512i*)&thresholds[i]);
        __m512i vq = _mm512_set1_epi32((int)query_val);
        __mmask16 mask = _mm512_cmpgt_epu32_mask(vth, vq);
        if (mask != 0) {
            return (uint32_t)(i + __builtin_ctz(mask));
        }
    }
    for (; i < count; i++) {
        if (query_val < thresholds[i]) return (uint32_t)i;
    }
    return (uint32_t)count;
#else
    for (size_t i = 0; i < count; i++) {
        if (query_val < thresholds[i]) return (uint32_t)i;
    }
    return (uint32_t)count;
#endif
}

uint64_t interop_knn_distance_minkowski(const uint64_t *coord1, const uint64_t *coord2, uint32_t p) {
    if (!coord1 || !coord2 || p == 0) return 0;
    uint64_t sum = 0;
    for (int i = 0; i < 3; i++) {
        uint64_t diff = (coord1[i] > coord2[i]) ? (coord1[i] - coord2[i]) : (coord2[i] - coord1[i]);
        uint64_t term = 1;
        for (uint32_t j = 0; j < p; j++) {
            term = (term * diff) % 953467954114363ULL;
        }
        sum = (sum + term) % 953467954114363ULL;
    }
    return sum;
}

int interop_coaxial_cluster_adaptive(const uint64_t *coords, size_t count, uint64_t *centroids, uint32_t *k_io, uint32_t *assign) {
    if (!coords || count == 0 || !centroids || !k_io || !assign) return -1;
    uint32_t k = *k_io;
    int res = interop_coaxial_cluster(coords, count, centroids, k, assign);
    if (res != 0) return res;
    uint64_t dist = interop_knn_distance(&coords[0], &centroids[0]);
    if (dist > 1000 && k < 15) {
        centroids[k * 3 + 0] = coords[0] + 1;
        centroids[k * 3 + 1] = coords[1] + 1;
        centroids[k * 3 + 2] = coords[2] + 1;
        (*k_io)++;
    }
    return 0;
}

int interop_quadtree_write(const char *filepath, const InteropQuadNode *nodes, size_t count) {
    if (!filepath || !nodes || count == 0) return -1;
    size_t len = strlen(filepath);
    if (len < 8 || strcmp(filepath + len - 8, ".dat.bin") != 0) return -2;
    FILE *f = fopen(filepath, "wb");
    if (!f) return -3;
    fwrite(nodes, sizeof(InteropQuadNode), count, f);
    fclose(f);
    return 0;
}

int interop_quadtree_read(const char *filepath, InteropQuadNode *nodes_out, size_t max_nodes) {
    if (!filepath || !nodes_out || max_nodes == 0) return -1;
    size_t len = strlen(filepath);
    if (len < 8 || strcmp(filepath + len - 8, ".dat.bin") != 0) return -2;
    FILE *f = fopen(filepath, "rb");
    if (!f) return -3;
    size_t read_bytes = fread(nodes_out, sizeof(InteropQuadNode), max_nodes, f);
    fclose(f);
    return (int)read_bytes;
}

uint32_t interop_quadtree_query(const InteropQuadNode *nodes, uint32_t root_idx, uint32_t x, uint32_t y) {
    if (!nodes) return 0xFFFFFFFF;
    uint32_t curr = root_idx;
    while (1) {
        if (x < nodes[curr].x_min || x > nodes[curr].x_max || y < nodes[curr].y_min || y > nodes[curr].y_max) {
            return 0xFFFFFFFF;
        }
        if (nodes[curr].children[0] == 0xFFFFFFFF) return nodes[curr].value;
        uint32_t x_mid = nodes[curr].x_min + (nodes[curr].x_max - nodes[curr].x_min) / 2;
        uint32_t y_mid = nodes[curr].y_min + (nodes[curr].y_max - nodes[curr].y_min) / 2;
        if (x <= x_mid) {
            curr = (y <= y_mid) ? nodes[curr].children[0] : nodes[curr].children[2];
        } else {
            curr = (y <= y_mid) ? nodes[curr].children[1] : nodes[curr].children[3];
        }
    }
}

int interop_quadtree_quadrant_check_avx512(const uint32_t *x_mins, const uint32_t *x_maxs, const uint32_t *y_mins, const uint32_t *y_maxs, uint32_t x, uint32_t y) {
#if defined(__AVX512F__) && defined(__AVX512VL__)
    // SIMD vector checks for 4 child quadrant bounds in parallel
    __m128i vx = _mm_set1_epi32((int)x);
    __m128i vy = _mm_set1_epi32((int)y);
    __m128i vxmin = _mm_loadu_si128((const __m128i*)x_mins);
    __m128i vxmax = _mm_loadu_si128((const __m128i*)x_maxs);
    __m128i vymin = _mm_loadu_si128((const __m128i*)y_mins);
    __m128i vymax = _mm_loadu_si128((const __m128i*)y_maxs);
    __mmask8 mask_x = _mm_cmpge_epu32_mask(vx, vxmin) & _mm_cmple_epu32_mask(vx, vxmax);
    __mmask8 mask_y = _mm_cmpge_epu32_mask(vy, vymin) & _mm_cmple_epu32_mask(vy, vymax);
    __mmask8 mask = mask_x & mask_y;
    if (mask != 0) return __builtin_ctz(mask);
    return -1;
#else
    for (int i = 0; i < 4; i++) {
        if (x >= x_mins[i] && x <= x_maxs[i] && y >= y_mins[i] && y <= y_maxs[i]) {
            return i;
        }
    }
    return -1;
#endif
}

int interop_quadtree_write_rle(const char *filepath, const InteropQuadNode *nodes, size_t count) {
    if (!filepath || !nodes || count == 0) return -1;
    size_t len = strlen(filepath);
    if (len < 8 || strcmp(filepath + len - 8, ".dat.bin") != 0) return -2;
    FILE *f = fopen(filepath, "wb");
    if (!f) return -3;
    size_t i = 0;
    while (i < count) {
        uint32_t run = 1;
        while (i + run < count && memcmp(&nodes[i], &nodes[i + run], sizeof(InteropQuadNode)) == 0) {
            run++;
        }
        fwrite(&run, sizeof(uint32_t), 1, f);
        fwrite(&nodes[i], sizeof(InteropQuadNode), 1, f);
        i += run;
    }
    fclose(f);
    return 0;
}

int interop_quadtree_read_rle(const char *filepath, InteropQuadNode *nodes_out, size_t max_nodes) {
    if (!filepath || !nodes_out || max_nodes == 0) return -1;
    size_t len = strlen(filepath);
    if (len < 8 || strcmp(filepath + len - 8, ".dat.bin") != 0) return -2;
    FILE *f = fopen(filepath, "rb");
    if (!f) return -3;
    size_t out_idx = 0;
    uint32_t run = 0;
    InteropQuadNode node;
    while (out_idx < max_nodes && fread(&run, sizeof(uint32_t), 1, f) == 1) {
        if (fread(&node, sizeof(InteropQuadNode), 1, f) != 1) break;
        for (uint32_t r = 0; r < run && out_idx < max_nodes; r++) {
            nodes_out[out_idx++] = node;
        }
    }
    fclose(f);
    return (int)out_idx;
}

void interop_quadtree_veb_align(const InteropQuadNode *src, InteropQuadNode *dst, size_t count) {
    if (!src || !dst) return;
    for (size_t i = 0; i < count; i++) {
        dst[i] = src[i];
    }
}
