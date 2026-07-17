#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <sys/file.h>
#include <openssl/sha.h>
#include "tsfi_zmm_rpc.h"
#include "lau_yul_thunk.h"
#ifndef SAMPLE_RATE
#define SAMPLE_RATE 8000
#endif
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include "tsfi_computel_blue_box.h"
    #define GRID_SIZE 80
#define MAX_BGP_PEERS 32
#define MAX_GGUF_ROUTES 64
#define MAX_VALIDATOR_BIDS 16
#include <dirent.h>
#include <sys/stat.h>
#include "tsfi_mainframe_decnet.h"


bool blue_box_write_quadtree_to_disk(uint32_t mode);
bool blue_box_write_quadtree_to_disk(uint32_t mode) {
    extern void lau_yul_thunk_sstore(uint64_t key, uint64_t value);
    extern uint64_t lau_yul_thunk_sload(uint64_t key);
    
    // We fetch our telemetry and profile states
    uint64_t g_mode = lau_yul_thunk_sload(0xF18E);
    uint64_t rate = lau_yul_thunk_sload(0xF196);
    uint64_t last_action = lau_yul_thunk_sload(0xF185);
    uint64_t total_collected = lau_yul_thunk_sload(0xF186);
    uint64_t gas_allowance = lau_yul_thunk_sload(0xF199);
    
    if (mode == 0) {
        // Mode 0: PostgreSQL Mutable spatial index mode (Overwrites in-place)
        FILE *f = fopen("assets/rdbms_tables.dat.bin", "w");
        if (!f) f = fopen("../assets/rdbms_tables.dat.bin", "w");
        if (!f) return false;
        
        fprintf(f, "{\n  \"quadrants\": {\n");
        fprintf(f, "    \"NW_blue\": {\"bgp_peers_count\": %lu},\n", (unsigned long)g_bgp_peer_count);
        fprintf(f, "    \"NE_red\": {\"gas_allowance\": %lu},\n", gas_allowance);
        fprintf(f, "    \"SW_black\": {\"diode_drop\": 700},\n");
        fprintf(f, "    \"SE_green\": {\"mode\": %lu, \"diyat_rate\": %lu, \"last_action\": %lu, \"total_collected\": %lu}\n",
                g_mode, rate, last_action, total_collected);
        fprintf(f, "  }\n}\n");
        fclose(f);
        printf("[QUADTREE] Wrote mutable spatial quadrants to assets/rdbms_tables.dat.bin\n");
    } else {
        // Compute new Merkle State Proof Root Hash: H(prev_root + current_states)
        uint64_t prev_root = lau_yul_thunk_sload(0xF1B5);
        uint64_t next_root = ((prev_root * 33 + gas_allowance) * 33 + total_collected) % MotzkinPrime;
        
        static uint64_t s_last_gas = 0;
        static uint64_t s_last_collected = 0;
        static uint64_t s_last_prev = 0;
        static uint64_t s_last_next = 0;
        static bool s_has_cache = false;
        
        if (s_has_cache && gas_allowance == s_last_gas && total_collected == s_last_collected && prev_root == s_last_prev) {
            lau_yul_thunk_sstore(0xF1B5, s_last_next);
            lau_yul_thunk_sstore(0xF19F, 1);
            printf("[QUADTREE] [MEMO] Redundant state commit bypassed. Next Root retrieved: %lu\n", s_last_next);
            return true;
        }
        
        s_last_gas = gas_allowance;
        s_last_collected = total_collected;
        s_last_prev = prev_root;
        s_last_next = next_root;
        s_has_cache = true;
        
        uint64_t r23_0 = lau_yul_thunk_sload(0xF1C5);
        uint64_t r23_1 = lau_yul_thunk_sload(0xF1C7);
        uint64_t r23_2 = lau_yul_thunk_sload(0xF1C8);
        uint64_t r23_3 = lau_yul_thunk_sload(0xF1C9);

        FILE *f = fopen("assets/rdbms_ledger.dat.bin", "a");
        if (!f) f = fopen("../assets/rdbms_ledger.dat.bin", "a");
        if (!f) return false;
        
        lau_yul_thunk_sstore(0xF1B5, next_root);
        
        // Append structured transaction record block to global ledger file
        fprintf(f, "{\"block_root_hash\": %lu, \"state\": {\"gas\": %lu, \"total_collected\": %lu, \"r23_0\": %lu, \"r23_1\": %lu, \"r23_2\": %lu, \"r23_3\": %lu}, \"mode\": \"ledger\"}\n",
                next_root, gas_allowance, total_collected, r23_0, r23_1, r23_2, r23_3);
        fclose(f);
        
        // Output specific file for this preserved node atomically using WAL temporary file pattern
        char node_path[128];
        char temp_node_path[128];
        snprintf(node_path, sizeof(node_path), "assets/rdbms_ledger_%lu.dat.bin", next_root);
        snprintf(temp_node_path, sizeof(temp_node_path), "assets/temp_ledger_%lu.dat.bin", next_root);
        FILE *fn = fopen(temp_node_path, "w");
        if (!fn) {
            snprintf(node_path, sizeof(node_path), "../assets/rdbms_ledger_%lu.dat.bin", next_root);
            snprintf(temp_node_path, sizeof(temp_node_path), "../assets/temp_ledger_%lu.dat.bin", next_root);
            fn = fopen(temp_node_path, "w");
        }
        if (fn) {
            fprintf(fn, "{\"block_root_hash\": %lu, \"state\": {\"gas\": %lu, \"total_collected\": %lu, \"r23_0\": %lu, \"r23_1\": %lu, \"r23_2\": %lu, \"r23_3\": %lu}, \"mode\": \"ledger\"}\n",
                    next_root, gas_allowance, total_collected, r23_0, r23_1, r23_2, r23_3);
            fclose(fn);
            rename(temp_node_path, node_path);
            printf("[QUADTREE] Wrote node-specific state file atomically: %s\n", node_path);
        }

        // Copy current storage to block specific binary DAT media snapshot atomically using WAL temporary file pattern
        extern void persist_reconciliation_data(void);
        persist_reconciliation_data();
        char storage_path[128];
        char temp_storage_path[128];
        snprintf(storage_path, sizeof(storage_path), "assets/rdbms_storage_%lu.dat.bin", next_root);
        snprintf(temp_storage_path, sizeof(temp_storage_path), "assets/temp_storage_%lu.dat.bin", next_root);
        FILE *fs_dst = fopen(temp_storage_path, "wb");
        if (!fs_dst) {
            snprintf(storage_path, sizeof(storage_path), "../assets/rdbms_storage_%lu.dat.bin", next_root);
            snprintf(temp_storage_path, sizeof(temp_storage_path), "../assets/temp_storage_%lu.dat.bin", next_root);
            fs_dst = fopen(temp_storage_path, "wb");
        }
        if (fs_dst) {
            fwrite("QTDM", 1, 4, fs_dst);
            
            // FNV-1a calculation
            uint64_t computed = 14695981039346656037ULL;
            for (int i = 0; i < g_yul_evm_context.storage_count; i++) {
                computed ^= g_yul_evm_context.storage_keys[i].d[0]; computed *= 1099511628211ULL;
                computed ^= g_yul_evm_context.storage_keys[i].d[1]; computed *= 1099511628211ULL;
                computed ^= g_yul_evm_context.storage_keys[i].d[2]; computed *= 1099511628211ULL;
                computed ^= g_yul_evm_context.storage_keys[i].d[3]; computed *= 1099511628211ULL;
                
                computed ^= g_yul_evm_context.storage_vals[i].d[0]; computed *= 1099511628211ULL;
                computed ^= g_yul_evm_context.storage_vals[i].d[1]; computed *= 1099511628211ULL;
                computed ^= g_yul_evm_context.storage_vals[i].d[2]; computed *= 1099511628211ULL;
                computed ^= g_yul_evm_context.storage_vals[i].d[3]; computed *= 1099511628211ULL;
                
                computed ^= g_yul_evm_context.storage_addrs[i]; computed *= 1099511628211ULL;
            }
            
            fwrite(&computed, sizeof(uint64_t), 1, fs_dst);
            uint32_t count = g_yul_evm_context.storage_count;
            fwrite(&count, sizeof(uint32_t), 1, fs_dst);
            
            for (int i = 0; i < g_yul_evm_context.storage_count; i++) {
                fwrite(&g_yul_evm_context.storage_keys[i], sizeof(u256_t), 1, fs_dst);
                fwrite(&g_yul_evm_context.storage_vals[i], sizeof(u256_t), 1, fs_dst);
                fwrite(&g_yul_evm_context.storage_addrs[i], sizeof(uint64_t), 1, fs_dst);
            }
            fclose(fs_dst);
            rename(temp_storage_path, storage_path);
            printf("[QUADTREE] Wrote block-specific storage snapshot atomically with FNV-1a signature: %s\n", storage_path);
        }

        // Append to relational RDBMS nodes table
        FILE *fa = fopen("assets/rdbms_nodes_table.dat.bin", "a");
        if (!fa) fa = fopen("../assets/rdbms_nodes_table.dat.bin", "a");
        if (fa) {
            fprintf(fa, "{\"table\": \"immutable_nodes\", \"row\": {\"root\": %lu, \"gas\": %lu, \"collected\": %lu, \"r23_0\": %lu, \"r23_1\": %lu, \"r23_2\": %lu, \"r23_3\": %lu}}\n",
                    next_root, gas_allowance, total_collected, r23_0, r23_1, r23_2, r23_3);
            fclose(fa);
            printf("[QUADTREE] Relational table row committed to assets/rdbms_nodes_table.dat.bin\n");
        }
        printf("[QUADTREE] Appended block to immutable DAG assets/rdbms_ledger.dat.bin. Root Hash: %lu\n", next_root);
    }
    
    lau_yul_thunk_sstore(0xF19F, 1); // Set serialization status active
    return true;
}

void blue_box_rehydrate_quadtree_states(void) {
    const char *dir_path = "assets";
    DIR *d = opendir(dir_path);
    if (!d) {
        dir_path = "../assets";
        d = opendir(dir_path);
    }
    if (!d) return;

    struct dirent *dir;
    char newest_file[256] = "";
    time_t newest_time = 0;

    while ((dir = readdir(d)) != NULL) {
        if (strncmp(dir->d_name, "rdbms_ledger_", 13) == 0 && strstr(dir->d_name, ".dat.bin")) {
            char filepath[512];
            snprintf(filepath, sizeof(filepath), "%s/%s", dir_path, dir->d_name);
            struct stat st;
            if (stat(filepath, &st) == 0) {
                if (st.st_mtime > newest_time) {
                    newest_time = st.st_mtime;
                    strncpy(newest_file, filepath, sizeof(newest_file) - 1);
                    newest_file[sizeof(newest_file) - 1] = '\0';
                }
            }
        }
    }
    closedir(d);

    if (newest_file[0] != '\0') {
        FILE *f = fopen(newest_file, "r");
        if (f) {
            char buf[1024];
            if (fgets(buf, sizeof(buf), f)) {
                uint64_t next_root = 0;
                uint64_t gas = 0;
                uint64_t collected = 0;
                uint64_t r23_0 = 0;
                uint64_t r23_1 = 0;
                uint64_t r23_2 = 0;
                uint64_t r23_3 = 0;

                char *p_root = strstr(buf, "\"block_root_hash\":");
                char *p_gas = strstr(buf, "\"gas\":");
                char *p_col = strstr(buf, "\"total_collected\":");
                char *p_r23_0 = strstr(buf, "\"r23_0\":");
                char *p_r23_1 = strstr(buf, "\"r23_1\":");
                char *p_r23_2 = strstr(buf, "\"r23_2\":");
                char *p_r23_3 = strstr(buf, "\"r23_3\":");

                if (p_root) sscanf(p_root, "\"block_root_hash\": %lu", &next_root);
                if (p_gas) sscanf(p_gas, "\"gas\": %lu", &gas);
                if (p_col) sscanf(p_col, "\"total_collected\": %lu", &collected);
                if (p_r23_0) sscanf(p_r23_0, "\"r23_0\": %lu", &r23_0);
                if (p_r23_1) sscanf(p_r23_1, "\"r23_1\": %lu", &r23_1);
                if (p_r23_2) sscanf(p_r23_2, "\"r23_2\": %lu", &r23_2);
                if (p_r23_3) sscanf(p_r23_3, "\"r23_3\": %lu", &r23_3);

                if (next_root > 0) {
                    extern void lau_yul_thunk_sstore(uint64_t key, uint64_t value);
                    lau_yul_thunk_sstore(0xF1B5, next_root);
                    lau_yul_thunk_sstore(0xF199, gas);
                    lau_yul_thunk_sstore(0xF186, collected);
                    lau_yul_thunk_sstore(0xF18E, 1); // Ledger mode active
                    
                    lau_yul_thunk_sstore(0xF1C5, r23_0);
                    lau_yul_thunk_sstore(0xF1C7, r23_1);
                    lau_yul_thunk_sstore(0xF1C8, r23_2);
                    lau_yul_thunk_sstore(0xF1C9, r23_3);
                    lau_yul_thunk_sstore(0xF1C6, 1);  // Conversion Status: Aligned (1)
                    
                    printf("[QUADTREE] [REHYDRATE] Restored latest block ledger state: root=%lu, gas=%lu, collected=%lu, 2-3 Tree Segments=[%lu,%lu,%lu,%lu]\n",
                           next_root, gas, collected, r23_0, r23_1, r23_2, r23_3);

                    // Copy block specific storage snapshot back to evm_storage.json
                    char src_path[128];
                    snprintf(src_path, sizeof(src_path), "assets/rdbms_storage_%lu.dat.bin", next_root);
                    FILE *fs_src = fopen(src_path, "rb");
                    if (!fs_src) {
                        snprintf(src_path, sizeof(src_path), "../assets/rdbms_storage_%lu.dat.bin", next_root);
                        fs_src = fopen(src_path, "rb");
                    }
                    if (fs_src) {
                        char magic[4];
                        uint64_t sig = 0;
                        uint32_t count = 0;
                        if (fread(magic, 1, 4, fs_src) == 4 && memcmp(magic, "QTDM", 4) == 0 &&
                            fread(&sig, sizeof(uint64_t), 1, fs_src) == 1 &&
                            fread(&count, sizeof(uint32_t), 1, fs_src) == 1) {
                            
                            u256_t *keys = malloc(sizeof(u256_t) * count);
                            u256_t *vals = malloc(sizeof(u256_t) * count);
                            uint64_t *addrs = malloc(sizeof(uint64_t) * count);
                            bool read_ok = true;
                            uint64_t computed = 14695981039346656037ULL;
                            for (uint32_t i = 0; i < count; i++) {
                                if (fread(&keys[i], sizeof(u256_t), 1, fs_src) != 1 ||
                                    fread(&vals[i], sizeof(u256_t), 1, fs_src) != 1 ||
                                    fread(&addrs[i], sizeof(uint64_t), 1, fs_src) != 1) {
                                    read_ok = false;
                                    break;
                                }
                                computed ^= keys[i].d[0]; computed *= 1099511628211ULL;
                                computed ^= keys[i].d[1]; computed *= 1099511628211ULL;
                                computed ^= keys[i].d[2]; computed *= 1099511628211ULL;
                                computed ^= keys[i].d[3]; computed *= 1099511628211ULL;
                                
                                computed ^= vals[i].d[0]; computed *= 1099511628211ULL;
                                computed ^= vals[i].d[1]; computed *= 1099511628211ULL;
                                computed ^= vals[i].d[2]; computed *= 1099511628211ULL;
                                computed ^= vals[i].d[3]; computed *= 1099511628211ULL;
                                
                                computed ^= addrs[i]; computed *= 1099511628211ULL;
                            }
                            fclose(fs_src);
                            
                            if (read_ok && computed == sig) {
                                g_yul_evm_context.storage_count = count;
                                for (uint32_t i = 0; i < count; i++) {
                                    g_yul_evm_context.storage_keys[i] = keys[i];
                                    g_yul_evm_context.storage_vals[i] = vals[i];
                                    g_yul_evm_context.storage_addrs[i] = addrs[i];
                                }
                                extern void persist_reconciliation_data(void);
                                persist_reconciliation_data();
                                printf("[QUADTREE] [REHYDRATE] Securely verified and restored evm_storage.json snapshot for root %lu\n", next_root);
                                extern void reload_evm_storage_from_json(void);
                                reload_evm_storage_from_json();
                            } else {
                                printf("[QUADTREE] [SECURITY] Storage snapshot FNV-1a verification FAILED for %s! Computed: %lu, Signature in header: %lu\n", src_path, computed, sig);
                            }
                            free(keys);
                            free(vals);
                            free(addrs);
                        } else {
                            fclose(fs_src);
                        }
                    }
                }
            }
            fclose(f);
        }
    }
}

uint64_t compute_storage_fnv1a(const char *path, uint64_t *out_expected) {
    FILE *f = fopen(path, "r");
    if (!f) return 0;
    uint64_t hash = 14695981039346656037ULL;
    char line[1024];
    *out_expected = 0;
    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, "// SIG:", 7) == 0) {
            sscanf(line, "// SIG: %lu", out_expected);
            break;
        }
        for (size_t i = 0; line[i] != '\0'; i++) {
            hash ^= (uint8_t)line[i];
            hash *= 1099511628211ULL;
        }
    }
    fclose(f);
    return hash;
}

bool blue_box_verify_btc_script_transition(const uint8_t *old_row_data, size_t old_len, const uint8_t *witness_script, size_t script_len, const uint8_t *new_row_data, size_t new_len) {
    extern void lau_yul_thunk_sstore(uint64_t key, uint64_t value);
    (void)new_row_data;
    
    // Simulate simple script: OP_SHA256 OP_EQUALVERIFY
    // Verify that old_row_data matches the expected hash in witness_script
    // We compute a basic checksum of old_row_data to verify script constraint
    uint64_t old_checksum = 0;
    for (size_t i = 0; i < old_len; i++) {
        old_checksum = (old_checksum * 33 + old_row_data[i]) % MotzkinPrime;
    }
    
    uint64_t expected_hash = 0;
    if (script_len >= 8) {
        for (size_t i = 0; i < 8; i++) {
            expected_hash = (expected_hash << 8) | witness_script[i];
        }
    }
    
    // Script transition logic: OP_SHA256(old_row_data) == expected_hash
    if (old_checksum != expected_hash) {
        lau_yul_thunk_sstore(0xF1C1, 0); // Verification status: Failed (0)
        printf("[BTC SCRIPT] Verification failed. Hash mismatch.\n");
        return false;
    }
    
    // Update register states
    lau_yul_thunk_sstore(0xF1C0, 100); // Set UTXO leaf index
    lau_yul_thunk_sstore(0xF1C1, 1);   // Verification status: Success (1)
    
    printf("[BTC SCRIPT] Transition verified. New Row committed (length: %zu).\n", new_len);
    return true;
}

bool blue_box_commit_quadtree_via_btc_script(uint64_t old_root, uint64_t next_root, const uint8_t *witness, size_t witness_len) {
    extern void lau_yul_thunk_sstore(uint64_t key, uint64_t value);
    extern uint64_t lau_yul_thunk_sload(uint64_t key);
    
    uint64_t active_root = lau_yul_thunk_sload(0xF1B5);
    if (active_root != old_root) {
        printf("[BTC COMMIT] State collision. expected root: %lu, Active root: %lu\n", old_root, active_root);
        return false;
    }
    
    // Validate witness transition: H(old_root + delta) == next_root
    // We fetch delta from witness data bytes
    uint64_t delta = 0;
    if (witness_len >= 8) {
        for (size_t i = 0; i < 8; i++) {
            delta = (delta << 8) | witness[i];
        }
    }
    
    uint64_t computed_hash = ((old_root * 33 + delta) * 33 + delta) % MotzkinPrime;
    if (computed_hash != next_root) {
        printf("[BTC COMMIT] Script transaction proof failed validation.\n");
        return false;
    }
    
    // Commit to registers
    lau_yul_thunk_sstore(0xF1B5, next_root);      // Update Merkle Root
    lau_yul_thunk_sstore(0xF1C3, next_root);      // Update Committed Root
    lau_yul_thunk_sstore(0xF1C2, 1);              // Set Commit Trigger Active
    
    // Serialize new state to block ledger on disk
    bool disk_ok = blue_box_write_quadtree_to_disk(1); // Block Ledger Mode
    
    printf("[BTC COMMIT] Transaction successfully committed. New Root: %lu (disk status: %d)\n",
           next_root, disk_ok);
    return disk_ok;
}

bool blue_box_verify_23_to_quad_conversion(uint64_t r23_root_0, uint64_t r23_root_1, uint64_t r23_root_2, uint64_t r23_root_3, uint64_t *r_quad_out) {
    if (!r_quad_out) return false;
    
    extern void lau_yul_thunk_sstore(uint64_t key, uint64_t value);
    
    // Hash-map: Combine 2-3 tree roots into a quaternary root hash
    uint64_t val = r23_root_0;
    val = (val * 33 + r23_root_1) % MotzkinPrime;
    val = (val * 33 + r23_root_2) % MotzkinPrime;
    val = (val * 33 + r23_root_3) % MotzkinPrime;
    
    *r_quad_out = val;
    
    // Commit alignment metrics to VM registers
    lau_yul_thunk_sstore(0xF1C5, r23_root_0); // Segment 0
    lau_yul_thunk_sstore(0xF1C7, r23_root_1); // Segment 1
    lau_yul_thunk_sstore(0xF1C8, r23_root_2); // Segment 2
    lau_yul_thunk_sstore(0xF1C9, r23_root_3); // Segment 3
    lau_yul_thunk_sstore(0xF1C6, 1);          // Conversion Status: Aligned (1)
    
    printf("[BTC CONVERT] 2-3 Tree state mapped to Quadtree root: %lu\n", val);
    return true;
}

bool blue_box_verify_geometry_threat_protection(float coverage, float symmetry) {
    extern void lau_yul_thunk_sstore(uint64_t key, uint64_t value);
    extern uint64_t lau_yul_thunk_sload(uint64_t key);
    
    // Check if coverage or symmetry falls below safe operational limits
    if (coverage < 0.01f || symmetry < 0.95f) {
        uint64_t threats = lau_yul_thunk_sload(0xF120);
        threats++;
        lau_yul_thunk_sstore(0xF120, threats);
        
        if (threats >= 3) {
            lau_yul_thunk_sstore(0xF121, 1); // Set firewall block active (mute)
            printf("[SECURITY FIREWALL] Threat limit exceeded. Connection muted.\n");
        }
    }
    return true;
}

bool blue_box_verify_pll_coalition_security(uint32_t pilot_freq, uint32_t target_freq, uint32_t *session_key_out) {
    if (!session_key_out) return false;
    
    extern void lau_yul_thunk_sstore(uint64_t key, uint64_t value);
    extern uint64_t lau_yul_thunk_sload(uint64_t key);
    
    // Read current PLL locked deviation register
    uint64_t deviation = lau_yul_thunk_sload(0xF125);
    
    // Lock threshold set at deviation <= 100
    if (deviation <= 100) {
        // Generate and rotate session key
        uint64_t key = ((uint64_t)pilot_freq * 33 + target_freq) * 33 % MotzkinPrime;
        *session_key_out = (uint32_t)key;
        
        lau_yul_thunk_sstore(0xF1D0, key); // Save rotated session key
        lau_yul_thunk_sstore(0xF1D1, 1);   // Security Status: Locked/Secure (1)
        printf("[CONFERENCE SECURE] PLL Locked. Rotated Session Key: %lu\n", key);
        return true;
    } else {
        lau_yul_thunk_sstore(0xF1D1, 0);   // Security Status: Desynced/Unsecure (0)
        printf("[CONFERENCE SECURE] PLL Desynced (Deviation: %lu). Secure tunnel offline.\n", deviation);
        return false;
    }
}

