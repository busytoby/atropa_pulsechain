#include "tsfi_tls.h"
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <unistd.h>
#include <stdio.h>

void tsfi_tls_init(TsfiTlsContext *ctx, int connected_sockfd) {
    ctx->state = 0;
    ctx->socket_fd = connected_sockfd;
    ctx->ssl_ctx = NULL;
    ctx->ssl = NULL;
}

bool tsfi_tls_handshake(TsfiTlsContext *ctx, const char *hostname) {
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();

    const SSL_METHOD *method = TLS_client_method();
    SSL_CTX *ssl_ctx = SSL_CTX_new(method);
    if (!ssl_ctx) {
        return false;
    }

    // Allow TLS 1.2 fallback and TLS 1.3
    SSL_CTX_set_min_proto_version(ssl_ctx, TLS1_2_VERSION);
    SSL_CTX_set_max_proto_version(ssl_ctx, TLS1_3_VERSION);

    SSL *ssl = SSL_new(ssl_ctx);
    if (!ssl) {
        SSL_CTX_free(ssl_ctx);
        return false;
    }

    SSL_set_fd(ssl, ctx->socket_fd);
    SSL_set_tlsext_host_name(ssl, hostname);

    if (SSL_connect(ssl) <= 0) {
        SSL_free(ssl);
        SSL_CTX_free(ssl_ctx);
        return false;
    }

    ctx->ssl_ctx = (void *)ssl_ctx;
    ctx->ssl = (void *)ssl;
    return true;
}

ssize_t tsfi_tls_write(TsfiTlsContext *ctx, const void *buf, size_t len) {
    if (!ctx->ssl) return -1;
    return SSL_write((SSL *)ctx->ssl, buf, len);
}

ssize_t tsfi_tls_read(TsfiTlsContext *ctx, void *buf, size_t max_len) {
    if (!ctx->ssl) return -1;
    return SSL_read((SSL *)ctx->ssl, buf, max_len);
}

void tsfi_tls_close(TsfiTlsContext *ctx) {
    if (ctx->ssl) {
        SSL_shutdown((SSL *)ctx->ssl);
        SSL_free((SSL *)ctx->ssl);
        ctx->ssl = NULL;
    }
    if (ctx->ssl_ctx) {
        SSL_CTX_free((SSL_CTX *)ctx->ssl_ctx);
        ctx->ssl_ctx = NULL;
    }
}
