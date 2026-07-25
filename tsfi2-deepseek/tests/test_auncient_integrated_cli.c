#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <time.h>

#include "tsfi_types.h"

#include "tsfi_wiring.h"
#include "tsfi_cli.h"
#include "auncient_sdk.h"

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT INTEGRATED CLI TEST\n");
    printf("=============================================================\n");

    // Initialize WaveSystem context
    WaveSystem *ws = tsfi_create_system();
    if (!ws) {
        fprintf(stderr, "FATAL: System creation failed\n");
        return 1;
    }

    // 1. Speak a pre-loaded word immediately (SPK_WRITE_LEDGER, ID 2)
    printf("[TEST] Speaking pre-loaded SPK_WRITE_LEDGER directly...\n");
    char cmd_pre_loaded[128] = "SPEAK 350.0 700.0 0.2 2";
    int status = tsfi_cli_process_line(ws, cmd_pre_loaded);
    assert(status == 0);

    // 2. Register and speak a new custom dynamic word (SPK_CUSTOM_CMD, ID 4)
    printf("[TEST] Dispatching WORD compile directive for SPK_CUSTOM_CMD...\n");
    char cmd_word[128] = "WORD SPK_CUSTOM_CMD 4 500.0 1000.0 0.3 00 02";
    status = tsfi_cli_process_line(ws, cmd_word);
    assert(status == 0);

    printf("[TEST] Dispatching SPEAK command wave for SPK_CUSTOM_CMD...\n");
    char cmd_speak[128] = "SPEAK 500.0 1000.0 0.3 4";
    status = tsfi_cli_process_line(ws, cmd_speak);
    assert(status == 0);

    // 3. Register a short-lived command and verify temporal expiration (Wheeler Jump Replay Gating)
    printf("[TEST] Dispatching short-lived SPK_EXPIRED_CMD...\n");
    char cmd_expired_word[128] = "WORD SPK_EXPIRED_CMD 5 400.0 800.0 0.1 00 02";
    status = tsfi_cli_process_line(ws, cmd_expired_word);
    assert(status == 0);

    struct timespec slp = { .tv_sec = 0, .tv_nsec = 150000000 };
    nanosleep(&slp, NULL); // 150ms > 100ms decay limit


    printf("[TEST] Speaking expired SPK_EXPIRED_CMD (should fail)...\n");
    char cmd_expired_speak[128] = "SPEAK 400.0 800.0 0.1 5";
    status = tsfi_cli_process_line(ws, cmd_expired_speak);
    assert(status == 1); // Confirms the CLI rejected the expired command wave
    // 3.5. Test lexicon compaction/garbage collection of expired words
    printf("[TEST] Dispatching LEXICON_COMPACT to prune expired words...\n");
    char cmd_compact[128] = "LEXICON_COMPACT";
    status = tsfi_cli_process_line(ws, cmd_compact);
    assert(status == 0);

    // 4. Test altering words in memory using COBOL strategy
    printf("[TEST] Dispatching COBOL_ALTER command to modify SPK_CUSTOM_CMD WMQ byte in memory...\n");
    char cmd_alter[128] = "COBOL_ALTER 3 WORD-WMQ 32"; // Index 3 is SPK_CUSTOM_CMD (Preloaded 0, 1, 2, then Custom at 3)
    status = tsfi_cli_process_line(ws, cmd_alter);
    assert(status == 0);

    printf("[TEST] Speaking SPK_CUSTOM_CMD again to confirm memory alteration (should now show WMQ: 0x20)...\n");
    status = tsfi_cli_process_line(ws, cmd_speak);
    assert(status == 0);
    // 5. Test pre-dispatch strategy validation
    printf("[TEST] Dispatching WORD compile directive for strategically invalid SPK_INVALID_CMD...\n");
    char cmd_invalid_word[128] = "WORD SPK_INVALID_CMD 6 500.0 500.0 0.3 00 02"; // f1 == f2 is invalid
    status = tsfi_cli_process_line(ws, cmd_invalid_word);
    assert(status == 0);

    printf("[TEST] Speaking invalid SPK_INVALID_CMD (should be rejected by strategy rules)...\n");
    char cmd_invalid_speak[128] = "SPEAK 500.0 500.0 0.3 6";
    status = tsfi_cli_process_line(ws, cmd_invalid_speak);
    assert(status == 1); // Confirms pre-dispatch strategy check blocked execution
    // 6. Test lexicon compiler auto-derivation
    printf("[TEST] Dispatching LEXICON_REGISTER for WMQ_AUTO_LOCK...\n");
    char cmd_lex_register[128] = "LEXICON_REGISTER WMQ_AUTO_LOCK 7 10 00"; // ID 7 -> F1 = 650Hz, F2 = 1300Hz, decay = 0.4s
    status = tsfi_cli_process_line(ws, cmd_lex_register);
    assert(status == 0);

    printf("[TEST] Speaking derived WMQ_AUTO_LOCK (should succeed)...\n");
    char cmd_lex_speak[128] = "SPEAK 650.0 1300.0 0.4 7";
    status = tsfi_cli_process_line(ws, cmd_lex_speak);
    assert(status == 0);
    // 7. Test atomic sentence execution using LEXICON_EXECUTE
    printf("[TEST] Dispatching LEXICON_EXECUTE sentence (SPK_LOCK_SCSI SPK_WRITE_LEDGER)...\n");
    char cmd_lex_execute[128] = "LEXICON_EXECUTE SPK_LOCK_SCSI SPK_WRITE_LEDGER";
    status = tsfi_cli_process_line(ws, cmd_lex_execute);
    assert(status == 0);
    // 8. Test lexicon exporter and Rule 13 extension constraint
    printf("[TEST] Dispatching LEXICON_EXPORT with invalid extension .json (should fail)...\n");
    char cmd_lex_exp_fail[128] = "LEXICON_EXPORT target_dictionary.json";
    status = tsfi_cli_process_line(ws, cmd_lex_exp_fail);
    assert(status == 1); // Confirms non-.dat.bin file extension is rejected

    printf("[TEST] Dispatching LEXICON_EXPORT with valid .dat.bin extension (version 2)...\n");
    char cmd_lex_exp_pass[128] = "LEXICON_EXPORT target_dictionary.dat.bin 2";
    status = tsfi_cli_process_line(ws, cmd_lex_exp_pass);
    assert(status == 0);


    printf("[TEST] Dispatching LEXICON_IMPORT with invalid extension .json (should fail)...\n");
    char cmd_lex_imp_fail[128] = "LEXICON_IMPORT target_dictionary.json";
    status = tsfi_cli_process_line(ws, cmd_lex_imp_fail);
    assert(status == 1); // Confirms non-.dat.bin file extension is rejected

    printf("[TEST] Dispatching LEXICON_IMPORT with valid .dat.bin extension...\n");
    char cmd_lex_imp_pass[128] = "LEXICON_IMPORT target_dictionary.dat.bin";
    status = tsfi_cli_process_line(ws, cmd_lex_imp_pass);
    assert(status == 0);

    printf("[TEST] Dispatching LEXICON_LIST to print active vocabulary...\n");
    char cmd_lex_list[128] = "LEXICON_LIST";
    status = tsfi_cli_process_line(ws, cmd_lex_list);
    assert(status == 0);

    printf("[TEST] Dispatching LEXICON_AUDIT to check vocabulary compliance...\n");
    char cmd_lex_audit[128] = "LEXICON_AUDIT";
    status = tsfi_cli_process_line(ws, cmd_lex_audit);
    assert(status == 0);


    remove("target_dictionary.dat.bin"); // Clean up generated file

    // 9. Test Python dictionary generator integration
    printf("[TEST] Running generate_lexicon_db.py to compile binary dictionary...\n");
    int py_ret = system("python3 ../tools/generate_lexicon_db.py -o py_dictionary.dat.bin --words WMQ_PYTHON_LOCK,8,10,00 ABI_PYTHON_WRITE,9,00,02");
    assert(py_ret == 0);

    printf("[TEST] Importing Python-compiled dictionary...\n");
    char cmd_py_imp[128] = "LEXICON_IMPORT py_dictionary.dat.bin";
    status = tsfi_cli_process_line(ws, cmd_py_imp);
    assert(status == 0);

    printf("[TEST] Speaking Python-derived word (F1 = 300 + 400 = 700Hz)...\n");
    char cmd_py_speak[128] = "SPEAK 700.0 1400.0 0.4 8";
    status = tsfi_cli_process_line(ws, cmd_py_speak);

    remove("py_dictionary.dat.bin");

    // 10. Test lexicon startup auto-load bootstrap via sub-process
    printf("[TEST] Verifying lexicon_default.dat.bin auto-load bootstrap...\n");
    int bootstrap_gen = system("python3 ../tools/generate_lexicon_db.py -o lexicon_default.dat.bin --words WMQ_BOOTSTRAP,42,10,00");
    assert(bootstrap_gen == 0);

    int sub_run = system("echo \"SPEAK 300.0 600.0 0.4 42\" | ../tsfi2 > /dev/null");
    assert(sub_run == 0); // Exits 0 if WMQ_BOOTSTRAP was successfully auto-loaded and spoken
    remove("lexicon_default.dat.bin");







    printf("   ✓ Integrated CLI commands executed and verified successfully.\n");
    printf("=============================================================\n");
    printf("INTEGRATED CLI TESTS COMPLETED\n");
    printf("=============================================================\n");

    return 0;
}
