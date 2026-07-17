#ifndef TSFI_MAJORMUD_H
#define TSFI_MAJORMUD_H

#include <stdint.h>

int tsfi_mf_zmachine_majormud_init(char *mud_state, int max_len);
int tsfi_mf_zmachine_majormud_command(const char *cmd, char *mud_state, char *response_out, int max_len);

#endif // TSFI_MAJORMUD_H
