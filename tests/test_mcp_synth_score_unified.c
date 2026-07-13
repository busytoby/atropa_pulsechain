#define _POSIX_C_SOURCE 199309L
#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "../firmware/tsfi_lib/tsfi_alsa_synth.c"

// Universal Opcode
typedef enum {
    YUL_OP_SYNTH_PLAY = 0x65
} YulOpcode;

// PPN Account Structure
typedef struct {
    uint16_t project;
    uint16_t programmer;
} PPN;

// 2-Channel Network Transaction Packet transmitted over TCP/IP
typedef struct {
    PPN      ppn;
    uint32_t key_id;
    double   frequency;
    double   duration;
    uint32_t opcode;
} TcpTxPacket;

// Global Gas Registry (Merkle 2-3 Tree representation)
uint64_t g_balances[100];
const uint64_t UNIVERSAL_GAS_FEE = 15;
ALSASynth g_synth_engine;

uint32_t get_ppn_slot(PPN ppn) {
    return (ppn.project + ppn.programmer) % 100;
}

// BTC Script 2-3 Operator execution engine
bool process_transaction_rts(TcpTxPacket *tx, const char **out_err) {
    uint32_t slot = get_ppn_slot(tx->ppn);
    uint64_t balance = g_balances[slot];
    
    // 1. Enforce Gas Payment Invariant
    if (balance < UNIVERSAL_GAS_FEE) {
        *out_err = "REVERT: INSUFFICIENT_GAS_BALANCE";
        return false;
    }
    
    // Deduct Gas
    g_balances[slot] = balance - UNIVERSAL_GAS_FEE;
    
    // 2. Enforce Key ACL check (Key 11 is Admin PPN [1,2])
    if (tx->key_id != 11 || tx->ppn.project != 1 || tx->ppn.programmer != 2) {
        *out_err = "REVERT: ACL_PERMISSION_DENIED";
        return false;
    }
    
    // 3. Route to pre-existing ALSA Synthesizer Engine
    if (tx->opcode == YUL_OP_SYNTH_PLAY) {
        printf("   [Yul RTS] BTC Script verified note. Freq: %.2f Hz | Balance: %lu Gas\n",
               tx->frequency, g_balances[slot]);
               
        // Set the synthesizer's dynamic frequency
        alsa_synth_set_frequency(&g_synth_engine, tx->frequency);
        
        // Wait for note duration
        usleep((useconds_t)(tx->duration * 1000000.0));
        
        // Return to silent / rest note state
        alsa_synth_set_frequency(&g_synth_engine, 0.0);
        return true;
    }
    
    *out_err = "REVERT: INVALID_OPCODE";
    return false;
}

// TCP/IP MCP Server loop
void* mcp_server_thread(void *arg) {
    (void)arg;
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(18080);
    
    bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    listen(server_fd, 3);
    
    printf("   [MCP Server] Listening on port 18080 over TCP/IP...\n");
    
    int client_fd = accept(server_fd, NULL, NULL);
    
    TcpTxPacket packet;
    while (recv(client_fd, &packet, sizeof(TcpTxPacket), 0) > 0) {
        const char *err = NULL;
        if (!process_transaction_rts(&packet, &err)) {
            printf("   [MCP Server] Transaction failed: %s\n", err);
        }
    }
    
    close(client_fd);
    close(server_fd);
    return NULL;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT ZMM VM: UNIFIED TCP/IP MCP SYNTH SCORE TESTS\n");
    printf("=============================================================\n");

    // Initialize our pre-existing ALSA Synthesizer Engine
    printf("1. Initializing ALSA Synthesizer Engine...\n");
    if (!alsa_synth_init(&g_synth_engine, 0.0)) {
        fprintf(stderr, "Failed to initialize ALSA Synthesizer.\n");
        return 1;
    }

    // Initialize user balances
    memset(g_balances, 0, sizeof(g_balances));
    PPN admin_ppn = { .project = 1, .programmer = 2 };
    g_balances[get_ppn_slot(admin_ppn)] = 300; // Seed 300 gas tokens

    // 1. Spawn MCP Server Thread (TCP/IP Listener)
    pthread_t server_tid;
    pthread_create(&server_tid, NULL, mcp_server_thread, NULL);
    usleep(100000); // Wait for socket bind

    // 2. Connect client and transmit score over TCP/IP
    printf("2. Client connecting over TCP/IP to submit arpeggio score...\n");
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(18080);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
    
    if (connect(client_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection to MCP Server failed");
        alsa_synth_stop(&g_synth_engine);
        return 1;
    }

    double score[4] = { 440.0, 523.25, 659.25, 783.99 }; // Am7 score
    for (int i = 0; i < 4; i++) {
        TcpTxPacket packet = {
            .ppn = admin_ppn,
            .key_id = 11,
            .frequency = score[i],
            .duration = 0.4,
            .opcode = YUL_OP_SYNTH_PLAY
        };
        send(client_fd, &packet, sizeof(TcpTxPacket), 0);
        usleep(450000); // Space notes out slightly
    }

    close(client_fd);
    pthread_join(server_tid, NULL);

    // Stop our pre-existing ALSA Synthesizer Engine
    alsa_synth_stop(&g_synth_engine);

    printf("\nUnified execution completed successfully.\n");
    printf("=============================================================\n");
    return 0;
}
