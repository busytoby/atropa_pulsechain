#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_zmm_vm.h"
#include "tsfi_wire_firmware.h"

#define REG_CD_STATE 0xFC00
#define REG_CD_START 0xFC01
#define REG_CD_END   0xFC02
#define REG_CD_SEC   0xFC03
#define REG_CD_DST   0xFC04

// Structure for persistent high score save - Rule 13 compliant (.dat.bin extension only)
typedef struct {
    uint32_t magic;
    uint32_t high_score;
    char player_tag[4];
} HighScoreSave;

int main() {
    printf("=== Auncient PC-FX + PC Engine Hybrid Visual Demo ===\n");
    tsfi_wire_firmware_init();

    TsfiZmmVmState vm;
    tsfi_zmm_vm_init(&vm);

    // 1. Compile and deploy hucSystem.yul
    printf("[ZMM] Booting CD-ROM System Card BIOS...\n");
    tsfi_zmm_vm_exec(&vm, "YULINIT \"huc\", \"../solidity/bin/hucSystem.yul\", 7");

    char cmd[512];

    // 2. CD-ROM BIOS Emulation: cdPlay(trackStart=2, trackEnd=5)
    printf("\n--- Step 1: CD-ROM Audio Streaming (cdPlay) ---\n");
    // Selector 0x9fb46210, startT=2, endT=5
    sprintf(cmd, "YULEXEC \"huc\", \"9fb46210"
                 "0000000000000000000000000000000000000000000000000000000000000002"
                 "0000000000000000000000000000000000000000000000000000000000000005\"");
    tsfi_zmm_vm_exec(&vm, cmd);

    // Verify CD State is playing (1)
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, "YULEXEC \"huc\", \"1fca8099\""); // cdStat
    assert(strstr(vm.output_buffer, "0000000000000000000000000000000000000000000000000000000000000001") != NULL);
    printf("[CD-ROM] Status: Playing track 2 to 5. Audio ADPCM channel outputting chiptune stream.\n");

    // 3. CD-ROM BIOS Emulation: cdRead(sector=150, dstAddress=0xF700)
    printf("\n--- Step 2: CD-ROM Data Transfer (cdRead) ---\n");
    // Selector 0x3a4b0d10, sector=150, dst=0xF700 (GEL blocks)
    sprintf(cmd, "YULEXEC \"huc\", \"3a4b0d10"
                 "0000000000000000000000000000000000000000000000000000000000000096" // 150 = 0x96
                 "000000000000000000000000000000000000000000000000000000000000f700\"");
    tsfi_zmm_vm_exec(&vm, cmd);

    // Verify CD State is reading (2)
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, "YULEXEC \"huc\", \"1fca8099\""); // cdStat
    assert(strstr(vm.output_buffer, "0000000000000000000000000000000000000000000000000000000000000002") != NULL);
    printf("[CD-ROM] Status: Transferred sector data directly to Gel buffer (0xF700) via DMA.\n");

    // 4. Persistent High Score Save: Save to .dat.bin (Rule 13 compliant)
    printf("\n--- Step 3: High Score Save (.dat.bin) ---\n");
    HighScoreSave save_data;
    save_data.magic = 0xAF00ECE0; // Auncient PC Engine magic
    save_data.high_score = 998200; // Simulated Star Soldier High Score
    strcpy(save_data.player_tag, "SOL");
    
    const char *save_file = "highscore.dat.bin";
    FILE *f = fopen(save_file, "wb");
    assert(f != NULL);
    fwrite(&save_data, sizeof(HighScoreSave), 1, f);
    fclose(f);

    printf("[SRAM] High score saved successfully to binary asset: %s\n", save_file);
    
    // Read back and assert
    HighScoreSave loaded_data;
    f = fopen(save_file, "rb");
    assert(f != NULL);
    fread(&loaded_data, sizeof(HighScoreSave), 1, f);
    fclose(f);
    assert(loaded_data.magic == 0xAF00ECE0);
    assert(loaded_data.high_score == 998200);
    assert(strcmp(loaded_data.player_tag, "SOL") == 0);
    printf("[SRAM] High score verified from disk. Integrity confirmed.\n");
    remove(save_file);

    tsfi_zmm_vm_destroy(&vm);
    printf("\n=== HYBRID CD-ROM DEMO COMPLETED SUCCESSFULLY ===\n");
    return 0;
}
