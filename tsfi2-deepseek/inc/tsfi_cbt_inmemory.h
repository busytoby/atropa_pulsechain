#ifndef TSFI_CBT_INMEMORY_H
#define TSFI_CBT_INMEMORY_H

#include <stddef.h>
#include <stdbool.h>
#include "tsfi_xplos_kernel.h"

/* Retrieve a CBT Tape zip file via HTTPS/HTTP into memory in C and mount it as a PDS */
bool tsfi_cbt_mount_inmemory_pds(XplosVirtualDisk *vfs, const char *server_path);

#endif
