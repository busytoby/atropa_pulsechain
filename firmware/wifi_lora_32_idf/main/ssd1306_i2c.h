#pragma once

#include <stdint.h>
#include "esp_err.h"

#define SSD1306_I2C_ADDRESS         0x3C

// Function Declarations
esp_err_t ssd1306_init(int sda_pin, int scl_pin, int rst_pin, int vext_pin);
void ssd1306_clear();
void ssd1306_draw_string(uint8_t page, uint8_t col, const char *str);
void ssd1306_power_save(bool enable);
