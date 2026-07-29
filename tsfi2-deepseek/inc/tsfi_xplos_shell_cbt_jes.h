#ifndef TSFI_XPLOS_SHELL_CBT_JES_H
#define TSFI_XPLOS_SHELL_CBT_JES_H

#include <stdbool.h>

bool tsfi_xplos_shell_cbt_jes(const char *cmd);

#endif
extern char g_hasp_spool_logs[10][4096];
extern int g_hasp_job_priority[10];
void append_spool_log(const char *job_name, const char *msg);
void hasp_dispatch_highest_priority(void);
