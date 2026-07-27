#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libantigravity_interop.h"

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <query_text> <target_rel>\n", argv[0]);
        return 1;
    }
    const char *query = argv[1];
    int target_rel = atoi(argv[2]);
    int context_hist[3] = { -1, -1, -1 };
    size_t hist_len = 0;
    const char *names = "Vitalik,Satoshi,Gavin";
    int ids[3] = { 101, 102, 103 };
    int conv_src[2] = { 101, 102 };
    int conv_rel[2] = { 50, 60 };
    int conv_dst[2] = { 201, 202 };
    char resp_buf[256] = {0};
    int res = interop_conv_search_query(query, names, ids, 3, conv_src, conv_rel, conv_dst, 2, context_hist, &hist_len, 3, target_rel, resp_buf, sizeof(resp_buf));
    if (res == 0) {
        printf("%s\n", resp_buf);
    } else {
        printf("No match found (code: %d)\n", res);
    }
    return 0;
}
