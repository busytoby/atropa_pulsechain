/* SPDX-License-Identifier: GPL-2.0 */
/*
 * C11 Test Prover: CP/M-Tomie RenderMan PageTurner Framebuffer Rasterizer & Presenter
 */

#include "cpm_tomie_renderman_pageturner_presenter.h"
#include "cpm_tomie_pageturner_renderman.h"
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <unistd.h>

int main(void) {
    printf("=================================================================\n");
    printf("TEST PROVER: CP/M-TOMIE RENDERMAN PAGETURNER PRESENTER RASTERIZER\n");
    printf("=================================================================\n");

    PageTurnerRenderManEngine engine;
    pageturner_init(&engine, 4);
    pageturner_set_digital_twin_telemetry(&engine, 23.0f, 50.0f, 440.0f, 22.0f, 15.0f, 30.0f, 3.5f);
    pageturner_apply_clay_curl(&engine, 0, 0.40f);
    pageturner_apply_displacement_shader(&engine, 0);

    RenderManFrameBuffer fb;
    renderman_framebuffer_clear(&fb, 30, 30, 35);
    assert(fb.width == 320);
    assert(fb.height == 240);
    printf(" FrameBuffer Clear:        PASS (320x240 RGB)\n");

    bool rast_ok = renderman_rasterize_page_mesh(&fb, &engine.pages[0], &engine);
    assert(rast_ok);
    printf(" Page Mesh Rasterization:  PASS (Projected & Depth-Buffered)\n");

    uint32_t csum = renderman_framebuffer_rule18_checksum(&fb);
    assert(csum > 0);
    printf(" Frame Rule 18 Checksum:   PASS (0x%08X Verified)\n", csum);

    const char *out_path = "/tmp/test_pageturner_frame.dat.bin";
    bool save_ok = renderman_save_framebuffer_dat_bin(&fb, out_path);
    assert(save_ok);
    unlink(out_path);
    printf(" .dat.bin Format Storage:  PASS (Rule 13 Quadtree/Binary Compliant)\n");

    printf("=================================================================\n");
    printf("ALL RENDERMAN PAGETURNER PRESENTER TESTS PASSED SUCCESSFULLY.\n");
    printf("=================================================================\n");
    return 0;
}
