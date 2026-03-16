#ifndef TSFI_ELF_REFLECT_H
#define TSFI_ELF_REFLECT_H

#include <stdint.h>
#include <stddef.h>

// Minimal equivalent of Dl_info from <dlfcn.h>
typedef struct {
    const char *dli_fname;  /* Pathname of shared object that contains address */
    void       *dli_fbase;  /* Base address at which shared object is loaded */
    const char *dli_sname;  /* Name of symbol whose definition overlaps addr */
    void       *dli_saddr;  /* Exact address of symbol named in dli_sname */
} TSFiDlInfo;

// Internal reflection function to replace dladdr
int tsfi_raw_dladdr(void *addr, TSFiDlInfo *info);

#endif // TSFI_ELF_REFLECT_H