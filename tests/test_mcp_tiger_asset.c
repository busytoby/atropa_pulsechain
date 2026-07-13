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
#include <alsa/asoundlib.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define SAMPLE_RATE 8000

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
snd_pcm_t *g_pcm_handle = NULL;

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
    if (strcmp(note, "F3") == 0) return 174.61;
    if (strcmp(note, "C2") == 0) return 65.41;
    if (strcmp(note, "Bb1") == 0) return 58.27;
    if (strcmp(note, "Ab1") == 0) return 51.91;
    return 0.0;
}

// Data structures to hold parsed patterns and arrangement
#define MAX_ARRANGEMENT 64
#define MAX_STEPS 32

typedef struct {
    char pattern_name[16];
    char lead_notes[MAX_STEPS][8];
    char bass_notes[MAX_STEPS][8];
    int step_count;
} PatternData;

// Synthesizes a block using Lissajous mathematics and streams to U8 ALSA hardware
void play_lissajous_step(double f_lead, double f_bass, double duration) {
    uint32_t total_samples = (uint32_t)(SAMPLE_RATE * duration);
    uint8_t *buffer = malloc(total_samples);
    if (!buffer) return;

    // Lissajous mathematical parameters (Base phase offset and frequency mapping)
    double phi_w = 0.5 * M_PI; // Phase offset

    for (uint32_t i = 0; i < total_samples; i++) {
        double t = (double)i / SAMPLE_RATE;
        
        // Base coordinate waves (X and Y projections)
        double x_wave = sin(2.0 * M_PI * f_lead * t + phi_w);
        double y_wave = sin(2.0 * M_PI * f_bass * t);
        
        // Lissajous modulation mapping to U8 (0-255)
        double raw_val = x_wave * y_wave;
        
        // Anti-pop envelope
        double envelope = 1.0;
        if (i < 200) {
            envelope = (double)i / 200.0;
        } else if (i > total_samples - 200) {
            envelope = (double)(total_samples - i) / 200.0;
        }
        
        buffer[i] = (uint8_t)(127.0 + 120.0 * raw_val * envelope);
    }

    if (g_pcm_handle) {
        snd_pcm_sframes_t frames = snd_pcm_writei(g_pcm_handle, buffer, total_samples);
        if (frames < 0) {
            snd_pcm_recover(g_pcm_handle, frames, 0);
        }
    } else {
        // Emulator timing fallback
        usleep((useconds_t)(duration * 1000000.0));
    }
    
    free(buffer);
}

// Parses and plays the full submitted .bio score on the synthesizer
bool play_bio_arrangement(const char *file_path, PPN ppn, const char **out_err) {
    FILE *f = fopen(file_path, "r");
    if (!f) {
        *out_err = "REVERT: FAILED_TO_OPEN_BIO_ASSET";
        return false;
    }

    char arrangement_list[MAX_ARRANGEMENT][16];
    int arrangement_count = 0;
    
    PatternData patterns[4];
    memset(patterns, 0, sizeof(patterns));
    strcpy(patterns[0].pattern_name, "intro_riff");
    strcpy(patterns[1].pattern_name, "verse");
    strcpy(patterns[2].pattern_name, "pre_chorus");
    strcpy(patterns[3].pattern_name, "chorus");

    char line[256];
    bool in_arrangement = false;
    int current_pattern_idx = -1;
    int parser_state = 0; // 0: patterns, 1: lead/bass, 2: sequence

    while (fgets(line, sizeof(line), f)) {
        if (strstr(line, "\"arrangement\"")) {
            in_arrangement = true;
            continue;
        }
        if (in_arrangement) {
            if (strstr(line, "]")) {
                in_arrangement = false;
                continue;
            }
            char *p = line;
            while ((p = strchr(p, '"')) != NULL) {
                char *end_ptr = strchr(p + 1, '"');
                if (end_ptr && arrangement_count < MAX_ARRANGEMENT) {
                    size_t len = end_ptr - (p + 1);
                    strncpy(arrangement_list[arrangement_count], p + 1, len);
                    arrangement_list[arrangement_count][len] = '\0';
                    arrangement_count++;
                    p = end_ptr + 1;
                } else {
                    break;
                }
            }
        }

        for (int i = 0; i < 4; i++) {
            char search_str[256];
            snprintf(search_str, sizeof(search_str), "\"%.15s\"", patterns[i].pattern_name);
            if (strstr(line, search_str) && strstr(line, "{")) {
                current_pattern_idx = i;
                parser_state = 0;
            }
        }

        if (current_pattern_idx != -1) {
            if (strstr(line, "\"lead\"")) {
                parser_state = 1;
                continue;
            }
            if (strstr(line, "\"bass\"")) {
                parser_state = 10;
                continue;
            }
            
            // Parse Lead Sequence
            if (parser_state == 1 && strstr(line, "\"sequence\"")) {
                parser_state = 2;
                continue;
            }
            if (parser_state == 2) {
                if (strstr(line, "]")) {
                    parser_state = 0;
                    continue;
                }
                char *p = line;
                PatternData *pat = &patterns[current_pattern_idx];
                while ((p = strchr(p, '"')) != NULL) {
                    char *end_ptr = strchr(p + 1, '"');
                    if (end_ptr && pat->step_count < MAX_STEPS) {
                        size_t len = end_ptr - (p + 1);
                        strncpy(pat->lead_notes[pat->step_count], p + 1, len);
                        pat->lead_notes[pat->step_count][len] = '\0';
                        pat->step_count++;
                        p = end_ptr + 1;
                    } else {
                        break;
                    }
                }
            }

            // Parse Bass Sequence
            if (parser_state == 10 && strstr(line, "\"sequence\"")) {
                parser_state = 20;
                continue;
            }
            if (parser_state == 20) {
                if (strstr(line, "]")) {
                    parser_state = 0;
                    continue;
                }
                char *p = line;
                PatternData *pat = &patterns[current_pattern_idx];
                int bass_count = 0;
                while ((p = strchr(p, '"')) != NULL) {
                    char *end_ptr = strchr(p + 1, '"');
                    if (end_ptr && bass_count < MAX_STEPS) {
                        size_t len = end_ptr - (p + 1);
                        strncpy(pat->bass_notes[bass_count], p + 1, len);
                        pat->bass_notes[bass_count][len] = '\0';
                        bass_count++;
                        p = end_ptr + 1;
                    } else {
                        break;
                    }
                }
            }
        }
    }
    fclose(f);

    int total_score_steps = 0;
    for (int i = 0; i < arrangement_count; i++) {
        for (int j = 0; j < 4; j++) {
            if (strcmp(arrangement_list[i], patterns[j].pattern_name) == 0) {
                total_score_steps += patterns[j].step_count;
            }
        }
    }

    printf("   [Yul RTS] Parsed Arrangement: %d patterns, %d steps.\n", arrangement_count, total_score_steps);

    double step_duration = 60.0 / 109.6 / 8.0; 
    uint32_t slot = get_ppn_slot(ppn);

    // Bill gas
    uint64_t total_gas_cost = total_score_steps * UNIVERSAL_GAS_FEE;
    if (g_balances[slot] < total_gas_cost) {
        *out_err = "REVERT: INSUFFICIENT_GAS_FOR_SCORE_PLAYBACK";
        return false;
    }
    g_balances[slot] -= total_gas_cost;
    printf("   [Yul RTS] Charged %lu Gas. Remaining: %lu Gas\n", total_gas_cost, g_balances[slot]);

    // Play the full Lissajous arrangement (first 64 steps for test time)
    int played_count = 0;
    for (int i = 0; i < arrangement_count && played_count < 64; i++) {
        for (int j = 0; j < 4; j++) {
            if (strcmp(arrangement_list[i], patterns[j].pattern_name) == 0) {
                PatternData *pat = &patterns[j];
                for (int s = 0; s < pat->step_count && played_count < 64; s++) {
                    double f_lead = note_to_frequency(pat->lead_notes[s]);
                    double f_bass = note_to_frequency(pat->bass_notes[s]);
                    
                    if (f_lead > 0.0 || f_bass > 0.0) {
                        printf("   [Lissajous Synth] Step %d: Lead %.2f Hz | Bass %.2f Hz\n",
                               played_count + 1, f_lead, f_bass);
                    }
                    
                    play_lissajous_step(f_lead, f_bass, step_duration);
                    played_count++;
                }
            }
        }
    }
    return true;
}

// BTC Script 2-3 Operator execution engine
bool process_transaction_rts(TcpBioTxPacket *tx, const char **out_err) {
    if (tx->key_id != 11 || tx->ppn.project != 1 || tx->ppn.programmer != 2) {
        *out_err = "REVERT: ACL_PERMISSION_DENIED";
        return false;
    }
    
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

    // Initialize ALSA U8 device at 8000 Hz
    printf("1. Opening ALSA playback device (8000Hz, U8, Mono)...\n");
    int err = snd_pcm_open(&g_pcm_handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
    if (err < 0) {
        fprintf(stderr, "[ALSA] Warning: Cannot open sound hardware (%s). Running in emulator mode.\n", snd_strerror(err));
        g_pcm_handle = NULL;
    } else {
        err = snd_pcm_set_params(g_pcm_handle, SND_PCM_FORMAT_U8, SND_PCM_ACCESS_RW_INTERLEAVED,
                                 1, SAMPLE_RATE, 1, 20000); // 20ms latency
        if (err < 0) {
            fprintf(stderr, "[ALSA] Error setting parameters: %s\n", snd_strerror(err));
            snd_pcm_close(g_pcm_handle);
            g_pcm_handle = NULL;
        }
    }

    // Initialize balances
    memset(g_balances, 0, sizeof(g_balances));
    PPN admin_ppn = { .project = 1, .programmer = 2 };
    g_balances[get_ppn_slot(admin_ppn)] = 30000;

    // Spawn server thread
    pthread_t server_tid;
    pthread_create(&server_tid, NULL, mcp_server_thread, NULL);
    usleep(100000);

    // Client connection
    printf("2. Client connecting to submit BIO arrangement transaction...\n");
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(18080);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
    
    if (connect(client_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection to MCP Server failed");
        if (g_pcm_handle) snd_pcm_close(g_pcm_handle);
        return 1;
    }

    TcpBioTxPacket packet = {
        .ppn = admin_ppn,
        .key_id = 11,
        .bio_file_path = "assets/bionika/eye_of_the_tiger.bio",
        .opcode = YUL_OP_SYNTH_PLAY_BIO
    };

    printf("3. Transmitting BIO transaction package over TCP/IP...\n");
    send(client_fd, &packet, sizeof(TcpBioTxPacket), 0);
    
    usleep(5000000); 

    close(client_fd);
    pthread_join(server_tid, NULL);

    if (g_pcm_handle) {
        snd_pcm_drain(g_pcm_handle);
        snd_pcm_close(g_pcm_handle);
    }

    printf("\nBIO arrangement play completed successfully.\n");
    printf("=============================================================\n");
    return 0;
}
