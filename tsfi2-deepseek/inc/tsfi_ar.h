#ifndef TSFI_AR_H
#define TSFI_AR_H

#include <stdint.h>
#include <stdbool.h>

// Asset Resolver (Ar) context
typedef struct {
    char resolved_path[128];
} TSFiAssetResolver;

// Initialize the asset resolver context
void tsfi_ar_init(TSFiAssetResolver *ar);

// Resolves custom asset URI (e.g. auncient://mesh) to a local file with only .dat.bin extension
bool tsfi_ar_resolve(TSFiAssetResolver *ar, const char *uri, char *out_path);

// Integrates directly with XCOM, updating the receiver gate status
void tsfi_ar_integrate_xcom(bool resolve_ok, void *xcom_rx_ptr);

#endif // TSFI_AR_H
