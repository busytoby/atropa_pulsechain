#ifndef TSFI_XPLOS_SHELL_CBT_VTAM_H
#define TSFI_XPLOS_SHELL_CBT_VTAM_H

#include <stdbool.h>

bool tsfi_xplos_shell_cbt_vtam(const char *cmd);
void tsfi_vtam_coax_write_buffer(const char *data);
void tsfi_vtam_coax_read_buffer(char *dest, int max_len);

#endif
