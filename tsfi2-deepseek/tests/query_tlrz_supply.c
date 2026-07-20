#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "tsfi_hogan.h"

extern bool resolve_token_alias(const char *symbol_or_name, char *out_address, size_t out_max);
extern bool tsfi_pulse_rpc_call(const char *address, const char *data, char *result, size_t result_max);

double hex_to_double(const char *hex) {
    if (strncmp(hex, "0x", 2) == 0 || strncmp(hex, "0X", 2) == 0) {
        hex += 2;
    }
    while (*hex == '0') hex++;
    
    double val = 0.0;
    while (*hex) {
        char c = *hex++;
        int digit = 0;
        if (c >= '0' && c <= '9') digit = c - '0';
        else if (c >= 'a' && c <= 'f') digit = c - 'a' + 10;
        else if (c >= 'A' && c <= 'F') digit = c - 'A' + 10;
        else break;
        val = val * 16.0 + digit;
    }
    return val;
}

void format_commas(double val, char *out, size_t max_len) {
    char temp[128];
    // Format full value with 4 decimal places to string
    snprintf(temp, sizeof(temp), "%.4f", val);
    
    // Find decimal point
    char *dot = strchr(temp, '.');
    int int_len = dot ? (int)(dot - temp) : (int)strlen(temp);
    
    int commas = (int_len - 1) / 3;
    int new_int_len = int_len + commas;
    
    if (new_int_len + 5 >= (int)max_len) return;
    
    int src = int_len - 1;
    int dst = new_int_len - 1;
    int count = 0;
    
    while (src >= 0) {
        out[dst--] = temp[src--];
        count++;
        if (count == 3 && src >= 0) {
            out[dst--] = ',';
            count = 0;
        }
    }
    
    out[new_int_len] = '\0';
    if (dot) {
        strncat(out, dot, max_len - strlen(out) - 1);
    }
}

int main(void) {
    char tlz_addr[128] = {0};
    if (!resolve_token_alias("TLRz", tlz_addr, sizeof(tlz_addr))) {
        printf("[ERROR] Failed to resolve TLRz alias.\n");
        return 1;
    }
    
    char total_supply_hex[256] = {0};
    char decimals_hex[256] = {0};
    
    // Query totalSupply() -> 0x18160ddd
    if (!tsfi_pulse_rpc_call(tlz_addr, "0x18160ddd", total_supply_hex, sizeof(total_supply_hex))) {
        printf("[ERROR] Failed to query total supply.\n");
        return 1;
    }
    
    // Query decimals() -> 0x313ce567
    if (!tsfi_pulse_rpc_call(tlz_addr, "0x313ce567", decimals_hex, sizeof(decimals_hex))) {
        printf("[ERROR] Failed to query decimals.\n");
        return 1;
    }
    
    double total_supply_val = hex_to_double(total_supply_hex);
    double decimals_val = hex_to_double(decimals_hex);
    double true_supply = total_supply_val / pow(10.0, decimals_val);
    
    char formatted_supply[256] = {0};
    format_commas(true_supply, formatted_supply, sizeof(formatted_supply));
    
    printf("[SUCCESS] Raw TLRz Total Supply : %s\n", total_supply_hex);
    printf("[SUCCESS] Raw TLRz Decimals     : %s (%.0f)\n", decimals_hex, decimals_val);
    printf("[SUCCESS] Formatted True Supply : %s TLRz\n", formatted_supply);
    
    return 0;
}
