#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "esp_system.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_efuse.h"
#include "esp_mac.h"
#include "esp_spiffs.h"
#include "nvs_flash.h"
#include "driver/uart.h"
#include "driver/usb_serial_jtag.h"

#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/bignum.h"

#include "ssd1306_i2c.h"
#include "lora_sx1262.h"

static const char *TAG = "MainApp";
static SemaphoreHandle_t mutex;

// Display constants
#define SDA_OLED_PIN    18
#define SCL_OLED_PIN    17
#define RST_OLED_PIN    21
#define VEXT_PIN        36

#define BUFFER_SIZE     232
#define DISPLAY_CHARS   16 // SSD1306 Page direct fits fewer chars nicely, 16 fits 128px

static char txpacket[BUFFER_SIZE];
static char rxpacket[BUFFER_SIZE];

static uint32_t last_tx = 0;
static uint32_t last_refresh = 0;
static bool ScreenOn = true;
static char screenlines[6][64];
static int last_line = 0;

static int16_t txNumber = 0;
static int16_t rxNumber = 0;
static bool receiveflag = false;
static uint64_t chipid = 0;
static bool lora_idle = true;
static int idlemod = 1;

static char Version[8] = "0.302";
static char Handle[32] = "[:h changeme]";

void OnTxDone(void) {
    lora_idle = true;
    lora_radio_rx(0);
}

void OnTxTimeout(void) {
    ESP_LOGW(TAG, "TX Timeout occurred.");
}

void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr) {
    memcpy(rxpacket, payload, size);
    rxpacket[size] = '\0';
    
    // Print to serial console
    for (int i = 0; i < size; i++) {
        if (payload[i] != '\0') putchar(payload[i]);
    }
    putchar('\n');

    rxNumber++;

    xSemaphoreTake(mutex, pdMS_TO_TICKS(400));
    if (last_line < 5) {
        memset(screenlines[last_line], 0, sizeof(screenlines[last_line]));
        strncpy(screenlines[last_line], rxpacket, sizeof(screenlines[last_line]) - 1);
        last_line++;
    } else {
        for (int j = 0; j < 4; j++) {
            memset(screenlines[j], 0, sizeof(screenlines[j]));
            strncpy(screenlines[j], screenlines[j+1], sizeof(screenlines[j]) - 1);
        }
        memset(screenlines[4], 0, sizeof(screenlines[4]));
        strncpy(screenlines[4], rxpacket, sizeof(screenlines[4]) - 1);
    }
    last_refresh = 0;
    xSemaphoreGive(mutex);

    receiveflag = true;
    lora_idle = true;
}

// SPIFFS Filesystem Config File Path
#define CONFIG_FILE_PATH "/spiffs/hel_config"

void SaveConfig() {
    FILE *cf = fopen(CONFIG_FILE_PATH, "w");
    if (!cf) {
        ESP_LOGE(TAG, "Failed to open config file for writing");
    } else {
        fprintf(cf, "h %s\n", Handle);
        fclose(cf);
        ESP_LOGI(TAG, "Configuration written to SPIFFS.");
    }
}

void PrintConfig() {
    FILE *cf = fopen(CONFIG_FILE_PATH, "r");
    if (!cf) {
        ESP_LOGW(TAG, "No config file found in SPIFFS.");
    } else {
        ESP_LOGI(TAG, "Reading configuration file:");
        char line[64];
        while (fgets(line, sizeof(line), cf)) {
            printf("%s", line);
        }
        fclose(cf);
    }
}

// mbedTLS Math Constants and Variables
static char mpibuf[256] = "0x069e8fC82e18F4";
mbedtls_entropy_context entropy;
mbedtls_ctr_drbg_context ctr_drbg;
const char* APOGEE = "953473";
const char* APEX = "954114361";
const char* MotzkinPrime = "953467954114363";

mbedtls_mpi m, x, b, y, s, l, g, i, o, q, t, d, H, L;
bool math_init_complete = false;

#define PRIMEVERIFYROUNDS 1025
mbedtls_mpi_sint lb = 99;
mbedtls_mpi_sint gb = 49;
mbedtls_mpi_sint ib = 269;
mbedtls_mpi_sint ob = 257;
mbedtls_mpi_sint qb = 132;
mbedtls_mpi_sint tb = 693;
mbedtls_mpi_sint db = 110;
mbedtls_mpi_sint Hb = 187;
mbedtls_mpi_sint Lb = 359;

char* mpistring(const mbedtls_mpi *V) {
    size_t n_written;
    memset(mpibuf, 0, sizeof(mpibuf));
    int ret = mbedtls_mpi_write_string(V, 16, mpibuf, sizeof(mpibuf) - 1, &n_written);
    if (ret == 0 && n_written > 0) {
        mpibuf[n_written] = '\0';
        for (int k = 1; k < n_written; k += 2) mpibuf[k] = tolower((unsigned char)mpibuf[k]);
        return mpibuf;
    }
    ESP_LOGE(TAG, "MPI string writing failed: -0x%04X", -ret);
    return mpibuf;
}

void MathInit() {
    if (!math_init_complete) {
        mbedtls_mpi_init(&m);
        mbedtls_mpi_init(&x);
        mbedtls_mpi_init(&b);
        mbedtls_mpi_init(&y);
        mbedtls_mpi_init(&s);
        mbedtls_mpi_init(&l);
        mbedtls_mpi_init(&g);
        mbedtls_mpi_init(&i);
        mbedtls_mpi_init(&o);
        mbedtls_mpi_init(&q);
        mbedtls_mpi_init(&t);
        mbedtls_mpi_init(&d);
        mbedtls_mpi_init(&H);
        mbedtls_mpi_init(&L);

        mbedtls_entropy_init(&entropy);
        mbedtls_ctr_drbg_init(&ctr_drbg);

        mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
                              (const unsigned char *)mpibuf, strlen(mpibuf));
        math_init_complete = true;
    } else {
        mbedtls_ctr_drbg_reseed(&ctr_drbg, (const unsigned char *)mpibuf, strlen(mpibuf));
    }

    mbedtls_mpi_read_string(&m, 10, APOGEE);
    mbedtls_mpi_read_string(&x, 10, APEX);
    mbedtls_mpi_read_string(&b, 10, MotzkinPrime);
    mbedtls_mpi_mul_mpi(&y, &m, &x);
    mbedtls_mpi_add_mpi(&y, &y, &b);

    char DysnomiaPrime[64];
    strcpy(DysnomiaPrime, APOGEE);
    strcat(DysnomiaPrime, APEX);
    mbedtls_mpi_read_string(&s, 10, DysnomiaPrime);

    printf("# APOGEE       m= 0x%s\n", mpistring(&m));
    printf("# APEX         x= 0x%s\n", mpistring(&x));
    printf("# MotzkinPrime b= 0x%s\n", mpistring(&b));
    printf("# SLOPE        s= 0x%s\n", mpistring(&s));
    printf("# DYSNOMIA     y= 0x%s\n", mpistring(&y));

    // LOVE
    mbedtls_mpi_mul_mpi(&l, &m, &y);
    mbedtls_mpi_sub_int(&l, &l, lb);
    while (mbedtls_mpi_is_prime_ext(&l, PRIMEVERIFYROUNDS, mbedtls_ctr_drbg_random, &ctr_drbg) != 0) {
        mbedtls_mpi_sub_int(&l, &l, 1);
        lb++;
    }
    printf("# LOVE         l= 0x%s [%ld]\n", mpistring(&l), (long)lb);

    // GAIN
    mbedtls_mpi_mul_mpi(&g, &m, &s);
    mbedtls_mpi_add_mpi(&g, &g, &b);
    mbedtls_mpi_mul_mpi(&g, &g, &m);
    mbedtls_mpi_sub_int(&g, &g, gb);
    while (mbedtls_mpi_is_prime_ext(&g, PRIMEVERIFYROUNDS, mbedtls_ctr_drbg_random, &ctr_drbg) != 0) {
        mbedtls_mpi_sub_int(&g, &g, 1);
        gb++;
    }
    printf("# GAIN         g= 0x%s [%ld]\n", mpistring(&g), (long)gb);

    // I (1)
    mbedtls_mpi_mul_mpi(&i, &m, &b);
    mbedtls_mpi_add_mpi(&i, &i, &s);
    mbedtls_mpi_sub_int(&i, &i, ib);
    while (mbedtls_mpi_is_prime_ext(&i, PRIMEVERIFYROUNDS, mbedtls_ctr_drbg_random, &ctr_drbg) != 0) {
        mbedtls_mpi_sub_int(&i, &i, 1);
        ib++;
    }
    printf("# I            i= 0x%s [%ld]\n", mpistring(&i), (long)ib);

    // O (2)
    mbedtls_mpi_mul_mpi(&o, &y, &y);
    mbedtls_mpi_sub_int(&o, &o, ob);
    while (mbedtls_mpi_is_prime_ext(&o, PRIMEVERIFYROUNDS, mbedtls_ctr_drbg_random, &ctr_drbg) != 0) {
        mbedtls_mpi_sub_int(&o, &o, 1);
        ob++;
    }
    printf("# O            o= 0x%s [%ld]\n", mpistring(&o), (long)ob);

    // Q (3)
    mbedtls_mpi_mul_mpi(&q, &i, &i);
    mbedtls_mpi_sub_int(&q, &q, qb);
    while (mbedtls_mpi_is_prime_ext(&q, PRIMEVERIFYROUNDS, mbedtls_ctr_drbg_random, &ctr_drbg) != 0) {
        mbedtls_mpi_sub_int(&q, &q, 1);
        qb++;
    }
    printf("# Q            q= 0x%s [%ld]\n", mpistring(&q), (long)qb);

    // T (4)
    mbedtls_mpi_mul_mpi(&t, &o, &g);
    mbedtls_mpi_add_mpi(&t, &t, &q);
    mbedtls_mpi_sub_int(&t, &t, tb);
    while (mbedtls_mpi_is_prime_ext(&t, PRIMEVERIFYROUNDS, mbedtls_ctr_drbg_random, &ctr_drbg) != 0) {
        mbedtls_mpi_sub_int(&t, &t, 1);
        tb++;
    }
    printf("# T            t= 0x%s [%ld]\n", mpistring(&t), (long)tb);

    // D (5)
    mbedtls_mpi_mul_mpi(&d, &g, &y);
    mbedtls_mpi_add_mpi(&d, &d, &o);
    mbedtls_mpi_mul_mpi(&d, &g, &d);
    mbedtls_mpi_sub_int(&d, &d, db);
    while (mbedtls_mpi_is_prime_ext(&d, PRIMEVERIFYROUNDS, mbedtls_ctr_drbg_random, &ctr_drbg) != 0) {
        mbedtls_mpi_sub_int(&d, &d, 1);
        db++;
    }
    printf("# D            d= 0x%s [%ld]\n", mpistring(&d), (long)db);

    // H (6)
    mbedtls_mpi_add_int(&H, &d, db);
    mbedtls_mpi_sub_mpi(&H, &H, &b);
    mbedtls_mpi_sub_int(&H, &H, Hb);
    while (mbedtls_mpi_is_prime_ext(&H, PRIMEVERIFYROUNDS, mbedtls_ctr_drbg_random, &ctr_drbg) != 0) {
        mbedtls_mpi_sub_int(&H, &H, 1);
        Hb++;
    }
    printf("# H            H= 0x%s [%ld]\n", mpistring(&H), (long)Hb);

    // L (7)
    mbedtls_mpi k;
    mbedtls_mpi_init(&k);
    mbedtls_mpi_lset(&k, Hb - db);
    mbedtls_mpi_mul_mpi(&k, &k, &k);
    mbedtls_mpi_mul_mpi(&L, &t, &t);
    mbedtls_mpi_sub_mpi(&L, &L, &k);
    mbedtls_mpi_free(&k);
    mbedtls_mpi_sub_int(&L, &L, Lb);
    while (mbedtls_mpi_is_prime_ext(&L, PRIMEVERIFYROUNDS, mbedtls_ctr_drbg_random, &ctr_drbg) != 0) {
        mbedtls_mpi_sub_int(&L, &L, 1);
        Lb++;
    }
    printf("# L            L= 0x%s [%ld]\n", mpistring(&L), (long)Lb);
}

// Custom Time Representation String
char DateTime[40];
char* DTString() {
    uint64_t ctime = esp_timer_get_time();
    uint64_t TPD = 86400000000;
    uint64_t TPH = TPD / 34;
    uint64_t TPM = TPH / 100;
    uint64_t TPS = TPM / 34;
    
    uint16_t Day = (uint16_t)(ctime / TPD);
    uint16_t Hour = (ctime - (TPD * Day)) / TPH;
    uint16_t Minute = ((ctime - (TPD * Day) - (TPH * Hour)) / TPM);
    uint16_t Second = ((ctime - (TPD * Day) - (TPH * Hour) - (TPM * Minute)) / TPS);

    snprintf(DateTime, sizeof(DateTime), "Day:%X %02X:%02X.%02X", Day, Hour, Minute, Second);
    return DateTime;
}

// Key Entropy and Generator Logic
static char KeyEntropy[96];
char* GenKey() {
    for (int idx = 0; idx < 89; idx += 8) {
        uint32_t r = lora_radio_random();
        KeyEntropy[idx] ^= r & 0xF;
        KeyEntropy[idx+1] ^= (r >> 4) & 0xF;
        KeyEntropy[idx+2] ^= (r >> 8) & 0xF;
        KeyEntropy[idx+3] ^= (r >> 12) & 0xF;
        KeyEntropy[idx+4] ^= (r >> 16) & 0xF;
        KeyEntropy[idx+5] ^= (r >> 20) & 0xF;
        KeyEntropy[idx+6] ^= (r >> 24) & 0xF;
        KeyEntropy[idx+7] ^= (r >> 28) & 0xF;
    }

    mpistring(&L);
    size_t len = strlen(mpibuf);
    char *NewKey = (char *)malloc(len + 1);
    for (size_t idx = 0; idx < len; idx++) {
        NewKey[idx] = (KeyEntropy[idx] % 16) + '0';
        if (NewKey[idx] > '9') {
            NewKey[idx] = NewKey[idx] - 10 - '0' + ((idx % 2 == 0) ? 'A' : 'a');
        }
    }
    NewKey[len] = '\0';
    strcpy(mpibuf, NewKey);
    free(NewKey);
    return mpibuf;
}

void SendToRadio(const char* txt) {
    lora_idle = false;
    txNumber++;
    if (txt == NULL) {
        memset(txpacket, 0, sizeof(txpacket));
        uint32_t r = lora_radio_random();
        snprintf(txpacket, sizeof(txpacket), "失%u呂%s例%u子水%s約%u", (unsigned int)txNumber, Handle, (unsigned int)r, Version, (unsigned int)rxNumber);
    } else if (txt != txpacket) {
        memset(txpacket, 0, sizeof(txpacket));
        strncpy(txpacket, txt, sizeof(txpacket) - 1);
    }
    lora_radio_send((uint8_t *)txpacket, strlen(txpacket));
    printf(": %s\n", txpacket);
    idlemod = 1;
}

// Custom command processor for serial inputs
void ProcessCmd(const char *cmd) {
    if (cmd[0] == 'a') {
        printf("# random = %lu\n", (unsigned long)lora_radio_random());
    } else if (cmd[0] == 'b') {
        printf("# time = %s\n", DTString());
    } else if (cmd[0] == 'c') {
        PrintConfig();
    } else if (cmd[0] == 'h') {
        const char *handle_val = cmd + 1;
        while (*handle_val == ' ') handle_val++;
        if (strlen(handle_val) > 1) {
            strncpy(Handle, handle_val, sizeof(Handle) - 1);
        }
        printf("# Handle: %s\n", Handle);
    } else if (cmd[0] == 'n') {
        printf("# New Key: 0x%s\n", GenKey());
    } else if (cmd[0] == 'r') {
        printf("# rxCount = %d\n", rxNumber);
    } else if (cmd[0] == 't') {
        printf("# txCount = %d\n", txNumber);
    } else if (cmd[0] == 'x') {
        SaveConfig();
    }
}

// FreeRTOS Worker Task
void app_loop_task(void *pvParameters) {
    char serial_buffer[232];
    int serial_idx = 0;

    last_tx = esp_log_timestamp();

    while (1) {
        // Handle incoming Serial logs/commands via native USB-Serial/JTAG driver
        uint8_t rx_buf[64];
        int rxBytes = usb_serial_jtag_read_bytes(rx_buf, sizeof(rx_buf), 0);
        if (rxBytes > 0) {
            for (int i = 0; i < rxBytes; i++) {
                char ch = (char)rx_buf[i];
                if (ch == '\n' || ch == '\r') {
                    serial_buffer[serial_idx] = '\0';
                    if (serial_buffer[0] == ':') {
                        ProcessCmd(serial_buffer + 1);
                    } else if (serial_idx > 0) {
                        SendToRadio(serial_buffer);
                        last_tx = esp_log_timestamp();
                    }
                    serial_idx = 0;
                } else if (serial_idx < sizeof(serial_buffer) - 1) {
                    serial_buffer[serial_idx++] = ch;
                }
            }
        }

        // Handle periodic radio transmit
        uint32_t now = esp_log_timestamp();
        if ((now - last_tx > 55000) && lora_idle) {
            SendToRadio(NULL);
            last_tx = now;
        }

        // Refresh OLED Screen
        if (ScreenOn && (now - last_refresh > 911)) {
            xSemaphoreTake(mutex, pdMS_TO_TICKS(400));
            ssd1306_clear();
            for (int j = 0; j < last_line; j++) {
                ssd1306_draw_string(j, 0, screenlines[j]);
            }
            last_refresh = now;
            xSemaphoreGive(mutex);
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

extern "C" void app_main(void) {
    // 1. Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    mutex = xSemaphoreCreateMutex();

    // Initialize USB Serial/JTAG driver for input commands
    usb_serial_jtag_driver_config_t usb_config = {};
    usb_config.rx_buffer_size = 512;
    usb_config.tx_buffer_size = 512;
    usb_serial_jtag_driver_install(&usb_config);

    // Get chip ID (efuse MAC)
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    chipid = ((uint64_t)mac[0] << 40) | ((uint64_t)mac[1] << 32) | ((uint64_t)mac[2] << 24) |
             ((uint64_t)mac[3] << 16) | ((uint64_t)mac[4] << 8) | mac[5];

    // 2. Initialize SPIFFS File System
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true
    };
    ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mount SPIFFS (%s)", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "SPIFFS mounted successfully.");
    }

    // Load handle configuration from SPIFFS
    FILE *cf = fopen(CONFIG_FILE_PATH, "r");
    if (cf) {
        char line[64];
        if (fgets(line, sizeof(line), cf)) {
            if (line[0] == 'h') {
                char *val = line + 1;
                while (*val == ' ') val++;
                // Strip newline
                size_t len = strlen(val);
                if (len > 0 && val[len - 1] == '\n') val[len - 1] = '\0';
                strncpy(Handle, val, sizeof(Handle) - 1);
            }
        }
        fclose(cf);
    }
    SaveConfig();

    // 3. Initialize SSD1306 OLED Screen display
    ESP_ERROR_CHECK(ssd1306_init(SDA_OLED_PIN, SCL_OLED_PIN, RST_OLED_PIN, VEXT_PIN));
    ssd1306_clear();

    // Run Cryptographic calculations
    MathInit();

    // 4. Initialize SX1262 LoRa Radio
    RadioEvents_t events = {
        .TxDone = OnTxDone,
        .TxTimeout = OnTxTimeout,
        .RxDone = OnRxDone,
        .RxTimeout = NULL
    };
    ESP_ERROR_CHECK(lora_radio_init(&events));
    
    // Default config values matching Heltec/version3.ino settings
    lora_radio_set_channel(954114361); // 954.114 MHz
    lora_radio_set_tx_config(10, 0, 15, 3, 9); // +10dBm, SF15, CR4/7, Preamble 9
    lora_radio_set_rx_config(0, 15, 3, 9);

    ESP_LOGI(TAG, "Starting main system worker task...");
    xTaskCreate(app_loop_task, "app_loop_task", 1024 * 4, NULL, 5, NULL);
}
