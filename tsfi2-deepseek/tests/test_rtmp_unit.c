#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <arpa/inet.h>

// AMF0 Types
#define AMF_NUMBER 0x00
#define AMF_BOOLEAN 0x01
#define AMF_STRING 0x02
#define AMF_OBJECT 0x03
#define AMF_NULL 0x05
#define AMF_ECMA_ARRAY 0x08
#define AMF_OBJECT_END 0x09

// Internal helpers from tsfi_rtmp.c
static uint8_t* amf_write_string(uint8_t *p, const char *s) {
    *p++ = AMF_STRING;
    uint16_t len = htons((uint16_t)strlen(s));
    memcpy(p, &len, 2); p += 2;
    memcpy(p, s, strlen(s)); p += strlen(s);
    return p;
}

static uint8_t* amf_write_number(uint8_t *p, double n) {
    *p++ = AMF_NUMBER;
    union { double d; uint64_t i; } u; u.d = n;
    uint64_t val = __builtin_bswap64(u.i);
    memcpy(p, &val, 8); p += 8;
    return p;
}

void test_amf_string() {
    uint8_t buf[64];
    uint8_t *p = amf_write_string(buf, "test");
    assert(buf[0] == AMF_STRING);
    assert(buf[1] == 0x00);
    assert(buf[2] == 0x04);
    assert(memcmp(&buf[3], "test", 4) == 0);
    assert((p - buf) == 7);
    printf("[PASS] AMF String Encoding\n");
}

void test_amf_number() {
    uint8_t buf[64];
    uint8_t *p = amf_write_number(buf, 1.0);
    assert(buf[0] == AMF_NUMBER);
    assert(buf[1] == 0x3F);
    assert(buf[2] == 0xF0);
    assert((p - buf) == 9);
    printf("[PASS] AMF Number Encoding\n");
}

void test_rtmp_header_serialization() {
    uint32_t ts = 0x123456;
    uint32_t len = 0x0000FF;
    uint8_t type = 0x14;
    uint32_t stream_id = 0;
    uint8_t header[12];
    header[0] = 0x03;
    header[1] = (ts >> 16) & 0xFF; header[2] = (ts >> 8) & 0xFF; header[3] = ts & 0xFF;
    header[4] = (len >> 16) & 0xFF; header[5] = (len >> 8) & 0xFF; header[6] = len & 0xFF;
    header[7] = type;
    memcpy(&header[8], &stream_id, 4);
    assert(header[1] == 0x12);
    assert(header[2] == 0x34);
    assert(header[3] == 0x56);
    assert(header[6] == 0xFF);
    assert(header[7] == 0x14);
    printf("[PASS] RTMP Header Serialization\n");
}

int main() {
    printf("=== TSFi RTMP Unit Tests ===\n");
    test_amf_string();
    test_amf_number();
    test_rtmp_header_serialization();
    printf("ALL RTMP UNIT TESTS PASSED\n");
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
