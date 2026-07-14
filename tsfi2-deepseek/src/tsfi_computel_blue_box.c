#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <sys/file.h>

/*
 * Auncient Computel Single-Frequency (SF) & Multi-Frequency (MF) Switch Controller
 * 
 * Simulates trunk seizure via 2600 Hz SF tones and routing via MF digits (Blue Box).
 */

#ifndef SAMPLE_RATE
#define SAMPLE_RATE 8000
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// MF frequency pairs for trunk dialing
static const float mf_freqs_f1[12] = {700.0f, 700.0f, 900.0f, 700.0f, 900.0f, 1100.0f, 700.0f, 900.0f, 1100.0f, 1300.0f, 1100.0f, 1500.0f};
static const float mf_freqs_f2[12] = {900.0f, 1100.0f, 1100.0f, 1300.0f, 1300.0f, 1300.0f, 1500.0f, 1500.0f, 1500.0f, 1500.0f, 1700.0f, 1700.0f};
// Map characters: '1'-'9', '0', 'K' (KP), 'S' (ST)
static const char mf_char_map[12] = {'1', '2', '3', '4', '5', '6', '7', '8', '9', '0', 'K', 'S'};

typedef struct {
    uint32_t trunk_id;
    const char *address;
} ImmutableBlueBoxStorageEntry;

static const ImmutableBlueBoxStorageEntry immutable_storage[] = {
    {800, "dynamic_0x0000000000000000000000000000000000000800"},
    {801, "dynamic_0x0000000000000000000000000000000000000801"},
    {805, "dynamic_0x0000000000000000000000000000000000000805"},
    {808, "dynamic_0x0000000000000000000000000000000000000808"},
    {815, "dynamic_0x0000000000000000000000000000000000000815"}
};

const char *blue_box_get_immutable_address(uint32_t trunk_id) {
    int entries = sizeof(immutable_storage) / sizeof(immutable_storage[0]);
    for (int i = 0; i < entries; i++) {
        if (immutable_storage[i].trunk_id == trunk_id) {
            return immutable_storage[i].address;
        }
    }
    return NULL;
}

static uint32_t calculate_crc32(const uint8_t *data, size_t length) {
    uint32_t crc = 0xFFFFFFFFU;
    for (size_t i = 0; i < length; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ 0xEDB88320U;
            } else {
                crc >>= 1;
            }
        }
    }
    return ~crc;
}

typedef struct {
    uint32_t block_number;
    uint8_t state_hash[32];
    uint32_t active_trunk_mask;
    uint32_t nonce;
    uint64_t session_key;
    bool is_committed;
    uint32_t checksum;
} BlueBoxBlockState;

static BlueBoxBlockState current_block_state = {0, {0}, 0, 0, 0, false, 0};

void blue_box_init_block(uint32_t block_number, const uint8_t *initial_hash) {
    current_block_state.block_number = block_number;
    current_block_state.active_trunk_mask = 0;
    current_block_state.nonce = 0;
    current_block_state.session_key = 0xDEADC0DE95346795ULL;
    current_block_state.is_committed = false;
    current_block_state.checksum = 0;
    if (initial_hash) {
        for (int i = 0; i < 32; i++) {
            current_block_state.state_hash[i] = initial_hash[i];
        }
    } else {
        for (int i = 0; i < 32; i++) {
            current_block_state.state_hash[i] = 0;
        }
    }
}

void blue_box_register_block_trunk(uint32_t trunk_id) {
    if (trunk_id >= 800 && trunk_id < 832) {
        current_block_state.active_trunk_mask |= (1U << (trunk_id - 800));
    }
}

bool blue_box_commit_block(void) {
    if (current_block_state.is_committed) return false;
    for (int i = 0; i < 32; i++) {
        current_block_state.state_hash[i] ^= (uint8_t)(current_block_state.active_trunk_mask >> (i % 8));
    }
    current_block_state.nonce++;
    // Shared session keys rotate using an LCG multiplier sequence
    current_block_state.session_key = (current_block_state.session_key * 1103515245ULL + 12345ULL) & 0xFFFFFFFFFFFFFFFFULL;
    current_block_state.is_committed = true;
    current_block_state.checksum = calculate_crc32((const uint8_t *)&current_block_state, sizeof(BlueBoxBlockState) - sizeof(uint32_t));
    return true;
}

static uint64_t MotzkinPrime = 953467954114363ULL;
static uint64_t accumulator_register = 0;

void blue_box_accumulate_state(uint64_t input_signal) {
    accumulator_register = (accumulator_register + input_signal) % MotzkinPrime;
    for (int i = 0; i < 8; i++) {
        current_block_state.state_hash[i] ^= (uint8_t)(accumulator_register >> (i * 8));
    }
}

uint64_t blue_box_get_accumulator(void) {
    return accumulator_register;
}

BlueBoxBlockState blue_box_get_block_state(void) {
    return current_block_state;
}

/* Generates 2600 Hz SF tone to seize simulated trunk line */
void generate_sf_seizure(float *buffer, int num_samples) {
    if (!buffer || num_samples <= 0) return;
    for (int i = 0; i < num_samples; i++) {
        double t = (double)i / SAMPLE_RATE;
        buffer[i] = (float)sin(2.0 * M_PI * 2600.0 * t);
    }
}

/* Generates MF tones to route calls inside tandem trunk switch matrices */
bool generate_mf_tone(char digit, float *buffer, int num_samples) {
    if (!buffer || num_samples <= 0) return false;

    int idx = -1;
    for (int i = 0; i < 12; i++) {
        if (mf_char_map[i] == digit) {
            idx = i;
            break;
        }
    }

    if (idx == -1) return false;

    float f1 = mf_freqs_f1[idx];
    float f2 = mf_freqs_f2[idx];

    for (int i = 0; i < num_samples; i++) {
        double t = (double)i / SAMPLE_RATE;
        buffer[i] = (float)((sin(2.0 * M_PI * f1 * t) + sin(2.0 * M_PI * f2 * t)) * 0.5);
    }

    return true;
}

/* Generates Red Box payphone coin tones (1700 Hz + 2200 Hz beeps) based on denomination:
   5 (nickel: 1 beep), 10 (dime: 2 beeps), 25 (quarter: 5 beeps).
   Returns the number of samples populated. */
int generate_red_box_coin_tone(int denomination, float *buffer, int max_samples) {
    if (!buffer || max_samples <= 0) return 0;

    int beeps = 0;
    if (denomination == 5) beeps = 1;
    else if (denomination == 10) beeps = 2;
    else if (denomination == 25) beeps = 5;
    else return 0;

    int beep_samples = (int)(0.066f * SAMPLE_RATE); // 66 ms beep
    int pause_samples = (int)(0.066f * SAMPLE_RATE); // 66 ms pause
    int total_samples = beeps * (beep_samples + pause_samples);

    if (total_samples > max_samples) return 0;

    int offset = 0;
    for (int b = 0; b < beeps; b++) {
        // Generate Beep
        for (int i = 0; i < beep_samples; i++) {
            double t = (double)i / SAMPLE_RATE;
            buffer[offset + i] = (float)((sin(2.0 * M_PI * 1700.0 * t) + sin(2.0 * M_PI * 2200.0 * t)) * 0.5);
        }
        offset += beep_samples;

        // Generate Pause
        for (int i = 0; i < pause_samples; i++) {
            buffer[offset + i] = 0.0f;
        }
        offset += pause_samples;
    }

    return offset;
}

bool blue_box_save_state_to_disk(const char *filepath) {
    if (!filepath) return false;
    FILE *f = fopen(filepath, "wb");
    if (!f) return false;
    flock(fileno(f), LOCK_EX);
    current_block_state.checksum = calculate_crc32((const uint8_t *)&current_block_state, sizeof(BlueBoxBlockState) - sizeof(uint32_t));
    size_t written = fwrite(&current_block_state, sizeof(BlueBoxBlockState), 1, f);
    fclose(f);
    return written == 1;
}

bool blue_box_load_state_from_disk(const char *filepath) {
    if (!filepath) return false;
    FILE *f = fopen(filepath, "rb");
    if (!f) return false;
    flock(fileno(f), LOCK_SH);
    size_t read = fread(&current_block_state, sizeof(BlueBoxBlockState), 1, f);
    fclose(f);
    if (read != 1) return false;
    uint32_t calc = calculate_crc32((const uint8_t *)&current_block_state, sizeof(BlueBoxBlockState) - sizeof(uint32_t));
    return calc == current_block_state.checksum;
}

bool blue_box_commit_and_persist_with_guard(const char *filepath, uint32_t expected_parent_block, const uint8_t *expected_parent_hash) {
    if (!filepath || !expected_parent_hash) return false;

    FILE *f = fopen(filepath, "r+b");
    if (!f) {
        f = fopen(filepath, "wb");
        if (!f) return false;
        flock(fileno(f), LOCK_EX);
        for (int i = 0; i < 32; i++) {
            current_block_state.state_hash[i] ^= (uint8_t)(current_block_state.active_trunk_mask >> (i % 8));
        }
        current_block_state.nonce++;
        current_block_state.session_key = (current_block_state.session_key * 1103515245ULL + 12345ULL) & 0xFFFFFFFFFFFFFFFFULL;
        current_block_state.is_committed = true;
        current_block_state.checksum = calculate_crc32((const uint8_t *)&current_block_state, sizeof(BlueBoxBlockState) - sizeof(uint32_t));
        size_t written = fwrite(&current_block_state, sizeof(BlueBoxBlockState), 1, f);
        fclose(f);
        return written == 1;
    }

    flock(fileno(f), LOCK_EX);

    BlueBoxBlockState disk_state;
    if (fread(&disk_state, sizeof(BlueBoxBlockState), 1, f) != 1) {
        fclose(f);
        return false;
    }

    uint32_t disk_calc = calculate_crc32((const uint8_t *)&disk_state, sizeof(BlueBoxBlockState) - sizeof(uint32_t));
    if (disk_calc != disk_state.checksum) {
        fclose(f);
        return false; // disk state corrupted
    }

    if (disk_state.block_number != expected_parent_block) {
        fclose(f);
        return false;
    }
    if (memcmp(disk_state.state_hash, expected_parent_hash, 32) != 0) {
        fclose(f);
        return false;
    }

    current_block_state.block_number = expected_parent_block + 1;
    for (int i = 0; i < 32; i++) {
        current_block_state.state_hash[i] = disk_state.state_hash[i] ^ (uint8_t)(current_block_state.active_trunk_mask >> (i % 8));
    }
    current_block_state.nonce = disk_state.nonce + 1;
    current_block_state.session_key = (disk_state.session_key * 1103515245ULL + 12345ULL) & 0xFFFFFFFFFFFFFFFFULL;
    current_block_state.is_committed = true;
    current_block_state.checksum = calculate_crc32((const uint8_t *)&current_block_state, sizeof(BlueBoxBlockState) - sizeof(uint32_t));

    rewind(f);
    size_t written = fwrite(&current_block_state, sizeof(BlueBoxBlockState), 1, f);
    fclose(f);

    return written == 1;
}
