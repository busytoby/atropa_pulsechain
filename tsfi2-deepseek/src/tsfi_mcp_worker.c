#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/select.h>
#include <termios.h>

// Codebase RPC interface for metadata resolution
extern bool tsfi_pulse_rpc_exec_raw_body(const char *json_payload, char *out_buffer, size_t out_max_len);

int query_mcp(const char *payload, char *response, size_t resp_max) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) return 0;
    
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(10042);
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        close(sockfd);
        return 0;
    }
    
    if (write(sockfd, payload, strlen(payload)) < 0) {
        close(sockfd);
        return 0;
    }
    
    ssize_t n = read(sockfd, response, resp_max - 1);
    if (n > 0) {
        response[n] = '\0';
    }
    close(sockfd);
    return (n > 0);
}

void parse_abi_string(const char *json_body, char *out, size_t out_max) {
    if (!json_body) return;
    const char *hex = strstr(json_body, "\"result\":\"");
    if (!hex) return;
    hex += 10;
    if (strncmp(hex, "0x", 2) == 0) hex += 2;
    size_t len = 0;
    while (hex[len] && hex[len] != '"') {
        len++;
    }
    if (len < 128) return;
    char len_hex[65] = {0};
    memcpy(len_hex, hex + 64, 64);
    uint32_t str_len = (uint32_t)strtoul(len_hex, NULL, 16);
    if (str_len == 0 || str_len > 256) return;
    if (len < 128 + str_len * 2) return;
    size_t out_len = (str_len < out_max - 1) ? str_len : out_max - 1;
    for (size_t i = 0; i < out_len; i++) {
        char byte_hex[3] = { hex[128 + i * 2], hex[128 + i * 2 + 1], '\0' };
        out[i] = (char)strtol(byte_hex, NULL, 16);
    }
    out[out_len] = '\0';
}

void resolve_and_submit_metadata(const char *addr) {
    if (strcmp(addr, "0x0000000000000000000000000000000000000000") == 0) return;
    
    char name_payload[512];
    snprintf(name_payload, sizeof(name_payload),
             "{\"jsonrpc\":\"2.0\",\"method\":\"eth_call\",\"params\":[{\"to\":\"%s\",\"data\":\"0x06fdde03\"},\"latest\"],\"id\":1}",
             addr);
    char name_raw[4096] = {0};
    char name[128] = {0};
    if (tsfi_pulse_rpc_exec_raw_body(name_payload, name_raw, sizeof(name_raw))) {
        parse_abi_string(name_raw, name, sizeof(name));
    }
    
    if (name[0] == '\0') {
        return;
    }
    
    char sym_payload[512];
    snprintf(sym_payload, sizeof(sym_payload),
             "{\"jsonrpc\":\"2.0\",\"method\":\"eth_call\",\"params\":[{\"to\":\"%s\",\"data\":\"0x95d89b41\"},\"latest\"],\"id\":1}",
             addr);
    char sym_raw[4096] = {0};
    char symbol[32] = {0};
    if (tsfi_pulse_rpc_exec_raw_body(sym_payload, sym_raw, sizeof(sym_raw))) {
        parse_abi_string(sym_raw, symbol, sizeof(symbol));
    }
    if (name[0] != '\0' && symbol[0] != '\0') {
        printf("[WORKER] Resolved details for %s: Name=%s, Symbol=%s\n", addr, name, symbol);
        char submit_payload[1024];
        snprintf(submit_payload, sizeof(submit_payload),
                 "{\"jsonrpc\":\"2.0\",\"method\":\"wave64.add_discovered_token\",\"params\":{\"address\":\"%s\",\"symbol\":\"%s\",\"name\":\"%s\"},\"id\":2}\n",
                 addr, symbol, name);
        printf("[WORKER] Submitting newly discovered contract %s (%s) to MCP...\n", addr, symbol);
        char submit_resp[1024] = {0};
        if (query_mcp(submit_payload, submit_resp, sizeof(submit_resp))) {
            printf("[WORKER] MCP update response: %s\n", submit_resp);
        } else {
            printf("[WORKER] Warning: Failed to submit update to MCP for %s\n", addr);
        }
    }
}

void parse_json_value(const char *json, const char *key, char *out, size_t out_max) {
    out[0] = '\0';
    const char *p = strstr(json, key);
    if (!p) return;
    p = strchr(p, ':');
    if (!p) return;
    p++;
    while (*p == ' ' || *p == '\t') p++;
    if (*p == '"') {
        p++;
        const char *end = strchr(p, '"');
        if (end) {
            size_t len = end - p;
            if (len >= out_max) len = out_max - 1;
            memcpy(out, p, len);
            out[len] = '\0';
        }
    }
}

typedef struct {
    char address[43];
    char symbol[32];
    char name[128];
} CachedToken;

static CachedToken g_worker_token_cache[5000];
static int g_worker_token_cache_count = 0;

void load_mcp_contracts_cache() {
    g_worker_token_cache_count = 0;
    static char batch_resp[1048576];
    if (!query_mcp("{\"jsonrpc\":\"2.0\",\"method\":\"pulsechain.get_known_contracts\",\"params\":{},\"id\":1}\n", batch_resp, sizeof(batch_resp))) {
        return;
    }
    char *ptr = batch_resp;
    while ((ptr = strstr(ptr, "{\"address\":\"")) != NULL) {
        ptr += 12;
        if (g_worker_token_cache_count >= 5000) break;
        
        char addr[43] = {0};
        memcpy(addr, ptr, 42);
        
        char symbol[32] = {0};
        char *s_ptr = strstr(ptr, "\"symbol\":\"");
        if (s_ptr) {
            s_ptr += 10;
            char *end = strchr(s_ptr, '"');
            if (end && (end - s_ptr) < 30) {
                memcpy(symbol, s_ptr, end - s_ptr);
            }
        }
        
        char name[128] = {0};
        char *n_ptr = strstr(ptr, "\"name\":\"");
        if (n_ptr) {
            n_ptr += 8;
            char *end = strchr(n_ptr, '"');
            if (end && (end - n_ptr) < 120) {
                memcpy(name, n_ptr, end - n_ptr);
            }
        }
        
        if (addr[0] != '\0') {
            strcpy(g_worker_token_cache[g_worker_token_cache_count].address, addr);
            strcpy(g_worker_token_cache[g_worker_token_cache_count].symbol, symbol);
            strcpy(g_worker_token_cache[g_worker_token_cache_count].name, name);
            g_worker_token_cache_count++;
        }
        ptr += 42;
    }
    printf("[WORKER] Pre-loaded %d known contracts from MCP in batch.\n", g_worker_token_cache_count);
}

void resolve_address_metadata(const char *addr, char *out_symbol, char *out_name) {
    strcpy(out_symbol, "UNKNOWN");
    strcpy(out_name, "UNKNOWN");
    
    for (int i = 0; i < g_worker_token_cache_count; i++) {
        if (strcasecmp(g_worker_token_cache[i].address, addr) == 0) {
            strcpy(out_symbol, g_worker_token_cache[i].symbol);
            strcpy(out_name, g_worker_token_cache[i].name);
            return;
        }
    }
    
    char query_payload[512];
    snprintf(query_payload, sizeof(query_payload),
             "{\"jsonrpc\":\"2.0\",\"method\":\"wave64.query_knowledge_graph\",\"params\":{\"address\":\"%s\"},\"id\":1}\n",
             addr);
    char mcp_resp[4096] = {0};
    bool resolved = false;
    
    if (query_mcp(query_payload, mcp_resp, sizeof(mcp_resp))) {
        char name_buf[128] = {0};
        char sym_buf[32] = {0};
        parse_json_value(mcp_resp, "\"name\"", name_buf, sizeof(name_buf));
        parse_json_value(mcp_resp, "\"symbol\"", sym_buf, sizeof(sym_buf));
        
        if (name_buf[0] != '\0' && strcmp(name_buf, "UNKNOWN") != 0 && !strstr(mcp_resp, "not found")) {
            size_t n_len = strlen(name_buf);
            if (!(n_len >= 8 && strcmp(name_buf + n_len - 8, "Contract") == 0)) {
                strcpy(out_name, name_buf);
                strcpy(out_symbol, sym_buf);
                resolved = true;
            }
        }
    }
    
    if (!resolved) {
        resolve_and_submit_metadata(addr);
        if (query_mcp(query_payload, mcp_resp, sizeof(mcp_resp))) {
            char name_buf[128] = {0};
            char sym_buf[32] = {0};
            parse_json_value(mcp_resp, "\"name\"", name_buf, sizeof(name_buf));
            parse_json_value(mcp_resp, "\"symbol\"", sym_buf, sizeof(sym_buf));
            if (name_buf[0] != '\0') strcpy(out_name, name_buf);
            if (sym_buf[0] != '\0') strcpy(out_symbol, sym_buf);
        }
    }
    
    if (g_worker_token_cache_count < 5000) {
        strcpy(g_worker_token_cache[g_worker_token_cache_count].address, addr);
        strcpy(g_worker_token_cache[g_worker_token_cache_count].symbol, out_symbol);
        strcpy(g_worker_token_cache[g_worker_token_cache_count].name, out_name);
        g_worker_token_cache_count++;
    }
}

void run_sweep() {
    FILE *f = fopen("solidity/addresses.sol", "r");
    if (!f) {
        f = fopen("../solidity/addresses.sol", "r");
    }
    if (!f) {
        printf("[WORKER] Error: Failed to open addresses.sol\n");
        return;
    }
    
    printf("[WORKER] Loading known contracts from MCP in batch...\n");
    static char known_contracts[2000][43];
    int known_contracts_count = 0;
    static char batch_resp[524288];
    if (query_mcp("{\"jsonrpc\":\"2.0\",\"method\":\"pulsechain.get_known_contracts\",\"params\":{},\"id\":1}\n", batch_resp, sizeof(batch_resp))) {
        char *ptr = batch_resp;
        while ((ptr = strstr(ptr, "\"address\":\"")) != NULL) {
            ptr += 11;
            if (strncmp(ptr, "0x", 2) == 0 && known_contracts_count < 2000) {
                memcpy(known_contracts[known_contracts_count], ptr, 42);
                known_contracts[known_contracts_count][42] = '\0';
                known_contracts_count++;
            }
        }
    }
    
    printf("[WORKER] Sweeping addresses.sol for unsubmitted contracts (in-memory lookup)...\n");
    char line[512];
    while (fgets(line, sizeof(line), f)) {
        char *trim_line = line;
        while (*trim_line == ' ' || *trim_line == '\t') trim_line++;
        if (strncmp(trim_line, "//", 2) == 0 || strncmp(trim_line, "/*", 2) == 0) continue;
        
        char *p = strstr(line, "address(0x");
        if (p) {
            p += 8;
            char addr[64] = {0};
            char *end = strchr(p, ')');
            if (end && (end - p) < 50) {
                strncpy(addr, p, end - p);
                char clean_addr[43];
                snprintf(clean_addr, sizeof(clean_addr), "0x%.40s", addr + 2);
                
                bool already_known = false;
                for (int i = 0; i < known_contracts_count; i++) {
                    if (strcasecmp(known_contracts[i], clean_addr) == 0) {
                        already_known = true;
                        break;
                    }
                }
                if (already_known) {
                    continue;
                }
                
                resolve_and_submit_metadata(clean_addr);
            }
        }
    }
    fclose(f);
    printf("[WORKER] Sweep complete.\n");
}

typedef struct {
    char address[43];
    char symbol[32];
    uint32_t count;
} ContractStat;

int compare_stats(const void *a, const void *b) {
    return ((ContractStat*)b)->count - ((ContractStat*)a)->count;
}

static void set_conio_terminal_mode(struct termios *oldt) {
    struct termios newt;
    tcgetattr(STDIN_FILENO, oldt);
    newt = *oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
}

static void reset_conio_terminal_mode(const struct termios *oldt) {
    tcsetattr(STDIN_FILENO, TCSANOW, oldt);
}

static int kbhit(void) {
    struct timeval tv = {0, 0};
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    return select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv) > 0;
}

int main() {
    printf("[WORKER] Interactive TTY shell started.\n");
    
    printf("Enter commands:\n");
    printf("  0x[address]  - Query MCP server for a single contract address details\n");
    printf("  latest       - Query MCP server for the latest block transactions\n");
    printf("  contracts    - Query MCP server for all known contracts\n");
    printf("  sweep        - Scan addresses.sol, retrieve unknown metadata and submit to MCP\n");
    printf("  status       - Query daemon queue size and local cache status\n");
    printf("  exit / quit  - Exit worker\n");
    printf("> ");
    fflush(stdout);
    
    char line[256];
    while (fgets(line, sizeof(line), stdin)) {
        line[strcspn(line, "\n")] = '\0';
        if (strcmp(line, "exit") == 0 || strcmp(line, "quit") == 0) {
            break;
        } else if (strncmp(line, "0x", 2) == 0 && strlen(line) == 42) {
            printf("[WORKER] Querying details for single address: %s\n", line);
            char sym_buf[32] = {0};
            char name_buf[128] = {0};
            resolve_address_metadata(line, sym_buf, name_buf);
            printf("[WORKER] Final Resolved: Symbol=%s, Name=%s\n", sym_buf, name_buf);
        } else if (strcmp(line, "latest") == 0) {
            printf("[WORKER] Starting catchup sweep loop for 10 block window...\n");
            load_mcp_contracts_cache();
            
            static ContractStat stats[1000];
            int stats_count = 0;
            
            struct termios orig_termios;
            bool is_tty = isatty(STDIN_FILENO);
            if (is_tty) {
                set_conio_terminal_mode(&orig_termios);
            }
            
            int blocks_processed = 0;
            while (1) {
                if (kbhit()) {
                    char ch = getchar();
                    if (ch == 'q' || ch == 'Q') {
                        printf("\n[INTERRUPT] Graceful shutdown requested via keyboard.\n");
                        break;
                    }
                }
                static char resp[524288];
                if (!query_mcp("{\"jsonrpc\":\"2.0\",\"method\":\"pulsechain.get_latest_block\",\"params\":{},\"id\":1}\n", resp, sizeof(resp))) {
                    printf("[WORKER] Error: Failed to query latest block from MCP\n");
                    break;
                }
                
                uint64_t block_val = 0;
                char *p_num = strstr(resp, "\"number\":\"");
                if (p_num) {
                    p_num += 10;
                    char *end_num = strchr(p_num, '"');
                    if (end_num && (end_num - p_num) < 30) {
                        char block_num_str[32] = {0};
                        memcpy(block_num_str, p_num, end_num - p_num);
                        block_val = strtoull(block_num_str, NULL, 16);
                    }
                }
                
                if (block_val == 0) {
                    usleep(1000000);
                    continue;
                }
                
                printf("[WORKER] Processing block %lu...\n", block_val);
                
                char *p_tx = resp;
                while ((p_tx = strstr(p_tx, "{\"blockHash\":")) != NULL) {
                    char tx_hash[67] = {0};
                    char *h_ptr = strstr(p_tx, "\"hash\":\"");
                    if (h_ptr) {
                        h_ptr += 8;
                        memcpy(tx_hash, h_ptr, 66);
                    }
                    
                    char to_addr[43] = {0};
                    char *t_ptr = strstr(p_tx, "\"to\":\"");
                    if (t_ptr) {
                        t_ptr += 6;
                        if (strncmp(t_ptr, "0x", 2) == 0) {
                            memcpy(to_addr, t_ptr, 42);
                        }
                    }
                    
                    p_tx += 12;
                    
                    char to_symbol[32] = "UNKNOWN";
                    char to_name[128] = "UNKNOWN";
                    if (to_addr[0] != '\0') {
                        resolve_address_metadata(to_addr, to_symbol, to_name);
                    }
                    
                    printf("  TX: %s\n", tx_hash);
                    if (to_addr[0] != '\0') {
                        printf("    To: %s | Symbol: %s | Name: %s\n", to_addr, to_symbol, to_name);
                        
                        // Cumulative frequency stats collection
                        if (strcmp(to_symbol, "UNKNOWN") != 0 && strcmp(to_symbol, "NoContract") != 0) {
                            int f_idx = -1;
                            for (int i = 0; i < stats_count; i++) {
                                if (strcasecmp(stats[i].address, to_addr) == 0) {
                                    f_idx = i;
                                    break;
                                }
                            }
                            if (f_idx != -1) {
                                stats[f_idx].count++;
                            } else if (stats_count < 1000) {
                                strcpy(stats[stats_count].address, to_addr);
                                strcpy(stats[stats_count].symbol, to_symbol);
                                stats[stats_count].count = 1;
                                stats_count++;
                            }
                        }
                    }
                }
                
                uint32_t pending_count = 0;
                char *p_pend = strstr(resp, "\"pending_count\":");
                if (p_pend) {
                    p_pend += 16;
                    while (*p_pend == ' ' || *p_pend == '\t') p_pend++;
                    pending_count = (uint32_t)strtoul(p_pend, NULL, 10);
                }

                char prune_payload[256];
                snprintf(prune_payload, sizeof(prune_payload),
                         "{\"jsonrpc\":\"2.0\",\"method\":\"pulsechain.prune_block\",\"params\":{\"block\":%lu},\"id\":2}\n",
                         block_val);
                char prune_resp[1024];
                if (query_mcp(prune_payload, prune_resp, sizeof(prune_resp))) {
                    blocks_processed++;
                    printf("[WORKER] Successfully resolved and pruned block %lu. (Total pruned: %d, Remaining pending blocks: %u)\n", block_val, blocks_processed, pending_count);
                } else {
                    printf("[WORKER] Error: Failed to prune block %lu from MCP\n", block_val);
                    break;
                }
            }
            
            if (is_tty) {
                reset_conio_terminal_mode(&orig_termios);
            }
            
            // Print sorted frequency stats
            if (stats_count > 0) {
                qsort(stats, stats_count, sizeof(ContractStat), compare_stats);
                printf("\n[WORKER] Cumulative Contract Frequency Statistics:\n");
                for (int i = 0; i < stats_count; i++) {
                    printf("  %d. %s (%s): %u occurrences\n", i + 1, stats[i].address, stats[i].symbol, stats[i].count);
                }
                printf("\n");
            }
            
            printf("[WORKER] Catchup sweep complete. Cleared block cache window.\n");
        } else if (strcmp(line, "contracts") == 0) {
            printf("[WORKER] Querying all known contracts from MCP...\n");
            static char resp[524288];
            if (query_mcp("{\"jsonrpc\":\"2.0\",\"method\":\"pulsechain.get_known_contracts\",\"params\":{},\"id\":1}\n", resp, sizeof(resp))) {
                printf("[RESPONSE] %s\n", resp);
            } else {
                printf("[WORKER] Error: Failed to query known contracts from MCP\n");
            }
        } else if (strcmp(line, "status") == 0) {
            printf("[WORKER] Requesting daemon status...\n");
            static char resp[524288];
            if (query_mcp("{\"jsonrpc\":\"2.0\",\"method\":\"pulsechain.get_latest_block\",\"params\":{},\"id\":1}\n", resp, sizeof(resp))) {
                uint32_t pending_count = 0;
                char *p_pend = strstr(resp, "\"pending_count\":");
                if (p_pend) {
                    p_pend += 16;
                    while (*p_pend == ' ' || *p_pend == '\t') p_pend++;
                    pending_count = (uint32_t)strtoul(p_pend, NULL, 10);
                }
                uint64_t block_val = 0;
                char *p_num = strstr(resp, "\"number\":\"");
                if (p_num) {
                    p_num += 10;
                    char *end_num = strchr(p_num, '"');
                    if (end_num && (end_num - p_num) < 30) {
                        char block_num_str[32] = {0};
                        memcpy(block_num_str, p_num, end_num - p_num);
                        block_val = strtoull(block_num_str, NULL, 16);
                    }
                }
                printf("  [DAEMON] Active Pending Queue Size: %u blocks\n", pending_count);
                if (block_val > 0) {
                    printf("  [DAEMON] Newest Pending Block Num: %lu\n", block_val);
                } else {
                    printf("  [DAEMON] No pending blocks in queue.\n");
                }
            } else {
                printf("[WORKER] Error: Failed to contact daemon.\n");
            }
            printf("  [WORKER] Local Cache Size: %d preloaded contracts\n", g_worker_token_cache_count);
        } else if (strcmp(line, "sweep") == 0) {
            run_sweep();
        } else {
            printf("[WORKER] Unknown command: %s\n", line);
        }
        printf("> ");
        fflush(stdout);
    }
    return 0;
}
