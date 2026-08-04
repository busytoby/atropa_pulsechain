#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "inc/hathitrust_prman_velocity.h"
#include "inc/hathitrust_osl_gilding.h"
#include "inc/hathitrust_specular.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

static void test_velocity_solver(void) {
    printf("[Test] Running Deformation Velocity Solver tests...\n");
    
    HtrcVertex v1 = { 1.0, 2.0, 3.0 };
    HtrcVertex v2 = { 1.5, 1.8, 3.2 };
    double dt = 0.04; // 1/25th of a second frame step
    
    HtrcVertex velocity = hathitrust_prman_calc_velocity(&v1, &v2, dt);
    
    // velocity = (v2 - v1) / dt
    // x = 0.5 / 0.04 = 12.5
    // y = -0.2 / 0.04 = -5.0
    // z = 0.2 / 0.04 = 5.0
    assert(fabs(velocity.x - 12.5) < 1e-6);
    assert(fabs(velocity.y - (-5.0)) < 1e-6);
    assert(fabs(velocity.z - 5.0) < 1e-6);
    
    printf("[Test] Deformation Velocity Solver tests passed.\n");
}

static void test_gilding_shader(void) {
    printf("[Test] Running OSL Book-Edge Gilding Shader tests...\n");
    
    char *gold_osl = hathitrust_generate_gilding_osl(true);
    assert(gold_osl != NULL);
    printf("[Test] Generated Gold Gilding OSL:\n%s\n", gold_osl);
    
    assert(strstr(gold_osl, "shader HtrcBookGilding") != NULL);
    assert(strstr(gold_osl, "goldColor = color(0.9, 0.78, 0.34)") != NULL);
    
    char *paper_osl = hathitrust_generate_gilding_osl(false);
    assert(paper_osl != NULL);
    printf("[Test] Generated Stacked Paper OSL:\n%s\n", paper_osl);
    
    assert(strstr(paper_osl, "stripeColor = color(0.85, 0.82, 0.76)") != NULL);
    assert(strstr(paper_osl, "stripe = sin(v * 2000.0)") != NULL);
    
    free(gold_osl);
    free(paper_osl);
    printf("[Test] OSL Book-Edge Gilding Shader tests passed.\n");
}

static void test_specular_generator(void) {
    printf("[Test] Running Ink-to-Specular Texture Generator tests...\n");
    
    HtrcAltoPage page;
    page.token_count = 1;
    page.tokens = malloc(1 * sizeof(HtrcAltoToken));
    page.tokens[0].content = strdup("Test");
    page.tokens[0].hpos = 700; // Middle of assumption width (1400)
    page.tokens[0].vpos = 1000; // Middle of assumption height (2000)
    page.tokens[0].width = 140; // 10% of width
    page.tokens[0].height = 200; // 10% of height
    
    int w = 100;
    int h = 100;
    unsigned char *map = hathitrust_generate_specular_map(&page, w, h);
    assert(map != NULL);
    
    // scale maps 700 to 50, 1000 to 50
    // token covers x in [50, 60], y in [50, 60]
    // Let's assert paper background value is 200
    assert(map[0] == 200);
    assert(map[99] == 200);
    
    // Assert ink value is 25 inside the mapped word rectangle
    assert(map[55 * w + 55] == 25);
    
    free(map);
    hathitrust_alto_free(&page);
    printf("[Test] Ink-to-Specular Texture Generator tests passed.\n");
}

int main(void) {
    printf("[Test] Running HathiTrust RenderMan Blur & Specular tests...\n");
    test_velocity_solver();
    test_gilding_shader();
    test_specular_generator();
    printf("[Test] All RenderMan Blur & Specular tests completed successfully.\n");
    return 0;
}
