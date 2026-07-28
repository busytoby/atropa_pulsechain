#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <strings.h>

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

void print_help_menu(void) {
    printf("=== ZMM VM CLIST HELP SYSTEM ===\n");
    printf("Usage:\n");
    printf("  LIST                       - List all registered contracts\n");
    printf("  HELP <address>             - Show all methods for a contract\n");
    printf("  HELP <address> <method>    - Show detailed help for a specific method\n");
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
        const ContractABI *target = NULL;

        for (int i = 0; i < REGISTRY_SIZE; i++) {
            if (strcmp(GLOSSARY_REGISTRY[i].contract_address, addr) == 0 || 
                strcasecmp(GLOSSARY_REGISTRY[i].contract_name, addr) == 0) {
                target = &GLOSSARY_REGISTRY[i];
                break;
            }
        }

        if (!target) {
            fprintf(stderr, "Error: Contract '%s' not found in registry.\n", addr);
            return 1;
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

    print_help_menu();
    return 1;
}
