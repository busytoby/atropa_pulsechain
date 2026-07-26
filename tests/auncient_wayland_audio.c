// Wayland Markdown Editor audio emulation registers and variables
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint16_t freq;
    uint16_t pw;
    uint8_t ctrl;
    uint8_t adsr[2];
} SidVoice;

static struct {
    SidVoice voices[3];
    uint16_t filter_freq;
    uint8_t filter_ctrl;
    uint8_t volume;
    bool hard_sync_enabled;
    bool ring_mod_enabled;
} sid_chip;

static struct {
    struct {
        uint16_t freq;
        uint8_t volume;
        uint8_t pan;
        uint8_t waveform[32];
    } channels[6];
} huc6280_psg;

static struct {
    uint8_t ff06;
    uint8_t ff07;
    uint8_t ff09;
    uint8_t ff0a;
    uint8_t ff0b;
    uint8_t ff12;
    uint8_t ff14;
    uint8_t ff15;
    uint8_t ff16;
    uint8_t ff19;
} ted_chip;

static int sid_arp_step = 0;
static bool voice_active[3] = {true, true, true};
static int active_tune = 0;
static bool hidden_unlocked = false;

static const uint16_t sid_tunes[4][3] = {
    {0x1100, 0x1500, 0x1A00},
    {0x1200, 0x1600, 0x1C00},
    {0x1400, 0x1800, 0x1E00},
    {0x0800, 0x0C00, 0x1100}
};
