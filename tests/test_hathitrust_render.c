#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "inc/hathitrust_render.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

static void test_displacement_shader(void) {
    printf("[Test] Running DisplacementShader (wm sync) tests...\n");
    
    HtrcVertex v = { 1.0, 2.0, 0.0 };
    double angle = 0.5; // 0.5 radians
    double wm_scale = 1.25; // System register scale boundary
    
    HtrcVertex deformed = hathitrust_displacement_shader_calc(&v, angle, wm_scale);
    
    double expected_x = (1.0 * cos(0.5)) * 1.25;
    double expected_y = 2.0 * 1.25;
    double expected_z = (1.0 * sin(0.5)) * 1.25;
    
    // Validate calculations match expected precision boundaries
    assert(fabs(deformed.x - expected_x) < 1e-6);
    assert(fabs(deformed.y - expected_y) < 1e-6);
    assert(fabs(deformed.z - expected_z) < 1e-6);
    
    printf("[Test] DisplacementShader bending calculations verified.\n");
}

static void test_usda_exporter(void) {
    printf("[Test] Running USDA Stage exporter tests...\n");
    
    const char *htid = "nyp.33433082257217";
    const char *img_url = "https://babel.hathitrust.org/cgi/imgsrv/image?id=nyp.33433082257217;seq=1;size=100";
    
    char *usda = hathitrust_render_to_usda(htid, 1, img_url);
    assert(usda != NULL);
    
    // Assert target structures are correctly present
    assert(strstr(usda, "#usda 1.0") != NULL);
    assert(strstr(usda, "def Xform \"Book_nyp.33433082257217\"") != NULL);
    assert(strstr(usda, "def Xform \"Page_1\"") != NULL);
    assert(strstr(usda, "def Mesh \"PageMesh\"") != NULL);
    assert(strstr(usda, "def Material \"PageMaterial\"") != NULL);
    assert(strstr(usda, img_url) != NULL);
    
    free(usda);
    printf("[Test] USDA Stage exporter verified.\n");
}

static void test_dna_deriver(void) {
    printf("[Test] Running DNA page seed derivation tests...\n");
    
    const char *htid = "nyp.33433082257217";
    unsigned int seed1 = hathitrust_render_derive_dna(htid, 1, 250);
    unsigned int seed2 = hathitrust_render_derive_dna(htid, 1, 250);
    unsigned int seed3 = hathitrust_render_derive_dna(htid, 2, 250);
    
    // Assert deterministic reproducibility
    assert(seed1 == seed2);
    // Assert sensitivity to page change
    assert(seed1 != seed3);
    assert(seed1 > 0);
    
    printf("[Test] Deterministic DNA page seeds verified.\n");
}

int main(void) {
    printf("[Test] Running HathiTrust DisplacementShader, USDA, and DNA exporter tests...\n");
    test_displacement_shader();
    test_usda_exporter();
    test_dna_deriver();
    printf("[Test] All Render unit tests completed successfully.\n");
    return 0;
}
