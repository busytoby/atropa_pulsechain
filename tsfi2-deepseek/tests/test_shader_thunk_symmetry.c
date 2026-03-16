#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <immintrin.h>
#include "tsfi_wave_any.h"
#include "lau_memory.h"
#include "lau_thunk.h"

// Uniforms Definition (Must match thunks/wave_shader.c)
typedef struct {
    float intensity;
    float decay;
} Uniforms;

// External reference to the shader function (Simulating dlsym/JIT entry)
void wave_shader_main(const WaveStream* stream, const Uniforms* u);

// Define a test atom (4 floats = 16 bytes)
typedef struct { float r, g, b, a; } ColorAtom;

void test_symmetry() {
    printf("[TEST] Bijective Shader-Thunk Symmetry...\n");
    
    // 1. Setup Stream
    size_t count = 1024;
    WaveStream stream = {0};
    stream.atom_size = 16;
    stream.stride = 16; // Packed
    stream.count = count;
    stream.data = lau_memalign(64, count * stream.stride);
    
    // Fill with 1.0f
    for(size_t i=0; i<count; i++) {
        ColorAtom* c = (ColorAtom*)WAVE_PTR(&stream, i);
        c->r = 1.0f; c->g = 1.0f; c->b = 1.0f; c->a = 1.0f;
    }
    
    // 2. Setup Uniforms
    Uniforms u;
    u.intensity = 2.0f;
    u.decay = 0.5f;
    // Expected result: (1.0 * 2.0) - 0.5 = 1.5
    
    // 3. Create Thunk Proxy (The "Shader Pipeline")
    ThunkProxy* proxy = ThunkProxy_create();
    assert(proxy);
    
    // Emit "Baked" Thunk: Call shader with fixed stream/uniforms
    // void (*entry)(void)
    typedef void (*ShaderEntry)(void);
    
    // We bind "stream" and "u" as arguments 0 and 1
    ShaderEntry entry = (ShaderEntry)ThunkProxy_emit_baked(proxy, (void*)wave_shader_main, 2, &stream, &u);
    
    // 4. Dispatch (Simulate GPU Kernel Launch)
    printf("  [DISPATCH] Calling JIT Thunk -> Shader...\n");
    entry();
    
    // 5. Verify Results
    printf("  [VERIFY] Checking output...\n");
    for(size_t i=0; i<count; i++) {
        ColorAtom* c = (ColorAtom*)WAVE_PTR(&stream, i);
        if (c->r != 1.5f || c->g != 1.5f || c->b != 1.5f || c->a != 1.5f) {
            printf("FAIL at %zu: %f (Expected 1.5)\n", i, c->r);
            exit(1);
        }
    }
    
    // 6. Cleanup
    ThunkProxy_destroy(proxy);
    lau_free(stream.data);
    printf("[PASS] Symmetry Verified: Logic -> Thunk -> AVX-512 -> Memory\n");
}

int main() {
    test_symmetry();
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
