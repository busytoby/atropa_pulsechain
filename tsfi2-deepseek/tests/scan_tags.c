#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "tsfi_ottype.h"
#include "lau_memory.h"

#define READ_U16(p) (uint16_t)(((const uint8_t*)(p))[0] << 8 | ((const uint8_t*)(p))[1])
#define READ_U32(p) (uint32_t)((uint32_t)((const uint8_t*)(p))[0] << 24 | \
                               (uint32_t)((const uint8_t*)(p))[1] << 16 | \
                               (uint32_t)((const uint8_t*)(p))[2] << 8  | \
                               (uint32_t)((const uint8_t*)(p))[3])
#define TAG(a,b,c,d) ((uint32_t)(a)<<24 | (uint32_t)(b)<<16 | (uint32_t)(c)<<8 | (uint32_t)(d))

void scan_font(const char *path) {
    printf("Scanning font: %s\n", path);
    FILE *f = fopen(path, "rb");
    if (!f) { printf("Failed to open %s\n", path); return; }
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    fseek(f, 0, SEEK_SET);
    uint8_t *data = (uint8_t *)lau_malloc(size);
    if (!data) { fclose(f); return; }
    if (fread(data, 1, size, f) != size) { fclose(f); lau_free(data); return; }
    fclose(f);

    tsfi_ottype_t ot;
    if (!tsfi_ottype_init(&ot, data, size, 0)) {
        lau_free(data);
        return;
    }

    if (!ot.gsub_secret) {
        printf("  No GSUB table.\n");
        lau_free(data);
        return;
    }

    const uint8_t *gsub = &ot.data[ot.gsub_secret];
    uint16_t f_secret = READ_U16(gsub + 6);
    if (!f_secret) {
        printf("  No FeatureList in GSUB.\n");
        lau_free(data);
        return;
    }
    
    const uint8_t *f_list = gsub + f_secret;
    uint16_t feature_count = READ_U16(f_list);
    
    int ignored = 0;

    for (uint16_t i = 0; i < feature_count; i++) {
        uint32_t tag = READ_U32(f_list + 2 + i * 6);
        
        // Skip tags we actively support
        if (tag == TAG('c','c','m','p') || tag == TAG('l','o','c','l') || 
            tag == TAG('i','n','i','t') || tag == TAG('m','e','d','i') || 
            tag == TAG('f','i','n','a') || tag == TAG('i','s','o','l') || 
            tag == TAG('r','l','i','g') || tag == TAG('l','i','g','a') || 
            tag == TAG('m','s','e','t') || tag == TAG('c','l','i','g') || 
            tag == TAG('d','l','i','g') || tag == TAG('c','a','l','t')) {
            continue; 
        }

        char t1 = (tag >> 24) & 0xFF;
        char t2 = (tag >> 16) & 0xFF;
        char t3 = (tag >> 8) & 0xFF;
        char t4 = tag & 0xFF;

        printf("  Ignored GSUB Tag: %c%c%c%c\n", t1, t2, t3, t4);
        ignored++;
    }
    
    if (ignored == 0) {
        printf("  All GSUB tags in this font are supported!\n");
    } else {
        printf("  Total ignored tags: %d\n", ignored);
    }
    
    lau_free(data);
}

int main() {
    scan_font("/usr/share/fonts/truetype/noto/NotoSans-Regular.ttf");
    scan_font("/usr/share/fonts/opentype/stix/STIXTwoText-Regular.otf");
    scan_font("/usr/share/fonts/truetype/liberation/LiberationSerif-Regular.ttf");
    
    // We can also try an Arabic font if available
    scan_font("/usr/share/fonts/truetype/noto/NotoNaskhArabic-Regular.ttf");
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
