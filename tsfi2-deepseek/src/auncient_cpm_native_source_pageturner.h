#ifndef AUNCIENT_CPM_NATIVE_SOURCE_PAGETURNER_H
#define AUNCIENT_CPM_NATIVE_SOURCE_PAGETURNER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define NATIVE_PAGETURNER_TPA_BASE 0x0100
#define MAX_SOURCE_LINES_PER_PAGE 40
#define MAX_LINE_CHARACTERS 80

typedef enum {
    TOKEN_KEYWORD,
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_OPERATOR,
    TOKEN_STRING,
    TOKEN_COMMENT,
    TOKEN_UNKNOWN
} SourceTokenType;

typedef struct {
    char line_buffer[MAX_LINE_CHARACTERS];
    uint32_t line_number;
    uint32_t tokens_count;
    uint32_t syntax_color_ansi;
} RenderedSourceLine;

typedef struct {
    char current_file_path[128];
    uint32_t current_page;
    uint32_t total_pages;
    uint32_t total_source_lines;
    RenderedSourceLine page_lines[MAX_SOURCE_LINES_PER_PAGE];
    bool is_algol61_file;
    bool is_cobol_strategy_file;
    bool live_ast_verification_passed;
    uint32_t generated_witness_seal;
    uint32_t rule18_parity_checksum;
} NativeSourcePageTurner;

void auncient_native_pageturner_init(NativeSourcePageTurner *pt);
bool auncient_native_pageturner_load_file(NativeSourcePageTurner *pt, const char *file_path);
bool auncient_native_pageturner_render_page(NativeSourcePageTurner *pt, uint32_t page_num, char *rib_output_buf, size_t rib_buf_size);
uint32_t auncient_native_pageturner_compute_rule18(const NativeSourcePageTurner *pt);

#endif /* AUNCIENT_CPM_NATIVE_SOURCE_PAGETURNER_H */
