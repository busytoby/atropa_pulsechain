#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>

// EVM selector mapping
typedef struct {
    uint32_t selector;
    char signature[64];
} EVMSelectorMap;

const EVMSelectorMap EVM_SELECTORS[] = {
    {0x70a08231, "balanceOf(address)"},
    {0xa9059cbb, "transfer(address,uint256)"},
    {0x095ea7b3, "approve(address,uint256)"},
    {0xdd62ed3e, "allowance(address,address)"}
};
#define EVM_SELECTORS_SIZE 4

// WinchesterMQ register mapping
typedef struct {
    uint16_t reg_addr;
    char name[32];
} WMQRegisterMap;

const WMQRegisterMap WMQ_REGISTERS[] = {
    {0x4080, "DEFCON_LATCH"},
    {0x4800, "FIELDATA_MODEL"},
    {0x4840, "RANGE_VALUE"},
    {0x4860, "UTF6_ESCAPE"},
    {0xFFFF, "CMD_ABI_HELP"}
};
#define WMQ_REGISTERS_SIZE 5

// Resolves EVM selector signature
const char *resolve_evm_selector(uint32_t selector) {
    for (int i = 0; i < EVM_SELECTORS_SIZE; i++) {
        if (EVM_SELECTORS[i].selector == selector) {
            return EVM_SELECTORS[i].signature;
        }
    }
    return "UNKNOWN_SELECTOR";
}

// Resolves WinchesterMQ register name
const char *resolve_wmq_register(uint16_t reg_addr) {
    for (int i = 0; i < WMQ_REGISTERS_SIZE; i++) {
        if (WMQ_REGISTERS[i].reg_addr == reg_addr) {
            return WMQ_REGISTERS[i].name;
        }
    }
    return "UNKNOWN_REGISTER";
}

// Simulation of VM events
typedef enum {
    EVENT_EVM_TX,
    EVENT_WMQ_REG
} EventType;

typedef struct {
    uint64_t timestamp;
    EventType type;
    char contract_name[32];
    uint32_t identifier; // Selector or Register address
    uint64_t old_val;
    uint64_t new_val;
} VMEvent;

// Mock event queue for simulation
const VMEvent MOCK_EVENTS[] = {
    {1717000001, EVENT_WMQ_REG, "SHA", 0x4080, 0, 1}, // Latch defcon
    {1717000002, EVENT_EVM_TX,  "LAU", 0x70a08231, 0, 0}, // Query balance
    {1717000003, EVENT_WMQ_REG, "SHA", 0x4800, 100, 150}, // Update probability model
    {1717000004, EVENT_EVM_TX,  "LAU", 0xa9059cbb, 0, 1000000}, // Transfer Saat
    {1717000005, EVENT_WMQ_REG, "SHA", 0x4840, 0x1234, 0x5678}  // Code bounds shift
};
#define MOCK_EVENTS_SIZE 5

int main(void) {
    printf("=== TSFI: REAL-TIME ABI TELEMETRY MONITOR ===\n");
    printf("[MONITOR] Listening on WinchesterMQ diagnostic socket...\n\n");
    printf("  %-10s | %-12s | %-32s | %-16s -> %-16s\n", "Timestamp", "Context", "Target Signature / Register", "Old Value", "New Value");
    printf("  -----------+--------------+----------------------------------+------------------+-----------------\n");

    for (int i = 0; i < MOCK_EVENTS_SIZE; i++) {
        const VMEvent *ev = &MOCK_EVENTS[i];

        // Format time string
        char time_str[32];
        time_t raw_time = (time_t)ev->timestamp;
        struct tm *info = localtime(&raw_time);
        strftime(time_str, sizeof(time_str), "%H:%M:%S", info);

        if (ev->type == EVENT_EVM_TX) {
            const char *sig = resolve_evm_selector(ev->identifier);
            printf("  %-10s | EVM-TX       | %-32s | 0x%016lx -> 0x%016lx\n", 
                   time_str, sig, ev->old_val, ev->new_val);
        } else if (ev->type == EVENT_WMQ_REG) {
            const char *reg_name = resolve_wmq_register((uint16_t)ev->identifier);
            char reg_sig[64];
            snprintf(reg_sig, sizeof(reg_sig), "%s (0x%04x)", reg_name, ev->identifier);
            printf("  %-10s | WMQ-REG (%s) | %-32s | 0x%016lx -> 0x%016lx\n", 
                   time_str, ev->contract_name, reg_sig, ev->old_val, ev->new_val);
        }
        
        // Simulate real-time delay
        usleep(200000); 
    }

    printf("\n=== MONITOR SIMULATION COMPLETED ===\n");
    return 0;
}
