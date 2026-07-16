#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <assert.h>
#include <time.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>

#define ANVIL_IP "127.0.0.1"
#define ANVIL_PORT 8545
#define ADDR_FILE "tmp/winchestermq_address.txt"

static int connect_to_anvil() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    assert(sockfd >= 0);

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(ANVIL_PORT);
    inet_pton(AF_INET, ANVIL_IP, &serv_addr.sin_addr);

    int conn = connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if (conn < 0) {
        close(sockfd);
        return -1;
    }
    return sockfd;
}

static void send_rpc_request(const char *payload, char *response_buf, size_t max_len) {
    int sockfd = connect_to_anvil();
    if (sockfd < 0) {
        fprintf(stderr, "[ERROR] Failed to connect to Anvil RPC at %s:%d\n", ANVIL_IP, ANVIL_PORT);
        exit(1);
    }

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

static void wait_for_tx(const char *tx_hash) {
    char payload[2048];
    snprintf(payload, sizeof(payload),
             "{\"jsonrpc\":\"2.0\",\"method\":\"eth_getTransactionReceipt\",\"params\":[\"%s\"],\"id\":1}",
             tx_hash);
    char response[8192];
    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 25000000;
    for (int i = 0; i < 40; i++) {
        send_rpc_request(payload, response, sizeof(response));
        if (strstr(response, "\"result\":null") == NULL && strstr(response, "\"result\"") != NULL) {
            return;
        }
        nanosleep(&ts, NULL);
    }
}

static void execute_tx(const char *payload) {
    char response[8192];
    send_rpc_request(payload, response, sizeof(response));
    char *res_ptr = strstr(response, "\"result\":\"");
    assert(res_ptr != NULL);
    res_ptr += 10;
    char *end_quote = strchr(res_ptr, '"');
    assert(end_quote != NULL);
    char tx_hash[128];
    size_t len = end_quote - res_ptr;
    memcpy(tx_hash, res_ptr, len);
    tx_hash[len] = '\0';
    wait_for_tx(tx_hash);
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
             "\"gas\":\"0x1C9C380\""
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

static void load_wmq_address(char *addr, size_t max_len) {
    FILE *fp = fopen(ADDR_FILE, "r");
    if (!fp) {
        addr[0] = '\0';
        return;
    }
    if (fgets(addr, max_len, fp)) {
        size_t len = strlen(addr);
        while (len > 0 && (addr[len-1] == '\n' || addr[len-1] == '\r')) {
            addr[len-1] = '\0';
            len--;
        }
    }
    fclose(fp);
}

static void save_wmq_address(const char *addr) {
    int res = mkdir("tmp", 0777);
    (void)res;
    FILE *fp = fopen(ADDR_FILE, "w");
    if (fp) {
        fprintf(fp, "%s\n", addr);
        fclose(fp);
    }
}

int main(int argc, char *argv[]) {
    char wmq_addr[128] = {0};
    load_wmq_address(wmq_addr, sizeof(wmq_addr));

    if (argc > 1 && strcmp(argv[1], "deploy") == 0) {
        printf("[C-Client] Compiling and deploying WinchesterMQ...\n");
        char *wmq_hex = malloc(131072);
        assert(wmq_hex != NULL);
        compile_yul_to_bytecode("../solidity/bin/WinchesterMQ.yul", wmq_hex, 131072);
        deploy_contract_from_c(wmq_hex, wmq_addr, sizeof(wmq_addr));
        save_wmq_address(wmq_addr);
        printf("Deployed WinchesterMQ Address: %s\n", wmq_addr);
        free(wmq_hex);
        return 0;
    }

    if (argc > 1 && strcmp(argv[1], "mount") == 0) {
        if (argc < 4) {
            fprintf(stderr, "Usage: %s mount <lun> <disk_address>\n", argv[0]);
            return 1;
        }
        if (strlen(wmq_addr) == 0) {
            fprintf(stderr, "Error: WinchesterMQ not deployed. Run '%s deploy' first.\n", argv[0]);
            return 1;
        }
        unsigned long lun = strtoul(argv[2], NULL, 10);
        const char *disk_addr = argv[3];
        if (disk_addr[0] == '0' && (disk_addr[1] == 'x' || disk_addr[1] == 'X')) disk_addr += 2;

        char payload[1024];
        char lun_hex[65];
        snprintf(lun_hex, sizeof(lun_hex), "%064lx", lun);

        snprintf(payload, sizeof(payload),
                 "{\"jsonrpc\":\"2.0\",\"method\":\"eth_sendTransaction\",\"params\":[{"
                 "\"from\":\"0xf39Fd6e51aad88F6F4ce6aB8827279cffFb92266\","
                 "\"to\":\"%s\","
                 "\"data\":\"0x4d6f756e%s"
                 "000000000000000000000000%s\","
                 "\"gas\":\"0xF4240\""
                 "}],\"id\":1}",
                 wmq_addr, lun_hex, disk_addr);

        execute_tx(payload);
        printf("Mounted disk %s on LUN %lu\n", argv[3], lun);
        return 0;
    }

    if (argc > 1 && strcmp(argv[1], "unmount") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Usage: %s unmount <lun>\n", argv[0]);
            return 1;
        }
        if (strlen(wmq_addr) == 0) {
            fprintf(stderr, "Error: WinchesterMQ not deployed. Run '%s deploy' first.\n", argv[0]);
            return 1;
        }
        unsigned long lun = strtoul(argv[2], NULL, 10);

        char payload[1024];
        char lun_hex[65];
        snprintf(lun_hex, sizeof(lun_hex), "%064lx", lun);

        snprintf(payload, sizeof(payload),
                 "{\"jsonrpc\":\"2.0\",\"method\":\"eth_sendTransaction\",\"params\":[{"
                 "\"from\":\"0xf39Fd6e51aad88F6F4ce6aB8827279cffFb92266\","
                 "\"to\":\"%s\","
                 "\"data\":\"0x4d6f756e%s"
                 "0000000000000000000000000000000000000000000000000000000000000000\","
                 "\"gas\":\"0xF4240\""
                 "}],\"id\":1}",
                 wmq_addr, lun_hex);

        execute_tx(payload);
        printf("Unmounted LUN %lu\n", lun);
        return 0;
    }

    if (argc > 1 && strcmp(argv[1], "get") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Usage: %s get <lun>\n", argv[0]);
            return 1;
        }
        if (strlen(wmq_addr) == 0) {
            fprintf(stderr, "Error: WinchesterMQ not deployed. Run '%s deploy' first.\n", argv[0]);
            return 1;
        }
        unsigned long lun = strtoul(argv[2], NULL, 10);
        char lun_hex[65];
        snprintf(lun_hex, sizeof(lun_hex), "%064lx", lun);

        char query_payload[1024];
        snprintf(query_payload, sizeof(query_payload),
                 "{\"jsonrpc\":\"2.0\",\"method\":\"eth_call\",\"params\":[{"
                 "\"to\":\"%s\","
                 "\"data\":\"0x476f756e%s\""
                 "},\"latest\"],\"id\":1}",
                 wmq_addr, lun_hex);

        char response[8192];
        send_rpc_request(query_payload, response, sizeof(response));
        char *res_ptr = strstr(response, "\"result\":\"");
        assert(res_ptr != NULL);
        res_ptr += 10;
        char *end_quote = strchr(res_ptr, '"');
        assert(end_quote != NULL);
        char query_addr[128];
        size_t len = end_quote - res_ptr;
        memcpy(query_addr, res_ptr, len);
        query_addr[len] = '\0';
        printf("LUN %lu mounted disk address: 0x%s\n", lun, query_addr + 26);
        return 0;
    }

    // Default automated verification loop with strict C asserts
    printf("=============================================================\n");
    printf("SOLID STATE HARDWARE DYNAMICS: MOUNT/UNMOUNT VERIFICATION\n");
    printf("=============================================================\n");

    printf("[C-Test] Compiling WinchesterMQ and DiskSystem Yul objects...\n");
    char *wmq_hex = malloc(131072);
    char *disk_hex = malloc(131072);
    assert(wmq_hex != NULL && disk_hex != NULL);

    compile_yul_to_bytecode("../solidity/bin/WinchesterMQ.yul", wmq_hex, 131072);
    compile_yul_to_bytecode("../solidity/bin/diskSystem.yul", disk_hex, 131072);

    printf("[C-Test] Deploying WinchesterMQ on Anvil...\n");
    deploy_contract_from_c(wmq_hex, wmq_addr, sizeof(wmq_addr));
    printf("  [DEPLOYED] WinchesterMQ Address: %s\n", wmq_addr);
    save_wmq_address(wmq_addr);

    char disk_addr[128];
    printf("[C-Test] Deploying DiskSystem on Anvil...\n");
    deploy_contract_from_c(disk_hex, disk_addr, sizeof(disk_addr));
    printf("  [DEPLOYED] DiskSystem Address: %s\n", disk_addr);

    // Assert initially unmounted (0 address returned)
    char query_payload[1024];
    snprintf(query_payload, sizeof(query_payload),
             "{\"jsonrpc\":\"2.0\",\"method\":\"eth_call\",\"params\":[{"
             "\"to\":\"%s\","
             "\"data\":\"0x476f756e"
             "0000000000000000000000000000000000000000000000000000000000000000\""
             "},\"latest\"],\"id\":1}",
             wmq_addr);

    char response[8192];
    send_rpc_request(query_payload, response, sizeof(response));
    char *res_ptr = strstr(response, "\"result\":\"");
    assert(res_ptr != NULL);
    res_ptr += 10;
    char *end_quote = strchr(res_ptr, '"');
    assert(end_quote != NULL);
    char initial_addr[128];
    size_t len = end_quote - res_ptr;
    memcpy(initial_addr, res_ptr, len);
    initial_addr[len] = '\0';
    assert(strcmp(initial_addr + 26, "0000000000000000000000000000000000000000") == 0);

    // Mount disk address on LUN 0
    char mount_payload[1024];
    snprintf(mount_payload, sizeof(mount_payload),
             "{\"jsonrpc\":\"2.0\",\"method\":\"eth_sendTransaction\",\"params\":[{"
             "\"from\":\"0xf39Fd6e51aad88F6F4ce6aB8827279cffFb92266\","
             "\"to\":\"%s\","
             "\"data\":\"0x4d6f756e"
             "0000000000000000000000000000000000000000000000000000000000000000" // LUN = 0
             "000000000000000000000000%s\","
             "\"gas\":\"0xF4240\""
             "}],\"id\":1}",
             wmq_addr, disk_addr + 2);

    execute_tx(mount_payload);
    printf("  [MOUNT] Disk successfully mounted on LUN 0.\n");

    // Assert mount address checks match perfectly
    send_rpc_request(query_payload, response, sizeof(response));
    res_ptr = strstr(response, "\"result\":\"");
    assert(res_ptr != NULL);
    res_ptr += 10;
    end_quote = strchr(res_ptr, '"');
    assert(end_quote != NULL);
    char query_addr[128];
    len = end_quote - res_ptr;
    memcpy(query_addr, res_ptr, len);
    query_addr[len] = '\0';
    printf("  [MOUNT_CHECK] WinchesterMQ returned mounted address: 0x%s\n", query_addr + 26);
    assert(strcmp(query_addr + 26, disk_addr + 2) == 0);

    // ---------------------------------------------------------
    // ANVIL SECTOR READ/WRITE VERIFICATION (RAMAC ENHANCED)
    // ---------------------------------------------------------
    printf("[C-Test] Verifying Block Write (U2) and Read (U1) sectors on DiskSystem...\n");
    char write_cmd[2048];
    char data_hex[513];
    memset(data_hex, '0', 512);
    data_hex[512] = '\0';
    const char *data_str = "RAMAC_SYSTEM_TEST_DATA_VERIFIED_OVER_ANVIL_RPC";
    for (size_t i = 0; i < strlen(data_str); i++) {
        sprintf(data_hex + (i * 2), "%02x", (unsigned char)data_str[i]);
    }

    snprintf(write_cmd, sizeof(write_cmd),
             "{\"jsonrpc\":\"2.0\",\"method\":\"eth_sendTransaction\",\"params\":[{"
             "\"from\":\"0xf39Fd6e51aad88F6F4ce6aB8827279cffFb92266\","
             "\"to\":\"%s\","
             "\"data\":\"0x9812a4df"
             "0000000000000000000000000000000000000000000000000000000000000020" // offset
             "000000000000000000000000000000000000000000000000000000000000010b" // length 267
             "5532205430322053303300" // "U2 T02 S03\0"
             "%s\"," // 256 bytes hex data
             "\"gas\":\"0xF4240\""
             "}],\"id\":1}",
             disk_addr, data_hex);

    execute_tx(write_cmd);
    printf("  [WRITE] Successfully wrote sector (Track 2, Sector 3).\n");

    // Mine a block to bypass the switch bounce debouncer same-block restriction
    char mine_payload[128] = "{\"jsonrpc\":\"2.0\",\"method\":\"evm_mine\",\"params\":[],\"id\":1}";
    char mine_response[1024];
    send_rpc_request(mine_payload, mine_response, sizeof(mine_response));

    // Query read payload using eth_call with aligned caller "from" field
    char read_cmd[1024];
    snprintf(read_cmd, sizeof(read_cmd),
             "{\"jsonrpc\":\"2.0\",\"method\":\"eth_call\",\"params\":[{"
             "\"from\":\"0xf39Fd6e51aad88F6F4ce6aB8827279cffFb92266\","
             "\"to\":\"%s\","
             "\"data\":\"0x9812a4df"
             "0000000000000000000000000000000000000000000000000000000000000020"
             "000000000000000000000000000000000000000000000000000000000000000b"
             "5531205430322053303300\"" // "U1 T02 S03\0"
             "},\"latest\"],\"id\":1}",
             disk_addr);

    send_rpc_request(read_cmd, response, sizeof(response));
    char test_hex[128];
    test_hex[0] = '\0';
    for (size_t i = 0; i < strlen(data_str); i++) {
        sprintf(test_hex + (i * 2), "%02x", (unsigned char)data_str[i]);
    }
    assert(strstr(response, test_hex) != NULL);
    printf("  [READ] Verified sector contains written data: '%s'\n", data_str);

    // ---------------------------------------------------------
    // YUL-BASED RAMAC SYSTEM INQUIRY SYSTEM VERIFICATION ON ANVIL
    // ---------------------------------------------------------
    printf("[C-Test] Compiling and deploying Yul-based RamacSystem...\n");
    char *ramac_hex = malloc(131072);
    assert(ramac_hex != NULL);
    compile_yul_to_bytecode("../solidity/bin/ramacSystem.yul", ramac_hex, 131072);
    char ramac_addr[128];
    deploy_contract_from_c(ramac_hex, ramac_addr, sizeof(ramac_addr));
    printf("  [DEPLOYED] RamacSystem Address: %s\n", ramac_addr);

    printf("[C-Test] Executing Yul-based RAMAC Inquiry command (WRT)...\n");
    // WRT key (32 bytes padded): "key_test_123"
    // VAL val (32 bytes padded): "val_test_999"
    // Selector = 0xe28e404f
    // Offset = 0x20
    // Length = 68 (WRT + spaces + key + val) -> 0x44
    // CMD payload = "WRT " + key (32 bytes) + val (32 bytes) = 68 bytes
    // In hex: WRT = 57525420
    char wrt_inq_payload[1024];
    snprintf(wrt_inq_payload, sizeof(wrt_inq_payload),
             "{\"jsonrpc\":\"2.0\",\"method\":\"eth_sendTransaction\",\"params\":[{"
             "\"from\":\"0xf39Fd6e51aad88F6F4ce6aB8827279cffFb92266\","
             "\"to\":\"%s\","
             "\"data\":\"0xe28e404f"
             "0000000000000000000000000000000000000000000000000000000000000020" // offset
             "0000000000000000000000000000000000000000000000000000000000000044" // length 68
             "57525420" // "WRT "
             "6b65795f746573745f3132330000000000000000000000000000000000000000" // key: "key_test_123" (32 bytes)
             "76616c5f746573745f3939390000000000000000000000000000000000000000\"" // val: "val_test_999" (32 bytes)
             "}],\"id\":1}",
             ramac_addr);

    execute_tx(wrt_inq_payload);
    printf("  [WRT] Wrote key_test_123 to RamacSystem on Anvil.\n");

    printf("[C-Test] Executing Yul-based RAMAC Inquiry command (QRY)...\n");
    // QRY key (32 bytes padded): "key_test_123"
    // CMD payload = "QRY " + key (32 bytes) = 36 bytes (0x24)
    char qry_inq_payload[1024];
    snprintf(qry_inq_payload, sizeof(qry_inq_payload),
             "{\"jsonrpc\":\"2.0\",\"method\":\"eth_call\",\"params\":[{"
             "\"to\":\"%s\","
             "\"data\":\"0xe28e404f"
             "0000000000000000000000000000000000000000000000000000000000000020"
             "0000000000000000000000000000000000000000000000000000000000000024" // length 36
             "51525920" // "QRY "
             "6b65795f746573745f3132330000000000000000000000000000000000000000\""
             "},\"latest\"],\"id\":1}",
             ramac_addr);

    send_rpc_request(qry_inq_payload, response, sizeof(response));
    // Verify response contains "val_test_999" hex representation: "76616c5f746573745f393939"
    assert(strstr(response, "76616c5f746573745f393939") != NULL);
    printf("  [QRY] Successfully retrieved val_test_999 from EVM storage!\n");

    printf("[C-Test] Executing Yul-based RAMAC ALU program steps on Anvil...\n");
    // Send 6 instructions:
    // Inst 0: ADD ACC1 100 -> 0101000100000000000000000000000000000000000000000000000000000064
    // Inst 1: ADD ACC2 50  -> 0102000100000000000000000000000000000000000000000000000000000032
    // Inst 2: CMP ACC1 ACC2 -> 0401020000000000000000000000000000000000000000000000000000000000
    // Inst 3: JEQ 5        -> 0500000100000000000000000000000000000000000000000000000000000005
    // Inst 4: SUB ACC1 ACC2 -> 0201020000000000000000000000000000000000000000000000000000000000
    // Inst 5: ADD ACC1 5   -> 0101000100000000000000000000000000000000000000000000000000000005
    char alu_payload[2048];
    snprintf(alu_payload, sizeof(alu_payload),
             "{\"jsonrpc\":\"2.0\",\"method\":\"eth_call\",\"params\":[{"
             "\"to\":\"%s\","
             "\"data\":\"0xb1b6081e"
             "0000000000000000000000000000000000000000000000000000000000000020" // offset
             "00000000000000000000000000000000000000000000000000000000000000c0" // length 192 bytes (6 insts * 32)
             "0101000100000000000000000000000000000000000000000000000000000064"
             "0102000100000000000000000000000000000000000000000000000000000032"
             "0401020000000000000000000000000000000000000000000000000000000000"
             "0500000100000000000000000000000000000000000000000000000000000005"
             "0201020000000000000000000000000000000000000000000000000000000000"
             "0101000100000000000000000000000000000000000000000000000000000005\""
             "},\"latest\"],\"id\":1}",
             ramac_addr);

    send_rpc_request(alu_payload, response, sizeof(response));
    // Final ACC1 value should be 55 (0x37)
    assert(strstr(response, "0000000000000000000000000000000000000000000000000000000000000037") != NULL);
    printf("  [ALU] Yul ALU successfully verified. Final ACC1 = 55.\n");

    printf("[C-Test] Verifying Rule 12 Zero Division Interception in Yul ALU...\n");
    // Send 2 instructions:
    // Inst 0: ADD ACC1 100 -> 0101000100000000000000000000000000000000000000000000000000000064
    // Inst 1: DIV ACC1 0   -> 0301000100000000000000000000000000000000000000000000000000000000
    char div_zero_payload[1024];
    snprintf(div_zero_payload, sizeof(div_zero_payload),
             "{\"jsonrpc\":\"2.0\",\"method\":\"eth_call\",\"params\":[{"
             "\"to\":\"%s\","
             "\"data\":\"0xb1b6081e"
             "0000000000000000000000000000000000000000000000000000000000000020" // offset
             "0000000000000000000000000000000000000000000000000000000000000040" // length 64 bytes
             "0101000100000000000000000000000000000000000000000000000000000064"
             "0301000100000000000000000000000000000000000000000000000000000000\""
             "},\"latest\"],\"id\":1}",
             ramac_addr);
    send_rpc_request(div_zero_payload, response, sizeof(response));
    // Verify that the call reverted
    assert(strstr(response, "execution reverted") != NULL);
    printf("  [ALU_TRAP] Rule 12 Division by Zero successfully intercepted and reverted in Yul!\n");

    printf("  [ALU_TRAP] Rule 12 Division by Zero successfully intercepted and reverted in Yul!\n");

    printf("[C-Test] Loading Double-Array Trie (DAT) nodes into RamacSystem storage...\n");
    // Load Node 0: base = 10, check = 0
    char load_dat0[1024];
    snprintf(load_dat0, sizeof(load_dat0),
             "{\"jsonrpc\":\"2.0\",\"method\":\"eth_sendTransaction\",\"params\":[{"
             "\"from\":\"0xf39Fd6e51aad88F6F4ce6aB8827279cffFb92266\","
             "\"to\":\"%s\","
             "\"data\":\"0xa3bf305a"
             "0000000000000000000000000000000000000000000000000000000000000000" // index = 0
             "000000000000000000000000000000000000000000000000000000000000000a" // baseVal = 10
             "0000000000000000000000000000000000000000000000000000000000000000\"" // checkVal = 0
             "}],\"id\":1}",
             ramac_addr);
    execute_tx(load_dat0);

    // Load Node 107 (10 + 'a' [97]): base = 20, check = 0
    char load_dat107[1024];
    snprintf(load_dat107, sizeof(load_dat107),
             "{\"jsonrpc\":\"2.0\",\"method\":\"eth_sendTransaction\",\"params\":[{"
             "\"from\":\"0xf39Fd6e51aad88F6F4ce6aB8827279cffFb92266\","
             "\"to\":\"%s\","
             "\"data\":\"0xa3bf305a"
             "000000000000000000000000000000000000000000000000000000000000006b" // index = 107 (0x6b)
             "0000000000000000000000000000000000000000000000000000000000000014" // baseVal = 20
             "0000000000000000000000000000000000000000000000000000000000000000\"" // checkVal = 0
             "}],\"id\":1}",
             ramac_addr);
    execute_tx(load_dat107);

    // Load Node 118 (20 + 'b' [98]): base = 30, check = 107
    char load_dat118[1024];
    snprintf(load_dat118, sizeof(load_dat118),
             "{\"jsonrpc\":\"2.0\",\"method\":\"eth_sendTransaction\",\"params\":[{"
             "\"from\":\"0xf39Fd6e51aad88F6F4ce6aB8827279cffFb92266\","
             "\"to\":\"%s\","
             "\"data\":\"0xa3bf305a"
             "0000000000000000000000000000000000000000000000000000000000000076" // index = 118 (0x76)
             "000000000000000000000000000000000000000000000000000000000000001e" // baseVal = 30
             "000000000000000000000000000000000000000000000000000000000000006b\"" // checkVal = 107 (0x6b)
             "}],\"id\":1}",
             ramac_addr);
    execute_tx(load_dat118);

    printf("[C-Test] Executing Yul-based DAT lookup for key 'ab'...\n");
    // Selector = 0xbc8e3d0f
    // Offset = 0x20
    // Length = 2
    // Data = "ab" -> "6162"
    char dat_lookup[1024];
    snprintf(dat_lookup, sizeof(dat_lookup),
             "{\"jsonrpc\":\"2.0\",\"method\":\"eth_call\",\"params\":[{"
             "\"to\":\"%s\","
             "\"data\":\"0xbc8e3d0f"
             "0000000000000000000000000000000000000000000000000000000000000020"
             "0000000000000000000000000000000000000000000000000000000000000002"
             "6162000000000000000000000000000000000000000000000000000000000000\""
             "},\"latest\"],\"id\":1}",
             ramac_addr);
    send_rpc_request(dat_lookup, response, sizeof(response));
    printf("DEBUG: DAT response = %s\n", response);
    // Final state should be 118 (0x76)
    assert(strstr(response, "0000000000000000000000000000000000000000000000000000000000000076") != NULL);
    printf("  [DAT] Yul Double-Array Trie lookup verified successfully. Final State = 118.\n");

    printf("[C-Test] Loading 2-3 Tree nodes into RamacSystem storage...\n");
    // Load Parent Node 1: isLeaf = 0, keyCount = 1, key0 = 100, key1 = 0, child0 = 2, child1 = 3, child2 = 0
    char load_tree1[1024];
    snprintf(load_tree1, sizeof(load_tree1),
             "{\"jsonrpc\":\"2.0\",\"method\":\"eth_sendTransaction\",\"params\":[{"
             "\"from\":\"0xf39Fd6e51aad88F6F4ce6aB8827279cffFb92266\","
             "\"to\":\"%s\","
             "\"data\":\"0xd32a9305"
             "0000000000000000000000000000000000000000000000000000000000000001" // nodeId = 1
             "0000000000000000000000000000000000000000000000000000000000000000" // isLeaf = 0
             "0000000000000000000000000000000000000000000000000000000000000001" // keyCount = 1
             "0000000000000000000000000000000000000000000000000000000000000064" // key0 = 100 (0x64)
             "0000000000000000000000000000000000000000000000000000000000000000" // key1 = 0
             "0000000000000000000000000000000000000000000000000000000000000002" // child0 = 2
             "0000000000000000000000000000000000000000000000000000000000000003" // child1 = 3
             "0000000000000000000000000000000000000000000000000000000000000000\"" // child2 = 0
             "}],\"id\":1}",
             ramac_addr);
    execute_tx(load_tree1);

    // Load Leaf Node 2: isLeaf = 1, keyCount = 1, key0 = 50, key1 = 0, child0 = 0, child1 = 0, child2 = 0
    char load_tree2[1024];
    snprintf(load_tree2, sizeof(load_tree2),
             "{\"jsonrpc\":\"2.0\",\"method\":\"eth_sendTransaction\",\"params\":[{"
             "\"from\":\"0xf39Fd6e51aad88F6F4ce6aB8827279cffFb92266\","
             "\"to\":\"%s\","
             "\"data\":\"0xd32a9305"
             "0000000000000000000000000000000000000000000000000000000000000002" // nodeId = 2
             "0000000000000000000000000000000000000000000000000000000000000001" // isLeaf = 1
             "0000000000000000000000000000000000000000000000000000000000000001" // keyCount = 1
             "0000000000000000000000000000000000000000000000000000000000000032" // key0 = 50 (0x32)
             "0000000000000000000000000000000000000000000000000000000000000000"
             "0000000000000000000000000000000000000000000000000000000000000000"
             "0000000000000000000000000000000000000000000000000000000000000000"
             "0000000000000000000000000000000000000000000000000000000000000000\""
             "}],\"id\":1}",
             ramac_addr);
    execute_tx(load_tree2);

    // Load Leaf Node 3: isLeaf = 1, keyCount = 1, key0 = 150, key1 = 0, child0 = 0, child1 = 0, child2 = 0
    char load_tree3[1024];
    snprintf(load_tree3, sizeof(load_tree3),
             "{\"jsonrpc\":\"2.0\",\"method\":\"eth_sendTransaction\",\"params\":[{"
             "\"from\":\"0xf39Fd6e51aad88F6F4ce6aB8827279cffFb92266\","
             "\"to\":\"%s\","
             "\"data\":\"0xd32a9305"
             "0000000000000000000000000000000000000000000000000000000000000003" // nodeId = 3
             "0000000000000000000000000000000000000000000000000000000000000001" // isLeaf = 1
             "0000000000000000000000000000000000000000000000000000000000000001" // keyCount = 1
             "0000000000000000000000000000000000000000000000000000000000000096" // key0 = 150 (0x96)
             "0000000000000000000000000000000000000000000000000000000000000000"
             "0000000000000000000000000000000000000000000000000000000000000000"
             "0000000000000000000000000000000000000000000000000000000000000000"
             "0000000000000000000000000000000000000000000000000000000000000000\""
             "}],\"id\":1}",
             ramac_addr);
    execute_tx(load_tree3);

    printf("[C-Test] Executing Yul-based 2-3 Tree search for key 150...\n");
    char tree_search[1024];
    snprintf(tree_search, sizeof(tree_search),
             "{\"jsonrpc\":\"2.0\",\"method\":\"eth_call\",\"params\":[{"
             "\"to\":\"%s\","
             "\"data\":\"0x5f23a9d3"
             "0000000000000000000000000000000000000000000000000000000000000001" // start nodeId = 1
             "0000000000000000000000000000000000000000000000000000000000000096\"" // targetKey = 150
             "},\"latest\"],\"id\":1}",
             ramac_addr);
    send_rpc_request(tree_search, response, sizeof(response));
    // Should find the key in Node 3
    assert(strstr(response, "0000000000000000000000000000000000000000000000000000000000000003") != NULL);
    printf("  [Tree] 2-3 Tree search resolved successfully. Target node ID = 3.\n");

    printf("[C-Test] Loading System/370 Segment Table Entry (SX = 0 -> Page Table at 256)...\n");
    char load_seg[1024];
    snprintf(load_seg, sizeof(load_seg),
             "{\"jsonrpc\":\"2.0\",\"method\":\"eth_sendTransaction\",\"params\":[{"
             "\"from\":\"0xf39Fd6e51aad88F6F4ce6aB8827279cffFb92266\","
             "\"to\":\"%s\","
             "\"data\":\"0xf3a9012c"
             "0000000000000000000000000000000000000000000000000000000000000000" // isPageTable = 0 (segment table)
             "0000000000000000000000000000000000000000000000000000000000000000" // origin = 0
             "0000000000000000000000000000000000000000000000000000000000000000" // index = 0
             "0000000000000000000000000000000000000000000000000000000000000100" // val = 256 (page table origin)
             "0000000000000000000000000000000000000000000000000000000000000000\"" // invalid = 0
             "}],\"id\":1}",
             ramac_addr);
    execute_tx(load_seg);

    printf("[C-Test] Loading System/370 Page Table Entry (PX = 5 -> Real Address 0x8F000)...\n");
    char load_page[1024];
    snprintf(load_page, sizeof(load_page),
             "{\"jsonrpc\":\"2.0\",\"method\":\"eth_sendTransaction\",\"params\":[{"
             "\"from\":\"0xf39Fd6e51aad88F6F4ce6aB8827279cffFb92266\","
             "\"to\":\"%s\","
             "\"data\":\"0xf3a9012c"
             "0000000000000000000000000000000000000000000000000000000000000001" // isPageTable = 1 (page table)
             "0000000000000000000000000000000000000000000000000000000000000100" // origin = 256
             "0000000000000000000000000000000000000000000000000000000000000005" // index = 5
             "000000000000000000000000000000000000000000000000000000000008f000" // val = 0x8F000
             "0000000000000000000000000000000000000000000000000000000000000000\"" // invalid = 0
             "}],\"id\":1}",
             ramac_addr);
    execute_tx(load_page);

    printf("[C-Test] Executing Yul-based System/370 DAT translate for Virtual Address 0x5123...\n");
    char dat_trans[1024];
    snprintf(dat_trans, sizeof(dat_trans),
             "{\"jsonrpc\":\"2.0\",\"method\":\"eth_call\",\"params\":[{"
             "\"to\":\"%s\","
             "\"data\":\"0x12a3f9e0"
             "0000000000000000000000000000000000000000000000000000000000005123" // virtualAddr = 0x5123
             "0000000000000000000000000000000000000000000000000000000000000000\"" // segmentTableOrigin = 0
             "},\"latest\"],\"id\":1}",
             ramac_addr);
    send_rpc_request(dat_trans, response, sizeof(response));
    // Physical address should be 0x8F123
    assert(strstr(response, "000000000000000000000000000000000000000000000000000000000008f123") != NULL);
    printf("  [DAT] Yul System/370 address translation verified successfully. Physical Address = 0x8F123.\n");

    free(ramac_hex);

    // Unmount disk from LUN 0
    char unmount_payload[1024];
    snprintf(unmount_payload, sizeof(unmount_payload),
             "{\"jsonrpc\":\"2.0\",\"method\":\"eth_sendTransaction\",\"params\":[{"
             "\"from\":\"0xf39Fd6e51aad88F6F4ce6aB8827279cffFb92266\","
             "\"to\":\"%s\","
             "\"data\":\"0x4d6f756e"
             "0000000000000000000000000000000000000000000000000000000000000000" // LUN = 0
             "0000000000000000000000000000000000000000000000000000000000000000\","
             "\"gas\":\"0xF4240\""
             "}],\"id\":1}",
             wmq_addr);
    execute_tx(unmount_payload);
    printf("  [UNMOUNT] Disk successfully unmounted from LUN 0.\n");

    // Assert address reset to 0 after unmount
    send_rpc_request(query_payload, response, sizeof(response));
    res_ptr = strstr(response, "\"result\":\"");
    assert(res_ptr != NULL);
    res_ptr += 10;
    end_quote = strchr(res_ptr, '"');
    assert(end_quote != NULL);
    char post_unmount_addr[128];
    len = end_quote - res_ptr;
    memcpy(post_unmount_addr, res_ptr, len);
    post_unmount_addr[len] = '\0';
    assert(strcmp(post_unmount_addr + 26, "0000000000000000000000000000000000000000") == 0);

    free(wmq_hex);
    free(disk_hex);

    printf("[PASS] Solid State Hardware Dynamics mount/unmount loops verified successfully!\n");
    printf("=============================================================\n");
    return 0;
}
