#include "tsfi_pulsechain_rpc.h"
#include "tsfi_tls.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#define RPC_HOST "pulsechain-rpc.publicnode.com"
#define RPC_PORT "443"

bool exec_raw_http_rpc(const char *json_payload, char *out_hex_buffer, size_t out_max_len) {
    struct addrinfo hints, *res;
    int sockfd = -1;
    bool success = false;
    char *response = NULL;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(RPC_HOST, RPC_PORT, &hints, &res) != 0) {
        fprintf(stderr, "[RPC DEBUG] getaddrinfo failed for %s:%s\n", RPC_HOST, RPC_PORT);
        return false;
    }

    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd < 0) {
        fprintf(stderr, "[RPC DEBUG] socket creation failed\n");
        freeaddrinfo(res);
        return false;
    }

    struct timeval tv;
    tv.tv_sec = 3;
    tv.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
    setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tv, sizeof(tv));

    if (connect(sockfd, res->ai_addr, res->ai_addrlen) < 0) {
        fprintf(stderr, "[RPC DEBUG] connect failed to %s:%s\n", RPC_HOST, RPC_PORT);
        close(sockfd);
        freeaddrinfo(res);
        return false;
    }
    freeaddrinfo(res);

    TsfiTlsContext tls;
    tsfi_tls_init(&tls, sockfd);
    if (!tsfi_tls_handshake(&tls, RPC_HOST)) {
        fprintf(stderr, "[RPC DEBUG] TLS handshake failed for %s\n", RPC_HOST);
        tsfi_tls_close(&tls);
        close(sockfd);
        return false;
    }

    char request[131072];
    snprintf(request, sizeof(request),
             "POST / HTTP/1.1\r\n"
             "Host: %s\r\n"
             "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) Antigravity/2.0\r\n"
             "Content-Type: application/json\r\n"
             "Content-Length: %zu\r\n"
             "Connection: close\r\n"
             "\r\n"
             "%s",
             RPC_HOST, strlen(json_payload), json_payload);

    fprintf(stderr, "[RPC DEBUG] Sending Request: %s\n", request);

    if (tsfi_tls_write(&tls, request, strlen(request)) < 0) {
        fprintf(stderr, "[RPC DEBUG] tsfi_tls_write failed\n");
        tsfi_tls_close(&tls);
        close(sockfd);
        return false;
    }

    response = malloc(262144);
    if (!response) {
        fprintf(stderr, "[RPC DEBUG] malloc failed for response buffer\n");
        tsfi_tls_close(&tls);
        close(sockfd);
        return false;
    }
    memset(response, 0, 262144);
    size_t total_read = 0;
    ssize_t n;
    int content_length = -1;
    size_t header_end = 0;

    while (1) {
        n = tsfi_tls_read(&tls, response + total_read, 262144 - 1 - total_read);
        if (n <= 0) break;
        total_read += n;
        response[total_read] = '\0';

        if (header_end == 0) {
            char *hdr_end_ptr = strstr(response, "\r\n\r\n");
            if (hdr_end_ptr) {
                header_end = (hdr_end_ptr - response) + 4;
                const char *cl_ptr = response;
                while ((cl_ptr = strchr(cl_ptr, '\n'))) {
                    cl_ptr++;
                    if (strncasecmp(cl_ptr, "content-length:", 15) == 0) {
                        content_length = atoi(cl_ptr + 15);
                        break;
                    }
                }
            }
        }

        if (content_length >= 0 && header_end > 0) {
            if (total_read >= header_end + content_length) {
                break;
            }
        }

        if (header_end > 0) {
            const char *te_ptr = response;
            bool is_chunked = false;
            while ((te_ptr = strchr(te_ptr, '\n'))) {
                te_ptr++;
                if (strncasecmp(te_ptr, "transfer-encoding: chunked", 26) == 0) {
                    is_chunked = true;
                    break;
                }
            }
            if (is_chunked) {
                if (strstr(response + header_end, "\r\n0\r\n\r\n") || strstr(response + header_end, "0\r\n\r\n")) {
                    break;
                }
            }
        }

        if (total_read >= 262144 - 1) {
            break;
        }
    }
    tsfi_tls_close(&tls);
    close(sockfd);
    
    response[total_read] = '\0';

    char *body = strstr(response, "\r\n\r\n");
    if (!body) goto cleanup;
    body += 4;

    char *result_ptr = strstr(body, "\"result\":\"");
    if (result_ptr) {
        result_ptr += 10;
        char *end_quote = strchr(result_ptr, '"');
        if (end_quote) {
            size_t hex_len = end_quote - result_ptr;
            if (hex_len < out_max_len) {
                memcpy(out_hex_buffer, result_ptr, hex_len);
                out_hex_buffer[hex_len] = '\0';
                success = true;
                goto cleanup;
            } else {
                fprintf(stderr, "[RPC DEBUG] result_ptr found but hex_len (%zu) >= out_max_len (%zu)\n", hex_len, out_max_len);
            }
        }
    }

    // fallback for JSON arrays (starting with '[')
    char *result_arr = strstr(body, "\"result\":");
    if (result_arr) {
        result_arr += 9;
        fprintf(stderr, "[RPC DEBUG] result_arr found, first char: '%c'\n", *result_arr);
        if (*result_arr == '[') {
            char *end_arr = strrchr(result_arr, ']');
            if (end_arr) {
                size_t arr_len = end_arr - result_arr + 1;
                fprintf(stderr, "[RPC DEBUG] Found end of array, length: %zu\n", arr_len);
                if (arr_len < out_max_len) {
                    memcpy(out_hex_buffer, result_arr, arr_len);
                    out_hex_buffer[arr_len] = '\0';
                    success = true;
                    goto cleanup;
                } else {
                    fprintf(stderr, "[RPC DEBUG] arr_len (%zu) >= out_max_len (%zu)\n", arr_len, out_max_len);
                }
            } else {
                fprintf(stderr, "[RPC DEBUG] Could not find end of array ']' in result_arr\n");
            }
        }
    }

    // fallback for non-quoted result object strings
    char *result_obj = strstr(body, "\"result\":");
    if (result_obj) {
        result_obj += 9;
        if (*result_obj == '"') {
            result_obj++;
            char *end_quote = strchr(result_obj, '"');
            if (end_quote) {
                size_t hex_len = end_quote - result_obj;
                if (hex_len < out_max_len) {
                    memcpy(out_hex_buffer, result_obj, hex_len);
                    out_hex_buffer[hex_len] = '\0';
                    success = true;
                    goto cleanup;
                }
            }
        }
    }

    fprintf(stderr, "[RPC DEBUG] All parsing pathways failed for body: %.200s...\n", body);

cleanup:
    if (response) free(response);
    return success;
}

static bool exec_raw_http_rpc_body(const char *json_payload, char *out_buffer, size_t out_max_len) {
    struct addrinfo hints, *res;
    int sockfd = -1;
    bool success = false;
    char *response = NULL;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(RPC_HOST, RPC_PORT, &hints, &res) != 0) {
        fprintf(stderr, "[RPC DEBUG] getaddrinfo failed for %s:%s\n", RPC_HOST, RPC_PORT);
        return false;
    }

    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd < 0) {
        fprintf(stderr, "[RPC DEBUG] socket creation failed\n");
        freeaddrinfo(res);
        return false;
    }

    if (connect(sockfd, res->ai_addr, res->ai_addrlen) < 0) {
        fprintf(stderr, "[RPC DEBUG] connect failed to %s:%s\n", RPC_HOST, RPC_PORT);
        close(sockfd);
        freeaddrinfo(res);
        return false;
    }
    freeaddrinfo(res);

    TsfiTlsContext tls;
    tsfi_tls_init(&tls, sockfd);
    if (!tsfi_tls_handshake(&tls, RPC_HOST)) {
        fprintf(stderr, "[RPC DEBUG] TLS handshake failed for %s\n", RPC_HOST);
        tsfi_tls_close(&tls);
        close(sockfd);
        return false;
    }

    char request[131072];
    snprintf(request, sizeof(request),
             "POST / HTTP/1.1\r\n"
             "Host: %s\r\n"
             "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) Antigravity/2.0\r\n"
             "Content-Type: application/json\r\n"
             "Content-Length: %zu\r\n"
             "Connection: close\r\n"
             "\r\n"
             "%s",
             RPC_HOST, strlen(json_payload), json_payload);

    if (tsfi_tls_write(&tls, request, strlen(request)) < 0) {
        fprintf(stderr, "[RPC DEBUG] tsfi_tls_write failed\n");
        tsfi_tls_close(&tls);
        close(sockfd);
        return false;
    }

    response = malloc(524288);
    if (!response) {
        fprintf(stderr, "[RPC DEBUG] malloc failed for response buffer\n");
        tsfi_tls_close(&tls);
        close(sockfd);
        return false;
    }
    memset(response, 0, 524288);
    size_t total_read = 0;
    ssize_t n;
    int content_length = -1;
    size_t header_end = 0;

    while (1) {
        n = tsfi_tls_read(&tls, response + total_read, 524288 - 1 - total_read);
        if (n <= 0) break;
        total_read += n;
        response[total_read] = '\0';

        if (header_end == 0) {
            char *hdr_end_ptr = strstr(response, "\r\n\r\n");
            if (hdr_end_ptr) {
                header_end = (hdr_end_ptr - response) + 4;
                const char *cl_ptr = response;
                while ((cl_ptr = strchr(cl_ptr, '\n'))) {
                    cl_ptr++;
                    if (strncasecmp(cl_ptr, "content-length:", 15) == 0) {
                        content_length = atoi(cl_ptr + 15);
                        break;
                    }
                }
            }
        }

        if (content_length >= 0 && header_end > 0) {
            if (total_read >= header_end + content_length) {
                break;
            }
        }

        if (header_end > 0) {
            const char *te_ptr = response;
            bool is_chunked = false;
            while ((te_ptr = strchr(te_ptr, '\n'))) {
                te_ptr++;
                if (strncasecmp(te_ptr, "transfer-encoding: chunked", 26) == 0) {
                    is_chunked = true;
                    break;
                }
            }
            if (is_chunked) {
                if (strstr(response + header_end, "\r\n0\r\n\r\n") || strstr(response + header_end, "0\r\n\r\n")) {
                    break;
                }
            }
        }

        if (total_read >= 524288 - 1) {
            break;
        }
    }
    tsfi_tls_close(&tls);
    close(sockfd);
    
    response[total_read] = '\0';

    char *body = strstr(response, "\r\n\r\n");
    if (!body) goto cleanup;
    body += 4;

    size_t body_len = strlen(body);
    if (body_len < out_max_len) {
        strcpy(out_buffer, body);
        success = true;
    } else {
        fprintf(stderr, "[RPC DEBUG] body_len (%zu) >= out_max_len (%zu)\n", body_len, out_max_len);
    }

cleanup:
    if (response) free(response);
    return success;
}

static void ensure_cache_dirs() {
    mkdir("tmp", 0777);
    mkdir("tmp/rpc_cache", 0777);
}

bool tsfi_pulse_rpc_call(const char *to_address, const char *data_hex, char *out_hex_buffer, size_t out_max_len) {
    ensure_cache_dirs();
    
    char cache_file[512];
    const char *clean_to = to_address;
    if (strncmp(clean_to, "0x", 2) == 0) clean_to += 2;
    const char *clean_data = data_hex;
    if (strncmp(clean_data, "0x", 2) == 0) clean_data += 2;
    
    TsfiPulseHash h;
    tsfi_pulse_keccak256((const uint8_t*)clean_data, strlen(clean_data), &h);
    char hash_hex[65];
    for (int i = 0; i < 32; i++) {
        sprintf(hash_hex + i * 2, "%02x", h.data[i]);
    }
    hash_hex[64] = '\0';
    snprintf(cache_file, sizeof(cache_file), "tmp/rpc_cache/%.40s_%s.json", clean_to, hash_hex);
    
    FILE *f_cache = fopen(cache_file, "r");
    if (f_cache) {
        size_t n = fread(out_hex_buffer, 1, out_max_len - 1, f_cache);
        out_hex_buffer[n] = '\0';
        fclose(f_cache);
        if (n > 0) {
            extern void check_and_register_rpc_token_metadata(const char *to_addr, const char *data_hex, const char *response_hex);
            check_and_register_rpc_token_metadata(to_address, data_hex, out_hex_buffer);
            return true;
        }
    }
    
    char payload[1024];
    snprintf(payload, sizeof(payload), 
             "{\"jsonrpc\":\"2.0\",\"method\":\"eth_call\",\"params\":[{\"to\":\"%s\",\"data\":\"%s\"},\"latest\"],\"id\":1}",
             to_address, data_hex);
             
    bool success = exec_raw_http_rpc(payload, out_hex_buffer, out_max_len);
    if (success) {
        FILE *f_w = fopen(cache_file, "w");
        if (f_w) {
            fwrite(out_hex_buffer, 1, strlen(out_hex_buffer), f_w);
            fclose(f_w);
        }
        extern void check_and_register_rpc_token_metadata(const char *to_addr, const char *data_hex, const char *response_hex);
        check_and_register_rpc_token_metadata(to_address, data_hex, out_hex_buffer);
    }
    return success;
}

bool tsfi_pulse_rpc_call_from(const char *to_address, const char *from_address, const char *data_hex, char *out_hex_buffer, size_t out_max_len) {
    char payload[1536];
    snprintf(payload, sizeof(payload), 
             "{\"jsonrpc\":\"2.0\",\"method\":\"eth_call\",\"params\":[{\"to\":\"%s\",\"from\":\"%s\",\"data\":\"%s\"},\"latest\"],\"id\":1}",
             to_address, from_address, data_hex);
             
    return exec_raw_http_rpc(payload, out_hex_buffer, out_max_len);
}

bool tsfi_pulse_rpc_get_storage_at(const char *address, const char *slot_hex, char *out_hex_buffer, size_t out_max_len) {
    char payload[1024];
    snprintf(payload, sizeof(payload), 
             "{\"jsonrpc\":\"2.0\",\"method\":\"eth_getStorageAt\",\"params\":[\"%s\",\"%s\",\"latest\"],\"id\":1}",
             address, slot_hex);
             
    return exec_raw_http_rpc(payload, out_hex_buffer, out_max_len);
}

bool tsfi_pulse_rpc_send_raw_transaction(const char *signed_tx_hex, char *out_tx_hash, size_t out_max_len) {
    // The signed_tx_hex can be quite large (e.g., contract deployments or large calldata)
    // We dynamically allocate the payload to ensure it fits.
    size_t tx_len = strlen(signed_tx_hex);
    size_t payload_size = tx_len + 256;
    char *payload = malloc(payload_size);
    if (!payload) return false;

    snprintf(payload, payload_size, 
             "{\"jsonrpc\":\"2.0\",\"method\":\"eth_sendRawTransaction\",\"params\":[\"%s\"],\"id\":1}",
             signed_tx_hex);
             
    bool result = exec_raw_http_rpc(payload, out_tx_hash, out_max_len);
    free(payload);
    return result;
}

bool tsfi_pulse_rpc_send_wmq_transaction(const char *to_address, const char *data_hex) {
    char payload[2048];
    snprintf(payload, sizeof(payload),
             "{\"jsonrpc\":\"2.0\",\"method\":\"eth_sendTransaction\",\"params\":[{"
             "\"from\":\"0xf39Fd6e51aad88F6F4ce6aB8827279cffFb92266\","
             "\"to\":\"%s\","
             "\"data\":\"%s\","
             "\"gas\":\"0xF4240\""
             "}],\"id\":1}",
             to_address, data_hex);
             
    char out_hex[512];
    return exec_raw_http_rpc(payload, out_hex, sizeof(out_hex));
}

static uint64_t parse_hex64(const char *hex) {
    if (hex[0] == '0' && (hex[1] == 'x' || hex[1] == 'X')) {
        hex += 2;
    }
    size_t len = strlen(hex);
    if (len > 16) {
        hex += (len - 16);
    }
    return strtoull(hex, NULL, 16);
}

static uint64_t get_wmq_address_u64(char *out_addr_str) {
    FILE *f = fopen("../tmp/wmq_address.txt", "r");
    if (!f) {
        f = fopen("tmp/wmq_address.txt", "r");
    }
    if (!f) {
        f = fopen("/home/mariarahel/src/tsfi2/atropa_pulsechain/tmp/wmq_address.txt", "r");
    }
    char buf[128] = {0};
    if (f) {
        if (fgets(buf, sizeof(buf), f)) {
            size_t l = strlen(buf);
            while (l > 0 && (buf[l-1] == '\n' || buf[l-1] == '\r')) {
                buf[l-1] = '\0';
                l--;
            }
        }
        fclose(f);
    }
    if (out_addr_str) {
        strcpy(out_addr_str, buf);
    }
    if (strlen(buf) > 0) {
        return parse_hex64(buf);
    }
    return 0;
}

static void forward_to_mcp_server(const char *cmd) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) return;
    
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(10042);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
    
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) >= 0) {
        char payload[1024];
        snprintf(payload, sizeof(payload), 
                 "{\"jsonrpc\":\"2.0\",\"method\":\"wave64.dilemma_log\",\"params\":{"
                 "\"event\":\"%s\",\"source\":\"WinchesterMQ Bridge\",\"details\":\"Forwarded from C Thunk Broker\""
                 "},\"id\":1}", cmd);
        ssize_t nw = write(sockfd, payload, strlen(payload));
        (void)nw;
        char dummy[128];
        ssize_t nr = read(sockfd, dummy, sizeof(dummy));
        (void)nr;
    }
    close(sockfd);
}

void tsfi_thunk_publish_mq(const char *cmd) {
    char wmq_addr_str[128] = {0};
    uint64_t wmq_addr_u64 = get_wmq_address_u64(wmq_addr_str);
    if (wmq_addr_u64 == 0) {
        printf("[THUNK_MQ] Error: Auncient WinchesterMQ address not found in tmp/wmq_address.txt\n");
        return;
    }
    
    char cmd_buf[128];
    strncpy(cmd_buf, cmd, sizeof(cmd_buf) - 1);
    cmd_buf[sizeof(cmd_buf) - 1] = '\0';
    
    char processed[128] = {0};
    char *src = cmd_buf;
    char *dst = processed;
    while (*src && (dst - processed) < 120) {
        if (strncmp(src, "YOUTUBE:", 8) == 0) {
            strcpy(dst, "Y:");
            dst += 2;
            src += 8;
        } else if (strncmp(src, "MAIN:", 5) == 0) {
            strcpy(dst, "M:");
            dst += 2;
            src += 5;
        } else if (strncmp(src, "MOUSE_MOVE", 10) == 0) {
            strcpy(dst, "MM");
            dst += 2;
            src += 10;
        } else if (strncmp(src, "MOUSE_DOWN", 10) == 0) {
            strcpy(dst, "MD");
            dst += 2;
            src += 10;
        } else if (strncmp(src, "MOUSE_UP", 8) == 0) {
            strcpy(dst, "MU");
            dst += 2;
            src += 8;
        } else if (strncmp(src, "MOUSE_SCROLL", 12) == 0) {
            strcpy(dst, "MS");
            dst += 2;
            src += 12;
        } else if (strncmp(src, "KEY_DOWN", 8) == 0) {
            strcpy(dst, "KD");
            dst += 2;
            src += 8;
        } else if (strncmp(src, "KEY_UP", 6) == 0) {
            strcpy(dst, "KU");
            dst += 2;
            src += 6;
        } else {
            *dst++ = *src++;
        }
    }
    *dst = '\0';
    
    if (strstr(processed, "MM") || strstr(processed, "MD") || strstr(processed, "MU") ||
        strstr(processed, "MS") || strstr(processed, "KD") || strstr(processed, "KU") ||
        strstr(processed, "MO")) {
        printf("[THUNK_MQ] I/O Event bypassed RPC publish: %s\n", processed);
        return;
    }
    
    size_t len = strlen(processed);
    uint8_t data_bytes[32] = {0};
    memcpy(data_bytes, processed, len < 32 ? len : 32);
    
    char tx_data[128];
    strcpy(tx_data, "0xccb077a0");
    for (int i = 0; i < 32; i++) {
        sprintf(tx_data + 10 + i * 2, "%02x", data_bytes[i]);
    }
    
    bool ok = tsfi_pulse_rpc_send_wmq_transaction(wmq_addr_str, tx_data);
    printf("[THUNK_MQ] Event published to Auncient WinchesterMQ: %s (status: %s)\n", processed, ok ? "SUCCESS" : "FAILED");
    forward_to_mcp_server(processed);
}

bool tsfi_pulse_rpc_exec_raw(const char *json_payload, char *out_buffer, size_t out_max_len) {
    return exec_raw_http_rpc(json_payload, out_buffer, out_max_len);
}

bool tsfi_pulse_rpc_exec_raw_body(const char *json_payload, char *out_buffer, size_t out_max_len) {
    return exec_raw_http_rpc_body(json_payload, out_buffer, out_max_len);
}

bool tsfi_pulse_explorer_get_holders_page(const char *token_addr, int page, char *out_buffer, size_t out_max_len) {
    ensure_cache_dirs();
    
    char cache_file[512];
    const char *clean_addr = token_addr;
    if (strncmp(clean_addr, "0x", 2) == 0) clean_addr += 2;
    
    snprintf(cache_file, sizeof(cache_file), "tmp/rpc_cache/explorer_%.40s_page_%d.json", clean_addr, page);
    
    FILE *f_cache = fopen(cache_file, "r");
    if (f_cache) {
        size_t n = fread(out_buffer, 1, out_max_len - 1, f_cache);
        out_buffer[n] = '\0';
        fclose(f_cache);
        if (n > 0) {
            return true;
        }
    }
    
    struct addrinfo hints, *res;
    int sockfd = -1;
    bool success = false;
    char *response = NULL;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    const char *host = "api.scan.pulsechain.com";
    if (getaddrinfo(host, "443", &hints, &res) != 0) {
        return false;
    }

    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd < 0) {
        freeaddrinfo(res);
        return false;
    }

    struct timeval tv_timeout;
    tv_timeout.tv_sec = 5;
    tv_timeout.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv_timeout, sizeof(tv_timeout));
    setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &tv_timeout, sizeof(tv_timeout));

    int flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

    int conn_res = connect(sockfd, res->ai_addr, res->ai_addrlen);
    if (conn_res < 0) {
        if (errno == EINPROGRESS) {
            fd_set write_fds;
            FD_ZERO(&write_fds);
            FD_SET(sockfd, &write_fds);
            struct timeval tv_select = { 1, 0 };
            int sel_res = select(sockfd + 1, NULL, &write_fds, NULL, &tv_select);
            if (sel_res <= 0 || !FD_ISSET(sockfd, &write_fds)) {
                close(sockfd);
                freeaddrinfo(res);
                return false;
            }
            int optval;
            socklen_t optlen = sizeof(optval);
            if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0 || optval != 0) {
                close(sockfd);
                freeaddrinfo(res);
                return false;
            }
        } else {
            close(sockfd);
            freeaddrinfo(res);
            return false;
        }
    }

    fcntl(sockfd, F_SETFL, flags);
    freeaddrinfo(res);

    TsfiTlsContext tls;
    tsfi_tls_init(&tls, sockfd);
    if (!tsfi_tls_handshake(&tls, host)) {
        tsfi_tls_close(&tls);
        close(sockfd);
        return false;
    }

    char request[2048];
    snprintf(request, sizeof(request),
             "GET /api?module=token&action=getTokenHolders&contractaddress=%s&page=%d&offset=100 HTTP/1.1\r\n"
             "Host: %s\r\n"
             "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) Antigravity/2.0\r\n"
             "Connection: close\r\n"
             "\r\n",
             token_addr, page, host);

    if (tsfi_tls_write(&tls, request, strlen(request)) < 0) {
        tsfi_tls_close(&tls);
        close(sockfd);
        return false;
    }

    response = malloc(524288);
    if (!response) {
        tsfi_tls_close(&tls);
        close(sockfd);
        return false;
    }
    memset(response, 0, 524288);
    size_t total_read = 0;
    ssize_t n;
    while ((n = tsfi_tls_read(&tls, response + total_read, 524288 - 1 - total_read)) > 0) {
        total_read += n;
        if (total_read >= 524288 - 1) break;
    }
    tsfi_tls_close(&tls);
    close(sockfd);

    char *body = strstr(response, "\r\n\r\n");
    if (body) {
        body += 4;
        size_t body_len = strlen(body);
        if (body_len < out_max_len) {
            strcpy(out_buffer, body);
            success = true;
            
            // Save to cache
            FILE *f_w = fopen(cache_file, "w");
            if (f_w) {
                fwrite(body, 1, body_len, f_w);
                fclose(f_w);
            }
        }
    }
    free(response);
    return success;
}

bool tsfi_pulse_explorer_get_holders(const char *token_addr, char *out_buffer, size_t out_max_len) {
    char *combined = malloc(524288);
    if (!combined) return false;
    strcpy(combined, "{\"result\":[");
    size_t comb_len = strlen(combined);
    
    bool any_success = false;
    int items_added = 0;
    for (int p = 1; p <= 5; p++) {
        char *p_buf = malloc(131072);
        if (!p_buf) continue;
        if (tsfi_pulse_explorer_get_holders_page(token_addr, p, p_buf, 131072)) {
            char *res_ptr = strstr(p_buf, "\"result\":[");
            if (res_ptr) {
                res_ptr += 10;
                char *res_end = strchr(res_ptr, ']');
                if (res_end && res_end > res_ptr) {
                    size_t chunk_len = res_end - res_ptr;
                    if (chunk_len > 0 && comb_len + chunk_len + 3 < 524288) {
                        if (items_added > 0) {
                            combined[comb_len++] = ',';
                        }
                        memcpy(combined + comb_len, res_ptr, chunk_len);
                        comb_len += chunk_len;
                        combined[comb_len] = '\0';
                        items_added++;
                        any_success = true;
                    }
                }
            }
        }
        free(p_buf);
    }
    strcat(combined, "]}");
    if (any_success && strlen(combined) < out_max_len) {
        strcpy(out_buffer, combined);
    }
    free(combined);
    return any_success;
}

bool tsfi_dexscreener_get_price(const char *token_addr, double *out_price_usd) {
    const char *host = "api.dexscreener.com";
    const char *port = "443";
    
    // Create cache directory if needed
    struct stat st_dir;
    if (stat("tmp", &st_dir) != 0) {
        mkdir("tmp", 0777);
    }
    
    // Determine cache file path
    char clean_addr[64];
    const char *addr_ptr = token_addr;
    if (strncmp(addr_ptr, "0x", 2) == 0) addr_ptr += 2;
    strcpy(clean_addr, addr_ptr);
    for (int i = 0; clean_addr[i]; i++) {
        if (clean_addr[i] >= 'A' && clean_addr[i] <= 'Z') {
            clean_addr[i] += 32;
        }
    }
    char cache_file[256];
    snprintf(cache_file, sizeof(cache_file), "tmp/dex_cache_%s.json", clean_addr);
    
    // Check if cache file is less than 60 seconds old
    struct stat st_file;
    if (stat(cache_file, &st_file) == 0) {
        time_t now = time(NULL);
        if (now - st_file.st_mtime < 60) {
            FILE *f_cache = fopen(cache_file, "r");
            if (f_cache) {
                char price_buf[64] = {0};
                if (fgets(price_buf, sizeof(price_buf) - 1, f_cache)) {
                    *out_price_usd = strtod(price_buf, NULL);
                    fclose(f_cache);
                    if (*out_price_usd > 0.0) {
                        return true;
                    }
                } else {
                    fclose(f_cache);
                }
            }
        }
    }
    
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    
    if (getaddrinfo(host, port, &hints, &res) != 0) {
        return false;
    }
    
    int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd < 0) {
        freeaddrinfo(res);
        return false;
    }
    
    struct timeval timeout;
    timeout.tv_sec = 3;
    timeout.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
    
    if (connect(sockfd, res->ai_addr, res->ai_addrlen) < 0) {
        close(sockfd);
        freeaddrinfo(res);
        return false;
    }
    freeaddrinfo(res);
    
    TsfiTlsContext tls;
    tsfi_tls_init(&tls, sockfd);
    if (!tsfi_tls_handshake(&tls, host)) {
        tsfi_tls_close(&tls);
        close(sockfd);
        return false;
    }
    
    char request[512];
    snprintf(request, sizeof(request),
             "GET /latest/dex/tokens/%s HTTP/1.1\r\n"
             "Host: %s\r\n"
             "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) Antigravity/2.0\r\n"
             "Connection: close\r\n"
             "\r\n",
             token_addr, host);
             
    if (tsfi_tls_write(&tls, request, strlen(request)) < 0) {
        tsfi_tls_close(&tls);
        close(sockfd);
        return false;
    }
    
    char *response = malloc(262144);
    if (!response) {
        tsfi_tls_close(&tls);
        close(sockfd);
        return false;
    }
    memset(response, 0, 262144);
    
    size_t total_read = 0;
    ssize_t n;
    while ((n = tsfi_tls_read(&tls, response + total_read, 262144 - 1 - total_read)) > 0) {
        total_read += n;
        if (total_read >= 262144 - 1) break;
    }
    tsfi_tls_close(&tls);
    close(sockfd);
    
    bool success = false;
    char *body = strstr(response, "\r\n\r\n");
    if (body) {
        body += 4;
        char *price_ptr = strstr(body, "\"priceUsd\":\"");
        if (price_ptr) {
            price_ptr += 12;
            char *end_price = strchr(price_ptr, '"');
            if (end_price) {
                char price_str[64];
                size_t len = end_price - price_ptr;
                if (len < 63) {
                    strncpy(price_str, price_ptr, len);
                    price_str[len] = '\0';
                    *out_price_usd = strtod(price_str, NULL);
                    if (*out_price_usd > 0.0) {
                        success = true;
                        // Cache the price to file
                        FILE *f_cache = fopen(cache_file, "w");
                        if (f_cache) {
                            fprintf(f_cache, "%f\n", *out_price_usd);
                            fclose(f_cache);
                        }
                    }
                }
            }
        }
    }
    free(response);
    return success;
}

bool tsfi_dexscreener_get_pairs_json(const char *token_addr, char *out_json, size_t out_max_len) {
    const char *host = "api.dexscreener.com";
    const char *port = "443";
    
    // Create cache directory if needed
    struct stat st_dir;
    if (stat("tmp", &st_dir) != 0) {
        mkdir("tmp", 0777);
    }
    
    // Determine cache file path
    char clean_addr[64];
    const char *addr_ptr = token_addr;
    if (strncmp(addr_ptr, "0x", 2) == 0) addr_ptr += 2;
    strcpy(clean_addr, addr_ptr);
    for (int i = 0; clean_addr[i]; i++) {
        if (clean_addr[i] >= 'A' && clean_addr[i] <= 'Z') {
            clean_addr[i] += 32;
        }
    }
    char cache_file[256];
    snprintf(cache_file, sizeof(cache_file), "tmp/dex_pairs_cache_%s.json", clean_addr);
    
    // Check if cache file is less than 60 seconds old
    struct stat st_file;
    if (stat(cache_file, &st_file) == 0) {
        time_t now = time(NULL);
        if (now - st_file.st_mtime < 60) {
            FILE *f_cache = fopen(cache_file, "r");
            if (f_cache) {
                size_t n = fread(out_json, 1, out_max_len - 1, f_cache);
                out_json[n] = '\0';
                fclose(f_cache);
                if (n > 0) {
                    return true;
                }
            }
        }
    }
    
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    
    if (getaddrinfo(host, port, &hints, &res) != 0) {
        return false;
    }
    
    int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd < 0) {
        freeaddrinfo(res);
        return false;
    }
    
    struct timeval timeout;
    timeout.tv_sec = 3;
    timeout.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
    
    if (connect(sockfd, res->ai_addr, res->ai_addrlen) < 0) {
        close(sockfd);
        freeaddrinfo(res);
        return false;
    }
    freeaddrinfo(res);
    
    TsfiTlsContext tls;
    tsfi_tls_init(&tls, sockfd);
    if (!tsfi_tls_handshake(&tls, host)) {
        tsfi_tls_close(&tls);
        close(sockfd);
        return false;
    }
    
    char request[512];
    snprintf(request, sizeof(request),
             "GET /latest/dex/tokens/%s HTTP/1.1\r\n"
             "Host: %s\r\n"
             "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) Antigravity/2.0\r\n"
             "Accept: application/json\r\n"
             "Connection: close\r\n"
             "\r\n",
             token_addr, host);
             
    if (tsfi_tls_write(&tls, request, strlen(request)) < 0) {
        tsfi_tls_close(&tls);
        close(sockfd);
        return false;
    }
    
    char *response = malloc(524288);
    if (!response) {
        tsfi_tls_close(&tls);
        close(sockfd);
        return false;
    }
    memset(response, 0, 524288);
    
    size_t total_read = 0;
    ssize_t n;
    while ((n = tsfi_tls_read(&tls, response + total_read, 524288 - 1 - total_read)) > 0) {
        total_read += n;
        if (total_read >= 524288 - 1) break;
    }
    tsfi_tls_close(&tls);
    close(sockfd);
    
    bool success = false;
    char *body = strstr(response, "\r\n\r\n");
    if (body) {
        body += 4;
        size_t body_len = strlen(body);
        if (body_len < out_max_len) {
            strcpy(out_json, body);
            success = true;
            
            // Cache the full JSON response
            FILE *f_cache = fopen(cache_file, "w");
            if (f_cache) {
                fwrite(body, 1, body_len, f_cache);
                fclose(f_cache);
            }
        }
    }
    free(response);
    return success;
}



