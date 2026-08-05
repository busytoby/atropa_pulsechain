#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "hathitrust_solr.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define HOST "catalog.hathitrust.org"
#define PORT "443"

typedef struct {
    char *data;
    size_t len;
    size_t cap;
} ResponseBuffer;

static void append_response_buffer(ResponseBuffer *buf, const char *data, size_t len) {
    if (buf->len + len >= buf->cap) {
        buf->cap = (buf->len + len) * 2 + 1024;
        buf->data = realloc(buf->data, buf->cap);
    }
    memcpy(buf->data + buf->len, data, len);
    buf->len += len;
    buf->data[buf->len] = '\0';
}

static void url_encode_query(const char *src, char *dest, size_t dest_len) {
    size_t i = 0;
    while (*src && i < dest_len - 4) {
        unsigned char c = (unsigned char)*src;
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') ||
            c == '-' || c == '_' || c == '.' || c == '~') {
            dest[i++] = c;
        } else if (c == ' ') {
            dest[i++] = '+';
        } else {
            snprintf(dest + i, dest_len - i, "%%%02X", c);
            i += 3;
        }
        src++;
    }
    dest[i] = '\0';
}

char* hathitrust_solr_search(const char *field, const char *query) {
    if (!field || !query) return NULL;
    
    char encoded_query[512];
    url_encode_query(query, encoded_query, sizeof(encoded_query));
    
    char path[1024];
    snprintf(path, sizeof(path), "/api/search/brief/json/%s:%s", field, encoded_query);
    
    // Set up DNS/Socket
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
    
    // Set up SSL
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
    
    // Send HTTP GET
    char request[2048];
    snprintf(request, sizeof(request),
             "GET %s HTTP/1.1\r\n"
             "Host: %s\r\n"
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
