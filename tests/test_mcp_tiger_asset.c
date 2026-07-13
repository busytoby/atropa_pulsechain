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

#define SAMPLE_RATE 8000

// Universal Opcodes
typedef enum {
    YUL_OP_SYNTH_PLAY_BIO = 0x66,
    YUL_OP_MOUNT_INSTRUMENT = 0x71
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
    char     bio_file_path[128];
    char     mount_target[32];
    uint32_t opcode;
} TcpBioTxPacket;

// Global Gas Registry (Merkle 2-3 Tree representation)
uint64_t g_balances[100];
const uint64_t UNIVERSAL_GAS_FEE = 15;
pa_simple *g_pulse_stream = NULL;

// Instrument Mount State Registers
typedef struct {
    bool lead_mounted;
    bool bass_mounted;
    bool growl_mounted;
    bool drums_mounted;
} SynthMounts;

SynthMounts g_mounts = { false, false, false, false };

uint32_t get_ppn_slot(PPN ppn) {
    return (ppn.project + ppn.programmer) % 100;
}

// Map musical note strings to frequencies
double note_to_frequency(const char *note) {
    if (strcmp(note, "C2") == 0) return 65.41;
    if (strcmp(note, "Bb1") == 0) return 58.27;
    if (strcmp(note, "Ab1") == 0) return 51.91;
    if (strcmp(note, "C3") == 0) return 130.81;
    if (strcmp(note, "Eb3") == 0) return 155.56;
    if (strcmp(note, "G3") == 0) return 196.00;
    if (strcmp(note, "F3") == 0) return 174.61;
    if (strcmp(note, "Ab3") == 0) return 207.65;
    if (strcmp(note, "Bb3") == 0) return 233.08;
    if (strcmp(note, "C4") == 0) return 261.63;
    if (strcmp(note, "Eb4") == 0) return 311.13;
    if (strcmp(note, "F4") == 0) return 349.23;
    if (strcmp(note, "G4") == 0) return 392.00;
    if (strcmp(note, "Ab4") == 0) return 415.30;
    if (strcmp(note, "Bb4") == 0) return 466.16;
    if (strcmp(note, "Ab0") == 0) return 25.96;
    if (strcmp(note, "Bb0") == 0) return 29.14;
    if (strcmp(note, "C1") == 0) return 32.70;
    return 0.0;
}

// Data structures to hold parsed patterns and arrangement
#define MAX_ARRANGEMENT 64
#define MAX_STEPS 32

typedef struct {
    char pattern_name[16];
    char lead_notes[MAX_STEPS][8];
    char bass_notes[MAX_STEPS][8];
    char growl_notes[MAX_STEPS][8];
    double growl_gain[MAX_STEPS];
    double growl_mod[MAX_STEPS];
    uint8_t drum_kick[MAX_STEPS];
    uint8_t drum_snare[MAX_STEPS];
    int lead_count;
    int bass_count;
    int growl_count;
    int gain_count;
    int mod_count;
    int kick_count;
    int snare_count;
} PatternData;

// Synthesizes polyphonic multi-voice audio using the exact video-test math
void play_polyphonic_step(double f_lead, double f_bass, double f_growl, double growl_gain, double growl_mod,
                           bool has_kick, bool has_snare, double duration) {
    uint32_t total_samples = (uint32_t)(SAMPLE_RATE * duration);
    uint8_t *buffer = malloc(total_samples);
    if (!buffer) return;

    for (uint32_t i = 0; i < total_samples; i++) {
        double t = (double)i / SAMPLE_RATE;
        double mix = 0.0;

        // 1. Lead Channel (only if mounted)
        if (f_lead > 0.0 && g_mounts.lead_mounted) {
            double lead_saw = 0.15 * (2.0 * (t * f_lead - floor(t * f_lead)) - 1.0);
            double lead_sq = 0.08 * (sin(2.0 * M_PI * f_lead * t) > 0.0 ? 1.0 : -1.0);
            double lead_env = exp(-6.0 * t);
            mix += (lead_saw + lead_sq) * lead_env * 0.5;
        }

        // 2. Bass Channel (only if mounted)
        if (f_bass > 0.0 && g_mounts.bass_mounted) {
            double bass_tri = 0.3 * (2.0 * fabs(2.0 * (t * f_bass - floor(t * f_bass + 0.5))) - 1.0);
            double bass_env = exp(-4.0 * t);
            mix += bass_tri * bass_env * 0.7;
        }

        // 3. Sub-Growl Channel (only if mounted)
        if (f_growl > 0.0 && growl_gain > 0.0 && g_mounts.growl_mounted) {
            double wobble = sin(2.0 * M_PI * growl_mod * t);
            double growl_sig = 0.6 * sin(2.0 * M_PI * f_growl * t + 3.5 * wobble);
            double growl_env = exp(-1.2 * t) * (1.0 - exp(-35.0 * t)) * (1.0 + 0.4 * wobble);
            mix += growl_sig * growl_env * (growl_gain * 2.2) * 0.5;
        }

        // 4. Kick Drum (only if drums mounted)
        if (has_kick && g_mounts.drums_mounted) {
            double kick_freq = 120.0 * exp(-35.0 * t) + 40.0;
            double kick_sig = 0.6 * sin(2.0 * M_PI * kick_freq * t) * exp(-8.0 * t);
            mix += kick_sig * 0.8;
        }

        // 5. Snare Drum (only if drums mounted)
        if (has_snare && g_mounts.drums_mounted) {
            double rand_noise = ((double)rand() / RAND_MAX) * 2.0 - 1.0;
            double snare_noise = rand_noise * 0.15 * exp(-12.0 * t);
            double snare_body = 0.35 * sin(2.0 * M_PI * 180.0 * t) * exp(-18.0 * t);
            mix += (snare_noise + snare_body) * 0.45 * 0.7;
        }

        double val = 127.0 + 120.0 * mix;
        if (val < 0.0) val = 0.0;
        if (val > 255.0) val = 255.0;
        buffer[i] = (uint8_t)val;
    }

    if (g_pulse_stream) {
        int error;
        pa_simple_write(g_pulse_stream, buffer, total_samples, &error);
    } else {
        usleep((useconds_t)(duration * 1000000.0));
    }
    
    free(buffer);
}

// Parses and plays the full submitted .bio score on the synthesizer
bool play_bio_arrangement(const char *file_path, PPN ppn, const char **out_err) {
    if (!g_mounts.lead_mounted && !g_mounts.bass_mounted && !g_mounts.growl_mounted && !g_mounts.drums_mounted) {
        *out_err = "REVERT: NO_INSTRUMENTS_MOUNTED_ON_SYNTHESIZER";
        return false;
    }

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
    int parser_state = 0;

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
            if (strstr(line, "\"lead\"")) { parser_state = 1; continue; }
            if (strstr(line, "\"bass\"")) { parser_state = 10; continue; }
            if (strstr(line, "\"sub_growl\"")) { parser_state = 30; continue; }
            if (strstr(line, "\"drums\"")) { parser_state = 40; continue; }
            
            PatternData *pat = &patterns[current_pattern_idx];

            if (parser_state == 1 && strstr(line, "\"sequence\"")) { parser_state = 2; continue; }
            if (parser_state == 2) {
                if (strstr(line, "]")) { parser_state = 0; continue; }
                char *p = line;
                while ((p = strchr(p, '"')) != NULL) {
                    char *end_ptr = strchr(p + 1, '"');
                    if (end_ptr && pat->lead_count < MAX_STEPS) {
                        size_t len = end_ptr - (p + 1);
                        strncpy(pat->lead_notes[pat->lead_count], p + 1, len);
                        pat->lead_notes[pat->lead_count][len] = '\0';
                        pat->lead_count++;
                        p = end_ptr + 1;
                    } else { break; }
                }
            }

            if (parser_state == 10 && strstr(line, "\"sequence\"")) { parser_state = 20; continue; }
            if (parser_state == 20) {
                if (strstr(line, "]")) { parser_state = 0; continue; }
                char *p = line;
                while ((p = strchr(p, '"')) != NULL) {
                    char *end_ptr = strchr(p + 1, '"');
                    if (end_ptr && pat->bass_count < MAX_STEPS) {
                        size_t len = end_ptr - (p + 1);
                        strncpy(pat->bass_notes[pat->bass_count], p + 1, len);
                        pat->bass_notes[pat->bass_count][len] = '\0';
                        pat->bass_count++;
                        p = end_ptr + 1;
                    } else { break; }
                }
            }

            // Sub-growl parsing (order-independent, persistent indexing)
            if (parser_state == 30) {
                if (strstr(line, "\"sequence\"")) { parser_state = 31; continue; }
                if (strstr(line, "\"modulation_rate\"")) { parser_state = 33; continue; }
                if (strstr(line, "\"gain\"")) { parser_state = 35; continue; }
            }
            if (parser_state == 31) {
                if (strstr(line, "]")) { parser_state = 30; continue; }
                char *p = line;
                while ((p = strchr(p, '"')) != NULL) {
                    char *end_ptr = strchr(p + 1, '"');
                    if (end_ptr && pat->growl_count < MAX_STEPS) {
                        size_t len = end_ptr - (p + 1);
                        strncpy(pat->growl_notes[pat->growl_count], p + 1, len);
                        pat->growl_notes[pat->growl_count][len] = '\0';
                        pat->growl_count++;
                        p = end_ptr + 1;
                    } else { break; }
                }
            }
            if (parser_state == 33) {
                if (strstr(line, "]")) { parser_state = 30; continue; }
                char *p = line;
                while (p && pat->mod_count < MAX_STEPS) {
                    char *val_ptr = strpbrk(p, "0123456789.");
                    if (val_ptr) {
                        pat->growl_mod[pat->mod_count++] = atof(val_ptr);
                        p = strchr(val_ptr, ',');
                        if (p) p++;
                    } else { break; }
                }
            }
            if (parser_state == 35) {
                if (strstr(line, "]")) { parser_state = 30; continue; }
                char *p = line;
                while (p && pat->gain_count < MAX_STEPS) {
                    char *val_ptr = strpbrk(p, "0123456789.");
                    if (val_ptr) {
                        pat->growl_gain[pat->gain_count++] = atof(val_ptr);
                        p = strchr(val_ptr, ',');
                        if (p) p++;
                    } else { break; }
                }
            }

            // Drums parsing (order-independent, persistent indexing)
            if (parser_state == 40) {
                if (strstr(line, "\"kick\"")) { parser_state = 41; continue; }
                if (strstr(line, "\"snare\"")) { parser_state = 43; continue; }
            }
            if (parser_state == 41) {
                if (strstr(line, "]")) { parser_state = 40; continue; }
                char *p = line;
                while (p && pat->kick_count < MAX_STEPS) {
                    char *val_ptr = strpbrk(p, "01");
                    if (val_ptr) {
                        pat->drum_kick[pat->kick_count++] = (uint8_t)atoi(val_ptr);
                        p = val_ptr + 1;
                    } else { break; }
                }
            }
            if (parser_state == 43) {
                if (strstr(line, "]")) { parser_state = 40; continue; }
                char *p = line;
                while (p && pat->snare_count < MAX_STEPS) {
                    char *val_ptr = strpbrk(p, "01");
                    if (val_ptr) {
                        pat->drum_snare[pat->snare_count++] = (uint8_t)atoi(val_ptr);
                        p = val_ptr + 1;
                    } else { break; }
                }
            }
        }
    }
    fclose(f);

    int total_score_steps = 0;
    for (int i = 0; i < arrangement_count; i++) {
        for (int j = 0; j < 4; j++) {
            if (strcmp(arrangement_list[i], patterns[j].pattern_name) == 0) {
                total_score_steps += patterns[j].lead_count;
            }
        }
    }

    printf("   [Yul RTS] Parsed Arrangement: %d patterns, %d steps.\n", arrangement_count, total_score_steps);

    double step_duration = 60.0 / 109.6 / 8.0; 
    uint32_t slot = get_ppn_slot(ppn);

    uint64_t total_gas_cost = total_score_steps * UNIVERSAL_GAS_FEE;
    if (g_balances[slot] < total_gas_cost) {
        *out_err = "REVERT: INSUFFICIENT_GAS_FOR_SCORE_PLAYBACK";
        return false;
    }
    g_balances[slot] -= total_gas_cost;
    printf("   [Yul RTS] Charged %lu Gas. Remaining: %lu Gas\n", total_gas_cost, g_balances[slot]);

    // Play the full modular polyphonic arrangement
    int played_count = 0;
    for (int i = 0; i < arrangement_count; i++) {
        for (int j = 0; j < 4; j++) {
            if (strcmp(arrangement_list[i], patterns[j].pattern_name) == 0) {
                PatternData *pat = &patterns[j];
                for (int s = 0; s < pat->lead_count; s++) {
                    double f_lead = note_to_frequency(pat->lead_notes[s]);
                    double f_bass = note_to_frequency(pat->bass_notes[s]);
                    double f_growl = note_to_frequency(pat->growl_notes[s]);
                    double g_val = pat->growl_gain[s];
                    double m_val = pat->growl_mod[s];
                    bool kick = pat->drum_kick[s] > 0;
                    bool snare = pat->drum_snare[s] > 0;
                    
                    if (played_count % 16 == 0) {
                        printf("   [PolySynth] Playing Step %d/%d (Tempo Sync)...\n", played_count + 1, total_score_steps);
                    }
                    
                    play_polyphonic_step(f_lead, f_bass, f_growl, g_val, m_val, kick, snare, step_duration);
                    played_count++;
                }
            }
        }
    }
    return true;
}

// BTC Script 2-3 Operator execution engine
bool process_transaction_rts(TcpBioTxPacket *tx, const char **out_err) {
    uint32_t slot = get_ppn_slot(tx->ppn);
    
    if (g_balances[slot] < UNIVERSAL_GAS_FEE) {
        *out_err = "REVERT: INSUFFICIENT_GAS";
        return false;
    }
    g_balances[slot] -= UNIVERSAL_GAS_FEE;

    if (tx->key_id != 11 || tx->ppn.project != 1 || tx->ppn.programmer != 2) {
        *out_err = "REVERT: ACL_PERMISSION_DENIED";
        return false;
    }
    
    if (tx->opcode == YUL_OP_MOUNT_INSTRUMENT) {
        printf("   [Mount System] Mount request received for: %s\n", tx->mount_target);
        if (strcmp(tx->mount_target, "lead") == 0) {
            g_mounts.lead_mounted = true;
            printf("   [Mount System] Installed LEAD synthesizer module. Remaining: %lu Gas\n", g_balances[slot]);
            return true;
        } else if (strcmp(tx->mount_target, "bass") == 0) {
            g_mounts.bass_mounted = true;
            printf("   [Mount System] Installed BASS synthesizer module. Remaining: %lu Gas\n", g_balances[slot]);
            return true;
        } else if (strcmp(tx->mount_target, "growl") == 0) {
            g_mounts.growl_mounted = true;
            printf("   [Mount System] Installed GROWL synthesizer module. Remaining: %lu Gas\n", g_balances[slot]);
            return true;
        } else if (strcmp(tx->mount_target, "drums") == 0) {
            g_mounts.drums_mounted = true;
            printf("   [Mount System] Installed DRUMS synthesizer module. Remaining: %lu Gas\n", g_balances[slot]);
            return true;
        }
        *out_err = "REVERT: UNKNOWN_INSTRUMENT_MODULE";
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
    setvbuf(stdout, NULL, _IONBF, 0);
    printf("=============================================================\n");
    printf("AUNCIENT ZMM VM: MCP BIO ARRANGEMENT TRANSACTION SUBMISSION\n");
    printf("=============================================================\n");

    // Initialize PulseAudio Simple API stream (U8 format, 8000 Hz, Mono)
    printf("1. Connecting to PulseAudio server (8000Hz, U8, Mono)...\n");
    pa_sample_spec ss;
    ss.format = PA_SAMPLE_U8;
    ss.rate = SAMPLE_RATE;
    ss.channels = 1;
    int error;
    g_pulse_stream = pa_simple_new(NULL, "ZMM_MCP_Tiger", PA_STREAM_PLAYBACK, NULL, "Synthesizer", &ss, NULL, NULL, &error);
    if (!g_pulse_stream) {
        fprintf(stderr, "Failed to connect to PulseAudio: %s\n", pa_strerror(error));
        return 1;
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
        if (g_pulse_stream) pa_simple_free(g_pulse_stream);
        return 1;
    }

    // 3. Transmit Mount Transactions first (Mount lead, bass, growl, drums)
    const char *instruments[] = { "lead", "bass", "growl", "drums" };
    printf("3. Transmitting Instrument Mount Transactions over TCP/IP...\n");
    for (int i = 0; i < 4; i++) {
        TcpBioTxPacket mount_packet = {
            .ppn = admin_ppn,
            .key_id = 11,
            .opcode = YUL_OP_MOUNT_INSTRUMENT
        };
        strcpy(mount_packet.mount_target, instruments[i]);
        send(client_fd, &mount_packet, sizeof(TcpBioTxPacket), 0);
        usleep(50000); // Space mount transactions out
    }

    // 4. Submit Play Transaction
    TcpBioTxPacket play_packet = {
        .ppn = admin_ppn,
        .key_id = 11,
        .opcode = YUL_OP_SYNTH_PLAY_BIO
    };
    strcpy(play_packet.bio_file_path, "assets/bionika/eye_of_the_tiger.bio");

    printf("4. Transmitting BIO playback transaction package...\n");
    send(client_fd, &play_packet, sizeof(TcpBioTxPacket), 0);
    
    // Wait for the full duration of the song playback thread (~135 seconds) to complete
    usleep(135000000); 

    close(client_fd);
    pthread_join(server_tid, NULL);

    if (g_pulse_stream) {
        pa_simple_drain(g_pulse_stream, &error);
        pa_simple_free(g_pulse_stream);
    }

    printf("\nBIO arrangement play completed successfully.\n");
    printf("=============================================================\n");
    return 0;
}
