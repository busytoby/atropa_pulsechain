#ifndef TSFI_PARC_RUNCIBLE_LANG_H
#define TSFI_PARC_RUNCIBLE_LANG_H

#include <stdint.h>
#include <stddef.h>

typedef enum {
    RUNCIBLE_CMD_UNKNOWN = 0,
    RUNCIBLE_CMD_CALL,
    RUNCIBLE_CMD_KB,
    RUNCIBLE_CMD_SLIDE,
    RUNCIBLE_CMD_STATUS,
    RUNCIBLE_CMD_SCSI,
    RUNCIBLE_CMD_KEYSET,
    RUNCIBLE_CMD_SCHUR,
    RUNCIBLE_CMD_PEEK,
    RUNCIBLE_CMD_POKE,
    RUNCIBLE_CMD_YUL,
    RUNCIBLE_CMD_MACRO,
    RUNCIBLE_CMD_DISCHARGE,
    RUNCIBLE_CMD_PUP,
    RUNCIBLE_CMD_CLIP,
    RUNCIBLE_CMD_ST,
    RUNCIBLE_CMD_VOID,
    RUNCIBLE_CMD_TAPE_CHECK,
    RUNCIBLE_CMD_TAPE_LABEL,
    RUNCIBLE_CMD_TAPE_CATALOG,
    RUNCIBLE_CMD_TAPE_BOUNDS,
    RUNCIBLE_CMD_TAPE_PHASE,
    RUNCIBLE_CMD_TAPE_PROOF
} tsfi_runcible_cmd_type_t;

typedef struct {
    tsfi_runcible_cmd_type_t type;
    uint32_t target_addr;
    uint32_t val;
    uint8_t kb_row;
    uint8_t kb_col;
    uint8_t kb_shift;
    uint8_t scsi_op; // 0=READ, 1=WRITE
    uint8_t scsi_reg;
    uint32_t scsi_val;
    uint8_t keyset_chord;
    uint8_t dest_net;
    uint8_t dest_host;
    uint8_t pup_type;
    float dt;
    int fet_id;
    char sub_op[16];
    char raw_args[64];
} tsfi_runcible_cmd_t;

/* Parse a TTY command line string into a Runcible command structure */
int tsfi_runcible_parse_cmd(const char *cmd_line, tsfi_runcible_cmd_t *out_cmd);

/* Execute a parsed Runcible command directly against VM registers / TTY state */
int tsfi_runcible_exec_cmd(const tsfi_runcible_cmd_t *cmd);

/* Main interactive TTY Runcible evaluation step called from main loop */
int tsfi_runcible_main_step(const char *tty_input);

#endif // TSFI_PARC_RUNCIBLE_LANG_H
