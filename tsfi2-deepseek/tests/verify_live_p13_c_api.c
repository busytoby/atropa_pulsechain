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

static char* fetch_hathitrust_raw(const char *host, const char *path) {
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    
    if (getaddrinfo(host, PORT, &hints, &res) != 0) return NULL;
    
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
    SSL_set_tlsext_host_name(ssl, host);
    
    if (SSL_connect(ssl) != 1) {
        SSL_free(ssl);
        SSL_CTX_free(ctx);
        close(sockfd);
        return NULL;
    }
    
    char request[2048];
    snprintf(request, sizeof(request),
             "GET %s HTTP/1.0\r\n"
             "Host: %s\r\n"
             "User-Agent: Auncient-C-Client/1.0\r\n"
             "Accept: */*\r\n"
             "Connection: close\r\n\r\n",
             path, host);
             
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
    printf("=== TESTING EXACT C API PAGE TEXT RETRIEVAL FOR PAGE 13 ===\n");
    printf("Target Volume: njp.32101074872001 (Deel 01, 1854)\n");
    printf("Target Sequence: seq=13\n\n");

    const char *endpoints[] = {
        "/cgi/imgsrv/download/text?id=njp.32101074872001;seq=13",
        "/cgi/imgsrv/ocr?id=njp.32101074872001;seq=13",
        "/cgi/imgsrv/manifest/njp.32101074872001",
        NULL
    };

    for (int i = 0; endpoints[i] != NULL; i++) {
        printf("--- Querying: https://%s%s ---\n", HOST, endpoints[i]);
        char *resp = fetch_hathitrust_raw(HOST, endpoints[i]);
        if (resp) {
            printf("HTTP Response Header Status:\n");
            char *hdr_end = strstr(resp, "\r\n\r\n");
            if (hdr_end) {
                size_t h_len = hdr_end - resp;
                char hdr[512] = {0};
                if (h_len < sizeof(hdr)) strncpy(hdr, resp, h_len);
                else strncpy(hdr, resp, sizeof(hdr) - 1);
                printf("%s\n\n", hdr);
            } else {
                printf("%.300s\n\n", resp);
            }
            free(resp);
        } else {
            printf("Socket connection or query failed.\n\n");
        }
    }

    return 0;
}
