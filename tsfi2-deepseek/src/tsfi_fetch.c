#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "tsfi_raw.h"
#include "tsfi_wire_pty.h"
#include "tsfi_io.h"
#include "tsfi_config.h"
#include "lau_memory.h"

void tsfi_fetch_url(const char *url, const char *output_path) {
    const char *cookie = tsfi_config_get("FLOW_COOKIE");
    if (!cookie) {
        tsfi_io_printf(stderr, "[FETCH] Error: FLOW_COOKIE not found in config.\n");
        return;
    }

    char cmd[8192];
    snprintf(cmd, sizeof(cmd), 
        "curl -s -L -H 'Cookie: %s' "
        "-H 'User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36' "
        "-H 'Accept: */*' -H 'Referer: https://labs.google/fx/library' "
        "-H 'x-nextjs-data: 1' '%s' -o %s", 
        cookie, url, output_path);

    tsfi_io_printf(stdout, "[FETCH] Executing native fetch wavefront: %s\n", url);
    
    LauWirePty *pty = tsfi_wire_pty_fork(cmd);
    if (!pty) {
        tsfi_io_printf(stderr, "[FETCH] Failed to spawn fetch PTY.\n");
        return;
    }

    while (pty->is_active) {
        tsfi_wire_pty_step(pty);
        tsfi_raw_usleep(10000);
    }

    tsfi_io_printf(stdout, "[FETCH] Wavefront complete. Result saved to %s\n", output_path);
    lau_free(pty);
}

int main(int argc, char **argv) {
    if (argc < 3) {
        tsfi_io_printf(stderr, "Usage: %s <url> <output_path>\n", argv[0]);
        return 1;
    }

    tsfi_fetch_url(argv[1], argv[2]);
    
    tsfi_config_teardown();
    lau_report_memory_metrics();
    return 0;
}
