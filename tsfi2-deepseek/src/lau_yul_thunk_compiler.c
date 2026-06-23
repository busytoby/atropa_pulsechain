#include "lau_yul_thunk_internal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <strings.h>
#include <sys/stat.h>

static char* check_cache(const char *name, const char *src_path) {
    (void)src_path;
    char cache_path[512];
    snprintf(cache_path, sizeof(cache_path), "/home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/.yul_cache/%s.hex", name);
    
    FILE *f = fopen(cache_path, "r");
    if (!f) return NULL;
    
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    if (len <= 0) {
        fclose(f);
        return NULL;
    }
    fseek(f, 0, SEEK_SET);
    
    char *hex = malloc(len + 1);
    if (hex) {
        size_t n = fread(hex, 1, len, f);
        hex[n] = '\0';
        while (n > 0 && (hex[n-1] == '\n' || hex[n-1] == '\r' || hex[n-1] == ' ')) {
            hex[n-1] = '\0';
            n--;
        }
    }
    fclose(f);
    return hex;
}

static void write_cache(const char *name, const char *hex) {
    mkdir("/home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/.yul_cache", 0755);
    char cache_path[512];
    snprintf(cache_path, sizeof(cache_path), "/home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/.yul_cache/%s.hex", name);
    FILE *f = fopen(cache_path, "w");
    if (f) {
        fputs(hex, f);
        fclose(f);
    }
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
    const char *path = "evm_storage.json";
    FILE *fp = fopen(path, "r");
    if (!fp) {
        path = "tsfi2-deepseek/evm_storage.json";
        fp = fopen(path, "r");
    }
    if (!fp) {
        printf("[YUL_THUNK] evm_storage.json not found!\n");
        return;
    }
    printf("[YUL_THUNK] Loading reconciliation data from %s...\n", path);
    char line[512];
    int loaded = 0;
    while (fgets(line, sizeof(line), fp)) {
        char key_str[128] = {0};
        char val_str[128] = {0};
        char addr_str[128] = {0};
        int parsed = sscanf(line, " { \"key\": \"%[^\"]\", \"val\": \"%[^\"]\", \"addr\": \"%[^\"]\" }", key_str, val_str, addr_str);
        if (parsed < 2) {
            parsed = sscanf(line, " { \"key\": \"%[^\"]\", \"val\": \"%[^\"]\" }", key_str, val_str);
            addr_str[0] = '\0';
        }
        if (parsed >= 2) {
            u256_t key = {{0}};
            u256_t val = {{0}};
            uint64_t addr = 0;
            if (strlen(key_str) == 64 && strlen(val_str) == 64) {
                sscanf(key_str, "%16lx%16lx%16lx%16lx", &key.d[3], &key.d[2], &key.d[1], &key.d[0]);
                sscanf(val_str, "%16lx%16lx%16lx%16lx", &val.d[3], &val.d[2], &val.d[1], &val.d[0]);
                if (addr_str[0] != '\0') {
                    sscanf(addr_str, "%lx", &addr);
                }
                if (g_yul_evm_context.storage_count < 32768) {
                    g_yul_evm_context.storage_keys[g_yul_evm_context.storage_count] = key;
                    g_yul_evm_context.storage_vals[g_yul_evm_context.storage_count] = val;
                    g_yul_evm_context.storage_addrs[g_yul_evm_context.storage_count] = addr;
                    g_yul_evm_context.storage_count++;
                    loaded++;
                }
            }
        }
    }
    printf("[YUL_THUNK] Loaded %d keys from %s. Total global: %d\n", loaded, path, g_yul_evm_context.storage_count);
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
    // Check if another contract with the same name has already been compiled (checking cache files)
    char cache_path_chk[512];
    snprintf(cache_path_chk, sizeof(cache_path_chk), "/home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/.yul_cache/%s.hex", name);
    if (access(cache_path_chk, F_OK) != 0) {
        // If not on disk, check if another contract with the same path has already been compiled
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
        if (access(resolved_path, F_OK) != 0) {
            const char *slash = strrchr(yul_path, '/');
            const char *basename = slash ? slash + 1 : yul_path;
            snprintf(resolved_path, sizeof(resolved_path), "/home/mariarahel/src/tsfi2/atropa_pulsechain/solidity/bin/%s", basename);
        }
    }
    // Force copy to solidity/bin if compile target is YueReactSimulator to hook it correctly
    if (strcmp(name, "YueReactSimulator") == 0) {
        char copy_cmd[1024];
        snprintf(copy_cmd, sizeof(copy_cmd), "cp %s /home/mariarahel/src/tsfi2/atropa_pulsechain/solidity/bin/YueReactSimulator.yul", resolved_path);
        int ret = system(copy_cmd);
        (void)ret;
        snprintf(resolved_path, sizeof(resolved_path), "/home/mariarahel/src/tsfi2/atropa_pulsechain/solidity/bin/YueReactSimulator.yul");
    }

    bool is_solidity = (strstr(resolved_path, ".sol") != NULL);
    bool is_library = false;

    char *bytecode_hex = check_cache(name, resolved_path);
    if (bytecode_hex) {
        printf("[YUL_THUNK] Loaded cached bytecode for %s\n", name);
    } else {
        char cmd[1024];
        if (is_solidity) {
            if (is_library) {
                snprintf(cmd, sizeof(cmd), "solc --evm-version shanghai --optimize --bin-runtime --allow-paths .. \"%s\" 2>/dev/null", resolved_path);
            } else {
                snprintf(cmd, sizeof(cmd), "solc --evm-version shanghai --optimize --bin --allow-paths .. \"%s\" 2>/dev/null", resolved_path);
            }
        } else {
            snprintf(cmd, sizeof(cmd), "solc --evm-version shanghai --strict-assembly --bin --allow-paths .. \"%s\" 2>/dev/null", resolved_path);
        }
        FILE *fp = popen(cmd, "r");
        if (!fp) {
            printf("[YUL_THUNK] Error: Failed to run solc command\n");
            return false;
        }

        static char line[524288];
        char current_contract[128] = {0};
        bool found_bin = false;

        while (fgets(line, sizeof(line), fp)) {
            if (strstr(line, "======= ")) {
                current_contract[0] = '\0';
                found_bin = false;
                if (!is_solidity) {
                    if (strstr(line, ".yul")) {
                        const char *slash = strrchr(line, '/');
                        const char *start = slash ? slash + 1 : strstr(line, "======= ") + 8;
                        int j = 0;
                        while (start[j] && start[j] != ' ' && start[j] != '\n' && start[j] != '\r' && start[j] != '.' && j < 127) {
                            current_contract[j] = start[j];
                            j++;
                        }
                        current_contract[j] = '\0';
                    }
                } else {
                    const char *colon = strrchr(line, ':');
                    if (colon) {
                        int j = 0;
                        colon++;
                        while (colon[j] && colon[j] != ' ' && colon[j] != '\n' && colon[j] != '\r' && j < 127) {
                            current_contract[j] = colon[j];
                            j++;
                        }
                        current_contract[j] = '\0';
                    }
                }
                continue;
            }
            if (current_contract[0] == '\0') {
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
                    // Check standard match names to align with our VM system targets
                    const char *mapped_name = current_contract;
                    if (strcasecmp(current_contract, "attribute") == 0) mapped_name = "libattribute";
                    else if (strcasecmp(current_contract, "stringlib") == 0) mapped_name = "strings";
                    else if (strcasecmp(current_contract, "corereactionslib") == 0) mapped_name = "corereactions";
                    else if (strcasecmp(current_contract, "qing") == 0 && strstr(name, "qing") != NULL) mapped_name = name;
                    else if (strcasecmp(current_contract, "yi") == 0) mapped_name = "yi";
                    else if (strcasecmp(current_contract, "zheng") == 0) mapped_name = "zheng";
                    else if (strcasecmp(current_contract, "zhou") == 0) mapped_name = "zhou";
                    else if (strcasecmp(current_contract, "yau") == 0) mapped_name = "yau";
                    else if (strcasecmp(current_contract, "yang") == 0) mapped_name = "yang";
                    else if (strcasecmp(current_contract, "siu") == 0) mapped_name = "siu";
                    else if (strcasecmp(current_contract, "void") == 0) mapped_name = "void";
                    else if (strcasecmp(current_contract, "laufactory") == 0) mapped_name = "laufactory";
                    else if (strcasecmp(current_contract, "lau") == 0) mapped_name = "lau";
                    else if (strcasecmp(current_contract, "cho") == 0) mapped_name = "cho";
                    else if (strcasecmp(current_contract, "map") == 0) mapped_name = "map";
                    else if (strcasecmp(current_contract, "qi") == 0) mapped_name = "qi";
                    else if (strcasecmp(current_contract, "mai") == 0) mapped_name = "mai";
                    else if (strcasecmp(current_contract, "xia") == 0) mapped_name = "xia";
                    else if (strcasecmp(current_contract, "xie") == 0) mapped_name = "xie";
                    else if (strcasecmp(current_contract, "chan") == 0) mapped_name = "chan";
                    else if (strcasecmp(current_contract, "sei") == 0) mapped_name = "sei";
                    else if (strcasecmp(current_contract, "choa") == 0) mapped_name = "choa";
                    else if (strcasecmp(current_contract, "cheon") == 0) mapped_name = "cheon";

                    write_cache(mapped_name, start);
                    if (strcasecmp(mapped_name, name) == 0) {
                        bytecode_hex = strdup(start);
                    }
                }
                found_bin = false;
            }
            if (strstr(line, "Binary representation:") || strstr(line, "Binary:") || strstr(line, "Binary of the runtime part:")) {
                found_bin = true;
            }
        }
        pclose(fp);

        if (!bytecode_hex) {
            // Check if it was written into cache by a previous compiler output line
            bytecode_hex = check_cache(name, resolved_path);
        }
        if (bytecode_hex) {
            write_cache(name, bytecode_hex);
        }
    }

    if (!bytecode_hex) {
        printf("[YUL_THUNK] Error: Contract %s has no cached bytecode\n", name);
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
            const char *base_name = name;
            if (strncmp(name, "qing_", 5) == 0) {
                base_name = "qing";
            }
            if (strcmp(base_name, "yi") == 0) {
                write_abi_arg(args, get_contract_address("SHAFactory"));
                write_abi_arg(args + 32, get_contract_address("SHIOFactory"));
                write_abi_arg(args + 64, get_contract_address("VMREQ"));
            } else if (strcmp(base_name, "zheng") == 0) {
                write_abi_arg(args, get_contract_address("yi"));
            } else if (strcmp(base_name, "zhou") == 0) {
                write_abi_arg(args, get_contract_address("zheng"));
            } else if (strcmp(base_name, "yau") == 0) {
                write_abi_arg(args, get_contract_address("zhou"));
            } else if (strcmp(base_name, "yang") == 0) {
                write_abi_arg(args, get_contract_address("yau"));
            } else if (strcmp(base_name, "siu") == 0) {
                write_abi_arg(args, get_contract_address("yang"));
            } else if (strcmp(base_name, "void") == 0) {
                write_abi_arg(args, get_contract_address("siu"));
            } else if (strcmp(base_name, "laufactory") == 0) {
                write_abi_arg(args, get_contract_address("void"));
            } else if (strcmp(base_name, "lau") == 0) {
                write_abi_arg(args, 0x60);
                write_abi_arg(args + 32, 0xa0);
                write_abi_arg(args + 64, get_contract_address("void"));
                write_abi_arg(args + 96, 12);
                memcpy(args + 128, "User Token 1", 12);
                write_abi_arg(args + 160, 10);
                memcpy(args + 192, "USERTOKEN1", 10);
            } else if (strcmp(base_name, "cho") == 0) {
                write_abi_arg(args, get_contract_address("void"));
            } else if (strcmp(base_name, "map") == 0) {
                write_abi_arg(args, get_contract_address("cho"));
                write_abi_arg(args + 32, get_contract_address("HECKE"));
            } else if (strcmp(base_name, "qing") == 0) {
                write_abi_arg(args, 123456789ULL);
                write_abi_arg(args + 32, get_contract_address("void"));
                write_abi_arg(args + 64, get_contract_address("cho"));
            } else if (strcmp(base_name, "qi") == 0) {
                write_abi_arg(args, get_contract_address("qing"));
            } else if (strcmp(base_name, "mai") == 0) {
                write_abi_arg(args, get_contract_address("qi"));
            } else if (strcmp(base_name, "xia") == 0) {
                write_abi_arg(args, get_contract_address("mai"));
            } else if (strcmp(base_name, "xie") == 0) {
                write_abi_arg(args, get_contract_address("xia"));
            } else if (strcmp(base_name, "chan") == 0) {
                write_abi_arg(args, get_contract_address("xie"));
            } else if (strcmp(base_name, "sei") == 0) {
                write_abi_arg(args, get_contract_address("chan"));
            } else if (strcmp(base_name, "choa") == 0) {
                write_abi_arg(args, get_contract_address("sei"));
            } else if (strcmp(base_name, "cheon") == 0) {
                write_abi_arg(args, get_contract_address("sei"));
            } else if (strcmp(base_name, "strings") == 0) {
                write_abi_arg(args, get_contract_address("void"));
            } else if (strcmp(base_name, "libattribute") == 0) {
                write_abi_arg(args, get_contract_address("void"));
            } else if (strcmp(base_name, "corereactions") == 0) {
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
        init_ctx->caller_address.d[0] = 0x4cc;
        init_ctx->is_initcode = true;

        // Copy only storage keys that do NOT belong to this virtual_address
        init_ctx->storage_count = 0;
        for (int i = 0; i < g_yul_evm_context.storage_count; i++) {
            if (g_yul_evm_context.storage_addrs[i] != virtual_address) {
                init_ctx->storage_keys[init_ctx->storage_count] = g_yul_evm_context.storage_keys[i];
                init_ctx->storage_vals[init_ctx->storage_count] = g_yul_evm_context.storage_vals[i];
                init_ctx->storage_addrs[init_ctx->storage_count] = g_yul_evm_context.storage_addrs[i];
                init_ctx->storage_count++;
            }
        }

        g_initcode_running = true;
        bool init_success = run_yul_bytecode(init_ctx, padded_bin, padded_len, name);
        g_initcode_running = false;
        if (padded_bin != raw_bin) free(padded_bin);
        if (!init_success || init_ctx->reverted || init_ctx->return_size == 0) {
            printf("[YUL_THUNK] Error: Initcode execution failed for %s. init_success=%d reverted=%d return_size=%zu\n", name, init_success, init_ctx->reverted, init_ctx->return_size);
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

        // Copy storage mutations back to g_yul_evm_context
        for (int i = 0; i < init_ctx->storage_count; i++) {
            u256_t raw_key = init_ctx->storage_keys[i];
            bool found = false;
            for (int j = 0; j < g_yul_evm_context.storage_count; j++) {
                if (u256_eq(g_yul_evm_context.storage_keys[j], raw_key)) {
                    g_yul_evm_context.storage_vals[j] = init_ctx->storage_vals[i];
                    g_yul_evm_context.storage_addrs[j] = init_ctx->storage_addrs[i];
                    found = true;
                    break;
                }
            }
            if (!found && g_yul_evm_context.storage_count < 32768) {
                g_yul_evm_context.storage_keys[g_yul_evm_context.storage_count] = raw_key;
                g_yul_evm_context.storage_vals[g_yul_evm_context.storage_count] = init_ctx->storage_vals[i];
                g_yul_evm_context.storage_addrs[g_yul_evm_context.storage_count] = init_ctx->storage_addrs[i];
                g_yul_evm_context.storage_count++;
                g_storage_dirty = true;
            }
        }
        printf("[YUL_THUNK] Copied %d storage keys from constructor of %s back to global context. Total global storage: %d\n", init_ctx->storage_count, name, g_yul_evm_context.storage_count);
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
