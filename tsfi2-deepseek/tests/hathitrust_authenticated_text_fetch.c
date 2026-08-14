#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define HOST "babel.hathitrust.org"
#define PORT "443"

typedef struct {
    char *data;
    size_t len;
    size_t cap;
} ResponseBuffer;

static void append_response_buffer(ResponseBuffer *buf, const char *data, size_t len) {
    if (buf->len + len >= buf->cap) {
        buf->cap = (buf->len + len) * 2 + 4096;
        buf->data = realloc(buf->data, buf->cap);
    }
    memcpy(buf->data + buf->len, data, len);
    buf->len += len;
    buf->data[buf->len] = '\0';
}

char* hathitrust_fetch_authenticated(const char *htid, int seq, const char *cookie_header) {
    if (!htid) return NULL;
    
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    
    if (getaddrinfo(HOST, PORT, &hints, &res) != 0) return NULL;
    
    int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd < 0) {
        freeaddrinfo(res);
        return NULL;
    }
    
    if (connect(sockfd, res->ai_addr, res->ai_addrlen) != 0) {
        close(sockfd);
        freeaddrinfo(res);
        return NULL;
    }
    freeaddrinfo(res);
    
    SSL_library_init();
    SSL_load_error_strings();
    const SSL_METHOD *method = TLS_client_method();
    SSL_CTX *ctx = SSL_CTX_new(method);
    if (!ctx) {
        close(sockfd);
        return NULL;
    }
    
    SSL *ssl = SSL_new(ctx);
    SSL_set_fd(ssl, sockfd);
    SSL_set_tlsext_host_name(ssl, HOST);
    
    if (SSL_connect(ssl) != 1) {
        SSL_free(ssl);
        SSL_CTX_free(ctx);
        close(sockfd);
        return NULL;
    }
    
    char request[4096];
    if (cookie_header && strlen(cookie_header) > 0) {
        snprintf(request, sizeof(request),
                 "GET /cgi/imgsrv/download/text?id=%s;seq=%d HTTP/1.1\r\n"
                 "Host: %s\r\n"
                 "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36\r\n"
                 "Accept: text/plain, */*\r\n"
                 "Cookie: %s\r\n"
                 "Connection: close\r\n\r\n",
                 htid, seq, HOST, cookie_header);
    } else {
        snprintf(request, sizeof(request),
                 "GET /cgi/imgsrv/download/text?id=%s;seq=%d HTTP/1.1\r\n"
                 "Host: %s\r\n"
                 "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36\r\n"
                 "Accept: text/plain, */*\r\n"
                 "Connection: close\r\n\r\n",
                 htid, seq, HOST);
    }
    
    SSL_write(ssl, request, strlen(request));
    
    ResponseBuffer recv_buf = {NULL, 0, 0};
    char temp[4096];
    int n;
    while ((n = SSL_read(ssl, temp, sizeof(temp))) > 0) {
        append_response_buffer(&recv_buf, temp, n);
    }
    
    SSL_free(ssl);
    SSL_CTX_free(ctx);
    close(sockfd);
    
    return recv_buf.data;
}

int main(int argc, char **argv) {
    const char *htid = "uiug.30112109778511";
    int seq = 13;
    const char *cookie = (argc > 1) ? argv[1] : "";

    printf("=== AUTHENTICATED HATHITRUST PAGE TEXT INGESTION HARNESS (C) ===\n");
    printf("Target HTID: %s | Seq: %d\n", htid, seq);
    if (strlen(cookie) > 0) {
        printf("Using User-Supplied Session/Challenge Cookie: %.50s...\n\n", cookie);
    } else {
        printf("No Cookie supplied. (Usage: %s \"<cookie-string>\")\n\n", argv[0]);
    }

    char *resp = hathitrust_fetch_authenticated(htid, seq, cookie);
    if (resp) {
        char *body = strstr(resp, "\r\n\r\n");
        if (body) {
            body += 4;
            printf("--- SERVER RESPONSE BODY ---\n%s\n", body);
        } else {
            printf("--- RAW RESPONSE ---\n%s\n", resp);
        }
        free(resp);
    } else {
        printf("Socket query failed.\n");
    }

    return 0;
}
