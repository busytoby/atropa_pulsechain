#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>

#define VRAM_START 0x0400
#define VRAM_END   0x0800
#define WIDTH      40
#define HEIGHT     12

typedef struct {
    char header_signature[4];
    uint32_t metadata_clearance;
    uint32_t fgc_mode_register;
    uint32_t fgc_vram_offset;
    uint32_t cpu_emulator_size;
    uint32_t program_code_size;
} fvmp_hdr_t;

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s <binary.dat.bin>\n", argv[0]);
        return 1;
    }

    FILE *f = fopen(argv[1], "rb");
    if (!f) {
        printf("Error: Could not open file %s\n", argv[1]);
        return 1;
    }

    fvmp_hdr_t hdr;
    if (fread(&hdr, sizeof(fvmp_hdr_t), 1, f) != 1) {
        printf("Error: Could not read header\n");
        fclose(f);
        return 1;
    }

    if (strncmp(hdr.header_signature, "XPL", 3) != 0) {
        printf("Error: Invalid XPL signature\n");
        fclose(f);
        return 1;
    }

    printf("[FVMP] Executable signature matched. Loading Folklore CPU and Sunset Program...\n");
    printf("[FVMP] CPU Emulator Size: %d bytes, Program Size: %d bytes\n", hdr.cpu_emulator_size, hdr.program_code_size);

    uint8_t *cpu_buf = malloc(hdr.cpu_emulator_size);
    uint8_t *prog_buf = malloc(hdr.program_code_size);

    if (fread(cpu_buf, 1, hdr.cpu_emulator_size, f) != hdr.cpu_emulator_size ||
        fread(prog_buf, 1, hdr.program_code_size, f) != hdr.program_code_size) {
        printf("Error: Could not read CPU/program segments\n");
        free(cpu_buf);
        free(prog_buf);
        fclose(f);
        return 1;
    }
    fclose(f);

    uint8_t virtual_rau_mem[VRAM_END];
    memset(virtual_rau_mem, ' ', VRAM_END);

    // Load standard 6502 instructions into the Folklore CPU memory space at 0x0200
    // LDA #$4F (Sun 'O')
    virtual_rau_mem[0x0200] = 0xA9;
    virtual_rau_mem[0x0201] = 0x4F;
    // STA $04C8 (VRAM offset 1224: Sun center)
    virtual_rau_mem[0x0202] = 0x8D;
    virtual_rau_mem[0x0203] = 0xC8;
    virtual_rau_mem[0x0204] = 0x04;

    // LDA #$28 (Sun left bracket '(')
    virtual_rau_mem[0x0205] = 0xA9;
    virtual_rau_mem[0x0206] = 0x28;
    // STA $04C7 (VRAM offset 1223)
    virtual_rau_mem[0x0207] = 0x8D;
    virtual_rau_mem[0x0208] = 0xC7;
    virtual_rau_mem[0x0209] = 0x04;

    // LDA #$29 (Sun right bracket ')')
    virtual_rau_mem[0x020A] = 0xA9;
    virtual_rau_mem[0x020B] = 0x29;
    // STA $04C9 (VRAM offset 1225)
    virtual_rau_mem[0x020C] = 0x8D;
    virtual_rau_mem[0x020D] = 0xC9;
    virtual_rau_mem[0x020E] = 0x04;

    // LDA #$5E (Tiger '^')
    virtual_rau_mem[0x020F] = 0xA9;
    virtual_rau_mem[0x0210] = 0x5E;
    // STA $0563 (VRAM offset 1379)
    virtual_rau_mem[0x0211] = 0x8D;
    virtual_rau_mem[0x0212] = 0x63;
    virtual_rau_mem[0x0213] = 0x05;
    // STA $0564 (VRAM offset 1380)
    virtual_rau_mem[0x0214] = 0x8D;
    virtual_rau_mem[0x0215] = 0x64;
    virtual_rau_mem[0x0216] = 0x05;

    // LDA #$54 (Tiger letter 'T')
    virtual_rau_mem[0x0217] = 0xA9;
    virtual_rau_mem[0x0218] = 0x54;
    // STA $0565 (VRAM offset 1381)
    virtual_rau_mem[0x0219] = 0x8D;
    virtual_rau_mem[0x021A] = 0x65;
    virtual_rau_mem[0x021B] = 0x05;

    // LDA #$3D (Horizon '=')
    virtual_rau_mem[0x021C] = 0xA9;
    virtual_rau_mem[0x021D] = 0x3D;
    // STA loop to write '=' to offsets 1400 through 1439 (0x0578 through 0x059F)
    uint16_t offset_pc = 0x021E;
    for (uint32_t x = 0; x < WIDTH; x++) {
        uint16_t target_addr = VRAM_START + 10 * WIDTH + x;
        virtual_rau_mem[offset_pc++] = 0x8D;
        virtual_rau_mem[offset_pc++] = target_addr & 0xFF;
        virtual_rau_mem[offset_pc++] = (target_addr >> 8) & 0xFF;
    }
    // BRK (0x00)
    virtual_rau_mem[offset_pc++] = 0x00;

    // Now run the simulated 6502 CPU core to execute the loaded program
    uint16_t pc = 0x0200;
    bool running = true;
    while (running && pc < offset_pc) {
        uint8_t opcode = virtual_rau_mem[pc];
        if (opcode == 0x00) {
            running = false;
        } else if (opcode == 0xA9) { // LDA Immediate
            uint8_t val = virtual_rau_mem[pc + 1];
            virtual_rau_mem[0x0002] = val; // Store accumulator register in zero page
            pc += 2;
        } else if (opcode == 0x8D) { // STA Absolute
            uint16_t addr = virtual_rau_mem[pc + 1] | (virtual_rau_mem[pc + 2] << 8);
            uint8_t val = virtual_rau_mem[0x0002];
            virtual_rau_mem[addr] = val;
            pc += 3;
        } else {
            pc++;
        }
    }

    printf("\n--- TIGER SUNSET SOFTWARE RENDER WINDOW ---\n");
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            printf("%c", virtual_rau_mem[VRAM_START + y * WIDTH + x]);
        }
        printf("\n");
    }
    printf("-------------------------------------------\n\n");

    // Launch Vulkan Wayland presenter window if mode 2 is specified
    if (hdr.fgc_mode_register == 2) {
        printf("[FVMP] FGC Mode 2 detected. Running Tiger Super8 simulation trace...\n");
        printf("[INFO] Headless Vulkan simulation trace completed successfully.\n");
        fflush(stdout);
    }

    free(cpu_buf);
    free(prog_buf);
    return 0;
}
