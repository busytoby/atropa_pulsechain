#include "tsfi_zmm_vm.h"
#include "tsfi_zmm_rpc.h"
#include "lau_memory.h"
#include "tsfi_wire_firmware.h"
#include "lau_yul_thunk.h"
#include "tsfi_types.h"
#include "lau_thunk.h"
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
        while (**p && **p != ' ' && **p != '\t' && **p != '\n' && **p != ',' && i < max - 1) {
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
        else if (*p == 'Y' || *p == 'y') {
            p++;
            if (match_kw(&p, "ULINIT")) {
                char name[256] = {0};
                char path[512] = {0};
                parse_string_arg(&p, name, sizeof(name));
                skip_comma(&p);
                parse_string_arg(&p, path, sizeof(path));
                skip_comma(&p);
                char *end;
                uint64_t virt_addr = strtoull(p, &end, 10);
                p = end;
                
                lau_yul_thunk_init(name, path, virt_addr);
            }
            else if (match_kw(&p, "ULEXEC")) {
                char name[256] = {0};
                char calldata_hex[8192] = {0};
                parse_string_arg(&p, name, sizeof(name));
                skip_comma(&p);
                parse_string_arg(&p, calldata_hex, sizeof(calldata_hex));
                
                size_t cd_len = 0;
                uint8_t cd_bytes[4096];
                size_t len = strlen(calldata_hex);
                if (len > 8192) len = 8192;
                for (size_t i = 0; i < len; i += 2) {
                    unsigned int byteval = 0;
                    sscanf(&calldata_hex[i], "%02x", &byteval);
                    cd_bytes[i/2] = (uint8_t)byteval;
                    cd_len++;
                }
                
                uint8_t retval[4096];
                size_t retval_len = sizeof(retval);
                lau_yul_thunk_execute(name, cd_bytes, cd_len, retval, &retval_len);
                
                for (size_t i = 0; i < retval_len && state->output_pos < 4090; i++) {
                    sprintf(&state->output_buffer[state->output_pos], "%02x", retval[i]);
                    state->output_pos += 2;
                }
            }
        }
        skip_line(&p);
    }
}

// --- Instruction Pipelining (Fused Block Execution) ---

size_t tsfi_zmm_vm_compile_block_buffer(const char *code, void *buffer, size_t max_len) {
    uint32_t *b = (uint32_t*)buffer;
    uint32_t *start = b;
    uint32_t *limit = b + (max_len / 4);
    const char *p = code;
    
    while (*p) {
        while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') p++;
        if (!*p) break;
        
        if (b + 12 >= limit) break;
        
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
        else if (*p == 'Y' || *p == 'y') {
            p++;
            if (match_kw(&p, "ULINIT")) {
                char name[256] = {0};
                char path[512] = {0};
                parse_string_arg(&p, name, sizeof(name));
                skip_comma(&p);
                parse_string_arg(&p, path, sizeof(path));
                skip_comma(&p);
                char *end;
                uint64_t virt_addr = strtoull(p, &end, 10);
                p = end;

                char *p_name = strdup(name);
                char *p_path = strdup(path);
                uint32_t name_hi = (uint32_t)((uint64_t)p_name >> 32);
                uint32_t name_lo = (uint32_t)((uint64_t)p_name & 0xFFFFFFFF);
                uint32_t path_hi = (uint32_t)((uint64_t)p_path >> 32);
                uint32_t path_lo = (uint32_t)((uint64_t)p_path & 0xFFFFFFFF);
                uint32_t addr_hi = (uint32_t)(virt_addr >> 32);
                uint32_t addr_lo = (uint32_t)(virt_addr & 0xFFFFFFFF);

                *b++ = VM_OP_YUL_INIT;
                *b++ = name_hi; *b++ = name_lo;
                *b++ = path_hi; *b++ = path_lo;
                *b++ = addr_hi; *b++ = addr_lo;
            }
            else if (match_kw(&p, "ULEXEC")) {
                char name[256] = {0};
                char calldata_hex[8192] = {0};
                parse_string_arg(&p, name, sizeof(name));
                skip_comma(&p);
                parse_string_arg(&p, calldata_hex, sizeof(calldata_hex));

                char *p_name = strdup(name);
                size_t len = strlen(calldata_hex);
                if (len > 8192) len = 8192;
                uint8_t *cd_bytes = malloc(len / 2 + 1);
                size_t cd_len = 0;
                for (size_t i = 0; i < len; i += 2) {
                    unsigned int byteval = 0;
                    sscanf(&calldata_hex[i], "%02x", &byteval);
                    cd_bytes[i/2] = (uint8_t)byteval;
                    cd_len++;
                }

                uint8_t *retval = malloc(65536);
                size_t *retval_len = malloc(sizeof(size_t));
                *retval_len = 65536;

                uint32_t name_hi = (uint32_t)((uint64_t)p_name >> 32);
                uint32_t name_lo = (uint32_t)((uint64_t)p_name & 0xFFFFFFFF);
                uint32_t cd_hi = (uint32_t)((uint64_t)cd_bytes >> 32);
                uint32_t cd_lo = (uint32_t)((uint64_t)cd_bytes & 0xFFFFFFFF);
                uint32_t ret_hi = (uint32_t)((uint64_t)retval >> 32);
                uint32_t ret_lo = (uint32_t)((uint64_t)retval & 0xFFFFFFFF);
                uint32_t ret_len_hi = (uint32_t)((uint64_t)retval_len >> 32);
                uint32_t ret_len_lo = (uint32_t)((uint64_t)retval_len & 0xFFFFFFFF);

                *b++ = VM_OP_YUL_EXEC;
                *b++ = name_hi; *b++ = name_lo;
                *b++ = cd_hi; *b++ = cd_lo;
                *b++ = (uint32_t)cd_len;
                *b++ = ret_hi; *b++ = ret_lo;
                *b++ = ret_len_hi; *b++ = ret_len_lo;
            }
        }
        skip_line(&p);
    }
    if (b < limit) *b++ = ZMM_OP_END;
    return (size_t)((char*)b - (char*)start);
}

void* tsfi_zmm_vm_compile_block(const char *code) {
    size_t len = strlen(code);
    size_t max_size = len * 8 + 2048;
    void *buffer = lau_malloc(max_size);
    if (buffer) {
        tsfi_zmm_vm_compile_block_buffer(code, buffer, max_size);
    }
    return buffer;
}

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
                    fw->cell_wave_exec(1, d, s1, s2);
                break;
            }
            case ZMM_OP_WMUL: {
                int d = *b++; int s1 = *b++; int s2 = *b++;
                if (d < TSFI_ZMM_REG_COUNT)
                    fw->cell_wave_exec(2, d, s1, s2);
                break;
            }
            case ZMM_OP_WSTORE: {
                (void)*b++; (void)*b++;
                state->output_pos++;
                break;
            }
            case VM_OP_SEAL: {
                uint32_t hi = *b++; uint32_t lo = *b++;
                void *ptr = (void*)(((uint64_t)hi << 32) | lo);
                extern void lau_seal_object_loc(void*, const char*, int);
                lau_seal_object_loc(ptr, "tsfi_zmm_vm", 0);
                break;
            }
            case VM_OP_UNSEAL: {
                uint32_t hi = *b++; uint32_t lo = *b++;
                void *ptr = (void*)(((uint64_t)hi << 32) | lo);
                extern void lau_unseal_object_loc(void*, const char*, int);
                lau_unseal_object_loc(ptr, "tsfi_zmm_vm", 0);
                break;
            }
            case VM_OP_INVOKE: {
                uint32_t hi = *b++; uint32_t lo = *b++; uint32_t offset = *b++;
                void *ptr = (void*)(((uint64_t)hi << 32) | lo);
                extern LauMetadata* lau_registry_find(void*);
                LauMetadata *md = lau_registry_find(ptr);
                if (md) {
                    LauWiredHeader *h = (LauWiredHeader*)((char*)md->payload_start - 8192);
                    if (h->schema) {
                        for (int i=0; i < h->schema_count; i++) {
                            if (h->schema[i].offset == offset) {
                                void **target_member = (void**)((char*)ptr + offset);
                                if (*target_member) {
                                    void (*thunk_call)(void*) = (void(*)(void*))*target_member;
                                    thunk_call(ptr);
                                }
                                break;
                            }
                        }
                    }
                }
                break;
            }
            case VM_OP_YUL_INIT: {
                uint32_t name_hi = *b++; uint32_t name_lo = *b++;
                uint32_t path_hi = *b++; uint32_t path_lo = *b++;
                uint32_t addr_hi = *b++; uint32_t addr_lo = *b++;
                const char *name = (const char*)(((uint64_t)name_hi << 32) | name_lo);
                const char *path = (const char*)(((uint64_t)path_hi << 32) | path_lo);
                uint64_t virt_addr = ((uint64_t)addr_hi << 32) | addr_lo;
                lau_yul_thunk_init(name, path, virt_addr);
                break;
            }
            case VM_OP_YUL_EXEC: {
                uint32_t name_hi = *b++; uint32_t name_lo = *b++;
                uint32_t cd_hi = *b++; uint32_t cd_lo = *b++;
                uint32_t cd_size = *b++;
                uint32_t ret_hi = *b++; uint32_t ret_lo = *b++;
                uint32_t ret_len_hi = *b++; uint32_t ret_len_lo = *b++;
                
                const char *name = (const char*)(((uint64_t)name_hi << 32) | name_lo);
                const uint8_t *calldata = (const uint8_t*)(((uint64_t)cd_hi << 32) | cd_lo);
                uint8_t *retval = (uint8_t*)(((uint64_t)ret_hi << 32) | ret_lo);
                size_t *retval_len = (size_t*)(((uint64_t)ret_len_hi << 32) | ret_len_lo);
                
                lau_yul_thunk_execute(name, calldata, cd_size, retval, retval_len);
                break;
            }
            default: return;
        }
    }
}

void tsfi_zmm_vm_free_block(void *block) {
    if (block) {
        uint32_t *b = (uint32_t*)block;
        while (1) {
            uint32_t op = *b++;
            if (op == ZMM_OP_END) break;
            switch (op) {
                case ZMM_OP_WLOAD:
                    b += 2;
                    break;
                case ZMM_OP_WADD:
                case ZMM_OP_WMUL:
                    b += 3;
                    break;
                case ZMM_OP_WSTORE:
                    b += 2;
                    break;
                case VM_OP_SEAL:
                case VM_OP_UNSEAL:
                    b += 2;
                    break;
                case VM_OP_INVOKE:
                    b += 3;
                    break;
                case VM_OP_YUL_INIT: {
                    uint32_t name_hi = *b++; uint32_t name_lo = *b++;
                    uint32_t path_hi = *b++; uint32_t path_lo = *b++;
                    b += 2;
                    free((void*)(((uint64_t)name_hi << 32) | name_lo));
                    free((void*)(((uint64_t)path_hi << 32) | path_lo));
                    break;
                }
                case VM_OP_YUL_EXEC: {
                    uint32_t name_hi = *b++; uint32_t name_lo = *b++;
                    uint32_t cd_hi = *b++; uint32_t cd_lo = *b++;
                    b++;
                    uint32_t ret_hi = *b++; uint32_t ret_lo = *b++;
                    uint32_t ret_len_hi = *b++; uint32_t ret_len_lo = *b++;
                    free((void*)(((uint64_t)name_hi << 32) | name_lo));
                    free((void*)(((uint64_t)cd_hi << 32) | cd_lo));
                    free((void*)(((uint64_t)ret_hi << 32) | ret_lo));
                    free((void*)(((uint64_t)ret_len_hi << 32) | ret_len_lo));
                    break;
                }
                default:
                    goto done;
            }
        }
done:
        lau_free(block);
    }
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
        case VM_OP_SEAL:    return 3;
        case VM_OP_UNSEAL:  return 3;
        case VM_OP_INVOKE:  return 4;
        case VM_OP_YUL_INIT: return 7;
        case VM_OP_YUL_EXEC: return 10;
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