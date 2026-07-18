#ifndef TSFI_MAINFRAME_COMPUTERWORLD_H
#define TSFI_MAINFRAME_COMPUTERWORLD_H

#include <stdint.h>
#include <stddef.h>

// Y2K Date Windowing Strategy: Pivot year 50 (Years < 50 are 2000s, >= 50 are 1900s)
uint32_t tsfi_cw_y2k_resolve_year(uint32_t two_digit_year);
int tsfi_cw_y2k_check_date(uint32_t yy, uint32_t mm, uint32_t dd, int *is_valid);

// IBM 80-Column Punch Card Record Parser (enforcing Area A / Area B boundary rules)
typedef struct {
    char sequence[7];       // Cols 1-6
    char indicator;         // Col 7 (e.g. '*', '-', 'D', '/' or space)
    char area_a[5];         // Cols 8-11
    char area_b[62];        // Cols 12-72
    char identification[9]; // Cols 73-80
} tsfi_cw_punch_card;

int tsfi_cw_parse_punch_card(const char *card_line, tsfi_cw_punch_card *card_out);

// EBCDIC <-> ASCII Translation (IBM Code Page 037/1047 standard subset)
uint8_t tsfi_cw_ascii_to_ebcdic(uint8_t ascii_char);
uint8_t tsfi_cw_ebcdic_to_ascii(uint8_t ebcdic_char);

int tsfi_cw_ascii_to_ebcdic_buf(const char *ascii_in, uint8_t *ebcdic_out, int len);
int tsfi_cw_ebcdic_to_ascii_buf(const uint8_t *ebcdic_in, char *ascii_out, int len);

// COBOL COMP-3 Packed Decimal Standard Emulation
int tsfi_cw_pack_comp3(const char *ascii_num, uint8_t *comp3_out, int max_out_len, int *out_len);
int tsfi_cw_unpack_comp3(const uint8_t *comp3_in, int comp3_len, char *ascii_out, int max_ascii_len);

// 1. IBM Hexadecimal Floating-Point (HFP) Conversions
float tsfi_cw_ibm_to_ieee_float(uint32_t ibm_float);
uint32_t tsfi_cw_ieee_to_ibm_float(float ieee_float);

// 2. COBOL Copybook Parsing & Structured Data Layouts
typedef enum {
    COBOL_TYPE_ALPHA,
    COBOL_TYPE_NUMERIC,
    COBOL_TYPE_COMP3,
    COBOL_TYPE_COMP5
} tsfi_cw_cobol_type;

typedef struct {
    int level;
    char name[32];
    tsfi_cw_cobol_type type;
    int length;
    int decimal_places;
    int offset;
    char value[32];
    char redefines[32];
    int occurs;
} tsfi_cw_cobol_field;

typedef struct {
    tsfi_cw_cobol_field fields[16];
    int field_count;
    int record_length;
} tsfi_cw_copybook;

int tsfi_cw_parse_copybook_line(const char *copybook_line, tsfi_cw_copybook *cb);

// Dynamic Y2K date windowing functions
uint32_t tsfi_cw_y2k_resolve_year_ex(uint32_t two_digit_year, uint32_t pivot);
int tsfi_cw_y2k_check_date_ex(uint32_t yy, uint32_t mm, uint32_t dd, uint32_t pivot, int *is_valid);

// EBCDIC Control Character mappings
int tsfi_cw_ebcdic_to_ascii_control(uint8_t ebcdic_char, uint8_t *ascii_char_out);

// Julian date conversion utilities
int tsfi_cw_gregorian_to_julian(uint32_t yy, uint32_t mm, uint32_t dd, char *julian_out, int max_len);
int tsfi_cw_julian_to_gregorian(const char *julian_in, uint32_t *yy_out, uint32_t *mm_out, uint32_t *dd_out);

// EBCDIC record padding/stripping
int tsfi_cw_ebcdic_pad_record(const char *ascii_in, uint8_t *ebcdic_out, int target_len);
int tsfi_cw_ebcdic_strip_record(const uint8_t *ebcdic_in, int input_len, char *ascii_out, int max_ascii_len);

// Punch Card helpers
int tsfi_cw_card_is_continuation(const tsfi_cw_punch_card *card);
int tsfi_cw_card_is_comment(const tsfi_cw_punch_card *card);

// 3. VSAM Key-Sequenced Data Set (KSDS) Index Emulator
typedef struct {
    char key[16];
    uint32_t offset;
    uint32_t length;
    uint8_t active;
} tsfi_cw_vsam_entry;

typedef struct {
    char filepath[256];
    tsfi_cw_vsam_entry index[128];
    int entry_count;
    uint32_t current_file_size;
} tsfi_cw_vsam_ksds;

int tsfi_cw_vsam_open(tsfi_cw_vsam_ksds *ksds, const char *filepath);
int tsfi_cw_vsam_write(tsfi_cw_vsam_ksds *ksds, const char *key, const uint8_t *data, int len);
int tsfi_cw_vsam_read(tsfi_cw_vsam_ksds *ksds, const char *key, uint8_t *data_out, int max_len, int *out_len);

// VSAM Control Interval Split Emulator
typedef struct {
    uint32_t ci_id;
    uint32_t record_count;
    char keys[4][16];
} tsfi_cw_vsam_ci;

typedef struct {
    tsfi_cw_vsam_ci cis[8];
    int ci_count;
} tsfi_cw_vsam_ci_set;

void tsfi_cw_vsam_ci_init(tsfi_cw_vsam_ci_set *set);
int tsfi_cw_vsam_ci_insert(tsfi_cw_vsam_ci_set *set, uint32_t ci_idx, const char *key);

// VSAM Alternate Index (AIX) Emulator
typedef struct {
    char alt_key[16];
    char primary_key[16];
    uint8_t active;
} tsfi_cw_vsam_aix_entry;

typedef struct {
    tsfi_cw_vsam_aix_entry entries[64];
    int entry_count;
} tsfi_cw_vsam_aix;

void tsfi_cw_vsam_aix_init(tsfi_cw_vsam_aix *aix);
int tsfi_cw_vsam_aix_add(tsfi_cw_vsam_aix *aix, const char *alt_key, const char *primary_key);
int tsfi_cw_vsam_aix_resolve(tsfi_cw_vsam_aix *aix, const char *alt_key, char *primary_key_out);

// JCL Dataset Dispositions
typedef struct {
    char dsn[64];
    char status[8];
    char normal[8];
    char abnormal[8];
} tsfi_cw_jcl_disp;

int tsfi_cw_jcl_parse_disp(const char *disp_str, tsfi_cw_jcl_disp *disp_out);

// 4. Job Control Language (JCL) Execution Simulator
int tsfi_cw_run_jcl(const char **cards, int card_count);
int tsfi_cw_run_jcl_ex(const char **cards, int card_count, int initial_rc);
int tsfi_cw_run_jcl_sysin(const char **cards, int card_count, char *sysin_out, int max_sysin_len);

#endif // TSFI_MAINFRAME_COMPUTERWORLD_H

