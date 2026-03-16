#include "tsfi_helmholtz.h"
#include "tsfi_svdag.h"
#include "tsfi_wire_firmware.h"
#include "lau_telemetry.h"
#include "tsfi_pool.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

int g_tsfi_verbose = 0;

void* lau_malloc_loc(size_t size, const char *f, int l) { (void)f;(void)l; return malloc(size); }
void* lau_memalign_loc(size_t a, size_t s, const char *f, int l) { (void)f;(void)l; void *p; posix_memalign(&p, a, s); return p; }
void lau_free(void* p) { free(p); }
void lau_report_memory_metrics(void) {}
void lau_assert_zero_unsealed_leaks(const char *c, void *p) { (void)c;(void)p; }
void* lau_malloc_wired_loc(size_t s, const char *f, int l) { (void)f;(void)l; return malloc(s); }
void lau_seal_object_loc(void *p, const char *f, int l) { (void)p;(void)f;(void)l; }
void ThunkProxy_destroy_authoritative(ThunkProxy *p) { (void)p; }
ThunkProxy* ThunkProxy_create(void) { return NULL; }
void* ThunkProxy_emit_io_poll(ThunkProxy *p, int fd) { (void)p;(void)fd; return NULL; }
void ThunkProxy_seal(ThunkProxy *p) { (void)p; }
unsigned long long get_time_ns(void) { return 0; }
LauWireFirmware* tsfi_wire_firmware_get_no_init(void) { return NULL; }
LauWireFirmware* tsfi_wire_firmware_get(void) { return NULL; }
void tsfi_wire_firmware_load_waveform(LauWireFirmware *fw, int i, void *w) { (void)fw;(void)i;(void)w; }
LauTelemetryState* tsfi_telemetry_get(void) { return NULL; }
void lau_telemetry_integrity_update(LauTelemetryState *s) { (void)s; }
void lau_telemetry_report_event(LauTelemetryState *s, uint64_t p, uint64_t sz, uint32_t t, uint32_t f) { (void)s;(void)p;(void)sz;(void)t;(void)f; }
void lau_audit_init(void) {}
void lau_audit_record(void *p, size_t s, const char *c, int l) { (void)p;(void)s;(void)c;(void)l; }
void* ThunkProxy_emit_baked(ThunkProxy *p, void *f, int c, ...) { (void)p;(void)f;(void)c; return NULL; }
float tsfi_svdag_execute(const TSFiHelmholtzSVDAG *d) { (void)d; return 0.0f; }
void thunk_check_bounds(ThunkProxy *p, size_t s) { (void)p;(void)s; }
void tsfi_hilbert_init_glyph(TSFiHilbertGlyph *g) { (void)g; }
void tsfi_hilbert_project_box(TSFiHilbertGlyph *g, float w, float h) { (void)g;(void)w;(void)h; }
float tsfi_fabsf(float x) { return fabsf(x); }
float tsfi_sqrtf(float x) { return sqrtf(x); }
void tsfi_pool_init(void) {}
void tsfi_pool_teardown(void) {}
TSFiBigInt* tsfi_pool_acquire_bn(void) { return (TSFiBigInt*)calloc(1, sizeof(TSFiBigInt)); }
void tsfi_pool_release_bn(TSFiBigInt *b) { free(b); }
LauMetadata* lau_registry_find(void *p) { (void)p; return NULL; }
_Atomic int g_init_in_progress = 0;
_Atomic int g_teardown_in_progress = 0;
