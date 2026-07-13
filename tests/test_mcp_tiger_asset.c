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

// 2-Channel Network Transaction Packet
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

// Map musical note strings to frequencies
double note_to_frequency(const char *note) {
    if (strcmp(note, "C4") == 0) return 261.63;
    if (strcmp(note, "Bb3") == 0) return 233.08;
    if (strcmp(note, "Ab3") == 0) return 207.65;
    if (strcmp(note, "C3") == 0) return 130.81;
    if (strcmp(note, "Eb3") == 0) return 155.56;
    if (strcmp(note, "G3") == 0) return 196.00;
    return 0.0; // REST or unknown
}

// BTC Script 2-3 Operator execution engine
bool process_transaction_rts(TcpTxPacket *tx, const char **out_err) {
    uint32_t slot = get_ppn_slot(tx->ppn);
    uint64_t balance = g_balances[slot];
    
    if (balance < UNIVERSAL_GAS_FEE) {
        *out_err = "REVERT: INSUFFICIENT_GAS_BALANCE";
        return false;
    }
    
    g_balances[slot] = balance - UNIVERSAL_GAS_FEE;
    
    if (tx->opcode == YUL_OP_SYNTH_PLAY) {
        printf("   [Yul RTS] BTC Script verified note. Freq: %.2f Hz | Balance: %lu Gas\n",
               tx->frequency, g_balances[slot]);
               
        alsa_synth_set_frequency(&g_synth_engine, tx->frequency);
        usleep((useconds_t)(tx->duration * 1000000.0));
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
    printf("AUNCIENT ZMM VM: MCP ASSET SUBMISSION - EYE OF THE TIGER\n");
    printf("=============================================================\n");

    // Initialize ALSASynth
    if (!alsa_synth_init(&g_synth_engine, 0.0)) {
        fprintf(stderr, "Failed to initialize ALSA Synthesizer.\n");
        return 1;
    }

    // Initialize user balances
    memset(g_balances, 0, sizeof(g_balances));
    PPN admin_ppn = { .project = 1, .programmer = 2 };
    g_balances[get_ppn_slot(admin_ppn)] = 1000; // Seed 1000 gas tokens

    // 1. Spawn TCP/IP Server
    pthread_t server_tid;
    pthread_create(&server_tid, NULL, mcp_server_thread, NULL);
    usleep(100000);

    // 2. Read and parse eye_of_the_tiger.bio asset file
    printf("2. Loading Eye of the Tiger score asset file...\n");
    FILE *f = fopen("assets/bionika/eye_of_the_tiger.bio", "r");
    if (!f) {
        perror("Failed to open assets/bionika/eye_of_the_tiger.bio");
        alsa_synth_stop(&g_synth_engine);
        return 1;
    }

    // Extract sequence array strings using state machine to skip arrangement
    char line[256];
    char notes_parsed[32][8];
    int parsed_count = 0;
    int parser_state = 0; // 0: find patterns, 1: find intro_riff, 2: find lead, 3: find sequence, 4: parse notes

    while (fgets(line, sizeof(line), f)) {
        if (parser_state == 0 && strstr(line, "\"patterns\"")) {
            parser_state = 1;
        } else if (parser_state == 1 && strstr(line, "\"intro_riff\"")) {
            parser_state = 2;
        } else if (parser_state == 2 && strstr(line, "\"lead\"")) {
            parser_state = 3;
        } else if (parser_state == 3 && strstr(line, "\"sequence\"")) {
            parser_state = 4;
            continue;
        }
        
        if (parser_state == 4) {
            if (strstr(line, "]")) {
                break; // End of sequence
            }
            // Parse all tokens in double quotes on the current line
            char *p = line;
            while ((p = strchr(p, '"')) != NULL) {
                char *end_ptr = strchr(p + 1, '"');
                if (end_ptr && parsed_count < 32) {
                    size_t len = end_ptr - (p + 1);
                    strncpy(notes_parsed[parsed_count], p + 1, len);
                    notes_parsed[parsed_count][len] = '\0';
                    parsed_count++;
                    p = end_ptr + 1;
                } else {
                    break;
                }
            }
        }
    }
    fclose(f);

    printf("   ✓ Successfully parsed %d notes from asset file.\n", parsed_count);

    // 3. Connect client and transmit score over TCP/IP
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

    // Tempo-based duration calculation (109.6 BPM, 16th notes = ~0.0684s duration)
    double step_duration = 60.0 / 109.6 / 8.0; 

    // Play first 16 steps of the lead sequence from the asset
    printf("3. Transmitting lead riff via TCP/IP transactions to MCP...\n");
    for (int i = 0; i < 16; i++) {
        double freq = note_to_frequency(notes_parsed[i]);
        TcpTxPacket packet = {
            .ppn = admin_ppn,
            .key_id = 11,
            .frequency = freq,
            .duration = step_duration,
            .opcode = YUL_OP_SYNTH_PLAY
        };
        send(client_fd, &packet, sizeof(TcpTxPacket), 0);
        usleep((useconds_t)((step_duration + 0.01) * 1000000.0));
    }

    close(client_fd);
    pthread_join(server_tid, NULL);

    alsa_synth_stop(&g_synth_engine);

    printf("\nUnified asset score play completed successfully.\n");
    printf("=============================================================\n");
    return 0;
}
