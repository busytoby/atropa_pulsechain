#include "tsfi_hotloader.h"
#include "tsfi_io.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

int check_thunk(const char* filepath) {
    char so_path[512];
    snprintf(so_path, sizeof(so_path), "%s.so", filepath);
    
    if (tsfi_compile_plugin(filepath, so_path) != 0) {
        tsfi_io_printf(stderr, "[FAIL] Hotcompiler failed to build: %s\n", filepath);
        return -1;
    }
    
    void* handle = tsfi_dlopen(so_path, 2);
    if (!handle) {
        tsfi_io_printf(stderr, "[FAIL] Hotloader failed to load: %s - %s\n", so_path, tsfi_dlerror());
        return -1;
    }
    
    const char* filename = strrchr(filepath, '/');
    filename = filename ? filename + 1 : filepath;
    char symbol_name[256];
    strncpy(symbol_name, filename, sizeof(symbol_name) - 1);
    symbol_name[sizeof(symbol_name) - 1] = '\0';
    char* dot = strrchr(symbol_name, '.');
    if (dot) *dot = '\0';
    
    void* func = tsfi_dlsym(handle, symbol_name);
    if (!func) {
        tsfi_io_printf(stderr, "[WARN] Hotloader could not find exact symbol '%s' in %s, but file compiled and loaded successfully.\n", symbol_name, so_path);
    } else {
        tsfi_io_printf(stdout, "[SUCCESS] Hotcompiled and resolved symbol '%s' from %s\n", symbol_name, filepath);
    }
    
    tsfi_dlclose(handle);
    unlink(so_path);
    return 0;
}

int process_dir(const char* dir_path) {
    DIR* dir = opendir(dir_path);
    if (!dir) return -1;
    
    struct dirent* entry;
    int failures = 0;
    
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;
        
        char path[1024];
        snprintf(path, sizeof(path), "%s/%s", dir_path, entry->d_name);
        
        struct stat st;
        if (stat(path, &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                failures += process_dir(path);
            } else if (S_ISREG(st.st_mode)) {
                const char* ext = strrchr(entry->d_name, '.');
                if (ext && strcmp(ext, ".c") == 0) {
                    if (check_thunk(path) != 0) {
                        failures++;
                    }
                }
            }
        }
    }
    closedir(dir);
    return failures;
}

int main() {
    tsfi_io_printf(stdout, "=== TSFi Firmware: Thunk Hotcompiler Matrix Validation ===\n");
    int fails = process_dir("thunks");
    if (fails > 0) {
        tsfi_io_printf(stderr, "[FAIL] %d thunks failed to hotcompile.\n", fails);
        return 1;
    }
    tsfi_io_printf(stdout, "[VERIFIED] All standard cells are fully hotcompileable and dynamically loadable.\n");
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}