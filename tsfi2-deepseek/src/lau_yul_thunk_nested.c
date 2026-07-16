#include "lau_yul_thunk_internal.h"
#include "tsfi_qing_bst.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool execute_nested_call(YulEvmContext *ctx, uint64_t target_addr, uint64_t argsOffset, uint64_t argsSize, uint64_t retOffset, uint64_t retSize, u256_t *success_out) {
    success_out->d[0] = 1;

    uint32_t selector = 0;
    if (argsSize >= 4 && argsOffset < 524288) {
        selector = ((uint32_t)ctx->memory[argsOffset] << 24) |
                   ((uint32_t)ctx->memory[argsOffset + 1] << 16) |
                   ((uint32_t)ctx->memory[argsOffset + 2] << 8) |
                   ((uint32_t)ctx->memory[argsOffset + 3]);
    }

    char wmq_addr_str[128] = {0};
    FILE *wmq_file = fopen("../tmp/wmq_address.txt", "r");
    if (!wmq_file) wmq_file = fopen("tmp/wmq_address.txt", "r");
    if (!wmq_file) wmq_file = fopen("/home/mariarahel/src/tsfi2/atropa_pulsechain/tmp/wmq_address.txt", "r");
    if (wmq_file) {
        if (fgets(wmq_addr_str, sizeof(wmq_addr_str), wmq_file)) {
            size_t l = strlen(wmq_addr_str);
            while (l > 0 && (wmq_addr_str[l-1] == '\n' || wmq_addr_str[l-1] == '\r')) {
                wmq_addr_str[l-1] = '\0';
                l--;
            }
        }
        fclose(wmq_file);
    }
    if (strlen(wmq_addr_str) > 0) {
        uint64_t wmq_addr_u64 = parse_hex64(wmq_addr_str);
        if (target_addr == wmq_addr_u64) {
            printf("[EVM_INTERPRETER] Intercepted WinchesterMQ contract call at address 0x%lx with selector 0x%08x\n", target_addr, selector);
            if (argsSize > 0 && argsOffset < 524288) {
                bool is_io = false;
                if (selector == 0x98d400c0) {
                    is_io = true;
                } else if (argsSize >= 36 && selector == 0xccb077a0) {
                    char cmd_str[33] = {0};
                    size_t cmd_len = argsSize - 4;
                    if (cmd_len > 32) cmd_len = 32;
                    for (size_t i = 0; i < cmd_len; i++) {
                        cmd_str[i] = (char)ctx->memory[argsOffset + 4 + i];
                    }
                    if (strstr(cmd_str, "MM") || strstr(cmd_str, "MD") || strstr(cmd_str, "MU") ||
                        strstr(cmd_str, "MS") || strstr(cmd_str, "KD") || strstr(cmd_str, "KU") ||
                        strstr(cmd_str, "MO") || strstr(cmd_str, "MOUSE") || strstr(cmd_str, "KEY")) {
                        is_io = true;
                    }
                }
                
                if (is_io) {
                    printf("[EVM_INTERPRETER] WinchesterMQ keyboard/mouse event intercepted. Bypassing EVM RPC transaction.\n");
                } else {
                    char *data_hex = malloc(argsSize * 2 + 3);
                    if (data_hex) {
                        strcpy(data_hex, "0x");
                        for (size_t i = 0; i < argsSize; i++) {
                            sprintf(data_hex + 2 + i * 2, "%02x", ctx->memory[argsOffset + i]);
                        }
                        extern bool tsfi_pulse_rpc_send_wmq_transaction(const char *to_address, const char *data_hex);
                        bool rpc_ok = tsfi_pulse_rpc_send_wmq_transaction(wmq_addr_str, data_hex);
                        printf("[EVM_INTERPRETER] WinchesterMQ direct transaction sent: %s (status: %s)\n", data_hex, rpc_ok ? "SUCCESS" : "FAILED");
                        free(data_hex);
                    }
                }
            }
            success_out->d[0] = 1;
            return true;
        }
    }

    if (selector == 0x6872fc3c) { // Cho()
        u256_t mock_ret = {{ get_contract_address("cho"), 0, 0, 0 }};
        if (retOffset < 524288 && retSize > 0) {
            uint8_t temp_buf[32] = {0};
            write_u256_be(temp_buf, mock_ret);
            size_t src_offset = (retSize < 32) ? (32 - retSize) : 0;
            size_t copy_len = (retSize < 32) ? retSize : 32;
            for (size_t i = 0; i < copy_len; i++) {
                uint64_t dest = retOffset + i;
                if (dest < 524288) ctx->memory[dest] = temp_buf[src_offset + i];
            }
            ctx->return_size = retSize;
            memset(ctx->return_data, 0, sizeof(ctx->return_data));
            size_t to_copy = retSize > 524288 ? 524288 : retSize;
            memcpy(ctx->return_data, ctx->memory + retOffset, to_copy);
        }
        success_out->d[0] = 1;
        return true;
    }

    if (selector == 0xa5754463) { // Choa()
        u256_t mock_ret = {{ get_contract_address("choa"), 0, 0, 0 }};
        if (retOffset < 524288 && retSize > 0) {
            uint8_t temp_buf[32] = {0};
            write_u256_be(temp_buf, mock_ret);
            size_t src_offset = (retSize < 32) ? (32 - retSize) : 0;
            size_t copy_len = (retSize < 32) ? retSize : 32;
            for (size_t i = 0; i < copy_len; i++) {
                uint64_t dest = retOffset + i;
                if (dest < 524288) ctx->memory[dest] = temp_buf[src_offset + i];
            }
            ctx->return_size = retSize;
            memset(ctx->return_data, 0, sizeof(ctx->return_data));
            size_t to_copy = retSize > 524288 ? 524288 : retSize;
            memcpy(ctx->return_data, ctx->memory + retOffset, to_copy);
        }
        success_out->d[0] = 1;
        return true;
    }

    if (selector == 0x6fb7329c) { // Xiao()
        u256_t mock_ret = {{ get_contract_address("vmreq"), 0, 0, 0 }};
        if (retOffset < 524288 && retSize > 0) {
            uint8_t temp_buf[32] = {0};
            write_u256_be(temp_buf, mock_ret);
            size_t src_offset = (retSize < 32) ? (32 - retSize) : 0;
            size_t copy_len = (retSize < 32) ? retSize : 32;
            for (size_t i = 0; i < copy_len; i++) {
                uint64_t dest = retOffset + i;
                if (dest < 524288) ctx->memory[dest] = temp_buf[src_offset + i];
            }
            ctx->return_size = retSize;
            memset(ctx->return_data, 0, sizeof(ctx->return_data));
            size_t to_copy = retSize > 524288 ? 524288 : retSize;
            memcpy(ctx->return_data, ctx->memory + retOffset, to_copy);
        }
        success_out->d[0] = 1;
        return true;
    }

    if (selector == 0x604a6fa9) { // Random()
        uint64_t vmreq_addr = get_contract_address("vmreq");
        if (target_addr != vmreq_addr) {
            uint64_t rand_val = 50000 + (rand() % 50000);
            u256_t mock_ret = {{ rand_val, 0, 0, 0 }};
            if (retOffset < 524288 && retSize > 0) {
                uint8_t temp_buf[32] = {0};
                write_u256_be(temp_buf, mock_ret);
                size_t src_offset = (retSize < 32) ? (32 - retSize) : 0;
                size_t copy_len = (retSize < 32) ? retSize : 32;
                for (size_t i = 0; i < copy_len; i++) {
                    uint64_t dest = retOffset + i;
                    if (dest < 524288) ctx->memory[dest] = temp_buf[src_offset + i];
                }
                ctx->return_size = retSize;
                memset(ctx->return_data, 0, sizeof(ctx->return_data));
                size_t to_copy = retSize > 524288 ? 524288 : retSize;
                memcpy(ctx->return_data, ctx->memory + retOffset, to_copy);
            }
            success_out->d[0] = 1;
            return true;
        }
    }

    if (selector == 0xd9270a5a) { // GetLibraryAddress(string)
        bool is_strings = false;
        bool is_attribute = false;
        bool is_reactions = false;
        for (uint64_t idx = 4; idx + 7 <= argsSize; idx++) {
            if (memcmp(&ctx->memory[argsOffset + idx], "strings", 7) == 0) is_strings = true;
            if (memcmp(&ctx->memory[argsOffset + idx], "attribute", 9) == 0) is_attribute = true;
            if (memcmp(&ctx->memory[argsOffset + idx], "reactions", 9) == 0) is_reactions = true;
        }
        uint64_t lib_addr = 0;
        if (is_strings) {
            lib_addr = get_contract_address("strings");
        } else if (is_attribute) {
            lib_addr = get_contract_address("libattribute");
        } else if (is_reactions) {
            lib_addr = get_contract_address("corereactions");
        }
        if (lib_addr != 0) {
            u256_t mock_ret = {{ lib_addr, 0, 0, 0 }};
            if (retOffset < 524288 && retSize > 0) {
                uint8_t temp_buf[32] = {0};
                write_u256_be(temp_buf, mock_ret);
                size_t src_offset = (retSize < 32) ? (32 - retSize) : 0;
                size_t copy_len = (retSize < 32) ? retSize : 32;
                for (size_t i = 0; i < copy_len; i++) {
                    uint64_t dest = retOffset + i;
                    if (dest < 524288) ctx->memory[dest] = temp_buf[src_offset + i];
                }
                ctx->return_size = retSize;
                memset(ctx->return_data, 0, sizeof(ctx->return_data));
                size_t to_copy = retSize > 524288 ? 524288 : retSize;
                memcpy(ctx->return_data, ctx->memory + retOffset, to_copy);
            }
            success_out->d[0] = 1;
            return true;
        }
    }

    if (target_addr == 0x05) { // BigModExp
        uint8_t temp_calldata[192] = {0};
        uint64_t copy_calldata_size = argsSize < 192 ? argsSize : 192;
        for (uint64_t i = 0; i < copy_calldata_size; i++) {
            uint64_t src = argsOffset + i;
            temp_calldata[i] = (src < 524288) ? ctx->memory[src] : 0;
        }

        u256_t base = read_u256_be(temp_calldata + 96);
        u256_t exp = read_u256_be(temp_calldata + 128);
        u256_t mod = read_u256_be(temp_calldata + 160);
        u256_t res = u256_mod_pow(base, exp, mod);
        printf("[DEBUG_MATH] BigModExp: base=%lu exp=%lu mod=%lu res=%lu\n", base.d[0], exp.d[0], mod.d[0], res.d[0]);

        uint8_t res_bytes[32];
        write_u256_be(res_bytes, res);

        uint64_t copy_size = retSize < 32 ? retSize : 32;
        for (uint64_t i = 0; i < copy_size; i++) {
            uint64_t dest = retOffset + i;
            if (dest < 524288) {
                ctx->memory[dest] = res_bytes[i];
            }
        }
        ctx->return_size = 32;
        memcpy(ctx->return_data, res_bytes, 32);
        success_out->d[0] = 1;
        return true;
    }

    tsfi_qing_bst_populate_safe();
    CachedContract *target = tsfi_qing_bst_find_safe(target_addr);
    if (!target) {
        success_out->d[0] = 1;
        u256_t mock_ret = {{ 0x1000, 0, 0, 0 }};
        if (retOffset < 524288 && retSize > 0) {
            uint8_t temp_buf[32] = {0};
            write_u256_be(temp_buf, mock_ret);
            size_t src_offset = (retSize < 32) ? (32 - retSize) : 0;
            size_t copy_len = (retSize < 32) ? retSize : 32;
            for (size_t i = 0; i < copy_len; i++) {
                uint64_t dest = retOffset + i;
                if (dest < 524288) ctx->memory[dest] = temp_buf[src_offset + i];
            }
            ctx->return_size = retSize;
            memset(ctx->return_data, 0, sizeof(ctx->return_data));
            size_t to_copy = retSize > 524288 ? 524288 : retSize;
            memcpy(ctx->return_data, ctx->memory + retOffset, to_copy);
        }
        return true;
    }
    
    YulEvmContext *nested_ctx = calloc(1, sizeof(YulEvmContext));
    if (nested_ctx) {
        nested_ctx->calldatasize = (argsSize > 65536) ? 65536 : argsSize;
        for (uint64_t i = 0; i < nested_ctx->calldatasize; i++) {
            uint64_t src = argsOffset + i;
            nested_ctx->calldata[i] = (src < 524288) ? ctx->memory[src] : 0;
        }
        
        nested_ctx->storage_count = ctx->storage_count;
        memcpy(nested_ctx->storage_keys, ctx->storage_keys, sizeof(ctx->storage_keys));
        memcpy(nested_ctx->storage_vals, ctx->storage_vals, sizeof(ctx->storage_vals));
        nested_ctx->self_address = target_addr;

        if ((strcmp(target->name, "diyat") == 0 || target_addr == 858021) && nested_ctx->calldatasize >= 4) {
            uint32_t sel = ((uint32_t)nested_ctx->calldata[0] << 24) |
                            ((uint32_t)nested_ctx->calldata[1] << 16) |
                            ((uint32_t)nested_ctx->calldata[2] << 8)  |
                            ((uint32_t)nested_ctx->calldata[3]);
            if (sel == 0x904a4bc3 && nested_ctx->calldatasize >= 68) {
                uint64_t taxAmount = 0;
                for (int i = 0; i < 8; i++) {
                    taxAmount = (taxAmount << 8) | nested_ctx->calldata[60 + i];
                }
                g_transaction_diyat_tax_total += taxAmount;
            }
        }
        nested_ctx->caller_address.d[0] = ctx->self_address;
        nested_ctx->caller_address.d[1] = 0;
        nested_ctx->caller_address.d[2] = 0;
        nested_ctx->caller_address.d[3] = 0;
        
        bool run_ok = run_yul_bytecode(nested_ctx, target->bytecode, target->size, target->name);
        
        if (run_ok && !nested_ctx->reverted) {
            uint64_t copy_size = retSize;
            if (copy_size > nested_ctx->return_size) {
                copy_size = nested_ctx->return_size;
            }
            for (uint64_t i = 0; i < copy_size; i++) {
                uint64_t dest = retOffset + i;
                if (dest < 524288) {
                    ctx->memory[dest] = nested_ctx->return_data[i];
                }
            }
            
            for (int i = 0; i < nested_ctx->storage_count; i++) {
                u256_t raw_key = nested_ctx->storage_keys[i];
                bool found = false;
                for (int j = 0; j < ctx->storage_count; j++) {
                    if (u256_eq(ctx->storage_keys[j], raw_key)) {
                        ctx->storage_vals[j] = nested_ctx->storage_vals[i];
                        ctx->storage_addrs[j] = nested_ctx->storage_addrs[i];
                        found = true;
                        break;
                    }
                }
                if (!found && ctx->storage_count < 32768) {
                    ctx->storage_keys[ctx->storage_count] = raw_key;
                    ctx->storage_vals[ctx->storage_count] = nested_ctx->storage_vals[i];
                    ctx->storage_addrs[ctx->storage_count] = nested_ctx->storage_addrs[i];
                    ctx->storage_count++;
                }
            }
            
            ctx->return_size = nested_ctx->return_size;
            if (ctx->return_size > 524288) ctx->return_size = 524288;
            memcpy(ctx->return_data, nested_ctx->return_data, ctx->return_size);
        } else {
            printf("[DEBUG_EVM] Nested call to %s reverted/failed. run_ok=%d reverted=%d return_size=%zu\n", target->name, run_ok, nested_ctx->reverted, nested_ctx->return_size);
            if (nested_ctx->return_size > 0) {
                printf("  Revert data hex: ");
                for (size_t i = 0; i < nested_ctx->return_size && i < 128; i++) {
                    printf("%02x", nested_ctx->return_data[i]);
                }
                printf("\n");
            }
            success_out->d[0] = 0;
            if (retOffset < 524288 && retSize > 0 && nested_ctx->return_size > 0) {
                size_t copy_size = retSize < nested_ctx->return_size ? retSize : nested_ctx->return_size;
                for (size_t i = 0; i < copy_size; i++) {
                    uint64_t dest = retOffset + i;
                    if (dest < 524288) ctx->memory[dest] = nested_ctx->return_data[i];
                }
                ctx->return_size = nested_ctx->return_size;
                memcpy(ctx->return_data, nested_ctx->return_data, ctx->return_size);
            } else {
                ctx->return_size = 0;
            }
        }
        free(nested_ctx);
    } else {
        success_out->d[0] = 0;
    }
    return true;
}
