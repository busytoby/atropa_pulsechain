#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "inc/hathitrust_osl_double_sided.h"
#include "inc/hathitrust_animator.h"
#include "inc/hathitrust_spine.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static void test_double_sided_shader(void) {
    printf("[Test] Running OSL Double-Sided Page Shader tests...\n");
    
    char *osl = hathitrust_generate_double_sided_osl("front.png", "back.png");
    assert(osl != NULL);
    printf("[Test] Generated Double-Sided OSL:\n%s\n", osl);
    
    assert(strstr(osl, "shader HtrcDoubleSidedPage") != NULL);
    assert(strstr(osl, "front.png") != NULL);
    assert(strstr(osl, "back.png") != NULL);
    assert(strstr(osl, "backfacing()") != NULL);
    
    free(osl);
    printf("[Test] OSL Double-Sided Page Shader tests passed.\n");
}

static void test_page_curl_animator(void) {
    printf("[Test] Running RIB Page-Curl Keyframe Animator tests...\n");
    
    char *anim = hathitrust_generate_page_curl_keyframes_rib(1, 10);
    assert(anim != NULL);
    printf("[Test] Generated Animator Sequence length: %zu bytes\n", strlen(anim));
    
    assert(strstr(anim, "FrameBegin 1") != NULL);
    assert(strstr(anim, "FrameBegin 10") != NULL);
    assert(strstr(anim, "HtrcPageCurlShader") != NULL);
    assert(strstr(anim, "curlAngle") != NULL);
    
    free(anim);
    printf("[Test] RIB Page-Curl Keyframe Animator tests passed.\n");
}

static void test_book_binding_generator(void) {
    printf("[Test] Running Book Binding & Spine Generator tests...\n");
    
    char *rib = hathitrust_generate_book_binding_rib(150); // 150 pages
    assert(rib != NULL);
    printf("[Test] Generated Spine RIB:\n%s\n", rib);
    
    assert(strstr(rib, "book_spine") != NULL);
    assert(strstr(rib, "front_cover") != NULL);
    assert(strstr(rib, "back_cover") != NULL);
    assert(strstr(rib, "Patch \"bilinear\"") != NULL);
    
    free(rib);
    printf("[Test] Book Binding & Spine Generator tests passed.\n");
}

#include "tsfi_displacementshader.h"

static void test_page_curl_displacement(void) {
    printf("[Test] Evaluating page-curl displacement math under WinchesterMQ boundary constraints...\n");
    TSFiDisplacementShader ds;
    tsfi_displacementshader_init(&ds, 2.0, 1.0);
    
    double dy = 0.0, dz = 0.0;
    assert(tsfi_displacementshader_eval_page_curl(&ds, 0.5, 0.8, 1.05, &dy, &dz));
    assert(dy != 0.8); // Should be curled/displaced
    assert(dz > 0.0);
    
    // Test boundary conditions
    assert(tsfi_displacementshader_eval_page_curl(&ds, 0.5, 0.8, 0.0, &dy, &dz));
    assert(dy == 0.8); // Should remain flat when curl_angle is 0.0
    assert(dz == 0.0);
    
    printf("[Test] Page-curl displacement evaluation verified successfully.\n");
}

int main(void) {
    printf("[Test] Running HathiTrust RenderMan Animation integration tests...\n");
    test_double_sided_shader();
    test_page_curl_animator();
    test_book_binding_generator();
    test_page_curl_displacement();
    printf("[Test] All RenderMan Animation tests completed successfully.\n");
    return 0;
}
