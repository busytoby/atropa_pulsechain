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

#define PROXY_PORT     8088
#define SERVICE_PORT   9099

#define AES_KEY_SIZE   32
#define AES_IV_SIZE    16

// Bidirectional simulated MCP pipe channel
int g_mcp_pipe_b_to_a[2]; // Peer B -> Peer A
int g_mcp_pipe_a_to_b[2]; // Peer A -> Peer B

// MUX Packet Header definition
typedef struct __attribute__((packed)) {
    uint32_t connection_id;
    uint32_t payload_len;
    uint8_t iv[AES_IV_SIZE];
} MuxHeader;

// Shared keys
uint8_t g_secret_key[AES_KEY_SIZE];
uint8_t g_shared_iv[AES_IV_SIZE];

// AES helper
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

typedef struct {
    pthread_t thread;
    bool is_running;
} WorkerContext;

// Active proxy clients mapping on Peer A
#define MAX_CLIENTS 16
int g_client_fds[MAX_CLIENTS];
pthread_mutex_t g_clients_mutex = PTHREAD_MUTEX_INITIALIZER;

// Destination Echo service (Run by Peer B)
void *echo_service_thread(void *arg) {
    WorkerContext *ctx = (WorkerContext *)arg;
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(SERVICE_PORT);
    
    bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr));
    listen(listen_fd, 10);
    
    while (ctx->is_running) {
        struct timeval tv = { .tv_sec = 0, .tv_usec = 10000 };
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(listen_fd, &rfds);
        
        int r = select(listen_fd + 1, &rfds, NULL, NULL, &tv);
        if (r > 0) {
            int client_fd = accept(listen_fd, NULL, NULL);
            char buf[256];
            ssize_t n = recv(client_fd, buf, sizeof(buf) - 1, 0);
            if (n > 0) {
                buf[n] = '\0';
                char resp[512];
                snprintf(resp, sizeof(resp), "[ECHO: %s]", buf);
                send(client_fd, resp, strlen(resp), 0);
            }
            close(client_fd);
        }
    }
    close(listen_fd);
    return NULL;
}

// Peer B MCP Handler: Demuxes connection stream, decrypts, sends to service, encrypts, and responds
void *peer_b_mcp_handler(void *arg) {
    WorkerContext *ctx = (WorkerContext *)arg;
    uint8_t recv_packet[1024];
    
    while (ctx->is_running) {
        struct timeval tv = { .tv_sec = 0, .tv_usec = 10000 };
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(g_mcp_pipe_a_to_b[0], &rfds);
        
        int r = select(g_mcp_pipe_a_to_b[0] + 1, &rfds, NULL, NULL, &tv);
        if (r > 0) {
            // Read MUX header
            MuxHeader header;
            ssize_t n = read(g_mcp_pipe_a_to_b[0], &header, sizeof(header));
            if (n <= 0) break;
            
            // Read Encrypted payload
            ssize_t rd_len = read(g_mcp_pipe_a_to_b[0], recv_packet, header.payload_len);
            (void)rd_len;
            
            // Decrypt request
            uint8_t plaintext[512];
            int dec_len = aes_decrypt(recv_packet, header.payload_len, g_secret_key, header.iv, plaintext);
            plaintext[dec_len] = '\0';
            
            // Forward to local service
            int svc_fd = socket(AF_INET, SOCK_STREAM, 0);
            struct sockaddr_in addr;
            addr.sin_family = AF_INET;
            addr.sin_addr.s_addr = inet_addr("127.0.0.1");
            addr.sin_port = htons(SERVICE_PORT);
            
            if (connect(svc_fd, (struct sockaddr *)&addr, sizeof(addr)) == 0) {
                send(svc_fd, plaintext, dec_len, 0);
                
                char svc_resp[256];
                ssize_t svc_n = recv(svc_fd, svc_resp, sizeof(svc_resp), 0);
                if (svc_n > 0) {
                    // Encrypt response using a fresh IV
                    uint8_t res_iv[AES_IV_SIZE];
                    RAND_bytes(res_iv, AES_IV_SIZE);
                    
                    uint8_t enc_resp[512];
                    int enc_resp_len = aes_encrypt((uint8_t*)svc_resp, svc_n, g_secret_key, res_iv, enc_resp);
                    
                    // Format response packet
                    MuxHeader res_header;
                    res_header.connection_id = header.connection_id;
                    res_header.payload_len = enc_resp_len;
                    memcpy(res_header.iv, res_iv, AES_IV_SIZE);
                    
                    // Send back to Peer A
                    ssize_t w1 = write(g_mcp_pipe_b_to_a[1], &res_header, sizeof(res_header));
                    ssize_t w2 = write(g_mcp_pipe_b_to_a[1], enc_resp, enc_resp_len);
                    (void)w1;
                    (void)w2;
                }
            }
            close(svc_fd);
        }
    }
    return NULL;
}

// Peer A Demuxer Thread: reads from MCP, decrypts response, routes to correct client FD
void *peer_a_demuxer_thread(void *arg) {
    WorkerContext *ctx = (WorkerContext *)arg;
    uint8_t recv_packet[1024];
    
    while (ctx->is_running) {
        struct timeval tv = { .tv_sec = 0, .tv_usec = 10000 };
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(g_mcp_pipe_b_to_a[0], &rfds);
        
        int r = select(g_mcp_pipe_b_to_a[0] + 1, &rfds, NULL, NULL, &tv);
        if (r > 0) {
            MuxHeader header;
            ssize_t n = read(g_mcp_pipe_b_to_a[0], &header, sizeof(header));
            if (n <= 0) break;
            
            ssize_t rd_len = read(g_mcp_pipe_b_to_a[0], recv_packet, header.payload_len);
            (void)rd_len;
            
            // Decrypt response payload
            uint8_t plaintext[512];
            int dec_len = aes_decrypt(recv_packet, header.payload_len, g_secret_key, header.iv, plaintext);
            plaintext[dec_len] = '\0';
            
            // Send back to corresponding client FD
            pthread_mutex_lock(&g_clients_mutex);
            int client_fd = -1;
            if (header.connection_id < MAX_CLIENTS) {
                client_fd = g_client_fds[header.connection_id];
                g_client_fds[header.connection_id] = -1; // Reset
            }
            pthread_mutex_unlock(&g_clients_mutex);
            
            if (client_fd != -1) {
                send(client_fd, plaintext, dec_len, 0);
                close(client_fd);
            }
        }
    }
    return NULL;
}

// Client Acceptor Loop on Peer A
void *peer_a_proxy_listener(void *arg) {
    WorkerContext *ctx = (WorkerContext *)arg;
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(PROXY_PORT);
    
    bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr));
    listen(listen_fd, 10);
    
    uint32_t conn_counter = 0;
    
    while (ctx->is_running) {
        struct timeval tv = { .tv_sec = 0, .tv_usec = 10000 };
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(listen_fd, &rfds);
        
        int r = select(listen_fd + 1, &rfds, NULL, NULL, &tv);
        if (r > 0) {
            int client_fd = accept(listen_fd, NULL, NULL);
            char buf[256];
            ssize_t n = recv(client_fd, buf, sizeof(buf), 0);
            if (n > 0) {
                pthread_mutex_lock(&g_clients_mutex);
                uint32_t conn_id = conn_counter % MAX_CLIENTS;
                g_client_fds[conn_id] = client_fd;
                conn_counter++;
                pthread_mutex_unlock(&g_clients_mutex);
                
                // Encrypt payload with random IV
                uint8_t req_iv[AES_IV_SIZE];
                RAND_bytes(req_iv, AES_IV_SIZE);
                
                uint8_t enc_req[512];
                int enc_len = aes_encrypt((uint8_t*)buf, n, g_secret_key, req_iv, enc_req);
                
                // Format MUX header
                MuxHeader header;
                header.connection_id = conn_id;
                header.payload_len = enc_len;
                memcpy(header.iv, req_iv, AES_IV_SIZE);
                
                // Write out over MCP channel
                ssize_t w1 = write(g_mcp_pipe_a_to_b[1], &header, sizeof(header));
                ssize_t w2 = write(g_mcp_pipe_a_to_b[1], enc_req, enc_len);
                (void)w1;
                (void)w2;
            } else {
                close(client_fd);
            }
        }
    }
    close(listen_fd);
    return NULL;
}

// Client helper thread to simulate concurrent client connections
typedef struct {
    char req[32];
    char resp_expected[64];
    bool success;
} ClientJob;

void *client_simulator_thread(void *arg) {
    ClientJob *job = (ClientJob *)arg;
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(PROXY_PORT);
    
    if (connect(client_fd, (struct sockaddr *)&addr, sizeof(addr)) == 0) {
        send(client_fd, job->req, strlen(job->req), 0);
        
        char buf[512] = {0};
        ssize_t n = recv(client_fd, buf, sizeof(buf) - 1, 0);
        if (n > 0) {
            buf[n] = '\0';
            if (strcmp(buf, job->resp_expected) == 0) {
                job->success = true;
            }
        }
    }
    close(client_fd);
    return NULL;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT ZMM VM: ENCRYPTED MULTIPLEXED PEER PROXY TESTS\n");
    printf("=============================================================\n");

    // Initialize cryptographic algorithms
    OpenSSL_add_all_algorithms();
    RAND_bytes(g_secret_key, AES_KEY_SIZE);

    // Initialize MCP channels
    if (pipe(g_mcp_pipe_b_to_a) != 0 || pipe(g_mcp_pipe_a_to_b) != 0) {
        perror("Pipe initialization failed");
        return 1;
    }

    memset(g_client_fds, -1, sizeof(g_client_fds));

    WorkerContext svc_ctx = { .is_running = true };
    WorkerContext b_mcp_ctx = { .is_running = true };
    WorkerContext a_demux_ctx = { .is_running = true };
    WorkerContext a_proxy_ctx = { .is_running = true };

    // Spin up background service, demuxers, and listeners
    pthread_create(&svc_ctx.thread, NULL, echo_service_thread, &svc_ctx);
    pthread_create(&b_mcp_ctx.thread, NULL, peer_b_mcp_handler, &b_mcp_ctx);
    pthread_create(&a_demux_ctx.thread, NULL, peer_a_demuxer_thread, &a_demux_ctx);
    pthread_create(&a_proxy_ctx.thread, NULL, peer_a_proxy_listener, &a_proxy_ctx);

    usleep(50000); // Allow thread startup

    // Setup concurrent clients jobs
    ClientJob job1 = { .req = "CLIENT_ONE", .resp_expected = "[ECHO: CLIENT_ONE]", .success = false };
    ClientJob job2 = { .req = "CLIENT_TWO", .resp_expected = "[ECHO: CLIENT_TWO]", .success = false };

    pthread_t thread1, thread2;
    printf("1. Launching two concurrent TCP clients targeting Peer A local proxy port %d...\n", PROXY_PORT);
    pthread_create(&thread1, NULL, client_simulator_thread, &job1);
    pthread_create(&thread2, NULL, client_simulator_thread, &job2);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    printf("2. Verifying responses from the multiplexed encrypted pipeline...\n");
    printf("   -> Client 1 Response Verified: %s\n", job1.success ? "YES" : "NO");
    printf("   -> Client 2 Response Verified: %s\n", job2.success ? "YES" : "NO");

    assert(job1.success);
    assert(job2.success);
    printf("   ✓ Both client streams multiplexed and decrypted correctly over the single MCP channel!\n\n");

    // Clean up
    svc_ctx.is_running = false;
    b_mcp_ctx.is_running = false;
    a_demux_ctx.is_running = false;
    a_proxy_ctx.is_running = false;

    close(g_mcp_pipe_b_to_a[0]);
    close(g_mcp_pipe_b_to_a[1]);
    close(g_mcp_pipe_a_to_b[0]);
    close(g_mcp_pipe_a_to_b[1]);

    pthread_join(svc_ctx.thread, NULL);
    pthread_join(b_mcp_ctx.thread, NULL);
    pthread_join(a_demux_ctx.thread, NULL);
    pthread_join(a_proxy_ctx.thread, NULL);

    printf("=============================================================\n");
    printf("AUNCIENT MULTIPLEXED ENCRYPTED PEER PROXY TESTS PASSED\n");
    printf("=============================================================\n");
    return 0;
}
