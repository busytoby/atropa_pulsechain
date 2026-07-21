#ifndef TSFI_PARC_RUNCIBLE_LANG_H
#define TSFI_PARC_RUNCIBLE_LANG_H

#include <stdint.h>
#include <stddef.h>

typedef enum {
    RUNCIBLE_CMD_UNKNOWN = 0,
    RUNCIBLE_CMD_CALL,
    RUNCIBLE_CMD_KB,
    RUNCIBLE_CMD_SLIDE,
    RUNCIBLE_CMD_STATUS
} tsfi_runcible_cmd_type_t;

typedef struct {
    tsfi_runcible_cmd_type_t type;
    uint32_t target_addr;
    uint8_t kb_row;
    uint8_t kb_col;
    uint8_t kb_shift;
    char raw_args[64];
} tsfi_runcible_cmd_t;

/* Parse a TTY command line string into a Runcible command structure */
int tsfi_runcible_parse_cmd(const char *cmd_line, tsfi_runcible_cmd_t *out_cmd);

/* Execute a parsed Runcible command directly against VM registers / TTY state */
int tsfi_runcible_exec_cmd(const tsfi_runcible_cmd_t *cmd);

/* Main interactive TTY Runcible evaluation step called from main loop */
int tsfi_runcible_main_step(const char *tty_input);

#endif // TSFI_PARC_RUNCIBLE_LANG_H
