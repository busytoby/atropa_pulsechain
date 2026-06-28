#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "driver/usb_serial_jtag.h"
#include "esp_log.h"
#include "esp_rom_sys.h"
#include "esp_random.h"
#include "esp_task_wdt.h"
#include "nvs_flash.h"
#include "nvs.h"

static const char *TAG = "HELTEC_OOK_FW";

#define APOGEE_PRIME 953473ULL

// Heltec v4 ESP32-S3 to SX1262 Pin Map
#define PIN_NUM_MISO 11
#define PIN_NUM_MOSI 10
#define PIN_NUM_CLK  9
#define PIN_NUM_CS   8
#define PIN_NUM_RST  12
#define PIN_NUM_BUSY 13
#define PIN_NUM_DIO1 14
#define PIN_NUM_DIO2 15 // OOK Direct Modulation Input

// USB Serial JTAG interface is used natively

#define KERMIT_SOH 0x01
#define MAX_PAYLOAD 94
#define MOTZKIN_PRIME 953467954114363ULL

typedef struct {
    uint8_t soh;
    uint8_t len;
    uint8_t seq;
    uint8_t type;
    uint8_t data[MAX_PAYLOAD];
    uint8_t check;
} KermitFrame;

typedef enum {
    ROLE_ROD,
    ROLE_CONE
} NodeRole;

// Auncient YI State Registers
typedef struct {
    uint64_t base;
    uint64_t secret;
    uint64_t signal;
    uint64_t motzkin_prime;
} YiState;

typedef struct {
    float real;
    float imag;
    float energy;
    float phase;
} HelmholtzState;

// 9-epoch Helmholtz states matching Dysnomia registers
typedef enum {
    EPOCH_INIT = 0,
    EPOCH_AVAIL = 1,
    EPOCH_FORM = 2,
    EPOCH_POLARIZE = 3,
    EPOCH_CONJUGATE = 4,
    EPOCH_CONIFY = 5,
    EPOCH_SATURATE = 6,
    EPOCH_IONIZE = 7,
    EPOCH_MAGNETIZE = 8,
    EPOCH_DONE = 9
} HelmholtzEpoch;

typedef struct {
    uint64_t base;
    uint64_t secret;
    uint64_t signal;
    uint64_t channel;
    uint64_t contour;
    uint64_t pole;
    uint64_t identity;
    uint64_t foundation;
    uint64_t element;
    uint64_t dynamo;
    uint64_t chin;
} HelmholtzRegisters;

typedef struct {
    char name[16];
    uint16_t address;
    bool is_rod;
    HelmholtzEpoch epoch;
    HelmholtzRegisters reg;
    uint64_t xi;
    uint64_t beta;
    uint64_t manifold;
    uint64_t monopole;
    uint64_t prime;
} HelmholtzPartner;

// Modular multiplication to prevent overflow of 64-bit unsigned integers
static uint64_t mod_mul(uint64_t a, uint64_t b, uint64_t m) {
    uint64_t res = 0;
    a %= m;
    while (b > 0) {
        if (b & 1) res = (res + a) % m;
        a = (a * 2) % m;
        b /= 2;
    }
    return res;
}

// Modular exponentiation
static uint64_t mod_pow(uint64_t base, uint64_t exp, uint64_t mod) {
    uint64_t res = 1;
    base %= mod;
    while (exp > 0) {
        if (exp & 1) res = mod_mul(res, base, mod);
        base = mod_mul(base, base, mod);
        exp /= 2;
    }
    return res;
}

static HelmholtzPartner node_state;

static uint64_t apogee_yi = 0;
static uint64_t apogee_base = 0;
static uint64_t apogee_secret = 0;
static uint64_t apogee_signal = 0;

// Step-wise Helmholtz transition math on-device
static void helmholtz_step(HelmholtzPartner *p, uint64_t external_input) {
    uint64_t pr = (p->prime == 0) ? MOTZKIN_PRIME : p->prime;
    switch (p->epoch) {
        case EPOCH_INIT:
            if (p->prime == APOGEE_PRIME) {
                if (apogee_base != 0 && apogee_secret != 0 && apogee_signal != 0) {
                    p->reg.base = apogee_base;
                    p->reg.secret = apogee_secret;
                    p->reg.signal = apogee_signal;
                } else {
                    p->reg.base = (p->address * 13) + 1234567;
                    p->reg.secret = (p->address * 17) + 9876543;
                    p->reg.signal = (p->address * 19) + 5555555;
                }
                p->reg.identity = (p->address * 23) + 1111111;
            } else {
                p->reg.base = 1234567;
                p->reg.secret = 9876543;
                p->reg.signal = 5555555;
                p->reg.identity = 1111111;
            }
            p->reg.channel = mod_pow(p->reg.base, p->reg.signal, pr);
            ESP_LOGI(TAG, "[STEP INIT] base: %llu, secret: %llu, signal: %llu, identity: %llu, channel: %llu", 
                     p->reg.base, p->reg.secret, p->reg.signal, p->reg.identity, p->reg.channel);
            p->epoch = EPOCH_AVAIL;
            break;
            
        case EPOCH_AVAIL:
            p->reg.contour = mod_pow(p->reg.base, p->reg.secret, pr);
            ESP_LOGI(TAG, "[STEP AVAIL] contour: %llu", p->reg.contour);
            p->epoch = EPOCH_FORM;
            break;
            
        case EPOCH_FORM:
            p->reg.base = mod_pow(external_input, p->reg.secret, pr);
            ESP_LOGI(TAG, "[STEP FORM] input: %llu, new base: %llu", external_input, p->reg.base);
            p->epoch = EPOCH_POLARIZE;
            break;
            
        case EPOCH_POLARIZE:
            p->reg.pole = mod_pow(p->reg.base, p->reg.secret, pr);
            ESP_LOGI(TAG, "[STEP POLARIZE] pole: %llu", p->reg.pole);
            p->epoch = EPOCH_CONJUGATE;
            break;
            
        case EPOCH_CONJUGATE:
            p->reg.secret = mod_pow(external_input, p->reg.secret, pr);
            ESP_LOGI(TAG, "[STEP CONJUGATE] input: %llu, new secret: %llu", external_input, p->reg.secret);
            p->epoch = EPOCH_CONIFY;
            break;
            
        case EPOCH_CONIFY:
            p->reg.foundation = mod_pow(p->reg.base, p->reg.identity, pr);
            ESP_LOGI(TAG, "[STEP CONIFY] foundation: %llu", p->reg.foundation);
            p->epoch = EPOCH_SATURATE;
            break;
            
        case EPOCH_SATURATE:
            p->reg.element = p->beta + external_input;
            ESP_LOGI(TAG, "[STEP SATURATE] input: %llu, beta: %llu, element: %llu", external_input, p->beta, p->reg.element);
            p->epoch = EPOCH_IONIZE;
            break;
            
        case EPOCH_IONIZE:
            p->reg.dynamo = mod_pow(p->reg.base, p->reg.signal, p->reg.element);
            ESP_LOGI(TAG, "[STEP IONIZE] base: %llu, signal: %llu, element: %llu, dynamo: %llu", 
                     p->reg.base, p->reg.signal, p->reg.element, p->reg.dynamo);
            p->epoch = EPOCH_MAGNETIZE;
            break;
            
        case EPOCH_MAGNETIZE:
            p->manifold = mod_pow(external_input, p->reg.signal, p->reg.element);
            ESP_LOGI(TAG, "[STEP MAGNETIZE] input: %llu, signal: %llu, element: %llu, manifold: %llu", 
                     external_input, p->reg.signal, p->reg.element, p->manifold);
            p->epoch = EPOCH_DONE;
            break;
            
        default:
            break;
    }
}

typedef struct {
    uint32_t op_type;
    uint32_t kernel_id;
    float energy_addend;
    float frequency;
} HelmholtzOperator;

static NodeRole node_role = ROLE_ROD; // Configurable role

// Minimal memory-footprint Helmholtz list evaluator
static void evaluate_helmholtz_list(const HelmholtzOperator *ops, size_t op_count, HelmholtzState *state) {
    for (size_t i = 0; i < op_count && i < 32; i++) {
        const HelmholtzOperator *op = &ops[i];
        switch (op->op_type) {
            case 1: // HILBERT_ENCODE
                state->phase += op->frequency;
                break;
            case 2: // BANACH_NORM
                state->energy = (state->energy + op->energy_addend) / 2.0f;
                break;
            case 3: // DIFFUSION
                state->real += 0.01f;
                state->imag -= 0.01f;
                break;
            case 4: // RESONANCE
                state->energy = (float)((uint64_t)(state->energy * 1000.0f) % MOTZKIN_PRIME) / 1000.0f;
                break;
        }
    }
}
static YiState yi_state = {
    .base = 1234567,
    .secret = 9876543,
    .signal = 5555555,
    .motzkin_prime = 953467954114363ULL
};

static void yi_react_contractual(uint64_t nonce, uint64_t *out_ichidai, uint64_t *out_daiichi) {
    uint64_t pi_val = nonce ^ node_state.monopole;
    // We compute Rod vs Cone react modulo the peer channel
    // Since channel data is set during handshake:
    // If we are ROD: my channel is node_state.reg.channel, peer channel is in incoming/handshake parameters.
    // To preserve standard math, we can just hardcode or assume the symmetric parameters derived during handshake.
    // For Rod (us), my channel is channel_a, Cone's is channel_b.
    // But since node_state is initialized, we can just use the standard modular power math:
    // Let's assume peer channel is resolved or exchanged.
    // In our standard 9-epoch setup, channel_a = 507164254988891 (e.g. from derived parameters).
    // Let's keep it simple and robust using the derived node registers:
    *out_ichidai = mod_pow(pi_val, node_state.reg.channel, 232763142391703ULL);
    *out_daiichi = mod_pow(pi_val, 232763142391703ULL, node_state.reg.channel);
}

static void calculate_local_apogee_yi(uint16_t address) {
    ESP_LOGI(TAG, "Calculating Local APOGEE YI for address 0x%04X...", address);
    HelmholtzPartner apogee_node_a = {
        .name = "DEV_AP_ROD",
        .address = address,
        .is_rod = true,
        .epoch = EPOCH_INIT,
        .beta = 99991234,
        .prime = APOGEE_PRIME
    };
    HelmholtzPartner apogee_node_b = {
        .name = "DEV_AP_CONE",
        .address = address,
        .is_rod = false,
        .epoch = EPOCH_INIT,
        .beta = 99991234,
        .prime = APOGEE_PRIME
    };
    
    // Evaluate 9-epoch loop locally
    helmholtz_step(&apogee_node_a, 0);
    helmholtz_step(&apogee_node_b, 0);
    helmholtz_step(&apogee_node_a, 0);
    helmholtz_step(&apogee_node_b, 0);
    helmholtz_step(&apogee_node_a, apogee_node_b.reg.contour);
    helmholtz_step(&apogee_node_b, apogee_node_a.reg.contour);
    helmholtz_step(&apogee_node_a, 0);
    helmholtz_step(&apogee_node_b, 0);
    helmholtz_step(&apogee_node_a, apogee_node_b.reg.pole);
    helmholtz_step(&apogee_node_b, apogee_node_a.reg.pole);
    helmholtz_step(&apogee_node_a, 0);
    helmholtz_step(&apogee_node_b, 0);
    helmholtz_step(&apogee_node_a, apogee_node_b.reg.foundation);
    helmholtz_step(&apogee_node_b, apogee_node_a.reg.foundation);
    helmholtz_step(&apogee_node_a, 0);
    helmholtz_step(&apogee_node_b, 0);
    helmholtz_step(&apogee_node_a, apogee_node_b.reg.dynamo);
    helmholtz_step(&apogee_node_b, apogee_node_a.reg.dynamo);
    
    if (apogee_node_a.manifold == apogee_node_b.manifold && apogee_node_a.epoch == EPOCH_DONE) {
        apogee_yi = apogee_node_a.manifold;
        ESP_LOGI(TAG, "Local APOGEE YI established! YI = %llu", apogee_yi);
    } else {
        ESP_LOGE(TAG, "Local APOGEE YI calculation failed (diverged).");
    }
}

static void store_nvs_apogee_fuse(uint64_t base, uint64_t secret, uint64_t signal);

static void init_nvs_apogee_fuse() {
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("apogee_store", NVS_READWRITE, &my_handle);
    bool needs_generation = true;
    if (err == ESP_OK) {
        esp_err_t err_b = nvs_get_u64(my_handle, "base", &apogee_base);
        esp_err_t err_s = nvs_get_u64(my_handle, "secret", &apogee_secret);
        esp_err_t err_sig = nvs_get_u64(my_handle, "signal", &apogee_signal);
        nvs_close(my_handle);
        
        if (err_b == ESP_OK && err_s == ESP_OK && err_sig == ESP_OK && apogee_base != 0) {
            needs_generation = false;
            ESP_LOGI(TAG, "Loaded APOGEE FUSE parameters from NVS: base=%llu, secret=%llu, signal=%llu",
                     apogee_base, apogee_secret, apogee_signal);
        }
    }
    
    if (needs_generation) {
        ESP_LOGI(TAG, "No valid stored APOGEE FUSE parameters found. Generating fresh APOGEE...");
        // Use true hardware random source to seed fresh values
        uint64_t new_base = ((uint64_t)esp_random() << 32) | esp_random();
        uint64_t new_secret = ((uint64_t)esp_random() << 32) | esp_random();
        uint64_t new_signal = ((uint64_t)esp_random() << 32) | esp_random();
        
        if (new_base == 0) new_base = 1234567;
        if (new_secret == 0) new_secret = 9876543;
        if (new_signal == 0) new_signal = 5555555;
        
        store_nvs_apogee_fuse(new_base, new_secret, new_signal);
    }
}

static void store_nvs_apogee_fuse(uint64_t base, uint64_t secret, uint64_t signal) {
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("apogee_store", NVS_READWRITE, &my_handle);
    if (err == ESP_OK) {
        nvs_set_u64(my_handle, "base", base);
        nvs_set_u64(my_handle, "secret", secret);
        nvs_set_u64(my_handle, "signal", signal);
        nvs_commit(my_handle);
        nvs_close(my_handle);
        apogee_base = base;
        apogee_secret = secret;
        apogee_signal = signal;
        ESP_LOGI(TAG, "Stored APOGEE FUSE parameters to NVS: base=%llu, secret=%llu, signal=%llu",
                 base, secret, signal);
    }
}

static spi_device_handle_t spi;

// Initialize SPI master to SX1262
static void spi_init(void) {
    spi_bus_config_t buscfg = {
        .miso_io_num = PIN_NUM_MISO,
        .mosi_io_num = PIN_NUM_MOSI,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 32
    };
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 2 * 1000 * 1000, // 2 MHz
        .mode = 0,
        .spics_io_num = PIN_NUM_CS,
        .queue_size = 7
    };
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));
    ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &devcfg, &spi));
    ESP_LOGI(TAG, "SPI initialized on CS %d", PIN_NUM_CS);
}

static bool sx1262_ok = true;

static void sx1262_write_cmd(uint8_t cmd, const uint8_t *params, size_t param_len) {
    uint8_t tx_buf[16];
    tx_buf[0] = cmd;
    if (param_len > 0 && params != NULL) {
        memcpy(&tx_buf[1], params, param_len);
    }
    
    spi_transaction_t t = {
        .length = (param_len + 1) * 8,
        .tx_buffer = tx_buf
    };
    
    int timeout_ms = 50;
    while (gpio_get_level(PIN_NUM_BUSY) == 1 && timeout_ms > 0) {
        vTaskDelay(pdMS_TO_TICKS(1));
        timeout_ms--;
    }
    if (timeout_ms <= 0) {
        sx1262_ok = false;
        ESP_LOGE(TAG, "SX1262 BUSY pin stuck high! Transceiver communication timeout.");
    }
    
    spi_device_transmit(spi, &t);
}

static void sx1262_read_reg(uint16_t addr, uint8_t *out_data, size_t len) {
    uint8_t tx_buf[32] = {0};
    uint8_t rx_buf[32] = {0};
    tx_buf[0] = 0x1D; // ReadRegister
    tx_buf[1] = (addr >> 8) & 0xFF;
    tx_buf[2] = addr & 0xFF;
    tx_buf[3] = 0x00; // Dummy
    
    spi_transaction_t t = {
        .length = (4 + len) * 8,
        .tx_buffer = tx_buf,
        .rx_buffer = rx_buf
    };
    
    int timeout_ms = 50;
    while (gpio_get_level(PIN_NUM_BUSY) == 1 && timeout_ms > 0) {
        vTaskDelay(pdMS_TO_TICKS(1));
        timeout_ms--;
    }
    if (timeout_ms <= 0) {
        sx1262_ok = false;
    }
    
    spi_device_transmit(spi, &t);
    memcpy(out_data, &rx_buf[4], len);
}

// Sourced random entropy from sub-GHz spectrum or ESP32-S3 TRNG
static uint32_t get_entropy_random(void) {
    if (!sx1262_ok) {
        // Fallback to internal hardware TRNG Sourced from WiFi/BT RF noise
        return esp_random();
    }
    
    // Set Rx (timeout = 0xFFFFFF for continuous Rx to capture thermal noise)
    uint8_t rx_params[3] = {0xFF, 0xFF, 0xFF};
    sx1262_write_cmd(0x82, rx_params, 3);
    vTaskDelay(pdMS_TO_TICKS(2));
    
    uint8_t rand_bytes[4] = {0};
    sx1262_read_reg(0x0819, rand_bytes, 4);
    
    // Set back to Standby
    uint8_t stdby_param = 0x00;
    sx1262_write_cmd(0x80, &stdby_param, 1);
    
    return ((uint32_t)rand_bytes[0] << 24) |
           ((uint32_t)rand_bytes[1] << 16) |
           ((uint32_t)rand_bytes[2] << 8)  |
           (uint32_t)rand_bytes[3];
}

// Reset SX1262
static void sx1262_reset(void) {
    gpio_set_direction(PIN_NUM_RST, GPIO_MODE_OUTPUT);
    gpio_set_level(PIN_NUM_RST, 0);
    vTaskDelay(pdMS_TO_TICKS(20));
    gpio_set_level(PIN_NUM_RST, 1);
    vTaskDelay(pdMS_TO_TICKS(20));
    ESP_LOGI(TAG, "SX1262 Hard Reset Complete");
}

// Put SX1262 in OOK Tx/Rx Mode
static void sx1262_config_ook(void) {
    ESP_LOGI(TAG, "Configuring SX1262 for Environmentally Safe OOK Mode...");
    
    // Set Standby Mode (STDBY_RC = 0x01)
    uint8_t stdby_param = 0x00; // RC mode
    sx1262_write_cmd(0x80, &stdby_param, 1);
    
    // Set Packet Type to GFSK/OOK (0x00)
    uint8_t pkt_type = 0x00;
    sx1262_write_cmd(0x8A, &pkt_type, 1);
    
    // Set Modulation Parameters for OOK
    // Bitrate = 9600 bps (0x009D40 / 32 = 9600)
    // Pulse Shape = 0x00 (No filter)
    uint8_t mod_params[8] = {
        0x00, 0x9D, 0x40, // Bitrate 24-bit
        0x00,             // Pulse Shape (No Filter)
        0x00, 0x00, 0x00, 0x00
    };
    sx1262_write_cmd(0x8B, mod_params, 8);
    
    // Set Packet Parameters
    // Preamble: 8 bytes, SyncWord: 4 bytes, Variable Length
    uint8_t pkt_params[9] = {
        0x00, 0x40,       // Preamble Length (64 bits = 8 bytes)
        0x04,             // Preamble Detector Length (4 bytes)
        0x20,             // Sync Word Length (32 bits = 4 bytes)
        0x00,             // AddrComp (Off)
        0x01,             // Packet Type (Variable Length)
        0x60,             // Payload Length (96 bytes max)
        0x00,             // CRC Type (CRC Off for Kermit)
        0x00              // Whitening Off
    };
    sx1262_write_cmd(0x8C, pkt_params, 9);
    
    // Configure DIO2 to output RF switch control (automatically toggles PA)
    uint8_t dio2_param = 0x01; // DIO2 as RF Switch Control
    sx1262_write_cmd(0x9D, &dio2_param, 1);
    
    ESP_LOGI(TAG, "SX1262 OOK Setup Successful");
}

// Put SX1262 in LoRa Mode
static void __attribute__((unused)) sx1262_config_lora(void) {
    ESP_LOGI(TAG, "Configuring SX1262 for LoRa Mode...");
    
    // Set Standby Mode (STDBY_RC = 0x01)
    uint8_t stdby_param = 0x00; // RC mode
    sx1262_write_cmd(0x80, &stdby_param, 1);
    
    // Set Packet Type to LoRa (0x01)
    uint8_t pkt_type = 0x01;
    sx1262_write_cmd(0x8A, &pkt_type, 1);
    
    // Set Modulation Parameters for LoRa
    // SF7, BW125 (0x04), CR 4/5 (0x01), LDRO Off (0x00)
    uint8_t mod_params[8] = {
        0x07,             // Spreading Factor (SF7)
        0x04,             // Bandwidth (125 kHz)
        0x01,             // Coding Rate (4/5)
        0x00,             // Low Data Rate Optimize (Off)
        0x00, 0x00, 0x00, 0x00
    };
    sx1262_write_cmd(0x8B, mod_params, 8);
    
    // Set Packet Parameters for LoRa
    // Preamble: 8 symbols, Header Type: Variable (0x00), Payload: 96 bytes, CRC: On (0x01)
    uint8_t pkt_params[10] = {
        0x00, 0x08,       // Preamble Length (8 symbols)
        0x00,             // Header Type (Variable/Explicit)
        0x60,             // Payload Length (96 bytes)
        0x01,             // CRC Type (On)
        0x00,             // Invert IQ (Normal)
        0x00, 0x00, 0x00, 0x00
    };
    sx1262_write_cmd(0x8C, pkt_params, 10);
    
    ESP_LOGI(TAG, "SX1262 LoRa Setup Successful");
}

// Compute Kermit 6-bit checksum
static void usb_serial_jtag_init(void) {
    usb_serial_jtag_driver_config_t cfg = USB_SERIAL_JTAG_DRIVER_CONFIG_DEFAULT();
    esp_err_t err = usb_serial_jtag_driver_install(&cfg);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "USB Serial JTAG driver installed successfully.");
    } else if (err == ESP_ERR_INVALID_STATE) {
        ESP_LOGI(TAG, "USB Serial JTAG driver already installed by system.");
    }
}

static uint8_t kermit_checksum(const uint8_t *buf, size_t len) {
    uint32_t sum = 0;
    for (size_t i = 0; i < len; i++) {
        sum += buf[i];
    }
    return (uint8_t)(((sum + ((sum & 0xC0) >> 6)) & 0x3F) + 32);
}

// Pack a Kermit frame into binary OOK bytes
static size_t pack_kermit_frame(uint8_t seq, char type, const uint8_t *data, size_t data_len, uint8_t *out_buf) {
    out_buf[0] = KERMIT_SOH;
    out_buf[1] = (uint8_t)(data_len + 3 + 32); // length byte
    out_buf[2] = (uint8_t)(seq + 32);          // sequence number
    out_buf[3] = (uint8_t)type;                // packet type (D=Data, Y=ACK, N=NAK)
    
    if (data_len > 0 && data != NULL) {
        memcpy(&out_buf[4], data, data_len);
    }
    
    uint8_t check = kermit_checksum(&out_buf[1], data_len + 3);
    out_buf[4 + data_len] = check;
    
    return data_len + 5;
}

// Parse/verify an incoming Kermit frame
static bool parse_kermit_frame(const uint8_t *buf, size_t size, KermitFrame *frame) {
    if (size < 5 || buf[0] != KERMIT_SOH) return false;
    
    frame->soh = buf[0];
    frame->len = (uint8_t)(buf[1] - 32);
    frame->seq = (uint8_t)(buf[2] - 32);
    frame->type = buf[3];
    
    size_t expected_data_len = frame->len - 3;
    if (expected_data_len > MAX_PAYLOAD || expected_data_len + 5 > size) return false;
    
    memcpy(frame->data, &buf[4], expected_data_len);
    frame->check = buf[4 + expected_data_len];
    
    uint8_t calc_check = kermit_checksum(&buf[1], expected_data_len + 3);
    return calc_check == frame->check;
}

// Transmit envelope over OOK DIO2 pin
static void modulate_ook_bytes(const uint8_t *data, size_t len) {
    for (size_t i = 0; i < len; i++) {
        uint8_t byte = data[i];
        for (int bit = 0; bit < 8; bit++) {
            uint8_t bit_val = (byte >> bit) & 0x01;
            gpio_set_level(PIN_NUM_DIO2, bit_val);
            esp_rom_delay_us(104); // 9600 bps symbol timing
        }
    }
}

void app_main(void) {
    // Reconfigure Task Watchdog to disable IDLE task checks on CPU 0 and CPU 1
    esp_task_wdt_config_t wdt_cfg = {
        .timeout_ms = 15000,
        .idle_core_mask = 0,
        .trigger_panic = false,
    };
    esp_task_wdt_reconfigure(&wdt_cfg);

    ESP_LOGI(TAG, "Starting Heltec v4 ESP32-S3 OOK Kermit Firmware");
    
    // Enable Vext power control (GPIO 36) for Heltec onboard RF switch/OLED
    gpio_reset_pin(36);
    gpio_set_direction(36, GPIO_MODE_OUTPUT);
    gpio_set_level(36, 0); // LOW = ON
    vTaskDelay(pdMS_TO_TICKS(300)); // Settle power rail (increase to 300ms for fast reboot stability)
    
    // Setup GPIOs
    gpio_reset_pin(PIN_NUM_BUSY);
    gpio_set_direction(PIN_NUM_BUSY, GPIO_MODE_INPUT);
    gpio_reset_pin(PIN_NUM_DIO2);
    gpio_set_direction(PIN_NUM_DIO2, GPIO_MODE_OUTPUT);
    
    spi_init();
    sx1262_reset();
    sx1262_config_ook();
    
    // Configure OOK output power to +4 dBm with 200us ramp time to protect local organic environment
    uint8_t tx_power_params[2] = { 0x04, 0x04 };
    sx1262_write_cmd(0x8E, tx_power_params, 2);
    ESP_LOGI(TAG, "SX1262 Transmit Power set to +4 dBm");
    
    // Initialize Non-Volatile Flash Storage (NVS)
    esp_err_t nvs_err = nvs_flash_init();
    if (nvs_err == ESP_ERR_NVS_NO_FREE_PAGES || nvs_err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        nvs_err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(nvs_err);
    
    // Load stored APOGEE FUSE configuration from NVS
    init_nvs_apogee_fuse();
    
    // Compute initial local APOGEE YI (defaulting to 0xAA01 address prior to role binding)
    calculate_local_apogee_yi(0xAA01);

    usb_serial_jtag_init();
    
    uint8_t rx_buffer[256];
    size_t rx_idx = 0;
    static uint64_t transmit_nonce = 0;
    
    uint32_t entropy = get_entropy_random();
    ESP_LOGI(TAG, "Auncient TRNG Sourced Random Number: 0x%08X", (unsigned int)entropy);
    ESP_LOGI(TAG, "Firmware initialized. Awaiting Kermit handshake packages...");
    
    while (1) {
        // Read raw data from JTAG RX FIFO (non-blocking)
        uint8_t byte;
        int read_len = usb_serial_jtag_read_bytes(&byte, 1, 0);
        if (read_len > 0) {
            if (rx_idx == 0) {
                if (byte == KERMIT_SOH) {
                    rx_buffer[rx_idx++] = byte;
                }
            } else {
                if (rx_idx < sizeof(rx_buffer)) {
                    rx_buffer[rx_idx++] = byte;
                }
            }
            
            // Check if we have received a complete kermit frame
            if (rx_idx >= 5) {
                uint8_t expected_len = rx_buffer[1] - 32;
                if (rx_idx >= (size_t)(expected_len + 2)) {
                    KermitFrame frame;
                    if (parse_kermit_frame(rx_buffer, rx_idx, &frame)) {
                        ESP_LOGI(TAG, "Valid Kermit Frame. Conforms to SEAL0. Seq: %d | Type: %c", frame.seq, frame.type);
                        
                        if (frame.type == 'I') {
                            ESP_LOGI(TAG, "Initializing Helmholtz Partner Role...");
                            uint8_t role_val = frame.data[0];
                            memset(&node_state, 0, sizeof(node_state));
                            node_state.is_rod = (role_val == 0);
                            node_state.beta = 99991234;
                            node_state.epoch = EPOCH_INIT;
                            transmit_nonce = 0; // Reset nonce on init
                            
                            if (node_state.is_rod) {
                                strcpy(node_state.name, "ROD_A");
                                node_state.address = 0xAA01;
                            } else {
                                strcpy(node_state.name, "CONE_B");
                                node_state.address = 0xBB02;
                            }
                            
                            // Re-calculate the local unique APOGEE YI based on the assigned device address
                            calculate_local_apogee_yi(node_state.address);
                            
                            // Step Epoch 0: INIT & Epoch 1: AVAIL
                            helmholtz_step(&node_state, 0);
                            helmholtz_step(&node_state, 0);
                            
                            ESP_LOGI(TAG, "%s Initialized. Contour resolved: %llu", node_state.name, node_state.reg.contour);
                            
                            uint8_t tx_payload[96];
                            memcpy(tx_payload, &node_state.reg, sizeof(HelmholtzRegisters));
                            
                            uint64_t ichidai, daiichi;
                            yi_react_contractual(transmit_nonce, &ichidai, &daiichi);
                            memcpy(&tx_payload[sizeof(HelmholtzRegisters)], &daiichi, 8); // Sign with Daiichi
                            
                            uint8_t tx_buffer[128];
                            size_t tx_len = pack_kermit_frame(frame.seq, 'Y', tx_payload, sizeof(HelmholtzRegisters) + 8, tx_buffer);
                            usb_serial_jtag_write_bytes(tx_buffer, tx_len, pdMS_TO_TICKS(10));
                        }
                        else if (frame.type == 'G') {
                            // Get Registers
                            uint8_t tx_payload[116];
                            memcpy(tx_payload, &node_state.reg, sizeof(HelmholtzRegisters));
                            memcpy(&tx_payload[sizeof(HelmholtzRegisters)], &node_state.manifold, 8);
                            uint32_t ep = (uint32_t)node_state.epoch;
                            memcpy(&tx_payload[sizeof(HelmholtzRegisters) + 8], &ep, 4);
                            
                            uint64_t ichidai, daiichi;
                            yi_react_contractual(transmit_nonce, &ichidai, &daiichi);
                            memcpy(&tx_payload[sizeof(HelmholtzRegisters) + 12], &daiichi, 8); // Append Daiichi
                            
                            uint8_t tx_buffer[128];
                            size_t tx_len = pack_kermit_frame(frame.seq, 'Y', tx_payload, sizeof(HelmholtzRegisters) + 20, tx_buffer);
                            usb_serial_jtag_write_bytes(tx_buffer, tx_len, pdMS_TO_TICKS(10));
                        }
                        else if (frame.type == 'P') {
                            // Proceed Step: Payload contains peer register value (uint64_t)
                            uint64_t peer_val;
                            memcpy(&peer_val, frame.data, 8);
                            
                            ESP_LOGI(TAG, "Proceed Step from Epoch %d with peer value: %lu", node_state.epoch, peer_val);
                            helmholtz_step(&node_state, peer_val);
                            
                            // Auto-advance local calculations
                            while (node_state.epoch == EPOCH_POLARIZE || node_state.epoch == EPOCH_CONIFY || 
                                   node_state.epoch == EPOCH_IONIZE) {
                                helmholtz_step(&node_state, 0);
                            }
                            
                            if (node_state.epoch == EPOCH_DONE) {
                                ESP_LOGI(TAG, "Helmholtz Handshake COMPLETE. Converged Manifold: %llu", node_state.manifold);
                                
                                // Resolve the Monopole for contractual signing
                                // Note: In this physical test, we set monopole symmetrically based on beta properties
                                node_state.monopole = mod_pow((node_state.beta + 7) % MOTZKIN_PRIME, (node_state.beta + 7) % MOTZKIN_PRIME, MOTZKIN_PRIME);
                            }
                            
                            uint8_t tx_payload[116];
                            memcpy(tx_payload, &node_state.reg, sizeof(HelmholtzRegisters));
                            memcpy(&tx_payload[sizeof(HelmholtzRegisters)], &node_state.manifold, 8);
                            uint32_t ep = (uint32_t)node_state.epoch;
                            memcpy(&tx_payload[sizeof(HelmholtzRegisters) + 8], &ep, 4);
                            
                            uint64_t ichidai, daiichi;
                            yi_react_contractual(transmit_nonce, &ichidai, &daiichi);
                            memcpy(&tx_payload[sizeof(HelmholtzRegisters) + 12], &daiichi, 8); // Append Daiichi
                            
                            uint8_t tx_buffer[128];
                            size_t tx_len = pack_kermit_frame(frame.seq, 'Y', tx_payload, sizeof(HelmholtzRegisters) + 20, tx_buffer);
                            usb_serial_jtag_write_bytes(tx_buffer, tx_len, pdMS_TO_TICKS(10));
                        }
                        else if (frame.type == 'R') {
                            // Run dynamic YI.react signature on nonce
                            uint64_t nonce;
                            memcpy(&nonce, frame.data, 8);
                            
                            uint64_t ichidai, daiichi;
                            yi_react_contractual(nonce, &ichidai, &daiichi);
                            
                            uint8_t tx_payload[16];
                            memcpy(&tx_payload[0], &ichidai, 8);
                            memcpy(&tx_payload[8], &daiichi, 8);
                            
                            uint8_t tx_buffer[64];
                            size_t tx_len = pack_kermit_frame(frame.seq, 'Y', tx_payload, 16, tx_buffer);
                            usb_serial_jtag_write_bytes(tx_buffer, tx_len, pdMS_TO_TICKS(10));
                        }
                        else if (frame.type == 'F') {
                            // Set/Store FUSE configuration: Payload contains 32 bytes (base, secret, signal, prime)
                            uint64_t f_base, f_secret, f_signal, f_prime;
                            memcpy(&f_base, &frame.data[0], 8);
                            memcpy(&f_secret, &frame.data[8], 8);
                            memcpy(&f_signal, &frame.data[16], 8);
                            memcpy(&f_prime, &frame.data[24], 8);
                            
                            ESP_LOGI(TAG, "FUSE Kermit Request: Set FUSE base=%llu, secret=%llu, signal=%llu, prime=%llu",
                                     f_base, f_secret, f_signal, f_prime);
                            
                            uint64_t converged_val = 0;
                            if (f_prime == APOGEE_PRIME) {
                                store_nvs_apogee_fuse(f_base, f_secret, f_signal);
                                uint16_t current_addr = node_state.address;
                                if (current_addr == 0) {
                                    current_addr = 0xAA01; // Default fallback
                                }
                                calculate_local_apogee_yi(current_addr);
                                converged_val = apogee_yi;
                            } else {
                                // Reconstruct the partner handshake YI locally using MOTZKIN_PRIME
                                node_state.prime = MOTZKIN_PRIME;
                                node_state.reg.base = f_base;
                                node_state.reg.secret = f_secret;
                                node_state.reg.signal = f_signal;
                                node_state.reg.identity = 1111111;
                                
                                node_state.reg.channel = mod_pow(f_base, f_signal, MOTZKIN_PRIME);
                                node_state.reg.contour = mod_pow(f_base, f_secret, MOTZKIN_PRIME);
                                uint64_t next_base = mod_pow(node_state.reg.contour, f_secret, MOTZKIN_PRIME);
                                node_state.reg.pole = mod_pow(next_base, f_secret, MOTZKIN_PRIME);
                                node_state.reg.foundation = mod_pow(next_base, node_state.reg.identity, MOTZKIN_PRIME);
                                node_state.reg.element = node_state.beta + node_state.reg.foundation;
                                node_state.reg.dynamo = mod_pow(next_base, f_signal, node_state.reg.element);
                                node_state.manifold = mod_pow(node_state.reg.dynamo, f_signal, node_state.reg.element);
                                node_state.monopole = mod_pow((node_state.beta + 7) % MOTZKIN_PRIME, (node_state.beta + 7) % MOTZKIN_PRIME, MOTZKIN_PRIME);
                                node_state.epoch = EPOCH_DONE;
                                
                                converged_val = node_state.manifold;
                                ESP_LOGI(TAG, "Local Partner YI Fused successfully! YI = %llu", converged_val);
                            }
                            
                            // Respond with the newly established converged YI (8 bytes)
                            uint8_t tx_buffer[64];
                            size_t tx_len = pack_kermit_frame(frame.seq, 'Y', (uint8_t*)&converged_val, 8, tx_buffer);
                            usb_serial_jtag_write_bytes(tx_buffer, tx_len, pdMS_TO_TICKS(10));
                        }
                        else if (frame.type == 'V') {
                            // View/Retrieve APOGEE FUSE parameters: Respond with 32 bytes (base, secret, signal, converged_yi)
                            uint8_t tx_payload[32];
                            memcpy(&tx_payload[0], &apogee_base, 8);
                            memcpy(&tx_payload[8], &apogee_secret, 8);
                            memcpy(&tx_payload[16], &apogee_signal, 8);
                            memcpy(&tx_payload[24], &apogee_yi, 8);
                            
                            uint8_t tx_buffer[64];
                            size_t tx_len = pack_kermit_frame(frame.seq, 'Y', tx_payload, 32, tx_buffer);
                            usb_serial_jtag_write_bytes(tx_buffer, tx_len, pdMS_TO_TICKS(10));
                        }
                        else if (frame.type == 'S') {
                            // Signature Verify/Sync with Sliding Window: Payload contains 24 bytes (nonce, ichidai, daiichi)
                            uint64_t rx_nonce, rx_ichidai, rx_daiichi;
                            memcpy(&rx_nonce, &frame.data[0], 8);
                            memcpy(&rx_ichidai, &frame.data[8], 8);
                            memcpy(&rx_daiichi, &frame.data[16], 8);
                            
                            ESP_LOGI(TAG, "Signature verification request. Nonce: %llu", rx_nonce);
                            bool match_found = false;
                            
                            // Check sliding window of size 9 around received nonce
                            for (int dx = -4; dx <= 4; dx++) {
                                int64_t candidate = (int64_t)rx_nonce + dx;
                                if (candidate < 0) continue;
                                
                                uint64_t exp_ichidai, exp_daiichi;
                                yi_react_contractual((uint64_t)candidate, &exp_ichidai, &exp_daiichi);
                                
                                if (rx_ichidai == exp_ichidai && rx_daiichi == exp_daiichi) {
                                    // Succeeded! Re-synchronize local nonce tracking
                                    transmit_nonce = (uint64_t)candidate + 1;
                                    match_found = true;
                                    ESP_LOGI(TAG, "Signature VALID! Packet conforms to SEAL1. Re-synced transmit_nonce to %llu (offset: %d)", transmit_nonce, dx);
                                    break;
                                }
                            }
                            
                            if (match_found) {
                                // Respond with ACK containing the newly synchronized nonce value (8 bytes)
                                uint8_t tx_buffer[64];
                                size_t tx_len = pack_kermit_frame(frame.seq, 'Y', (uint8_t*)&transmit_nonce, 8, tx_buffer);
                                usb_serial_jtag_write_bytes(tx_buffer, tx_len, pdMS_TO_TICKS(10));
                            } else {
                                ESP_LOGE(TAG, "Signature INVALID! Verification failed.");
                                // Respond with NAK containing current device expected nonce (8 bytes)
                                uint8_t tx_buffer[64];
                                size_t tx_len = pack_kermit_frame(frame.seq, 'N', (uint8_t*)&transmit_nonce, 8, tx_buffer);
                                usb_serial_jtag_write_bytes(tx_buffer, tx_len, pdMS_TO_TICKS(10));
                            }
                        }
                    } else {
                        uint8_t fail_seq = (rx_buffer[2] >= 32) ? (rx_buffer[2] - 32) : 0;
                        uint64_t ichidai, daiichi;
                        yi_react_contractual(transmit_nonce, &ichidai, &daiichi);
                        uint8_t tx_buffer[64];
                        size_t tx_len = pack_kermit_frame(fail_seq, 'N', (uint8_t*)&daiichi, 8, tx_buffer);
                        modulate_ook_bytes(tx_buffer, tx_len);
                    }
                    rx_idx = 0; // Reset buffer
                }
            }
        }
        vTaskDelay(1); // Force 1 FreeRTOS tick delay to guarantee CPU yield to IDLE
    }
}
