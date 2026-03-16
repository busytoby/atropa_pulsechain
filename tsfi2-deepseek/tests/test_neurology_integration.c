#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include "tsfi_svdag.h"
#include "tsfi_font_ai.h"
#include "tsfi_wire_firmware.h"
#include "lau_memory.h"

// Stub for Font AI context
typedef struct {
    void *dummy;
} MockSys;

void stub_evolve(void *ctx, float intensity) {
    (void)ctx; (void)intensity;
}

int main() {
    printf("=== Test Neurology Integration (Main Project) ===\n");
    
    // 1. SVDAG
    printf("[1] SVDAG\n");
    TSFiHelmholtzSVDAG *dag = tsfi_svdag_create(1024);
    assert(dag);
    // Real compile will run on empty glyph
    TSFiHilbertGlyph glyph;
    memset(&glyph, 0, sizeof(glyph));
    tsfi_svdag_compile(dag, &glyph);
    float mass = tsfi_svdag_execute(dag);
    printf("  Mass: %f\n", mass);
    tsfi_svdag_destroy(dag);
    
    // 2. Font AI
    printf("[2] Font AI\n");
    TSFiFontSystem *sys = (TSFiFontSystem*)lau_malloc(sizeof(TSFiFontSystem));
    memset(sys, 0, sizeof(TSFiFontSystem));
    tsfi_font_ai_init(sys);
    tsfi_font_ai_bind_evolve(sys, (void*)stub_evolve, NULL);
    tsfi_font_ai_invoke_evolve(sys, 0.5f);
    tsfi_font_ai_destroy(sys);
    lau_free(sys);

    // 3. Neural Observation (Log Scraping)
    printf("[3] Neural Observation Wavefront\n");
    tsfi_wire_firmware_init();
    LauWireFirmware *fw = tsfi_wire_firmware_get();
    LauWireLog *wl = &fw->wavefront.stdout_log;

    char intent_out[256] = {0};
    char insight_out[1024] = {0};

    const char *sample_data = "Thinking... [INTENT] OPTIMIZE_ZMM_REGS\nMore analysis follows.\n[INSIGHT] Alignment is 16-byte restricted.\nDone.";
    memcpy(wl->buffer, sample_data, strlen(sample_data));
    *wl->head = strlen(sample_data);

    fw->cell_neural_observe(0, intent_out, insight_out);

    assert(strcmp(intent_out, "OPTIMIZE_ZMM_REGS") == 0);
    assert(strcmp(insight_out, "Alignment is 16-byte restricted.") == 0);
    assert(*wl->tail == 0); // Non-destructive sampling
    printf("  Intent: %s\n", intent_out);
    printf("  Insight: %s\n", insight_out);
    
    extern void tsfi_wire_firmware_teardown(void);
    tsfi_wire_firmware_teardown();
    
    printf("\n=== Integration Test Passed ===\n");

    extern void tsfi_svdag_teardown(void);
    tsfi_svdag_teardown();

    extern void lau_registry_teardown(void);
    lau_registry_teardown();
    
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}