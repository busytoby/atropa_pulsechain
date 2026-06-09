# ESP-IDF Conversion Plan: HelTec WiFi LoRa 32 V3/V4

We are migrating the HelTec WiFi LoRa 32 Arduino project ([version3.ino](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/firmware/version3.ino)) to a native ESP-IDF application under `firmware/wifi_lora_32_idf/`.

---

## 1. Directory Structure

```
firmware/wifi_lora_32_idf/
├── CMakeLists.txt              # Root CMake configuration
├── sdkconfig.defaults          # Default configuration parameters (SPIFFS, mbedTLS, etc.)
└── main/
    ├── CMakeLists.txt          # Main component CMake configuration
    ├── main.cpp                # Core application loop and initialization
    ├── lora_sx1262.h           # Native ESP-IDF SX1262 SPI/GPIO driver
    ├── lora_sx1262.cpp
    ├── ssd1306_i2c.h           # Native ESP-IDF SSD1306 OLED I2C driver
    └── ssd1306_i2c.cpp
```

---

## 2. API Mapping & Replacements

| Arduino API | ESP-IDF / Native C++ Replacement |
| :--- | :--- |
| **`setup()` / `loop()`** | `app_main()` spawning a dedicated FreeRTOS worker task. |
| **`Serial.begin()` / `Serial.print()`** | ESP-IDF UART driver / `esp_log.h` or standard `printf`. |
| **`xSemaphoreCreateMutex()`** | Native FreeRTOS `xSemaphoreCreateMutex()`. |
| **`LittleFS`** | Native ESP-IDF SPIFFS mounted via Virtual Filesystem (`esp_spiffs.h`). |
| **`mbedtls` (Arduino wrapper)** | Built-in ESP-IDF mbedTLS component (`mbedtls_mpi` functions). |
| **`Radio` (LoRaWan_APP)** | Custom native SX1262 SPI controller (`lora_sx1262.cpp`). |
| **`U8g2`** | Native SSD1306 OLED writer using I2C commands (`ssd1306_i2c.cpp`). |
| **`ESP.getEfuseMac()`** | `esp_efuse_mac_get_default()`. |
| **`esp_timer_get_time()`** | Native `esp_timer_get_time()`. |

---

## 3. Hardware Pinout Configuration (HelTec V3/V4)

* **OLED Screen (SSD1306 I2C)**:
  * SDA: `GPIO 18` (or `SDA_OLED` pin designation)
  * SCL: `GPIO 17` (or `SCL_OLED` pin designation)
  * Reset: `GPIO 21` (or `RST_OLED` pin designation)
  * Vext control: `GPIO 36` (low to power on displays and sensors)
* **LoRa (SX1262 SPI)**:
  * NSS (CS): `GPIO 8`
  * SCK: `GPIO 9`
  * MOSI: `GPIO 10`
  * MISO: `GPIO 11`
  * BUSY: `GPIO 13`
  * NRESET: `GPIO 12`
  * DIO1 (IRQ): `GPIO 14`

---

## 4. Implementation Steps

1. **Step 1**: Create the core project directory and root `CMakeLists.txt` + `sdkconfig.defaults`.
2. **Step 2**: Implement the native I2C SSD1306 screen display writer component.
3. **Step 3**: Implement the native SPI SX1262 LoRa radio driver.
4. **Step 4**: Port the mbedTLS big number primality verification sequence and LittleFS (SPIFFS) config saving logic.
5. **Step 5**: Write `main.cpp` containing `app_main()` and setup the FreeRTOS task loops.
