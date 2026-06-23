#define _GNU_SOURCE
#include "tsfi_dom.h"
#include "tsfi_layout.h"
#include "tsfi_paint.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int tsfi_jpeg_encode(unsigned char **out_buf, unsigned long *out_size, const unsigned char *rgb24, int width, int height, int quality);

static void save_jpeg(const char *filename, uint32_t *pixels, int w, int h) {
    uint8_t *rgb24 = (uint8_t *)malloc(w * h * 3);
    for (int i = 0; i < w * h; i++) {
        uint32_t pixel = pixels[i];
        rgb24[i * 3 + 0] = (pixel >> 16) & 0xFF; // R
        rgb24[i * 3 + 1] = (pixel >> 8) & 0xFF;  // G
        rgb24[i * 3 + 2] = pixel & 0xFF;         // B
    }
    unsigned char *jpeg_buf = NULL;
    unsigned long jpeg_size = 0;
    int res = tsfi_jpeg_encode(&jpeg_buf, &jpeg_size, rgb24, w, h, 90);
    if (res == 0) {
        FILE *f = fopen(filename, "wb");
        if (f) {
            fwrite(jpeg_buf, 1, jpeg_size, f);
            fclose(f);
            printf("[Capture] Saved %s (%lu bytes)\n", filename, jpeg_size);
        }
    }
    free(rgb24);
}

int main(void) {
    int w = 1024, h = 768;
    uint32_t *pixels = (uint32_t *)calloc(w * h, sizeof(uint32_t));

    const char *html_templates[5] = {
        // State 0: Basic YouTube Page
        "<div style=\"background-color: #0f0f0f; width: 1024px; height: 768px; color: #f1f1f1; padding: 20px;\">\n"
        "  <div style=\"display: flex; background-color: #0f0f0f; width: 980px; height: 60px; margin-bottom: 20px; border-bottom: 1px solid #2f2f2f; align-items: center;\">\n"
        "    <span style=\"color: #ff0000; font-size: 28px; font-weight: bold; width: 200px;\">YouTube</span>\n"
        "    <span style=\"background-color: #121212; width: 500px; height: 40px; color: #f1f1f1; padding: 10px; border: 1px solid #303030; font-size: 18px; margin-left: 50px;\"></span>\n"
        "    <span style=\"background-color: #303030; width: 80px; height: 40px; color: #ffffff; padding: 10px; font-size: 16px; font-weight: bold; margin-left: 10px; text-align: center;\">Search</span>\n"
        "  </div>\n"
        "  <div style=\"display: flex; width: 980px; height: 580px;\">\n"
        "    <div style=\"display: block; width: 240px; height: 500px; margin-right: 20px; background-color: #0f0f0f;\">\n"
        "      <span style=\"display: block; padding: 10px; color: #ffffff; font-size: 16px;\">Home</span>\n"
        "      <span style=\"display: block; padding: 10px; color: #aaaaaa; font-size: 16px;\">Trending</span>\n"
        "      <span style=\"display: block; padding: 10px; color: #aaaaaa; font-size: 16px;\">Subscriptions</span>\n"
        "    </div>\n"
        "    <div style=\"display: block; width: 700px; height: 500px;\">\n"
        "      <div style=\"background-color: #333333; width: 640px; height: 360px; margin-bottom: 10px;\"></div>\n"
        "      <div style=\"font-size: 22px; font-weight: bold; margin-bottom: 10px; color: #ffffff;\">Atropa Pulsechain: The Helmholtz Web Launch</div>\n"
        "    </div>\n"
        "  </div>\n"
        "</div>",
 
        // State 1: Entering "atropa" into the query box
        "<div style=\"background-color: #0f0f0f; width: 1024px; height: 768px; color: #f1f1f1; padding: 20px;\">\n"
        "  <div style=\"display: flex; background-color: #0f0f0f; width: 980px; height: 60px; margin-bottom: 20px; border-bottom: 1px solid #2f2f2f; align-items: center;\">\n"
        "    <span style=\"color: #ff0000; font-size: 28px; font-weight: bold; width: 200px;\">YouTube</span>\n"
        "    <span style=\"background-color: #121212; width: 500px; height: 40px; color: #ffffff; padding: 10px; border: 1px solid #303030; font-size: 18px; margin-left: 50px;\">atropa</span>\n"
        "    <span style=\"background-color: #303030; width: 80px; height: 40px; color: #ffffff; padding: 10px; font-size: 16px; font-weight: bold; margin-left: 10px; text-align: center;\">Search</span>\n"
        "  </div>\n"
        "  <div style=\"display: flex; width: 980px; height: 580px;\">\n"
        "    <div style=\"display: block; width: 240px; height: 500px; margin-right: 20px; background-color: #0f0f0f;\">\n"
        "      <span style=\"display: block; padding: 10px; color: #ffffff; font-size: 16px;\">Home</span>\n"
        "      <span style=\"display: block; padding: 10px; color: #aaaaaa; font-size: 16px;\">Trending</span>\n"
        "    </div>\n"
        "  </div>\n"
        "</div>",
 
        // State 2: Second Page (Search Results)
        "<div style=\"background-color: #0f0f0f; width: 1024px; height: 768px; color: #f1f1f1; padding: 20px;\">\n"
        "  <div style=\"display: flex; background-color: #0f0f0f; width: 980px; height: 60px; margin-bottom: 20px; border-bottom: 1px solid #2f2f2f; align-items: center;\">\n"
        "    <span style=\"color: #ff0000; font-size: 28px; font-weight: bold; width: 200px;\">YouTube</span>\n"
        "    <span style=\"background-color: #121212; width: 500px; height: 40px; color: #f1f1f1; padding: 10px; border: 1px solid #303030; font-size: 18px; margin-left: 50px;\">atropa</span>\n"
        "    <span style=\"background-color: #303030; width: 80px; height: 40px; color: #ffffff; padding: 10px; font-size: 16px; font-weight: bold; margin-left: 10px; text-align: center;\">Search</span>\n"
        "  </div>\n"
        "  <div style=\"display: flex; width: 980px; height: 40px; margin-bottom: 15px; align-items: center;\">\n"
        "    <span style=\"background-color: #303030; width: 100px; height: 30px; color: #ffffff; padding: 5px; font-size: 14px; text-align: center;\">Filters</span>\n"
        "    <span style=\"color: #aaaaaa; font-size: 14px; margin-left: 15px;\">About 4,210 results for \"atropa\"</span>\n"
        "  </div>\n"
        "  <div style=\"display: block; width: 980px; height: 580px;\">\n"
        "    <div style=\"display: flex; width: 950px; height: 120px; margin-bottom: 20px; background-color: #111111;\">\n"
        "      <div style=\"background-color: #333333; width: 200px; height: 110px;\"></div>\n"
        "      <div style=\"display: block; margin-left: 20px; width: 700px;\">\n"
        "        <div style=\"font-size: 20px; font-weight: bold; color: #ffffff;\">Atropa Pulsechain: Complete Auncient Guide (2026)</div>\n"
        "        <div style=\"color: #aaaaaa; font-size: 13px; margin-top: 5px;\">Auncient Tech &bull; 85K views &bull; 3 months ago</div>\n"
        "      </div>\n"
        "    </div>\n"
        "  </div>\n"
        "</div>",
 
        // State 3: Selecting "This week" Filter
        "<div style=\"background-color: #0f0f0f; width: 1024px; height: 768px; color: #f1f1f1; padding: 20px;\">\n"
        "  <div style=\"display: flex; background-color: #0f0f0f; width: 980px; height: 60px; margin-bottom: 20px; border-bottom: 1px solid #2f2f2f; align-items: center;\">\n"
        "    <span style=\"color: #ff0000; font-size: 28px; font-weight: bold; width: 200px;\">YouTube</span>\n"
        "    <span style=\"background-color: #121212; width: 500px; height: 40px; color: #f1f1f1; padding: 10px; border: 1px solid #303030; font-size: 18px; margin-left: 50px;\">atropa</span>\n"
        "    <span style=\"background-color: #303030; width: 80px; height: 40px; color: #ffffff; padding: 10px; font-size: 16px; font-weight: bold; margin-left: 10px; text-align: center;\">Search</span>\n"
        "  </div>\n"
        "  <div style=\"width: 980px; height: 140px; background-color: #161616; padding: 15px; margin-bottom: 15px; border: 1px solid #3f3f3f;\">\n"
        "    <div style=\"font-weight: bold; font-size: 14px; margin-bottom: 10px; color: #aaaaaa;\">UPLOAD DATE</div>\n"
        "    <div style=\"font-size: 14px; margin-bottom: 8px; color: #ffffff;\">Today</div>\n"
        "    <div style=\"font-size: 14px; margin-bottom: 8px; color: #ff0000; font-weight: bold;\">This week</div>\n"
        "  </div>\n"
        "</div>",
 
        // State 4: Third Page (Filtered Results)
        "<div style=\"background-color: #0f0f0f; width: 1024px; height: 768px; color: #f1f1f1; padding: 20px;\">\n"
        "  <div style=\"display: flex; background-color: #0f0f0f; width: 980px; height: 60px; margin-bottom: 20px; border-bottom: 1px solid #2f2f2f; align-items: center;\">\n"
        "    <span style=\"color: #ff0000; font-size: 28px; font-weight: bold; width: 200px;\">YouTube</span>\n"
        "    <span style=\"background-color: #121212; width: 500px; height: 40px; color: #f1f1f1; padding: 10px; border: 1px solid #303030; font-size: 18px; margin-left: 50px;\">atropa</span>\n"
        "    <span style=\"background-color: #303030; width: 80px; height: 40px; color: #ffffff; padding: 10px; font-size: 16px; font-weight: bold; margin-left: 10px; text-align: center;\">Search</span>\n"
        "  </div>\n"
        "  <div style=\"display: flex; width: 980px; height: 40px; margin-bottom: 15px; align-items: center;\">\n"
        "    <span style=\"background-color: #ff0000; width: 150px; height: 30px; color: #ffffff; padding: 5px; font-size: 14px; font-weight: bold; border-radius: 4px; text-align: center;\">Filter: This Week</span>\n"
        "    <span style=\"color: #aaaaaa; font-size: 14px; margin-left: 15px;\">124 results found this week for \"atropa\"</span>\n"
        "  </div>\n"
        "  <div style=\"display: block; width: 980px; height: 580px;\">\n"
        "    <div style=\"display: flex; width: 950px; height: 120px; margin-bottom: 20px; background-color: #111111;\">\n"
        "      <div style=\"background-color: #333333; width: 200px; height: 110px;\"></div>\n"
        "      <div style=\"display: block; margin-left: 20px; width: 700px;\">\n"
        "        <div style=\"font-size: 20px; font-weight: bold; color: #ff0000;\">Atropa Pulsechain: Segment 0</div>\n"
        "        <div style=\"color: #aaaaaa; font-size: 13px; margin-top: 5px;\">Auncient Tech &bull; 3.2K views &bull; 2 days ago &bull; NEW</div>\n"
        "      </div>\n"
        "    </div>\n"
        "  </div>\n"
        "</div>"
    };

    char out_path[512];
    for (int i = 0; i < 5; i++) {
        memset(pixels, 0, w * h * sizeof(uint32_t));
        TsfiNode *dom = tsfi_dom_parse_html(html_templates[i]);
        if (dom) {
            TsfiLayoutBox *layout = tsfi_layout_tree_build(dom);
            if (layout) {
                tsfi_layout_solve(layout, 0.0f, 0.0f, (float)w, (float)h);
                tsfi_paint_tree(layout, pixels, w, h);
                tsfi_layout_box_free(layout);
            }
            tsfi_node_free(dom);
        }
        snprintf(out_path, sizeof(out_path), "/home/mariarahel/.gemini/antigravity-cli/brain/7df5dc30-81e4-429d-a952-75e131607428/state_%d.jpg", i);
        save_jpeg(out_path, pixels, w, h);
    }

    free(pixels);
    printf("[Success] States captured successfully.\n");
    return 0;
}
