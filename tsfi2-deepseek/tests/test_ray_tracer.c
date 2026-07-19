#include "tsfi_ray_tracer.h"
#include "tsfi_svdag.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    printf("[TEST] Running CGI/CGM-driven Ray Tracer validation...\n");

    tsfi_cgm_scene scene;
    tsfi_cgm_scene_init(&scene);

    // 1. Parse CGI/CGM Commands
    // Add Sphere at (0, 0, 5) with radius 1.5 and color red (255, 0, 0)
    uint8_t sphere_cmd[28] = {0};
    sphere_cmd[0] = 0x01; // Op: Add Sphere
    float sx = 0.0f, sy = 0.0f, sz = 5.0f;
    float srad = 1.5f;
    memcpy(&sphere_cmd[4], &sx, 4);
    memcpy(&sphere_cmd[8], &sy, 4);
    memcpy(&sphere_cmd[12], &sz, 4);
    memcpy(&sphere_cmd[16], &srad, 4);
    sphere_cmd[20] = 255; // R
    sphere_cmd[21] = 0;   // G
    sphere_cmd[22] = 0;   // B

    int res1 = tsfi_cgm_parse_command(&scene, sphere_cmd, sizeof(sphere_cmd));
    assert(res1 == 0);
    assert(scene.primitive_count == 1);
    assert(scene.primitives[0].type == CGM_PRIM_SPHERE);
    assert(scene.primitives[0].param1 == 1.5f);
    assert(scene.primitives[0].color.x == 1.0f);

    // Add Plane at normal (0, 1, 0) with distance -2.0 and color green (0, 255, 0)
    uint8_t plane_cmd[28] = {0};
    plane_cmd[0] = 0x02; // Op: Add Plane
    float px = 0.0f, py = 1.0f, pz = 0.0f;
    float pd = -2.0f;
    memcpy(&plane_cmd[4], &px, 4);
    memcpy(&plane_cmd[8], &py, 4);
    memcpy(&plane_cmd[12], &pz, 4);
    memcpy(&plane_cmd[16], &pd, 4);
    plane_cmd[20] = 0;   // R
    plane_cmd[21] = 255; // G
    plane_cmd[22] = 0;   // B

    int res2 = tsfi_cgm_parse_command(&scene, plane_cmd, sizeof(plane_cmd));
    assert(res2 == 0);
    assert(scene.primitive_count == 2);
    assert(scene.primitives[1].type == CGM_PRIM_PLANE);
    assert(scene.primitives[1].color.y == 1.0f);

    // Set Light direction to (1.0, 1.0, -1.0)
    uint8_t light_cmd[16] = {0};
    light_cmd[0] = 0x03; // Op: Set Light
    float lx = 1.0f, ly = 1.0f, lz = -1.0f;
    memcpy(&light_cmd[4], &lx, 4);
    memcpy(&light_cmd[8], &ly, 4);
    memcpy(&light_cmd[12], &lz, 4);

    int res3 = tsfi_cgm_parse_command(&scene, light_cmd, sizeof(light_cmd));
    assert(res3 == 0);

    // 2. Perform rendering
    int width = 320;
    int height = 240;
    uint32_t *img_buf = malloc(width * height * sizeof(uint32_t));
    assert(img_buf != NULL);

    int render_res = tsfi_ray_tracer_render(&scene, img_buf, width, height);
    assert(render_res == 0);

    // Verify center pixel intersects the red sphere
    uint32_t center_pixel = img_buf[(height / 2) * width + (width / 2)];
    uint8_t r = (center_pixel >> 16) & 0xFF;
    uint8_t g = (center_pixel >> 8) & 0xFF;
    uint8_t b = center_pixel & 0xFF;

    assert(r > 0);
    assert(g == 0);
    assert(b == 0);

    // Verify Subsurface Scattering (SSS) wrap rendering
    printf("[TEST] Validating Subsurface Scattering (SSS) shader integration...\n");
    scene.primitives[0].param_vec = (tsfi_rt_vec3){0.5f, 1.0f, 0.5f}; // Enable SSS
    int render_res_sss = tsfi_ray_tracer_render(&scene, img_buf, width, height);
    assert(render_res_sss == 0);
    uint32_t sss_pixel = img_buf[(height / 2) * width + (width / 2)];
    uint8_t sss_r = (sss_pixel >> 16) & 0xFF;
    uint8_t sss_g = (sss_pixel >> 8) & 0xFF;
    uint8_t sss_b = sss_pixel & 0xFF;
    printf("DEBUG: sss_pixel=0x%08X (R=%d, G=%d, B=%d)\n", sss_pixel, sss_r, sss_g, sss_b);
    assert(sss_r > 0);
    assert(sss_g > 0); // Warm SSS scatter color mixed in green component

    // 3. Verify Punched Card CAD System Parsing
    printf("[TEST] Validating CAD Punched Card parsing...\n");
    tsfi_cgm_scene cad_scene;
    tsfi_cgm_scene_init(&cad_scene);

    const char *sphere_card = "CAD_SPHERE X:0.5 Y:-0.2 Z:6.0 R:1.8 COLOR:B";
    int cad_res1 = tsfi_cad_parse_punched_card(&cad_scene, sphere_card);
    assert(cad_res1 == 0);
    assert(cad_scene.primitive_count == 1);
    assert(cad_scene.primitives[0].type == CGM_PRIM_SPHERE);
    assert(cad_scene.primitives[0].position.x == 0.5f);
    assert(cad_scene.primitives[0].param1 == 1.8f);
    assert(cad_scene.primitives[0].color.z == 1.0f); // Green / Blue

    const char *light_card = "CAD_LIGHT X:0.1 Y:0.9 Z:-0.3";
    int cad_res2 = tsfi_cad_parse_punched_card(&cad_scene, light_card);
    assert(cad_res2 == 0);
    assert(cad_scene.light_dir.x == 0.1f);

    // 4. Verify GKS Stream to CGI/CGM Scene Conversion
    printf("[TEST] Validating GKS to CGI/CGM conversion...\n");
    tsfi_cgm_scene gks_scene;
    tsfi_cgm_scene_init(&gks_scene);
    uint8_t gks_stream[8] = { 0x01, 0x02, 10, 20, 30, 40, 0, 0 }; // Polyline, 2 points
    int gks_conv_res = tsfi_gks_to_ray_tracer(gks_stream, 6, &gks_scene);
    assert(gks_conv_res == 0);
    assert(gks_scene.primitive_count == 2);
    assert(gks_scene.primitives[0].type == CGM_PRIM_SPHERE);
    assert(gks_scene.primitives[0].position.x == -1.0f); // 10 / 10.0 - 2.0 = -1.0
    assert(gks_scene.primitives[0].position.y == 0.0f);  // 20 / 10.0 - 2.0 = 0.0
    printf("  [PASS] GKS to CGI/CGM ray tracer coordinate mapping verified.\n");

    // 5. Verify Benson-Lehner OSCAR Trace Conversion
    printf("[TEST] Validating Benson-Lehner OSCAR trace conversion...\n");
    tsfi_cgm_scene oscar_scene;
    tsfi_cgm_scene_init(&oscar_scene);
    float coefs[3] = { 1.0f, 0.5f, -0.2f }; // y = 1.0 + 0.5x - 0.2x^2
    int oscar_res = tsfi_oscar_trace_to_ray_tracer(coefs, 3, -1.0f, 1.0f, 4, &oscar_scene);
    assert(oscar_res == 0);
    assert(oscar_scene.primitive_count == 5);
    // At x = 0.0 (index 2): y = 1.0 + 0 - 0 = 1.0
    assert(oscar_scene.primitives[2].position.x == 0.0f);
    assert(oscar_scene.primitives[2].position.y == 1.0f);
    printf("  [PASS] Benson-Lehner OSCAR trace coordinate mapping verified.\n");

    // 6. Verify FIPS 54 Computer Output Microform Grid Conversion
    printf("[TEST] Validating FIPS 54 COM grid conversion...\n");
    tsfi_cgm_scene com_scene;
    tsfi_cgm_scene_init(&com_scene);
    int com_res = tsfi_fips54_com_to_ray_tracer(3, 4, 1.0f, &com_scene);
    assert(com_res == 0);
    assert(com_scene.primitive_count == 12);
    // Center column at index 6 should be at y=0.0, x=0.5
    assert(com_scene.primitives[6].position.x == 0.5f);
    assert(com_scene.primitives[6].position.y == 0.0f);
    printf("  [PASS] FIPS 54 COM layout grid coordinate mapping verified.\n");

    // 7. Verify Vaesen Silhouette and Supernatural Aura integrations
    printf("[TEST] Validating Vaesen integrations...\n");
    // Register and record sight telemetry for Uppsala region
    extern int tsfi_vsen_vaesen_register(const char *name, const char *type, int risk_level, const char *status);
    extern int tsfi_vsen_vaesen_record_sight(const char *name, const char *location, int fear_level);
    
    assert(tsfi_vsen_vaesen_register("Mara", "Nightmare", 2, "Active") == 0);
    assert(tsfi_vsen_vaesen_record_sight("Mara", "Uppsala", 5) == 0);

    tsfi_cgm_scene vaesen_scene;
    tsfi_cgm_scene_init(&vaesen_scene);
    tsfi_cgm_scene_add_primitive(&vaesen_scene, CGM_PRIM_SPHERE, (tsfi_rt_vec3){0, 0, 5}, (tsfi_rt_vec3){1.0f, 0.0f, 0.0f}, 1.5f, (tsfi_rt_vec3){0,0,0});

    // Test Silhouette
    assert(tsfi_ray_tracer_apply_vaesen_silhouette(&vaesen_scene, "Mara") == 0);
    // Radius should be scaled: 1.5f * (1.0f + 0.2f * 2) = 1.5 * 1.4 = 2.1f
    assert(vaesen_scene.primitives[0].param1 > 2.09f && vaesen_scene.primitives[0].param1 < 2.11f);

    // Test Aura
    assert(tsfi_ray_tracer_apply_vaesen_aura(&vaesen_scene, "Uppsala") == 0);
    assert(vaesen_scene.ambient_color.x > 0.3f); // Base 0.1f + 0.05f * 5 = 0.35f

    // Test Unified VSEn Ray Tracer Renderer
    printf("[TEST] Validating Unified VSEn Ray Tracer Renderer...\n");
    uint32_t *vsen_img = malloc(width * height * sizeof(uint32_t));
    assert(vsen_img != NULL);
    assert(tsfi_vsen_ray_tracer_render("Mara", "Uppsala", vsen_img, width, height) == 0);
    free(vsen_img);

    // Test Moondream2 Guided Drawing
    printf("[TEST] Validating VSEn Moondream2 Guided Drawing...\n");
    tsfi_cgm_scene draw_scene;
    tsfi_cgm_scene_init(&draw_scene);
    assert(tsfi_vsen_ray_tracer_draw_element(&draw_scene, "Shield", "iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mNk+M9QDwADhgGAWjR9awAAAABJRU5ErkJggg==") == 0);
    assert(draw_scene.primitive_count == 1);

    // Test Combined VSEn Zorse SVDAG Renderer
    printf("[TEST] Validating VSEn Zorse SVDAG Renderer...\n");
    TSFiTasteTree *taste_tree = tsfi_svdag_create(10);
    assert(taste_tree != NULL);
    uint32_t *zorse_svdag_img = malloc(width * height * sizeof(uint32_t));
    assert(zorse_svdag_img != NULL);
    assert(tsfi_vsen_ray_tracer_render_zorse_svdag(taste_tree, "Shield", "iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mNk+M9QDwADhgGAWjR9awAAAABJRU5ErkJggg==", zorse_svdag_img, width, height) == 0);
    free(zorse_svdag_img);
    tsfi_svdag_destroy(taste_tree);

    free(img_buf);
    printf("[SUCCESS] CGI/CGM Ray Tracer validation completed successfully!\n");
    return 0;
}
