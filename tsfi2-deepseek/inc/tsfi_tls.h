#ifndef TSFI_TLS_H
#define TSFI_TLS_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

// A minimal, strictly bounded TLS 1.3 engine hardcoded for rpc.pulsechain.com
// Cipher: TLS_AES_256_GCM_SHA384 (0x13, 0x02)
// Key Exchange: X25519 (0x00, 0x1d)

typedef struct {
    int state;
    int socket_fd;
    uint8_t read_buffer[8192];
    size_t read_len;
    size_t read_pos;
    
    // Cryptographic State
    uint8_t client_random[32];
    uint8_t server_random[32];
    uint8_t session_hash[48]; // SHA-384
    
    uint8_t client_write_key[32]; // AES-256
    uint8_t client_write_iv[12];  // GCM IV
    uint8_t server_write_key[32]; // AES-256
    uint8_t server_write_iv[12];  // GCM IV
    
    uint64_t client_sequence_num;
    uint64_t server_sequence_num;

} TsfiTlsContext;

// Initialize the TLS context around a connected raw POSIX TCP socket
void tsfi_tls_init(TsfiTlsContext *ctx, int connected_sockfd);

// Perform the TLS 1.3 handshake (ClientHello -> ServerHello -> Finished)
bool tsfi_tls_handshake(TsfiTlsContext *ctx, const char *hostname);

// Encrypt and send application data
ssize_t tsfi_tls_write(TsfiTlsContext *ctx, const void *buf, size_t len);

// Read and decrypt application data
ssize_t tsfi_tls_read(TsfiTlsContext *ctx, void *buf, size_t max_len);

// Gracefully close the TLS session
void tsfi_tls_close(TsfiTlsContext *ctx);

#ifdef __cplusplus
}
#endif

#endif // TSFI_TLS_H
