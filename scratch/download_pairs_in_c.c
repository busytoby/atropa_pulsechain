#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/stat.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

bool fetch_pairs_for_address(const char *addr, char *out_buf, size_t max_len) {
    const char *host = "api.dexscreener.com";
    const char *port = "443";
    
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    
    if (getaddrinfo(host, port, &hints, &res) != 0) {
        printf("  DEBUG: getaddrinfo failed\n");
        return false;
    }
    
    int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd < 0) {
        printf("  DEBUG: socket failed\n");
        freeaddrinfo(res);
        return false;
    }
    
    struct timeval tv;
    tv.tv_sec = 4;
    tv.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
    
    if (connect(sockfd, res->ai_addr, res->ai_addrlen) < 0) {
        printf("  DEBUG: connect failed\n");
        close(sockfd);
        freeaddrinfo(res);
        return false;
    }
    freeaddrinfo(res);
    
    SSL_CTX *ctx = SSL_CTX_new(TLS_client_method());
    if (!ctx) {
        printf("  DEBUG: SSL_CTX_new failed\n");
        close(sockfd);
        return false;
    }
    
    SSL *ssl = SSL_new(ctx);
    if (!ssl) {
        printf("  DEBUG: SSL_new failed\n");
        SSL_CTX_free(ctx);
        close(sockfd);
        return false;
    }
    
    SSL_set_fd(ssl, sockfd);
    SSL_set_tlsext_host_name(ssl, host);
    if (SSL_connect(ssl) <= 0) {
        unsigned long err = ERR_get_error();
        char err_buf[256];
        ERR_error_string_n(err, err_buf, sizeof(err_buf));
        printf("  DEBUG: SSL_connect failed: %s\n", err_buf);
        SSL_free(ssl);
        SSL_CTX_free(ctx);
        close(sockfd);
        return false;
    }
    
    char addr_lower[128];
    strncpy(addr_lower, addr, sizeof(addr_lower) - 1);
    addr_lower[sizeof(addr_lower) - 1] = '\0';
    for (int i = 0; addr_lower[i]; i++) {
        if (addr_lower[i] >= 'A' && addr_lower[i] <= 'Z') {
            addr_lower[i] += 32;
        }
    }
    
    char request[512];
    snprintf(request, sizeof(request),
             "GET /latest/dex/tokens/%s HTTP/1.0\r\n"
             "Host: %s\r\n"
             "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) Antigravity/2.0\r\n"
             "Accept: application/json\r\n"
             "Connection: close\r\n\r\n",
             addr_lower, host);
             
    if (SSL_write(ssl, request, strlen(request)) <= 0) {
        printf("  DEBUG: SSL_write failed\n");
        SSL_free(ssl);
        SSL_CTX_free(ctx);
        close(sockfd);
        return false;
    }
    
    size_t total = 0;
    char temp[4096];
    int n;
    while ((n = SSL_read(ssl, temp, sizeof(temp) - 1)) > 0) {
        if (total + n < max_len) {
            memcpy(out_buf + total, temp, n);
            total += n;
        } else {
            break;
        }
    }
    out_buf[total] = '\0';
    
    SSL_free(ssl);
    SSL_CTX_free(ctx);
    close(sockfd);
    
    char *body = strstr(out_buf, "\r\n\r\n");
    if (body) {
        body += 4;
        memmove(out_buf, body, strlen(body) + 1);
        return true;
    }
    printf("DEBUG: out_buf first 200 chars: %.200s\n", out_buf);
    return false;
}

int main() {
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    
    FILE *f_sol = fopen("solidity/addresses.sol", "r");
    if (!f_sol) {
        f_sol = fopen("../solidity/addresses.sol", "r");
    }
    if (!f_sol) {
        printf("Failed to open solidity/addresses.sol\n");
        return 1;
    }
    
    char line[512];
    char addresses[256][64];
    int addr_count = 0;
    
    while (fgets(line, sizeof(line), f_sol)) {
        char *addr_start = strstr(line, "address(0x");
        if (addr_start) {
            addr_start += 8;
            char *addr_end = strchr(addr_start, ')');
            if (addr_end && (addr_end - addr_start) == 42) {
                char clean_addr[64] = "0x";
                strncat(clean_addr, addr_start + 2, 40);
                
                bool dup = false;
                for (int k = 0; k < addr_count; k++) {
                    if (strcasecmp(addresses[k], clean_addr) == 0) {
                        dup = true;
                        break;
                    }
                }
                if (!dup && addr_count < 256) {
                    strcpy(addresses[addr_count++], clean_addr);
                }
            }
        }
    }
    fclose(f_sol);
    
    printf("Parsed %d tokens from addresses.sol\n", addr_count);
    
    // Create cache directory if needed
    struct stat st_dir;
    if (stat("tsfi2-deepseek/tmp", &st_dir) != 0) {
        mkdir("tsfi2-deepseek/tmp", 0777);
    }
    
    char *res_buf = malloc(512 * 1024);
    for (int i = 0; i < addr_count; i++) {
        char clean_lower[64];
        strcpy(clean_lower, addresses[i] + 2);
        for (int k = 0; clean_lower[k]; k++) {
            if (clean_lower[k] >= 'A' && clean_lower[k] <= 'Z') {
                clean_lower[k] += 32;
            }
        }
        
        char cache_file[512];
        snprintf(cache_file, sizeof(cache_file), "tsfi2-deepseek/tmp/dex_pairs_cache_%s.json", clean_lower);
        
        // Overwrite if the cache file is missing, empty, or incomplete (less than 5KB)
        struct stat st;
        if (stat(cache_file, &st) == 0 && st.st_size < 5000) {
            unlink(cache_file);
        }
        
        if (stat(cache_file, &st) == 0) {
            // Already fetched successfully and is complete
            continue;
        }
        
        bool success = false;
        int retries = 0;
        while (!success && retries < 5) {
            printf("[%d/%d] Fetching pairs for %s (%s) [attempt %d]...\n", i + 1, addr_count, addresses[i], clean_lower, retries + 1);
            if (fetch_pairs_for_address(addresses[i], res_buf, 512 * 1024)) {
                if (strstr(res_buf, "1015") || strstr(res_buf, "error code")) {
                    printf("  Rate limited (Error 1015). Sleeping for 30 seconds...\n");
                    sleep(30);
                    retries++;
                } else {
                    FILE *f_cache = fopen(cache_file, "w");
                    if (f_cache) {
                        fputs(res_buf, f_cache);
                        fclose(f_cache);
                        printf("  Saved %zu bytes\n", strlen(res_buf));
                    }
                    success = true;
                }
            } else {
                printf("  Failed to fetch. Retrying in 10 seconds...\n");
                sleep(10);
                retries++;
            }
            sleep(5);
        }
    }
    
    free(res_buf);
    printf("C-based batch download complete!\n");
    return 0;
}
