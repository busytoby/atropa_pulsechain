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

// Zoned decimal sign encoding
int tsfi_cw_pack_zoned(const char *ascii_num, uint8_t *zoned_out, int max_out_len, int *out_len);
int tsfi_cw_unpack_zoned(const uint8_t *zoned_in, int zoned_len, char *ascii_out, int max_ascii_len);

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

typedef enum {
    COBOL_USAGE_DISPLAY,
    COBOL_USAGE_COMP,
    COBOL_USAGE_COMP3,
    COBOL_USAGE_COMP5
} tsfi_cw_cobol_usage;

typedef struct {
    int level;
    char name[32];
    tsfi_cw_cobol_type type;
    tsfi_cw_cobol_usage usage;
    int length;
    int decimal_places;
    int offset;
    char value[32];
    char redefines[32];
    int occurs;
    char depending_on[32];
    char indexed_by[32];
    char renames_start[32];
    char renames_end[32];
    int blank_when_zero;
    int justified_right;
    int sync_align;
    int sign_leading;
    int sign_separate;
    char occurs_depending_on[32];
} tsfi_cw_cobol_field;

typedef struct {
    tsfi_cw_cobol_field fields[32];
    int field_count;
    int record_length;
} tsfi_cw_copybook;

int tsfi_cw_parse_copybook_line(const char *copybook_line, tsfi_cw_copybook *cb);
int tsfi_cw_cobol_get_dynamic_record_length(tsfi_cw_copybook *cb, const char *dep_field_name, int dep_field_value);

// Dynamic Y2K date windowing functions
uint32_t tsfi_cw_y2k_resolve_year_ex(uint32_t two_digit_year, uint32_t pivot);
int tsfi_cw_y2k_check_date_ex(uint32_t yy, uint32_t mm, uint32_t dd, uint32_t pivot, int *is_valid);

// EBCDIC Control Character mappings
int tsfi_cw_ebcdic_to_ascii_control(uint8_t ebcdic_char, uint8_t *ascii_char_out);

// EBCDIC CP 285 (UK National) translation
uint8_t tsfi_cw_ascii_to_ebcdic_cp285(uint8_t ascii_char);
uint8_t tsfi_cw_ebcdic_to_ascii_cp285(uint8_t ebcdic_char);

// Century leap boundary rule checks
int tsfi_cw_y2k_is_century_leap(uint32_t year);

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
    uint8_t lock_state;
    uint32_t lock_attempts;
    uint32_t checksum;
} tsfi_cw_vsam_entry;

typedef struct {
    char filepath[256];
    tsfi_cw_vsam_entry index[128];
    int entry_count;
    uint32_t current_file_size;
    uint32_t ci_splits;
    uint32_t raw_bytes_written;
    uint32_t compressed_bytes_written;
    uint32_t raw_key_bytes;
    uint32_t compressed_key_bytes;
    char cache_keys[4][16];
    int cache_hits;
    uint32_t key_prefix_savings;
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

// VSAM Control Area (CA) Split Emulator
typedef struct {
    tsfi_cw_vsam_ci_set cis_sets[4];
    int ca_count;
} tsfi_cw_vsam_ca_set;

void tsfi_cw_vsam_ca_init(tsfi_cw_vsam_ca_set *set);
int tsfi_cw_vsam_ca_insert(tsfi_cw_vsam_ca_set *set, uint32_t ca_idx, const char *key);

// JCL Dataset Dispositions
typedef struct {
    char dsn[64];
    char status[8];
    char normal[8];
    char abnormal[8];
} tsfi_cw_jcl_disp;

int tsfi_cw_jcl_parse_disp(const char *disp_str, tsfi_cw_jcl_disp *disp_out);

// Multi-century Y2K helper
uint32_t tsfi_cw_y2k_resolve_year_multi(uint32_t two_digit_year, uint32_t century_prefix);
int tsfi_cw_parse_multi_format_date(const char *date_str, const char *format, uint32_t *yy, uint32_t *mm, uint32_t *dd);

// Dynamic EBCDIC tables
void tsfi_cw_set_custom_translation_tables(const uint8_t *ascii_to_ebcdic, const uint8_t *ebcdic_to_ascii);

// VSAM Entry-Sequenced Data Sets (ESDS) Emulator
typedef struct {
    uint32_t rba;
    uint32_t length;
    uint8_t active;
} tsfi_cw_vsam_esds_entry;

typedef struct {
    tsfi_cw_vsam_esds_entry entries[64];
    char filepath[256];
    uint32_t current_rba;
    int entry_count;
} tsfi_cw_vsam_esds;

void tsfi_cw_vsam_esds_init(tsfi_cw_vsam_esds *esds, const char *path);
int tsfi_cw_vsam_esds_write(tsfi_cw_vsam_esds *esds, const uint8_t *data, int len, uint32_t *rba_out);
int tsfi_cw_vsam_esds_read(tsfi_cw_vsam_esds *esds, uint32_t rba, uint8_t *data_out, int max_len, int *out_len);

// VSAM Key-Range Partitioning (KRDS) Emulator
typedef struct {
    char range_start[16];
    char range_end[16];
    char filepath[256];
} tsfi_cw_vsam_krds_partition;

typedef struct {
    tsfi_cw_vsam_krds_partition partitions[4];
    int partition_count;
} tsfi_cw_vsam_krds;

void tsfi_cw_vsam_krds_init(tsfi_cw_vsam_krds *krds);
int tsfi_cw_vsam_krds_add_partition(tsfi_cw_vsam_krds *krds, const char *start, const char *end, const char *path);
const char *tsfi_cw_vsam_krds_resolve(tsfi_cw_vsam_krds *krds, const char *key);

// Fixed-Block (FB 80) record blocking
int tsfi_cw_block_fb80(const char *unix_stream, uint8_t *block_out, int max_block_size, int *records_blocked);

// Leap Second adjustments
int tsfi_cw_y2k_adjust_leap_seconds(uint32_t year, int *seconds_offset);

// VSAM Relative Record Data Sets (RRDS) Emulator
typedef struct {
    uint32_t rrn;
    uint32_t length;
    uint8_t active;
} tsfi_cw_vsam_rrds_entry;

typedef struct {
    tsfi_cw_vsam_rrds_entry slots[64];
    char filepath[256];
    int max_rrn;
    uint64_t occupancy_map;
} tsfi_cw_vsam_rrds;

void tsfi_cw_vsam_rrds_init(tsfi_cw_vsam_rrds *rrds, const char *path);
int tsfi_cw_vsam_rrds_write(tsfi_cw_vsam_rrds *rrds, uint32_t rrn, const uint8_t *data, int len);
int tsfi_cw_vsam_rrds_read(tsfi_cw_vsam_rrds *rrds, uint32_t rrn, uint8_t *data_out, int max_len, int *out_len);
int tsfi_cw_vsam_rrds_is_occupied(tsfi_cw_vsam_rrds *rrds, uint32_t rrn);

// JCL Procedure Variable Substitutions
int tsfi_cw_run_jcl_set(const char **cards, int card_count, char *jcl_out, int max_jcl_len);

// VSAM Key Compression Emulator
int tsfi_cw_vsam_compress_key(const char *key, const char *prev_key, char *compressed_out, int max_len);
int tsfi_cw_vsam_decompress_key(const char *compressed, const char *prev_key, char *decompressed_out, int max_len);

// JCL Temporary Workspace pool
typedef struct {
    char name[32];
    char filepath[256];
    int active;
} tsfi_cw_jcl_temp_ds;

typedef struct {
    tsfi_cw_jcl_temp_ds datasets[8];
    int count;
} tsfi_cw_jcl_temp_pool;

void tsfi_cw_jcl_temp_pool_init(tsfi_cw_jcl_temp_pool *pool);
int tsfi_cw_jcl_temp_pool_add(tsfi_cw_jcl_temp_pool *pool, const char *name, const char *path);
const char *tsfi_cw_jcl_temp_pool_get(tsfi_cw_jcl_temp_pool *pool, const char *name);

// Packed Decimal decimal-alignment
int tsfi_cw_align_comp3_fractional(const char *ascii_num, int decimal_places, char *aligned_out, int max_len);

// Julian date century standardizer
int tsfi_cw_julian_standardize(const char *julian_in, uint32_t pivot, char *julian_out, int max_len);

// VSAM Linear Data Sets (LDS) Emulator
typedef struct {
    uint8_t pages[4][4096];
    char filepath[256];
    int page_count;
} tsfi_cw_vsam_lds;

void tsfi_cw_vsam_lds_init(tsfi_cw_vsam_lds *lds, const char *path);
int tsfi_cw_vsam_lds_write_page(tsfi_cw_vsam_lds *lds, int page_idx, const uint8_t *page_data);
int tsfi_cw_vsam_lds_read_page(tsfi_cw_vsam_lds *lds, int page_idx, uint8_t *page_data_out);

// EBCDIC Double-Byte Character Set (DBCS) CP930
int tsfi_cw_ebcdic_is_dbcs(const uint8_t *ebcdic_str, int len, int *dbcs_count);

// Century leap check 2100
int tsfi_cw_y2k_check_century_leap_2100(uint32_t year, int *is_leap);

// COBOL SIGN SEPARATE (Display Sign) formats
int tsfi_cw_pack_sign_separate(const char *ascii_num, char *separate_out, int max_len, int leading);
int tsfi_cw_unpack_sign_separate(const char *separate_in, char *ascii_out, int max_len, int leading);

// JCL DD statement concatenations
int tsfi_cw_run_jcl_concat(const char **cards, int card_count, char *concat_out, int max_out_len);

// Dynamic Century Epoch offset mapping
uint32_t tsfi_cw_y2k_resolve_epoch_base(uint32_t two_digit_year, uint32_t base_epoch);

// VSAM Partitioned Data Sets (PDS) Emulator
typedef struct {
    char name[12];
    uint32_t offset;
    uint32_t length;
    int active;
} tsfi_cw_pds_member;

typedef struct {
    tsfi_cw_pds_member members[16];
    char filepath[256];
    int member_count;
} tsfi_cw_vsam_pds;

void tsfi_cw_vsam_pds_init(tsfi_cw_vsam_pds *pds, const char *path);
int tsfi_cw_vsam_pds_add_member(tsfi_cw_vsam_pds *pds, const char *name, uint32_t offset, uint32_t length);
int tsfi_cw_vsam_pds_find_member(tsfi_cw_vsam_pds *pds, const char *name, uint32_t *offset_out, uint32_t *length_out);

// EBCDIC Custom Padding character converter
int tsfi_cw_ascii_to_ebcdic_pad(const char *ascii_in, uint8_t *ebcdic_out, int len, uint8_t pad_char);

// JCL step evaluation
int tsfi_cw_jcl_eval_cond(int step_rc, int cond_code, const char *operator);

// Julian to Gregorian Y2K date converter
int tsfi_cw_julian_to_gregorian_y2k(const char *julian_in, uint32_t pivot, char *greg_out, int max_len);

// COBOL SIGN Embedded Zone Nibbles (Zoned Decimal Format)
int tsfi_cw_pack_zoned_sign(const char *ascii_num, uint8_t *ebcdic_out, int max_len, int leading);
int tsfi_cw_unpack_zoned_sign(const uint8_t *ebcdic_in, int len, char *ascii_out, int max_len, int leading);

// EBCDIC CP273 (Germany) translation map
uint8_t tsfi_cw_ascii_to_ebcdic_cp273(uint8_t ascii_char);
uint8_t tsfi_cw_ebcdic_to_ascii_cp273(uint8_t ebcdic_char);

// JCL EXPORT Variable Cards
int tsfi_cw_run_jcl_export(const char **cards, int card_count, char *exp_name, char *exp_val);

// Dynamic Gregorian Day-of-Month bounds
int tsfi_cw_y2k_check_date_bounds(uint32_t yy, uint32_t mm, uint32_t dd, uint32_t pivot);

// VSAM Alternate Index Paths
int tsfi_cw_vsam_path_read(tsfi_cw_vsam_ksds *ksds, tsfi_cw_vsam_aix *aix, const char *alt_key, uint8_t *data_out, int max_len, int *out_len);

// EBCDIC CP500 (International) translation map
uint8_t tsfi_cw_ascii_to_ebcdic_cp500(uint8_t ascii_char);
uint8_t tsfi_cw_ebcdic_to_ascii_cp500(uint8_t ebcdic_char);

// JCL SYSOUT print redirect buffers
typedef struct {
    char buffer[1024];
    int length;
} tsfi_cw_jcl_sysout;

void tsfi_cw_jcl_sysout_init(tsfi_cw_jcl_sysout *sysout);
int tsfi_cw_jcl_sysout_write(tsfi_cw_jcl_sysout *sysout, const char *text);

// Y2K Date Duration calculator
int tsfi_cw_y2k_date_diff(uint32_t yy1, uint32_t mm1, uint32_t dd1, uint32_t yy2, uint32_t mm2, uint32_t dd2, uint32_t pivot, int *days_out);

// VSAM Record Level Sharing (RLS) locks
int tsfi_cw_vsam_lock_record(tsfi_cw_vsam_ksds *ksds, const char *key);
int tsfi_cw_vsam_unlock_record(tsfi_cw_vsam_ksds *ksds, const char *key);

// EBCDIC CP850 translation map
uint8_t tsfi_cw_ascii_to_ebcdic_cp850(uint8_t ascii_char);
uint8_t tsfi_cw_ebcdic_to_ascii_cp850(uint8_t ebcdic_char);

// JCL PROC step overrides
int tsfi_cw_run_jcl_override(const char **cards, int card_count, const char *step_name, char *override_out, int max_len);

// Y2K Day of Week calculator
int tsfi_cw_y2k_day_of_week(uint32_t yy, uint32_t mm, uint32_t dd, uint32_t pivot, int *dow_out);

// VSAM Control Interval Split tracker
int tsfi_cw_vsam_get_ci_splits(tsfi_cw_vsam_ksds *ksds);
int tsfi_cw_vsam_get_compression_ratio(tsfi_cw_vsam_ksds *ksds, float *ratio_out);
int tsfi_cw_vsam_get_key_compression_ratio(tsfi_cw_vsam_ksds *ksds, float *ratio_out);
int tsfi_cw_vsam_get_cache_hits(tsfi_cw_vsam_ksds *ksds);
uint32_t tsfi_cw_vsam_get_key_prefix_savings(tsfi_cw_vsam_ksds *ksds);

int tsfi_cw_vsam_lock_record_ex(tsfi_cw_vsam_ksds *ksds, const char *key, uint32_t max_attempts);

// EBCDIC DBCS validation
int tsfi_cw_ebcdic_validate_dbcs_boundaries(const uint8_t *ebcdic_str, int len);

// EBCDIC CP285 translation
uint8_t tsfi_cw_ascii_to_ebcdic_cp285(uint8_t ascii_char);
uint8_t tsfi_cw_ebcdic_to_ascii_cp285(uint8_t ebcdic_char);

// EBCDIC CP273 extended translation
uint8_t tsfi_cw_ascii_to_ebcdic_cp273_ex(uint8_t ascii_char);
uint8_t tsfi_cw_ebcdic_to_ascii_cp273_ex(uint8_t ebcdic_char);

// EBCDIC CP278 translation
uint8_t tsfi_cw_ascii_to_ebcdic_cp278(uint8_t ascii_char);
uint8_t tsfi_cw_ebcdic_to_ascii_cp278(uint8_t ebcdic_char);

// EBCDIC CP935 Chinese DBCS translation
int tsfi_cw_ebcdic_to_utf8_cp935(const uint8_t *ebcdic_str, int len, char *utf8_out, int max_len);
int tsfi_cw_utf8_to_ebcdic_cp935(const char *utf8_str, uint8_t *ebcdic_out, int max_len);

// EBCDIC CP937 Chinese DBCS translation
int tsfi_cw_ebcdic_to_utf8_cp937(const uint8_t *ebcdic_str, int len, char *utf8_out, int max_len);
int tsfi_cw_utf8_to_ebcdic_cp937(const char *utf8_str, uint8_t *ebcdic_out, int max_len);

// EBCDIC CP939 Chinese DBCS translation
int tsfi_cw_ebcdic_to_utf8_cp939(const uint8_t *ebcdic_str, int len, char *utf8_out, int max_len);
int tsfi_cw_utf8_to_ebcdic_cp939(const char *utf8_str, uint8_t *ebcdic_out, int max_len);

// EBCDIC Zhumadian Cantonese dialect translation
int tsfi_cw_ebcdic_to_utf8_zhumadian_cantonese(const uint8_t *ebcdic_str, int len, char *utf8_out, int max_len);
int tsfi_cw_utf8_to_ebcdic_zhumadian_cantonese(const char *utf8_str, uint8_t *ebcdic_out, int max_len);

// EBCDIC CP935 Extended Single-byte translator and control escape sequence mapper
uint8_t tsfi_cw_ebcdic_cp935_extended_translate(uint8_t ebcdic_char);
uint8_t tsfi_cw_ebcdic_translate_control_escape(uint8_t ebcdic_char);

// JCL GDG resolver, COND chain evaluation, step parameter parser, symbol substitution, name validator, quotes validator, card limit check, and multi symbol substituter
int tsfi_cw_jcl_resolve_gdg(const char *dsn_str, int current_gen, char *resolved_out, int max_len);
int tsfi_cw_jcl_eval_cond_chain(int step_rc, int cond_code_1, const char *op_1, int cond_code_2, const char *op_2);
int tsfi_cw_jcl_parse_parm(const char *card, char *parm_out, int max_len);
int tsfi_cw_jcl_substitute_symbol(const char *card, const char *sym_name, const char *sym_val, char *resolved_out, int max_len);
int tsfi_cw_jcl_substitute_symbols_multi(const char *card, const char **sym_names, const char **sym_vals, int sym_count, char *resolved_out, int max_len);
int tsfi_cw_jcl_validate_symbol_name(const char *sym_name);
int tsfi_cw_jcl_check_parm_quotes(const char *card);
int tsfi_cw_jcl_sysin_limit_check(int card_count);

// COBOL occurs, justified right, picture clause, record offset, and overlap validators
int tsfi_cw_cobol_validate_occurs_range(int current_occurs, int max_occurs);
int tsfi_cw_cobol_validate_justified_right_dynamic(tsfi_cw_cobol_field *f, int dynamic_occurs, const char *val);
int tsfi_cw_cobol_validate_picture_numeric_bounds(const char *pic_str, double val);
int tsfi_cw_cobol_validate_record_offset(int calculated_offset, int max_buffer_limit);
int tsfi_cw_cobol_validate_offset_overlap(int field_offset, int preceding_field_end);

// Y2K leap year checker, Month days resolver, Julian day validator, leap adjustments counter, and pivot year range validator
int tsfi_cw_y2k_is_leap_year(uint32_t year);
int tsfi_cw_y2k_get_month_days(uint32_t year, uint32_t month, int *days_out);
int tsfi_cw_gregorian_to_julian_y2k(const char *greg_in, uint32_t pivot, char *julian_out, int max_len);
int tsfi_cw_y2k_validate_format(const char *date_str);
int tsfi_cw_y2k_validate_julian_day(uint32_t year, uint32_t day_of_year);
int tsfi_cw_y2k_count_leap_adjustments(uint32_t year1, uint32_t year2);
int tsfi_cw_y2k_validate_pivot_range(uint32_t pivot);

// VSAM key length validator, checksum tools, cached verifier, index validator, checksum audit stats query, and reset tool
int tsfi_cw_vsam_validate_compressed_key_len(const char *raw_key, const char *comp_key);
uint32_t tsfi_cw_vsam_calculate_checksum(const uint8_t *data, int len);
int tsfi_cw_vsam_verify_record_checksum(const uint8_t *data, int len, uint32_t expected_checksum);
int tsfi_cw_vsam_verify_checksum_cached(tsfi_cw_vsam_ksds *ksds, int mid, const uint8_t *data_out, int read_len);
int tsfi_cw_vsam_verify_index_checksums(tsfi_cw_vsam_ksds *ksds);
int tsfi_cw_vsam_get_checksum_audit_stats(uint32_t *audits_out, uint32_t *mismatches_out);
void tsfi_cw_vsam_reset_checksum_audit_stats(void);
int tsfi_cw_vsam_format_checksum_stats(char *buf_out, int max_len);
int tsfi_cw_vsam_format_and_reset_checksum_stats(char *buf_out, int max_len);
int tsfi_cw_vsam_query_and_reset_checksum_audit_stats(uint32_t *audits_out, uint32_t *mismatches_out);
int tsfi_cw_vsam_get_checksum_audit_stats_ex(const tsfi_cw_vsam_ksds *ksds, char *filepath_out, int max_len, uint32_t *audits_out, uint32_t *mismatches_out);
int tsfi_cw_vsam_format_and_reset_checksum_stats_ex(const tsfi_cw_vsam_ksds *ksds, char *buf_out, int max_len);
int tsfi_cw_vsam_format_checksum_status_ex(const tsfi_cw_vsam_ksds *ksds, char *buf_out, int max_len, const char *status_msg);
int tsfi_cw_vsam_format_and_reset_checksum_status_ex(const tsfi_cw_vsam_ksds *ksds, char *buf_out, int max_len, const char *status_msg);
int tsfi_cw_vsam_format_and_reset_checksum_status_ex2(const tsfi_cw_vsam_ksds *ksds, char *buf_out, int max_len, const char *status_msg, int *rc_out);
int tsfi_cw_vsam_format_and_reset_checksum_status_ex3(const tsfi_cw_vsam_ksds *ksds, char *buf_out, int max_len, const char *status_msg, int *rc_out, int *len_out);

typedef struct {
    char pad_char;
    int alignment;
    int max_size;
} tsfi_cw_cobol_padding_config;

// COBOL custom padding validator, alignment checker, padding limits validator, alignment padding mapper, field alignment verifier, alignment limits validator, config tool, config query, reset tool, alignment ex checker, boundary alignment checker, alignment ex 3 boundary check, alignment ex 4 fallback boundary checker, alignment ex 5 margin offset check, and alignment ex 6 failure diagnostics
int tsfi_cw_cobol_validate_custom_padding(char pad_char);
int tsfi_cw_cobol_validate_custom_padding_ex(char pad_char, int pad_len, int max_len);
int tsfi_cw_cobol_map_custom_padding_byte(char input_char, char *mapped_out);
int tsfi_cw_cobol_validate_padding_alignment(int field_offset, int alignment_modulus);
int tsfi_cw_cobol_validate_padding_limits(int pad_len, int max_allowed);
int tsfi_cw_cobol_get_alignment_padding(int offset, int alignment_modulus, int *padding_out);
int tsfi_cw_cobol_verify_field_alignment(int offset, int size, int alignment);
int tsfi_cw_cobol_validate_padding_limits_ex(int pad_len, int max_allowed, int alignment);
int tsfi_cw_cobol_configure_padding_alignment(tsfi_cw_cobol_padding_config *cfg, char pad_char, int alignment, int max_size);
int tsfi_cw_cobol_query_padding_alignment(const tsfi_cw_cobol_padding_config *cfg, char *pad_char_out, int *alignment_out, int *max_size_out);
int tsfi_cw_cobol_reset_padding_alignment(tsfi_cw_cobol_padding_config *cfg);
int tsfi_cw_cobol_verify_field_alignment_ex(int offset, int size, int alignment, int max_size);
int tsfi_cw_cobol_verify_field_alignment_ex2(int offset, int size, int alignment, int max_size, char pad_char);
int tsfi_cw_cobol_verify_field_alignment_ex3(int offset, int size, int alignment, int max_size, char pad_char, int margin);
int tsfi_cw_cobol_verify_field_alignment_ex4(int offset, int size, int alignment, int max_size, char pad_char, int margin, int fallback_pad);
int tsfi_cw_cobol_verify_field_alignment_ex5(int offset, int size, int alignment, int max_size, char pad_char, int margin, int fallback_pad, int offset_limit);
int tsfi_cw_cobol_verify_field_alignment_ex6(int offset, int size, int alignment, int max_size, char pad_char, int margin, int fallback_pad, int offset_limit, int *failure_offset_out);
int tsfi_cw_cobol_validate_dod1967_compliance(const char *source_code, int *violation_count_out);

typedef struct {
    uint32_t total_parity_checks;
    uint8_t current_so;
    uint8_t current_si;
} tsfi_cw_ebcdic_parity_metrics;

// EBCDIC nesting validator, escape override, custom markers override, parity checks counter query, parity checks reset tool, query-and-reset tool, diagnostics formatter, diagnostics format-reset tool, metrics ex query, query-reset ex helper, metrics ex formatter, metrics ex 2 formatter format-reset, metrics ex 3 formatter format-reset ex, metrics ex 4 formatter format-reset ex 2, and metrics ex 5 format-reset ex 3
int tsfi_cw_ebcdic_check_dbcs_nesting(const uint8_t *ebcdic_str, int len);
uint8_t tsfi_cw_ebcdic_translate_control_escape_override(uint8_t ebcdic_char, uint8_t custom_lf, uint8_t custom_cr);
void tsfi_cw_ebcdic_override_dbcs_markers(uint8_t new_so, uint8_t new_si);
int tsfi_cw_ebcdic_get_parity_checks_count(uint32_t *count_out);
void tsfi_cw_ebcdic_reset_parity_checks_count(void);
int tsfi_cw_ebcdic_query_and_reset_parity_checks(uint32_t *count_out);
int tsfi_cw_ebcdic_format_parity_diagnostics(char *buf_out, int max_len);
int tsfi_cw_ebcdic_format_and_reset_parity_diagnostics(char *buf_out, int max_len);
int tsfi_cw_ebcdic_get_parity_metrics_ex(tsfi_cw_ebcdic_parity_metrics *metrics_out);
int tsfi_cw_ebcdic_query_and_reset_parity_metrics_ex(tsfi_cw_ebcdic_parity_metrics *metrics_out);
int tsfi_cw_ebcdic_format_and_reset_parity_metrics_ex(const tsfi_cw_ebcdic_parity_metrics *metrics, char *buf_out, int max_len);
int tsfi_cw_ebcdic_format_and_reset_parity_metrics_ex2(tsfi_cw_ebcdic_parity_metrics *metrics, char *buf_out, int max_len);
int tsfi_cw_ebcdic_format_and_reset_parity_metrics_ex3(tsfi_cw_ebcdic_parity_metrics *metrics, char *buf_out, int max_len, const char *prefix);
int tsfi_cw_ebcdic_format_and_reset_parity_metrics_ex4(tsfi_cw_ebcdic_parity_metrics *metrics, char *buf_out, int max_len, const char *prefix, int *len_out);
int tsfi_cw_ebcdic_format_and_reset_parity_metrics_ex5(tsfi_cw_ebcdic_parity_metrics *metrics, char *buf_out, int max_len, const char *prefix, int *len_out, int *status_out);

// JCL circular dependency checker, recursion depth validator, PROC recursion depth checker, recursion limit setter, limit query, substitution with custom depth limit, substitution limit configuration setter/getter, limit reset tool, limit query-reset tool, limit boundary setter, limit boundary query, limits fallback configuration query, symbol substitution stats query, symbol substitution stats query-reset, symbol substitution stats query-reset ex, and symbol substitution stats query-reset ex 2 tool
int tsfi_cw_jcl_detect_circular_symbols(const char **sym_names, const char **sym_vals, int sym_count);
int tsfi_cw_jcl_validate_substitution_depth(int current_depth, int max_depth);
int tsfi_cw_jcl_validate_proc_recursion_depth(int depth, int max_depth);
void tsfi_cw_jcl_set_custom_proc_recursion_limit(int limit);
int tsfi_cw_jcl_query_recursion_limit(int *limit_out);
int tsfi_cw_jcl_substitute_symbols_multi_limit(const char *card, const char **sym_names, const char **sym_vals, int sym_count, char *resolved_out, int max_len, int max_depth);
void tsfi_cw_jcl_set_substitution_depth_limit(int limit);
int tsfi_cw_jcl_get_substitution_depth_limit(int *limit_out);
void tsfi_cw_jcl_reset_substitution_depth_limit(void);
int tsfi_cw_jcl_query_and_reset_substitution_limit(int *limit_out);
int tsfi_cw_jcl_set_substitution_depth_limit_boundary(int limit, int max_allowed_boundary);
int tsfi_cw_jcl_get_substitution_depth_limit_boundary(int *limit_out, int *boundary_out);
int tsfi_cw_jcl_get_substitution_depth_limit_boundary_ex(int *limit_out, int *boundary_out, int *fallback_out);
int tsfi_cw_jcl_get_substitution_stats(uint32_t *substitutions_out);
int tsfi_cw_jcl_query_and_reset_substitution_stats(uint32_t *substitutions_out);
int tsfi_cw_jcl_query_and_reset_substitution_stats_ex(uint32_t *substitutions_out, int *limit_out);
int tsfi_cw_jcl_query_and_reset_substitution_stats_ex2(uint32_t *substitutions_out, int *limit_out, int *boundary_out);
int tsfi_cw_jcl_simulate_s360_job(const char *jcl_script, char *log_out, int max_log_len, int *job_status_out);

// Y2K dates chronological order check, reset tool, query interface, query-and-reset tool, violations query formatter, query format-reset tool, violations list printer, print-reset list tool, list printer ex, print-reset list ex, violations format-reset ex settings, print-reset list ex 2, chronological violations list ex 2 format, chronological violations list ex 2 format-reset, and chronological violations list ex 3 format-reset tool
int tsfi_cw_y2k_validate_chronological_order(uint32_t yy1, uint32_t mm1, uint32_t dd1, uint32_t yy2, uint32_t mm2, uint32_t dd2, uint32_t pivot);
uint32_t tsfi_cw_y2k_get_chronological_violations(void);
void tsfi_cw_y2k_reset_chronological_violations(void);
int tsfi_cw_y2k_query_chronological_violations(uint32_t *violations_out);
int tsfi_cw_y2k_query_and_reset_violations(uint32_t *violations_out);
int tsfi_cw_y2k_format_chronological_violations(char *buf_out, int max_len);
int tsfi_cw_y2k_format_and_reset_violations(char *buf_out, int max_len);
int tsfi_cw_y2k_print_diagnostic_violations_list(char *buf_out, int max_len);
int tsfi_cw_y2k_print_and_reset_violations_list(char *buf_out, int max_len);
int tsfi_cw_y2k_print_diagnostic_violations_list_ex(char *buf_out, int max_len, uint32_t pivot);
int tsfi_cw_y2k_print_and_reset_violations_list_ex(char *buf_out, int max_len, uint32_t pivot);
int tsfi_cw_y2k_format_and_reset_violations_ex(char *buf_out, int max_len, uint32_t pivot);
int tsfi_cw_y2k_print_and_reset_violations_list_ex2(char *buf_out, int max_len, uint32_t pivot, const char *prefix);
int tsfi_cw_y2k_format_diagnostic_violations_list_ex2(char *buf_out, int max_len, uint32_t pivot, const char *prefix, int *len_out);
int tsfi_cw_y2k_format_and_reset_violations_ex2(char *buf_out, int max_len, uint32_t pivot, const char *prefix, int *len_out);
int tsfi_cw_y2k_format_and_reset_violations_ex3(char *buf_out, int max_len, uint32_t pivot, const char *prefix, int *len_out, uint32_t *violations_out);

typedef struct {
    uint32_t leap_checks_performed;
    uint32_t leap_year_hits;
    uint32_t span_leap_adjustments_tracked;
} tsfi_cw_y2k_diagnostics;

void tsfi_cw_y2k_get_diagnostics(tsfi_cw_y2k_diagnostics *diag);

// 4. Job Control Language (JCL) Execution Simulator
int tsfi_cw_run_jcl(const char **cards, int card_count);
int tsfi_cw_run_jcl_ex(const char **cards, int card_count, int initial_rc);
int tsfi_cw_run_jcl_sysin(const char **cards, int card_count, char *sysin_out, int max_sysin_len);
int tsfi_cw_run_jcl_sysin_ex(const char **cards, int card_count, char *sysin_out, int max_sysin_len, int *truncated_count);
int tsfi_cw_run_jcl_proc(const char **cards, int card_count, const char **proc_cards, int proc_card_count, int initial_rc);
int tsfi_cw_run_jcl_restart(const char **cards, int card_count, const char *restart_step);
int tsfi_cw_run_jcl_proc_nested(const char **cards, int card_count, const char **proc_cards, int proc_card_count, int initial_rc, int depth);

// B5500, Honeywell, and Univac historical mainframe conversions
int tsfi_b5500_prt_resolve_descriptor(uint64_t descriptor, uint32_t offset, const uint64_t *memory_pool, uint32_t mem_size, uint64_t *val_out);
int tsfi_cw_honeywell_bcd_to_ebcdic(const uint8_t *bcd_in, int bcd_len, uint8_t *ebcdic_out);
double tsfi_cw_univac_to_double(uint64_t univac_val);
uint64_t tsfi_cw_double_to_univac(double val);

// Punched Card Payroll Processing structures and functions
typedef struct {
    char employee_id[8];
    double hours_worked;
    double hourly_rate;
    int exemptions;
    double gross_pay;
    double fica_withholding;
    double fed_withholding;
    double net_pay;
} tsfi_cw_payroll_record;

int tsfi_cw_payroll_process_card(const char *card_line, tsfi_cw_payroll_record *rec_out);

// IBM RAMAC Inventory Control structures and functions
typedef struct {
    char part_id[8];
    char description[24];
    int quantity_on_hand;
    double unit_cost;
    int reorder_point;
} tsfi_cw_ramac_stock;

int tsfi_cw_ramac_process_transaction(tsfi_cw_ramac_stock *stock, const char *tx_card_line, int *reorder_triggered);

// Corporate Salary Survey Analyzer structures and functions
typedef struct {
    char dept_id[8];
    char role[4]; // "PG", "OP", "SA"
    double salary;
    char gender; // 'M', 'F'
} tsfi_cw_salary_record;

typedef struct {
    double avg_programmer_salary;
    double avg_operator_salary;
    double avg_analyst_salary;
    int female_count;
    int male_count;
} tsfi_cw_survey_stats;

int tsfi_cw_salary_process_cards(const char **card_lines, int card_count, tsfi_cw_survey_stats *stats_out);

// Simplex Production Optimizer structures and functions
typedef struct {
    double c1, c2;       // Profit coefficients
    double a11, a12, b1; // Constraint 1: a11*x1 + a12*x2 <= b1
    double a21, a22, b2; // Constraint 2: a21*x1 + a22*x2 <= b2
} tsfi_cw_simplex_problem;

int tsfi_cw_simplex_optimize(const tsfi_cw_simplex_problem *prob, double *x1_opt, double *x2_opt, double *profit_opt);

// Critical Path Method (CPM) structures and functions
typedef struct {
    int id;
    int duration;
    int predecessors[4];
    int pred_count;
    int early_start;
    int early_finish;
    int late_start;
    int late_finish;
    int slack;
    int is_critical;
} tsfi_cw_cpm_task;

int tsfi_cw_cpm_schedule(tsfi_cw_cpm_task *tasks, int task_count);

// Accounts Receivable Ledger structures and functions
typedef struct {
    char customer_id[8];
    double balance_current;
    double balance_30_days;
    double balance_60_days;
    double balance_90_days;
    double total_balance;
} tsfi_cw_ar_statement;

int tsfi_cw_ar_process_ledger(const char **tx_cards, int card_count, const char *ref_date_yymmdd, tsfi_cw_ar_statement *statements_out, int *stmt_count_out);

// Punched Card General Ledger compiler
typedef struct {
    double total_assets;
    double total_liabilities;
    double total_equity;
    double net_income;
} tsfi_cw_gl_balance_sheet;

int tsfi_cw_gl_compile_balance_sheet(const char **tx_cards, int card_count, tsfi_cw_gl_balance_sheet *sheet_out);

// Mainframe Economic Order Quantity (EOQ) calculator
typedef struct {
    double annual_demand;
    double order_cost;
    double holding_cost;
} tsfi_cw_eoq_problem;

int tsfi_cw_eoq_calculate(const tsfi_cw_eoq_problem *prob, double *eoq_out, double *total_cost_out);

// PERT Estimator
typedef struct {
    int id;
    double opt_duration;   // a
    double most_likely;    // m
    double pess_duration;  // b
    int predecessors[4];
    int pred_count;
    double expected_duration; // calculated: (a + 4m + b)/6
    double variance;          // calculated: ((b - a)/6)^2
} tsfi_cw_pert_task;

int tsfi_cw_pert_calculate(tsfi_cw_pert_task *tasks, int task_count, double *expected_project_length_out, double *project_variance_out);

// Punched Card Asset Depreciation
typedef struct {
    char asset_id[8];
    double cost;
    double salvage_value;
    int useful_life;
} tsfi_cw_depreciation_asset;

int tsfi_cw_depreciation_calculate(const tsfi_cw_depreciation_asset *asset, char method, int target_year, double *expense_out, double *book_value_out);

// Simplex Transportation Optimizer (2x2)
typedef struct {
    double supply[2];
    double demand[2];
    double cost[2][2];
} tsfi_cw_transportation_problem;

int tsfi_cw_transport_optimize(const tsfi_cw_transportation_problem *prob, double shipment[2][2], double *total_cost_out);

// Three-Way PO Matcher
typedef struct {
    char po_id[8];
    char part_id[8];
    int quantity;
    double unit_price;
} tsfi_cw_po_record;

typedef struct {
    char po_id[8];
    char part_id[8];
    int quantity_received;
} tsfi_cw_receiving_record;

typedef struct {
    char po_id[8];
    char part_id[8];
    int quantity_invoiced;
    double invoice_amount;
} tsfi_cw_invoice_record;

typedef struct {
    int matches;
    int qty_mismatch;  // flag 1 if received != invoiced or PO quantity
    int price_mismatch; // flag 1 if invoice amount != invoiced_qty * PO unit price
    int status_approved; // flag 1 if matched and correct, else 0
} tsfi_cw_match_result;

int tsfi_cw_three_way_match(const tsfi_cw_po_record *po, const tsfi_cw_receiving_record *rr, const tsfi_cw_invoice_record *invoice, tsfi_cw_match_result *result_out);

// PERT/CPM Resource Leveling
typedef struct {
    int task_id;
    int duration;
    int early_start;
    int late_start;
    int resource_rate; // Resources used per day
    int scheduled_start; // Output: optimized start day
} tsfi_cw_leveling_task;

int tsfi_cw_resource_level(tsfi_cw_leveling_task *tasks, int task_count, int project_horizon, int *peak_resource_out);

// Safety Stock & Reorder Point (ROP) Calculator
typedef struct {
    double service_factor_z;      // e.g. 1.65
    double avg_daily_demand;      // D
    double demand_std_dev;         // sigma_D
    double avg_lead_time_days;    // LT
    double lead_time_std_dev;      // sigma_LT
} tsfi_cw_rop_problem;

int tsfi_cw_rop_calculate(const tsfi_cw_rop_problem *prob, double *safety_stock_out, double *reorder_point_out);

// ICP Directory Product Registry
typedef struct {
    char product_id[8];
    char name[32];
    char vendor[32];
    char hardware_platform[16];
    double unit_price;
    int install_count;
} tsfi_cw_icp_product;

// ICP Million Dollar Award Status
typedef struct {
    double total_revenue;
    int qualifies_for_million_dollar_award;
} tsfi_cw_icp_award_status;

int tsfi_cw_icp_register_product(tsfi_cw_icp_product *catalog, int *catalog_size, const tsfi_cw_icp_product *new_prod);
int tsfi_cw_icp_check_award(const tsfi_cw_icp_product *prod, tsfi_cw_icp_award_status *status_out);

// ICP Licensing Commission Ledger
typedef struct {
    char agent_id[8];
    double total_sales;
    double base_salary;
    double commission_earned;
    double total_payout;
} tsfi_cw_icp_agent;

int tsfi_cw_icp_process_agent_sale(tsfi_cw_icp_agent *agent, const char *sale_card_line);

// ICP Maintenance Contract Auditor
typedef struct {
    char contract_id[8];
    char product_id[8];
    double monthly_fee;
    int contract_months;
    int months_elapsed;
} tsfi_cw_icp_contract;

int tsfi_cw_icp_audit_contract(const tsfi_cw_icp_contract *contract, double *total_value_out, double *remaining_value_out);

// ICP Compatibility Matrix Auditor and Royalty Distributor
int tsfi_cw_icp_audit_compatibility(const tsfi_cw_icp_product *prod, const char *target_hardware, int *is_compatible_out);
int tsfi_cw_icp_distribute_royalties(const tsfi_cw_icp_product *prod, double rate, double *royalty_out);

// ICP Directory Search and Version Upgrade Auditor
int tsfi_cw_icp_search_directory(const tsfi_cw_icp_product *catalog, int catalog_size, const char *hardware_filter, tsfi_cw_icp_product *results_out, int *results_count_out);
int tsfi_cw_icp_migration_audit(const tsfi_cw_icp_product *old_prod, const tsfi_cw_icp_product *new_prod, int *upgrade_allowed_out);

// ICP Compliance Auditor and Sales Quota Tracker
int tsfi_cw_icp_audit_license_compliance(int purchased, int active, int *warning_out);
int tsfi_cw_icp_track_quota(double target, double actual, double *bonus_payout_out);

// ICP Subscription Structure
typedef struct {
    char sub_id[8];
    char product_id[8];
    double monthly_rate;
    int billing_cycles_total;
    int cycles_paid;
} tsfi_cw_icp_subscription;

// ICP Bundle Pricer and Subscription Calculator
int tsfi_cw_icp_calculate_bundle_price(const tsfi_cw_icp_product *products, int count, double discount_rate, double *total_price_out);
int tsfi_cw_icp_subscription_status(const tsfi_cw_icp_subscription *sub, double *total_paid_out, double *remaining_obligations_out);

// ICP Vendor Analyzer and Transfer Tax Calculator
int tsfi_cw_icp_analyze_vendor(const tsfi_cw_icp_product *catalog, int catalog_size, const char *vendor_name, double *total_revenue_out, int *award_count_out);
int tsfi_cw_icp_calculate_transfer_tax(double license_amount, double tax_rate, double *tax_amount_out);

// ICP Depreciation Estimator and Renewal Penalty Auditor
int tsfi_cw_icp_calculate_software_depreciation(double cost, double salvage_value, int lifespan_years, int target_year, double *depreciated_value_out);
int tsfi_cw_icp_calculate_renewal_penalty(double renewal_fee, int days_late, double penalty_rate_per_day, double *total_due_out);

// ICP License Transfer Auditor and Royalty Ledger Consolidator
int tsfi_cw_icp_audit_transfer(tsfi_cw_icp_product *prod, const char *new_vendor, double transfer_fee_rate, double *transfer_fee_out);
int tsfi_cw_icp_consolidate_royalties(const tsfi_cw_icp_product *catalog, int catalog_size, const char *vendor, double royalty_rate, double *consolidated_payment_out);

// ICP Expiration Monitor and Software Payback Calculator
int tsfi_cw_icp_check_expiration(int current_year, int current_month, int current_day, int exp_year, int exp_month, int exp_day, int *expired_out, int *days_remaining_out);
int tsfi_cw_icp_calculate_payback(double purchase_cost, double annual_savings, double *payback_years_out);

// ICP Release Record
typedef struct {
    char product_id[8];
    char version_string[16];
    int release_year;
    int release_month;
    int release_day;
} tsfi_cw_icp_release_record;

// ICP Vendor Record
typedef struct {
    char vendor_name[32];
    char country_code[4];
    int is_active;
    int is_compliant;
} tsfi_cw_icp_vendor_record;

// ICP Release and Vendor Compliance Functions
int tsfi_cw_icp_register_release(tsfi_cw_icp_release_record *releases, int *release_count, const tsfi_cw_icp_release_record *new_release);
int tsfi_cw_icp_audit_vendor(const tsfi_cw_icp_vendor_record *vendor, int *is_compliant_out);

// ICP Product Category Mapping
typedef struct {
    char product_id[8];
    char category_name[16];
} tsfi_cw_icp_category_map;

// ICP Category Grouping and Subscription Grace Functions
int tsfi_cw_icp_assign_category(tsfi_cw_icp_category_map *maps, int *map_count, const char *prod_id, const char *category);
int tsfi_cw_icp_query_category(const tsfi_cw_icp_category_map *maps, int map_count, const char *category, char ids_out[][8], int *ids_count_out);
int tsfi_cw_icp_audit_grace_period(int days_late, int allowed_grace_days, int *suspended_out);

// ICP Upgrade Pricing and Support SLA Functions
int tsfi_cw_icp_calculate_upgrade_price(double current_version_price, double new_version_price, double loyalty_discount_rate, double *upgrade_price_out);
int tsfi_cw_icp_audit_support_sla(int response_time_mins, int target_sla_mins, double contract_monthly_fee, double *rebate_out);

// ESJ Mainframe Paging & Memory Telemetry
typedef struct {
    int page_ins;
    int page_outs;
    int uic; // Unreferenced Interval Count (MVS page aging metric)
    int total_frames;
    int free_frames;
} tsfi_cw_esj_paging_metrics;

int tsfi_cw_esj_analyze_paging(const tsfi_cw_esj_paging_metrics *metrics, double *thrashing_index_out, int *alert_flag_out);

// Marist ECR z/VM Live Guest Migration
typedef struct {
    double vm_memory_mb;
    double dirty_rate_mb_per_sec;
    double network_speed_mbps;
    double max_migration_time_sec;
} tsfi_cw_marist_zvm_migration;

// Marist ECR SDN Flow Rule
typedef struct {
    int rule_id;
    char src_ip[16];
    char dest_ip[16];
    int action; // 1 = FORWARD, 2 = DROP
    int packet_count;
} tsfi_cw_marist_sdn_rule;

int tsfi_cw_marist_optimize_migration(const tsfi_cw_marist_zvm_migration *config, double *est_time_out, int *can_migrate_out);
int tsfi_cw_marist_audit_sdn(const tsfi_cw_marist_sdn_rule *rules, int rule_count, const char *src_ip, const char *dest_ip, int *action_out);

// Marist LinuxONE Academic Tenant
typedef struct {
    char tenant_id[16];
    double allocated_cores;
    double used_cores;
    double allocated_mem_gb;
    double used_mem_gb;
} tsfi_cw_marist_tenant;

// Marist z/VM Guest Profile
typedef struct {
    char guest_id[16];
    char privilege_class; // 'A'..'Z'
    int allow_inter_vm_comm;
    int has_read_any_spool;
} tsfi_cw_marist_guest_profile;

int tsfi_cw_marist_audit_tenant(const tsfi_cw_marist_tenant *tenant, int *is_nominal_out);
int tsfi_cw_marist_audit_isolation(const tsfi_cw_marist_guest_profile *profile, int *is_secure_out);

// Marist z/OS Parallel Sysplex Coupling Facility Telemetry
typedef struct {
    int total_requests;
    int failed_requests;
    int avg_response_time_us;
    double buffer_util_percent;
} tsfi_cw_marist_sysplex_cf;

// Marist z/VM CPU Share Scheduler
typedef struct {
    int vm_count;
    int shares[16];
    int total_shares;
} tsfi_cw_marist_zvm_scheduler;

int tsfi_cw_marist_audit_sysplex(const tsfi_cw_marist_sysplex_cf *cf, int *alert_out);
int tsfi_cw_marist_calc_cpu_shares(const tsfi_cw_marist_zvm_scheduler *sched, double share_allocs_out[16]);

// Marist z/OS RMF CPU Workload Telemetry
typedef struct {
    int processor_id;
    int dispatch_delay_ms;
    double busy_percent;
    double parked_percent;
} tsfi_cw_marist_rmf_cpu;

// Marist z/VM VSWITCH Telemetry
typedef struct {
    char switch_name[16];
    double drop_rate_percent;
    int active_ports;
    int backup_ports_configured;
} tsfi_cw_marist_vswitch;

int tsfi_cw_marist_audit_rmf_cpu(const tsfi_cw_marist_rmf_cpu *cpus, int cpu_count, double *avg_busy_out, int *overloaded_out);
int tsfi_cw_marist_optimize_vswitch(const tsfi_cw_marist_vswitch *vsw, int *needs_failover_out);

// Marist z/OS WLM Service Class Telemetry
typedef struct {
    char class_name[8];
    double goal_velocity;
    double actual_velocity;
    int active_states_count;
    int delay_states_count;
} tsfi_cw_marist_wlm_service;

// Marist z/VM Cryptographic Coprocessor config
typedef struct {
    int coproc_id;
    int guest_shares[16];
    int guest_count;
    int total_crypto_units;
} tsfi_cw_marist_crypto_coproc;

int tsfi_cw_marist_audit_wlm(const tsfi_cw_marist_wlm_service *srv, int *needs_adjustment_out);
int tsfi_cw_marist_alloc_crypto(const tsfi_cw_marist_crypto_coproc *cfg, double guest_allocs_out[16]);

// Marist z/VM Minidisk Cache Telemetry
typedef struct {
    int read_requests;
    int read_hits;
    int write_requests;
    int cache_size_kb;
} tsfi_cw_marist_minidisk_cache;

// Marist z/OS ISGLOCK Clustered Lock Telemetry
typedef struct {
    int lock_requests;
    int contended_requests;
    int timeouts;
    int avg_grant_delay_ms;
} tsfi_cw_marist_isglock;

int tsfi_cw_marist_audit_minidisk_cache(const tsfi_cw_marist_minidisk_cache *cache, double *hit_ratio_out, int *needs_resize_out);
int tsfi_cw_marist_audit_isglock(const tsfi_cw_marist_isglock *lock, int *alert_out);

// NIU DeKalb HLASM & JCL functions
int tsfi_cw_niu_hlasm_exec(uint32_t regs[16], const char *op, int r1, int r2, int32_t immediate, int *overflow_out);
int tsfi_cw_niu_validate_jcl(const char *jcl_line, int *is_valid_out);

// NIU DeKalb HLASM Macro Preprocessor and COBOL Copybook structures
typedef struct {
    char name[16];
    int level;
    char type[8];
    int size_bytes;
} tsfi_cw_niu_copybook_field;

int tsfi_cw_niu_expand_macro(const char *macro_def, const char *param, char *output_out, size_t max_len);
int tsfi_cw_niu_audit_copybook(const tsfi_cw_niu_copybook_field *fields, int field_count, int *total_bytes_out);

// NIU DeKalb COBOL Working Storage variable
typedef struct {
    char name[32];
    int has_value_clause;
    int is_numeric;
    char initial_value_str[16];
} tsfi_cw_niu_cobol_var;

int tsfi_cw_niu_audit_jcl_dd(const char *dd_statement, int *is_valid_path_out);
int tsfi_cw_niu_audit_working_storage(const tsfi_cw_niu_cobol_var *vars, int var_count, int *uninitialized_count_out);

// ISU Normal LEAP defense application
typedef struct {
    char app_id[16];
    double database_size_gb;
    int transaction_rate_per_sec;
    int nato_irs_compliance_rating; // 0..5
} tsfi_cw_isu_leap_app;

int tsfi_cw_isu_leap_defense_audit(const tsfi_cw_isu_leap_app *app, int defcon_level, double *criticality_risk_out);
int tsfi_cw_isu_ulid_ssa_match(const char *ulid, const char *ssn_last4, int *is_match_out);

// ISU Normal legacy email log
typedef struct {
    char sender_ulid[16];
    char recipient_addr[64];
    int message_bytes;
    int is_mf_crossover;
} tsfi_cw_isu_email_log;

// ISU Normal State Farm SLA monitor
typedef struct {
    char batch_job_name[16];
    int elapsed_seconds;
    int max_allowed_seconds;
    int micro_focus_compat_flags;
} tsfi_cw_isu_state_farm_sla;

int tsfi_cw_isu_audit_email(const tsfi_cw_isu_email_log *log, int *is_valid_out);
int tsfi_cw_isu_audit_sf_sla(const tsfi_cw_isu_state_farm_sla *sla, int *is_compliant_out);

// ISU Normal Course Enrollment
typedef struct {
    char course_id[8];
    int current_enrolled;
    int max_capacity;
    int prereqs_met_count;
    int total_applicants;
} tsfi_cw_isu_course_enrollment;

// ISU Normal Gateway Status
typedef struct {
    char gateway_ip[16];
    int packets_sent;
    int packets_dropped;
    int rtt_ms;
} tsfi_cw_isu_gateway_status;

int tsfi_cw_isu_audit_enrollment(const tsfi_cw_isu_course_enrollment *enroll, int *can_register_out);
int tsfi_cw_isu_audit_gateway(const tsfi_cw_isu_gateway_status *gw, int *alert_out);

// UNT Denton Master the Mainframe Contestant
typedef struct {
    char name[32];
    int part1_completed;
    int part2_completed;
    int part3_completed_tasks; // 0..5
    int vsam_modified;
    int jcl_errors;
} tsfi_cw_unt_mtm_contestant;

// UNT Denton Mainframe-to-Cloud Cryptographic Pipeline
typedef struct {
    int pipeline_id;
    char encryption_protocol[16];
    int unencrypted_fields_count;
    int checksum_verified;
} tsfi_cw_unt_crypto_pipeline;

int tsfi_cw_unt_score_contestant(const tsfi_cw_unt_mtm_contestant *student, int *score_out);
int tsfi_cw_unt_audit_pipeline(const tsfi_cw_unt_crypto_pipeline *pipe, int *is_compliant_out);

// UNT Denton VSAM KSDS
typedef struct {
    char dataset_name[32];
    int control_interval_splits;
    int control_area_splits;
    int buffer_space_allocated_kb;
} tsfi_cw_unt_vsam_ksds;

// UNT Denton RACF log
typedef struct {
    char user_id[8];
    char resource_name[32];
    char access_level_requested[8];
    int is_authorized;
} tsfi_cw_unt_racf_log;

int tsfi_cw_unt_optimize_vsam(const tsfi_cw_unt_vsam_ksds *ksds, int *recommendation_flags_out);
int tsfi_cw_unt_audit_racf(const tsfi_cw_unt_racf_log *logs, int log_count, int *violations_out);

// RAF VITAL request
typedef struct {
    char part_number[16];
    char priority_level[8];
    int stock_available;
    int qty_requested;
} tsfi_cw_raf_vital_request;

// RAF RIDELS audit
typedef struct {
    char part_number[16];
    int physical_count;
    int system_count;
    double unit_cost;
} tsfi_cw_raf_ridels_audit;

int tsfi_cw_raf_allocate_vital(const tsfi_cw_raf_vital_request *req, int *approved_out, int *priority_score_out);
int tsfi_cw_raf_audit_ridels(const tsfi_cw_raf_ridels_audit *items, int item_count, double *total_discrepancy_value_out);

// UNT Denton PDS Status
typedef struct {
    char dataset_name[32];
    int total_directory_blocks;
    int used_directory_blocks;
    int members_count;
} tsfi_cw_unt_pds_status;

// UNT Denton CICS Transaction
typedef struct {
    char transaction_id[8];
    int response_time_ms;
    int db_calls_count;
    int cpu_time_ms;
} tsfi_cw_unt_cics_tran;

int tsfi_cw_unt_audit_pds(const tsfi_cw_unt_pds_status *pds, int *needs_compress_out);
int tsfi_cw_unt_profile_cics(const tsfi_cw_unt_cics_tran *trans, int tran_count, double *avg_response_time_out, int *slow_count_out);

// CICS Terminal Session Info
typedef struct {
    char user_id[8];
    char terminal_id[8];
    int is_active;
    int latency_ms;
} tsfi_cw_unt_cics_session;

// CICS Storage Queue Info
typedef struct {
    char queue_name[8];
    int item_count;
    char queue_type[8]; // TSQ or TDQ
    int total_bytes;
} tsfi_cw_unt_cics_queue;

int tsfi_cw_unt_cics_audit_session(const tsfi_cw_unt_cics_session *session, int *needs_reset_out);
int tsfi_cw_unt_cics_audit_queue(const tsfi_cw_unt_cics_queue *queue, int *alert_out);

#endif // TSFI_MAINFRAME_COMPUTERWORLD_H

