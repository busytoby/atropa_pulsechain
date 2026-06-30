#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdarg.h>


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
#include "driver/gpio.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_http_server.h"


#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/bignum.h"
#include "mbedtls/sha256.h"
#include "mbedtls/ecdsa.h"
#include "esp_random.h"

#include "ssd1306_i2c.h"
#include "lora_sx1262.h"

static TaskHandle_t app_task_handle = NULL;

void cli_printf(const char *format, ...);
void ProcessCmd(const char *cmd);
#define printf(...) cli_printf(__VA_ARGS__)


static void IRAM_ATTR lora_dio1_isr_handler(void *arg) {
    if (app_task_handle != NULL) {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        vTaskNotifyGiveFromISR(app_task_handle, &xHigherPriorityTaskWoken);
        if (xHigherPriorityTaskWoken) {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }
}


static const char *TAG = "MainApp";
static SemaphoreHandle_t mutex;

// Console security variables
static uint8_t current_challenge[32];
static bool console_locked = true;

// Master Public Key coordinates (secp256k1) - matches local developer private key
static const uint8_t MASTER_PUBKEY_X[32] = {
    0xba, 0x57, 0x34, 0xd8, 0xf7, 0x09, 0x17, 0x19, 0x47, 0x1e, 0x7f, 0x7e, 0xd6, 0xb9, 0xdf, 0x17,
    0x0d, 0xc7, 0x0c, 0xc6, 0x61, 0xca, 0x05, 0xe6, 0x88, 0x60, 0x1a, 0xd9, 0x84, 0xf0, 0x68, 0xb0
};
static const uint8_t MASTER_PUBKEY_Y[32] = {
    0xd6, 0x73, 0x51, 0xe5, 0xf0, 0x60, 0x73, 0x09, 0x24, 0x99, 0x33, 0x6a, 0xb0, 0x83, 0x9e, 0xf8,
    0xa5, 0x21, 0xaf, 0xd3, 0x34, 0xe5, 0x38, 0x07, 0x20, 0x5f, 0xa2, 0xf0, 0x8e, 0xec, 0x74, 0xf4
};


void GenerateChallenge() {
    esp_fill_random(current_challenge, sizeof(current_challenge));
    printf("# Challenge: ");
    for (int idx = 0; idx < 32; idx++) {
        printf("%02X", current_challenge[idx]);
    }
    printf("\n");
}

bool VerifySignature(const uint8_t *r_bin, const uint8_t *s_bin) {
    mbedtls_ecp_group grp;
    mbedtls_ecp_point Q;

    mbedtls_ecp_group_init(&grp);
    mbedtls_ecp_point_init(&Q);

    int ret = mbedtls_ecp_group_load(&grp, MBEDTLS_ECP_DP_SECP256K1);
    if (ret != 0) {
        ESP_LOGE(TAG, "Failed to load secp256k1 group: -0x%04X", -ret);
        mbedtls_ecp_group_free(&grp);
        mbedtls_ecp_point_free(&Q);
        return false;
    }

    // Parse the uncompressed public key: 0x04 prefix + 32 bytes X + 32 bytes Y
    uint8_t pubkey_bin[65];
    pubkey_bin[0] = 0x04;
    memcpy(pubkey_bin + 1, MASTER_PUBKEY_X, 32);
    memcpy(pubkey_bin + 33, MASTER_PUBKEY_Y, 32);

    ret = mbedtls_ecp_point_read_binary(&grp, &Q, pubkey_bin, sizeof(pubkey_bin));
    if (ret != 0) {
        ESP_LOGE(TAG, "Failed to parse public key point: -0x%04X", -ret);
        mbedtls_ecp_group_free(&grp);
        mbedtls_ecp_point_free(&Q);
        return false;
    }

    // Hash current challenge using SHA-256
    uint8_t hash[32];
    ret = mbedtls_sha256(current_challenge, 32, hash, 0);
    if (ret != 0) {
        ESP_LOGE(TAG, "SHA256 failed: -0x%04X", -ret);
        mbedtls_ecp_group_free(&grp);
        mbedtls_ecp_point_free(&Q);
        return false;
    }

    printf("# C++ Hash: ");
    for (int idx = 0; idx < 32; idx++) printf("%02X", hash[idx]);
    printf("\n");

    // Validate public key
    ret = mbedtls_ecp_check_pubkey(&grp, &Q);
    if (ret != 0) {
        printf("# Invalid public key: -0x%04X\n", -ret);
    }

    mbedtls_mpi r, s;
    mbedtls_mpi_init(&r);
    mbedtls_mpi_init(&s);
    mbedtls_mpi_read_binary(&r, r_bin, 32);
    mbedtls_mpi_read_binary(&s, s_bin, 32);

    ret = mbedtls_ecdsa_verify(&grp, hash, 32, &Q, &r, &s);
    if (ret != 0) {
        printf("# Verify error: -0x%04X\n", -ret);
    }

    mbedtls_mpi_free(&r);
    mbedtls_mpi_free(&s);
    mbedtls_ecp_group_free(&grp);
    mbedtls_ecp_point_free(&Q);

    return (ret == 0);
}

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
    if (size >= BUFFER_SIZE) size = BUFFER_SIZE - 1;
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
    strncpy(DysnomiaPrime, APOGEE, sizeof(DysnomiaPrime) - 1);
    DysnomiaPrime[sizeof(DysnomiaPrime) - 1] = '\0';
    strncat(DysnomiaPrime, APEX, sizeof(DysnomiaPrime) - strlen(DysnomiaPrime) - 1);
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
    strncpy(mpibuf, NewKey, sizeof(mpibuf) - 1);
    mpibuf[sizeof(mpibuf) - 1] = '\0';
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

// Helper to convert hex character to value
static inline uint8_t hex_to_val(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return 0;
}

// Command Handler Functions
static void handle_auth(const char *args) {
    const char *p = args;
    while (*p == ' ') p++;
    uint8_t r_bin[32];
    uint8_t s_bin[32];
    bool parse_ok = true;
    for (int idx = 0; idx < 32; idx++) {
        if (!isxdigit((unsigned char)p[0]) || !isxdigit((unsigned char)p[1])) {
            parse_ok = false;
            break;
        }
        r_bin[idx] = (hex_to_val(p[0]) << 4) | hex_to_val(p[1]);
        p += 2;
    }
    while (*p == ' ') p++;
    for (int idx = 0; idx < 32; idx++) {
        if (!isxdigit((unsigned char)p[0]) || !isxdigit((unsigned char)p[1])) {
            parse_ok = false;
            break;
        }
        s_bin[idx] = (hex_to_val(p[0]) << 4) | hex_to_val(p[1]);
        p += 2;
    }
    printf("# parsed R: ");
    for (int idx = 0; idx < 32; idx++) printf("%02x", r_bin[idx]);
    printf("\n");
    printf("# parsed S: ");
    for (int idx = 0; idx < 32; idx++) printf("%02x", s_bin[idx]);
    printf("\n");

    if (parse_ok && VerifySignature(r_bin, s_bin)) {
        console_locked = false;
        printf("# AUTH SUCCESS\n");
    } else {
        printf("# AUTH FAILED\n");
        GenerateChallenge();
    }
}

static void handle_random(const char *args) {
    printf("# random = %lu\n", (unsigned long)lora_radio_random());
}

static void handle_time(const char *args) {
    printf("# time = %s\n", DTString());
}

static void handle_config(const char *args) {
    PrintConfig();
}

static void handle_set_handle(const char *args) {
    const char *handle_val = args;
    while (*handle_val == ' ') handle_val++;
    if (strlen(handle_val) > 1) {
        strncpy(Handle, handle_val, sizeof(Handle) - 1);
    }
    printf("# Handle: %s\n", Handle);
}

static void handle_new_key(const char *args) {
    printf("# New Key: 0x%s\n", GenKey());
}

static void handle_rx_count(const char *args) {
    printf("# rxCount = %d\n", rxNumber);
}

static void handle_tx_count(const char *args) {
    printf("# txCount = %d\n", txNumber);
}

static void handle_lock(const char *args) {
    console_locked = true;
    printf("# Console locked.\n");
    GenerateChallenge();
}

static void handle_save(const char *args) {
    SaveConfig();
}

static char latest_time_str[16] = "N/A";
static char latest_lat_str[16] = "N/A";
static char latest_ns[2] = "";
static char latest_lon_str[16] = "N/A";
static char latest_ew[2] = "";
static char latest_fix[2] = "0";
static char latest_sats[4] = "00";
static char latest_alt_str[16] = "0.0";
static bool latest_has_fix_data = false;

#define MAX_WS_CLIENTS 4
static int ws_client_fds[MAX_WS_CLIENTS] = {-1, -1, -1, -1};
static httpd_handle_t ws_server_handle = NULL;

void broadcast_ws_message(const char *msg) {
    if (!ws_server_handle) return;
    
    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.payload = (uint8_t*)msg;
    ws_pkt.len = strlen(msg);
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
    
    for (int i = 0; i < MAX_WS_CLIENTS; i++) {
        int fd = ws_client_fds[i];
        if (fd != -1) {
            esp_err_t ret = httpd_ws_send_frame_async(ws_server_handle, fd, &ws_pkt);
            if (ret != ESP_OK) {
                ws_client_fds[i] = -1; // Remove dead socket connection
            }
        }
    }
}

#undef printf
void cli_printf(const char *format, ...) {
    char buffer[256];
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    broadcast_ws_message(buffer);
}
#define printf(...) cli_printf(__VA_ARGS__)

static esp_err_t ws_handler(httpd_req_t *req) {
    if (req->method == HTTP_GET) {
        int fd = httpd_req_to_sockfd(req);
        // Store client descriptor
        for (int i = 0; i < MAX_WS_CLIENTS; i++) {
            if (ws_client_fds[i] == fd) break;
            if (ws_client_fds[i] == -1) {
                ws_client_fds[i] = fd;
                break;
            }
        }
        return ESP_OK;
    }
    
    httpd_ws_frame_t ws_pkt;
    uint8_t *buf = NULL;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
    esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 0);
    if (ret != ESP_OK) return ret;
    
    if (ws_pkt.len > 0) {
        buf = (uint8_t*)calloc(1, ws_pkt.len + 1);
        if (buf == NULL) return ESP_ERR_NO_MEM;
        ws_pkt.payload = buf;
        ret = httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len);
        if (ret != ESP_OK) {
            free(buf);
            return ret;
        }
        
        char *payload_str = (char*)ws_pkt.payload;
        if (payload_str[0] == ':') {
            // Echo command
            broadcast_ws_message(payload_str);
            // Process command (skip leading ':')
            ProcessCmd(payload_str + 1);
        } else {
            // Respond/echo back
            broadcast_ws_message(payload_str);
        }
        free(buf);
    }
    return ESP_OK;
}

static void parse_gga(const char *line) {
    char time_str[16] = "N/A";
    char lat_str[16] = "N/A";
    char ns[2] = "";
    char lon_str[16] = "N/A";
    char ew[2] = "";
    char fix[2] = "0";
    char sats[4] = "00";
    char alt_str[16] = "0.0";

    int field_idx = 0;
    const char *p = line;
    while (*p) {
        const char *next = strchr(p, ',');
        int len = next ? (next - p) : strlen(p);
        
        if (field_idx == 1 && len > 0) { strncpy(time_str, p, len < 15 ? len : 15); time_str[len < 15 ? len : 15] = '\0'; }
        else if (field_idx == 2 && len > 0) { strncpy(lat_str, p, len < 15 ? len : 15); lat_str[len < 15 ? len : 15] = '\0'; }
        else if (field_idx == 3 && len > 0) { ns[0] = *p; ns[1] = '\0'; }
        else if (field_idx == 4 && len > 0) { strncpy(lon_str, p, len < 15 ? len : 15); lon_str[len < 15 ? len : 15] = '\0'; }
        else if (field_idx == 5 && len > 0) { ew[0] = *p; ew[1] = '\0'; }
        else if (field_idx == 6 && len > 0) { fix[0] = *p; fix[1] = '\0'; }
        else if (field_idx == 7 && len > 0) { strncpy(sats, p, len < 3 ? len : 3); sats[len < 3 ? len : 3] = '\0'; }
        else if (field_idx == 9 && len > 0) { strncpy(alt_str, p, len < 15 ? len : 15); alt_str[len < 15 ? len : 15] = '\0'; }

        field_idx++;
        if (!next) break;
        p = next + 1;
    }

    if (field_idx >= 10) {
        strncpy(latest_time_str, time_str, sizeof(latest_time_str));
        strncpy(latest_lat_str, lat_str, sizeof(latest_lat_str));
        strncpy(latest_ns, ns, sizeof(latest_ns));
        strncpy(latest_lon_str, lon_str, sizeof(latest_lon_str));
        strncpy(latest_ew, ew, sizeof(latest_ew));
        strncpy(latest_fix, fix, sizeof(latest_fix));
        strncpy(latest_sats, sats, sizeof(latest_sats));
        strncpy(latest_alt_str, alt_str, sizeof(latest_alt_str));
        latest_has_fix_data = true;

        // Broadcast parsed updates to WebSocket clients as JSON
        char status_json[256];
        snprintf(status_json, sizeof(status_json),
                 "{\"type\":\"status\",\"time\":\"%s\",\"lat\":\"%s\",\"ns\":\"%s\",\"lon\":\"%s\",\"ew\":\"%s\",\"sats\":\"%s\",\"alt\":\"%s\",\"fix\":\"%s\"}",
                 latest_time_str, latest_lat_str, latest_ns, latest_lon_str, latest_ew, latest_sats, latest_alt_str, latest_fix);
        broadcast_ws_message(status_json);
    }
}

static void gps_background_task(void *pvParameters) {
    // 1. Enable GNSS power (Active HIGH Vext)
    gpio_reset_pin((gpio_num_t)3);
    gpio_set_direction((gpio_num_t)3, GPIO_MODE_OUTPUT);
    gpio_set_level((gpio_num_t)3, 1); // Power ON

    // 2. Release GNSS reset (Active LOW reset, so HIGH to run)
    gpio_reset_pin((gpio_num_t)35);
    gpio_set_direction((gpio_num_t)35, GPIO_MODE_OUTPUT);
    gpio_set_level((gpio_num_t)35, 1); // Release reset

    vTaskDelay(pdMS_TO_TICKS(1000)); // wait for GNSS module to boot

    // Configure UART2 at 115200 baud (UC6580 standard)
    uart_config_t uart_config = {};
    uart_config.baud_rate = 115200;
    uart_config.data_bits = UART_DATA_8_BITS;
    uart_config.parity = UART_PARITY_DISABLE;
    uart_config.stop_bits = UART_STOP_BITS_1;
    uart_config.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
    uart_config.source_clk = UART_SCLK_DEFAULT;

    // Configure GPIO 33 with internal pull-up (GNSS TX)
    gpio_reset_pin((gpio_num_t)33);
    gpio_set_direction((gpio_num_t)33, GPIO_MODE_INPUT);
    gpio_set_pull_mode((gpio_num_t)33, GPIO_PULLUP_ONLY);

    if (uart_driver_install(UART_NUM_2, 1024, 0, 0, NULL, 0) != ESP_OK) {
        ESP_LOGE("GPS_Task", "Failed to install UART driver for GPS.");
        vTaskDelete(NULL);
        return;
    }
    uart_param_config(UART_NUM_2, &uart_config);
    // UART2 RX = GPIO 33 (GNSS TX), TX = GPIO 34 (GNSS RX)
    uart_set_pin(UART_NUM_2, 34, 33, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_flush(UART_NUM_2);

    ESP_LOGI("GPS_Task", "GPS Background Reader started. Listening on GPIO 33 (RX) / 34 (TX)...");

    char line_buf[128];
    int line_idx = 0;

    while (1) {
        uint8_t ch;
        int len = uart_read_bytes(UART_NUM_2, &ch, 1, pdMS_TO_TICKS(100));
        if (len > 0) {
            if (ch == '\n' || ch == '\r') {
                if (line_idx > 0) {
                    line_buf[line_idx] = '\0';
                    
                    // Broadcast raw NMEA sentence to all WebSocket clients
                    broadcast_ws_message(line_buf);
                    
                    // Parse if GGA sentence
                    if (strncmp(line_buf, "$GNGGA", 6) == 0 || strncmp(line_buf, "$GPGGA", 6) == 0) {
                        parse_gga(line_buf);
                    }
                    line_idx = 0;
                }
            } else if (line_idx < sizeof(line_buf) - 1) {
                line_buf[line_idx++] = (char)ch;
            }
        }
    }
}

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        printf("# WiFi Disconnected from AP. Reconnecting...\n");
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        char ip_str[32];
        esp_ip4addr_ntoa(&event->ip_info.ip, ip_str, sizeof(ip_str));
        printf("{\"type\":\"wifi_connected\",\"ip\":\"%s\"}\n", ip_str);
    }
}

static void wifi_init_apsta(void) {
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    
    esp_netif_create_default_wifi_ap();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));

    wifi_config_t wifi_config = {};
    strncpy((char*)wifi_config.ap.ssid, "Tracker-GNSS", sizeof(wifi_config.ap.ssid) - 1);
    wifi_config.ap.ssid_len = strlen("Tracker-GNSS");
    strncpy((char*)wifi_config.ap.password, "12345678", sizeof(wifi_config.ap.password) - 1);
    wifi_config.ap.channel = 1;
    wifi_config.ap.authmode = WIFI_AUTH_WPA2_PSK;
    wifi_config.ap.max_connection = 4;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI("WiFi", "wifi_init_apsta finished. SSID: %s password: %s", "Tracker-GNSS", "12345678");
}

static void handle_wifi_scan(const char *args) {
    wifi_scan_config_t scan_config = {};
    scan_config.show_hidden = true;

    printf("# Scanning Wi-Fi networks...\n");
    esp_err_t ret = esp_wifi_scan_start(&scan_config, true); // true = block
    if (ret != ESP_OK) {
        printf("{\"type\":\"wifi_scan_err\",\"msg\":\"Scan failed\"}\n");
        return;
    }

    uint16_t ap_count = 0;
    esp_wifi_scan_get_ap_num(&ap_count);
    if (ap_count == 0) {
        printf("{\"type\":\"wifi_scan_results\",\"networks\":[]}\n");
        return;
    }

    if (ap_count > 20) ap_count = 20; // Limit APs count

    wifi_ap_record_t *ap_records = (wifi_ap_record_t*)calloc(ap_count, sizeof(wifi_ap_record_t));
    if (!ap_records) {
        printf("{\"type\":\"wifi_scan_err\",\"msg\":\"Out of memory\"}\n");
        return;
    }

    if (esp_wifi_scan_get_ap_records(&ap_count, ap_records) == ESP_OK) {
        char json_buf[1024];
        int offset = snprintf(json_buf, sizeof(json_buf), "{\"type\":\"wifi_scan_results\",\"networks\":[");
        for (int i = 0; i < ap_count; i++) {
            offset += snprintf(json_buf + offset, sizeof(json_buf) - offset,
                               "{\"ssid\":\"%s\",\"rssi\":%d,\"auth\":%d}%s",
                               (char*)ap_records[i].ssid,
                               ap_records[i].rssi,
                               ap_records[i].authmode,
                               (i == ap_count - 1) ? "" : ",");
            if (offset >= sizeof(json_buf) - 64) {
                break;
            }
        }
        snprintf(json_buf + offset, sizeof(json_buf) - offset, "]}");
        printf("%s\n", json_buf);
    }
    free(ap_records);
}

static void handle_wifi_connect(const char *args) {
    const char *ssid_start = args;
    while (*ssid_start == ' ') ssid_start++;
    if (*ssid_start == '\0') {
        printf("# WiFi Connect failed: Missing SSID and password\n");
        return;
    }

    char ssid[32] = {};
    char pass[64] = {};

    const char *ssid_end = strchr(ssid_start, ' ');
    if (ssid_end) {
        int ssid_len = ssid_end - ssid_start;
        if (ssid_len > 31) ssid_len = 31;
        strncpy(ssid, ssid_start, ssid_len);

        const char *pass_start = ssid_end + 1;
        while (*pass_start == ' ') pass_start++;
        strncpy(pass, pass_start, sizeof(pass) - 1);
    } else {
        strncpy(ssid, ssid_start, sizeof(ssid) - 1);
    }

    printf("# Connecting to WiFi SSID: '%s'...\n", ssid);

    wifi_config_t wifi_config = {};
    strncpy((char*)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid) - 1);
    strncpy((char*)wifi_config.sta.password, pass, sizeof(wifi_config.sta.password) - 1);
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

    esp_wifi_disconnect();
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    esp_wifi_connect();
}


static httpd_handle_t start_web_server(void) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 80;
    config.ctrl_port = 32768;

    ESP_LOGI("WebServer", "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&ws_server_handle, &config) == ESP_OK) {
        httpd_uri_t ws = {};


        ws.uri        = "/ws";
        ws.method     = HTTP_GET;
        ws.handler    = ws_handler;
        ws.user_ctx   = NULL;
        ws.is_websocket = true;

        httpd_register_uri_handler(ws_server_handle, &ws);
        return ws_server_handle;
    }
    return NULL;
}


static void handle_gps(const char *args) {
    printf("# GPS Module Status (Live Background Reader):\n");
    printf("# SSID: Tracker-GNSS Password: 12345678 wss://192.168.4.1/ws\n");
    if (latest_has_fix_data) {
        printf("# Time:       %s UTC\n", latest_time_str);
        printf("# Position:   Lat %s %s, Lon %s %s\n", latest_lat_str, latest_ns, latest_lon_str, latest_ew);
        printf("# Satellites: %s\n", latest_sats);
        printf("# Altitude:   %s M\n", latest_alt_str);
        printf("# Fix Status: %s\n", latest_fix[0] == '0' ? "No Fix" : "Fix OK");
    } else {
        printf("# No GPS data received yet or GPS is still acquiring satellites.\n");
    }
}

// Registry of available CLI commands
struct ConsoleCommand {
    const char *name;
    void (*handler)(const char *args);
};

static const ConsoleCommand console_commands[] = {
    {"auth", handle_auth},
    {"a",    handle_random},
    {"b",    handle_time},
    {"c",    handle_config},
    {"h",    handle_set_handle},
    {"n",    handle_new_key},
    {"r",    handle_rx_count},
    {"t",    handle_tx_count},
    {"lock", handle_lock},
    {"x",    handle_save},
    {"gps",  handle_gps},
    {"wifi_scan", handle_wifi_scan},
    {"wifi_connect", handle_wifi_connect},
};

// Custom command processor for serial inputs
void ProcessCmd(const char *cmd) {
    while (*cmd == ' ') cmd++;
    if (*cmd == '\0') return;

    // Parse command name
    const char *p = cmd;
    while (*p && *p != ' ') p++;
    size_t cmd_len = p - cmd;

    char cmd_name[32];
    if (cmd_len >= sizeof(cmd_name)) cmd_len = sizeof(cmd_name) - 1;
    strncpy(cmd_name, cmd, cmd_len);
    cmd_name[cmd_len] = '\0';

    const char *args = p;
    while (*args == ' ') args++;

    // Security Gate: Locked console enforces signature auth
    if (console_locked) {
        if (strcmp(cmd_name, "auth") == 0) {
            handle_auth(args);
        } else {
            printf("# LOCKED: Signature verification required. Run :auth <r_hex> <s_hex>\n");
        }
        return;
    }

    // Lookup and execute matching command
    for (size_t i = 0; i < sizeof(console_commands) / sizeof(console_commands[0]); i++) {
        if (strcmp(cmd_name, console_commands[i].name) == 0) {
            console_commands[i].handler(args);
            return;
        }
    }
    printf("# Unknown command: %s\n", cmd_name);
}

// FreeRTOS Worker Task
void app_loop_task(void *pvParameters) {
    char serial_buffer[232];
    int serial_idx = 0;

    last_tx = esp_log_timestamp();

    while (1) {
        // Wait for DIO1 interrupt notification or timeout (10ms)
        ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(10));

        // Process interrupt if DIO1 is active (high)
        if (gpio_get_level((gpio_num_t)LORA_DIO1_PIN) == 1) {
            lora_radio_irq_process();
        }

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
    lora_radio_set_tx_config(2, 0, 15, 3, 9); // +2dBm, SF15, CR4/7, Preamble 9
    lora_radio_set_rx_config(0, 15, 3, 9);

    GenerateChallenge();

    ESP_LOGI(TAG, "Starting main system worker task...");
    xTaskCreate(app_loop_task, "app_loop_task", 1024 * 4, NULL, 5, &app_task_handle);

    // Initialize Wi-Fi AP and WebSocket Web Server in dual AP+STA mode
    wifi_init_apsta();
    start_web_server();


    // Spawn GPS Background reader task
    xTaskCreate(gps_background_task, "gps_background_task", 1024 * 4, NULL, 4, NULL);

    // Install GPIO ISR service and attach handler for DIO1 interrupt

    gpio_install_isr_service(0);
    gpio_config_t dio1_conf = {};
    dio1_conf.intr_type = GPIO_INTR_POSEDGE;
    dio1_conf.mode = GPIO_MODE_INPUT;
    dio1_conf.pin_bit_mask = (1ULL << LORA_DIO1_PIN);
    dio1_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    dio1_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&dio1_conf);
    gpio_isr_handler_add((gpio_num_t)LORA_DIO1_PIN, lora_dio1_isr_handler, NULL);
}
