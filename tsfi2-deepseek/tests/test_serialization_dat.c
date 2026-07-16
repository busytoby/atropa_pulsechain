#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>
#include "tsfi_dat.h"

#define MAX_STATE_RECORDS 100

#define ANVIL_IP "127.0.0.1"
#define ANVIL_PORT 8545

static char g_g15d_addr[128] = {0};
static char g_ncr_addr[128] = {0};

// G-15D Register offset maps matching g15d_rail.yul
#define REG_G15D_STEP            0x00
#define REG_G15D_PC              0x01
#define REG_G15D_SECTOR_OFFSET   0x10
#define REG_G15D_RELOC_BASE      0x11
#define REG_G15D_PREFETCH        0x12
#define REG_G15D_DOUBLE_PREC     0x13
#define REG_G15D_ACC_SIGN        0x14
#define REG_G15D_BASE            0x50
#define REG_G15D_DIRECTION       0x51
#define REG_G15D_BASE_OFFSET     0x52
#define REG_G15D_CLOCK_SOURCE    0x53
#define REG_G15D_HALTED          0x54

// NCR 304 Register offset maps matching ncr304_rail.yul
#define REG_NCR304_TAPE_SEQ      0x20
#define REG_NCR304_PARITY_CHK    0x21
#define REG_NCR304_RETRY_COUNT   0x22
#define REG_NCR304_WRITE_LOCK    0x23
#define REG_NCR304_WARN_IND      0x24
#define REG_NCR304_MERGE_VAL     0x25
#define REG_NCR304_CHK_ERR       0x26
#define REG_NCR304_PARITY_CODE   0x27
#define REG_NCR304_RETRY_LIMIT   0x28
#define REG_NCR304_IO_BUSY       0x29

// Constant configuration parameters of the single Red/Black 2-chip system shared globally
typedef struct {
    uint32_t write_lock_key;               // NCR 304 Tape write-protection key
    uint32_t block_density_selector;       // NCR 304 tape density configuration key
    uint32_t telemetry_block_size_validation;  // NCR 304 hardware block size validator / retry limit
    uint32_t sort_key_offset_pointer;      // NCR 304 comparator sorting offset pointer
    uint32_t base_index_modulus;           // G-15D Auto-modulo register limits
    uint32_t key_mask_filter;              // NCR 304 Comparator Key filter
    uint32_t memory_guard_boundary;        // G-15D physical drum protected guard boundary
    uint32_t timing_cycle_interrupt_mask;  // G-15D Mainframe timing interrupt mask / collision limit
    uint32_t segment_buffer_frame_size;    // NCR 304 multi-record segment buffer size / write lock boundary
    uint32_t overflow_intercept_target_address; // G-15D redirection pointer for math errors
    uint16_t telemetry_signal_warning_level; // NCR 304 tape read signal warning threshold
    uint8_t config_padding[6];             // Struct alignment padding to ensure 8-byte boundary
} DualRailSystemConfig;

// Mainframe DAT file header block (aligned to 8-byte boundary)
typedef struct {
    uint32_t dat_magic_signature;          // Magic number identifying the DAT file (e.g. 0x44415421 - "DAT!")
    uint32_t dat_version;                  // Version of the DAT layout schema
} VMStateHeader;

// Dynamic telemetry trace record structure - containing ONLY core EVM execution telemetry
typedef struct {
    uint32_t step;
    uint32_t pc;
    uint32_t sp;
    uint32_t asp;
    uint32_t halted;
    uint32_t debug_step_mode;          // Single-step execution toggle
    uint32_t debug_call_stack_depth;   // Diagnostic call stack depth tracking
    uint64_t debug_gas_remaining;      // Remaining gas budget
    char debug_current_contract[64];   // Target contract name boundary
} VMStateRecord;

// Global Yul virtual hardware storage array (registers moved from C struct to Yul hardware address space)
static uint64_t g_yul_storage[256] = {0};

// Single global instance of the Red/Black 2-rail hardware configuration parameters
static DualRailSystemConfig g_global_hardware_config = {
    .write_lock_key = 0x19571001,
    .block_density_selector = 0x0200,
    .telemetry_block_size_validation = 3,
    .sort_key_offset_pointer = 0x24,
    .base_index_modulus = 0x0010,
    .key_mask_filter = 0x000FFFFF,
    .memory_guard_boundary = 0xFFFF0000,
    .timing_cycle_interrupt_mask = 20,
    .segment_buffer_frame_size = 0x1000,
    .overflow_intercept_target_address = 0x1100,
    .telemetry_signal_warning_level = 0x0A,
    .config_padding = {0}
};

static int connect_to_anvil() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) return -1;
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(ANVIL_PORT);
    inet_pton(AF_INET, ANVIL_IP, &serv_addr.sin_addr);
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        close(sockfd);
        return -1;
    }
    return sockfd;
}

static void send_rpc_request(const char *payload, char *response_buf, size_t max_len) {
    int sockfd = connect_to_anvil();
    assert(sockfd >= 0);
    char http_request[150000];
    snprintf(http_request, sizeof(http_request),
             "POST / HTTP/1.1\r\n"
             "Host: %s:%d\r\n"
             "Content-Type: application/json\r\n"
             "Content-Length: %zu\r\n"
             "Connection: close\r\n"
             "\r\n"
             "%s",
             ANVIL_IP, ANVIL_PORT, strlen(payload), payload);
    ssize_t sent = write(sockfd, http_request, strlen(http_request));
    assert(sent == (ssize_t)strlen(http_request));
    memset(response_buf, 0, max_len);
    size_t total_read = 0;
    while (1) {
        ssize_t n = read(sockfd, response_buf + total_read, max_len - 1 - total_read);
        if (n <= 0) break;
        total_read += n;
        if (total_read >= max_len - 1) break;
    }
    response_buf[total_read] = '\0';
    close(sockfd);
}

static void compile_yul_to_bytecode(const char *yul_path, char *out_hex, size_t max_len) {
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "solc --strict-assembly %s --bin 2>/dev/null", yul_path);
    FILE *fp = popen(cmd, "r");
    assert(fp != NULL);

    char line[8192];
    out_hex[0] = '\0';
    while (fgets(line, sizeof(line), fp)) {
        size_t len = strlen(line);
        while (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r')) {
            line[len-1] = '\0';
            len--;
        }
        if (len > 10) {
            int all_hex = 1;
            for (size_t i = 0; i < len; i++) {
                char c = line[i];
                if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))) {
                    all_hex = 0;
                    break;
                }
            }
            if (all_hex) {
                assert(len < max_len);
                strcpy(out_hex, line);
            }
        }
    }
    pclose(fp);
    assert(strlen(out_hex) > 0);
}

static void deploy_contract_from_c(const char *bytecode_hex, char *out_addr, size_t max_len) {
    size_t payload_size = strlen(bytecode_hex) + 1024;
    char *payload = malloc(payload_size);
    assert(payload != NULL);

    snprintf(payload, payload_size,
             "{\"jsonrpc\":\"2.0\",\"method\":\"eth_sendTransaction\",\"params\":[{"
             "\"from\":\"0xf39Fd6e51aad88F6F4ce6aB8827279cffFb92266\","
             "\"data\":\"0x%s\","
             "\"gas\":\"0xF4240\""
             "}],\"id\":1}",
             bytecode_hex);

    char response[8192];
    send_rpc_request(payload, response, sizeof(response));
    free(payload);

    char *res_ptr = strstr(response, "\"result\":\"");
    assert(res_ptr != NULL);
    res_ptr += 10;
    char *end_quote = strchr(res_ptr, '"');
    assert(end_quote != NULL);
    char tx_hash[128];
    size_t len = end_quote - res_ptr;
    memcpy(tx_hash, res_ptr, len);
    tx_hash[len] = '\0';

    char receipt_payload[2048];
    snprintf(receipt_payload, sizeof(receipt_payload),
             "{\"jsonrpc\":\"2.0\",\"method\":\"eth_getTransactionReceipt\",\"params\":[\"%s\"],\"id\":1}",
             tx_hash);

    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 25000000;
    for (int i = 0; i < 40; i++) {
        send_rpc_request(receipt_payload, response, sizeof(response));
        char *addr_ptr = strstr(response, "\"contractAddress\":\"");
        if (addr_ptr) {
            addr_ptr += 19;
            char *end_addr = strchr(addr_ptr, '"');
            assert(end_addr != NULL);
            size_t addr_len = end_addr - addr_ptr;
            assert(addr_len < max_len);
            memcpy(out_addr, addr_ptr, addr_len);
            out_addr[addr_len] = '\0';
            return;
        }
        nanosleep(&ts, NULL);
    }
    fprintf(stderr, "[ERROR] Deployment transaction receipt not found\n");
    exit(1);
}

static void set_storage(const char *contract, const char *slot, uint64_t val) {
    char val_hex[128];
    snprintf(val_hex, sizeof(val_hex), "0x%064llx", (unsigned long long)val);
    char payload[1024];
    snprintf(payload, sizeof(payload),
             "{\"jsonrpc\":\"2.0\",\"method\":\"anvil_setStorageAt\",\"params\":[\"%s\",\"%s\",\"%s\"],\"id\":1}",
             contract, slot, val_hex);
    char response[8192];
    send_rpc_request(payload, response, sizeof(response));
}

static uint64_t get_storage(const char *contract, const char *slot) {
    char payload[1024];
    snprintf(payload, sizeof(payload),
             "{\"jsonrpc\":\"2.0\",\"method\":\"eth_getStorageAt\",\"params\":[\"%s\",\"%s\",\"latest\"],\"id\":1}",
             contract, slot);
    char response[8192];
    send_rpc_request(payload, response, sizeof(response));
    char *res_ptr = strstr(response, "\"result\":\"");
    if (res_ptr) {
        res_ptr += 10;
        char *end_quote = strchr(res_ptr, '"');
        if (end_quote) {
            *end_quote = '\0';
            return strtoull(res_ptr, NULL, 16);
        }
    }
    return 0;
}

static void wait_for_tx(const char *tx_hash) {
    char payload[2048];
    snprintf(payload, sizeof(payload),
             "{\"jsonrpc\":\"2.0\",\"method\":\"eth_getTransactionReceipt\",\"params\":[\"%s\"],\"id\":1}",
             tx_hash);
    char response[8192];
    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 25000000;
    for (int i = 0; i < 20; i++) {
        send_rpc_request(payload, response, sizeof(response));
        if (strstr(response, "\"result\":null") == NULL && strstr(response, "\"result\"") != NULL) {
            return;
        }
        nanosleep(&ts, NULL);
    }
}

static void trigger_execution(const char *contract) {
    char payload[1024];
    snprintf(payload, sizeof(payload),
             "{\"jsonrpc\":\"2.0\",\"method\":\"eth_sendTransaction\",\"params\":[{"
             "\"from\":\"0xf39Fd6e51aad88F6F4ce6aB8827279cffFb92266\","
             "\"to\":\"%s\","
             "\"data\":\"0x\","
             "\"gas\":\"0xF4240\""
             "}],\"id\":1}",
             contract);
    char response[8192];
    send_rpc_request(payload, response, sizeof(response));
    char *res_ptr = strstr(response, "\"result\":\"");
    if (res_ptr) {
        res_ptr += 10;
        char *end_quote = strchr(res_ptr, '"');
        if (end_quote) {
            char tx_hash[128];
            size_t len = end_quote - res_ptr;
            memcpy(tx_hash, res_ptr, len);
            tx_hash[len] = '\0';
            wait_for_tx(tx_hash);
        }
    }
}

void execute_yul_g15d_hardware(const DualRailSystemConfig *config, VMStateRecord *record) {
    set_storage(g_g15d_addr, "0x00", record->step);
    set_storage(g_g15d_addr, "0x01", record->pc);
    set_storage(g_g15d_addr, "0x10", g_yul_storage[REG_G15D_SECTOR_OFFSET]);
    set_storage(g_g15d_addr, "0x11", g_yul_storage[REG_G15D_RELOC_BASE]);
    set_storage(g_g15d_addr, "0x12", g_yul_storage[REG_G15D_PREFETCH]);
    set_storage(g_g15d_addr, "0x13", g_yul_storage[REG_G15D_DOUBLE_PREC]);
    set_storage(g_g15d_addr, "0x50", g_yul_storage[REG_G15D_BASE]);
    set_storage(g_g15d_addr, "0x51", g_yul_storage[REG_G15D_DIRECTION]);
    set_storage(g_g15d_addr, "0x52", g_yul_storage[REG_G15D_BASE_OFFSET]);
    set_storage(g_g15d_addr, "0x53", g_yul_storage[REG_G15D_CLOCK_SOURCE]);

    trigger_execution(g_g15d_addr);

    g_yul_storage[REG_G15D_STEP] = get_storage(g_g15d_addr, "0x00");
    g_yul_storage[REG_G15D_PC] = get_storage(g_g15d_addr, "0x01");
    g_yul_storage[REG_G15D_SECTOR_OFFSET] = get_storage(g_g15d_addr, "0x10");
    g_yul_storage[REG_G15D_BASE] = get_storage(g_g15d_addr, "0x50");
    g_yul_storage[REG_G15D_BASE_OFFSET] = get_storage(g_g15d_addr, "0x52");
    g_yul_storage[REG_G15D_CLOCK_SOURCE] = get_storage(g_g15d_addr, "0x53");

    record->pc = (uint32_t)g_yul_storage[REG_G15D_PC];

    if (record->step >= config->timing_cycle_interrupt_mask) {
        g_yul_storage[REG_G15D_HALTED] = 1;
        record->halted = 1;
    }
}

void execute_yul_ncr304_hardware(const DualRailSystemConfig *config, VMStateRecord *record) {
    (void)record;
    set_storage(g_ncr_addr, "0x20", g_yul_storage[REG_NCR304_TAPE_SEQ]);
    set_storage(g_ncr_addr, "0x21", g_yul_storage[REG_NCR304_PARITY_CHK]);
    set_storage(g_ncr_addr, "0x22", g_yul_storage[REG_NCR304_RETRY_COUNT]);
    set_storage(g_ncr_addr, "0x23", g_yul_storage[REG_NCR304_WRITE_LOCK]);
    set_storage(g_ncr_addr, "0x28", g_yul_storage[REG_NCR304_RETRY_LIMIT]);
    set_storage(g_ncr_addr, "0x30", config->segment_buffer_frame_size);

    trigger_execution(g_ncr_addr);

    g_yul_storage[REG_NCR304_TAPE_SEQ] = get_storage(g_ncr_addr, "0x20");
    g_yul_storage[REG_NCR304_PARITY_CHK] = get_storage(g_ncr_addr, "0x21");
    g_yul_storage[REG_NCR304_RETRY_COUNT] = get_storage(g_ncr_addr, "0x22");
    g_yul_storage[REG_NCR304_WARN_IND] = get_storage(g_ncr_addr, "0x24");
    g_yul_storage[REG_NCR304_MERGE_VAL] = get_storage(g_ncr_addr, "0x25");
    g_yul_storage[REG_NCR304_CHK_ERR] = get_storage(g_ncr_addr, "0x26");
    g_yul_storage[REG_NCR304_PARITY_CODE] = get_storage(g_ncr_addr, "0x27");
    g_yul_storage[REG_NCR304_IO_BUSY] = get_storage(g_ncr_addr, "0x29");
}

const char* tsfi_dat_search_hardware(tsfi_dat *dat, const char *key, const DualRailSystemConfig *config, VMStateRecord *trace, uint32_t *step_count) {
    if (!dat || !key || *key == '\0') return NULL;

    uint32_t step = 0;
    int state = 0;
    uint64_t hash_accum = 0;

    memset(g_yul_storage, 0, sizeof(g_yul_storage));
    g_yul_storage[REG_G15D_BASE] = 2;
    g_yul_storage[REG_NCR304_WRITE_LOCK] = config->write_lock_key;
    g_yul_storage[REG_NCR304_RETRY_LIMIT] = config->telemetry_block_size_validation;

    while (*key != '\0' && step < MAX_STATE_RECORDS) {
        hash_accum = (hash_accum * 33) + (unsigned char)*key;

        memset(&trace[step], 0, sizeof(VMStateRecord));
        trace[step].step = step;
        trace[step].pc = state;
        trace[step].sp = 4352;
        
        g_yul_storage[REG_NCR304_PARITY_CHK] = (uint64_t)(unsigned char)*key;
        if (config->base_index_modulus > 8) {
            g_yul_storage[REG_G15D_DOUBLE_PREC] = 1;
        }
        if (config->overflow_intercept_target_address != 0) {
            g_yul_storage[REG_G15D_RELOC_BASE] = 0x10;
        }

        execute_yul_g15d_hardware(config, &trace[step]);
        execute_yul_ncr304_hardware(config, &trace[step]);

        uint64_t dispatch_trigger = hash_accum & config->key_mask_filter;
        if (dispatch_trigger == 0) {
            trace[step].halted = 1;
            step++;
            *step_count = step;
            return NULL;
        }

        int b = dat->base[state];
        if (b < 0) {
            int tail_idx = -b - 1;
            g_yul_storage[REG_G15D_PREFETCH] = (uint32_t)tail_idx;
            step++;
            *step_count = step;

            if (strcmp(key, dat->tail + tail_idx) == 0) {
                return dat->values[state];
            }
            return NULL;
        }

        if (b == 0) {
            step++;
            *step_count = step;
            return NULL;
        }

        int next = b + (unsigned char)*key;
        if (next >= dat->capacity || dat->check[next] != state) {
            step++;
            *step_count = step;
            return NULL;
        }

        state = next;
        key++;
        step++;
    }

    *step_count = step;
    return dat->values[state];
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT SERIALIZATION: RAILS VM STATE TRACE (DYNAMIC DEPLOY)\n");
    printf("=============================================================\n");

    // Dynamic Compilation of Yul objects to bytecode
    printf("[C-Trace] Compiling G-15D Rail Yul object...\n");
    char *g15d_hex = malloc(131072);
    assert(g15d_hex != NULL);
    compile_yul_to_bytecode("src/firmware/g15d_rail.yul", g15d_hex, 131072);

    printf("[C-Trace] Compiling NCR 304 Rail Yul object...\n");
    char *ncr_hex = malloc(131072);
    assert(ncr_hex != NULL);
    compile_yul_to_bytecode("src/firmware/ncr304_rail.yul", ncr_hex, 131072);

    // Dynamic Deployment of contracts to Anvil
    printf("[C-Trace] Deploying G-15D contract dynamically from C...\n");
    deploy_contract_from_c(g15d_hex, g_g15d_addr, sizeof(g_g15d_addr));
    printf("  [DEPLOYED] G-15D Contract Address: %s\n", g_g15d_addr);

    printf("[C-Trace] Deploying NCR 304 contract dynamically from C...\n");
    deploy_contract_from_c(ncr_hex, g_ncr_addr, sizeof(g_ncr_addr));
    printf("  [DEPLOYED] NCR 304 Contract Address: %s\n", g_ncr_addr);

    FILE *af = fopen("assets/deployed_addresses.txt", "w");
    if (af) {
        fprintf(af, "G15D_ADDR=%s\n", g_g15d_addr);
        fprintf(af, "NCR_ADDR=%s\n", g_ncr_addr);
        fclose(af);
    }

    free(g15d_hex);
    free(ncr_hex);

    const char *out_path = "assets/logic_trace.dat.bin";

    VMStateHeader header = {
        .dat_magic_signature = 0x44415421,
        .dat_version = 1
    };

    tsfi_dat *router = tsfi_dat_init_ligature_router();
    assert(router != NULL);

    VMStateRecord search_trace[MAX_STATE_RECORDS];
    uint32_t step_count = 0;

    printf("Searching key 'ffi' utilizing live dynamically-deployed Yul hardware...\n");
    const char *res = tsfi_dat_search_hardware(router, "ffi", &g_global_hardware_config, search_trace, &step_count);
    assert(res != NULL && strcmp(res, "103") == 0);
    printf("✓ Resolved 'ffi' to value: %s in %u steps\n", res, step_count);

    assert(sizeof(VMStateHeader) % 8 == 0);
    assert(sizeof(VMStateRecord) % 8 == 0);
    printf("Header Record size: %zu bytes\n", sizeof(VMStateHeader));
    printf("Trace Record size: %zu bytes (per step)\n", sizeof(VMStateRecord));

    printf("Final Yul Hardware Registers state:\n");
    printf("  [G-15D] PC (0x01): %llu\n", (unsigned long long)g_yul_storage[REG_G15D_PC]);
    printf("  [G-15D] Sector (0x10): %llu\n", (unsigned long long)g_yul_storage[REG_G15D_SECTOR_OFFSET]);
    printf("  [G-15D] Base Reg (0x50): %llu\n", (unsigned long long)g_yul_storage[REG_G15D_BASE]);
    printf("  [NCR 304] Tape Seq (0x20): %llu\n", (unsigned long long)g_yul_storage[REG_NCR304_TAPE_SEQ]);
    printf("  [NCR 304] Parity Retries (0x22): %llu\n", (unsigned long long)g_yul_storage[REG_NCR304_RETRY_COUNT]);
    printf("  [NCR 304] Checksum Status (0x26): 0x%llX\n", (unsigned long long)g_yul_storage[REG_NCR304_CHK_ERR]);

    printf("Serializing header and %u trace records to %s...\n", step_count, out_path);

    FILE *f = fopen(out_path, "wb");
    if (!f) {
        out_path = "../assets/logic_trace.dat.bin";
        f = fopen(out_path, "wb");
        if (!f) {
            out_path = "logic_trace.dat.bin";
            f = fopen(out_path, "wb");
            if (!f) {
                printf("Error: Could not open output file for writing.\n");
                return 1;
            }
        }
    }

    fwrite(&header, sizeof(VMStateHeader), 1, f);
    fwrite(&step_count, sizeof(uint32_t), 1, f);

    for (uint32_t i = 0; i < step_count; i++) {
        fwrite(&search_trace[i], sizeof(VMStateRecord), 1, f);
    }
    fclose(f);

    printf("✓ Successfully persisted trace to %s\n", out_path);

    printf("Reading back trace from %s for verification...\n", out_path);
    f = fopen(out_path, "rb");
    if (!f) {
        printf("Error: Could not open file for verification reading.\n");
        return 1;
    }

    VMStateHeader read_header;
    size_t items_read = fread(&read_header, sizeof(VMStateHeader), 1, f);
    assert(items_read == 1);
    assert(read_header.dat_magic_signature == header.dat_magic_signature);
    assert(read_header.dat_version == header.dat_version);

    uint32_t read_count = 0;
    items_read = fread(&read_count, sizeof(uint32_t), 1, f);
    assert(items_read == 1);
    assert(read_count == step_count);

    for (uint32_t i = 0; i < read_count; i++) {
        VMStateRecord read_rec;
        items_read = fread(&read_rec, sizeof(VMStateRecord), 1, f);
        assert(items_read == 1);
        printf("  Step[%u]: PC=%u, SP=%u, ASP=%u, Halted=%u, Gas=%llu, Contract=%s\n",
               read_rec.step, read_rec.pc, read_rec.sp, read_rec.asp, read_rec.halted,
               (unsigned long long)read_rec.debug_gas_remaining, read_rec.debug_current_contract);

        assert(read_rec.step == search_trace[i].step);
        assert(read_rec.pc == search_trace[i].pc);
        assert(read_rec.sp == search_trace[i].sp);
        assert(read_rec.asp == search_trace[i].asp);
        assert(read_rec.halted == search_trace[i].halted);
        assert(read_rec.debug_step_mode == search_trace[i].debug_step_mode);
        assert(read_rec.debug_call_stack_depth == search_trace[i].debug_call_stack_depth);
        assert(read_rec.debug_gas_remaining == search_trace[i].debug_gas_remaining);
        assert(strcmp(read_rec.debug_current_contract, search_trace[i].debug_current_contract) == 0);
    }
    fclose(f);
    tsfi_dat_destroy(router);

    printf("✓ Read-back verification complete. C-level dynamic deploy pipeline validated successfully.\n");
    printf("=============================================================\n");
    return 0;
}
