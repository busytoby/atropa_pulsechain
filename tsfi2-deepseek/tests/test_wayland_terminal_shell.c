#define _GNU_SOURCE
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <poll.h>
#include <dirent.h>
#include <sys/stat.h>
#include <wayland-client.h>
#include "xdg-shell-client-protocol.h"
#include <linux/input.h>
#include <time.h>
#include "lau_memory.h"
#include "lau_registry.h"
#include "tsfi_wire_firmware.h"
#include "lau_vram.h"
#include "tsfi_zmm_vm.h"
#include "tsfi_staging.h"
#include "tsfi_vision.h"
#include "tsfi_jpeg_encoder.h"
#include "tsfi_ottype.h"

static bool g_superterm_mode = true;
static int g_superterm_cols = 132;
static int g_superterm_scroll_x = 0;

typedef enum {
    MODE_TERMINAL,
    MODE_WORDCRAFT,
    MODE_EASYSCRIPT,
    MODE_DNATYPEWRITER,
    MODE_ZMACHINE,
    MODE_INSTAWRITER,
    MODE_INSTACALC,
    MODE_APPLEPANIC,
    MODE_AIRASSAULT,
    MODE_SLINKYBEAR,
    MODE_SLINKYPANIC,
    MODE_YULBUILD,
    MODE_CREATOR,
    MODE_ALPINER,
    MODE_CHECKLIST,
    MODE_WHATSMYJOB,
    MODE_PTE,
    MODE_SPACEPATROL,
    MODE_CONSTRUCTION_CO,
    MODE_STUDIO64,
    MODE_MAGPIE,
    MODE_ALICE,
    MODE_TOP,
    MODE_FONTASIA,
    MODE_FLANKSPEED,
    MODE_BOOTER,
    MODE_HOPAROUND,
    MODE_TOWERS,
    MODE_DISINTEGRATOR,
    MODE_FIDGITS,
    MODE_MOXEY,
    MODE_DRUM,
    MODE_JEWEL,
    MODE_SANTA,
    MODE_CLOAK,
    MODE_GYPSY,
    MODE_MARTIAN,
    MODE_HAUNTED,
    MODE_INFRARAID,
    MODE_STREAMER,
    MODE_KNOCKOUT,
    MODE_ALARM,
    MODE_MEMCHECK,
    MODE_ARENA,
    MODE_HEADTOHEAD,
    MODE_CRABFIGHT,
    MODE_TREASURE,
    MODE_CHARDUMP,
    MODE_TERM128,
    MODE_ALCHEMIST,
    MODE_SKIFOLLY,
    MODE_DARTS,
    MODE_MAKEWAVE,
    MODE_PLOTWAVE
} EditorMode;
static EditorMode g_editor_mode = MODE_TERMINAL;
static bool g_faster64_active = false;
static bool g_dashboard_active = false;
static bool g_aitest_active = false;
static const char* g_test_statuses[24] = { "READY", "READY", "READY", "READY", "READY", "READY", "READY", "READY", "READY", "READY", "READY", "READY", "READY", "READY", "READY", "READY", "READY", "READY", "READY", "READY", "READY", "READY", "READY", "READY" };

// StagingBuffer is defined in tsfi_staging.h
// draw_debug_codepoint/draw_debug_text are defined in tsfi_staging.h

static struct wl_display *display = NULL;
static struct wl_registry *registry = NULL;
static struct wl_compositor *compositor = NULL;
static struct wl_shm *shm = NULL;
static struct xdg_wm_base *xdg_wm_base = NULL;
static struct wl_seat *seat = NULL;
static struct wl_keyboard *keyboard = NULL;
static struct wl_pointer *pointer = NULL;
static bool drag_selecting = false;
static int select_start_x = -1, select_start_y = -1;
static int select_end_x = -1, select_end_y = -1;
static int mouse_px = -1, mouse_py = -1;
static uint32_t last_click_time = 0;
static int click_count = 0;
typedef enum {
    GFX_LINE,
    GFX_CIRCLE,
    GFX_POINT,
    GFX_TEXT,
    GFX_STUFFED_3D,
    GFX_FILL_RECT,
    GFX_ROUND_RECT,
    GFX_PMG_PLAYER
} GfxType;

typedef struct {
    GfxType type;
    int x1, y1, x2, y2;
    int r;
    uint32_t color;
    char text[32];
    char query[32];
    int frame;
} GfxPrimitive;

#define MAX_GFX_PRIMITIVES 1024
static GfxPrimitive gfx_primitives[MAX_GFX_PRIMITIVES];
static int gfx_primitive_count = 0;
static int mon_x = 22;
static int mon_y = 67;
void render_terminal_display(void);
static void log_telemetry(const char *event_name);

static struct wl_surface *surface = NULL;
static struct xdg_surface *xdg_surface = NULL;
static struct xdg_toplevel *xdg_toplevel = NULL;
static struct wl_buffer *wl_buffers[2] = {NULL, NULL};
static int current_buffer_idx = 0;

static int win_width = 1280;
static int win_height = 720;
static uint32_t *pixel_datas[2] = {NULL, NULL};
static uint32_t *back_buffer = NULL;
static LauVRAM *g_vram = NULL;
static TsfiZmmVmState vm;
static char cmd_buf[512] = {0};
static int cmd_len = 0;
static volatile bool running = true;
static bool configured = false;
static int pending_width = 0;
static int pending_height = 0;
static bool resize_pending = false;
static bool g_mercenary_active = false;
static void update_mercenary_yul_camera(void);
static bool g_pong_active = false;
static bool g_pong_loaded = false;
static bool g_key_up_pressed = false;
static bool g_key_down_pressed = false;
static void update_pong_game(void);
static void update_alpiner_game(void);
static void redraw_alpiner_screen(void);
static void handle_alpiner_input(char ch);
static void init_checklist(void);
static void redraw_checklist_screen(void);
static void handle_checklist_input(char ch);
static void init_job_game(void);
static void redraw_job_screen(void);
static void handle_job_input(char ch);

#define PTE_MAX_LINES 128
#define PTE_LINE_WIDTH 128
static char g_pte_lines[PTE_MAX_LINES][PTE_LINE_WIDTH];
static int g_pte_line_count = 0;
static int g_pte_cursor_x = 0;
static int g_pte_cursor_y = 0;
static int g_pte_left_margin = 5;
static int g_pte_right_margin = 75;
static int g_pte_edit_margin_mode = 0; // 0=none, 1=left, 2=right
static char g_pte_status_msg[128];
static void init_pte(void);
static void redraw_pte_screen(void);
static void handle_pte_input(char ch);

static double g_sp_alt = 10000.0;
static double g_sp_speed = 350.0;
static double g_sp_pitch = 0.0;
static double g_sp_yaw = 0.0;
static int g_sp_throttle = 60;
static double g_sp_fuel = 95.5;
static double g_sp_shields = 100.0;
static double g_sp_target_dist = 5000.0;
static char g_sp_status[128];
static void init_spacepatrol(void);
static void redraw_spacepatrol_screen(void);
static void handle_spacepatrol_input(char ch);
static void update_spacepatrol_simulation(void);

// Construction Co Cargo Flight Simulator variables
static double g_cc_crane_x = 5.0;
static double g_cc_crane_y = 2.0;
static double g_cc_velocity_y = 0.0;
static double g_cc_velocity_x = 0.0;
static int g_cc_has_cargo = 0;
static int g_cc_score = 0;
static double g_cc_fuel = 100.0;
static int g_cc_grid[10][10] = {{0}}; // 0 = empty, 1 = scaffold, 2 = targeted area
static char g_cc_status[128];
static void init_construction_co(void);
static void redraw_construction_co_screen(void);
static void handle_construction_co_input(char ch);
static void update_construction_co_simulation(void);

// Studio 64 Music Word Processor variables
static int g_s64_cursor_step = 0; // 0 to 7
static int g_s64_cursor_pitch = 4; // 0 to 7 (C4 to C5)
static int g_s64_sequence[8] = {-1, -1, -1, -1, -1, -1, -1, -1}; // -1 = rest, 0-7 = note
static char g_s64_status[256];
static void init_studio64(void);
static void redraw_studio64_screen(void);
static void handle_studio64_input(char ch);

// Magpie Database Manager variables
typedef struct {
    char name[32];
    char dept[32];
    double salary;
} MagpieRecord;
static MagpieRecord g_magpie_db[10];
static int g_magpie_count = 0;
static char g_magpie_status[128];
static char g_magpie_query[32] = "";
static int g_magpie_graph_mode = 0; // 0 = list table, 1 = horizontal bar chart
static void init_magpie(void);
static void redraw_magpie_screen(void);
static void handle_magpie_input(char ch);

// Alice in Adventureland variables
static int g_alice_room = 0; // 0=Forest, 1=Rabbit Hole, 2=Wonderland Hall
static int g_alice_has_key = 0;
static int g_alice_door_unlocked = 0;
static char g_alice_status[256];
static char g_alice_input_buf[64];
static int g_alice_input_len = 0;
static void init_alice(void);
static void redraw_alice_screen(void);
static void handle_alice_input(char ch);

// To the Top variables
static int g_top_player_x = 10;
static int g_top_player_y = 9; // Bottom row
static int g_top_height = 0;
static int g_top_lives = 3;
static int g_top_score = 0;
static int g_top_rock_x = 5;
static int g_top_rock_y = 2;
static char g_top_status[128];
static void init_top(void);
static void redraw_top_screen(void);
static void handle_top_input(char ch);
static void update_top_simulation(void);

// Fontasia variables
static int g_fontasia_grid[8][8] = {{0}}; // 0=off, 1=on
static int g_fontasia_cursor_x = 0;
static int g_fontasia_cursor_y = 0;
static char g_fontasia_status[128];
static void init_fontasia(void);
static void redraw_fontasia_screen(void);
static void handle_fontasia_input(char ch);

// Flankspeed variables
static uint16_t g_flankspeed_start_addr = 0xC000;
static char g_flankspeed_buffer[16][8][3]; // 16 rows, 8 columns, 2 hex characters + null
static int g_flankspeed_cursor_row = 0;
static int g_flankspeed_cursor_col = 0;
static int g_flankspeed_char_idx = 0; // 0=first hex char, 1=second hex char
static char g_flankspeed_status[128];
static bool g_flankspeed_assembling = false;
static char g_flankspeed_asm_buf[64] = {0};
static int g_flankspeed_asm_len = 0;
static void init_flankspeed(void);
static void redraw_flankspeed_screen(void);
static void handle_flankspeed_input(char ch);
static bool flankspeed_assemble(const char *instr);

// Dynamic Assembly Symbol Dictionary
typedef struct {
    char name[32];
    char value[32];
} FlankspeedSymbol;
static FlankspeedSymbol g_flankspeed_symbols[128];
static int g_flankspeed_symbol_count = 0;

// Programmable Functions variables
static char g_fkey_macros[8][32] = {
    "LIST\n",
    "RUN\n",
    "LOAD\n",
    "SAVE\n",
    "DIR\n",
    "NEW\n",
    "OLD\n",
    "HELP\n"
};

// Booter variables
static const char *g_booter_entries[] = {
    "ALICE (Adventure Game)",
    "TOP (Arcade Platformer)",
    "FONTASIA (Custom Sprite Editor)",
    "FLANKSPEED (Hex Assembler Entry)",
    "CHECKLIST (To-Do Management Widget)",
    "YULBUILD (Yul compiler and assembly tool)",
    "HOPAROUND (Board Strategy Game)",
    "TOWERS (Towers of Hanoi Puzzle)",
    "DISINTEGRATOR (ML Particle Grid Shooter)",
    "FIDGITS (Alphabet Sorting Game)",
    "MOXEY'S PORCH (Text Adventure Game)",
    "RHYTHMIC BITS (Step Sequencer Drum Machine)",
    "JEWEL QUEST (Arcade Logic Match Game)",
    "SANTA'S BUSY DAY (Festive chimney delivery)",
    "CLOAK (Espionage Text Adventure)",
    "GYPSY STARSHIP (Space Trading Simulation)"
};
static int g_booter_count = 16;
static int g_booter_cursor = 0;
static char g_booter_status[128];
static void init_booter(void);
static void redraw_booter_screen(void);
static void handle_booter_input(char ch);

// Hop Around variables
static int g_hoparound_grid[8][8];
static int g_hoparound_x = 4;
static int g_hoparound_y = 4;
static int g_hoparound_score = 0;
static int g_hoparound_moves = 0;
static char g_hoparound_status[128];
static void init_hoparound(void);
static void redraw_hoparound_screen(void);
static void handle_hoparound_input(char ch);

// Towers of Hanoi variables
static int g_towers_pegs[3][5];
static int g_towers_counts[3];
static int g_towers_moves = 0;
static int g_towers_src = -1; // -1 means none
static char g_towers_status[128];
static void init_towers(void);
static void redraw_towers_screen(void);
static void handle_towers_input(char ch);

// Disintegrator variables
static int g_disint_px = 4;
static int g_disint_py = 4;
static int g_disint_tx = 0;
static int g_disint_ty = 0;
static int g_disint_score = 0;
static int g_disint_lives = 3;
static char g_disint_status[128];
static void init_disintegrator(void);
static void redraw_disintegrator_screen(void);
static void handle_disintegrator_input(char ch);

// Fidgits variables
static char g_fidgits_arr[5];
static int g_fidgits_cursor = 0;
static int g_fidgits_selected = -1;
static int g_fidgits_moves = 0;
static char g_fidgits_status[128];
static void init_fidgits(void);
static void redraw_fidgits_screen(void);
static void handle_fidgits_input(char ch);

// Moxey's Porch variables
static int g_moxey_room = 0; // 0=Porch, 1=Living Room, 2=Kitchen, 3=Basement
static bool g_moxey_has_key = false;
static bool g_moxey_has_lantern = false;
static bool g_moxey_lantern_lit = false;
static bool g_moxey_chest_unlocked = false;
static char g_moxey_status[128];
static void init_moxey(void);
static void redraw_moxey_screen(void);
static void handle_moxey_input(char ch);

// Rhythmic Bits variables
static bool g_drum_grid[4][8]; // 4 tracks, 8 steps
static int g_drum_cursor_track = 0;
static int g_drum_cursor_step = 0;
static int g_drum_play_step = -1;
static bool g_drum_playing = false;
static uint32_t g_drum_last_tick = 0;
static char g_drum_status[128];
static void init_drum(void);
static void redraw_drum_screen(void);
static void handle_drum_input(char ch);
static void update_drum_seq(uint32_t current_time);

// Jewel Quest variables
static char g_jewel_grid[6][6];
static int g_jewel_cx = 0, g_jewel_cy = 0;
static int g_jewel_score = 0;
static char g_jewel_status[128];
static void init_jewel(void);
static void redraw_jewel_screen(void);
static void handle_jewel_input(char ch);

// Santa's Busy Day variables
static int g_santa_x = 0;
static int g_santa_dir = 1;
static int g_present_y = -1;
static int g_present_x = -1;
static int g_chimney_x[3];
static int g_santa_score = 0;
static char g_santa_status[128];
static void init_santa(void);
static void redraw_santa_screen(void);
static void handle_santa_input(char ch);
static void update_santa(uint32_t current_time);
static uint32_t g_santa_last_tick = 0;

// Cloak variables
static int g_cloak_room = 0; // 0=Entrance, 1=Vault, 2=Security
static bool g_cloak_has_film = false;
static bool g_cloak_alarm = false;
static char g_cloak_status[128];
static void init_cloak(void);
static void redraw_cloak_screen(void);
static void handle_cloak_input(char ch);

// Gypsy Starship variables
static int g_gypsy_fuel = 100;
static int g_gypsy_credits = 500;
static int g_gypsy_cargo = 0;
static int g_gypsy_sector = 1;
static char g_gypsy_status[128];
static void init_gypsy(void);
static void redraw_gypsy_screen(void);
static void handle_gypsy_input(char ch);

// Martian Monsters variables & functions
static int g_martian_ship_x = 20;
static int g_martian_monster_x = 10;
static int g_martian_monster_y = 2;
static int g_martian_score = 0;
static int g_martian_lives = 3;
static char g_martian_status[128];
static void init_martian(void);
static void redraw_martian_screen(void);
static void handle_martian_input(char ch);
static void update_martian(uint32_t ms);

// Haunted Castle variables & functions
static int g_haunted_x = 1;
static int g_haunted_y = 1;
static int g_haunted_gold = 0;
static int g_haunted_ghost_x = 8;
static int g_haunted_ghost_y = 8;
static char g_haunted_status[128];
static void init_haunted(void);
static void redraw_haunted_screen(void);
static void handle_haunted_input(char ch);

// Infraraid variables & functions
static int g_infraraid_angle = 90;
static int g_infraraid_sweep = 0;
static int g_infraraid_targets = 5;
static char g_infraraid_status[128];
static void init_infraraid(void);
static void redraw_infraraid_screen(void);
static void handle_infraraid_input(char ch);

// Streamer Font variables & functions
static uint8_t g_streamer_char[8] = { 0x3C, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x66, 0x00 }; // Capital 'A'
static int g_streamer_cursor_x = 0;
static int g_streamer_cursor_y = 0;
static char g_streamer_status[128];
static void init_streamer(void);
static void redraw_streamer_screen(void);
static void handle_streamer_input(char ch);

// Knockout variables & functions
static int g_knockout_paddle_x = 16;
static int g_knockout_ball_x = 10;
static int g_knockout_ball_y = 10;
static int g_knockout_ball_dx = 1;
static int g_knockout_ball_dy = -1;
static int g_knockout_score = 0;
static uint8_t g_knockout_blocks[4][20];
static void init_knockout(void);
static void redraw_knockout_screen(void);
static void handle_knockout_input(char ch);
static void update_knockout(uint32_t ms);

// Alarm Clock variables & functions
static int g_alarm_h = 12;
static int g_alarm_m = 0;
static int g_alarm_s = 0;
static int g_alarm_set_h = 12;
static int g_alarm_set_m = 5;
static bool g_alarm_triggered = false;
static char g_alarm_status[128];
static void init_alarm(void);
static void redraw_alarm_screen(void);
static void handle_alarm_input(char ch);

// Memory Check variables & functions
static uint32_t g_memcheck_addr = 0;
static int g_memcheck_errors = 0;
static char g_memcheck_status[128];
static void init_memcheck(void);
static void redraw_memcheck_screen(void);
static void handle_memcheck_input(char ch);
static void update_memcheck(uint32_t ms);

// Arena variables & functions
static int g_arena_x = 2;
static int g_arena_y = 2;
static int g_arena_enemy_x = 8;
static int g_arena_enemy_y = 6;
static int g_arena_score = 0;
static char g_arena_status[128];
static void init_arena(void);
static void redraw_arena_screen(void);
static void handle_arena_input(char ch);
static void update_arena(uint32_t ms);

// Head to Head variables & functions
static int g_h2h_p1_x = 0;
static int g_h2h_p1_y = 0;
static int g_h2h_p2_x = 9;
static int g_h2h_p2_y = 9;
static uint8_t g_h2h_grid[10][10];
static int g_h2h_p1_score = 0;
static int g_h2h_p2_score = 0;
static char g_h2h_status[128];
static void init_headtohead(void);
static void redraw_headtohead_screen(void);
static void handle_headtohead_input(char ch);

// Crabfight variables & functions
static int g_crab_x = 10;
static int g_crab_opp_x = 30;
static int g_crab_hp = 100;
static int g_crab_opp_hp = 100;
static char g_crab_status[128];
static void init_crabfight(void);
static void redraw_crabfight_screen(void);
static void handle_crabfight_input(char ch);

// Treasure Wheel variables & functions
static char g_wheel_phrase[32] = "AHOY MAGAZINE";
static char g_wheel_guessed[32] = "____ ________";
static int g_wheel_score = 0;
static int g_wheel_spin_val = 100;
static char g_wheel_status[128];
static void init_treasure(void);
static void redraw_treasure_screen(void);
static void handle_treasure_input(char ch);

// Character Dump variables & functions
static uint16_t g_chardump_offset = 0;
static char g_chardump_status[128];
static void init_chardump(void);
static void redraw_chardump_screen(void);
static void handle_chardump_input(char ch);
static void update_chardump(uint32_t ms);

// Term 128 variables & functions
static int g_term128_state = 0; // 0=dialing, 1=connected
static uint32_t g_term128_connect_time = 0;
static char g_term128_buffer[1024];
static void init_term128(void);
static void redraw_term128_screen(void);
static void handle_term128_input(char ch);
static void update_term128(uint32_t ms);

// Alchemist's Apprentice variables & functions
static char g_alchemist_log[512];
static char g_alchemist_input[64];
static int g_alchemist_input_len = 0;
static bool g_alchemist_has_flask = false;
static bool g_alchemist_has_powder = false;
static bool g_alchemist_potion_mixed = false;
static void init_alchemist(void);
static void redraw_alchemist_screen(void);
static void handle_alchemist_input(char ch);

// Ski Folly variables & functions
static int g_skifolly_player_x = 40;
static int g_skifolly_score = 0;
static int g_skifolly_gate_x = 40;
static int g_skifolly_gate_w = 12;
static int g_skifolly_row_counter = 0;
static bool g_skifolly_crashed = false;
static void init_skifolly(void);
static void redraw_skifolly_screen(void);
static void handle_skifolly_input(char ch);
static void update_skifolly(uint32_t ms);

// English Darts variables & functions
static int g_darts_aim_x = 40;
static int g_darts_aim_y = 12;
static int g_darts_score = 0;
static int g_darts_throws = 3;
static int g_darts_state = 0; // 0=aiming x, 1=aiming y, 2=thrown
static int g_darts_aim_dir = 1;
static void init_darts(void);
static void redraw_darts_screen(void);
static void handle_darts_input(char ch);
static void update_darts(uint32_t ms);

// MAKEWAVE & PLOTWAVE variables & functions
static int g_makewave_type = 0; // 0: Sine, 1: Square, 2: Triangle, 3: Sawtooth, 4: Noise
static float g_makewave_frequency = 4.0f; // low freq for visual plotting
static float g_makewave_amplitude = 1.0f;
static float g_makewave_duty = 0.5f;
static float g_makewave_table[256];
static char g_makewave_status[128];
static void init_makewave(void);
static void redraw_makewave_screen(void);
static void handle_makewave_input(char ch);
static void init_plotwave(void);
static void redraw_plotwave_screen(void);
static void handle_plotwave_input(char ch);
static void update_plotwave(uint32_t ms);


static double g_calc_cells[5][5] = {
    { 100.0, 50.0, 150.0, 0.0, 0.0 },
    { 20.0,  30.0, 50.0,  0.0, 0.0 },
    { 120.0, 80.0, 200.0, 0.0, 0.0 },
    { 0.0,   0.0,  0.0,   0.0, 0.0 },
    { 0.0,   0.0,  0.0,   0.0, 0.0 }
};
static int g_calc_cursor_row = 0;
static int g_calc_cursor_col = 0;
static char g_calc_input_buffer[32] = {0};
static int g_calc_input_len = 0;

static bool g_applepanic_active = false;
static int g_panic_player_x = 2;
static int g_panic_player_y = 3;
static int g_panic_player_facing = 1;
static int g_panic_monster_x = 25;
static int g_panic_monster_y = 3;
static int g_panic_monster_stuck_ticks = 0;
static int g_panic_score = 0;
static int g_panic_lives = 3;
static int g_panic_dig_ticks[4][40] = {{0}};

static bool g_airassault_active = false;
static int g_air_player_x = 20;
static int g_air_missile_x = -1;
static int g_air_missile_y = -1;
static int g_air_invaders_x[5] = { 5, 15, 25, 35, 10 };
static int g_air_invaders_y[5] = { 0, 1, 0, 2, 1 };
static int g_air_score = 0;
static int g_air_shields = 3;

static bool g_slinkybear_active = false;
static int g_slinky_row = 0;
static int g_slinky_col = 0;
static int g_slinky_blocks[5][5] = {{0}};
static int g_slinky_score = 0;
static int g_slinky_lives = 3;

static bool g_slinkypanic_active = false;
static int g_slinky_monster_row = 4;
static int g_slinky_monster_col = 4;
static int g_slinky_monster_stuck = 0;
static int g_slinky_hole[5][5] = {{0}};

static bool g_alpiner_active = false;
static int g_alpiner_player_x = 22;
static int g_alpiner_player_y = 12;
static int g_alpiner_yeti_x = 18;
static int g_alpiner_yeti_y = 6;
static int g_alpiner_rock_x = 22;
static int g_alpiner_rock_y = 4;
static int g_alpiner_score = 0;
static int g_alpiner_lives = 3;
static char g_alpiner_speech[64] = "ALPINER!";
static int g_alpiner_speech_ticks = 15;
static int g_alpiner_mountain = 1;
static uint8_t g_alpiner_tms5220_status = 0x40;
static uint8_t g_alpiner_tms5220_cmd = 0x50;
static int g_alpiner_tms5220_fifo = 16;

typedef struct {
    bool checked;
    char text[64];
    char category[32];
} ChecklistItem;

static bool g_checklist_active = false;
static ChecklistItem g_checklist_items[30];
static int g_checklist_count = 0;
static int g_checklist_cursor = 0;
static char g_checklist_status_msg[128] = "";
static int g_checklist_entry_mode = 0; // 0 = nav, 1 = add desc, 2 = add cat, 3 = edit desc, 4 = edit cat
static char g_checklist_entry_buf[64] = "";
static int g_checklist_entry_len = 0;

typedef struct {
    char text[128];
    int yes_child;
    int no_child;
} JobNode;

static bool g_jobs_active = false;
static JobNode g_job_nodes[50];
static int g_job_nodes_count = 0;
static int g_job_current_node = 0;
static int g_job_prev_node = -1;
static bool g_job_last_answer_was_yes = false;
static int g_job_submode = 0; // 0 = traversal, 1 = guess, 2 = learn name, 3 = learn quest, 4 = yesno
static char g_job_new_name[64] = "";
static char g_job_new_question[128] = "";

typedef struct {
    char key;
    char cmd[64];
} ChrgetHook;
static ChrgetHook g_chrget_hooks[16];
static int g_chrget_hooks_count = 0;

// Registry listeners
static void registry_handle_global(void *data, struct wl_registry *registry, uint32_t name, const char *interface, uint32_t version) {
    (void)data; (void)version;
    if (strcmp(interface, wl_compositor_interface.name) == 0) {
        compositor = wl_registry_bind(registry, name, &wl_compositor_interface, 4);
    } else if (strcmp(interface, wl_shm_interface.name) == 0) {
        shm = wl_registry_bind(registry, name, &wl_shm_interface, 1);
    } else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
        xdg_wm_base = wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
    } else if (strcmp(interface, wl_seat_interface.name) == 0) {
        seat = wl_registry_bind(registry, name, &wl_seat_interface, 1);
    }
}
static void registry_handle_global_remove(void *data, struct wl_registry *registry, uint32_t name) {
    (void)data; (void)registry; (void)name;
}
static const struct wl_registry_listener registry_listener = {
    .global = registry_handle_global,
    .global_remove = registry_handle_global_remove
};

// Keyboard listeners
static void keyboard_handle_keymap(void *data, struct wl_keyboard *keyboard, uint32_t format, int32_t fd, uint32_t size) {
    (void)data; (void)keyboard; (void)format; (void)size;
    close(fd);
}
static void keyboard_handle_enter(void *data, struct wl_keyboard *keyboard, uint32_t serial, struct wl_surface *surface, struct wl_array *keys) {
    (void)data; (void)keyboard; (void)serial; (void)surface; (void)keys;
}
static void keyboard_handle_leave(void *data, struct wl_keyboard *keyboard, uint32_t serial, struct wl_surface *surface) {
    (void)data; (void)keyboard; (void)serial; (void)surface;
}

extern uint32_t active_modifiers;
static void keyboard_handle_modifiers(void *data, struct wl_keyboard *keyboard, uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group) {
    (void)data; (void)keyboard; (void)serial; (void)group; (void)mods_latched;
    if (mods_depressed & 1) active_modifiers |= 1;
    else active_modifiers &= ~1;
    if (mods_locked & 2) active_modifiers |= 16;
    else active_modifiers &= ~16;
}
static void keyboard_handle_repeat_info(void *data, struct wl_keyboard *keyboard, int32_t rate, int32_t delay) {
    (void)data; (void)keyboard; (void)rate; (void)delay;
}

static void add_line(int x1, int y1, int x2, int y2, uint32_t color);
static void add_circle(int x, int y, int r, uint32_t color);
static void add_text(int x, int y, const char *text, uint32_t color);
static void add_fill_rect(int x1, int y1, int x2, int y2, uint32_t color);
static void add_round_rect(int x1, int y1, int x2, int y2, int r, uint32_t color);
static void add_pmg_player(int player_idx, int x_offset, int y_offset);

static void terminal_write_string(LauVRAM *vram, const char *str, int len) {
    static int state = 0; // 0: normal, 1: esc, 2: bracket, 3: gfx_command
    static char parse_buf[128];
    static int parse_len = 0;
    static int vidtex_x = 0;
    
    for (int i = 0; i < len; i++) {
        char c = str[i];
        if (state == 0) {
            if (c == '\x1b') {
                state = 1;
            } else if (c == '\x01') {
                state = 7; // HMI Frame Parsing state
                parse_len = 0;
            } else {
                lau_vram_write_char(vram, c);
            }
        } else if (state == 1) {
            if (c == '[') {
                state = 2;
            } else if (c == '\x1b') {
                state = 4; // CompuServe Vidtex ESC ESC State
            } else {
                state = 0;
                lau_vram_write_char(vram, '\x1b');
                lau_vram_write_char(vram, c);
            }
        } else if (state == 2) {
            if (c == 'G') {
                state = 3;
                parse_len = 0;
            } else {
                state = 0;
                lau_vram_write_char(vram, '\x1b');
                lau_vram_write_char(vram, '[');
                lau_vram_write_char(vram, c);
            }
        } else if (state == 3) {
            if (c == 'm') {
                parse_buf[parse_len] = '\0';
                char *tokens[16];
                int tok_count = 0;
                char *tok = strtok(parse_buf, ";");
                while (tok && tok_count < 16) {
                    tokens[tok_count++] = tok;
                    tok = strtok(NULL, ";");
                }
                if (tok_count >= 1) {
                    int cmd = atoi(tokens[0]);
                    if (cmd == 0) {
                        gfx_primitive_count = 0;
                    } else if (cmd == 1 && tok_count >= 5) {
                        int x1 = atoi(tokens[1]);
                        int y1 = atoi(tokens[2]);
                        int x2 = atoi(tokens[3]);
                        int y2 = atoi(tokens[4]);
                        uint32_t color = (tok_count >= 6) ? (uint32_t)strtoul(tokens[5], NULL, 0) : 0xFF50FA7B;
                        if (gfx_primitive_count < MAX_GFX_PRIMITIVES) {
                            GfxPrimitive *gp = &gfx_primitives[gfx_primitive_count++];
                            gp->type = GFX_LINE; gp->x1 = x1; gp->y1 = y1; gp->x2 = x2; gp->y2 = y2; gp->color = color;
                        }
                    } else if (cmd == 2 && tok_count >= 4) {
                        int x = atoi(tokens[1]);
                        int y = atoi(tokens[2]);
                        int r = atoi(tokens[3]);
                        uint32_t color = (tok_count >= 5) ? (uint32_t)strtoul(tokens[4], NULL, 0) : 0xFF8BE9FD;
                        if (gfx_primitive_count < MAX_GFX_PRIMITIVES) {
                            GfxPrimitive *gp = &gfx_primitives[gfx_primitive_count++];
                            gp->type = GFX_CIRCLE; gp->x1 = x; gp->y1 = y; gp->r = r; gp->color = color;
                        }
                    } else if (cmd == 3 && tok_count >= 3) {
                        int x = atoi(tokens[1]);
                        int y = atoi(tokens[2]);
                        uint32_t color = 0xFF50FA7B;
                        if (tok_count >= 4) {
                            char *col_str = tokens[3];
                            if (strlen(col_str) <= 2) {
                                int idx = atoi(col_str);
                                color = (idx == 1) ? 0xFFFF5555 : 0xFFF1FA8C;
                            } else {
                                color = (uint32_t)strtoul(col_str, NULL, 0);
                            }
                        }
                        if (gfx_primitive_count < MAX_GFX_PRIMITIVES) {
                            GfxPrimitive *gp = &gfx_primitives[gfx_primitive_count++];
                            gp->type = GFX_POINT; gp->x1 = x; gp->y1 = y; gp->color = color;
                        }
                    } else if (cmd == 4 && tok_count >= 4) {
                        int x = atoi(tokens[1]);
                        int y = atoi(tokens[2]);
                        uint32_t color = (uint32_t)strtoul(tokens[3], NULL, 0);
                        char text_buf[64] = "";
                        if (tok_count >= 5) {
                            int tlen = strlen(tokens[4]);
                            if (tlen > 63) tlen = 63;
                            memcpy(text_buf, tokens[4], tlen);
                            text_buf[tlen] = '\0';
                        }
                        if (gfx_primitive_count < MAX_GFX_PRIMITIVES) {
                            GfxPrimitive *gp = &gfx_primitives[gfx_primitive_count++];
                            gp->type = GFX_TEXT; gp->x1 = x; gp->y1 = y; gp->color = color;
                            int glen = strlen(text_buf);
                            if (glen > 31) glen = 31;
                            memcpy(gp->text, text_buf, glen);
                            gp->text[glen] = '\0';
                        }
                    }
                }
                state = 0;
                vram->is_dirty = true;
            } else {
                if (parse_len < (int)sizeof(parse_buf) - 2) {
                    parse_buf[parse_len++] = c;
                }
            }
        } else if (state == 4) {
            if (c == 'd') {
                lau_vram_write_char(vram, '\x1b');
                lau_vram_write_char(vram, '\x1b');
                lau_vram_write_char(vram, 'd'); // clear screen sequence
                state = 0;
            } else if (c == 'c') {
                fprintf(stderr, "[COMPUSERVE] Received capability query (ESC ESC c). Responding with Vidtex ID.\n");
                lau_vram_write_string(vram, "\r\n[CIS VIDTEX TERMINAL IDENTIFIED: HMI v2.0]\r\n", 44);
                state = 0;
            } else if (c == 'I') {
                state = 5;
            } else {
                state = 0;
            }
        } else if (state == 5) {
            int x = (unsigned char)c;
            if (x >= 32) x -= 32;
            vidtex_x = x;
            state = 6;
        } else if (state == 6) {
            int y = (unsigned char)c;
            if (y >= 32) y -= 32;
            vram->cursor_x = vidtex_x;
            vram->cursor_y = y;
            vram->is_dirty = true;
            state = 0;
        } else if (state == 7) {
            if (c == '\x03') {
                state = 8;
                parse_buf[parse_len] = '\0';
            } else {
                if (parse_len < (int)sizeof(parse_buf) - 2) {
                    parse_buf[parse_len++] = c;
                }
            }
        } else if (state == 8) {
            state = 9; // Discard checksum byte 1
        } else if (state == 9) {
            state = 0; // Discard checksum byte 2, process HMI payload
            if (parse_len > 1) {
                char type = parse_buf[0];
                char *payload = &parse_buf[1];
                if (type == 'M') {
                    gfx_primitive_count = 0; // Clear old primitives
                    
                    // Tokenize layout by '|'
                    char *saveptr;
                    char *widget = strtok_r(payload, "|", &saveptr);
                    while (widget) {
                        char *w_type = NULL;
                        char *sx = NULL;
                        char *sy = NULL;
                        char *sw = NULL;
                        char *sh = NULL;
                        char *label = NULL;
                        
                        char *inner_save;
                        w_type = strtok_r(widget, ",", &inner_save);
                        if (w_type) {
                            if (strcmp(w_type, "BOX") == 0) {
                                sx = strtok_r(NULL, ",", &inner_save);
                                sy = strtok_r(NULL, ",", &inner_save);
                                sw = strtok_r(NULL, ",", &inner_save);
                                sh = strtok_r(NULL, ",", &inner_save);
                                label = strtok_r(NULL, ",", &inner_save);
                                if (sx && sy && sw && sh && label) {
                                    int x = atoi(sx);
                                    int y = atoi(sy);
                                    int w = atoi(sw);
                                    int h = atoi(sh);
                                    // 1. Drop shadow
                                    add_fill_rect(x + 4, y + 4, x + w + 4, y + h + 4, 0x80000000);
                                    // 2. Solid panel background
                                    add_fill_rect(x, y, x + w, y + h, 0xFF1E1F29);
                                    // 3. Purple border
                                    add_line(x, y, x + w, y, 0xFFBD93F9);
                                    add_line(x, y + h, x + w, y + h, 0xFFBD93F9);
                                    add_line(x, y, x, y + h, 0xFFBD93F9);
                                    add_line(x + w, y, x + w, y + h, 0xFFBD93F9);
                                    // 4. Header text
                                    add_text(x + 8, y + 8, label, 0xFFF1FA8C);
                                }
                            } else if (strcmp(w_type, "BUTTON") == 0) {
                                sx = strtok_r(NULL, ",", &inner_save);
                                sy = strtok_r(NULL, ",", &inner_save);
                                label = strtok_r(NULL, ",", &inner_save);
                                if (sx && sy && label) {
                                    int x = atoi(sx);
                                    int y = atoi(sy);
                                    int w = strlen(label) * 9 + 14;
                                    int h = 22;
                                    // 1. Button shadow
                                    add_fill_rect(x + 2, y + 2, x + w + 2, y + h + 2, 0x60000000);
                                    // 2. Button base (rounded green rect)
                                    add_round_rect(x, y, x + w, y + h, 4, 0xFF50FA7B);
                                    // 3. Button text
                                    add_text(x + 7, y + 5, label, 0xFF282A36);
                                }
                            } else if (strcmp(w_type, "PMG") == 0) {
                                sx = strtok_r(NULL, ",", &inner_save);
                                sy = strtok_r(NULL, ",", &inner_save);
                                char *s_pidx = strtok_r(NULL, ",", &inner_save);
                                if (sx && sy && s_pidx) {
                                    int x = atoi(sx);
                                    int y = atoi(sy);
                                    int pidx = atoi(s_pidx);
                                    add_pmg_player(pidx, x, y);
                                }
                            }
                        }
                        widget = strtok_r(NULL, "|", &saveptr);
                    }
                    vram->is_dirty = true;
                }
            }
        }
    }
}

static void add_line(int x1, int y1, int x2, int y2, uint32_t color) {
    if (gfx_primitive_count < MAX_GFX_PRIMITIVES) {
        GfxPrimitive *gp = &gfx_primitives[gfx_primitive_count++];
        gp->type = GFX_LINE; gp->x1 = x1; gp->y1 = y1; gp->x2 = x2; gp->y2 = y2; gp->color = color;
    }
}
static void add_circle(int x, int y, int r, uint32_t color) {
    if (gfx_primitive_count < MAX_GFX_PRIMITIVES) {
        GfxPrimitive *gp = &gfx_primitives[gfx_primitive_count++];
        gp->type = GFX_CIRCLE; gp->x1 = x; gp->y1 = y; gp->r = r; gp->color = color;
    }
}

static void add_text(int x, int y, const char *text, uint32_t color) {
    if (gfx_primitive_count < MAX_GFX_PRIMITIVES) {
        GfxPrimitive *gp = &gfx_primitives[gfx_primitive_count++];
        gp->type = GFX_TEXT; gp->x1 = x; gp->y1 = y; gp->color = color;
        strncpy(gp->text, text, sizeof(gp->text));
        gp->text[sizeof(gp->text) - 1] = '\0';
    }
}

static void add_fill_rect(int x1, int y1, int x2, int y2, uint32_t color) {
    if (gfx_primitive_count < MAX_GFX_PRIMITIVES) {
        GfxPrimitive *gp = &gfx_primitives[gfx_primitive_count++];
        gp->type = GFX_FILL_RECT; gp->x1 = x1; gp->y1 = y1; gp->x2 = x2; gp->y2 = y2; gp->color = color;
    }
}

static void add_round_rect(int x1, int y1, int x2, int y2, int r, uint32_t color) {
    if (gfx_primitive_count < MAX_GFX_PRIMITIVES) {
        GfxPrimitive *gp = &gfx_primitives[gfx_primitive_count++];
        gp->type = GFX_ROUND_RECT; gp->x1 = x1; gp->y1 = y1; gp->x2 = x2; gp->y2 = y2; gp->r = r; gp->color = color;
    }
}

static void add_pmg_player(int player_idx, int x_offset, int y_offset) {
    if (gfx_primitive_count < MAX_GFX_PRIMITIVES) {
        GfxPrimitive *gp = &gfx_primitives[gfx_primitive_count++];
        gp->type = GFX_PMG_PLAYER; gp->r = player_idx; gp->x1 = x_offset; gp->y1 = y_offset;
    }
}

static int g_frame_counter = 0;

static inline float smin(float a, float b, float k) {
    float h = fmaxf(k - fabsf(a - b), 0.0f) / k;
    return fminf(a, b) - h * h * k * 0.25f;
}

static float sdf_teddy_bear(float x, float y, float z) {
    float d_body = sqrtf(x*x*1.2f + y*y*0.8f + z*z*1.2f) - 0.35f;
    float d_head = sqrtf(x*x + (y - 0.35f)*(y - 0.35f) + z*z) - 0.25f;
    float ex = fabsf(x) - 0.2f, ey = y - 0.55f, ez = z;
    float d_ear = sqrtf(ex*ex + ey*ey + ez*ez) - 0.08f;
    float d_snout = sqrtf(x*x + (y - 0.3f)*(y - 0.3f) + (z - 0.2f)*(z - 0.2f)) - 0.1f;
    
    // Arms
    float ax = fabsf(x) - 0.35f, ay = y - 0.1f, az = z;
    float d_arm = sqrtf(ax*ax*1.5f + ay*ay*1.5f + az*az) - 0.1f;
    
    // Legs
    float lx = fabsf(x) - 0.22f, ly = y + 0.35f, lz = z - 0.1f;
    float d_leg = sqrtf(lx*lx + ly*ly + lz*lz) - 0.12f;
    
    float res = smin(d_body, d_head, 0.12f);
    res = smin(res, d_ear, 0.04f);
    res = smin(res, d_snout, 0.05f);
    res = smin(res, d_arm, 0.05f);
    res = smin(res, d_leg, 0.05f);
    return res;
}

static float sdf_crow(float x, float y, float z) {
    float d_body = sqrtf(x*x*1.5f + y*y + z*z) - 0.3f;
    float d_head = sqrtf((x - 0.3f)*(x - 0.3f) + (y - 0.2f)*(y - 0.2f) + z*z) - 0.15f;
    float bx = x - 0.45f, by = y - 0.2f, bz = z;
    float d_beak = sqrtf(bx*bx*5.0f + by*by*5.0f + bz*bz*5.0f) - 0.1f;
    float wx = fabsf(x), wy = y - 0.1f, wz = fabsf(z) - 0.25f;
    float d_wing = sqrtf(wx*wx*2.0f + wy*wy*0.5f + wz*wz*3.0f) - 0.12f;
    float res = smin(d_body, d_head, 0.08f);
    res = smin(res, d_wing, 0.05f);
    res = smin(res, d_beak, 0.03f);
    return res;
}

static float sdf_cat(float x, float y, float z) {
    float d_body = sqrtf(x*x*1.3f + y*y + z*z) - 0.3f;
    float d_head = sqrtf((x - 0.3f)*(x - 0.3f) + (y - 0.2f)*(y - 0.2f) + z*z) - 0.18f;
    float ex = fabsf(x - 0.35f) - 0.08f, ey = y - 0.38f, ez = z;
    float d_ear = sqrtf(ex*ex*2.0f + ey*ey*2.0f + ez*ez*2.0f) - 0.05f;
    float tx = x + 0.35f, ty = y + 0.1f, tz = z;
    float d_tail = sqrtf(tx*tx + ty*ty + tz*tz) - 0.06f;
    float res = smin(d_body, d_head, 0.08f);
    res = smin(res, d_ear, 0.03f);
    res = smin(res, d_tail, 0.05f);
    return res;
}

static float sdf_fish(float x, float y, float z) {
    float d_body = sqrtf(x*x*0.5f + y*y*2.0f + z*z*3.0f) - 0.25f;
    float tx = x + 0.35f, ty = y, tz = z;
    float d_tail = sqrtf(tx*tx*4.0f + ty*ty*0.2f + tz*tz*4.0f) - 0.15f;
    return smin(d_body, d_tail, 0.05f);
}

static float sdf_tree(float x, float y, float z) {
    float d_trunk = sqrtf(x*x*8.0f + (y + 0.2f)*(y + 0.2f) + z*z*8.0f) - 0.2f;
    float lx = x, ly = y, lz = z;
    float d_leaves1 = sqrtf(lx*lx*3.0f + (ly - 0.1f)*(ly - 0.1f) + lz*lz*3.0f) - 0.35f;
    float d_leaves2 = sqrtf(lx*lx*4.0f + (ly - 0.4f)*(ly - 0.4f) + lz*lz*4.0f) - 0.25f;
    float res = smin(d_trunk, d_leaves1, 0.05f);
    res = smin(res, d_leaves2, 0.05f);
    return res;
}

static float sdf_car(float x, float y, float z) {
    float d_body = sqrtf(x*x*0.8f + (y + 0.05f)*(y + 0.05f)*4.0f + z*z*1.2f) - 0.3f;
    float d_cab = sqrtf(x*x*1.5f + (y - 0.15f)*(y - 0.15f)*2.0f + z*z*2.0f) - 0.2f;
    return smin(d_body, d_cab, 0.05f);
}

static float eval_sdf(const char *query, float x, float y, float z) {
    if (strcasestr(query, "crow") || strcasestr(query, "bird")) {
        return sdf_crow(x, y, z);
    } else if (strcasestr(query, "tree") || strcasestr(query, "plant") || strcasestr(query, "forest")) {
        return sdf_tree(x, y, z);
    } else if (strcasestr(query, "cat") || strcasestr(query, "dog") || strcasestr(query, "pet")) {
        return sdf_cat(x, y, z);
    } else if (strcasestr(query, "fish") || strcasestr(query, "ocean") || strcasestr(query, "sea")) {
        return sdf_fish(x, y, z);
    } else if (strcasestr(query, "car") || strcasestr(query, "drive") || strcasestr(query, "vehicle")) {
        return sdf_car(x, y, z);
    } else {
        return sdf_teddy_bear(x, y, z);
    }
}

static uint32_t get_sdf_color(const char *query, float x, float y, float z, float intensity) {
    uint8_t r = 0, g = 0, b = 0;
    if (strcasestr(query, "crow") || strcasestr(query, "bird")) {
        float bx = x - 0.45f, by = y - 0.2f, bz = z;
        float eyex = x - 0.32f, eyey = y - 0.23f, eyez = fabsf(z) - 0.12f;
        if (sqrtf(eyex*eyex + eyey*eyey + eyez*eyez) < 0.03f) {
            r = 248; g = 248; b = 242; // White of the eye
        } else if (sqrtf(eyex*eyex + eyey*eyey + (eyez-0.01f)*(eyez-0.01f)) < 0.015f) {
            r = 0; g = 0; b = 0; // Black pupil
        } else if (sqrtf(bx*bx*5.0f + by*by*5.0f + bz*bz*5.0f) - 0.1f < 0.05f) {
            r = 255; g = 184; b = 108; // Orange beak
        } else {
            r = 40; g = 42; b = 54; // Dark gray body
        }
    } else if (strcasestr(query, "tree") || strcasestr(query, "plant") || strcasestr(query, "forest")) {
        if (y < -0.1f) {
            r = 139; g = 69; b = 19;
        } else {
            r = 80; g = 250; b = 123;
        }
    } else if (strcasestr(query, "cat") || strcasestr(query, "dog") || strcasestr(query, "pet")) {
        float eyex = fabsf(x - 0.3f) - 0.07f;
        float eyey = y - 0.23f;
        float eyez = z - 0.15f;
        float nosex = x - 0.3f, nosey = y - 0.18f, nosez = z - 0.17f;
        if (sqrtf(eyex*eyex + eyey*eyey + eyez*eyez) < 0.035f) {
            r = 80; g = 250; b = 123; // Green eyes
        } else if (sqrtf(nosex*nosex + nosey*nosey + nosez*nosez) < 0.025f) {
            r = 255; g = 184; b = 108; // Peach nose
        } else {
            r = 255; g = 121; b = 198; // Pink body
        }
    } else if (strcasestr(query, "fish") || strcasestr(query, "ocean") || strcasestr(query, "sea")) {
        float eyex = x - 0.15f, eyey = y - 0.05f, eyez = fabsf(z) - 0.12f;
        if (sqrtf(eyex*eyex + eyey*eyey + eyez*eyez) < 0.035f) {
            r = 255; g = 255; b = 255; // White eye
        } else if (sqrtf(eyex*eyex + eyey*eyey + (eyez-0.01f)*(eyez-0.01f)) < 0.018f) {
            r = 0; g = 0; b = 0; // Pupil
        } else {
            r = 139; g = 233; b = 253; // Blue body
        }
    } else if (strcasestr(query, "car") || strcasestr(query, "drive") || strcasestr(query, "vehicle")) {
        r = 255; g = 85; b = 85;
    } else {
        // Teddy Bear
        float eyex = fabsf(x) - 0.08f;
        float eyey = y - 0.4f;
        float eyez = z - 0.22f;
        float nosex = x, nosey = y - 0.32f, nosez = z - 0.29f;
        if (sqrtf(eyex*eyex + eyey*eyey + eyez*eyez) < 0.04f) {
            r = 10; g = 10; b = 10; // Black eyes
        } else if (sqrtf(nosex*nosex + nosey*nosey + nosez*nosez) < 0.03f) {
            r = 0; g = 0; b = 0; // Black nose
        } else {
            r = 180; g = 120; b = 80; // Brown body
        }
    }
    uint32_t cr = (uint32_t)(r * intensity);
    uint32_t cg = (uint32_t)(g * intensity);
    uint32_t cb = (uint32_t)(b * intensity);
    if (cr > 255) cr = 255;
    if (cg > 255) cg = 255;
    if (cb > 255) cb = 255;
    return 0xFF000000 | (cr << 16) | (cg << 8) | cb;
}

static void draw_3d_stuffed_animal(uint32_t *buffer, int w_width, int w_height, int cx, int cy, int size, const char *query, int frame) {
    float cosY = cosf(frame * 0.08f), sinY = sinf(frame * 0.08f);
    float cosX = cosf(frame * 0.05f), sinX = sinf(frame * 0.05f);
    int r_bound = size;
    int halo_bound = (int)(size * 1.3f);
    for (int dy = -halo_bound; dy <= halo_bound; dy++) {
        for (int dx = -halo_bound; dx <= halo_bound; dx++) {
            int tx = cx + dx;
            int ty = cy + dy;
            if (tx < 12 || tx >= w_width - 22 || ty < 57 || ty >= w_height - 32) continue;
            
            float dist_from_center = sqrtf(dx*dx + dy*dy) / (float)size;
            if (dist_from_center < 1.3f) {
                uint32_t orig = buffer[ty * w_width + tx];
                uint8_t r = (orig >> 16) & 0xFF;
                uint8_t g = (orig >> 8) & 0xFF;
                uint8_t b = orig & 0xFF;
                float dim = 0.15f + 0.85f * (dist_from_center / 1.3f);
                r = (uint8_t)(r * dim);
                g = (uint8_t)(g * dim);
                b = (uint8_t)(b * dim);
                buffer[ty * w_width + tx] = 0xFF000000 | (r << 16) | (g << 8) | b;
            }
            
            if (abs(dx) <= r_bound && abs(dy) <= r_bound) {
                float rx = (float)dx / (float)size;
                float ry = -(float)dy / (float)size;
                float ro_x = rx, ro_y = ry, ro_z = -1.5f;
                float rd_x = 0.0f, rd_y = 0.0f, rd_z = 1.0f;
                float t = 0.0f;
                int hit = 0;
                float hx = 0, hy = 0, hz = 0;
                for (int step = 0; step < 16; step++) {
                    float px = ro_x + rd_x * t;
                    float py = ro_y + rd_y * t;
                    float pz = ro_z + rd_z * t;
                    float rot_x = px * cosY - pz * sinY;
                    float rot_z = px * sinY + pz * cosY;
                    float rot_y = py * cosX - rot_z * sinX;
                    rot_z = py * sinX + rot_z * cosX;
                    float d = eval_sdf(query, rot_x, rot_y, rot_z);
                    if (d < 0.01f) {
                        hit = 1;
                        hx = rot_x; hy = rot_y; hz = rot_z;
                        break;
                    }
                    t += d;
                    if (t > 3.0f) break;
                }
                if (hit) {
                    float eps = 0.01f;
                    float nx = eval_sdf(query, hx + eps, hy, hz) - eval_sdf(query, hx - eps, hy, hz);
                    float ny = eval_sdf(query, hx, hy + eps, hz) - eval_sdf(query, hx, hy - eps, hz);
                    float nz = eval_sdf(query, hx, hy, hz + eps) - eval_sdf(query, hx, hy, hz - eps);
                    float n_len = sqrtf(nx*nx + ny*ny + nz*nz);
                    if (n_len > 0.0f) {
                        nx /= n_len; ny /= n_len; nz /= n_len;
                    }
                    float lx = 0.577f, ly = 0.577f, lz = -0.577f;
                    float dot = nx * lx + ny * ly + nz * lz;
                    float intensity = dot * 0.6f + 0.4f;
                    if (intensity < 0.0f) intensity = 0.0f;
                    if (intensity > 1.0f) intensity = 1.0f;
                    buffer[ty * w_width + tx] = get_sdf_color(query, hx, hy, hz, intensity);
                }
            }
        }
    }
}

static void add_query_icon(const char *query, int x, int y, uint32_t color) {
    (void)color;
    if (gfx_primitive_count < MAX_GFX_PRIMITIVES) {
        GfxPrimitive *gp = &gfx_primitives[gfx_primitive_count++];
        gp->type = GFX_STUFFED_3D; gp->x1 = x; gp->y1 = y; gp->r = 22;
        strncpy(gp->query, query, sizeof(gp->query) - 1);
        gp->query[sizeof(gp->query) - 1] = '\0';

// --- Modular Subcomponent Includes (Rule 8 compliance) ---
#include "wayland_part1.c"
#include "wayland_part2.c"
#include "wayland_part3.c"
#include "wayland_part4.c"
#include "wayland_part5.c"
#include "wayland_part6.c"
#include "wayland_part7.c"
#include "wayland_part8.c"
#include "wayland_part9.c"
#include "wayland_part10.c"
