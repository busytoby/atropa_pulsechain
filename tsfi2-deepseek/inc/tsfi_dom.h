#ifndef TSFI_DOM_H
#define TSFI_DOM_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    TSFI_NODE_DOCUMENT,
    TSFI_NODE_ELEMENT,
    TSFI_NODE_TEXT
} TsfiNodeType;

typedef struct TsfiAttribute {
    char *name;
    char *value;
    struct TsfiAttribute *next;
} TsfiAttribute;

typedef struct TsfiNode {
    TsfiNodeType type;
    char *tag_name;    // For ELEMENT nodes
    char *text_content; // For TEXT nodes

    TsfiAttribute *attributes;

    struct TsfiNode *parent;
    struct TsfiNode *first_child;
    struct TsfiNode *last_child;
    struct TsfiNode *prev_sibling;
    struct TsfiNode *next_sibling;

    // Layout engine metrics (Phase 2 placeholder)
    float layout_x;
    float layout_y;
    float layout_w;
    float layout_h;
} TsfiNode;

#ifdef __cplusplus
extern "C" {
#endif

// DOM lifecycle functions
TsfiNode* tsfi_node_create(TsfiNodeType type);
void tsfi_node_free(TsfiNode *node);
void tsfi_node_add_child(TsfiNode *parent, TsfiNode *child);
void tsfi_node_set_attribute(TsfiNode *node, const char *name, const char *value);
const char* tsfi_node_get_attribute(const TsfiNode *node, const char *name);

// DOM printing utility for testing
void tsfi_dom_print(const TsfiNode *node, int depth);

// HTML Parser entrypoint
TsfiNode* tsfi_dom_parse_html(const char *html_str);

#ifdef __cplusplus
}
#endif

#endif // TSFI_DOM_H
