#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "hathitrust_image.h"
#include "hathitrust_oauth.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define HOST "babel.hathitrust.org"
#define PORT "443"

typedef struct {
    unsigned char *data;
    size_t len;
    size_t cap;
} ImageBuffer;

static void append_image_buffer(ImageBuffer *buf, const unsigned char *data, size_t len) {
    if (buf->len + len >= buf->cap) {
        buf->cap = (buf->len + len) * 2 + 4096;
        buf->data = realloc(buf->data, buf->cap);
    }
    memcpy(buf->data + buf->len, data, len);
    buf->len += len;
}

int hathitrust_download_page_image(
    const char *htid,
    int seq,
    int size,
    unsigned char **out_img,
    size_t *out_len
) {
    if (!htid || !out_img || !out_len) return 0;
    
    char path[1024];
    snprintf(path, sizeof(path), "/cgi/imgsrv/image?id=%s;seq=%d;size=%d", htid, seq, size);
    
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    
    if (getaddrinfo(HOST, PORT, &hints, &res) != 0) {
        return 0;
    }
    
    int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd < 0) {
        freeaddrinfo(res);
        return 0;
    }
    
    if (connect(sockfd, res->ai_addr, res->ai_addrlen) != 0) {
        close(sockfd);
        freeaddrinfo(res);
        return 0;
    }
    freeaddrinfo(res);
    
    SSL_library_init();
    SSL_load_error_strings();
    const SSL_METHOD *method = TLS_client_method();
    SSL_CTX *ctx = SSL_CTX_new(method);
    if (!ctx) {
        close(sockfd);
        return 0;
    }
    
    SSL *ssl = SSL_new(ctx);
    SSL_set_fd(ssl, sockfd);
    SSL_set_tlsext_host_name(ssl, HOST);
    
    if (SSL_connect(ssl) != 1) {
        SSL_free(ssl);
        SSL_CTX_free(ctx);
        close(sockfd);
        return 0;
    }
    
    // HTTP 1.0 request to avoid chunked transfer encoding parsing
    char request[2048];
    snprintf(request, sizeof(request),
             "GET %s HTTP/1.0\r\n"
             "Host: %s\r\n"
             "Connection: close\r\n\r\n",
             path, HOST);
             
    SSL_write(ssl, request, strlen(request));
    
    ImageBuffer recv_buf = {NULL, 0, 0};
    unsigned char temp[8192];
    int n;
    while ((n = SSL_read(ssl, temp, sizeof(temp))) > 0) {
        append_image_buffer(&recv_buf, temp, n);
    }
    
    SSL_free(ssl);
    SSL_CTX_free(ctx);
    close(sockfd);
    
    if (recv_buf.data == NULL) {
        return 0;
    }
    
    // Locate double CRLF separating headers from body
    unsigned char *body = NULL;
    for (size_t i = 0; i < recv_buf.len - 3; i++) {
        if (recv_buf.data[i] == '\r' && recv_buf.data[i+1] == '\n' &&
            recv_buf.data[i+2] == '\r' && recv_buf.data[i+3] == '\n') {
            body = recv_buf.data + i + 4;
            break;
        }
    }
    
    if (!body) {
        free(recv_buf.data);
        return 0;
    }
    
    size_t header_len = body - recv_buf.data;
    size_t body_len = recv_buf.len - header_len;
    
    unsigned char *img = malloc(body_len);
    if (!img) {
        free(recv_buf.data);
        return 0;
    }
    
    memcpy(img, body, body_len);
    *out_img = img;
    *out_len = body_len;
    
    free(recv_buf.data);
    return 1;
}

int hathitrust_download_page_image_oauth(
    const char *htid,
    int seq,
    int size,
    const char *consumer_key,
    const char *consumer_secret,
    const char *token,
    const char *token_secret,
    unsigned char **out_img,
    size_t *out_len
) {
    if (!htid || !consumer_key || !consumer_secret || !out_img || !out_len) return 0;
    
    char url[512];
    snprintf(url, sizeof(url), "https://%s/cgi/imgsrv/image?id=%s;seq=%d;size=%d", HOST, htid, seq, size);
    
    char nonce[64];
    snprintf(nonce, sizeof(nonce), "auncient_%lx", (unsigned long)time(NULL));
    
    char timestamp[32];
    snprintf(timestamp, sizeof(timestamp), "%ld", (long)time(NULL));
    
    char auth_header[2048];
    if (!hathitrust_oauth_generate_header(
            "GET", url, consumer_key, consumer_secret,
            token, token_secret, nonce, timestamp,
            auth_header, sizeof(auth_header))) {
        return 0;
    }
    
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    
    if (getaddrinfo(HOST, PORT, &hints, &res) != 0) return 0;
    
    int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd < 0) {
        freeaddrinfo(res);
        return 0;
    }
    
    if (connect(sockfd, res->ai_addr, res->ai_addrlen) != 0) {
        close(sockfd);
        freeaddrinfo(res);
        return 0;
    }
    freeaddrinfo(res);
    
    SSL_library_init();
    SSL_load_error_strings();
    const SSL_METHOD *method = TLS_client_method();
    SSL_CTX *ctx = SSL_CTX_new(method);
    if (!ctx) {
        close(sockfd);
        return 0;
    }
    
    SSL *ssl = SSL_new(ctx);
    SSL_set_fd(ssl, sockfd);
    SSL_set_tlsext_host_name(ssl, HOST);
    
    if (SSL_connect(ssl) != 1) {
        SSL_free(ssl);
        SSL_CTX_free(ctx);
        close(sockfd);
        return 0;
    }
    
    char request[4096];
    snprintf(request, sizeof(request),
             "GET /cgi/imgsrv/image?id=%s;seq=%d;size=%d HTTP/1.1\r\n"
             "Host: %s\r\n"
             "Authorization: %s\r\n"
             "User-Agent: Auncient-HathiTrust-OAuth/1.0\r\n"
             "Accept: image/jpeg, image/png, */*\r\n"
             "Connection: close\r\n\r\n",
             htid, seq, size, HOST, auth_header);
             
    SSL_write(ssl, request, strlen(request));
    
    ImageBuffer recv_buf = {NULL, 0, 0};
    unsigned char temp[8192];
    int n;
    while ((n = SSL_read(ssl, temp, sizeof(temp))) > 0) {
        append_image_buffer(&recv_buf, temp, n);
    }
    
    SSL_free(ssl);
    SSL_CTX_free(ctx);
    close(sockfd);
    
    if (recv_buf.data == NULL) return 0;
    
    unsigned char *body = NULL;
    for (size_t i = 0; i < recv_buf.len - 3; i++) {
        if (recv_buf.data[i] == '\r' && recv_buf.data[i+1] == '\n' &&
            recv_buf.data[i+2] == '\r' && recv_buf.data[i+3] == '\n') {
            body = recv_buf.data + i + 4;
            break;
        }
    }
    
    if (!body) {
        free(recv_buf.data);
        return 0;
    }
    
    size_t header_len = body - recv_buf.data;
    size_t body_len = recv_buf.len - header_len;
    
    unsigned char *img = malloc(body_len);
    if (!img) {
        free(recv_buf.data);
        return 0;
    }
    
    memcpy(img, body, body_len);
    *out_img = img;
    *out_len = body_len;
    
    free(recv_buf.data);
    return 1;
}

