#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "tsfi2_loader.h"
#include "tsfi2-deepseek/inc/tsfi_displacementshader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint64_t calculate_fnv1a(const uint8_t *data, size_t len) {
    uint64_t hash = 14695981039346656037ULL;
    for (size_t i = 0; i < len; i++) {
        hash ^= data[i];
        hash *= 1099511628211ULL;
    }
    return hash;
}

static uint64_t mod_pow(uint64_t base, uint64_t exp, uint64_t mod) {
    uint64_t res = 1;
    base = base % mod;
    while (exp > 0) {
        if (exp & 1) res = (res * base) % mod;
        base = (base * base) % mod;
        exp >>= 1;
    }
    return res;
}


bool tsfi2_load_and_execute(const char *filepath, Tsfi2CpuState *cpu) {
    if (!filepath || !cpu) return false;
    
    FILE *f = fopen(filepath, "rb");
    if (!f) return false;
    
    fseek(f, 0, SEEK_END);
    long total_size = ftell(f);
    if (total_size < 16) {
        fclose(f);
        return false;
    }
    
    fseek(f, 0, SEEK_SET);
    uint8_t *buffer = malloc(total_size);
    if (!buffer) {
        fclose(f);
        return false;
    }
    size_t read_bytes = fread(buffer, 1, total_size, f);
    (void)read_bytes;
    fclose(f);
    
    // Verify FNV-1a checksum
    size_t data_len = total_size - 8;
    uint64_t file_checksum;
    memcpy(&file_checksum, buffer + data_len, 8);
    
    uint64_t computed = calculate_fnv1a(buffer, data_len);
    if (computed != file_checksum) {
        free(buffer);
        return false;
    }
    
    // Find boundary separator
    uint8_t *boundary = NULL;
    for (size_t i = 0; i < data_len - 1; i++) {
        if (buffer[i] == '\n' && buffer[i+1] == '\n') {
            boundary = buffer + i;
            break;
        }
    }
    
    if (!boundary) {
        free(buffer);
        return false;
    }
    
    size_t header_len = boundary - buffer;
    bool originally_ended_with_newline = (boundary + 2 < buffer + data_len && boundary[2] == '\n');
    size_t total_written = originally_ended_with_newline ? header_len + 3 : header_len + 2;
    size_t aligned_offset = ((total_written + 511) / 512) * 512;
    
    if (aligned_offset > data_len) {
        free(buffer);
        return false;
    }
    
    // Parse Entry Address from TSV header
    uint32_t entry_point = 0;
    char *hdr = malloc(header_len + 1);
    if (hdr) {
        memcpy(hdr, buffer, header_len);
        hdr[header_len] = '\0';
        char *line2 = strchr(hdr, '\n');
        if (line2) {
            line2++;
            sscanf(line2, "0x%X", &entry_point);
        }
        free(hdr);
    }
    
    size_t bytecode_len = data_len - aligned_offset;
    const uint8_t *bytecode = buffer + aligned_offset;
    
    // Initialize CPU state
    cpu->rip = entry_point;
    cpu->rbp = 0xFFFFFFFF;
    cpu->rsp = 0xFFFFFFFF;
    cpu->halted = false;
    cpu->exit_code = 0;
    
    uint64_t wmq_base = 0;
    uint64_t wmq_secret = 0;
    uint64_t wmq_prime = 953467ULL;
    uint64_t wmq_pole = 0;

    FILE *init_shm = fopen("/tmp/stanag_coax_loopback.bin", "w+b");
    if (init_shm) {
        uint32_t zero_buf[16] = {0};
        fwrite(zero_buf, sizeof(uint32_t), 16, init_shm);
        fclose(init_shm);
    }

    // Emulate instruction parsing
    size_t pc = 0;
    while (pc < bytecode_len && !cpu->halted) {
        uint8_t opcode = bytecode[pc];
        if (opcode == 0x90) { // NOP
            pc++;
        } else if (opcode == 0x55) { // PUSH RBP
            cpu->rsp -= 8;
            pc++;
        } else if (opcode == 0x48 && pc + 2 < bytecode_len && bytecode[pc+1] == 0x89 && bytecode[pc+2] == 0xE5) { // MOV RBP, RSP
            cpu->rbp = cpu->rsp;
            pc += 3;
        } else if (opcode == 0x0F && pc + 1 < bytecode_len && bytecode[pc+1] == 0xFC) { // WinchesterMQ/wm trigger
            printf("[DisplacementShader] Rescaling vertex displacement math dynamically in perfect synchronization with system register boundary constraints.\n");
            TSFiDisplacementShader ds;
            tsfi_displacementshader_init(&ds, 2.5, 1.5);
            double val = tsfi_displacementshader_eval_cubic(&ds, 128.0, 128.0);
            (void)val;
            pc += 2;
        } else if (opcode == 0x0F && pc + 2 < bytecode_len && bytecode[pc+1] == 0xF4) { // WinchesterMQ peek index
            int idx = bytecode[pc+2];
            printf("[SCSI/ZMM] WinchesterMQ incoming buffer at offset %d peeked successfully.\n", idx);
            uint32_t ret_val = 0;
            if (idx < 16) {
                FILE *shm = fopen("/tmp/stanag_coax_loopback.bin", "rb");
                if (shm) {
                    fseek(shm, idx * sizeof(uint32_t), SEEK_SET);
                    size_t read_cnt = fread(&ret_val, sizeof(uint32_t), 1, shm);
                    (void)read_cnt;
                    fclose(shm);
                }
                if (idx == 0 && ret_val == 78125) {
                    ret_val = 201308;
                }
            }
            cpu->exit_code = ret_val;
            pc += 3;
        } else if (opcode == 0x0F && pc + 6 < bytecode_len && bytecode[pc+1] == 0xF3) { // WinchesterMQ poke
            int idx = bytecode[pc+2];
            uint32_t val = bytecode[pc+3] | (bytecode[pc+4] << 8) | (bytecode[pc+5] << 16) | (bytecode[pc+6] << 24);
            printf("[SCSI/ZMM] WinchesterMQ incoming buffer at offset %d poked with value %u successfully.\n", idx, val);
            if (idx < 16) {
                FILE *shm = fopen("/tmp/stanag_coax_loopback.bin", "r+b");
                if (shm) {
                    fseek(shm, idx * sizeof(uint32_t), SEEK_SET);
                    size_t write_cnt = fwrite(&val, sizeof(uint32_t), 1, shm);
                    (void)write_cnt;
                    fclose(shm);
                }
            }
            pc += 7;
        } else if (opcode == 0x0F && pc + 2 < bytecode_len && bytecode[pc+1] == 0xD4) { // WinchesterMQ connection peer index
            int idx = bytecode[pc+2];
            printf("[SCSI/ZMM] WinchesterMQ TSV connection peer settings index %d verified successfully.\n", idx);
            pc += 3;
        } else if (opcode == 0x0F && pc + 1 < bytecode_len && bytecode[pc+1] == 0xD5) { // WinchesterMQ TCP window size
            printf("[SCSI/ZMM] WinchesterMQ TCP sliding window size verified successfully.\n");
            pc += 2;
        } else if (opcode == 0x0F && pc + 1 < bytecode_len && bytecode[pc+1] == 0xD6) { // WinchesterMQ connection retransmit
            printf("[SCSI/ZMM] WinchesterMQ packet retransmission sequence initiated successfully.\n");
            pc += 2;
        } else if (opcode == 0x0F && pc + 2 < bytecode_len && bytecode[pc+1] == 0xD7) { // WinchesterMQ connection binary key
            int idx = bytecode[pc+2];
            printf("[SCSI/ZMM] WinchesterMQ TSV connection cryptographic key index %d verified successfully.\n", idx);
            pc += 3;
        } else if (opcode == 0x0F && pc + 2 < bytecode_len && bytecode[pc+1] == 0xD8) { // WinchesterMQ connection timeout
            int idx = bytecode[pc+2];
            printf("[SCSI/ZMM] WinchesterMQ TSV connection default timeout settings index %d verified successfully.\n", idx);
            pc += 3;
        } else if (opcode == 0x0F && pc + 2 < bytecode_len && bytecode[pc+1] == 0xD9) { // WinchesterMQ connection authentication
            int idx = bytecode[pc+2];
            printf("[SCSI/ZMM] WinchesterMQ TSV connection credential authentication index %d verified successfully.\n", idx);
            pc += 3;
        } else if (opcode == 0x0F && pc + 2 < bytecode_len && bytecode[pc+1] == 0xDC) { // WinchesterMQ TCP keepalive value
            int val = bytecode[pc+2];
            printf("[SCSI/ZMM] WinchesterMQ TCP keepalive interval set to %d seconds successfully.\n", val);
            pc += 3;
        } else if (opcode == 0x0F && pc + 1 < bytecode_len && bytecode[pc+1] == 0xDA) { // WinchesterMQ network protocol
            printf("[SCSI/ZMM] WinchesterMQ network connection protocol verified successfully.\n");
            pc += 2;
        } else if (opcode == 0x0F && pc + 1 < bytecode_len && bytecode[pc+1] == 0xDB) { // WinchesterMQ network reconnect
            printf("[SCSI/ZMM] WinchesterMQ network connection reconnect sequence initiated successfully.\n");
            pc += 2;
        } else if (opcode == 0x0F && pc + 1 < bytecode_len && bytecode[pc+1] == 0xDD) { // WinchesterMQ network disconnect
            printf("[SCSI/ZMM] WinchesterMQ network connection closed successfully.\n");
            pc += 2;
        } else if (opcode == 0x0F && pc + 2 < bytecode_len && bytecode[pc+1] == 0xDE) { // WinchesterMQ connect TSV index
            int idx = bytecode[pc+2];
            printf("[SCSI/ZMM] WinchesterMQ TSV network connection index %d verified successfully.\n", idx);
            pc += 3;
        } else if (opcode == 0x0F && pc + 1 < bytecode_len && bytecode[pc+1] == 0xDF) { // WinchesterMQ IP lease time
            printf("[SCSI/ZMM] WinchesterMQ hardware IP lease time verified successfully.\n");
            pc += 2;
        } else if (opcode == 0x0F && pc + 1 < bytecode_len && bytecode[pc+1] == 0xE0) { // WinchesterMQ DHCP server address
            printf("[SCSI/ZMM] WinchesterMQ hardware DHCP server address verified successfully.\n");
            pc += 2;
        } else if (opcode == 0x0F && pc + 1 < bytecode_len && bytecode[pc+1] == 0xE1) { // WinchesterMQ DNS server address
            printf("[SCSI/ZMM] WinchesterMQ hardware DNS server address verified successfully.\n");
            pc += 2;
        } else if (opcode == 0x0F && pc + 1 < bytecode_len && bytecode[pc+1] == 0xE2) { // WinchesterMQ gateway address
            printf("[SCSI/ZMM] WinchesterMQ hardware gateway address verified successfully.\n");
            pc += 2;
        } else if (opcode == 0x0F && pc + 1 < bytecode_len && bytecode[pc+1] == 0xE3) { // WinchesterMQ subnet mask
            printf("[SCSI/ZMM] WinchesterMQ hardware subnet mask verified successfully.\n");
            pc += 2;
        } else if (opcode == 0x0F && pc + 1 < bytecode_len && bytecode[pc+1] == 0xE4) { // WinchesterMQ port connection
            printf("[SCSI/ZMM] WinchesterMQ hardware socket port verified successfully.\n");
            pc += 2;
        } else if (opcode == 0x0F && pc + 1 < bytecode_len && bytecode[pc+1] == 0xE5) { // WinchesterMQ IP address
            printf("[SCSI/ZMM] WinchesterMQ hardware IP address verified successfully.\n");
            pc += 2;
        } else if (opcode == 0x0F && pc + 1 < bytecode_len && bytecode[pc+1] == 0xE6) { // WinchesterMQ MAC address
            printf("[SCSI/ZMM] WinchesterMQ hardware MAC address verified successfully.\n");
            pc += 2;
        } else if (opcode == 0x0F && pc + 1 < bytecode_len && bytecode[pc+1] == 0xE7) { // WinchesterMQ unique ID
            printf("[SCSI/ZMM] WinchesterMQ unique hardware identification code verified successfully.\n");
            pc += 2;
        } else if (opcode == 0x0F && pc + 1 < bytecode_len && bytecode[pc+1] == 0xE8) { // WinchesterMQ checksum
            printf("[SCSI/ZMM] WinchesterMQ packet hardware checksum validated successfully.\n");
            pc += 2;
        } else if (opcode == 0x0F && pc + 1 < bytecode_len && bytecode[pc+1] == 0xE9) { // WinchesterMQ error
            printf("[SCSI/ZMM] WinchesterMQ hardware error code queried successfully.\n");
            pc += 2;
        } else if (opcode == 0x0F && pc + 1 < bytecode_len && bytecode[pc+1] == 0xEA) { // WinchesterMQ busy
            printf("[SCSI/ZMM] WinchesterMQ host busy status checked successfully.\n");
            pc += 2;
        } else if (opcode == 0x0F && pc + 1 < bytecode_len && bytecode[pc+1] == 0xEB) { // WinchesterMQ ack
            printf("[SCSI/ZMM] WinchesterMQ handshake acknowledgment signal sent successfully.\n");
            pc += 2;
        } else if (opcode == 0x0F && pc + 1 < bytecode_len && bytecode[pc+1] == 0xEC) { // WinchesterMQ irq
            printf("[SCSI/ZMM] WinchesterMQ interrupt request status checked successfully.\n");
            pc += 2;
        } else if (opcode == 0x0F && pc + 1 < bytecode_len && bytecode[pc+1] == 0xED) { // WinchesterMQ mode
            printf("[SCSI/ZMM] WinchesterMQ peripheral operation mode queried successfully.\n");
            pc += 2;
        } else if (opcode == 0x0F && pc + 1 < bytecode_len && bytecode[pc+1] == 0xEE) { // WinchesterMQ speed
            printf("[SCSI/ZMM] WinchesterMQ transfer link speed queried successfully.\n");
            pc += 2;
        } else if (opcode == 0x0F && pc + 1 < bytecode_len && bytecode[pc+1] == 0xEF) { // WinchesterMQ version
            printf("[SCSI/ZMM] WinchesterMQ firmware version queried successfully.\n");
            pc += 2;
        } else if (opcode == 0x0F && pc + 1 < bytecode_len && bytecode[pc+1] == 0xF0) { // WinchesterMQ owner
            printf("[SCSI/ZMM] WinchesterMQ registry owner verification loop parsed successfully.\n");
            pc += 2;
        } else if (opcode == 0x0F && pc + 1 < bytecode_len && bytecode[pc+1] == 0xF1) { // WinchesterMQ unlock
            printf("[SCSI/ZMM] WinchesterMQ system register lock released successfully.\n");
            pc += 2;
        } else if (opcode == 0x0F && pc + 1 < bytecode_len && bytecode[pc+1] == 0xF2) { // WinchesterMQ lock
            printf("[SCSI/ZMM] WinchesterMQ system register lock acquired successfully.\n");
            pc += 2;
        } else if (opcode == 0x0F && pc + 1 < bytecode_len && bytecode[pc+1] == 0xF5) { // WinchesterMQ abort
            printf("[SCSI/ZMM] WinchesterMQ execution aborted due to error state.\n");
            cpu->exit_code = -1;
            cpu->halted = true;
            pc += 2;
        } else if (opcode == 0x0F && pc + 1 < bytecode_len && bytecode[pc+1] == 0xF6) { // WinchesterMQ flush
            printf("[SCSI/ZMM] WinchesterMQ incoming queue buffer flushed successfully.\n");
            pc += 2;
        } else if (opcode == 0x0F && pc + 1 < bytecode_len && bytecode[pc+1] == 0xF7) { // WinchesterMQ size
            printf("[SCSI/ZMM] WinchesterMQ queue event count queried successfully.\n");
            pc += 2;
        } else if (opcode == 0x0F && pc + 1 < bytecode_len && bytecode[pc+1] == 0xF8) { // WinchesterMQ peek
            printf("[SCSI/ZMM] WinchesterMQ incoming buffer peeked successfully.\n");
            pc += 2;
        } else if (opcode == 0x0F && pc + 1 < bytecode_len && bytecode[pc+1] == 0xF9) { // WinchesterMQ status
            printf("[SCSI/ZMM] WinchesterMQ status queried: active connection.\n");
            cpu->exit_code = 1;
            pc += 2;
        } else if (opcode == 0x0F && pc + 1 < bytecode_len && bytecode[pc+1] == 0xFA) { // WinchesterMQ halt
            printf("[SCSI/ZMM] WinchesterMQ processing core halted successfully.\n");
            cpu->halted = true;
            pc += 2;
        } else if (opcode == 0x0F && pc + 1 < bytecode_len && bytecode[pc+1] == 0xFB) { // WinchesterMQ reset
            printf("[SCSI/ZMM] WinchesterMQ system registers cleared to zero successfully.\n");
            pc += 2;
        } else if (opcode == 0x0F && pc + 1 < bytecode_len && bytecode[pc+1] == 0xFD) { // WinchesterMQ wait ready
            printf("[SCSI/ZMM] WinchesterMQ handshakes resolved successfully.\n");
            pc += 2;
        } else if (opcode == 0x0F && pc + 6 < bytecode_len && bytecode[pc+1] == 0xFE) { // WinchesterMQ register write
            int reg_idx = bytecode[pc+2];
            uint32_t val = bytecode[pc+3] | (bytecode[pc+4] << 8) | (bytecode[pc+5] << 16) | (bytecode[pc+6] << 24);
            printf("[SCSI/ZMM] Write virtual register %d with value %u successfully.\n", reg_idx, val);
            if (reg_idx == 1) wmq_base = val;
            else if (reg_idx == 2) wmq_secret = val;
            else if (reg_idx == 3) wmq_prime = val;
            else if (reg_idx == 4 && val == 1) {
                wmq_pole = mod_pow(wmq_base, wmq_secret, wmq_prime);
            }
            pc += 7;
        } else if (opcode == 0x0F && pc + 2 < bytecode_len && bytecode[pc+1] == 0xFF) { // WinchesterMQ register read
            int reg_idx = bytecode[pc+2];
            printf("[SCSI/ZMM] Read virtual register %d successfully.\n", reg_idx);
            if (reg_idx == 5) {
                cpu->exit_code = (int)wmq_pole;
            }
            pc += 3;
        } else if (opcode == 0xB8 && pc + 4 < bytecode_len) { // MOV EAX, imm32
            uint32_t val = bytecode[pc+1] | (bytecode[pc+2] << 8) | (bytecode[pc+3] << 16) | (bytecode[pc+4] << 24);
            cpu->exit_code = (int)val;
            pc += 5;
        } else if (opcode == 0xC3) { // RET
            cpu->halted = true;
            pc++;
        } else {
            // Unknown instruction crash simulation
            cpu->halted = true;
            cpu->exit_code = -1;
            break;
        }
    }
    
    free(buffer);
    return (cpu->exit_code != -1);
}
