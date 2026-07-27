#define _GNU_SOURCE
#include "tsfi_dom.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static void test_basic_structure(void) {
    printf("[Test] Running basic structure test...\n");
    const char *html = "<html><head><title>Test Title</title></head><body><div><p>Hello</p></div></body></html>";
    TsfiNode *doc = tsfi_dom_parse_html(html);
    assert(doc != NULL);
    assert(doc->type == TSFI_NODE_DOCUMENT);

    TsfiNode *html_node = doc->first_child;
    assert(html_node != NULL && strcmp(html_node->tag_name, "html") == 0);

    TsfiNode *head = html_node->first_child;
    assert(head != NULL && strcmp(head->tag_name, "head") == 0);

    TsfiNode *title = head->first_child;
    assert(title != NULL && strcmp(title->tag_name, "title") == 0);

    TsfiNode *title_text = title->first_child;
    assert(title_text != NULL && title_text->type == TSFI_NODE_TEXT);
    assert(strcmp(title_text->text_content, "Test Title") == 0);

    TsfiNode *body = head->next_sibling;
    assert(body != NULL && strcmp(body->tag_name, "body") == 0);

    tsfi_node_free(doc);
    printf("[Test] Basic structure test passed!\n");
}

static void test_attributes(void) {
    printf("[Test] Running attributes test...\n");
    const char *html = "<div id=\"main\" class='container' data-custom=value style=\"color: red;\"></div>";
    TsfiNode *doc = tsfi_dom_parse_html(html);
    assert(doc != NULL);

    TsfiNode *div = doc->first_child;
    assert(div != NULL && strcmp(div->tag_name, "div") == 0);

    const char *id = tsfi_node_get_attribute(div, "id");
    assert(id != NULL && strcmp(id, "main") == 0);

    const char *class_attr = tsfi_node_get_attribute(div, "class");
    assert(class_attr != NULL && strcmp(class_attr, "container") == 0);

    const char *data = tsfi_node_get_attribute(div, "data-custom");
    assert(data != NULL && strcmp(data, "value") == 0);

    const char *style = tsfi_node_get_attribute(div, "style");
    assert(style != NULL && strcmp(style, "color: red;") == 0);

    tsfi_node_free(doc);
    printf("[Test] Attributes test passed!\n");
}

static void test_self_closing_and_comments(void) {
    printf("[Test] Running self-closing tags and comments test...\n");
    const char *html = "<!-- This is a comment --><div><br><img src=\"test.jpg\" /> <hr> </div>";
    TsfiNode *doc = tsfi_dom_parse_html(html);
    assert(doc != NULL);

    TsfiNode *div = doc->first_child;
    assert(div != NULL && strcmp(div->tag_name, "div") == 0);

    TsfiNode *br = div->first_child;
    assert(br != NULL && strcmp(br->tag_name, "br") == 0);
    // br should have no children and next sibling should be img
    assert(br->first_child == NULL);

    TsfiNode *img = br->next_sibling;
    assert(img != NULL && strcmp(img->tag_name, "img") == 0);
    assert(strcmp(tsfi_node_get_attribute(img, "src"), "test.jpg") == 0);

    TsfiNode *hr = img->next_sibling;
    assert(hr != NULL && strcmp(hr->tag_name, "hr") == 0);

    tsfi_node_free(doc);
    printf("[Test] Self-closing tags and comments test passed!\n");
}

static void test_malformed_html(void) {
    printf("[Test] Running malformed HTML resilience test...\n");
    // Nested missing closing tags
    const char *html = "<div><p>Unclosed paragraph <div>Next Div";
    TsfiNode *doc = tsfi_dom_parse_html(html);
    assert(doc != NULL);

    // Verify parser handles unclosed tags gracefully without crashing
    TsfiNode *div = doc->first_child;
    assert(div != NULL && strcmp(div->tag_name, "div") == 0);

    tsfi_node_free(doc);
    printf("[Test] Malformed HTML resilience test passed!\n");
}

int main(void) {
    printf("=== Starting Phase 1 DOM Unit Tests ===\n");
    test_basic_structure();
    test_attributes();
    test_self_closing_and_comments();
    test_malformed_html();
    printf("=== All Phase 1 DOM Unit Tests Passed Successfully ===\n");
    return 0;
}
