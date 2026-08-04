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

int main(void) {
    printf("[Test] Running HathiTrust RenderMan Animation integration tests...\n");
    test_double_sided_shader();
    test_page_curl_animator();
    test_book_binding_generator();
    printf("[Test] All RenderMan Animation tests completed successfully.\n");
    return 0;
}
