#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <strings.h>
#include <stdint.h>

typedef struct {
    char name[32];
    char args[64];
    char desc[128];
} ABIMethod;

typedef struct {
    char contract_address[64];
    char contract_name[32];
    ABIMethod methods[8];
    int method_count;
} ContractABI;

// Mock registry of permanent glossary contracts and their ABI methods
const ContractABI GLOSSARY_REGISTRY[] = {
    {
        "dynamic_0x4800", "SHA",
        {
            {"Seed", "seed_val", "Constructor state setup initializing Base, Secret, and Signal."},
            {"Form", "chi", "Transitive state modification propagating external coordinates into Base."},
            {"Fuse", "base secret signal", "Owner-only administrative override transaction replacing core registers."},
            {"Tune", "None", "Resolves transmission Channel from Base and Signal."},
            {"Polarize", "None", "Resolves Pole from Base and Secret to shift camera projection axis."},
            {"Conify", "None", "Resolves Foundation from Base and Identity to map reference boundaries."},
            {"Bond", "None", "Binds dynamic velocity Dynamo and zeroes Pole."}
        }, 7
    },
    {
        "dynamic_0x4900", "CHO",
        {
            {"Delegate", "user_token", "Links a wallet address to a verified LAU token contract structure."},
            {"Saat", "index", "Queries seed parameters mapping unique user identifiers."}
        }, 2
    },
    {
        "dynamic_0x4A00", "LAU",
        {
            {"Audit", "None", "Performs clean-room validation audit of physical teddy bear assets."},
            {"Transfer", "to amount", "Transfers Saat endowments between qualifying system participants."}
        }, 2
    }
};
#define REGISTRY_SIZE 3

// EVM 4-byte selector mapping structure
typedef struct {
    uint32_t selector;
    char signature[64];
    char description[128];
} EVMSelectorMap;

const EVMSelectorMap EVM_SELECTORS[] = {
    {0x70a08231, "balanceOf(address)", "Queries the token balance of the specified account address."},
    {0xa9059cbb, "transfer(address,uint256)", "Transfers the specified amount of tokens to a target address."},
    {0x095ea7b3, "approve(address,uint256)", "Approves the target spender to withdraw tokens up to the specified allowance."},
    {0xdd62ed3e, "allowance(address,address)", "Returns the remaining number of tokens that the spender is allowed to withdraw."},
    {0x06fdde03, "name()", "Returns the name of the token."},
    {0x95d89b41, "symbol()", "Returns the symbol of the token."}
};
#define EVM_SELECTORS_SIZE 6

// WinchesterMQ Virtual SCSI Register mapping
typedef struct {
    uint16_t reg_addr;
    char name[32];
    char role[128];
} WMQRegisterMap;

const WMQRegisterMap WMQ_REGISTERS[] = {
    {0x4080, "DEFCON_LATCH", "Persistent state latch preventing de-escalation of power warning alarms."},
    {0x4800, "FIELDATA_MODEL", "RDBMS database weights tracking context-adaptive arithmetic probabilities."},
    {0x4840, "RANGE_VALUE", "Current value code bounds inside the TERSE arithmetic range coder."},
    {0x4860, "UTF6_ESCAPE", "Shift indicator registry mapping 18, 24, and 36-bit variable Unicode points."},
    {0xFFFF, "CMD_ABI_HELP", "Low-level SCSI query address returning raw ABI reflection schemas."}
};
#define WMQ_REGISTERS_SIZE 5

void print_help_menu(void) {
    printf("=== ZMM VM CLIST HELP SYSTEM ===\n");
    printf("Usage:\n");
    printf("  LIST                       - List all registered contracts\n");
    printf("  HELP <address>             - Show all methods for a contract\n");
    printf("  HELP <address> <method>    - Show detailed help for a specific method\n");
    printf("  HELP <evm_selector>        - Decode and explain a 4-byte EVM hex selector (e.g. 0x70a08231)\n");
    printf("  WMQ <register>             - Query WinchesterMQ SCSI register role (e.g. 0x4800 or 0xFFFF)\n");
    printf("\n");
}

int main(int argc, char **argv) {
    if (argc < 2) {
        print_help_menu();
        return 0;
    }

    const char *cmd = argv[1];

    if (strcmp(cmd, "LIST") == 0) {
        printf("Registered ZMM Contract Address Registry:\n");
        printf("-----------------------------------------------------------------\n");
        printf("  %-16s | %-12s | %-16s\n", "Address", "Name", "Methods Count");
        printf("-----------------------------------------------------------------\n");
        for (int i = 0; i < REGISTRY_SIZE; i++) {
            printf("  %-16s | %-12s | %-16d\n", 
                   GLOSSARY_REGISTRY[i].contract_address, 
                   GLOSSARY_REGISTRY[i].contract_name, 
                   GLOSSARY_REGISTRY[i].method_count);
        }
        printf("-----------------------------------------------------------------\n");
        return 0;
    }

    if (strcmp(cmd, "HELP") == 0) {
        if (argc < 3) {
            print_help_menu();
            return 1;
        }

        const char *addr = argv[2];

        // 1. Check if argument is a 4-byte EVM Selector (e.g. starts with "0x" and is 10 chars long)
        if (strncmp(addr, "0x", 2) == 0 && strlen(addr) == 10) {
            unsigned int sel_val = 0;
            if (sscanf(addr, "0x%x", &sel_val) == 1) {
                int found_evm = 0;
                for (int i = 0; i < EVM_SELECTORS_SIZE; i++) {
                    if (EVM_SELECTORS[i].selector == sel_val) {
                        printf("EVM Selector : %s (0x%08X)\n", addr, EVM_SELECTORS[i].selector);
                        printf("Signature    : %s\n", EVM_SELECTORS[i].signature);
                        printf("Description  : %s\n", EVM_SELECTORS[i].description);
                        found_evm = 1;
                        break;
                    }
                }
                if (!found_evm) {
                    fprintf(stderr, "Error: EVM Selector '%s' not found in decoder mapping.\n", addr);
                    return 1;
                }
                return 0;
            }
        }

        // 2. Resolve target contract address
        const ContractABI *target = NULL;
        for (int i = 0; i < REGISTRY_SIZE; i++) {
            if (strcmp(GLOSSARY_REGISTRY[i].contract_address, addr) == 0 || 
                strcasecmp(GLOSSARY_REGISTRY[i].contract_name, addr) == 0) {
                target = &GLOSSARY_REGISTRY[i];
                break;
            }
        }

        if (!target) {
            fprintf(stderr, "Error: Contract/Selector '%s' not found in registry.\n", addr);
            return 1;
        }

        // Check for --export flag
        int export_json = 0;
        int export_yaml = 0;
        for (int argi = 2; argi < argc; argi++) {
            if (strcmp(argv[argi], "--export") == 0 && argi + 1 < argc) {
                if (strcasecmp(argv[argi+1], "json") == 0) export_json = 1;
                if (strcasecmp(argv[argi+1], "yaml") == 0) export_yaml = 1;
            }
        }

        if (export_json) {
            printf("[\n");
            for (int j = 0; j < target->method_count; j++) {
                printf("  {\n");
                printf("    \"name\": \"%s\",\n", target->methods[j].name);
                printf("    \"type\": \"function\",\n");
                printf("    \"inputs\": [\n");
                if (strcmp(target->methods[j].args, "None") != 0) {
                    // Split space-separated args
                    char args_copy[64];
                    strncpy(args_copy, target->methods[j].args, sizeof(args_copy));
                    args_copy[sizeof(args_copy)-1] = '\0';
                    char *token = strtok(args_copy, " ");
                    int first = 1;
                    while (token) {
                        if (!first) printf(",\n");
                        printf("      {\"name\": \"%s\", \"type\": \"uint256\"}", token);
                        first = 0;
                        token = strtok(NULL, " ");
                    }
                    printf("\n");
                }
                printf("    ],\n");
                printf("    \"description\": \"%s\"\n", target->methods[j].desc);
                printf("  }%s\n", (j == target->method_count - 1) ? "" : ",");
            }
            printf("]\n");
            return 0;
        }

        if (export_yaml) {
            printf("contract: %s\n", target->contract_name);
            printf("address: %s\n", target->contract_address);
            printf("methods:\n");
            for (int j = 0; j < target->method_count; j++) {
                printf("  - name: %s\n", target->methods[j].name);
                printf("    description: \"%s\"\n", target->methods[j].desc);
                printf("    inputs: [");
                if (strcmp(target->methods[j].args, "None") != 0) {
                    char args_copy[64];
                    strncpy(args_copy, target->methods[j].args, sizeof(args_copy));
                    args_copy[sizeof(args_copy)-1] = '\0';
                    char *token = strtok(args_copy, " ");
                    int first = 1;
                    while (token) {
                        if (!first) printf(", ");
                        printf("%s", token);
                        first = 0;
                        token = strtok(NULL, " ");
                    }
                }
                printf("]\n");
            }
            return 0;
        }

        if (argc < 4) {
            // Print all methods for the contract
            printf("Contract: %s (%s)\n", target->contract_name, target->contract_address);
            printf("Exposed ABI Methods:\n");
            printf("-----------------------------------------------------------------\n");
            for (int j = 0; j < target->method_count; j++) {
                printf("  %-10s (%s)\n", target->methods[j].name, target->methods[j].args);
                printf("             %s\n", target->methods[j].desc);
            }
            printf("-----------------------------------------------------------------\n");
        } else {
            // Print detailed help for a specific method
            const char *method_name = argv[3];
            int found = 0;
            for (int j = 0; j < target->method_count; j++) {
                if (strcasecmp(target->methods[j].name, method_name) == 0) {
                    printf("Contract   : %s (%s)\n", target->contract_name, target->contract_address);
                    printf("Method     : %s\n", target->methods[j].name);
                    printf("Parameters : %s\n", target->methods[j].args);
                    printf("Description: %s\n", target->methods[j].desc);
                    found = 1;
                    break;
                }
            }
            if (!found) {
                fprintf(stderr, "Error: Method '%s' not found on contract '%s'.\n", method_name, target->contract_name);
                return 1;
            }
        }
        return 0;
    }

    if (strcmp(cmd, "WMQ") == 0) {
        if (argc < 3) {
            print_help_menu();
            return 1;
        }

        const char *reg_str = argv[2];
        unsigned int reg_val = 0;
        if (sscanf(reg_str, "0x%x", &reg_val) != 1) {
            fprintf(stderr, "Error: Register address must be in hex format (e.g. 0x4800).\n");
            return 1;
        }

        int found_reg = 0;
        for (int i = 0; i < WMQ_REGISTERS_SIZE; i++) {
            if (WMQ_REGISTERS[i].reg_addr == reg_val) {
                printf("WinchesterMQ SCSI Register: 0x%04X\n", WMQ_REGISTERS[i].reg_addr);
                printf("Name                      : %s\n", WMQ_REGISTERS[i].name);
                printf("Description/Role          : %s\n", WMQ_REGISTERS[i].role);
                found_reg = 1;
                break;
            }
        }

        if (!found_reg) {
            fprintf(stderr, "Error: WinchesterMQ register 0x%04X not found in mapping.\n", reg_val);
            return 1;
        }
        return 0;
    }

    print_help_menu();
    return 1;
}
