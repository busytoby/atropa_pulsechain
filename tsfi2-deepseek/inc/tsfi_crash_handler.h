#ifndef TSFI_CRASH_HANDLER_H
#define TSFI_CRASH_HANDLER_H

#include <stdint.h>
#include <stdatomic.h>

void tsfi_install_crash_handler();
void tsfi_register_fault_flag(_Atomic uint32_t *flag);

#endif
