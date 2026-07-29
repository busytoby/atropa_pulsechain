#ifndef TSFI_XPLOS_KERNEL_INTERNAL_H
#define TSFI_XPLOS_KERNEL_INTERNAL_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "tsfi_xplos_kernel.h"

// Contextual structure definitions
typedef struct {
    char job_id[16];
    char job_name[16];
    char status[16];
    char class_char;
    bool active;
    bool cics_origin;
} CbtSpoolJob;

typedef struct {
    char job_name[16];
    char step_name[16];
    char program[16];
    bool active;
} CbtSpoolStep;

typedef struct CbtTreeNode {
    int keys[2];
    int num_keys;
    struct CbtTreeNode *children[3];
    CbtSpoolJob queue[10];
    int queue_count;
    int worker_task_id;
} CbtTreeNode;

typedef struct {
    uint16_t record_len;
    uint16_t segment_desc;
    uint8_t system_indicator;
    uint8_t record_type;
    uint32_t time_hundredths;
    uint32_t date_packed;
    char system_id[4];
} __attribute__((packed)) SmfRecordHeader;

// Shared internal function prototypes
void cbt_node_worker_task_handler(void *arg);
void resolve_pds_name(const char *input, char *output, size_t max_len);
uint32_t fnv1a_hash(const uint8_t *data, size_t len, uint32_t seed);
void shell_task_handler(void *arg);
bool tsfi_cbt_mount_inmemory_pds(XplosVirtualDisk *disk, const char *path);

bool tsfi_xplos_shell_legacy(const char *cmd);
bool tsfi_xplos_shell_tape(const char *cmd);
bool tsfi_xplos_shell_vsam(const char *cmd);
bool tsfi_xplos_shell_explorer(const char *cmd);
bool tsfi_xplos_shell_book(const char *cmd);
bool tsfi_xplos_shell_xdc(const char *cmd);

#endif
