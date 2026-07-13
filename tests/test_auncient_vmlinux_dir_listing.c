#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

int main(void) {
    printf("=============================================================\n");
    printf("Auniciet ZMM VM: Virtual Linux Init Execution Prover\n");
    printf("=============================================================\n");

    // Execute the statically compiled zmm_init as a child process
    // (capturing its output to verify Helmholtz & disk audits)
    FILE *fp = popen("firmware/tsfi_lib/zmm_init", "r");
    if (!fp) {
        printf("Error: Failed to execute firmware/tsfi_lib/zmm_init\n");
        return 1;
    }

    char buffer[256];
    bool saw_init_banner = false;
    bool saw_helmholtz_audit = false;
    bool saw_directory_listing = false;
    bool saw_disk_space = false;

    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        printf("%s", buffer);
        if (strstr(buffer, "AUNCIENT LINUX ROOT INIT")) {
            saw_init_banner = true;
        }
        if (strstr(buffer, "Auditing Helmholtz state")) {
            saw_helmholtz_audit = true;
        }
        if (strstr(buffer, "Listing Directory for path")) {
            saw_directory_listing = true;
        }
        if (strstr(buffer, "Disk space on")) {
            saw_disk_space = true;
        }
    }
    pclose(fp);

    // Verify all diagnostic points ran successfully
    assert(saw_init_banner == true);
    assert(saw_helmholtz_audit == true);
    assert(saw_directory_listing == true);
    assert(saw_disk_space == true);

    printf("\n=============================================================\n");
    printf("VIRTUAL LINUX INIT RUN INTEROP VERIFICATION PASSED\n");
    printf("=============================================================\n");
    return 0;
}
