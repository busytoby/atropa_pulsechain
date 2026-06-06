#pragma once

#include <stdint.h>
#include "esp_err.h"

// Heltec WiFi LoRa 32 V3/V4 SX1262 pin definitions
#define LORA_MOSI_PIN    10
#define LORA_MISO_PIN    11
#define LORA_SCK_PIN     9
#define LORA_CS_PIN      8
#define LORA_RESET_PIN   12
#define LORA_BUSY_PIN    13
#define LORA_DIO1_PIN    14

// Opcode Definitions for SX1262 Commands
#define SX1262_OP_SET_STANDBY              0x80
#define SX1262_OP_SET_PACKET_TYPE          0x8A
#define SX1262_OP_SET_RF_FREQUENCY         0x86
#define SX1262_OP_SET_PA_CONFIG            0x95
#define SX1262_OP_SET_TX_PARAMS            0x8E
#define SX1262_OP_SET_BUFFER_BASE_ADDR     0x8F
#define SX1262_OP_SET_MODULATION_PARAMS    0x8B
#define SX1262_OP_SET_PACKET_PARAMS        0x8C
#define SX1262_OP_SET_DIO_IRQ_PARAMS       0x08
#define SX1262_OP_CLEAR_IRQ_STATUS         0x02
#define SX1262_OP_GET_IRQ_STATUS           0x12
#define SX1262_OP_WRITE_BUFFER             0x0E
#define SX1262_OP_READ_BUFFER              0x1E
#define SX1262_OP_SET_TX                   0x83
#define SX1262_OP_SET_RX                   0x82

typedef struct {
    void (*TxDone)(void);
    void (*TxTimeout)(void);
    void (*RxDone)(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr);
    void (*RxTimeout)(void);
} RadioEvents_t;

// API functions
esp_err_t lora_radio_init(const RadioEvents_t *events);
void lora_radio_set_channel(uint32_t freq_hz);
void lora_radio_set_tx_config(int8_t power, uint32_t bandwidth, uint32_t sf, uint32_t cr, uint16_t preamble_len);
void lora_radio_set_rx_config(uint32_t bandwidth, uint32_t sf, uint32_t cr, uint16_t preamble_len);
void lora_radio_send(const uint8_t *payload, uint8_t size);
void lora_radio_rx(uint32_t timeout_ms);
void lora_radio_irq_process(void);
uint32_t lora_radio_random(void);
