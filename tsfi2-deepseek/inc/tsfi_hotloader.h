#ifndef TSFI_HOTLOADER_H
#define TSFI_HOTLOADER_H

#include "tsfi_plugin.h"

// Compiles a C source file into a shared object
// Returns 0 on success, non-zero on failure.
int tsfi_compile_plugin(const char *source_path, const char *output_so);

// Loads a plugin from a shared object file.
// Populates the table with pointers.
// Returns 0 on success.
#include <stdatomic.h>
void tsfi_hotreload_poll_async(const char* thunk_path, const char* symbol_name, _Atomic(void*)* target_ptr);
void* tsfi_hotload_thunk(const char* thunk_path, const char* symbol_name);
int tsfi_load_plugin(const char *so_path, TSFiLogicTable *table);

void* tsfi_dlopen(const char *filename, int flags);
void* tsfi_dlsym(void *handle, const char *symbol);
char* tsfi_dlerror(void);
int   tsfi_dlclose(void *handle);

#endif // TSFI_HOTLOADER_H
