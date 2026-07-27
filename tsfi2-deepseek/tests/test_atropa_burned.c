#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <strings.h>
#include "tsfi_pulsechain_rpc.h"

double parse_hex_double(const char *hex) {
    if (!hex) return 0.0;
    const char *ptr = hex;
    if (ptr[0] == '0' && (ptr[1] == 'x' || ptr[1] == 'X')) ptr += 2;
    double val = 0.0;
    while (*ptr) {
        char c = *ptr++;
        int digit = 0;
        if (c >= '0' && c <= '9') digit = c - '0';
        else if (c >= 'a' && c <= 'f') digit = c - 'a' + 10;
        else if (c >= 'A' && c <= 'F') digit = c - 'A' + 10;
        else break;
        val = val * 16.0 + digit;
    }
    return val;
}

const char *stable_pools[] = {
    "0x5ef7aac0de4f2012cb36730da140025b113fada4", // ATROPA / DAI
    "0xc636bfe0bae34824380b4e26bc34e4614e55e483"  // ATROPA / USDC
};

const char *stable_tokens[] = {
    "0x6b175474e89094c44da98b954eedeac495271d0f", // DAI (18 dec)
    "0xa0b86991c6218b36c1d19d4a2e9eb0ce3606eb48"  // USDC (6 dec)
};

const char *pool_names[] = {
    "ATROPA / DAI",
    "ATROPA / USDC"
};

double stable_decimals[] = {
    1e18,
    1e6
};

int main(void) {
    printf("================================================================================\n");
    printf("              ATROPA BURNED LP DOLLAR VALUATION (REAL-TIME ON-CHAIN)\n");
    printf("================================================================================\n");
    printf("%-42s | %-15s | %-8s | %-16s | %-16s\n", "Pool Address", "Pair Name", "Burn %", "Stable Reserve", "Burned Value USD");
    printf("--------------------------------------------------------------------------------\n");

    int count = sizeof(stable_pools) / sizeof(stable_pools[0]);
    double total_burned_usd = 0.0;

    for (int i = 0; i < count; i++) {
        const char *pool = stable_pools[i];
        const char *stable = stable_tokens[i];
        double dec = stable_decimals[i];
        char r_buf[1024];
        double supply = 0.0;
        double bal0 = 0.0;
        double bal_dead = 0.0;
        double stable_reserve = 0.0;

        // 1. Query Total Supply of the LP token
        if (tsfi_pulse_rpc_call(pool, "0x18160ddd", r_buf, sizeof(r_buf))) {
            supply = parse_hex_double(r_buf);
        }

        // 2. Query Balance of 0x00...00
        if (tsfi_pulse_rpc_call(pool, "0x70a082310000000000000000000000000000000000000000000000000000000000000000", r_buf, sizeof(r_buf))) {
            bal0 = parse_hex_double(r_buf);
        }

        // 3. Query Balance of 0x00...dead
        if (tsfi_pulse_rpc_call(pool, "0x70a08231000000000000000000000000000000000000000000000000000000000000dead", r_buf, sizeof(r_buf))) {
            bal_dead = parse_hex_double(r_buf);
        }

        // 4. Query stablecoin reserve balance in the pool
        char calldata[128];
        // 0x70a08231 + 32-byte padded pool address
        sprintf(calldata, "0x70a08231000000000000000000000000%s", pool + 2);
        if (tsfi_pulse_rpc_call(stable, calldata, r_buf, sizeof(r_buf))) {
            stable_reserve = parse_hex_double(r_buf) / dec;
        }

        double burned_pct = (supply > 0.0) ? (bal0 + bal_dead) / supply : 0.0;
        double total_pool_value_usd = 2.0 * stable_reserve;
        double burned_value_usd = total_pool_value_usd * burned_pct;
        total_burned_usd += burned_value_usd;

        printf("%-42s | %-15s | %6.2f%% | $%15.2f | $%15.2f\n", 
               pool, pool_names[i], burned_pct * 100.0, stable_reserve, burned_value_usd);
    }
    printf("--------------------------------------------------------------------------------\n");
    printf("TOTAL REAL-TIME ON-CHAIN BURNED VALUE (DAI + USDC POOLS):       $%15.2f\n", total_burned_usd);
    printf("================================================================================\n");

    return 0;
}
