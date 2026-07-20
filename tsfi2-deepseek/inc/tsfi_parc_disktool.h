#ifndef TSFI_PARC_DISKTOOL_H
#define TSFI_PARC_DISKTOOL_H

typedef struct {
    int cylinders;
    int heads;
    int sectors;
    int is_formatted;
} tsfi_parc_disktool_t;

/* Initialize disk formatter tool */
void tsfi_parc_disktool_init(tsfi_parc_disktool_t *tool);

/* Format emulated Diablo drive sectors, establishing volume headers */
int tsfi_parc_disktool_format(tsfi_parc_disktool_t *tool, const char *vol_name);

#endif // TSFI_PARC_DISKTOOL_H
