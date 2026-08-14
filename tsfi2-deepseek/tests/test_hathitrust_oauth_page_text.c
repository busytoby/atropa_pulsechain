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

char* hathitrust_oauth_download_text(
    const char *htid,
    int seq,
    const char *consumer_key,
    const char *consumer_secret,
    const char *token,
    const char *token_secret
) {
    if (!htid || !consumer_key || !consumer_secret) return NULL;

    char url[512];
    snprintf(url, sizeof(url), "https://%s/cgi/imgsrv/download/text?id=%s;seq=%d", HOST, htid, seq);

    char nonce[64];
    snprintf(nonce, sizeof(nonce), "auncient_%lx", (unsigned long)time(NULL));

    char timestamp[32];
    snprintf(timestamp, sizeof(timestamp), "%ld", (long)time(NULL));

    char auth_header[2048];
    if (!hathitrust_oauth_generate_header(
            "GET",
            url,
            consumer_key,
            consumer_secret,
            token,
            token_secret,
            nonce,
            timestamp,
            auth_header,
            sizeof(auth_header))) {
        return NULL;
    }

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
    snprintf(request, sizeof(request),
             "GET /cgi/imgsrv/download/text?id=%s;seq=%d HTTP/1.1\r\n"
             "Host: %s\r\n"
             "Authorization: %s\r\n"
             "User-Agent: Auncient-HathiTrust-OAuth-Client/1.0\r\n"
             "Accept: text/plain, */*\r\n"
             "Connection: close\r\n\r\n",
             htid, seq, HOST, auth_header);

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

    printf("=== HATHITRUST RFC 5849 OAUTH 1.0a PAGE TEXT RETRIEVER (C) ===\n");
    printf("Target HTID: %s | Sequence: %d\n\n", htid, seq);

    if (argc < 3) {
        printf("Usage: %s <consumer_key> <consumer_secret> [token] [token_secret]\n", argv[0]);
        printf("Note: Obtains authorized challenge keys from standard Google Chrome OAuth login.\n\n");
        return 0;
    }

    const char *key = argv[1];
    const char *secret = argv[2];
    const char *token = (argc > 3) ? argv[3] : NULL;
    const char *token_sec = (argc > 4) ? argv[4] : NULL;

    printf("Executing signed OAuth GET request to HathiTrust...\n");
    char *resp = hathitrust_oauth_download_text(htid, seq, key, secret, token, token_sec);
    if (resp) {
        printf("--- RESPONSE RECEIVED (%zu bytes) ---\n", strlen(resp));
        printf("%s\n", resp);
        free(resp);
    } else {
        printf("OAuth request execution failed.\n");
    }

    return 0;
}
