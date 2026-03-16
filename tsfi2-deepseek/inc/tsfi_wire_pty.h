#ifndef TSFI_WIRE_PTY_H
#define TSFI_WIRE_PTY_H

#include "tsfi_types.h"
#include "lau_thunk.h"
#include "lau_wire_log.h"
#include <unistd.h>
#include <stdbool.h>
#include <stdatomic.h>
#include <stdarg.h>

DEFINE_MAPPED_STRUCT(LauWirePty,
    int master_fd;
    int slave_fd;
    pid_t child_pid;
    char pts_name[256];
    bool is_active;

    // Direct Wavefront logs
    LauWireLog log_stdin;
    LauWireLog log_stdout;
)

// Custom implementation of forkpty using Thunks and Helmholtz logic
LauWirePty* tsfi_wire_pty_fork(const char *cmd);

// Custom printf that routes directly to the PTY master or stdout based on context
int tsfi_printf(LauWirePty *pty, const char *format, ...);

// Send input to the PTY
ssize_t tsfi_wire_pty_write(LauWirePty *pty, const char *buf, size_t n);

// Evaluate PTY status and perform reads (The Helmholtz Step)
void tsfi_wire_pty_step(LauWirePty *pty);

#endif // TSFI_WIRE_PTY_H
