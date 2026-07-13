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
#include <pulse/simple.h>
#include <pulse/error.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

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
    double   duration; // Custom note duration in seconds
    uint32_t opcode;
} TcpTxPacket;

// Global Gas Registry (Merkle 2-3 Tree representation)
uint64_t g_balances[100];
const uint64_t UNIVERSAL_GAS_FEE = 15;
pa_simple *g_pulse_stream = NULL;

uint32_t get_ppn_slot(PPN ppn) {
    return (ppn.project + ppn.programmer) % 100;
}

// Streams audio frames to PulseAudio
void play_sound_pulse(double freq, double duration) {
    if (!g_pulse_stream || freq <= 0.0) {
        // Rest note handling
        usleep((useconds_t)(duration * 1000000.0));
        return;
    }
    
    uint32_t sample_rate = 44100;
    uint32_t total_samples = (uint32_t)(sample_rate * duration);
    
    int16_t *buffer = malloc(total_samples * sizeof(int16_t));
    if (!buffer) return;
    
    for (uint32_t i = 0; i < total_samples; i++) {
        double t = (double)i / sample_rate;
        double raw_val = sin(2.0 * M_PI * freq * t);
        
        // Anti-pop envelope
        double envelope = 1.0;
        if (i < 800) {
            envelope = (double)i / 800.0;
        } else if (i > total_samples - 800) {
            envelope = (double)(total_samples - i) / 800.0;
        }
        buffer[i] = (int16_t)(raw_val * envelope * 16384.0);
    }
    
    int error;
    pa_simple_write(g_pulse_stream, buffer, total_samples * sizeof(int16_t), &error);
    free(buffer);
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
        printf("   [Yul RTS] BTC Script verified note. Freq: %.2f Hz | Dur: %.2f s | Balance: %lu Gas\n",
               tx->frequency, tx->duration, g_balances[slot]);
        play_sound_pulse(tx->frequency, tx->duration);
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
    printf("AUNCIENT ZMM VM: MCP SUBMISSION - EYE OF THE TIGER RIFF\n");
    printf("=============================================================\n");

    // Initialize PulseAudio
    pa_sample_spec ss;
    ss.format = PA_SAMPLE_S16LE;
    ss.rate = 44100;
    ss.channels = 1;
    int error;
    g_pulse_stream = pa_simple_new(NULL, "ZMM_MCP_Tiger", PA_STREAM_PLAYBACK, NULL, "Synthesizer", &ss, NULL, NULL, &error);
    if (!g_pulse_stream) {
        fprintf(stderr, "Failed to initialize PulseAudio: %s\n", pa_strerror(error));
        return 1;
    }

    // Initialize balances
    memset(g_balances, 0, sizeof(g_balances));
    PPN admin_ppn = { .project = 1, .programmer = 2 };
    g_balances[get_ppn_slot(admin_ppn)] = 500; // Seed 500 gas to support the score sequence

    // 1. Spawn TCP/IP Server
    pthread_t server_tid;
    pthread_create(&server_tid, NULL, mcp_server_thread, NULL);
    usleep(100000);

    // 2. Connect client and transmit score over TCP/IP
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(18080);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
    
    if (connect(client_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection to MCP Server failed");
        return 1;
    }

    // "Eye of the Tiger" opening riff frequencies (C5, Bb4, Ab4)
    double notes[]    = { 523.25, 0.0, 523.25, 466.16, 523.25, 0.0, 523.25, 466.16, 523.25, 0.0, 523.25, 466.16, 415.30, 466.16 };
    double durations[] = { 0.4,    0.1, 0.3,    0.3,    0.5,    0.1, 0.3,    0.3,    0.5,    0.1, 0.3,    0.3,    0.4,    0.8    };
    int total_notes = sizeof(notes) / sizeof(notes[0]);

    printf("Deploying score over TCP/IP to MCP (Total Notes: %d)...\n", total_notes);
    for (int i = 0; i < total_notes; i++) {
        TcpTxPacket packet = {
            .ppn = admin_ppn,
            .key_id = 11,
            .frequency = notes[i],
            .duration = durations[i],
            .opcode = YUL_OP_SYNTH_PLAY
        };
        send(client_fd, &packet, sizeof(TcpTxPacket), 0);
        // Sleep on the client-side to synchronize with the server's real-time playback
        usleep((useconds_t)((durations[i] + 0.02) * 1000000.0));
    }

    close(client_fd);
    pthread_join(server_tid, NULL);

    if (g_pulse_stream) {
        pa_simple_drain(g_pulse_stream, &error);
        pa_simple_free(g_pulse_stream);
    }

    printf("\nUnified execution completed successfully.\n");
    printf("=============================================================\n");
    return 0;
}
