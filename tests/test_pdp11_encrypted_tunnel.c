#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

// PDP-11 Style MMIO Register Offsets
#define CSR_OFFSET     0x00       // Control Status Register (Bit 7: Active, Bit 6: Data Ready)
#define DATA_OFFSET    0x04       // Data transmission register port

#define AES_KEY_SIZE   32
#define AES_IV_SIZE    16

// AES encryption helper
int aes_encrypt(const uint8_t *plaintext, int plaintext_len, const uint8_t *key,
                const uint8_t *iv, uint8_t *ciphertext) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    int len = 0, ciphertext_len = 0;
    EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv);
    EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len);
    ciphertext_len = len;
    EVP_EncryptFinal_ex(ctx, ciphertext + len, &len);
    ciphertext_len += len;
    EVP_CIPHER_CTX_free(ctx);
    return ciphertext_len;
}

// AES decryption helper
int aes_decrypt(const uint8_t *ciphertext, int ciphertext_len, const uint8_t *key,
                const uint8_t *iv, uint8_t *plaintext) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    int len = 0, plaintext_len = 0;
    EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv);
    EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len);
    plaintext_len = len;
    EVP_DecryptFinal_ex(ctx, plaintext + len, &len);
    plaintext_len += len;
    EVP_CIPHER_CTX_free(ctx);
    return plaintext_len;
}

// Mock MMIO Space
typedef struct {
    uint32_t csr;
    uint32_t data;
} MmioInterface;

// Peer Node Context
typedef struct {
    int socket_fd;
    uint8_t secret_key[AES_KEY_SIZE];
    uint8_t iv[AES_IV_SIZE];
    MmioInterface mmio;
    pthread_t thread;
    bool is_running;
} PeerNode;

// Peer Receiver Thread
void *peer_receiver_thread(void *arg) {
    PeerNode *node = (PeerNode *)arg;
    uint8_t encrypted_packet[64];
    uint8_t decrypted_payload[32];
    
    while (node->is_running) {
        ssize_t n = recv(node->socket_fd, encrypted_packet, sizeof(encrypted_packet), 0);
        if (n <= 0) break;
        
        // Decrypt the raw payload
        int dec_len = aes_decrypt(encrypted_packet, (int)n, node->secret_key, node->iv, decrypted_payload);
        if (dec_len >= 4) {
            uint32_t received_val = ((uint32_t)decrypted_payload[0]) |
                                    ((uint32_t)decrypted_payload[1] << 8) |
                                    ((uint32_t)decrypted_payload[2] << 16) |
                                    ((uint32_t)decrypted_payload[3] << 24);
            
            // Mount updates: write directly to local MMIO registers
            node->mmio.data = received_val;
            node->mmio.csr |= 0x40; // Set DATA READY flag (Bit 6)
        }
    }
    return NULL;
}

// Write operation that intercepts MMIO and routes to the encrypted TCP socket
void dispatch_mmio_write(PeerNode *node, uint32_t offset, uint32_t val) {
    if (offset == DATA_OFFSET) {
        node->mmio.data = val;
        
        // Encrypt the register value
        uint8_t plaintext[4];
        plaintext[0] = (uint8_t)(val & 0xFF);
        plaintext[1] = (uint8_t)((val >> 8) & 0xFF);
        plaintext[2] = (uint8_t)((val >> 16) & 0xFF);
        plaintext[3] = (uint8_t)((val >> 24) & 0xFF);
        
        uint8_t ciphertext[64];
        int enc_len = aes_encrypt(plaintext, 4, node->secret_key, node->iv, ciphertext);
        
        // Broadcast over TCP
        send(node->socket_fd, ciphertext, enc_len, 0);
    } else if (offset == CSR_OFFSET) {
        node->mmio.csr = val;
    }
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT ZMM VM: ENCRYPTED PEER MMIO TUNNEL DEMO\n");
    printf("=============================================================\n");

    // Initialize Cryptography Engine
    OpenSSL_add_all_algorithms();
    
    // Shared parameters
    uint8_t shared_key[AES_KEY_SIZE];
    uint8_t shared_iv[AES_IV_SIZE];
    RAND_bytes(shared_key, AES_KEY_SIZE);
    RAND_bytes(shared_iv, AES_IV_SIZE);

    // Setup TCP Server Socket
    int server_listen = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_listen, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(9876);
    
    bind(server_listen, (struct sockaddr *)&addr, sizeof(addr));
    listen(server_listen, 1);

    // Peer A (Host) Context
    PeerNode peer_a;
    memset(&peer_a, 0, sizeof(peer_a));
    memcpy(peer_a.secret_key, shared_key, AES_KEY_SIZE);
    memcpy(peer_a.iv, shared_iv, AES_IV_SIZE);
    peer_a.mmio.csr = 0x80; // Active status
    peer_a.is_running = true;

    // Peer B (Remote Client) Context
    PeerNode peer_b;
    memset(&peer_b, 0, sizeof(peer_b));
    memcpy(peer_b.secret_key, shared_key, AES_KEY_SIZE);
    memcpy(peer_b.iv, shared_iv, AES_IV_SIZE);
    peer_b.mmio.csr = 0x80; // Active status
    peer_b.is_running = true;

    // Connect Client (Peer B) to Server
    peer_b.socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    connect(peer_b.socket_fd, (struct sockaddr *)&addr, sizeof(addr));

    // Accept Connection (Peer A)
    peer_a.socket_fd = accept(server_listen, NULL, NULL);

    // Spin up receiver threads for both ends of the tunnel
    pthread_create(&peer_a.thread, NULL, peer_receiver_thread, &peer_a);
    pthread_create(&peer_b.thread, NULL, peer_receiver_thread, &peer_b);

    // --- Start Demo Verification ---
    printf("1. Mounted encrypted tunnel established between local loopback sockets.\n");
    printf("2. Peer A writes to its local DATA register at offset 0x%X...\n", DATA_OFFSET);
    
    // Peer A writes 0xDEADC0DE to its local MMIO register
    uint32_t val_to_send = 0xDEADC0DE;
    dispatch_mmio_write(&peer_a, DATA_OFFSET, val_to_send);
    printf("   -> Plaintext 0x%X encrypted with AES-256 and sent over network.\n", val_to_send);

    // Low-latency busy-wait polling on Peer B's CSR register for data ready flag
    int timeout_count = 0;
    while ((peer_b.mmio.csr & 0x40) == 0 && timeout_count++ < 100000) {
        usleep(1);
    }
    // Check Peer B's MMIO register state
    printf("3. Reading Peer B's MMIO registers...\n");
    printf("   -> Peer B CSR Register: 0x%X (Flag 0x40 indicates Data Ready)\n", peer_b.mmio.csr);
    printf("   -> Peer B DATA Register: 0x%X (Decrypted payload match)\n", peer_b.mmio.data);

    assert((peer_b.mmio.csr & 0x40) != 0);
    assert(peer_b.mmio.data == 0xDEADC0DE);
    printf("   ✓ Encrypted tunnel mount verified successfully!\n\n");

    // Clean up
    peer_a.is_running = false;
    peer_b.is_running = false;
    shutdown(peer_a.socket_fd, SHUT_RDWR);
    shutdown(peer_b.socket_fd, SHUT_RDWR);
    close(peer_a.socket_fd);
    close(peer_b.socket_fd);
    close(server_listen);
    pthread_join(peer_a.thread, NULL);
    pthread_join(peer_b.thread, NULL);

    printf("=============================================================\n");
    printf("AUNCIENT ENCRYPTED PEER MMIO TUNNEL DEMO SUCCESS\n");
    printf("=============================================================\n");
    return 0;
}
