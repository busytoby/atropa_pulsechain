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
} tsfi_cw_vsam_entry;

typedef struct {
    char filepath[256];
    tsfi_cw_vsam_entry index[128];
    int entry_count;
    uint32_t current_file_size;
    uint32_t ci_splits;
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

// EBCDIC DBCS validation
int tsfi_cw_ebcdic_validate_dbcs_boundaries(const uint8_t *ebcdic_str, int len);

// JCL GDG resolver
int tsfi_cw_jcl_resolve_gdg(const char *dsn_str, int current_gen, char *resolved_out, int max_len);

// Y2K leap year checker
int tsfi_cw_y2k_is_leap_year(uint32_t year);

// 4. Job Control Language (JCL) Execution Simulator
int tsfi_cw_run_jcl(const char **cards, int card_count);
int tsfi_cw_run_jcl_ex(const char **cards, int card_count, int initial_rc);
int tsfi_cw_run_jcl_sysin(const char **cards, int card_count, char *sysin_out, int max_sysin_len);
int tsfi_cw_run_jcl_proc(const char **cards, int card_count, const char **proc_cards, int proc_card_count, int initial_rc);
int tsfi_cw_run_jcl_restart(const char **cards, int card_count, const char *restart_step);
int tsfi_cw_run_jcl_proc_nested(const char **cards, int card_count, const char **proc_cards, int proc_card_count, int initial_rc, int depth);

#endif // TSFI_MAINFRAME_COMPUTERWORLD_H

