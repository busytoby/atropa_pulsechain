#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "inc/hathitrust_osl_bleed.h"
#include "inc/hathitrust_camera_path.h"
#include "inc/hathitrust_rib_to_iiif.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static void test_osl_bleed_generator(void) {
    printf("[Test] Running OSL Bleed and Bump Generator tests...\n");
    
    char *osl = hathitrust_generate_bleed_osl(0.35, 0.05);
    assert(osl != NULL);
    printf("[Test] Generated Bleed OSL:\n%s\n", osl);
    
    assert(strstr(osl, "shader HtrcInkBleed") != NULL);
    assert(strstr(osl, "bleedFactor = 0.350000") != NULL);
    assert(strstr(osl, "bumpScale = 0.050000") != NULL);
    assert(strstr(osl, "normalOffset = vector") != NULL);
    
    free(osl);
    printf("[Test] OSL Bleed and Bump Generator tests passed.\n");
}

static void test_camera_path_generator(void) {
    printf("[Test] Running RIB Camera Path Generator tests...\n");
    
    HtrcAltoPage page;
    page.token_count = 3;
    page.tokens = malloc(3 * sizeof(HtrcAltoToken));
    
    // First line tokens (vpos = 100)
    page.tokens[0].content = strdup("First");
    page.tokens[0].hpos = 50;
    page.tokens[0].vpos = 100;
    page.tokens[0].width = 40;
    page.tokens[0].height = 10;
    
    page.tokens[1].content = strdup("Line");
    page.tokens[1].hpos = 100;
    page.tokens[1].vpos = 100;
    page.tokens[1].width = 30;
    page.tokens[1].height = 10;
    
    // Second line token (vpos = 150)
    page.tokens[2].content = strdup("Second");
    page.tokens[2].hpos = 60;
    page.tokens[2].vpos = 150;
    page.tokens[2].width = 50;
    page.tokens[2].height = 10;
    
    char *rib_path = hathitrust_generate_camera_path_rib(&page);
    assert(rib_path != NULL);
    printf("[Test] Generated Camera Path RIB:\n%s\n", rib_path);
    
    // Assert target structures are present
    assert(strstr(rib_path, "FrameBegin 1") != NULL);
    assert(strstr(rib_path, "FrameBegin 2") != NULL);
    assert(strstr(rib_path, "Translate") != NULL);
    
    free(rib_path);
    hathitrust_alto_free(&page);
    printf("[Test] RIB Camera Path Generator tests passed.\n");
}

static void test_rib_to_iiif_transcoder(void) {
    printf("[Test] Running RIB-to-IIIF Transcoder tests...\n");
    
    const char *mock_rib =
        "  AttributeBegin\n"
        "    Pattern \"PxrTexture\" \"page_tex_1\" \"string filename\" [\"https://babel.hathitrust.org/cgi/imgsrv/image?id=uc1.31970011037748;seq=1;size=100\"]\n"
        "  AttributeEnd\n";
        
    char *iiif = hathitrust_rib_to_iiif(mock_rib);
    assert(iiif != NULL);
    printf("[Test] RIB-to-IIIF Manifest:\n%s\n", iiif);
    
    assert(strstr(iiif, "\"@context\": \"http://iiif.io/api/presentation/2/context.json\"") != NULL);
    assert(strstr(iiif, "uc1.31970011037748") != NULL);
    assert(strstr(iiif, "\"label\": \"Page 1\"") != NULL);
    
    free(iiif);
    printf("[Test] RIB-to-IIIF Transcoder tests passed.\n");
}

int main(void) {
    printf("[Test] Running HathiTrust RenderMan Extra integration tests...\n");
    test_osl_bleed_generator();
    test_camera_path_generator();
    test_rib_to_iiif_transcoder();
    printf("[Test] All RenderMan Extra tests completed successfully.\n");
    return 0;
}
