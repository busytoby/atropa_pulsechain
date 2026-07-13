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
    YUL_OP_SYNTH_PLAY_BIO = 0x66
} YulOpcode;

// PPN Account Structure
typedef struct {
    uint16_t project;
    uint16_t programmer;
} PPN;

// Transaction packet carrying the path to the .bio arrangement file
typedef struct {
    PPN      ppn;
    uint32_t key_id;
    char     bio_file_path[128];
    uint32_t opcode;
} TcpBioTxPacket;

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
    return 0.0;
}

// Parses and plays the submitted .bio score on the synthesizer
bool play_bio_arrangement(const char *file_path, PPN ppn, const char **out_err) {
    FILE *f = fopen(file_path, "r");
    if (!f) {
        *out_err = "REVERT: FAILED_TO_OPEN_BIO_ASSET";
        return false;
    }

    char line[256];
    char notes_parsed[32][8];
    int parsed_count = 0;
    int parser_state = 0; // 0: patterns, 1: intro_riff, 2: lead, 3: sequence, 4: parse

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
                break;
            }
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

    printf("   [Yul RTS] Parsed %d lead steps from submitted arrangement.\n", parsed_count);

    double step_duration = 60.0 / 109.6 / 8.0; 
    uint32_t slot = get_ppn_slot(ppn);

    // Charge gas for executing the score (15 gas tokens per note block)
    uint64_t total_gas_cost = parsed_count * UNIVERSAL_GAS_FEE;
    if (g_balances[slot] < total_gas_cost) {
        *out_err = "REVERT: INSUFFICIENT_GAS_FOR_SCORE_PLAYBACK";
        return false;
    }
    g_balances[slot] -= total_gas_cost;
    printf("   [Yul RTS] Charged %lu Gas. Remaining: %lu Gas\n", total_gas_cost, g_balances[slot]);

    // Play the parsed sequence
    for (int i = 0; i < parsed_count; i++) {
        double freq = note_to_frequency(notes_parsed[i]);
        if (freq > 0.0) {
            printf("   [Speaker] Note %d: %.2f Hz\n", i + 1, freq);
        }
        alsa_synth_set_frequency(&g_synth_engine, freq);
        usleep((useconds_t)(step_duration * 1000000.0));
    }
    alsa_synth_set_frequency(&g_synth_engine, 0.0);
    return true;
}

// BTC Script 2-3 Operator execution engine
bool process_transaction_rts(TcpBioTxPacket *tx, const char **out_err) {
    // Enforce Key ACL check (Key 11 is Admin PPN [1,2])
    if (tx->key_id != 11 || tx->ppn.project != 1 || tx->ppn.programmer != 2) {
        *out_err = "REVERT: ACL_PERMISSION_DENIED";
        return false;
    }
    
    // Route to .bio parser/player
    if (tx->opcode == YUL_OP_SYNTH_PLAY_BIO) {
        printf("   [Yul RTS] BTC Script verified BIO transaction submission for: %s\n", tx->bio_file_path);
        return play_bio_arrangement(tx->bio_file_path, tx->ppn, out_err);
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
    
    TcpBioTxPacket packet;
    while (recv(client_fd, &packet, sizeof(TcpBioTxPacket), 0) > 0) {
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
    printf("AUNCIENT ZMM VM: MCP BIO ARRANGEMENT TRANSACTION SUBMISSION\n");
    printf("=============================================================\n");

    // Initialize ALSASynth
    if (!alsa_synth_init(&g_synth_engine, 0.0)) {
        fprintf(stderr, "Failed to initialize ALSA Synthesizer.\n");
        return 1;
    }

    // Initialize balances
    memset(g_balances, 0, sizeof(g_balances));
    PPN admin_ppn = { .project = 1, .programmer = 2 };
    g_balances[get_ppn_slot(admin_ppn)] = 1000; // Seed 1000 gas tokens

    // 1. Spawn TCP/IP Server
    pthread_t server_tid;
    pthread_create(&server_tid, NULL, mcp_server_thread, NULL);
    usleep(100000);

    // 2. Connect client and submit .bio transaction over TCP/IP
    printf("2. Client connecting to submit BIO arrangement transaction...\n");
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

    // Package the file path directly in the transaction packet
    TcpBioTxPacket packet = {
        .ppn = admin_ppn,
        .key_id = 11,
        .bio_file_path = "assets/bionika/eye_of_the_tiger.bio",
        .opcode = YUL_OP_SYNTH_PLAY_BIO
    };

    printf("3. Transmitting BIO transaction package over TCP/IP...\n");
    send(client_fd, &packet, sizeof(TcpBioTxPacket), 0);
    
    // Wait for playback loop on server-side to finish
    usleep(3000000); 

    close(client_fd);
    pthread_join(server_tid, NULL);

    alsa_synth_stop(&g_synth_engine);

    printf("\nBIO arrangement play completed successfully.\n");
    printf("=============================================================\n");
    return 0;
}
