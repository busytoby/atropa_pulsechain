#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "inc/hathitrust_osl_impression.h"
#include "inc/hathitrust_ao_pass.h"
#include "inc/hathitrust_pxrmaterial.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static void test_impression_shader(void) {
    printf("[Test] Running OSL Letterpress Impression Shader tests...\n");
    
    char *osl = hathitrust_generate_impression_osl(0.025);
    assert(osl != NULL);
    printf("[Test] Generated Impression OSL:\n%s\n", osl);
    
    assert(strstr(osl, "shader HtrcLetterpressImpression") != NULL);
    assert(strstr(osl, "indentDepth = 0.025000") != NULL);
    assert(strstr(osl, "displacementAmount = -indentDepth") != NULL);
    
    free(osl);
    printf("[Test] OSL Letterpress Impression Shader tests passed.\n");
}

static void test_ao_pass(void) {
    printf("[Test] Running RIB Ambient Occlusion Pass Compiler tests...\n");
    
    char *rib = hathitrust_generate_ao_pass_rib();
    assert(rib != NULL);
    printf("[Test] Generated AO RIB:\n%s\n", rib);
    
    assert(strstr(rib, "## RIB Ambient Occlusion Render Pass") != NULL);
    assert(strstr(rib, "Integrator \"PxrOcclusion\"") != NULL);
    assert(strstr(rib, "numSamples\" [64]") != NULL);
    
    free(rib);
    printf("[Test] RIB Ambient Occlusion Pass Compiler tests passed.\n");
}

static void test_material_binding(void) {
    printf("[Test] Running PxrMaterial Binding Generator tests...\n");
    
    char *rib = hathitrust_generate_pxrmaterial_rib(
        "page1", "diff.png", "spec.png", "rough.png"
    );
    assert(rib != NULL);
    printf("[Test] Generated Material Bxdf:\n%s\n", rib);
    
    assert(strstr(rib, "Pattern \"PxrTexture\" \"diff_page1\"") != NULL);
    assert(strstr(rib, "Pattern \"PxrTexture\" \"spec_page1\"") != NULL);
    assert(strstr(rib, "Bxdf \"PxrDisney\" \"mat_page1\"") != NULL);
    assert(strstr(rib, "baseColor\" [\"diff_page1:resultRGB\"]") != NULL);
    
    free(rib);
    printf("[Test] PxrMaterial Binding Generator tests passed.\n");
}

int main(void) {
    printf("[Test] Running HathiTrust RenderMan Final integration tests...\n");
    test_impression_shader();
    test_ao_pass();
    test_material_binding();
    printf("[Test] All RenderMan Final tests completed successfully.\n");
    return 0;
}
