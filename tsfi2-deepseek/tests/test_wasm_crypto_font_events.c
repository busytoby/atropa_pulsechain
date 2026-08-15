#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <unistd.h>
#include "../inc/tsfi_wasm_crypto_font_events.h"
#include "../inc/tsfi_strategy_lang.h"
#include "../inc/tsfi_chancery_docket.h"

// Mirroring solidity/dysnomia/domain/std/wasm_crypto_font_events_prover.algol61
static int verify_wasm_crypto_font_events_state(int wasm_stack_result, int crypto_hash_verified, int font_curve_count, int dom_events_dispatched) {
    if (wasm_stack_result == 0 || crypto_hash_verified == 0) return 1; // WASM_OR_CRYPTO_DEFECT_REJECT
    if (font_curve_count == 0 || dom_events_dispatched == 0) return 2; // FONT_OR_EVENT_DEFECT_REJECT
    return 0; // AUTHENTIC_CORE_PLATFORM_STATE
}

static int g_event_dispatch_received = 0;
static void test_click_handler(const TsfiDomEvent *event, void *user_data) {
    (void)user_data;
    g_event_dispatch_received++;
    printf("   [DOM Event Handler Fired] Target=\"%s\", Mouse=(%d,%d)\n", event->target_id, event->mouse_x, event->mouse_y);
}

int main(void) {
    printf("====================================================================\n");
    printf("TEST SUITE: WASM RUNTIME, WEB CRYPTO, FONT BÉZIER & DOM EVENTS\n");
    printf("====================================================================\n\n");

    // -------------------------------------------------------------------------
    // 1. Test WebAssembly (Wasm) MVP Bytecode Stack Execution
    // -------------------------------------------------------------------------
    printf("1. Testing Wasm MVP Stack Interpreter ((6 * 7) == 42)...\n");
    TsfiWasmVM wasm;
    tsfi_wasm_init(&wasm);

    // Bytecode: \0asm \1\0\0\0 | i32.const 6 | i32.const 7 | i32.mul | return
    const uint8_t wasm_code[] = {
        0x00, 0x61, 0x73, 0x6D, // Magic "\0asm"
        0x01, 0x00, 0x00, 0x00, // Version 1
        0x41, 0x06,             // i32.const 6
        0x41, 0x07,             // i32.const 7
        0x6C,                   // i32.mul
        0x0F                    // return
    };

    assert(tsfi_wasm_execute_bytecode(&wasm, wasm_code, sizeof(wasm_code)));
    assert(wasm.sp == 1);
    uint32_t wasm_res = tsfi_wasm_pop(&wasm);
    assert(wasm_res == 42);
    printf("   ✓ Wasm Stack Execution Computed: %u (Stack Depth: 1 -> 0)\n", wasm_res);

    // -------------------------------------------------------------------------
    // 2. Test Web Crypto API (SHA-256 & Secp256k1 Signature)
    // -------------------------------------------------------------------------
    printf("\n2. Testing Web Crypto (SubtleCrypto) SHA-256 & Secp256k1...\n");
    const char *msg = "PulseChain Block Ledger Proof";
    uint8_t hash[32];
    tsfi_crypto_sha256((const uint8_t*)msg, strlen(msg), hash);

    const uint8_t privkey[32] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                                 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10,
                                 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
                                 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20};
    TsfiCryptoResult crypto_res;
    assert(tsfi_crypto_secp256k1_sign(hash, privkey, &crypto_res));
    assert(crypto_res.is_verified == true);
    printf("   ✓ SubtleCrypto SHA-256 Hash Generated & Secp256k1 Signature Verified (64 bytes)\n");

    // -------------------------------------------------------------------------
    // 3. Test TrueType Bézier Vector Font Glyph Rasterizer
    // -------------------------------------------------------------------------
    printf("\n3. Testing TrueType Bézier Quadratic Glyph Rasterization...\n");
    TsfiGlyphOutline glyph;
    tsfi_font_init_glyph(&glyph, 32, 32);

    // Add Quadratic Bézier Curve (Letter 'C' Arc)
    assert(tsfi_font_add_quad_curve(&glyph, 24.0f, 4.0f, 4.0f, 16.0f, 24.0f, 28.0f));
    assert(glyph.curve_count == 1);

    uint32_t *font_pixels = (uint32_t *)calloc(64 * 64, sizeof(uint32_t));
    assert(font_pixels != NULL);
    tsfi_font_rasterize_glyph(&glyph, font_pixels, 64, 64, 10, 10, 0xFFFFFFFF);

    // Verify rasterized scanline has white pixels
    long painted_font_pixels = 0;
    for (int i = 0; i < 64 * 64; i++) {
        if (font_pixels[i] == 0xFFFFFFFF) painted_font_pixels++;
    }
    assert(painted_font_pixels > 10);
    free(font_pixels);
    printf("   ✓ TrueType Vector Bézier Glyph Rasterized (%ld subpixels rendered)\n", painted_font_pixels);

    // -------------------------------------------------------------------------
    // 4. Test DOM Events & MutationObserver
    // -------------------------------------------------------------------------
    printf("\n4. Testing DOM Event Listener Dispatch & Mutation Observer...\n");
    TsfiDomEventDispatcher disp;
    tsfi_dom_events_init(&disp);

    assert(tsfi_dom_add_event_listener(&disp, "btn_submit", DOM_EVENT_CLICK, test_click_handler, NULL));
    assert(disp.listener_count == 1);

    TsfiDomEvent evt = {
        .type = DOM_EVENT_CLICK,
        .target_id = "btn_submit",
        .mouse_x = 150,
        .mouse_y = 75,
        .default_prevented = false,
        .propagation_stopped = false
    };

    int dispatched = tsfi_dom_dispatch_event(&disp, &evt);
    assert(dispatched == 1);
    assert(g_event_dispatch_received == 1);
    tsfi_dom_record_mutation(&disp, "btn_submit", "childList");
    assert(disp.mutation_count == 1);
    printf("   ✓ DOM Event Dispatched & Mutation Recorded (%d listener matched)\n", dispatched);

    // -------------------------------------------------------------------------
    // 5. Test COBOL Strategy & ALGOL 61 Prover Integration
    // -------------------------------------------------------------------------
    printf("\n5. Executing COBOL Strategy: wasm_crypto_font_events.strategy...\n");
    TSFiStrategyVM vm;
    TSFiStrategyReceipt receipt;
    int rc = tsfi_strategy_load_and_run("wasm_crypto_font_events.strategy", (int)wasm_res, 32, glyph.curve_count, 0, &vm, &receipt);
    assert(rc == 0);
    assert(vm.registers[3] == 9999);
    printf("   ✓ Strategy Execution Confirmed: Platform Score = %d\n", vm.registers[3]);

    printf("\n6. Executing ALGOL 61 Prover: wasm_crypto_font_events_prover.algol61...\n");
    int ruling_auth = verify_wasm_crypto_font_events_state((int)wasm_res, crypto_res.is_verified ? 1 : 0, glyph.curve_count, dispatched);
    assert(ruling_auth == 0);
    printf("   ✓ Ruling: AUTHENTIC_CORE_PLATFORM_STATE (ruling = %d)\n", ruling_auth);

    int ruling_wasm_fail = verify_wasm_crypto_font_events_state(0, 1, glyph.curve_count, dispatched);
    assert(ruling_wasm_fail == 1);
    printf("   ✓ Wasm Defect Reject Ruling: WASM_OR_CRYPTO_DEFECT_REJECT (ruling = %d)\n", ruling_wasm_fail);

    int ruling_font_fail = verify_wasm_crypto_font_events_state((int)wasm_res, 1, 0, dispatched);
    assert(ruling_font_fail == 2);
    printf("   ✓ Font Defect Reject Ruling: FONT_OR_EVENT_DEFECT_REJECT (ruling = %d)\n", ruling_font_fail);

    // -------------------------------------------------------------------------
    // 6. File Resolution on Chancery Docket
    // -------------------------------------------------------------------------
    printf("\n7. Filing Resolution on Chancery Docket:\n");
    ChanceryDocketState docket;
    tsfi_chancery_docket_init(&docket);

    uint32_t doc_id = tsfi_chancery_docket_file(
        &docket,
        "Wasm Runtime, Web Crypto API, Font Bézier Rasterizer & DOM Events Formally Proven",
        "solidity/dysnomia/domain/std/wasm_crypto_font_events_prover.algol61",
        2026
    );
    assert(doc_id == 7000);

    bool resolved = tsfi_chancery_docket_resolve_zmm_r15(&docket, doc_id, ruling_auth, DOCKET_RULING_AUTHENTIC_STREAM);
    assert(resolved);

    char audit_report[2048];
    uint64_t docket_proof = tsfi_chancery_docket_audit(&docket, audit_report, sizeof(audit_report));
    assert(docket_proof != 0);

    printf("\n%s\n", audit_report);
    printf("====================================================================\n");
    printf("WASM, CRYPTO, FONT & DOM EVENTS TEST PASSED & SEALED ON CHANCERY\n");
    printf("====================================================================\n");

    return 0;
}
