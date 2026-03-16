#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdatomic.h>
#include "tsfi_hotloader.h"
#include "tsfi_io.h"
#include "tsfi_zhao.h"

void mock_thunk_target() {
    printf("[MOCK] Initial thunk logic.\n");
}

int main() {
    TSFiLogicTable table;
    const char *src = "plugins/minimal_plugin.c";
    const char *so = "tests/minimal_plugin.so";

    // Initialize the background timeline for tsfi_zhao_submit (1 thread for testing)
    tsfi_zhao_init(1);

    // Generate a temporary mock thunk to prove hot-recompilation paths
    const char *mock_thunk_src = "tests/mock_hot_thunk.c";
    FILE* f = fopen(mock_thunk_src, "w");
    if (f) {
        fprintf(f, "#include <stdio.h>\nvoid test_hot_thunk_entry() { printf(\"[HOT] Upgraded logic executed!\\n\"); }\n");
        fclose(f);
    }

    tsfi_io_log(stdout, TSFI_LOG_INFO, "HOTLOADER_TEST", "Evaluating Async Hot-Thunk Polling...");
    _Atomic(void*) active_thunk_ptr = (void*)mock_thunk_target;
    
    // This simulates what a plugin does: it tells the TSFi system to background-compile a thunk and swap the pointer when ready
    tsfi_hotreload_poll_async(mock_thunk_src, "test_hot_thunk_entry", &active_thunk_ptr);
    
    // Give the background thread time to compile
    sleep(1);

    // Give time for swap via thread background loop since it uses normal pthread background processing
    // Sleep previously did this, so we just wait a tiny bit more if needed.
    
    // Verify the thunk pointer was successfully swapped autonomously
    void (*executed_thunk)() = (void(*)())atomic_load(&active_thunk_ptr);
    if (executed_thunk != mock_thunk_target) {
        tsfi_io_log(stdout, TSFI_LOG_INFO, "HOTLOADER_TEST", "Success: async thunk pointer autonomously swapped");
        executed_thunk(); // Prove it executes
    } else {
        tsfi_io_log(stdout, TSFI_LOG_FAIL, "HOTLOADER_TEST", "Failure: async thunk pointer not swapped");
        return 1;
    }

    // Now modify the file again to test the persistent polling loop
    tsfi_io_log(stdout, TSFI_LOG_INFO, "HOTLOADER_TEST", "Modifying the thunk source to trigger a persistent reload...");
    f = fopen(mock_thunk_src, "w");
    if (f) {
        fprintf(f, "#include <stdio.h>\nvoid test_hot_thunk_entry() { printf(\"[HOT] 2nd Generation Logic Executed!\\n\"); }\n");
        fclose(f);
    }
    
    // Wait for the poll loop to detect, recompile, and swap
    sleep(2);
    
    void (*executed_thunk_gen2)() = (void(*)())atomic_load(&active_thunk_ptr);
    if (executed_thunk_gen2 != executed_thunk && executed_thunk_gen2 != mock_thunk_target) {
        tsfi_io_log(stdout, TSFI_LOG_INFO, "HOTLOADER_TEST", "Success: Gen2 async thunk pointer autonomously swapped");
        executed_thunk_gen2(); // Prove the new one executes
    } else {
        tsfi_io_log(stdout, TSFI_LOG_FAIL, "HOTLOADER_TEST", "Failure: Gen2 async thunk pointer not swapped");
        return 1;
    }

    tsfi_io_log(stdout, TSFI_LOG_INFO, "HOTLOADER_TEST", "Compiling plugin...");
    if (tsfi_compile_plugin(src, so) != 0) {
        tsfi_io_log(stdout, TSFI_LOG_FAIL, "HOTLOADER_TEST", "Compilation failed");
        return 1;
    }

    tsfi_io_log(stdout, TSFI_LOG_INFO, "HOTLOADER_TEST", "Loading plugin...");
    if (tsfi_load_plugin(so, &table) != 0) {
        tsfi_io_log(stdout, TSFI_LOG_FAIL, "HOTLOADER_TEST", "Loading failed");
        return 1;
    }

    tsfi_io_log(stdout, TSFI_LOG_INFO, "HOTLOADER_TEST", "Verifying logic execution...");
    int version = 42;
    table.logic_epoch(&version);
    if (version == 43) {
        tsfi_io_log(stdout, TSFI_LOG_INFO, "HOTLOADER_TEST", "Success: version incremented to %d", version);
    } else {
        tsfi_io_log(stdout, TSFI_LOG_FAIL, "HOTLOADER_TEST", "Failure: version is %d, expected 43", version);
        return 1;
    }

    remove(so);
    remove(mock_thunk_src);
    
    // Physically decompose tracking boundaries
    extern void tsfi_dl_thunks_teardown(void);
    extern void lau_registry_teardown(void);
    tsfi_dl_thunks_teardown();
    tsfi_zhao_shutdown();
    lau_registry_teardown();
    
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    
    
    extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
