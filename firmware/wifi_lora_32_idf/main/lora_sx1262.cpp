#include "lora_sx1262.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_random.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "SX1262";
static spi_device_handle_t spi_handle;
static RadioEvents_t radio_events;

// Wait for BUSY pin to go low (Active High)
static void lora_wait_busy(void) {
    while (gpio_get_level((gpio_num_t)LORA_BUSY_PIN) == 1) {
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

// Send standard SPI command
static void lora_write_cmd(uint8_t opcode, const uint8_t *params, size_t len) {
    lora_wait_busy();
    
    // SPI transaction
    spi_transaction_t t = {};
    t.flags = 0;
    t.length = 8 * (1 + len);
    
    uint8_t *buf = (uint8_t *)malloc(1 + len);
    buf[0] = opcode;
    if (len > 0) {
        memcpy(buf + 1, params, len);
    }
    t.tx_buffer = buf;
    
    spi_device_polling_transmit(spi_handle, &t);
    free(buf);
}

// Write payload bytes to the SX1262 internal buffer
static void lora_write_buffer(uint8_t offset, const uint8_t *data, size_t len) {
    lora_wait_busy();
    
    spi_transaction_t t = {};
    t.length = 8 * (2 + len);
    
    uint8_t *buf = (uint8_t *)malloc(2 + len);
    buf[0] = SX1262_OP_WRITE_BUFFER;
    buf[1] = offset;
    memcpy(buf + 2, data, len);
    
    t.tx_buffer = buf;
    spi_device_polling_transmit(spi_handle, &t);
    free(buf);
}

// Read payload bytes from the SX1262 internal buffer
static void lora_read_buffer(uint8_t offset, uint8_t *data, size_t len) {
    lora_wait_busy();
    
    spi_transaction_t t = {};
    t.length = 8 * (3 + len); // Command + Offset + NOP + data
    
    uint8_t *tx_buf = (uint8_t *)malloc(3 + len);
    uint8_t *rx_buf = (uint8_t *)malloc(3 + len);
    memset(tx_buf, 0, 3 + len);
    
    tx_buf[0] = SX1262_OP_READ_BUFFER;
    tx_buf[1] = offset;
    tx_buf[2] = 0x00; // Status / NOP byte
    
    t.tx_buffer = tx_buf;
    t.rx_buffer = rx_buf;
    
    spi_device_polling_transmit(spi_handle, &t);
    memcpy(data, rx_buf + 3, len);
    
    free(tx_buf);
    free(rx_buf);
}

// Get register status
static uint16_t lora_get_irq_status(void) {
    lora_wait_busy();
    
    spi_transaction_t t = {};
    t.length = 8 * 4;
    
    uint8_t tx_buf[4] = {SX1262_OP_GET_IRQ_STATUS, 0x00, 0x00, 0x00};
    uint8_t rx_buf[4] = {0};
    
    t.tx_buffer = tx_buf;
    t.rx_buffer = rx_buf;
    
    spi_device_polling_transmit(spi_handle, &t);
    return ((uint16_t)rx_buf[2] << 8) | rx_buf[3];
}

esp_err_t lora_radio_init(const RadioEvents_t *events) {
    if (events) {
        radio_events = *events;
    }

    // 1. Initialize GPIO Control Pins (Reset, Busy, CS)
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << LORA_RESET_PIN);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);

    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << LORA_BUSY_PIN) | (1ULL << LORA_DIO1_PIN);
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&io_conf);

    // Hardware Reset SX1262
    gpio_set_level((gpio_num_t)LORA_RESET_PIN, 0);
    vTaskDelay(pdMS_TO_TICKS(20));
    gpio_set_level((gpio_num_t)LORA_RESET_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(20));

    // 2. Initialize SPI Master Bus
    spi_bus_config_t buscfg = {};
    buscfg.mosi_io_num = LORA_MOSI_PIN;
    buscfg.miso_io_num = LORA_MISO_PIN;
    buscfg.sclk_io_num = LORA_SCK_PIN;
    buscfg.quadwp_io_num = -1;
    buscfg.quadhd_io_num = -1;
    buscfg.max_transfer_sz = 4096;

    esp_err_t ret = spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) return ret;

    spi_device_interface_config_t devcfg = {};
    devcfg.clock_speed_hz = 8000000; // 8 MHz
    devcfg.mode = 0;                 // CPOL=0, CPHA=0
    devcfg.spics_io_num = LORA_CS_PIN;
    devcfg.queue_size = 7;

    ret = spi_bus_add_device(SPI2_HOST, &devcfg, &spi_handle);
    if (ret != ESP_OK) return ret;

    // 3. Configure Basic SX1262 Settings
    uint8_t stdby_mode[] = {0x01}; // Standby RC mode
    lora_write_cmd(SX1262_OP_SET_STANDBY, stdby_mode, 1);

    uint8_t packet_type[] = {0x01}; // LoRa Mode
    lora_write_cmd(SX1262_OP_SET_PACKET_TYPE, packet_type, 1);

    ESP_LOGI(TAG, "SX1262 Radio initialized successfully.");
    return ESP_OK;
}

void lora_radio_set_channel(uint32_t freq_hz) {
    // RfFreq = (FreqHz * 2^25) / 32,000,000
    uint64_t freq_calc = ((uint64_t)freq_hz << 25) / 32000000ULL;
    uint8_t buf[4];
    buf[0] = (freq_calc >> 24) & 0xFF;
    buf[1] = (freq_calc >> 16) & 0xFF;
    buf[2] = (freq_calc >> 8) & 0xFF;
    buf[3] = freq_calc & 0xFF;

    lora_write_cmd(SX1262_OP_SET_RF_FREQUENCY, buf, 4);
    ESP_LOGI(TAG, "LoRa Channel set to %lu Hz", freq_hz);
}

void lora_radio_set_tx_config(int8_t power, uint32_t bandwidth, uint32_t sf, uint32_t cr, uint16_t preamble_len) {
    // Set PA Config
    uint8_t pa_config[] = {0x04, 0x07, 0x00, 0x01};
    lora_write_cmd(SX1262_OP_SET_PA_CONFIG, pa_config, 4);

    // Set Tx Params
    uint8_t tx_params[2];
    tx_params[0] = power;
    tx_params[1] = 0x02; // Ramp time: 40us
    lora_write_cmd(SX1262_OP_SET_TX_PARAMS, tx_params, 2);

    // Set Modulation Params
    uint8_t mod_params[8];
    mod_params[0] = sf;
    mod_params[1] = bandwidth; 
    mod_params[2] = cr;
    mod_params[3] = 0x00; // Low Data Rate Optimize Off
    memset(mod_params + 4, 0, 4);
    lora_write_cmd(SX1262_OP_SET_MODULATION_PARAMS, mod_params, 8);

    // Set Packet Params
    uint8_t pkt_params[6];
    pkt_params[0] = (preamble_len >> 8) & 0xFF;
    pkt_params[1] = preamble_len & 0xFF;
    pkt_params[2] = 0x00; // Explicit Header
    pkt_params[3] = 232;  // Payload Size
    pkt_params[4] = 0x00; // CRC Off
    pkt_params[5] = 0x00; // Inverted IQ off
    lora_write_cmd(SX1262_OP_SET_PACKET_PARAMS, pkt_params, 6);
}

void lora_radio_set_rx_config(uint32_t bandwidth, uint32_t sf, uint32_t cr, uint16_t preamble_len) {
    // Set Modulation Params
    uint8_t mod_params[8];
    mod_params[0] = sf;
    mod_params[1] = bandwidth; 
    mod_params[2] = cr;
    mod_params[3] = 0x00; // Low Data Rate Optimize Off
    memset(mod_params + 4, 0, 4);
    lora_write_cmd(SX1262_OP_SET_MODULATION_PARAMS, mod_params, 8);

    // Set Packet Params
    uint8_t pkt_params[6];
    pkt_params[0] = (preamble_len >> 8) & 0xFF;
    pkt_params[1] = preamble_len & 0xFF;
    pkt_params[2] = 0x00; // Explicit Header
    pkt_params[3] = 232;  // Payload Size
    pkt_params[4] = 0x00; // CRC Off
    pkt_params[5] = 0x00; // Inverted IQ off
    lora_write_cmd(SX1262_OP_SET_PACKET_PARAMS, pkt_params, 6);
}

void lora_radio_send(const uint8_t *payload, uint8_t size) {
    // Write payload to buffer
    lora_write_buffer(0x00, payload, size);

    // Set Tx Buffer base
    uint8_t base_addr[] = {0x00, 0x00};
    lora_write_cmd(SX1262_OP_SET_BUFFER_BASE_ADDR, base_addr, 2);

    // Set DIO Irq Params: TxDone, TxTimeout
    uint8_t irq_params[] = {0x00, 0x03, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00};
    lora_write_cmd(SX1262_OP_SET_DIO_IRQ_PARAMS, irq_params, 8);

    // Set Tx mode (Timeout = 0: Single Tx)
    uint8_t timeout[] = {0x00, 0x00, 0x00};
    lora_write_cmd(SX1262_OP_SET_TX, timeout, 3);
}

void lora_radio_rx(uint32_t timeout_ms) {
    // Set Rx Buffer base
    uint8_t base_addr[] = {0x00, 0x00};
    lora_write_cmd(SX1262_OP_SET_BUFFER_BASE_ADDR, base_addr, 2);

    // Set DIO Irq Params: RxDone, RxTimeout
    uint8_t irq_params[] = {0x00, 0x0C, 0x00, 0x0C, 0x00, 0x00, 0x00, 0x00};
    lora_write_cmd(SX1262_OP_SET_DIO_IRQ_PARAMS, irq_params, 8);

    // Set Rx mode (Timeout 0xFFFFFF: Continuous Rx)
    uint8_t timeout[3] = {0xFF, 0xFF, 0xFF};
    lora_write_cmd(SX1262_OP_SET_RX, timeout, 3);
}

void lora_radio_irq_process(void) {
    uint16_t status = lora_get_irq_status();

    // Clear IRQ status
    uint8_t clear_buf[2] = { (uint8_t)((status >> 8) & 0xFF), (uint8_t)(status & 0xFF) };
    lora_write_cmd(SX1262_OP_CLEAR_IRQ_STATUS, clear_buf, 2);

    if (status & 0x01) { // TxDone
        if (radio_events.TxDone) radio_events.TxDone();
    }
    if (status & 0x02) { // RxDone
        uint8_t rx_data[232];
        lora_read_buffer(0x00, rx_data, 232);
        if (radio_events.RxDone) {
            radio_events.RxDone(rx_data, 232, -50, 10);
        }
    }
    if (status & 0x04) { // TxTimeout / RxTimeout
        if (radio_events.RxTimeout) radio_events.RxTimeout();
    }
}

uint32_t lora_radio_random(void) {
    // ESP32-S3 has a robust Hardware RNG which is much better than reading radio RSSI registers.
    return esp_random();
}
