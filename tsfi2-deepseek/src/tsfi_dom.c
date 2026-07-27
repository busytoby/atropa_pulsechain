#define _GNU_SOURCE
#include "tsfi_dom.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

TsfiNode* tsfi_node_create(TsfiNodeType type) {
    TsfiNode *node = (TsfiNode*)calloc(1, sizeof(TsfiNode));
    if (node) {
        node->type = type;
    }
    return node;
}

void tsfi_node_free(TsfiNode *node) {
    if (!node) return;

    // Free children recursively
    TsfiNode *child = node->first_child;
    while (child) {
        TsfiNode *next = child->next_sibling;
        tsfi_node_free(child);
        child = next;
    }

    // Free attributes
    TsfiAttribute *attr = node->attributes;
    while (attr) {
        TsfiAttribute *next = attr->next;
        free(attr->name);
        free(attr->value);
        free(attr);
        attr = next;
    }

    free(node->tag_name);
    free(node->text_content);
    free(node);
}

void tsfi_node_add_child(TsfiNode *parent, TsfiNode *child) {
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

void tsfi_node_set_attribute(TsfiNode *node, const char *name, const char *value) {
    if (!node || !name) return;

    // Check if attribute already exists
    TsfiAttribute *attr = node->attributes;
    while (attr) {
        if (strcmp(attr->name, name) == 0) {
            free(attr->value);
            attr->value = strdup(value ? value : "");
            return;
        }
        attr = attr->next;
    }

    // Otherwise insert new attribute
    TsfiAttribute *new_attr = (TsfiAttribute*)malloc(sizeof(TsfiAttribute));
    if (new_attr) {
        new_attr->name = strdup(name);
        new_attr->value = strdup(value ? value : "");
        new_attr->next = node->attributes;
        node->attributes = new_attr;
    }
}

const char* tsfi_node_get_attribute(const TsfiNode *node, const char *name) {
    if (!node || !name) return NULL;
    const TsfiAttribute *attr = node->attributes;
    while (attr) {
        if (strcmp(attr->name, name) == 0) {
            return attr->value;
        }
        attr = attr->next;
    }
    return NULL;
}

void tsfi_dom_print(const TsfiNode *node, int depth) {
    if (!node) return;

    for (int i = 0; i < depth; i++) printf("  ");

    if (node->type == TSFI_NODE_DOCUMENT) {
        printf("[DOCUMENT]\n");
    } else if (node->type == TSFI_NODE_ELEMENT) {
        printf("<%s", node->tag_name);
        TsfiAttribute *attr = node->attributes;
        while (attr) {
            printf(" %s=\"%s\"", attr->name, attr->value);
            attr = attr->next;
        }
        printf(">\n");
    } else if (node->type == TSFI_NODE_TEXT) {
        // Strip newlines or print nicely
        printf("\"%s\"\n", node->text_content);
    }

    TsfiNode *child = node->first_child;
    while (child) {
        tsfi_dom_print(child, depth + 1);
        child = child->next_sibling;
    }

    if (node->type == TSFI_NODE_ELEMENT) {
        for (int i = 0; i < depth; i++) printf("  ");
        printf("</%s>\n", node->tag_name);
    }
}

// Simple helper to skip whitespace
static const char* skip_ws(const char *p) {
    while (*p && isspace((unsigned char)*p)) p++;
    return p;
}

// Parses attributes from a tag string like 'id="foo" class="bar"'
static void parse_attributes(TsfiNode *node, const char *start, const char *end) {
    const char *p = start;
    while (p < end) {
        p = skip_ws(p);
        if (p >= end) break;

        // Parse attribute name
        const char *name_start = p;
        while (p < end && *p != '=' && !isspace((unsigned char)*p) && *p != '/' && *p != '>') p++;
        size_t name_len = p - name_start;
        if (name_len == 0) break;

        char *name = strndup(name_start, name_len);

        p = skip_ws(p);
        char *value = NULL;
        if (p < end && *p == '=') {
            p++; // skip '='
            p = skip_ws(p);
            if (p < end && (*p == '"' || *p == '\'')) {
                char quote = *p;
                p++; // skip quote
                const char *val_start = p;
                while (p < end && *p != quote) p++;
                value = strndup(val_start, p - val_start);
                if (p < end) p++; // skip closing quote
            } else {
                // Unquoted attribute value
                const char *val_start = p;
                while (p < end && !isspace((unsigned char)*p) && *p != '/' && *p != '>') p++;
                value = strndup(val_start, p - val_start);
            }
        }

        tsfi_node_set_attribute(node, name, value ? value : "");
        free(name);
        free(value);
    }
}

TsfiNode* tsfi_dom_parse_html(const char *html_str) {
    if (!html_str) return NULL;

    TsfiNode *doc = tsfi_node_create(TSFI_NODE_DOCUMENT);
    TsfiNode *current = doc;

    const char *p = html_str;

    while (*p) {
        // Find next tag
        const char *tag_open = strchr(p, '<');
        if (!tag_open) {
            // Treat remaining as text node if not empty
            p = skip_ws(p);
            if (*p) {
                TsfiNode *txt = tsfi_node_create(TSFI_NODE_TEXT);
                txt->text_content = strdup(p);
                tsfi_node_add_child(current, txt);
            }
            break;
        }

        // Handle text before tag
        if (tag_open > p) {
            // Trim whitespace for simplicity, but standard HTML preserves it.
            // Let's copy it and skip if it is purely empty whitespace.
            size_t txt_len = tag_open - p;
            char *raw_txt = strndup(p, txt_len);
            // check if non-whitespace
            bool non_ws = false;
            for (size_t i = 0; i < txt_len; i++) {
                if (!isspace((unsigned char)raw_txt[i])) {
                    non_ws = true;
                    break;
                }
            }
            if (non_ws) {
                TsfiNode *txt = tsfi_node_create(TSFI_NODE_TEXT);
                txt->text_content = raw_txt;
                tsfi_node_add_child(current, txt);
            } else {
                free(raw_txt);
            }
        }

        p = tag_open + 1;

        if (*p == '!') {
            // Skip comments and doctype
            if (strncmp(p, "!--", 3) == 0) {
                const char *comment_end = strstr(p, "-->");
                if (comment_end) {
                    p = comment_end + 3;
                } else {
                    p = p + strlen(p); // Go to end
                }
            } else {
                const char *tag_close = strchr(p, '>');
                if (tag_close) {
                    p = tag_close + 1;
                } else {
                    p = p + strlen(p);
                }
            }
            continue;
        }

        bool is_closing = false;
        if (*p == '/') {
            is_closing = true;
            p++;
        }

        const char *tag_close = strchr(p, '>');
        if (!tag_close) {
            // Malformed tag
            break;
        }

        // Get tag content
        const char *tag_end = tag_close;
        bool is_self_closing = false;
        if (*(tag_end - 1) == '/') {
            is_self_closing = true;
            tag_end--;
        }

        // Parse tag name
        p = skip_ws(p);
        const char *name_start = p;
        while (p < tag_end && !isspace((unsigned char)*p)) p++;
        size_t name_len = p - name_start;

        if (name_len > 0) {
            char *tag_name = strndup(name_start, name_len);

            if (is_closing) {
                // Climb up parent stack if tags match
                if (current->parent && strcasecmp(current->tag_name, tag_name) == 0) {
                    current = current->parent;
                }
                free(tag_name);
            } else {
                TsfiNode *elem = tsfi_node_create(TSFI_NODE_ELEMENT);
                elem->tag_name = tag_name;

                // Parse attributes
                parse_attributes(elem, p, tag_end);

                tsfi_node_add_child(current, elem);

                // Handle self-closing elements (like <br>, <img> or explicit <tag/>)
                bool auto_self = (strcasecmp(tag_name, "br") == 0 ||
                                  strcasecmp(tag_name, "img") == 0 ||
                                  strcasecmp(tag_name, "hr") == 0 ||
                                  strcasecmp(tag_name, "meta") == 0 ||
                                  strcasecmp(tag_name, "link") == 0 ||
                                  strcasecmp(tag_name, "input") == 0);

                if (!is_self_closing && !auto_self) {
                    current = elem;
                }
            }
        }

        p = tag_close + 1;
    }

    return doc;
}
