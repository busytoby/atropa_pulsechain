#define _GNU_SOURCE
#include "tsfi_tls.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>

typedef struct {
    int server_port;
    EVP_PKEY *pkey;
    X509 *x509;
} ServerConfig;

static void *server_thread_func(void *arg) {
    ServerConfig *config = (ServerConfig *)arg;

    // Create server SSL context
    const SSL_METHOD *method = TLS_server_method();
    SSL_CTX *server_ctx = SSL_CTX_new(method);
    assert(server_ctx != NULL);

    SSL_CTX_set_min_proto_version(server_ctx, TLS1_3_VERSION);
    SSL_CTX_set_max_proto_version(server_ctx, TLS1_3_VERSION);

    // Use programmatic certificate and key
    int ret = SSL_CTX_use_certificate(server_ctx, config->x509);
    assert(ret == 1);
    ret = SSL_CTX_use_PrivateKey(server_ctx, config->pkey);
    assert(ret == 1);

    // Bind TCP socket
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    assert(listen_fd >= 0);

    int reuse = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons(config->server_port);

    ret = bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr));
    assert(ret == 0);

    ret = listen(listen_fd, 1);
    assert(ret == 0);

    // Accept connection
    int client_fd = accept(listen_fd, NULL, NULL);
    assert(client_fd >= 0);

    // Perform TLS handshake
    SSL *ssl = SSL_new(server_ctx);
    assert(ssl != NULL);
    SSL_set_fd(ssl, client_fd);

    ret = SSL_accept(ssl);
    if (ret <= 0) {
        ERR_print_errors_fp(stderr);
        assert(false);
    }

    // Read payload
    char buf[256];
    int bytes = SSL_read(ssl, buf, sizeof(buf) - 1);
    assert(bytes > 0);
    buf[bytes] = '\0';

    // Verify received data
    assert(strcmp(buf, "Hello TLS 1.3 Echo") == 0);

    // Write back response
    ret = SSL_write(ssl, "Echo: Hello TLS 1.3 Echo", 24);
    assert(ret > 0);

    // Clean up
    SSL_shutdown(ssl);
    SSL_free(ssl);
    close(client_fd);
    close(listen_fd);
    SSL_CTX_free(server_ctx);

    return NULL;
}

static void test_tls_handshake_and_transfer(void) {
    printf("[Test] Running TLS 1.3 handshake and transfer loopback test...\n");

    // Initialize OpenSSL
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();

    // Generate keys and certificate programmatically to avoid external dependency
    EVP_PKEY *pkey = NULL;
    EVP_PKEY_CTX *pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);
    assert(pctx != NULL);
    int ret = EVP_PKEY_keygen_init(pctx);
    assert(ret == 1);
    ret = EVP_PKEY_CTX_set_rsa_keygen_bits(pctx, 2048);
    assert(ret == 1);
    ret = EVP_PKEY_keygen(pctx, &pkey);
    assert(ret == 1);
    EVP_PKEY_CTX_free(pctx);

    X509 *x509 = X509_new();
    assert(x509 != NULL);
    ASN1_INTEGER_set(X509_get_serialNumber(x509), 1);
    X509_gmtime_adj(X509_get_notBefore(x509), 0);
    X509_gmtime_adj(X509_get_notAfter(x509), 3600); // 1 hour validity
    X509_set_pubkey(x509, pkey);

    X509_NAME *name = X509_get_subject_name(x509);
    X509_NAME_add_entry_by_txt(name, "C", MBSTRING_ASC, (unsigned char *)"US", -1, -1, 0);
    X509_NAME_add_entry_by_txt(name, "O", MBSTRING_ASC, (unsigned char *)"Atropa", -1, -1, 0);
    X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC, (unsigned char *)"127.0.0.1", -1, -1, 0);
    X509_set_issuer_name(x509, name);

    ret = X509_sign(x509, pkey, EVP_sha256());
    assert(ret > 0);

    // Pick an ephemeral port
    int dummy_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in dummy_addr;
    memset(&dummy_addr, 0, sizeof(dummy_addr));
    dummy_addr.sin_family = AF_INET;
    dummy_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    dummy_addr.sin_port = 0;
    bind(dummy_fd, (struct sockaddr *)&dummy_addr, sizeof(dummy_addr));
    socklen_t addr_len = sizeof(dummy_addr);
    getsockname(dummy_fd, (struct sockaddr *)&dummy_addr, &addr_len);
    int port = ntohs(dummy_addr.sin_port);
    close(dummy_fd);

    ServerConfig config;
    config.server_port = port;
    config.pkey = pkey;
    config.x509 = x509;

    pthread_t thread;
    ret = pthread_create(&thread, NULL, server_thread_func, &config);
    assert(ret == 0);

    // Wait a brief moment for the server to bind and listen
    usleep(50000);

    // Connect client TCP socket
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    assert(sock_fd >= 0);

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    server_addr.sin_port = htons(port);

    ret = connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    assert(ret == 0);

    // Wrap in TsfiTlsContext and handshake
    TsfiTlsContext tls_ctx;
    tsfi_tls_init(&tls_ctx, sock_fd);
    
    bool handshake_ok = tsfi_tls_handshake(&tls_ctx, "127.0.0.1");
    assert(handshake_ok);

    // Write application data
    ssize_t written = tsfi_tls_write(&tls_ctx, "Hello TLS 1.3 Echo", 18);
    assert(written == 18);

    // Read echo response
    char response[256];
    ssize_t read_bytes = tsfi_tls_read(&tls_ctx, response, sizeof(response) - 1);
    assert(read_bytes > 0);
    response[read_bytes] = '\0';
    assert(strcmp(response, "Echo: Hello TLS 1.3 Echo") == 0);

    // Close TLS and TCP socket
    tsfi_tls_close(&tls_ctx);
    close(sock_fd);

    // Wait for server thread to finish
    pthread_join(thread, NULL);

    X509_free(x509);
    EVP_PKEY_free(pkey);

    printf("[Test] TLS 1.3 handshake and transfer loopback test passed!\n");
}

int main(void) {
    printf("=== Starting Phase 4 TLS Unit Tests ===\n");
    test_tls_handshake_and_transfer();
    printf("=== All Phase 4 TLS Unit Tests Passed Successfully ===\n");
    return 0;
}
