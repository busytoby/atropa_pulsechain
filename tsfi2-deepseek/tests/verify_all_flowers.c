#include "lau_memory.h"
#include "tsfi_vision.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <dirent.h>

int main() {
    printf("[VERIFY] Starting All-Flowers Classification Check...\n");

    DIR *d = opendir("assets/flowers");
    if (!d) {
        perror("opendir");
        return 1;
    }

    struct dirent *dir;
    int checked = 0;
    int passed = 0;

    while ((dir = readdir(d)) != NULL) {
        if (strstr(dir->d_name, ".checkpoint")) {
            char full_path[512];
            snprintf(full_path, 511, "assets/flowers/%s", dir->d_name);
            
            TSFiResonanceAnalysis a;
            int w, h;
            FILE *f = fopen(full_path, "rb");
            if (!f) {
                printf("[FAIL] Could not load checkpoint: %s\n", dir->d_name);
                continue;
            }
            if (fread(&a, sizeof(TSFiResonanceAnalysis), 1, f) != 1) { fclose(f); continue; }
            if (fread(&w, sizeof(int), 1, f) != 1) { fclose(f); continue; }
            if (fread(&h, sizeof(int), 1, f) != 1) { fclose(f); continue; }
            fclose(f);

            printf("[CHECK] Phenotype: %s\n", dir->d_name);
            printf("        Profile: %.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n",
                   a.captured_radial_profile[0], a.captured_radial_profile[1],
                   a.captured_radial_profile[2], a.captured_radial_profile[3],
                   a.captured_radial_profile[4], a.captured_radial_profile[5],
                   a.captured_radial_profile[6], a.captured_radial_profile[7]);

            // Allocate a wired object representing a VISA manifestation
            void *obj = lau_malloc_wired(sizeof(uint32_t) + sizeof(TSFiResonanceAnalysis));
            uint32_t *magic = (uint32_t*)obj;
            *magic = 0x41534956;
            TSFiResonanceAnalysis *vis_ptr = (TSFiResonanceAnalysis*)(magic + 1);
            *vis_ptr = a;

            int class_id = tsfi_vision_classify_thunk(obj);
            printf("        Classified as: %d\n", class_id);

            if (class_id == TSFI_CLASS_FLORAL || class_id >= 10) {
                passed++;
            } else {
                printf("        [ERROR] %s did NOT classify as FLORAL or Species (Got %d)\n", dir->d_name, class_id);
            }

            lau_free(obj);
            checked++;
        }
    }
    closedir(d);

    printf("[VERIFY] Checked %d flowers. Passed: %d\n", checked, passed);
    
    if (checked > 0 && passed == checked) {
        printf("[SUCCESS] All flowers correctly classify as FLORAL.\n");
            extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
    } else {
        printf("[FAILURE] Some flowers failed classification.\n");
        return 1;
    }
}