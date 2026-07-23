#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_zmm_vm.h"
#include "tsfi_wire_firmware.h"

// Mock XPL Lexer Tokens
typedef enum {
    TOKEN_BYTE,
    TOKEN_LPAREN,
    TOKEN_NUMBER,
    TOKEN_RPAREN,
    TOKEN_ASSIGN,
    TOKEN_SEMICOLON,
    TOKEN_UNKNOWN
} XplToken;

// Basic XPL compiler statement parser: BYTE(addr) = val;
int mock_xpl_compile_and_execute(TsfiZmmVmState *vm, const char *source) {
    printf("[XPL] Compiling statement: %s\n", source);
    
    // Minimal token parsing
    if (strncmp(source, "BYTE", 4) != 0) return -1;
    
    const char *p = source + 4;
    while (*p == ' ' || *p == '\t') p++;
    if (*p != '(') return -2;
    p++;
    
    char addr_buf[32] = {0};
    int addr_idx = 0;
    while (*p >= '0' && *p <= '9') {
        addr_buf[addr_idx++] = *p++;
    }
    if (*p != ')') return -3;
    p++;
    
    while (*p == ' ' || *p == '\t') p++;
    if (*p != '=') return -4;
    p++;
    
    while (*p == ' ' || *p == '\t') p++;
    char val_buf[32] = {0};
    int val_idx = 0;
    while (*p >= '0' && *p <= '9') {
        val_buf[val_idx++] = *p++;
    }
    if (*p != ';') return -5;
    
    unsigned int address = atoi(addr_buf);
    unsigned int value = atoi(val_buf);
    
    printf("[XPL] Resolved Address: %u, Value: %u\n", address, value);
    
    // Translate to ZMM VM execution call (VCE selector: vceWriteColor)
    // 62208 (0xF300) maps to VCE color index 0. We write R/G/B component parts
    if (address >= 62208 && address <= 62719) {
        unsigned int color_idx = address - 62208;
        // Mock RGB extraction from value (packed 9-bit representation)
        unsigned int r = (value >> 6) & 0x7;
        unsigned int g = (value >> 3) & 0x7;
        unsigned int b = value & 0x7;
        
        char calldata[512];
        // vceWriteColor(colorIdx, r, g, b) selector: 0xc14ab90d
        sprintf(calldata, "YULEXEC \"huc\", \"c14ab90d"
                          "%064x"
                          "%064x"
                          "%064x"
                          "%064x\"",
                color_idx, r, g, b);
        
        tsfi_zmm_vm_exec(vm, calldata);
        return 0;
    }
    
    return -6;
}

int main() {
    printf("=== TSFi ZMM VM Auncient XPL compiler VCE Test ===\n");
    tsfi_wire_firmware_init();

    TsfiZmmVmState vm;
    tsfi_zmm_vm_init(&vm);

    // 1. Compile and deploy hucSystem.yul
    printf("[ZMM] Loading hucSystem.yul to VM...\n");
    tsfi_zmm_vm_exec(&vm, "YULINIT \"huc\", \"../solidity/bin/hucSystem.yul\", 7");

    // 2. Compile XPL statement targeting VCE index 0 (address 62208)
    // Packed value 306 = (4 << 6) | (6 << 3) | 2 = 0x132
    int status = mock_xpl_compile_and_execute(&vm, "BYTE(62208) = 306;");
    assert(status == 0);

    // 3. Verify VCE register index 0 holds 306 (0x132) via vceReadColor
    printf("[ZMM] Querying VCE Color Index 0 via vceReadColor...\n");
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, "YULEXEC \"huc\", \"22bbfa840000000000000000000000000000000000000000000000000000000000000000\"");
    printf("[ZMM] vceReadColor(0) returned: %s\n", vm.output_buffer);
    assert(strstr(vm.output_buffer, "0000000000000000000000000000000000000000000000000000000000000132") != NULL);

    // 4. Test invalid statements to verify parser bounds
    assert(mock_xpl_compile_and_execute(&vm, "INVALID(62208) = 1;") < 0);
    assert(mock_xpl_compile_and_execute(&vm, "BYTE 62208 = 1;") < 0);
    
    tsfi_zmm_vm_destroy(&vm);
    printf("=== ALL AUNCIENT XPL PARSER TESTS PASSED ===\n");
    return 0;
}
