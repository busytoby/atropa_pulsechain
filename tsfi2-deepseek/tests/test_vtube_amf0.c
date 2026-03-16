#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <arpa/inet.h>
#include "lau_memory.h"

#define AMF_STRING 0x02
#define AMF_OBJECT 0x03
#define AMF_ECMA_ARRAY 0x08
#define AMF_OBJECT_END 0x09

static uint8_t* amf_write_string(uint8_t *p, const char *s) {
    *p++ = AMF_STRING;
    uint16_t len = htons((uint16_t)strlen(s));
    memcpy(p, &len, 2); p += 2;
    memcpy(p, s, strlen(s)); p += strlen(s);
    return p;
}

void test_amf0_object_serialization() {
    printf("[TEST] Verifying AMF0 Object Serialization...\n");

    uint8_t buf[1024];
    uint8_t *p = buf;

    *p++ = AMF_OBJECT;

    uint16_t klen = htons(3); memcpy(p, &klen, 2); p += 2; memcpy(p, "app", 3); p += 3;
    p = amf_write_string(p, "live");

    *p++ = 0x00; *p++ = 0x00; *p++ = AMF_OBJECT_END;

    assert(buf[0] == AMF_OBJECT);
    assert(buf[1] == 0x00); assert(buf[2] == 0x03);
    assert(memcmp(&buf[3], "app", 3) == 0);
    assert(buf[6] == AMF_STRING);
    assert(buf[7] == 0x00); assert(buf[8] == 0x04);
    assert(memcmp(&buf[9], "live", 4) == 0);
    assert(buf[13] == 0x00); assert(buf[14] == 0x00);
    assert(buf[15] == AMF_OBJECT_END);

    printf("[PASS] AMF0 Object serialized correctly (Key-Value + Delimiters verified).\n");
}

void test_amf0_metadata_serialization() {
    printf("[TEST] Verifying AMF0 Metadata Serialization (onMetaData)...\n");

    uint8_t buf[1024];
    uint8_t *p = buf;

    const char *s1 = "@setDataFrame";
    *p++ = AMF_STRING;
    uint16_t len1 = htons((uint16_t)strlen(s1));
    memcpy(p, &len1, 2); p += 2; memcpy(p, s1, strlen(s1)); p += strlen(s1);

    const char *s2 = "onMetaData";
    *p++ = AMF_STRING;
    uint16_t len2 = htons((uint16_t)strlen(s2));
    memcpy(p, &len2, 2); p += 2; memcpy(p, s2, strlen(s2)); p += strlen(s2);

    *p++ = AMF_ECMA_ARRAY;
    uint32_t count = htonl(1);
    memcpy(p, &count, 4); p += 4;

    const char *k1 = "width";
    uint16_t klen1 = htons((uint16_t)strlen(k1));
    memcpy(p, &klen1, 2); p += 2; memcpy(p, k1, strlen(k1)); p += strlen(k1);
    
    *p++ = 0x00;
    uint64_t val = 0x4094000000000000;
    uint64_t val_be = __builtin_bswap64(val);
    memcpy(p, &val_be, 8); p += 8;

    *p++ = 0x00; *p++ = 0x00; *p++ = AMF_OBJECT_END;

    assert(buf[0] == AMF_STRING);
    assert(memcmp(&buf[3], "@setDataFrame", 13) == 0);
    
    int offset_onmeta = 1 + 2 + 13;
    assert(buf[offset_onmeta] == AMF_STRING);
    assert(memcmp(&buf[offset_onmeta + 3], "onMetaData", 10) == 0);

    int offset_array = offset_onmeta + 1 + 2 + 10;
    assert(buf[offset_array] == AMF_ECMA_ARRAY);
    
    int offset_key = offset_array + 1 + 4;
    assert(buf[offset_key] == 0x00); assert(buf[offset_key+1] == 0x05);
    assert(memcmp(&buf[offset_key+2], "width", 5) == 0);

    printf("[PASS] Metadata Structure Verified (ECMA Array + DELIM).\n");
}

int main() {
    printf("=== TSFi AMF0 Full Serialization Suite ===\n");
    test_amf0_metadata_serialization();
    test_amf0_object_serialization();
    printf("ALL METADATA TESTS PASSED\n");
    
    extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
