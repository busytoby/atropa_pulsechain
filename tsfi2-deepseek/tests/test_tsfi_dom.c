#include "tsfi_dom.h"
#include <stdio.h>

int main(void) {
    const char *html = 
        "<html>\n"
        "  <head>\n"
        "    <title>Auncient Test</title>\n"
        "  </head>\n"
        "  <body class=\"main-body\" id=\"root\">\n"
        "    <div class=\"container\">\n"
        "      <h1>Hello Auncient DOM</h1>\n"
        "      <p>This is a custom layout engine DOM test.</p>\n"
        "      <img src=\"assets/logo.png\" />\n"
        "      <br>\n"
        "      <p>More text nodes.</p>\n"
        "    </div>\n"
        "  </body>\n"
        "</html>";

    printf("=== HTML Input ===\n%s\n\n", html);
    printf("=== Parsing ===\n");
    TsfiNode *root = tsfi_dom_parse_html(html);
    if (!root) {
        fprintf(stderr, "Failed to parse HTML.\n");
        return 1;
    }

    printf("=== DOM Tree ===\n");
    tsfi_dom_print(root, 0);

    tsfi_node_free(root);
    printf("=== Done ===\n");
    return 0;
}
