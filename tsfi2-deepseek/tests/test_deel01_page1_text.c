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

char* hathitrust_get_endpoint_body(const char *path) {
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    
    if (getaddrinfo(HOST, PORT, &hints, &res) != 0) {
        return NULL;
    }
    
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
             "GET %s HTTP/1.0\r\n"
             "Host: %s\r\n"
             "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) Auncient-Client/1.0\r\n"
             "Accept: text/plain, text/html, application/json, */*\r\n"
             "Connection: close\r\n\r\n",
             path, HOST);
             
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
    
    if (recv_buf.data == NULL) return NULL;
    return recv_buf.data;
}

int main(void) {
    printf("=== TESTING PAGE 1 TEXT EXTRACTION IN C FOR DEEL 01 ===\n");
    const char *htid = "njp.32101074872001";
    int seq = 1;

    // Test text retrieval endpoints
    const char *text_endpoints[] = {
        "/cgi/imgsrv/download/text?id=%s;seq=%d",
        "/cgi/imgsrv/ocr?id=%s;seq=%d",
        "/cgi/pt?id=%s;view=1up;seq=%d;skin=mobile",
        NULL
    };

    for (int i = 0; text_endpoints[i] != NULL; i++) {
        char path[1024];
        snprintf(path, sizeof(path), text_endpoints[i], htid, seq);
        printf("\n--- Querying text endpoint: %s ---\n", path);
        char *resp = hathitrust_get_endpoint_body(path);
        if (resp) {
            printf("Response Length: %zu bytes\n", strlen(resp));
            printf("Preview:\n%.500s\n...\n", resp);
            free(resp);
        } else {
            printf("Lookup failed.\n");
        }
    }

    return 0;
}
