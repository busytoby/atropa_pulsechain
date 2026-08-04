#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "hathitrust_client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

typedef struct {
    char *data;
    size_t size;
    size_t capacity;
} ResponseBuffer;

static void append_response_buffer(ResponseBuffer *buf, const char *new_data, size_t len) {
    if (buf->size + len + 1 > buf->capacity) {
        buf->capacity = (buf->capacity + len + 1) * 2;
        char *temp = realloc(buf->data, buf->capacity);
        if (temp) {
            buf->data = temp;
        } else {
            return;
        }
    }
    memcpy(buf->data + buf->size, new_data, len);
    buf->size += len;
    buf->data[buf->size] = '\0';
}

static char* hathitrust_query_endpoint(const char *endpoint_type, const char *id_type, const char *id_val) {
    const char *host = "catalog.hathitrust.org";
    const char *port = "443";
    
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    
    if (getaddrinfo(host, port, &hints, &res) != 0) {
        fprintf(stderr, "[ERROR] getaddrinfo failed for host: %s\n", host);
        return NULL;
    }
    
    int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd < 0) {
        fprintf(stderr, "[ERROR] socket creation failed\n");
        freeaddrinfo(res);
        return NULL;
    }
    
    struct timeval tv;
    tv.tv_sec = 10;
    tv.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
    
    if (connect(sockfd, res->ai_addr, res->ai_addrlen) < 0) {
        fprintf(stderr, "[ERROR] socket connection failed\n");
        close(sockfd);
        freeaddrinfo(res);
        return NULL;
    }
    freeaddrinfo(res);
    
    SSL_CTX *ctx = SSL_CTX_new(TLS_client_method());
    if (!ctx) {
        fprintf(stderr, "[ERROR] SSL context creation failed\n");
        close(sockfd);
        return NULL;
    }
    
    SSL *ssl = SSL_new(ctx);
    if (!ssl) {
        fprintf(stderr, "[ERROR] SSL object creation failed\n");
        SSL_CTX_free(ctx);
        close(sockfd);
        return NULL;
    }
    
    SSL_set_fd(ssl, sockfd);
    SSL_set_tlsext_host_name(ssl, host);
    if (SSL_connect(ssl) <= 0) {
        unsigned long err = ERR_get_error();
        char err_buf[256];
        ERR_error_string_n(err, err_buf, sizeof(err_buf));
        fprintf(stderr, "[ERROR] SSL connection failed: %s\n", err_buf);
        SSL_free(ssl);
        SSL_CTX_free(ctx);
        close(sockfd);
        return NULL;
    }
    
    char request[1024];
    snprintf(request, sizeof(request),
             "GET /api/volumes/%s/%s/%s.json HTTP/1.0\r\n"
             "Host: %s\r\n"
             "User-Agent: Auncient-HathiTrust-Client/2.0\r\n"
             "Connection: close\r\n\r\n",
             endpoint_type, id_type, id_val, host);
             
    if (SSL_write(ssl, request, strlen(request)) <= 0) {
        fprintf(stderr, "[ERROR] SSL write failed\n");
        SSL_free(ssl);
        SSL_CTX_free(ctx);
        close(sockfd);
        return NULL;
    }
    
    ResponseBuffer recv_buf = {NULL, 0, 0};
    char temp[4096];
    int n;
    
    while ((n = SSL_read(ssl, temp, sizeof(temp))) > 0) {
        append_response_buffer(&recv_buf, temp, n);
    }
    
    SSL_free(ssl);
    SSL_CTX_free(ctx);
    close(sockfd);
    
    if (recv_buf.data == NULL) {
        return NULL;
    }
    
    char *body = strstr(recv_buf.data, "\r\n\r\n");
    if (!body) {
        free(recv_buf.data);
        return NULL;
    }
    
    body += 4;
    char *result = strdup(body);
    free(recv_buf.data);
    return result;
}

char* hathitrust_query_brief(const char *id_type, const char *id_val) {
    return hathitrust_query_endpoint("brief", id_type, id_val);
}

char* hathitrust_query_full(const char *id_type, const char *id_val) {
    return hathitrust_query_endpoint("full", id_type, id_val);
}
