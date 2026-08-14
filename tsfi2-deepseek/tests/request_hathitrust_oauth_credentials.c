#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include "../inc/hathitrust_oauth.h"

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

// Request temporary credentials (request token) or initiate OAuth handshake in C
char* hathitrust_oauth_initiate_request_token(const char *callback_url) {
    const char *url = "https://babel.hathitrust.org/cgi/oauth/request_token";
    
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
    
    char request[2048];
    snprintf(request, sizeof(request),
             "POST /cgi/oauth/request_token HTTP/1.1\r\n"
             "Host: %s\r\n"
             "User-Agent: Auncient-HathiTrust-OAuth/1.0\r\n"
             "Accept: application/x-www-form-urlencoded, text/plain, */*\r\n"
             "Content-Type: application/x-www-form-urlencoded\r\n"
             "Content-Length: %zu\r\n"
             "Connection: close\r\n\r\n"
             "oauth_callback=%s",
             HOST, strlen(callback_url) + 15, callback_url);
             
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

int main(void) {
    printf("=== REQUESTING OAUTH CREDENTIALS & HANDSHAKE VIA HATHITRUST API (C) ===\n\n");
    const char *callback = "oob"; // Out-of-band / PIN callback for desktop client

    printf("1. Requesting Temporary OAuth Request Token from https://%s/cgi/oauth/request_token...\n", HOST);
    char *resp = hathitrust_oauth_initiate_request_token(callback);
    if (resp) {
        printf("--- SERVER RESPONSE ---\n%s\n", resp);
        free(resp);
    } else {
        printf("Socket connection or request failed.\n");
    }

    return 0;
}
