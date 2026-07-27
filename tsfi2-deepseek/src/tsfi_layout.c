#define _GNU_SOURCE
#include "tsfi_layout.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

TsfiLayoutBox* tsfi_layout_box_create(TsfiNode *node) {
    TsfiLayoutBox *box = (TsfiLayoutBox*)calloc(1, sizeof(TsfiLayoutBox));
    if (box) {
        box->node = node;
        box->type = TSFI_BOX_BLOCK; // default
    }
    return box;
}

void tsfi_layout_box_free(TsfiLayoutBox *box) {
    if (!box) return;
    TsfiLayoutBox *child = box->first_child;
    while (child) {
        TsfiLayoutBox *next = child->next_sibling;
        tsfi_layout_box_free(child);
        child = next;
    }
    free(box);
}

void tsfi_layout_add_child(TsfiLayoutBox *parent, TsfiLayoutBox *child) {
    if (!parent || !child) return;
    child->parent = parent;
    child->prev_sibling = parent->last_child;
    child->next_sibling = NULL;

    if (parent->last_child) {
        parent->last_child->next_sibling = child;
    } else {
        parent->first_child = child;
    }
    parent->last_child = child;
}

// Parses numeric values from style properties, e.g. "100px" or "10"
static float parse_px(const char *val) {
    if (!val) return 0.0f;
    while (*val && isspace((unsigned char)*val)) val++;
    float num = 0.0f;
    if (sscanf(val, "%f", &num) == 1) {
        return num;
    }
    return 0.0f;
}

// Inline style property extractor
static const char* get_style_val(const char *style, const char *prop_name, char *buf, size_t buf_sz) {
    if (!style) return NULL;
    const char *p = style;
    size_t prop_len = strlen(prop_name);
    while (*p) {
        p = strstr(p, prop_name);
        if (!p) break;

        // Check it is indeed the property (not a substring of another property)
        if (p == style || *(p - 1) == ';' || isspace((unsigned char)*(p - 1))) {
            const char *colon = strchr(p, ':');
            if (colon) {
                const char *val_start = colon + 1;
                while (*val_start && isspace((unsigned char)*val_start)) val_start++;
                const char *semi = strchr(val_start, ';');
                size_t val_len = semi ? (size_t)(semi - val_start) : strlen(val_start);
                // trim trailing spaces
                while (val_len > 0 && isspace((unsigned char)val_start[val_len - 1])) val_len--;

                if (val_len < buf_sz) {
                    memcpy(buf, val_start, val_len);
                    buf[val_len] = '\0';
                    return buf;
                }
            }
        }
        p += prop_len;
    }
    return NULL;
}

TsfiLayoutBox* tsfi_layout_tree_build(TsfiNode *dom_node) {
    if (!dom_node) return NULL;

    // Check display: none
    const char *style = tsfi_node_get_attribute(dom_node, "style");
    char buf[128];
    if (get_style_val(style, "display", buf, sizeof(buf))) {
        if (strcmp(buf, "none") == 0) {
            return NULL; // Skip this node and children
        }
    }

    TsfiLayoutBox *box = tsfi_layout_box_create(dom_node);
    if (!box) return NULL;

    // Parse display type
    if (get_style_val(style, "display", buf, sizeof(buf))) {
        if (strcmp(buf, "flex") == 0) {
            box->type = TSFI_BOX_FLEX;
        } else if (strcmp(buf, "inline") == 0) {
            box->type = TSFI_BOX_INLINE;
        } else {
            box->type = TSFI_BOX_BLOCK;
        }
    } else {
        // Default box types based on tags
        if (dom_node->type == TSFI_NODE_TEXT) {
            box->type = TSFI_BOX_INLINE;
        } else if (dom_node->tag_name && (
            strcasecmp(dom_node->tag_name, "span") == 0 ||
            strcasecmp(dom_node->tag_name, "a") == 0 ||
            strcasecmp(dom_node->tag_name, "strong") == 0 ||
            strcasecmp(dom_node->tag_name, "em") == 0 ||
            strcasecmp(dom_node->tag_name, "title") == 0)) {
            box->type = TSFI_BOX_INLINE;
        } else if (dom_node->tag_name && (
            strcasecmp(dom_node->tag_name, "video") == 0 ||
            strcasecmp(dom_node->tag_name, "audio") == 0)) {
            box->type = TSFI_BOX_VIDEO;
        } else {
            box->type = TSFI_BOX_BLOCK;
        }
    }

    // Parse margin, padding, border
    if (get_style_val(style, "margin-left", buf, sizeof(buf))) box->margin_left = parse_px(buf);
    if (get_style_val(style, "margin-right", buf, sizeof(buf))) box->margin_right = parse_px(buf);
    if (get_style_val(style, "margin-top", buf, sizeof(buf))) box->margin_top = parse_px(buf);
    if (get_style_val(style, "margin-bottom", buf, sizeof(buf))) box->margin_bottom = parse_px(buf);

    if (get_style_val(style, "padding-left", buf, sizeof(buf))) box->padding_left = parse_px(buf);
    if (get_style_val(style, "padding-right", buf, sizeof(buf))) box->padding_right = parse_px(buf);
    if (get_style_val(style, "padding-top", buf, sizeof(buf))) box->padding_top = parse_px(buf);
    if (get_style_val(style, "padding-bottom", buf, sizeof(buf))) box->padding_bottom = parse_px(buf);

    if (get_style_val(style, "border-left", buf, sizeof(buf))) box->border_left = parse_px(buf);
    if (get_style_val(style, "border-right", buf, sizeof(buf))) box->border_right = parse_px(buf);
    if (get_style_val(style, "border-top", buf, sizeof(buf))) box->border_top = parse_px(buf);
    if (get_style_val(style, "border-bottom", buf, sizeof(buf))) box->border_bottom = parse_px(buf);

    // Build child boxes
    TsfiNode *child_dom = dom_node->first_child;
    while (child_dom) {
        TsfiLayoutBox *child_box = tsfi_layout_tree_build(child_dom);
        if (child_box) {
            tsfi_layout_add_child(box, child_box);
        }
        child_dom = child_dom->next_sibling;
    }

    return box;
}

void tsfi_layout_solve(TsfiLayoutBox *box, float containing_x, float containing_y, float containing_w, float containing_h) {
    (void)containing_h;
    if (!box) return;

    const char *style = box->node ? tsfi_node_get_attribute(box->node, "style") : NULL;
    char buf[128];

    // Compute explicit sizes if present
    float explicit_w = -1.0f;
    float explicit_h = -1.0f;
    if (get_style_val(style, "width", buf, sizeof(buf))) {
        explicit_w = parse_px(buf);
    } else if (box->node && box->node->tag_name && strcasecmp(box->node->tag_name, "video") == 0) {
        const char *attr_w = tsfi_node_get_attribute(box->node, "width");
        if (attr_w) explicit_w = atof(attr_w);
    }

    if (get_style_val(style, "height", buf, sizeof(buf))) {
        explicit_h = parse_px(buf);
    } else if (box->node && box->node->tag_name && strcasecmp(box->node->tag_name, "video") == 0) {
        const char *attr_h = tsfi_node_get_attribute(box->node, "height");
        if (attr_h) explicit_h = atof(attr_h);
    }

    // Default sizing behaviors
    if (explicit_w >= 0.0f) {
        box->w = explicit_w;
    } else {
        if (box->type == TSFI_BOX_BLOCK) {
            box->w = containing_w - box->margin_left - box->margin_right;
        } else if (box->type == TSFI_BOX_VIDEO) {
            box->w = 300.0f; // HTML5 Default Video Width
        } else if (box->node && box->node->type == TSFI_NODE_TEXT) {
            box->w = strlen(box->node->text_content) * 8.0f;
        } else {
            box->w = 0.0f; // Expand content width dynamically later
        }
    }

    if (explicit_h >= 0.0f) {
        box->h = explicit_h;
    } else if (box->type == TSFI_BOX_VIDEO) {
        box->h = 150.0f; // HTML5 Default Video Height
    } else if (box->node && box->node->type == TSFI_NODE_TEXT) {
        box->h = 16.0f;
    } else {
        box->h = 0.0f;
    }

    // Solve children layouts
    float current_y = containing_y + box->margin_top + box->padding_top + box->border_top;
    float current_x = containing_x + box->margin_left + box->padding_left + box->border_left;
    float max_child_h_row = 0.0f;
    float row_current_x = current_x;
    float row_current_y = current_y;

    TsfiLayoutBox *child = box->first_child;

    if (box->type == TSFI_BOX_FLEX) {
        // Flex Layout (Horizontal alignment with wrapping if child overflows containing width)
        float flex_limit_w = box->w - box->padding_left - box->padding_right;
        while (child) {
            // Child coordinates start at current row position
            tsfi_layout_solve(child, row_current_x, row_current_y, flex_limit_w, box->h);
            
            float child_outer_w = child->w + child->margin_left + child->margin_right + child->padding_left + child->padding_right + child->border_left + child->border_right;
            float child_outer_h = child->h + child->margin_top + child->margin_bottom + child->padding_top + child->padding_bottom + child->border_top + child->border_bottom;
            
            // Wrap to next flex line if width exceeded
            if (row_current_x + child_outer_w > current_x + flex_limit_w && row_current_x > current_x) {
                row_current_x = current_x;
                row_current_y += max_child_h_row;
                row_current_y += 10.0f; // Gap between rows
                max_child_h_row = 0.0f;
                // Re-solve at wrapped coordinates
                tsfi_layout_solve(child, row_current_x, row_current_y, flex_limit_w, box->h);
                child_outer_w = child->w + child->margin_left + child->margin_right + child->padding_left + child->padding_right + child->border_left + child->border_right;
                child_outer_h = child->h + child->margin_top + child->margin_bottom + child->padding_top + child->padding_bottom + child->border_top + child->border_bottom;
            }

            child->x = row_current_x + child->margin_left;
            child->y = row_current_y + child->margin_top;

            row_current_x += child_outer_w;
            if (child_outer_h > max_child_h_row) {
                max_child_h_row = child_outer_h;
            }
            child = child->next_sibling;
        }
        
        if (explicit_h >= 0.0f) {
            box->h = explicit_h;
        } else {
            box->h = (row_current_y + max_child_h_row) - (containing_y + box->margin_top);
        }
    } else {
        // Standard Block / Flow Layout
        float content_h = 0.0f;
        float line_w = 0.0f;
        float line_h = 0.0f;
        float inline_x = current_x;
        float inline_y = current_y;

        while (child) {
            if (child->type == TSFI_BOX_INLINE) {
                // First pass solve to let it compute its own content width/height recursively
                tsfi_layout_solve(child, inline_x + child->margin_left, inline_y + child->margin_top, box->w - box->padding_left - box->padding_right, box->h);

                float child_outer_w = child->w + child->margin_left + child->margin_right + child->padding_left + child->padding_right + child->border_left + child->border_right;
                float child_outer_h = child->h + child->margin_top + child->margin_bottom + child->padding_top + child->padding_bottom + child->border_top + child->border_bottom;

                // Check line wrap boundaries
                float limit_w = box->w - box->padding_left - box->padding_right;
                if (line_w + child_outer_w > limit_w && line_w > 0.0f) {
                    inline_x = current_x;
                    inline_y += line_h;
                    content_h += line_h;
                    line_w = 0.0f;
                    line_h = 0.0f;

                    // Re-solve child at wrapped coordinates
                    tsfi_layout_solve(child, inline_x + child->margin_left, inline_y + child->margin_top, box->w - box->padding_left - box->padding_right, box->h);
                } else {
                    // Update child absolute coordinates
                    child->x = inline_x + child->margin_left;
                    child->y = inline_y + child->margin_top;
                }

                inline_x += child_outer_w;
                line_w += child_outer_w;
                if (child_outer_h > line_h) {
                    line_h = child_outer_h;
                }

                TsfiLayoutBox *next = child->next_sibling;
                if (!next || next->type != TSFI_BOX_INLINE) {
                    content_h += line_h;
                    current_y = inline_y + line_h;
                    line_w = 0.0f;
                    line_h = 0.0f;
                }
                child = next;
            } else {
                // Block element: vertical stack.
                // However, check if block has explicit narrow width relative to parent and next is also narrow, we can arrange them side-by-side like a grid
                float limit_w = box->w - box->padding_left - box->padding_right;
                float child_explicit_w = child->w + child->margin_left + child->margin_right + child->padding_left + child->padding_right + child->border_left + child->border_right;
                
                // Check if child has a class or style hinting at being a grid item (like video cards)
                bool side_by_side = false;
                if (child->w > 0 && child->w < limit_w * 0.6f && child->next_sibling && child->next_sibling->w > 0 && child->next_sibling->w < limit_w * 0.6f) {
                    side_by_side = true;
                }

                if (side_by_side && (row_current_x + child_explicit_w <= current_x + limit_w)) {
                    // Position side-by-side
                    tsfi_layout_solve(child, row_current_x, row_current_y, limit_w, box->h);
                    child->x = row_current_x + child->margin_left;
                    child->y = row_current_y + child->margin_top;
                    
                    float child_outer_h = child->h + child->margin_top + child->margin_bottom + child->padding_top + child->padding_bottom + child->border_top + child->border_bottom;
                    row_current_x += child_explicit_w;
                    if (child_outer_h > max_child_h_row) {
                        max_child_h_row = child_outer_h;
                    }
                    current_y = row_current_y + max_child_h_row;
                } else if (side_by_side) {
                    // Wrap to next line
                    row_current_x = current_x;
                    row_current_y += max_child_h_row;
                    max_child_h_row = 0.0f;
                    
                    tsfi_layout_solve(child, row_current_x, row_current_y, limit_w, box->h);
                    child->x = row_current_x + child->margin_left;
                    child->y = row_current_y + child->margin_top;
                    
                    float child_outer_h = child->h + child->margin_top + child->margin_bottom + child->padding_top + child->padding_bottom + child->border_top + child->border_bottom;
                    row_current_x += child_explicit_w;
                    max_child_h_row = child_outer_h;
                    current_y = row_current_y + max_child_h_row;
                } else {
                    // Reset grid line variables
                    if (row_current_x > current_x) {
                        current_y = row_current_y + max_child_h_row;
                        row_current_x = current_x;
                        row_current_y = current_y;
                        max_child_h_row = 0.0f;
                    }
                    tsfi_layout_solve(child, current_x, current_y, limit_w, box->h);
                    child->x = current_x + child->margin_left;
                    child->y = current_y + child->margin_top;

                    float child_outer_h = child->h + child->margin_top + child->margin_bottom + child->padding_top + child->padding_bottom + child->border_top + child->border_bottom;
                    current_y += child_outer_h;
                    row_current_y = current_y;
                }
                
                content_h = current_y - (containing_y + box->margin_top + box->padding_top + box->border_top);
                child = child->next_sibling;
            }
        }

        if (explicit_h >= 0.0f) {
            box->h = explicit_h;
        } else {
            box->h = content_h + box->padding_top + box->padding_bottom + box->border_top + box->border_bottom;
        }
    }

    // Resolve wrapper inline container dimensions based on child elements (skip text nodes)
    if (box->type == TSFI_BOX_INLINE && box->node && box->node->type != TSFI_NODE_TEXT) {
        if (explicit_w < 0.0f) {
            float total_w = 0.0f;
            TsfiLayoutBox *c = box->first_child;
            while (c) {
                total_w += c->w + c->margin_left + c->margin_right + c->padding_left + c->padding_right + c->border_left + c->border_right;
                c = c->next_sibling;
            }
            box->w = total_w;
        }
        if (explicit_h < 0.0f) {
            float max_h = 0.0f;
            TsfiLayoutBox *c = box->first_child;
            while (c) {
                float ch = c->h + c->margin_top + c->margin_bottom + c->padding_top + c->padding_bottom + c->border_top + c->border_bottom;
                if (ch > max_h) max_h = ch;
                c = c->next_sibling;
            }
            box->h = max_h > 0.0f ? max_h : 16.0f;
        }
    }

    // Save final absolute coords including margins
    box->x = containing_x + box->margin_left;
    box->y = containing_y + box->margin_top;
}

void tsfi_layout_print(const TsfiLayoutBox *box, int depth) {
    if (!box) return;
    for (int i = 0; i < depth; i++) printf("  ");
    printf("[%s] node=%s (x:%.1f, y:%.1f, w:%.1f, h:%.1f)\n",
           box->type == TSFI_BOX_BLOCK ? "BLOCK" : (box->type == TSFI_BOX_FLEX ? "FLEX" : "INLINE"),
           box->node && box->node->tag_name ? box->node->tag_name : (box->node && box->node->text_content ? "TEXT" : "DOC"),
           box->x, box->y, box->w, box->h);

    TsfiLayoutBox *child = box->first_child;
    while (child) {
        tsfi_layout_print(child, depth + 1);
        child = child->next_sibling;
    }
}
