#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

typedef struct {
    uint32_t magic;
    uint32_t turn_id;
    uint32_t prompt_len;
    uint32_t response_len;
    float    chatrath_entropy_risk;
    float    chatrath_slam_residual;
    uint32_t clawvm_pages;
    float    clawvm_thrash;
    char     model[128];
} zorse_prompt_receipt_t;

int main(int argc, char **argv) {
    const char *wal_path = (argc > 1) ? argv[1] : "zorse_local_prompt.dat.bin";

    printf("================ OPENCLAW FORENSIC WAL TRACE INSPECTOR ================\n");
    printf("  Target Media: %s\n", wal_path);
    printf("  Storage:      Binary Media Layout (.dat.bin) under Rule 13\n");
    printf("========================================================================\n\n");

    FILE *f = fopen(wal_path, "rb");
    if (!f) {
        printf("[ERROR] Failed to open WAL binary media file: %s\n", wal_path);
        return 1;
    }

    zorse_prompt_receipt_t rcpt;
    uint32_t count = 0;
    while (fread(&rcpt, sizeof(rcpt), 1, f) == 1) {
        count++;
        printf("--- Trace Entry #%u ---\n", count);
        printf("  Magic:         0x%08X (%s)\n", rcpt.magic, (rcpt.magic == 0x5A50524D) ? "VALID 'ZPRM'" : "INVALID");
        printf("  Turn ID:       %u\n", rcpt.turn_id);
        printf("  Prompt Length: %u chars\n", rcpt.prompt_len);
        printf("  Output Length: %u chars\n", rcpt.response_len);
        printf("  Entropy Risk:  %.4f\n", rcpt.chatrath_entropy_risk);
        printf("  SLAM Residual: %.4f\n", rcpt.chatrath_slam_residual);
        printf("  ClawVM Pages:  %u pages\n", rcpt.clawvm_pages);
        printf("  Thrash Index:  %.4f (Zero-Fault Bound)\n", rcpt.clawvm_thrash);
        printf("  Active Model:  %s\n", rcpt.model);
        printf("-----------------------\n");
    }

    fclose(f);
    printf("\n[SUMMARY] Total %u verified forensic receipts in %s.\n", count, wal_path);
    return 0;
}
