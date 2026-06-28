#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_log.h"

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

#define UART_PORT_NUM UART_NUM_1
#define PIN_UART_TX   18
#define PIN_UART_RX   19

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

// Write command/data to SX1262
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
    
    // Wait for BUSY pin to go low (device ready)
    while (gpio_get_level(PIN_NUM_BUSY) == 1) {
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    
    spi_device_transmit(spi, &t);
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
static void sx1262_config_lora(void) {
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
    uint8_t pkt_params[9] = {
        0x00, 0x08,       // Preamble Length (8 symbols)
        0x00,             // Header Type (Variable/Explicit)
        0x60,             // Payload Length (96 bytes)
        0x01,             // CRC Type (On)
        0x00,             // Invert IQ (Normal)
        0x00, 0x00, 0x00, 0x00
    };
    sx1262_write_cmd(0x8C, pkt_params, 9);
    
    ESP_LOGI(TAG, "SX1262 LoRa Setup Successful");
}

static void uart_init(void) {
    uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    ESP_ERROR_CHECK(uart_driver_install(UART_PORT_NUM, 256, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(UART_PORT_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_PORT_NUM, PIN_UART_TX, PIN_UART_RX, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_LOGI(TAG, "UART initialized at 9600 bps");
}

void app_main(void) {
    ESP_LOGI(TAG, "Starting Heltec v4 ESP32-S3 OOK Kermit Firmware");
    
    // Setup GPIOs
    gpio_reset_pin(PIN_NUM_BUSY);
    gpio_set_direction(PIN_NUM_BUSY, GPIO_MODE_INPUT);
    gpio_reset_pin(PIN_NUM_DIO2);
    gpio_set_direction(PIN_NUM_DIO2, GPIO_MODE_OUTPUT);
    
    spi_init();
    sx1262_reset();
    sx1262_config_ook();
    uart_init();
    
    uint8_t data_buf[128];
    while (1) {
        // Direct OOK Bridge Mode: UART Rx bytes write directly to DIO2
        int len = uart_read_bytes(UART_PORT_NUM, data_buf, sizeof(data_buf), pdMS_TO_TICKS(10));
        if (len > 0) {
            // Modulate DIO2 pin envelope matching the incoming serial bitstream
            for (int i = 0; i < len; i++) {
                uint8_t byte = data_buf[i];
                for (int bit = 0; bit < 8; bit++) {
                    uint8_t bit_val = (byte >> bit) & 0x01;
                    gpio_set_level(PIN_NUM_DIO2, bit_val);
                    // Bit timing at 9600 bps = 104 microseconds per bit
                    esp_rom_delay_us(104);
                }
            }
            ESP_LOGD(TAG, "Modulated %d bytes over OOK", len);
        }
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}
