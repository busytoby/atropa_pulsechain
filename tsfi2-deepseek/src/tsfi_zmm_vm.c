#include "tsfi_zmm_vm.h"
#include "tsfi_zmm_rpc.h"
#include "lau_memory.h"
#include "tsfi_wire_firmware.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <stdarg.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

// --- Opcodes for Pipelined Execution ---
enum {
    ZMM_OP_END = 0,
    ZMM_OP_WLOAD,   // [OP, REG, FLOAT_VAL]
    ZMM_OP_WADD,    // [OP, DEST, SRC1, SRC2]
    ZMM_OP_WMUL,    // [OP, DEST, SRC1, SRC2]
    ZMM_OP_WSTORE,  // [OP, SRC, IDX]
    ZMM_OP_WDUMP    // [OP, REG]
};

// ... Kernels ...
static void kernel_low(void *regs, ZmmSynapse *syn) {
    (void)regs;
    syn->mass_density += 0.1f;
    if (syn->mass_density > 10.0f) syn->request_kernel_swap = 1;
}

static void kernel_high(void *regs, ZmmSynapse *syn) {
    (void)regs;
    syn->mass_density -= 0.5f;
    if (syn->mass_density < 0.0f) syn->request_kernel_swap = -1;
}

void tsfi_zmm_vm_init(TsfiZmmVmState *state) {
    memset(state, 0, sizeof(TsfiZmmVmState));
    state->output_pos = 0;
    state->telem = NULL;
    
    state->manifest = (TsfiZmmManifest*)lau_memalign_wired(512, sizeof(TsfiZmmManifest));
    if (state->manifest) {
        memset(state->manifest, 0, sizeof(TsfiZmmManifest));
        state->manifest->kernel_low_density = kernel_low;
        state->manifest->kernel_high_density = kernel_high;
        state->manifest->micro_kernel = kernel_low;
        state->manifest->contiguous_rf = lau_memalign_wired(64, 2048);
        state->manifest->active_mask = 0; 
    }
}

void tsfi_zmm_vm_destroy(TsfiZmmVmState *state) {
    if (state->manifest) {
        if (state->manifest->contiguous_rf) lau_free(state->manifest->contiguous_rf);
        lau_free(state->manifest);
        state->manifest = NULL;
    }
    if (state->telem && state->telem != MAP_FAILED) {
        munmap(state->telem, sizeof(LauTelemetryState));
        state->telem = NULL;
    }
    
    // Automatically dissolve global telemetry registry manifold strictly inside the VM execution seal
    extern void lau_registry_teardown(void);
    lau_registry_teardown();
}

void tsfi_zmm_vm_attach_telemetry(TsfiZmmVmState *state, const char *id) {
    char shm_name[256];
    snprintf(shm_name, sizeof(shm_name), "/tsfi_telem_%s", id);
    
    int fd = shm_open(shm_name, O_RDWR, 0666);
    if (fd != -1) {
        state->telem = mmap(NULL, sizeof(LauTelemetryState), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (state->telem == MAP_FAILED) {
            state->telem = NULL;
        }
        close(fd);
    }
}

// --- High Performance Lexer ---

static inline void skip_ws(const char **p) {
    while (**p == ' ' || **p == '\t') (*p)++;
}

static inline void skip_line(const char **p) {
    while (**p && **p != '\n') (*p)++;
}

static inline int parse_reg_fast(const char **p) {
    skip_ws(p);
    if (**p == 'W' || **p == 'w') {
        (*p)++;
        int v = 0;
        if (!isdigit(**p)) return -1;
        while (isdigit(**p)) {
            v = v * 10 + (**p - '0');
            (*p)++;
        }
        return v;
    }
    return -1;
}

static inline void skip_comma(const char **p) {
    skip_ws(p);
    if (**p == ',') (*p)++;
}

static inline int match_kw(const char **p, const char *kw) {
    size_t len = strlen(kw);
    if (strncasecmp(*p, kw, len) == 0) {
        (*p) += len;
        return 1;
    }
    return 0;
}

static inline void parse_string_arg(const char **p, char *out, size_t max) {
    skip_ws(p);
    if (**p == '"') {
        (*p)++;
        size_t i = 0;
        while (**p && **p != '"' && **p != '\n' && i < max - 1) {
            out[i++] = *(*p)++;
        }
        out[i] = 0;
        if (**p == '"') (*p)++;
    } else {
        size_t i = 0;
        while (**p && **p != ' ' && **p != '\t' && **p != '\n' && i < max - 1) {
            out[i++] = *(*p)++;
        }
        out[i] = 0;
    }
}

void tsfi_zmm_vm_exec(TsfiZmmVmState *state, const char *code_in) {
    const char *p = code_in;
    LauWireFirmware *fw = tsfi_wire_firmware_get();

    while (*p) {
        while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') p++;
        if (!*p) break;
        
        if (*p == 'W' || *p == 'w') {
            p++;
            char c = *p;
            
            if (c == 'L' || c == 'l') { // WLOAD
                p++;
                if (match_kw(&p, "OAD")) {
                    int r = parse_reg_fast(&p);
                    skip_comma(&p);
                    char *end;
                    float val = strtof(p, &end);
                    p = end;
                    if (r >= 0 && r < TSFI_ZMM_REG_COUNT) {
                        wave512 w = wave512_set1(val);
                        tsfi_wire_firmware_load_waveform(fw, r, &w);
                    }
                }
            }
            else if (c == 'A' || c == 'a') { // WADD
                p++;
                if (match_kw(&p, "DD")) {
                    int d = parse_reg_fast(&p);
                    skip_comma(&p);
                    int s1 = parse_reg_fast(&p);
                    skip_comma(&p);
                    int s2 = parse_reg_fast(&p);
                    if (d>=0 && s1>=0 && s2>=0 && d < TSFI_ZMM_REG_COUNT && s1 < TSFI_ZMM_REG_COUNT && s2 < TSFI_ZMM_REG_COUNT) {
                        fw->cell_wave_exec(1, d, s1, s2);
                    }
                }
            }
            else if (c == 'M' || c == 'm') { // WMUL
                p++;
                if (match_kw(&p, "UL")) {
                    int d = parse_reg_fast(&p);
                    skip_comma(&p);
                    int s1 = parse_reg_fast(&p);
                    skip_comma(&p);
                    int s2 = parse_reg_fast(&p);
                    if (d>=0 && s1>=0 && s2>=0 && d < TSFI_ZMM_REG_COUNT && s1 < TSFI_ZMM_REG_COUNT && s2 < TSFI_ZMM_REG_COUNT) {
                        fw->cell_wave_exec(2, d, s1, s2);
                    }
                }
            }
            else if (c == 'S' || c == 's') { // WSTORE...
                p++;
                if (match_kw(&p, "TORE")) {
                    int r = parse_reg_fast(&p);
                    skip_comma(&p);
                    if (isdigit(*p)) strtol(p, (char**)&p, 10);
                    
                    if (r >= 0 && r < TSFI_ZMM_REG_COUNT) {
                        state->output_pos++; 
                    }
                }
            }
        }
        skip_line(&p);
    }
}

// --- Instruction Pipelining (Fused Block Execution) ---

// Compile to user-provided buffer (No malloc)
size_t tsfi_zmm_vm_compile_block_buffer(const char *code, void *buffer, size_t max_len) {
    uint32_t *b = (uint32_t*)buffer;
    uint32_t *start = b;
    uint32_t *limit = b + (max_len / 4);
    const char *p = code;
    
    while (*p) {
        while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') p++;
        if (!*p) break;
        
        if (b + 4 >= limit) break; // Overflow check
        
        if (*p == 'W' || *p == 'w') {
            p++;
            char c = *p;
            if (c == 'L' || c == 'l') { // WLOAD
                p++;
                if (match_kw(&p, "OAD")) {
                    int r = parse_reg_fast(&p);
                    skip_comma(&p);
                    char *end;
                    float val = strtof(p, &end);
                    p = end;
                    *b++ = ZMM_OP_WLOAD;
                    *b++ = r;
                    *(float*)b = val; b++;
                }
            } else if (c == 'A' || c == 'a') { // WADD
                p++;
                if (match_kw(&p, "DD")) {
                    int d = parse_reg_fast(&p); skip_comma(&p);
                    int s1 = parse_reg_fast(&p); skip_comma(&p);
                    int s2 = parse_reg_fast(&p);
                    *b++ = ZMM_OP_WADD; *b++ = d; *b++ = s1; *b++ = s2;
                }
            } else if (c == 'M' || c == 'm') { // WMUL
                p++;
                if (match_kw(&p, "UL")) {
                    int d = parse_reg_fast(&p); skip_comma(&p);
                    int s1 = parse_reg_fast(&p); skip_comma(&p);
                    int s2 = parse_reg_fast(&p);
                    *b++ = ZMM_OP_WMUL; *b++ = d; *b++ = s1; *b++ = s2;
                }
            } else if (c == 'S' || c == 's') { // WSTORE
                p++;
                if (match_kw(&p, "TORE")) {
                    int r = parse_reg_fast(&p); skip_comma(&p);
                    if (isdigit(*p)) strtol(p, (char**)&p, 10);
                    int idx = 0;
                    *b++ = ZMM_OP_WSTORE; *b++ = r; *b++ = idx;
                }
            }
        }
        skip_line(&p);
    }
    if (b < limit) *b++ = ZMM_OP_END;
    return (size_t)((char*)b - (char*)start);
}

// Wrapper for allocation
void* tsfi_zmm_vm_compile_block(const char *code) {
    size_t len = strlen(code);
    size_t max_size = len * 4 + 1024;
    void *buffer = lau_malloc(max_size);
    if (buffer) {
        tsfi_zmm_vm_compile_block_buffer(code, buffer, max_size);
    }
    return buffer;
}

#include "tsfi_wire_firmware.h"

// Execute a compiled block (Hardware Path)
void tsfi_zmm_vm_exec_block(TsfiZmmVmState *state, void *block) {
    if (!block) return;
    uint32_t *b = (uint32_t*)block;
    LauWireFirmware *fw = tsfi_wire_firmware_get();
    
    while (1) {
        uint32_t op = *b++;
        switch (op) {
            case ZMM_OP_END: return;
            case ZMM_OP_WLOAD: {
                int r = *b++;
                float val = *(float*)b; b++;
                if (r < TSFI_ZMM_REG_COUNT) {
                    wave512 w = wave512_set1(val);
                    tsfi_wire_firmware_load_waveform(fw, r, &w);
                }
                break;
            }
            case ZMM_OP_WADD: {
                int d = *b++; int s1 = *b++; int s2 = *b++;
                if (d < TSFI_ZMM_REG_COUNT) 
                    fw->cell_wave_exec(1, d, s1, s2); // 1 = VADDPS
                break;
            }
            case ZMM_OP_WMUL: {
                int d = *b++; int s1 = *b++; int s2 = *b++;
                if (d < TSFI_ZMM_REG_COUNT)
                    fw->cell_wave_exec(2, d, s1, s2); // 2 = VMULPS
                break;
            }
            case ZMM_OP_WSTORE: {
                (void)*b++; (void)*b++; // Skip r and idx
                // In hardware path, the result is already in the firmware WRF.
                state->output_pos++;
                break;
            }
            default: return;
        }
    }
}

void tsfi_zmm_vm_free_block(void *block) {
    if (block) lau_free(block);
}

// --- Binary Evolution Support ---

static int get_op_size(uint32_t op) {
    switch (op) {
        case ZMM_OP_END:    return 1;
        case ZMM_OP_WLOAD:  return 3;
        case ZMM_OP_WADD:   return 4;
        case ZMM_OP_WMUL:   return 4;
        case ZMM_OP_WSTORE: return 3;
        case ZMM_OP_WDUMP:  return 2;
        default: return 1;
    }
}

size_t tsfi_zmm_vm_splice_block(const void *blockA, const void *blockB, void *child_buf, size_t max_len) {
    if (!blockA || !blockB || !child_buf) return 0;
    
    uint32_t *ptrA = (uint32_t*)blockA;
    uint32_t *ptrB = (uint32_t*)blockB;
    uint32_t *out = (uint32_t*)child_buf;
    uint32_t *limit = out + (max_len / 4) - 1; 
    
    int countA = 0;
    uint32_t *scan = ptrA;
    while (*scan != ZMM_OP_END) { countA++; scan += get_op_size(*scan); }
    
    int split_idx = countA / 2;
    if (split_idx == 0 && countA > 0) split_idx = 1;
    
    int current = 0;
    scan = ptrA;
    while (*scan != ZMM_OP_END && current < split_idx) {
        int sz = get_op_size(*scan);
        if (out + sz >= limit) break;
        memcpy(out, scan, sz * 4);
        out += sz;
        scan += sz;
        current++;
    }
    
    int countB = 0;
    uint32_t *scanB = ptrB;
    while (*scanB != ZMM_OP_END) { countB++; scanB += get_op_size(*scanB); }
    
    int splitB = countB / 2;
    current = 0;
    scanB = ptrB;
    while (*scanB != ZMM_OP_END && current < splitB) {
        scanB += get_op_size(*scanB);
        current++;
    }
    while (*scanB != ZMM_OP_END) {
        int sz = get_op_size(*scanB);
        if (out + sz >= limit) break;
        memcpy(out, scanB, sz * 4);
        out += sz;
        scanB += sz;
    }
    
    *out = ZMM_OP_END;
    return (size_t)((char*)out - (char*)child_buf);
}