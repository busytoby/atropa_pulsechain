#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_dat.h"
#include "tsfi_trie.h"

int main(void) {
    printf("=== TSFi Double-Array Trie (DAT) Verification ===\n");

    // ----------------------------------------------------
    // TEST 1: WinchesterMQ SCSI Command Handshake DAT
    // ----------------------------------------------------
    printf("[DAT_SCSI] Initializing SCSI command Trie...\n");
    tsfi_trie_node *scsi_trie = tsfi_trie_create_node('\0');

    tsfi_trie_insert(scsi_trie, "CMD_READ", "SCSI_OP_READ");
    tsfi_trie_insert(scsi_trie, "CMD_WRITE", "SCSI_OP_WRITE");
    tsfi_trie_insert(scsi_trie, "CMD_FORMAT", "SCSI_OP_FORMAT");
    tsfi_trie_insert(scsi_trie, "STATUS_ERR", "SCSI_STATE_ERR");

    printf("[DAT_SCSI] Compiling SCSI command Trie to DAT...\n");
    tsfi_dat *scsi_dat = tsfi_dat_compile(scsi_trie);
    assert(scsi_dat != NULL);

    printf("[DAT_SCSI] Searching commands in DAT...\n");
    const char *s1 = tsfi_dat_search(scsi_dat, "CMD_READ");
    assert(s1 != NULL && strcmp(s1, "SCSI_OP_READ") == 0);
    printf("  [PASS] 'CMD_READ' -> %s\n", s1);

    const char *s2 = tsfi_dat_search(scsi_dat, "CMD_WRITE");
    assert(s2 != NULL && strcmp(s2, "SCSI_OP_WRITE") == 0);
    printf("  [PASS] 'CMD_WRITE' -> %s\n", s2);

    const char *s_none = tsfi_dat_search(scsi_dat, "CMD_INVALID");
    assert(s_none == NULL);
    printf("  [PASS] Invalid SCSI command correctly returned NULL.\n");

    tsfi_trie_destroy(scsi_trie);
    tsfi_dat_destroy(scsi_dat);

    // ----------------------------------------------------
    // TEST 2: Font Layout Kerning & Ligature DAT
    // ----------------------------------------------------
    printf("[DAT_FONT] Initializing Font Ligature & Kerning Trie...\n");
    tsfi_trie_node *font_trie = tsfi_trie_create_node('\0');

    tsfi_trie_insert(font_trie, "fi", "GLYPH_LIG_FI");
    tsfi_trie_insert(font_trie, "fl", "GLYPH_LIG_FL");
    tsfi_trie_insert(font_trie, "ffi", "GLYPH_LIG_FFI");
    tsfi_trie_insert(font_trie, "AV", "KERN_-20");
    tsfi_trie_insert(font_trie, "VA", "KERN_-20");

    printf("[DAT_FONT] Compiling Font Trie to DAT...\n");
    tsfi_dat *font_dat = tsfi_dat_compile(font_trie);
    assert(font_dat != NULL);

    printf("[DAT_FONT] Searching font ligatures & kerning in DAT...\n");
    const char *f1 = tsfi_dat_search(font_dat, "fi");
    assert(f1 != NULL && strcmp(f1, "GLYPH_LIG_FI") == 0);
    printf("  [PASS] Ligature 'fi' -> %s\n", f1);

    const char *f2 = tsfi_dat_search(font_dat, "ffi");
    assert(f2 != NULL && strcmp(f2, "GLYPH_LIG_FFI") == 0);
    printf("  [PASS] Ligature 'ffi' -> %s\n", f2);

    const char *k1 = tsfi_dat_search(font_dat, "AV");
    assert(k1 != NULL && strcmp(k1, "KERN_-20") == 0);
    printf("  [PASS] Kerning 'AV' -> %s\n", k1);

    tsfi_trie_destroy(font_trie);
    tsfi_dat_destroy(font_dat);

    printf("=== ALL DOUBLE-ARRAY TRIE TESTS PASSED ===\n");
    return 0;
}
