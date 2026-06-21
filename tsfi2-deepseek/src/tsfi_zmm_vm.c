#define _GNU_SOURCE
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
#include <dirent.h>
#include <inttypes.h>
#include <time.h>

typedef struct {
    char name[64];
    uint64_t value;
} RuleVariable;

static RuleVariable g_rule_vars[64];
static int g_rule_var_count = 0;

static uint64_t get_rule_var(const char *name) {
    for (int i = 0; i < g_rule_var_count; i++) {
        if (strcasecmp(g_rule_vars[i].name, name) == 0) {
            return g_rule_vars[i].value;
        }
    }
    return 0;
}

static void set_rule_var(const char *name, uint64_t val) {
    for (int i = 0; i < g_rule_var_count; i++) {
        if (strcasecmp(g_rule_vars[i].name, name) == 0) {
            g_rule_vars[i].value = val;
            return;
        }
    }
    if (g_rule_var_count < 64) {
        snprintf(g_rule_vars[g_rule_var_count].name, sizeof(g_rule_vars[g_rule_var_count].name), "%s", name);
        g_rule_vars[g_rule_var_count].value = val;
        g_rule_var_count++;
    }
}

static bool eval_rule_condition(const char *cond) {
    while (*cond == ' ' || *cond == '\t') cond++;
    
    const char *and_ptr = strcasestr(cond, " and ");
    if (and_ptr) {
        char left[256] = {0};
        int left_len = and_ptr - cond;
        if (left_len >= 256) left_len = 255;
        strncpy(left, cond, left_len);
        return eval_rule_condition(left) && eval_rule_condition(and_ptr + 5);
    }
    
    const char *or_ptr = strcasestr(cond, " or ");
    if (or_ptr) {
        char left[256] = {0};
        int left_len = or_ptr - cond;
        if (left_len >= 256) left_len = 255;
        strncpy(left, cond, left_len);
        return eval_rule_condition(left) || eval_rule_condition(or_ptr + 4);
    }
    
    char var_name[64] = {0};
    char op[32] = {0};
    uint64_t val = 0;
    
    const char *p = cond;
    int v_idx = 0;
    while (*p && *p != ' ' && *p != '\t' && v_idx < 63) {
        var_name[v_idx++] = *p++;
    }
    while (*p == ' ' || *p == '\t') p++;
    
    if (strncasecmp(p, "is ", 3) == 0) {
        p += 3;
    }
    while (*p == ' ' || *p == '\t') p++;
    
    if (strncasecmp(p, "equal to", 8) == 0) {
        strcpy(op, "==");
        p += 8;
    } else if (strncasecmp(p, "less than", 9) == 0) {
        strcpy(op, "<");
        p += 9;
    } else if (strncasecmp(p, "greater than", 12) == 0) {
        strcpy(op, ">");
        p += 12;
    } else {
        strcpy(op, "==");
    }
    
    while (*p == ' ' || *p == '\t') p++;
    
    if (*p >= '0' && *p <= '9') {
        val = strtoull(p, NULL, 0);
    } else {
        char ref_var[64] = {0};
        int r_idx = 0;
        while (*p && *p != ' ' && *p != '\t' && *p != '\n' && *p != '\r' && r_idx < 63) {
            ref_var[r_idx++] = *p++;
        }
        val = get_rule_var(ref_var);
    }
    
    uint64_t var_val = get_rule_var(var_name);
    if (strcmp(op, "==") == 0) return var_val == val;
    if (strcmp(op, "<") == 0) return var_val < val;
    if (strcmp(op, ">") == 0) return var_val > val;
    return false;
}

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
    
    struct timespec start, end;
    
    printf("[ZMM VM INIT] Running dual diagnostics: initializing Dysnomia structures...\n");
    clock_gettime(CLOCK_MONOTONIC, &start);
    struct Fa* fa_diag = allocFa();
    if (fa_diag) {
        fa_diag->Base = tsfi_bn_alloc();
        if (fa_diag->Base) {
            tsfi_bn_set_u64(fa_diag->Base, 43456);
            clock_gettime(CLOCK_MONOTONIC, &end);
            double ms = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_nsec - start.tv_nsec) / 1000000.0;
            printf("[ZMM VM INIT] Dysnomia Fa allocation verified: Base=%p (took %.3f ms)\n", (void*)fa_diag->Base, ms);
        }
        freeFa(fa_diag);
    } else {
        printf("[ZMM VM INIT WARNING] Dysnomia Fa allocation failed!\n");
    }

    printf("[ZMM VM INIT] Initializing Yul virtual machine compilers...\n");
    clock_gettime(CLOCK_MONOTONIC, &start);
    lau_yul_thunk_init("graphicsSystem", "../solidity/bin/graphicsSystem.yul", 0x2);
    lau_yul_thunk_init("musicMaker", "../solidity/bin/musicMaker.yul", 0x3);
    lau_yul_thunk_init("diskSystem", "../solidity/bin/diskSystem.yul", 0x4);
    clock_gettime(CLOCK_MONOTONIC, &end);
    double yul_ms = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_nsec - start.tv_nsec) / 1000000.0;
    printf("[ZMM VM INIT] Yul compilers initialization completed (took %.3f ms)\n", yul_ms);
    
    state->manifest = (TsfiZmmManifest*)lau_memalign_wired(512, sizeof(TsfiZmmManifest));
    if (state->manifest) {
        memset(state->manifest, 0, sizeof(TsfiZmmManifest));
        state->manifest->kernel_low_density = kernel_low;
        state->manifest->kernel_high_density = kernel_high;
        state->manifest->micro_kernel = kernel_low;
        state->manifest->contiguous_rf = lau_memalign_wired(64, 2048);
        state->manifest->active_mask = 0; 
    }
    
    // Allocate 512KB Virtual REU
    state->reu_size = 512 * 1024;
    state->reu_ram = (uint8_t*)malloc(state->reu_size);
    if (state->reu_ram) {
        memset(state->reu_ram, 0, state->reu_size);
    }
}

void tsfi_zmm_vm_destroy(TsfiZmmVmState *state) {
    if (state->manifest) {
        // Restore write access to manifest and contiguous_rf headers
        extern uint32_t lau_mprotect(void *ptr, int prot);
        lau_mprotect(state->manifest, PROT_READ | PROT_WRITE);

        extern LauWiredHeader* get_wired_header_external(void *payload);
        LauWiredHeader *wh_manifest = get_wired_header_external(state->manifest);
        if (wh_manifest && wh_manifest->proxy) {
            extern void ThunkProxy_destroy_authoritative(ThunkProxy *p);
            ThunkProxy_destroy_authoritative((ThunkProxy*)wh_manifest->proxy);
            wh_manifest->proxy = NULL;
        }

        if (state->manifest->contiguous_rf) {
            lau_mprotect(state->manifest->contiguous_rf, PROT_READ | PROT_WRITE);
            LauWiredHeader *wh_rf = get_wired_header_external(state->manifest->contiguous_rf);
            if (wh_rf && wh_rf->proxy) {
                extern void ThunkProxy_destroy_authoritative(ThunkProxy *p);
                ThunkProxy_destroy_authoritative((ThunkProxy*)wh_rf->proxy);
                wh_rf->proxy = NULL;
            }
            lau_free(state->manifest->contiguous_rf);
        }
        lau_free(state->manifest);
        state->manifest = NULL;
    }
    if (state->telem && state->telem != MAP_FAILED) {
        munmap(state->telem, sizeof(LauTelemetryState));
        state->telem = NULL;
    }
    if (state->record_file) {
        fclose((FILE*)state->record_file);
        state->record_file = NULL;
    }
    if (state->checkpoints) {
        free(state->checkpoints);
        state->checkpoints = NULL;
    }
    if (state->reu_ram) {
        free(state->reu_ram);
        state->reu_ram = NULL;
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

static inline uint64_t parse_num_arg(const char **p) {
    while (**p == ' ' || **p == '\t') (*p)++;
    uint64_t val = 0;
    if (**p == '$') {
        (*p)++;
        char *end;
        val = strtoull(*p, &end, 16);
        *p = end;
    } else if (**p == '0' && ((*p)[1] == 'x' || (*p)[1] == 'X')) {
        (*p) += 2;
        char *end;
        val = strtoull(*p, &end, 16);
        *p = end;
    } else if (**p == '%') {
        (*p)++;
        while (**p == '0' || **p == '1') {
            val = (val << 1) | (**p - '0');
            (*p)++;
        }
    } else if (isdigit(**p)) {
        char *end;
        val = strtoull(*p, &end, 10);
        *p = end;
    }
    return val;
}

static inline uint64_t parse_value(const char **p) {
    return parse_num_arg(p);
}

static inline uint64_t eval_expr(const char **p) {
    uint64_t val = parse_value(p);
    while (1) {
        while (**p == ' ' || **p == '\t') (*p)++;
        char op = **p;
        if (op == '\0' || op == '"' || op == '\n' || op == '\r' || op == ',') break;
        
        int op_len = 1;
        if (op == '<' && (*p)[1] == '<') op_len = 2;
        else if (op == '>' && (*p)[1] == '>') op_len = 2;
        
        (*p) += op_len;
        uint64_t next_val = parse_value(p);
        if (op == '+') val += next_val;
        else if (op == '-') val -= next_val;
        else if (op == '*') val *= next_val;
        else if (op == '/') val = next_val ? (val / next_val) : 0;
        else if (op == '&') val &= next_val;
        else if (op == '|') val |= next_val;
        else if (op == '^') val ^= next_val;
        else if (op == '<' && op_len == 2) val <<= next_val;
        else if (op == '>' && op_len == 2) val >>= next_val;
    }
    return val;
}

typedef struct {
    uint64_t registers[6]; // A, X, Y, SR, SP, PC
    uint8_t ram[16384];    // 16KB RAM snapshot
    char memo[256];        // Annotation memo
} ZmmCpuCheckpoint;

static void zmm_vm_save_checkpoint(TsfiZmmVmState *state) {
    if (state->checkpoint_count >= state->checkpoint_capacity) {
        state->checkpoint_capacity = state->checkpoint_capacity == 0 ? 8 : state->checkpoint_capacity * 2;
        state->checkpoints = realloc(state->checkpoints, state->checkpoint_capacity * sizeof(ZmmCpuCheckpoint));
    }
    
    ZmmCpuCheckpoint *cp = &((ZmmCpuCheckpoint*)state->checkpoints)[state->checkpoint_count];
    
    // Read registers
    for (int r = 0; r < 6; r++) {
        uint64_t addr = 0x80 + r;
        uint8_t peek_cd[36] = {0};
        peek_cd[0] = 0x78; peek_cd[1] = 0x61; peek_cd[2] = 0xd2; peek_cd[3] = 0x69;
        for (int i = 0; i < 8; i++) {
            peek_cd[4 + 31 - i] = (addr >> (i * 8)) & 0xFF;
        }
        uint8_t peek_ret[32] = {0};
        size_t peek_ret_len = 32;
        lau_yul_thunk_execute("cpu6502", peek_cd, 36, peek_ret, &peek_ret_len);
        
        uint64_t val = 0;
        for (int i = 0; i < 8; i++) {
            val |= ((uint64_t)peek_ret[31 - i]) << (i * 8);
        }
        cp->registers[r] = val;
    }
    
    // Read RAM
    for (uint64_t addr = 0; addr < 16384; addr++) {
        uint8_t peek_cd[36] = {0};
        peek_cd[0] = 0x78; peek_cd[1] = 0x61; peek_cd[2] = 0xd2; peek_cd[3] = 0x69;
        for (int i = 0; i < 8; i++) {
            peek_cd[4 + 31 - i] = (addr >> (i * 8)) & 0xFF;
        }
        uint8_t peek_ret[32] = {0};
        size_t peek_ret_len = 32;
        lau_yul_thunk_execute("cpu6502", peek_cd, 36, peek_ret, &peek_ret_len);
        
        cp->ram[addr] = peek_ret[31];
    }
    
    cp->memo[0] = '\0';
    state->checkpoint_count++;
    printf("[FLIPTRACK] Checkpoint %d saved successfully.\n", state->checkpoint_count - 1);
}

static void zmm_vm_load_checkpoint(TsfiZmmVmState *state, int index) {
    if (!state->checkpoints || index < 0 || index >= state->checkpoint_count) {
        printf("[FLIPTRACK] ERROR: Invalid checkpoint index %d (Total: %d)\n", index, state->checkpoint_count);
        return;
    }
    
    ZmmCpuCheckpoint *cp = &((ZmmCpuCheckpoint*)state->checkpoints)[index];
    
    // Write RAM first
    for (uint64_t addr = 0; addr < 16384; addr++) {
        uint64_t val = cp->ram[addr];
        uint8_t poke_cd[4 + 32 + 32] = {0};
        poke_cd[0] = 0x80; poke_cd[1] = 0x29; poke_cd[2] = 0xe7; poke_cd[3] = 0xc0;
        for (int i = 0; i < 8; i++) {
            poke_cd[4 + 31 - i] = (addr >> (i * 8)) & 0xFF;
            poke_cd[4 + 32 + 31 - i] = (val >> (i * 8)) & 0xFF;
        }
        uint8_t ret[32];
        size_t ret_len = 32;
        lau_yul_thunk_execute("cpu6502", poke_cd, 4 + 32 + 32, ret, &ret_len);
    }

    // Write registers after RAM to ensure they are not overwritten or truncated
    for (int r = 0; r < 6; r++) {
        uint64_t addr = 0x80 + r;
        uint64_t val = cp->registers[r];
        uint8_t poke_cd[4 + 32 + 32] = {0};
        poke_cd[0] = 0x80; poke_cd[1] = 0x29; poke_cd[2] = 0xe7; poke_cd[3] = 0xc0;
        for (int i = 0; i < 8; i++) {
            poke_cd[4 + 31 - i] = (addr >> (i * 8)) & 0xFF;
            poke_cd[4 + 32 + 31 - i] = (val >> (i * 8)) & 0xFF;
        }
        uint8_t ret[32];
        size_t ret_len = 32;
        lau_yul_thunk_execute("cpu6502", poke_cd, 4 + 32 + 32, ret, &ret_len);
    }
    
    printf("[FLIPTRACK] Checkpoint %d loaded successfully.\n", index);
}

static void zmm_vm_capture_screenshot(TsfiZmmVmState *state) {
    (void)state;
    printf("[FLIPTRACK] Screen Capture (40x25):\n");
    printf("+----------------------------------------+\n");
    for (int y = 0; y < 25; y++) {
        printf("|");
        for (int x = 0; x < 40; x++) {
            uint64_t addr = 1024 + y * 40 + x;
            uint8_t peek_cd[36] = {0};
            peek_cd[0] = 0x78; peek_cd[1] = 0x61; peek_cd[2] = 0xd2; peek_cd[3] = 0x69;
            for (int i = 0; i < 8; i++) {
                peek_cd[4 + 31 - i] = (addr >> (i * 8)) & 0xFF;
            }
            uint8_t peek_ret[32] = {0};
            size_t peek_ret_len = 32;
            lau_yul_thunk_execute("cpu6502", peek_cd, 36, peek_ret, &peek_ret_len);
            
            char c = (char)peek_ret[31];
            if (c >= 32 && c <= 126) {
                putchar(c);
            } else {
                putchar(' ');
            }
        }
        printf("|\n");
    }
    printf("+----------------------------------------+\n");
}

static void zmm_vm_record_cpu_state(TsfiZmmVmState *state, const char *name, const uint8_t *calldata, size_t cd_size, const uint8_t *retval, size_t retval_len) {
    if (!state->record_file) return;
    FILE *f = (FILE*)state->record_file;
    
    // Log basic execution event
    fprintf(f, "[YULEXEC] Contract: %s, Calldata Size: %zu\n", name, cd_size);
    fprintf(f, "  Calldata: ");
    for (size_t i = 0; i < cd_size; i++) fprintf(f, "%02x", calldata[i]);
    fprintf(f, "\n  Retval: ");
    for (size_t i = 0; i < retval_len; i++) fprintf(f, "%02x", retval[i]);
    fprintf(f, "\n");
    
    // If it's the 6502 CPU, also dump its internal registers
    if (strcmp(name, "cpu6502") == 0) {
        uint64_t regs[6] = {0};
        for (int r = 0; r < 6; r++) {
            uint64_t addr = 0x80 + r;
            uint8_t peek_cd[36] = {0};
            peek_cd[0] = 0x78; peek_cd[1] = 0x61; peek_cd[2] = 0xd2; peek_cd[3] = 0x69;
            for (int i = 0; i < 8; i++) {
                peek_cd[4 + 31 - i] = (addr >> (i * 8)) & 0xFF;
            }
            uint8_t peek_ret[32] = {0};
            size_t peek_ret_len = 32;
            lau_yul_thunk_execute(name, peek_cd, 36, peek_ret, &peek_ret_len);
            
            uint64_t val = 0;
            for (int i = 0; i < 8; i++) {
                val |= ((uint64_t)peek_ret[31 - i]) << (i * 8);
            }
            regs[r] = val;
        }
        fprintf(f, "  CPU6502 Registers -> A:%lu, X:%lu, Y:%lu, SR:0x%02lx, SP:0x%02lx, PC:%lu\n",
                regs[0], regs[1], regs[2], regs[3], regs[4], regs[5]);
    }
    fflush(f);
}

static void zmm_vm_memdump(TsfiZmmVmState *state, uint64_t start_addr, uint64_t length) {
    (void)state;
    printf("[MEMDUMP] Address range: $%" PRIx64 " - $%" PRIx64 " (%lu bytes)\n", start_addr, start_addr + length - 1, length);
    for (uint64_t addr = start_addr; addr < start_addr + length; addr += 16) {
        printf("  %04" PRIx64 ": ", addr);
        uint8_t row[16];
        for (int i = 0; i < 16; i++) {
            uint64_t curr = addr + i;
            if (curr >= start_addr + length) {
                printf("   ");
                row[i] = 0;
                continue;
            }
            uint8_t peek_cd[36] = {0};
            peek_cd[0] = 0x78; peek_cd[1] = 0x61; peek_cd[2] = 0xd2; peek_cd[3] = 0x69;
            for (int k = 0; k < 8; k++) {
                peek_cd[4 + 31 - k] = (curr >> (k * 8)) & 0xFF;
            }
            uint8_t peek_ret[32] = {0};
            size_t peek_ret_len = 32;
            lau_yul_thunk_execute("cpu6502", peek_cd, 36, peek_ret, &peek_ret_len);
            uint8_t b = peek_ret[31];
            row[i] = b;
            printf("%02x ", b);
        }
        printf(" |");
        for (int i = 0; i < 16; i++) {
            if (addr + i >= start_addr + length) break;
            uint8_t b = row[i];
            if (b >= 32 && b <= 126) printf("%c", b);
            else printf(".");
        }
        printf("|\n");
    }
}

static void zmm_vm_inspect_sprite(TsfiZmmVmState *state, uint64_t start_addr) {
    (void)state;
    printf("[SPRYTEBYTER] Sprite Visualization at $%lx (24x21 pixels):\n", start_addr);
    printf("+------------------------+\n");
    for (int y = 0; y < 21; y++) {
        printf("|");
        uint8_t row_bytes[3];
        for (int b = 0; b < 3; b++) {
            uint64_t curr = start_addr + y * 3 + b;
            uint8_t peek_cd[36] = {0};
            peek_cd[0] = 0x78; peek_cd[1] = 0x61; peek_cd[2] = 0xd2; peek_cd[3] = 0x69;
            for (int k = 0; k < 8; k++) {
                peek_cd[4 + 31 - k] = (curr >> (k * 8)) & 0xFF;
            }
            uint8_t peek_ret[32] = {0};
            size_t peek_ret_len = 32;
            lau_yul_thunk_execute("cpu6502", peek_cd, 36, peek_ret, &peek_ret_len);
            row_bytes[b] = peek_ret[31];
        }
        for (int b = 0; b < 3; b++) {
            uint8_t byteval = row_bytes[b];
            for (int bit = 7; bit >= 0; bit--) {
                if ((byteval >> bit) & 1) {
                    printf("X");
                } else {
                    printf(" ");
                }
            }
        }
        printf("|\n");
    }
    printf("+------------------------+\n");
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
                uint64_t virt_addr = strtoull(p, &end, 0);
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
                size_t start_idx = 0;
                if (len >= 2 && calldata_hex[0] == '0' && (calldata_hex[1] == 'x' || calldata_hex[1] == 'X')) {
                    start_idx = 2;
                }
                for (size_t i = start_idx; i < len; i += 2) {
                    unsigned int byteval = 0;
                    sscanf(&calldata_hex[i], "%02x", &byteval);
                    cd_bytes[cd_len++] = (uint8_t)byteval;
                }
                
                size_t max_retval_len = 262144;
                uint8_t *retval = malloc(max_retval_len);
                if (retval) {
                    size_t retval_len = max_retval_len;
                    bool success = lau_yul_thunk_execute(name, cd_bytes, cd_len, retval, &retval_len);
                    if (!success) {
                        retval_len = 0;
                    }
                    
                    for (size_t i = 0; i < retval_len && (size_t)state->output_pos < (sizeof(state->output_buffer) - 10); i++) {
                        sprintf(&state->output_buffer[state->output_pos], "%02x", retval[i]);
                        state->output_pos += 2;
                    }
                    
                    zmm_vm_record_cpu_state(state, name, cd_bytes, cd_len, retval, retval_len);
                    free(retval);
                }
            }
        }
        else if (*p == 'R' || *p == 'r') {
            p++;
            if (match_kw(&p, "ECSTART")) {
                char path[512] = {0};
                parse_string_arg(&p, path, sizeof(path));
                if (state->record_file) {
                    fclose((FILE*)state->record_file);
                }
                state->record_file = (void*)fopen(path, "w");
                if (state->record_file) {
                    fprintf((FILE*)state->record_file, "=== ZMM VM CPU Session Record Start ===\n");
                    fflush((FILE*)state->record_file);
                }
            }
            else if (match_kw(&p, "ECSTOP")) {
                if (state->record_file) {
                    fprintf((FILE*)state->record_file, "=== ZMM VM CPU Session Record End ===\n");
                    fclose((FILE*)state->record_file);
                    state->record_file = NULL;
                }
            }
            else if (match_kw(&p, "EU")) {
                char op[64] = {0};
                parse_string_arg(&p, op, sizeof(op));
                if (strcasecmp(op, "STASH") == 0) {
                    skip_comma(&p);
                    uint64_t host_addr = parse_num_arg(&p);
                    skip_comma(&p);
                    uint64_t reu_addr = parse_num_arg(&p);
                    skip_comma(&p);
                    uint64_t length = parse_num_arg(&p);
                    
                    if (reu_addr + length > state->reu_size) {
                        printf("[REU] ERROR: REU address boundary overflow.\n");
                    } else {
                        for (uint64_t i = 0; i < length; i++) {
                            uint64_t curr = host_addr + i;
                            uint8_t peek_cd[36] = {0};
                            peek_cd[0] = 0x78; peek_cd[1] = 0x61; peek_cd[2] = 0xd2; peek_cd[3] = 0x69;
                            for (int k = 0; k < 8; k++) {
                                peek_cd[4 + 31 - k] = (curr >> (k * 8)) & 0xFF;
                            }
                            uint8_t peek_ret[32] = {0};
                            size_t peek_ret_len = 32;
                            lau_yul_thunk_execute("cpu6502", peek_cd, 36, peek_ret, &peek_ret_len);
                            state->reu_ram[reu_addr + i] = peek_ret[31];
                        }
                        printf("[REU] STASH: Stashed $%lx bytes from host $%lx to REU $%lx\n", length, host_addr, reu_addr);
                    }
                }
                else if (strcasecmp(op, "FETCH") == 0) {
                    skip_comma(&p);
                    uint64_t reu_addr = parse_num_arg(&p);
                    skip_comma(&p);
                    uint64_t host_addr = parse_num_arg(&p);
                    skip_comma(&p);
                    uint64_t length = parse_num_arg(&p);
                    
                    if (reu_addr + length > state->reu_size) {
                        printf("[REU] ERROR: REU address boundary overflow.\n");
                    } else {
                        for (uint64_t i = 0; i < length; i++) {
                            uint64_t curr = host_addr + i;
                            uint8_t val = state->reu_ram[reu_addr + i];
                            uint8_t poke_cd[4 + 32 + 32] = {0};
                            poke_cd[0] = 0x80; poke_cd[1] = 0x29; poke_cd[2] = 0xe7; poke_cd[3] = 0xc0;
                            for (int k = 0; k < 8; k++) {
                                poke_cd[4 + 31 - k] = (curr >> (k * 8)) & 0xFF;
                                poke_cd[4 + 32 + 31 - k] = (val >> (k * 8)) & 0xFF;
                            }
                            uint8_t ret[32];
                            size_t ret_len = 32;
                            lau_yul_thunk_execute("cpu6502", poke_cd, 4 + 32 + 32, ret, &ret_len);
                        }
                        printf("[REU] FETCH: Fetched $%lx bytes from REU $%lx to host $%lx\n", length, reu_addr, host_addr);
                    }
                }
                else {
                    printf("=== Virtual REU DMA Telemetry Buffer (Sixth Sense Standard) ===\n");
                    printf("  Size: %u KB\n", state->reu_size / 1024);
                    printf("  Status: ACTIVE\n");
                }
            }
            else if (match_kw(&p, "ULE")) {
                char rule_str[512] = {0};
                parse_string_arg(&p, rule_str, sizeof(rule_str));
                
                char *r_start = rule_str;
                while (*r_start == ' ' || *r_start == '\t') r_start++;
                
                if (strncasecmp(r_start, "SET ", 4) == 0) {
                    char var_name[64] = {0};
                    char *v_ptr = r_start + 4;
                    while (*v_ptr == ' ' || *v_ptr == '\t') v_ptr++;
                    
                    int v_idx = 0;
                    while (*v_ptr && *v_ptr != ' ' && *v_ptr != '\t' && *v_ptr != '=' && v_idx < 63) {
                        var_name[v_idx++] = *v_ptr++;
                    }
                    while (*v_ptr == ' ' || *v_ptr == '\t' || *v_ptr == '=') v_ptr++;
                    
                    uint64_t val = strtoull(v_ptr, NULL, 0);
                    set_rule_var(var_name, val);
                    printf("[RULE] Variable SET: %s = %lu\n", var_name, val);
                } else {
                    if (strncasecmp(r_start, "the ", 4) == 0) {
                        r_start += 4;
                    }
                    
                    char outcome_var[64] = {0};
                    char *v_ptr = r_start;
                    int v_idx = 0;
                    while (*v_ptr && *v_ptr != ' ' && *v_ptr != '\t' && v_idx < 63) {
                        outcome_var[v_idx++] = *v_ptr++;
                    }
                    while (*v_ptr == ' ' || *v_ptr == '\t') v_ptr++;
                    
                    if (strncasecmp(v_ptr, "is ", 3) == 0) {
                        v_ptr += 3;
                    }
                    while (*v_ptr == ' ' || *v_ptr == '\t') v_ptr++;
                    
                    char *end_val_ptr = NULL;
                    uint64_t outcome_val = strtoull(v_ptr, &end_val_ptr, 0);
                    v_ptr = end_val_ptr;
                    while (*v_ptr == ' ' || *v_ptr == '\t') v_ptr++;
                    
                    if (strncasecmp(v_ptr, "if ", 3) == 0) {
                        v_ptr += 3;
                        bool cond_result = eval_rule_condition(v_ptr);
                        if (cond_result) {
                            set_rule_var(outcome_var, outcome_val);
                        }
                        printf("[RULE EVAL] Condition: \"%s\"\n", v_ptr);
                        printf("  Result:    %s\n", cond_result ? "TRUE" : "FALSE");
                        printf("  Outcome:   %s = %lu (set if TRUE)\n", outcome_var, get_rule_var(outcome_var));
                        
                        printf("  [YUL IR CODEGEN]:\n");
                        printf("    let _%s := mload(0x%lx)\n", outcome_var, (unsigned long)(0x1000 + g_rule_var_count * 32));
                        printf("    if eval_condition() {\n");
                        printf("      mstore(0x%lx, %lu)\n", (unsigned long)(0x1000 + g_rule_var_count * 32), outcome_val);
                        printf("    }\n");
                    } else {
                        printf("[RULE ERROR] Expected 'if' in rule: \"%s\"\n", rule_str);
                    }
                }
            }
        }
        else if (*p == 'F' || *p == 'f') {
            p++;
            if (match_kw(&p, "LIPSAVE")) {
                zmm_vm_save_checkpoint(state);
            }
            else if (match_kw(&p, "LIPLOAD")) {
                skip_ws(&p);
                char *end;
                int idx = (int)strtol(p, &end, 10);
                p = end;
                zmm_vm_load_checkpoint(state, idx);
            }
        }
        else if (*p == 'S' || *p == 's') {
            p++;
            if (match_kw(&p, "CREENSHOT")) {
                zmm_vm_capture_screenshot(state);
            }
            else if (match_kw(&p, "PRITE")) {
                uint64_t addr = parse_num_arg(&p);
                zmm_vm_inspect_sprite(state, addr);
            }
            else if (match_kw(&p, "WIFTLOAD")) {
                char filename[512] = {0};
                parse_string_arg(&p, filename, sizeof(filename));
                skip_comma(&p);
                uint64_t start_addr = parse_num_arg(&p);
                
                FILE *f = fopen(filename, "rb");
                if (!f) {
                    printf("[FLIPTRACK] SWIFTLOAD ERROR: Cannot open file \"%s\"\n", filename);
                } else {
                    uint8_t buf[1024];
                    size_t read_bytes;
                    uint64_t curr_addr = start_addr;
                    while ((read_bytes = fread(buf, 1, sizeof(buf), f)) > 0) {
                        for (size_t i = 0; i < read_bytes; i++) {
                            uint8_t val = buf[i];
                            uint8_t poke_cd[4 + 32 + 32] = {0};
                            poke_cd[0] = 0x80; poke_cd[1] = 0x29; poke_cd[2] = 0xe7; poke_cd[3] = 0xc0;
                            for (int k = 0; k < 8; k++) {
                                poke_cd[4 + 31 - k] = (curr_addr >> (k * 8)) & 0xFF;
                                poke_cd[4 + 32 + 31 - k] = (val >> (k * 8)) & 0xFF;
                            }
                            uint8_t ret[32];
                            size_t ret_len = 32;
                            lau_yul_thunk_execute("cpu6502", poke_cd, 4 + 32 + 32, ret, &ret_len);
                            curr_addr++;
                        }
                    }
                    fclose(f);
                    printf("[FLIPTRACK] SWIFTLOAD: Loaded \"%s\" into address range $%lx - $%lx\n", filename, start_addr, curr_addr);
                }
            }
            else if (match_kw(&p, "WIFTSAVE")) {
                char filename[512] = {0};
                parse_string_arg(&p, filename, sizeof(filename));
                skip_comma(&p);
                uint64_t start_addr = parse_num_arg(&p);
                skip_comma(&p);
                uint64_t end_addr = parse_num_arg(&p);
                
                FILE *f = fopen(filename, "wb");
                if (!f) {
                    printf("[FLIPTRACK] SWIFTSAVE ERROR: Cannot open file \"%s\" for writing\n", filename);
                } else {
                    for (uint64_t curr = start_addr; curr < end_addr; curr++) {
                        uint8_t peek_cd[36] = {0};
                        peek_cd[0] = 0x78; peek_cd[1] = 0x61; peek_cd[2] = 0xd2; peek_cd[3] = 0x69;
                        for (int k = 0; k < 8; k++) {
                            peek_cd[4 + 31 - k] = (curr >> (k * 8)) & 0xFF;
                        }
                        uint8_t peek_ret[32] = {0};
                        size_t peek_ret_len = 32;
                        lau_yul_thunk_execute("cpu6502", peek_cd, 36, peek_ret, &peek_ret_len);
                        uint8_t val = peek_ret[31];
                        fwrite(&val, 1, 1, f);
                    }
                    fclose(f);
                    printf("[FLIPTRACK] SWIFTSAVE: Saved address range $%lx - $%lx to \"%s\"\n", start_addr, end_addr, filename);
                }
            }
        }
        else if (*p == 'M' || *p == 'm') {
            p++;
            if (match_kw(&p, "EMO")) {
                char text[256] = {0};
                parse_string_arg(&p, text, sizeof(text));
                if (strlen(text) > 0) {
                    if (state->checkpoint_count > 0) {
                        ZmmCpuCheckpoint *cp = &((ZmmCpuCheckpoint*)state->checkpoints)[state->checkpoint_count - 1];
                        strcpy(cp->memo, text);
                        printf("[FLIPTRACK] Memo added to Checkpoint %d: \"%s\"\n", state->checkpoint_count - 1, cp->memo);
                    } else {
                        printf("[FLIPTRACK] ERROR: No checkpoints exist to add memo to.\n");
                    }
                } else {
                    printf("[FLIPTRACK] Saved Checkpoint Memos:\n");
                    for (int i = 0; i < state->checkpoint_count; i++) {
                        ZmmCpuCheckpoint *cp = &((ZmmCpuCheckpoint*)state->checkpoints)[i];
                        printf("  Checkpoint %d: \"%s\"\n", i, cp->memo);
                    }
                }
            }
            else if (match_kw(&p, "EMDUMP")) {
                uint64_t start_addr = parse_num_arg(&p);
                skip_comma(&p);
                uint64_t length = parse_num_arg(&p);
                if (length == 0) length = 256;
                zmm_vm_memdump(state, start_addr, length);
            }
        }
        else if (*p == 'D' || *p == 'd') {
            p++;
            if (match_kw(&p, "IR")) {
                DIR *dir;
                struct dirent *ent;
                if ((dir = opendir(".")) != NULL) {
                    printf("[FLIPTRACK] Directory Listing:\n");
                    while ((ent = readdir(dir)) != NULL) {
                        if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0) {
                            printf("  %s\n", ent->d_name);
                        }
                    }
                    closedir(dir);
                } else {
                    perror("[FLIPTRACK] Could not open directory");
                }
            }
            else if (match_kw(&p, "ISK")) {
                char cmd[256] = {0};
                parse_string_arg(&p, cmd, sizeof(cmd));
                if (strcasecmp(cmd, "CATALOG") == 0) {
                    DIR *dir;
                    struct dirent *ent;
                    if ((dir = opendir(".")) != NULL) {
                        printf("[FLIPTRACK] DISK CATALOG:\n");
                        while ((ent = readdir(dir)) != NULL) {
                            if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0) {
                                printf("  %s\n", ent->d_name);
                            }
                        }
                        closedir(dir);
                    }
                }
                else if (strncasecmp(cmd, "SCRATCH ", 8) == 0) {
                    const char *filename = cmd + 8;
                    if (remove(filename) == 0) {
                        printf("[FLIPTRACK] DISK: Scratched file \"%s\"\n", filename);
                    } else {
                        printf("[FLIPTRACK] DISK ERROR: Cannot scratch file \"%s\"\n", filename);
                    }
                }
                else if (strncasecmp(cmd, "RENAME ", 7) == 0) {
                    char oldname[256] = {0};
                    char newname[256] = {0};
                    const char *comma = strchr(cmd + 7, ',');
                    if (comma) {
                        size_t old_len = comma - (cmd + 7);
                        if (old_len >= sizeof(oldname)) old_len = sizeof(oldname) - 1;
                        strncpy(oldname, cmd + 7, old_len);
                        char *old_end = oldname + strlen(oldname) - 1;
                        while (old_end > oldname && (*old_end == ' ' || *old_end == '\t')) {
                            *old_end = '\0';
                            old_end--;
                        }
                        const char *new_start = comma + 1;
                        while (*new_start == ' ' || *new_start == '\t') new_start++;
                        strncpy(newname, new_start, sizeof(newname) - 1);
                        
                        if (rename(oldname, newname) == 0) {
                            printf("[FLIPTRACK] DISK: Renamed \"%s\" to \"%s\"\n", oldname, newname);
                        } else {
                            printf("[FLIPTRACK] DISK ERROR: Cannot rename \"%s\" to \"%s\"\n", oldname, newname);
                        }
                    }
                }
            }
        }
        else if (*p == 'C' || *p == 'c') {
            p++;
            if (match_kw(&p, "ALC")) {
                char expr[512] = {0};
                parse_string_arg(&p, expr, sizeof(expr));
                const char *expr_ptr = expr;
                uint64_t result = eval_expr(&expr_ptr);
                printf("[CALC] Result of expression \"%s\":\n", expr);
                printf("  Decimal: %lu\n", result);
                printf("  Hex:     $%" PRIx64 "\n", result);
                
                char bin_str[65];
                int b_idx = 0;
                uint64_t temp = result;
                if (temp == 0) bin_str[b_idx++] = '0';
                else {
                    int leading_zero = 1;
                    for (int i = 63; i >= 0; i--) {
                        int bit = (temp >> i) & 1;
                        if (bit) leading_zero = 0;
                        if (!leading_zero) {
                            bin_str[b_idx++] = bit ? '1' : '0';
                        }
                    }
                }
                bin_str[b_idx] = '\0';
                printf("  Binary:  %%%s\n", bin_str);
            }
        }
        else if (*p == 'O' || *p == 'o') {
            p++;
            if (match_kw(&p, "NICOMM")) {
                char format[64] = {0};
                parse_string_arg(&p, format, sizeof(format));
                
                if (state->telem) {
                    LauTelemetryState *t = (LauTelemetryState*)state->telem;
                    if (strcasecmp(format, "LLS") == 0) {
                        skip_comma(&p);
                        uint64_t lls_cmd = parse_num_arg(&p);
                        if (lls_cmd == 0) lls_cmd = 0x06; // Default to 0x06 Data Request
                        
                        uint8_t net_addr = 0x01;
                        if (lls_cmd == 0x06) {
                            // Data Request (0x06) -> Response prefix 0x31
                            uint8_t prefix = 0x31;
                            uint16_t fuel_level = (uint16_t)((t->coverage_pct / 100.0f) * 4095.0f);
                            uint8_t temp_val = 25 + 128; // 25 C
                            uint8_t crc = (uint8_t)(prefix + net_addr + 0x06 + (fuel_level & 0xFF) + ((fuel_level >> 8) & 0xFF) + temp_val);
                            
                            printf("[OMNICOMM LLS PACKET] 31 %02x 06 %02x %02x %02x %02x\n", 
                                   net_addr, fuel_level & 0xFF, (fuel_level >> 8) & 0xFF, temp_val, crc);
                            printf("  Sensor Address: 0x%02x\n", net_addr);
                            printf("  Fuel Level:     %u / 4095\n", fuel_level);
                            printf("  Temperature:    %d C\n", temp_val - 128);
                        }
                        else if (lls_cmd == 0x24) {
                            // Serial Number Request (0x24) -> Response prefix 0x3E
                            uint8_t prefix = 0x3E;
                            uint32_t serial = 987654321; // Emulated serial number
                            uint8_t crc = (uint8_t)(prefix + net_addr + 0x24 + (serial & 0xFF) + ((serial >> 8) & 0xFF) + ((serial >> 16) & 0xFF) + ((serial >> 24) & 0xFF));
                            
                            printf("[OMNICOMM LLS PACKET] 3E %02x 24 %02x %02x %02x %02x %02x\n", 
                                   net_addr, serial & 0xFF, (serial >> 8) & 0xFF, (serial >> 16) & 0xFF, (serial >> 24) & 0xFF, crc);
                            printf("  Sensor Address: 0x%02x\n", net_addr);
                            printf("  Serial Number:  %u\n", serial);
                        }
                        else {
                            printf("[OMNICOMM LLS] ERROR: Unsupported LLS Command 0x%02lx\n", lls_cmd);
                        }
                    }
                    else {
                        printf("=== OMNICOMM Telemetry Report ===\n");
                        printf("  ZMM Msg:            %s\n", t->zmm_msg);
                        printf("  ZMM Val:            %lu\n", (unsigned long)t->zmm_val);
                        printf("  Coverage Pct:       %.2f%%\n", (double)t->coverage_pct);
                        printf("  XOR Trap Active:    %s\n", t->xor_trap_active ? "TRUE" : "FALSE");
                        printf("  Frames Rendered:    %lu\n", (unsigned long)t->render_telemetry.frames_rendered);
                        printf("  Avg MS Per Frame:   %.2f ms\n", t->render_telemetry.avg_ms_per_frame);
                        printf("  Active Fibers:      %u\n", t->render_telemetry.active_fibers);
                        printf("  SVDAG Nodes:        %u\n", t->render_telemetry.svdag_nodes);
                        printf("  Blue Secret Flux:   %.2f\n", (double)t->render_telemetry.blue_secret_flux);
                        printf("  Current Epoch:      %u\n", t->render_telemetry.current_epoch);
                        printf("  Path Trace Evals:   %.2f M\n", (double)t->render_telemetry.path_trace_m_evals);
                    }
                } else {
                    printf("[OMNICOMM] ERROR: Telemetry state is not attached.\n");
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
                zmm_vm_record_cpu_state(state, name, calldata, cd_size, retval, *retval_len);
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