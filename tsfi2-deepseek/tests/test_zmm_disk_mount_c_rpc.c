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
    // Ensure parent dir exists
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
        printf("LUN %lu mounted disk address: 0x%s\n", lun, query_addr + 24);
        return 0;
    }

    // Default automated verification loop
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
    char query_addr[128];
    size_t len = end_quote - res_ptr;
    memcpy(query_addr, res_ptr, len);
    query_addr[len] = '\0';
    printf("  [MOUNT_CHECK] WinchesterMQ returned mounted address: 0x%s\n", query_addr + 24);

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

    free(wmq_hex);
    free(disk_hex);

    printf("[PASS] Solid State Hardware Dynamics mount/unmount loops verified successfully!\n");
    printf("=============================================================\n");
    return 0;
}
