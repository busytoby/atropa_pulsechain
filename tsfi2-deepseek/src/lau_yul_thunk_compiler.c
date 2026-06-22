#include "lau_yul_thunk_internal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <strings.h>

static const char* case_insensitive_strstr(const char *haystack, const char *needle) {
    if (!haystack || !needle) return NULL;
    size_t needle_len = strlen(needle);
    if (needle_len == 0) return haystack;
    for (; *haystack; haystack++) {
        size_t i;
        for (i = 0; i < needle_len; i++) {
            char h = haystack[i];
            char n = needle[i];
            if (h >= 'A' && h <= 'Z') h += 32;
            if (n >= 'A' && n <= 'Z') n += 32;
            if (h != n) break;
        }
        if (i == needle_len) return haystack;
    }
    return NULL;
}

static uint8_t* hex_to_bytes(const char *hex, size_t *out_len) {
    if (hex[0] == '0' && hex[1] == 'x') hex += 2;
    size_t len = strlen(hex);
    *out_len = len / 2;
    uint8_t *bytes = malloc(*out_len);
    if (!bytes) return NULL;
    for (size_t i = 0; i < *out_len; i++) {
        unsigned int val;
        sscanf(hex + i * 2, "%2x", &val);
        bytes[i] = (uint8_t)val;
    }
    return bytes;
}

static void load_reconciliation_data(void) {
    FILE *fp = fopen("tsfi2-deepseek/evm_storage.json", "r");
    if (!fp) return;
    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        char key_str[128] = {0};
        char val_str[128] = {0};
        if (sscanf(line, " { \"key\": \"%[^\"]\", \"val\": \"%[^\"]\" }", key_str, val_str) == 2) {
            u256_t key = {{0}};
            u256_t val = {{0}};
            if (strlen(key_str) == 64 && strlen(val_str) == 64) {
                sscanf(key_str, "%16lx%16lx%16lx%16lx", &key.d[3], &key.d[2], &key.d[1], &key.d[0]);
                sscanf(val_str, "%16lx%16lx%16lx%16lx", &val.d[3], &val.d[2], &val.d[1], &val.d[0]);
                if (g_yul_evm_context.storage_count < 4096) {
                    g_yul_evm_context.storage_keys[g_yul_evm_context.storage_count] = key;
                    g_yul_evm_context.storage_vals[g_yul_evm_context.storage_count] = val;
                    g_yul_evm_context.storage_count++;
                }
            }
        }
    }
    fclose(fp);
}

static void write_abi_arg(uint8_t *dest, uint64_t val) {
    memset(dest, 0, 32);
    for (int i = 0; i < 8; i++) {
        dest[31 - i] = (uint8_t)(val >> (i * 8));
    }
}

// Compiles a Yul contract file using solc and registers it
bool lau_yul_thunk_init(const char *name, const char *yul_path, uint64_t virtual_address) {
    if (g_cached_contracts_count == 0) {
        load_reconciliation_data();
    }
    // Check if already registered
    for (int i = 0; i < g_cached_contracts_count; i++) {
        if (strcmp(g_cached_contracts[i].name, name) == 0) {
            return true;
        }
    }
    // Check if another contract with the same path has already been compiled
    for (int i = 0; i < g_cached_contracts_count; i++) {
        if (strcmp(g_cached_contracts[i].path, yul_path) == 0) {
            if (g_cached_contracts_count >= MAX_CACHED_CONTRACTS) {
                printf("[YUL_THUNK] Error: Max cached contracts reached\n");
                return false;
            }
            CachedContract *c = &g_cached_contracts[g_cached_contracts_count++];
            strncpy(c->name, name, sizeof(c->name) - 1);
            c->bytecode = malloc(g_cached_contracts[i].size);
            memcpy(c->bytecode, g_cached_contracts[i].bytecode, g_cached_contracts[i].size);
            c->size = g_cached_contracts[i].size;
            c->virtual_address = virtual_address;
            strncpy(c->path, yul_path, sizeof(c->path) - 1);
            printf("[YUL_THUNK] Reused compiled thunk for %s at virtual addr 0x%lx\n", name, virtual_address);
            return true;
        }
    }

    if (g_cached_contracts_count >= MAX_CACHED_CONTRACTS) {
        printf("[YUL_THUNK] Error: Max cached contracts reached\n");
        return false;
    }

    char resolved_path[512];
    strncpy(resolved_path, yul_path, sizeof(resolved_path) - 1);
    resolved_path[sizeof(resolved_path) - 1] = '\0';

    if (access(resolved_path, F_OK) != 0) {
        if (strncmp(resolved_path, "../solidity", 11) == 0) {
            snprintf(resolved_path, sizeof(resolved_path), "%s", yul_path + 3);
        }
    }

    bool is_solidity = (strstr(resolved_path, ".sol") != NULL);
    bool is_library = false;

    char cmd[1024];
    if (is_solidity) {
        if (is_library) {
            snprintf(cmd, sizeof(cmd), "solc --optimize --bin-runtime --allow-paths .. \"%s\" 2>/dev/null", resolved_path);
        } else {
            snprintf(cmd, sizeof(cmd), "solc --optimize --bin --allow-paths .. \"%s\" 2>/dev/null", resolved_path);
        }
    } else {
        snprintf(cmd, sizeof(cmd), "solc --strict-assembly --bin --allow-paths .. \"%s\" 2>/dev/null", resolved_path);
    }
    FILE *fp = popen(cmd, "r");
    if (!fp) {
        printf("[YUL_THUNK] Error: Failed to run solc command\n");
        return false;
    }

    static char line[524288];
    char *bytecode_hex = NULL;
    bool found_contract = false;
    bool found_bin = false;

    while (fgets(line, sizeof(line), fp)) {
        if (is_solidity && !found_contract) {
            const char *match_name = name;
            if (strcmp(name, "libattribute") == 0) match_name = "attribute";
            if (strcmp(name, "strings") == 0) match_name = "stringlib";
            if (strcmp(name, "corereactions") == 0) match_name = "corereactionslib";
            if (case_insensitive_strstr(line, match_name) && strstr(line, "======= ")) {
                found_contract = true;
            }
            continue;
        }
        if (found_bin) {
            char *start = line;
            while (*start == ' ' || *start == '\t') {
                start++;
            }
            size_t len = strlen(start);
            while (len > 0 && (start[len-1] == '\n' || start[len-1] == '\r' || start[len-1] == ' ')) {
                start[len-1] = '\0';
                len--;
            }
            if (len > 10) {
                bytecode_hex = strdup(start);
                break;
            }
            found_bin = false;
        }
        if (strstr(line, "Binary representation:") || strstr(line, "Binary:") || strstr(line, "Binary of the runtime part:")) {
            found_bin = true;
        }
    }
    pclose(fp);

    if (!bytecode_hex) {
        printf("[YUL_THUNK] Error: solc output didn't contain bytecode for %s\n", name);
        return false;
    }

    size_t bin_len = 0;
    uint8_t *raw_bin = hex_to_bytes(bytecode_hex, &bin_len);
    free(bytecode_hex);

    if (!raw_bin) {
        return false;
    }

    uint8_t *runtime_bin = NULL;
    size_t runtime_len = 0;

    if (is_library) {
        runtime_bin = raw_bin;
        runtime_len = bin_len;
        printf("[YUL_THUNK] Library %s bypass constructor. Direct runtime len: %zu\n", name, runtime_len);
    } else {
        size_t padded_len = bin_len + 512;
        uint8_t *padded_bin = calloc(1, padded_len);
        if (padded_bin) {
            memcpy(padded_bin, raw_bin, bin_len);
            
            uint8_t *args = padded_bin + bin_len;
            if (strcmp(name, "yi") == 0) {
                write_abi_arg(args, get_contract_address("SHAFactory"));
                write_abi_arg(args + 32, get_contract_address("SHIOFactory"));
                write_abi_arg(args + 64, get_contract_address("VMREQ"));
            } else if (strcmp(name, "zheng") == 0) {
                write_abi_arg(args, get_contract_address("yi"));
            } else if (strcmp(name, "zhou") == 0) {
                write_abi_arg(args, get_contract_address("zheng"));
            } else if (strcmp(name, "yau") == 0) {
                write_abi_arg(args, get_contract_address("zhou"));
            } else if (strcmp(name, "yang") == 0) {
                write_abi_arg(args, get_contract_address("yau"));
            } else if (strcmp(name, "siu") == 0) {
                write_abi_arg(args, get_contract_address("yang"));
            } else if (strcmp(name, "void") == 0) {
                write_abi_arg(args, get_contract_address("siu"));
            } else if (strcmp(name, "laufactory") == 0) {
                write_abi_arg(args, get_contract_address("void"));
            } else if (strcmp(name, "lau") == 0) {
                write_abi_arg(args, 0x60);
                write_abi_arg(args + 32, 0xa0);
                write_abi_arg(args + 64, get_contract_address("void"));
                write_abi_arg(args + 96, 12);
                memcpy(args + 128, "User Token 1", 12);
                write_abi_arg(args + 160, 10);
                memcpy(args + 192, "USERTOKEN1", 10);
            } else if (strcmp(name, "cho") == 0) {
                write_abi_arg(args, get_contract_address("void"));
            } else if (strcmp(name, "map") == 0) {
                write_abi_arg(args, get_contract_address("cho"));
                write_abi_arg(args + 32, get_contract_address("HECKE"));
            } else if (strcmp(name, "qing") == 0) {
                write_abi_arg(args, 123456789ULL);
                write_abi_arg(args + 32, get_contract_address("void"));
                write_abi_arg(args + 64, get_contract_address("cho"));
            } else if (strcmp(name, "qi") == 0) {
                write_abi_arg(args, get_contract_address("qing"));
            } else if (strcmp(name, "mai") == 0) {
                write_abi_arg(args, get_contract_address("qi"));
            } else if (strcmp(name, "xia") == 0) {
                write_abi_arg(args, get_contract_address("mai"));
            } else if (strcmp(name, "xie") == 0) {
                write_abi_arg(args, get_contract_address("xia"));
            } else if (strcmp(name, "chan") == 0) {
                write_abi_arg(args, get_contract_address("xie"));
            } else if (strcmp(name, "sei") == 0) {
                write_abi_arg(args, get_contract_address("chan"));
            } else if (strcmp(name, "choa") == 0) {
                write_abi_arg(args, get_contract_address("sei"));
            } else if (strcmp(name, "cheon") == 0) {
                write_abi_arg(args, get_contract_address("sei"));
            } else if (strcmp(name, "strings") == 0) {
                write_abi_arg(args, get_contract_address("void"));
            } else if (strcmp(name, "libattribute") == 0) {
                write_abi_arg(args, get_contract_address("void"));
            } else if (strcmp(name, "corereactions") == 0) {
                write_abi_arg(args, get_contract_address("void"));
            }
        } else {
            padded_bin = raw_bin;
            padded_len = bin_len;
        }

        YulEvmContext *init_ctx = calloc(1, sizeof(YulEvmContext));
        if (!init_ctx) {
            free(raw_bin);
            return false;
        }
        init_ctx->self_address = virtual_address;
        init_ctx->storage_count = g_yul_evm_context.storage_count;
        memcpy(init_ctx->storage_keys, g_yul_evm_context.storage_keys, sizeof(g_yul_evm_context.storage_keys));
        memcpy(init_ctx->storage_vals, g_yul_evm_context.storage_vals, sizeof(g_yul_evm_context.storage_vals));
        bool init_success = run_yul_bytecode(init_ctx, padded_bin, padded_len, name);
        if (padded_bin != raw_bin) free(padded_bin);
        if (!init_success || init_ctx->reverted || init_ctx->return_size == 0) {
            printf("[YUL_THUNK] Error: Initcode execution failed for %s.\n", name);
            free(raw_bin);
            free(init_ctx);
            return false;
        }

        runtime_bin = malloc(init_ctx->return_size);
        if (!runtime_bin) {
            free(raw_bin);
            free(init_ctx);
            return false;
        }
        memcpy(runtime_bin, init_ctx->return_data, init_ctx->return_size);
        runtime_len = init_ctx->return_size;
        free(raw_bin);

        for (int i = 0; i < init_ctx->storage_count; i++) {
            u256_t raw_key = init_ctx->storage_keys[i];
            bool found = false;
            for (int j = 0; j < g_yul_evm_context.storage_count; j++) {
                if (u256_eq(g_yul_evm_context.storage_keys[j], raw_key)) {
                    g_yul_evm_context.storage_vals[j] = init_ctx->storage_vals[i];
                    found = true;
                    break;
                }
            }
            if (!found && g_yul_evm_context.storage_count < 4096) {
                g_yul_evm_context.storage_keys[g_yul_evm_context.storage_count] = raw_key;
                g_yul_evm_context.storage_vals[g_yul_evm_context.storage_count] = init_ctx->storage_vals[i];
                g_yul_evm_context.storage_count++;
            }
        }
        free(init_ctx);
    }

    CachedContract *c = &g_cached_contracts[g_cached_contracts_count++];
    strncpy(c->name, name, sizeof(c->name) - 1);
    c->bytecode = runtime_bin;
    c->size = runtime_len;
    c->virtual_address = virtual_address;
    strncpy(c->path, yul_path, sizeof(c->path) - 1);

    persist_reconciliation_data();
    printf("[YUL_THUNK] Compiled and registered %s thunk: %s (%zu bytes) at virtual addr 0x%lx\n", is_solidity ? "Solidity" : "Yul", name, runtime_len, virtual_address);
    return true;
}

size_t lau_yul_thunk_get_size(const char *name) {
    for (int i = 0; i < g_cached_contracts_count; i++) {
        if (strcmp(g_cached_contracts[i].name, name) == 0) {
            return g_cached_contracts[i].size;
        }
    }
    return 0;
}
