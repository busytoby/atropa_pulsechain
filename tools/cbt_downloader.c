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

bool download_cbt_file(const char *path, const char *out_filename) {
    const char *host = "www.cbttape.org";
    const char *port = "443";
    
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    
    if (getaddrinfo(host, port, &hints, &res) != 0) {
        fprintf(stderr, "[ERROR] getaddrinfo failed\n");
        return false;
    }
    
    int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd < 0) {
        fprintf(stderr, "[ERROR] socket failed\n");
        freeaddrinfo(res);
        return false;
    }
    
    struct timeval tv;
    tv.tv_sec = 10;
    tv.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
    
    if (connect(sockfd, res->ai_addr, res->ai_addrlen) < 0) {
        fprintf(stderr, "[ERROR] connect failed\n");
        close(sockfd);
        freeaddrinfo(res);
        return false;
    }
    freeaddrinfo(res);
    
    SSL_CTX *ctx = SSL_CTX_new(TLS_client_method());
    if (!ctx) {
        fprintf(stderr, "[ERROR] SSL_CTX_new failed\n");
        close(sockfd);
        return false;
    }
    
    SSL *ssl = SSL_new(ctx);
    if (!ssl) {
        fprintf(stderr, "[ERROR] SSL_new failed\n");
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
        fprintf(stderr, "[ERROR] SSL_connect failed: %s\n", err_buf);
        SSL_free(ssl);
        SSL_CTX_free(ctx);
        close(sockfd);
        return false;
    }
    
    char request[1024];
    snprintf(request, sizeof(request),
             "GET %s HTTP/1.0\r\n"
             "Host: %s\r\n"
             "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) Antigravity/2.0\r\n"
             "Connection: close\r\n\r\n",
             path, host);
             
    if (SSL_write(ssl, request, strlen(request)) <= 0) {
        fprintf(stderr, "[ERROR] SSL_write failed\n");
        SSL_free(ssl);
        SSL_CTX_free(ctx);
        close(sockfd);
        return false;
    }
    
    FILE *fp = fopen(out_filename, "wb");
    if (!fp) {
        fprintf(stderr, "[ERROR] Could not open output file %s\n", out_filename);
        SSL_free(ssl);
        SSL_CTX_free(ctx);
        close(sockfd);
        return false;
    }
    
    char temp[8192];
    int n;
    bool body_started = false;
    char header_buf[16384] = "";
    int header_len = 0;
    
    while ((n = SSL_read(ssl, temp, sizeof(temp))) > 0) {
        if (!body_started) {
            if (header_len + n < (int)sizeof(header_buf)) {
                memcpy(header_buf + header_len, temp, n);
                header_len += n;
                header_buf[header_len] = '\0';
                
                char *body = strstr(header_buf, "\r\n\r\n");
                if (body) {
                    body += 4;
                    int body_offset = body - header_buf;
                    int write_bytes = header_len - body_offset;
                    if (write_bytes > 0) {
                        fwrite(body, 1, write_bytes, fp);
                    }
                    body_started = true;
                }
            } else {
                body_started = true;
                fwrite(temp, 1, n, fp);
            }
        } else {
            fwrite(temp, 1, n, fp);
        }
    }
    
    fclose(fp);
    SSL_free(ssl);
    SSL_CTX_free(ctx);
    close(sockfd);
    
    return true;
}

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("Usage: %s <path_on_server> <output_file>\n", argv[0]);
        printf("Example: %s /ftp/cbt/CBT006.zip CBT006.zip\n", argv[0]);
        return 1;
    }
    
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    
    printf("[CBT DOWNLOADER] Fetching %s to %s...\n", argv[1], argv[2]);
    if (download_cbt_file(argv[1], argv[2])) {
        printf("[CBT DOWNLOADER] Successfully downloaded file.\n");
        return 0;
    } else {
        printf("[CBT DOWNLOADER ERROR] Download failed.\n");
        return 1;
    }
}
