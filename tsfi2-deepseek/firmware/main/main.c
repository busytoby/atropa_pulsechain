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
#include "driver/usb_serial_jtag.h"

static const char *TAG = "HELTEC_OOK_FW";

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

// Sign an incrementing nonce using the shared YI state parameters
static uint64_t sign_nonce_with_yi(uint64_t nonce, NodeRole role) {
    uint64_t signature = yi_state.base;
    if (role == ROLE_ROD) {
        signature = (signature ^ yi_state.secret ^ nonce) % yi_state.motzkin_prime;
    } else {
        signature = (signature ^ yi_state.signal ^ nonce) % yi_state.motzkin_prime;
    }
    return signature;
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
    // Deregister main task from Task Watchdog monitoring
    esp_task_wdt_delete(NULL);
    
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
    
    usb_serial_jtag_init();
    
    uint8_t rx_buffer[256];
    size_t rx_idx = 0;
    
    uint32_t entropy = get_entropy_random();
    ESP_LOGI(TAG, "Auncient TRNG Sourced Random Number: 0x%08X", (unsigned int)entropy);
    ESP_LOGI(TAG, "Firmware initialized. Awaiting Kermit handshake packages...");
    
    while (1) {
        // Read raw data from JTAG RX FIFO
        uint8_t byte;
        int read_len = usb_serial_jtag_read_bytes(&byte, 1, pdMS_TO_TICKS(5));
        if (read_len > 0) {
            if (byte == KERMIT_SOH) {
                rx_idx = 0;
            }
            if (rx_idx < sizeof(rx_buffer)) {
                rx_buffer[rx_idx++] = byte;
            }
            
            // Check if we have received a complete kermit frame
            if (rx_idx >= 5) {
                uint8_t expected_len = rx_buffer[1] - 32;
                if (rx_idx >= (size_t)(expected_len + 2)) {
                    KermitFrame frame;
                    if (parse_kermit_frame(rx_buffer, rx_idx, &frame)) {
                        ESP_LOGI(TAG, "Valid Kermit Frame. Seq: %d | Type: %c", frame.seq, frame.type);
                        
                        if (frame.type == 'S' && frame.len - 3 >= 32) {
                            ESP_LOGI(TAG, "Processing Open Key Exchange (Send-Init)...");
                            uint8_t peer_pubkey[32];
                            memcpy(peer_pubkey, frame.data, 32);
                            const uint8_t public_key_b[32] = {
                                0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                                0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10,
                                0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
                                0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20
                            };
                            
                            // Hash-reduce modulo MOTZKIN_PRIME
                            yi_state.base = 0;
                            for (int i = 0; i < 32; i++) {
                                yi_state.base = (yi_state.base * 31 + (public_key_b[i] ^ peer_pubkey[i])) % MOTZKIN_PRIME;
                            }
                            ESP_LOGI(TAG, "Derived Shared Secret YI Base: %lu", yi_state.base);
                            vTaskDelay(pdMS_TO_TICKS(5));
                            uint8_t tx_buffer[128];
                            size_t tx_len = pack_kermit_frame(frame.seq, 'Y', public_key_b, 32, tx_buffer);
                            usb_serial_jtag_write_bytes(tx_buffer, tx_len, pdMS_TO_TICKS(10));
                            ESP_LOGI(TAG, "Open ACK Frame returned over USB JTAG.");
                        }
                        else if (frame.type == 'Y' && frame.len - 3 == 32) {
                            ESP_LOGI(TAG, "Processing Open Key Exchange ACK...");
                            uint8_t peer_pubkey[32];
                            memcpy(peer_pubkey, frame.data, 32);
                            const uint8_t public_key_a[32] = {
                                0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                                0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
                                0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
                                0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
                            };
                            
                            // Hash-reduce modulo MOTZKIN_PRIME
                            yi_state.base = 0;
                            for (int i = 0; i < 32; i++) {
                                yi_state.base = (yi_state.base * 31 + (public_key_a[i] ^ peer_pubkey[i])) % MOTZKIN_PRIME;
                            }
                            ESP_LOGI(TAG, "Derived Shared Secret YI Base: %lu", yi_state.base);
                        }
                        else if (frame.type == 'D' && frame.len - 3 >= 48) {
                            ESP_LOGI(TAG, "Processing Handshake/Cryptographic Payload...");
                            
                            uint8_t rx_pubkey[32];
                            uint64_t rx_nonce;
                            uint64_t rx_sig;
                            memcpy(rx_pubkey, &frame.data[0], 32);
                            memcpy(&rx_nonce, &frame.data[32], 8);
                            memcpy(&rx_sig, &frame.data[40], 8);
                            
                            // Verify the incoming signature generated by the other node's role
                            uint64_t expected_sig = sign_nonce_with_yi(rx_nonce, ROLE_CONE);
                            if (rx_sig != expected_sig) {
                                ESP_LOGE(TAG, "YI.react Signature Verification FAILED! Expected: %lu, Got: %lu", expected_sig, rx_sig);
                                // Send NAK back over JTAG
                                uint8_t tx_buffer[64];
                                size_t tx_len = pack_kermit_frame(frame.seq, 'N', NULL, 0, tx_buffer);
                                usb_serial_jtag_write_bytes(tx_buffer, tx_len, pdMS_TO_TICKS(10));
                            } else {
                                ESP_LOGI(TAG, "YI.react Signature VERIFIED. Sourced valid nonce: %lu", rx_nonce);
                                
                                // Node B Ephemeral Public Key
                                const uint8_t public_key_b[32] = {
                                    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                                    0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10,
                                    0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
                                    0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20
                                };
                                
                                // Derive shared secret key
                                uint8_t shared_secret[32];
                                for (int i = 0; i < 32; i++) {
                                    shared_secret[i] = rx_pubkey[i] ^ public_key_b[i];
                                }
                                
                                ESP_LOGI(TAG, "Derived Shared Secret Hash Segment: 0x%02X%02X%02X%02X",
                                         shared_secret[0], shared_secret[1], shared_secret[2], shared_secret[3]);
                                
                                // Settle guard delay
                                vTaskDelay(pdMS_TO_TICKS(5));
                                
                                // Generate return signature: B (ROD role) signs (nonce + 1)
                                uint8_t ack_payload[48];
                                memcpy(ack_payload, public_key_b, 32);
                                uint64_t tx_nonce = rx_nonce + 1;
                                uint64_t tx_sig = sign_nonce_with_yi(tx_nonce, ROLE_ROD);
                                memcpy(&ack_payload[32], &tx_nonce, 8);
                                memcpy(&ack_payload[40], &tx_sig, 8);
                                
                                uint8_t tx_buffer[128];
                                size_t tx_len = pack_kermit_frame(frame.seq, 'Y', ack_payload, 48, tx_buffer);
                                
                                // Return the signed ACK frame back over JTAG
                                usb_serial_jtag_write_bytes(tx_buffer, tx_len, pdMS_TO_TICKS(10));
                                ESP_LOGI(TAG, "Signed ACK Frame returned over JTAG.");
                            }
                        }
                    } else {
                        ESP_LOGW(TAG, "Kermit Frame Checksum Failed. Transmitting NAK envelope...");
                        
                        // Parse header elements if SOH is valid to retrieve sequence number
                        uint8_t fail_seq = (rx_buffer[2] >= 32) ? (rx_buffer[2] - 32) : 0;
                        
                        // Delay 5ms to avoid collisions during half-duplex transitions
                        vTaskDelay(pdMS_TO_TICKS(5));
                        
                        uint8_t tx_buffer[64];
                        size_t tx_len = pack_kermit_frame(fail_seq, 'N', NULL, 0, tx_buffer);
                        modulate_ook_bytes(tx_buffer, tx_len);
                    }
                    rx_idx = 0; // Reset buffer
                }
            }
        }
        vTaskDelay(1); // Force 1 FreeRTOS tick delay to guarantee CPU yield to IDLE
    }
}
