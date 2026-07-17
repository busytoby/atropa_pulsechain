#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#include <unistd.h>
#include <pulse/simple.h>
#include <pulse/error.h>
#include <math.h>
#include "tsfi_zmm_rpc.h"
#include "tsfi_trie_dispatcher.h"
#include "tsfi_tst.h"
#include "tsfi_opt_zmm.h"
#include "tsfi_genetic.h"
#include "tsfi_svdag.h"
#include "tsfi_io.h"
#include "lau_memory.h"
#include "tsfi_block_monitor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#define MAX_DILEMMA_LOGS 128
#define MAX_STORE_KEYS 1024
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#define SAMPLE_RATE 8000
#define MAX_ARRANGEMENT 64
#define MAX_STEPS 32
#define MAX_REGISTERED_OPERATORS 8


int tsfi_zmm_rpc_dispatch_manifold(TsfiZmmVmState *state, int method_type, const char *min_ptr, char *output_buf, size_t out_max, int id) {
    if (method_type == 6) { // GENETIC.BENCHMARK
        int iterations = 50000;
        char *iter_key = strstr(min_ptr, "\"iterations\"");
        if (iter_key) {
            char *start = strchr(iter_key, ':');
            if (start) iterations = atoi(start + 1);
        }
        if (iterations <= 0) iterations = 50000;
        char report_buf[4096];
        tsfi_genetic_benchmark_run(iterations, report_buf, sizeof(report_buf));
        int n = snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": {\"report\": \"");
        char *optr = output_buf + n;
        size_t rem = out_max - n;
        for (char *c = report_buf; *c && rem > 16; c++) {
            if (*c == '\n') { *optr++ = '\\'; *optr++ = 'n'; rem -= 2; }
            else if (*c == '\t') { *optr++ = '\\'; *optr++ = 't'; rem -= 2; }
            else if (*c == '"') { *optr++ = '\\'; *optr++ = '"'; rem -= 2; }
            else if (*c == '\\') { *optr++ = '\\'; *optr++ = '\\'; rem -= 2; }
            else { *optr++ = *c; rem--; }
        }
        snprintf(optr, rem, "\"}, \"id\": %d}\n", id);
        return 1;
    }
    if (method_type == 11) { // GENETIC.ESTABLISH_LLM
        // This method bootstraps an internal "Model" structure (YANG) via crossover
        // of a baseline Architecture (Parent A) and a Strategic Input (Parent B).
        
        char arch[1024] = "WLOAD W0, 0.5\nWLOAD W1, 0.1\nWADD W2, W0, W1\nWSTORE W2, 0";
        char strategy[1024] = "WLOAD W0, 0.8\nWLOAD W1, 0.2\nWMUL W2, W0, W1\nWSTORE W2, 0";
        
        extract_json_string(min_ptr, "\"architecture\"", arch, sizeof(arch));
        extract_json_string(min_ptr, "\"strategy\"", strategy, sizeof(strategy));

        // Setup Parents
        GeneticNode* ParentA = (GeneticNode*)lau_malloc_wired(sizeof(GeneticNode));
        ParentA->type = GENETIC_TYPE_YI; ParentA->dys_ptr = allocYI();
        memset(ParentA->dys_ptr, 0, sizeof(struct YI));
        ((struct YI*)ParentA->dys_ptr)->Xi = tsfi_bn_alloc();
        tsfi_bn_from_bytes(((struct YI*)ParentA->dys_ptr)->Xi, (const uint8_t*)arch, strlen(arch));

        GeneticNode* ParentB = (GeneticNode*)lau_malloc_wired(sizeof(GeneticNode));
        ParentB->type = GENETIC_TYPE_YI; ParentB->dys_ptr = allocYI();
        memset(ParentB->dys_ptr, 0, sizeof(struct YI));
        ((struct YI*)ParentB->dys_ptr)->Xi = tsfi_bn_alloc();
        tsfi_bn_from_bytes(((struct YI*)ParentB->dys_ptr)->Xi, (const uint8_t*)strategy, strlen(strategy));

        // Perform Crossover In-Place (Zero-Copy)
        GeneticNode* InternalLLM = (GeneticNode*)lau_malloc_wired(sizeof(GeneticNode));
        InternalLLM->type = GENETIC_TYPE_YI; InternalLLM->dys_ptr = allocYI();
        memset(InternalLLM->dys_ptr, 0, sizeof(struct YI));
        Fourier_UniversalCrossover_InPlace(ParentA, ParentB, InternalLLM);

        // Extract resulting bytecode
        char evolved_code[1024] = {0};
        tsfi_bn_to_bytes(((struct YI*)InternalLLM->dys_ptr)->Xi, (uint8_t*)evolved_code, sizeof(evolved_code) - 1);
        
        // Return status and the evolved "Model" bytecode
        snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": {\"llm_id\": \"%s\", \"bytecode\": \"%s\"}, \"id\": %d}\n", 
                 InternalLLM->generation_id, evolved_code, id);

        // Cleanup
        freeYI((struct YI*)ParentA->dys_ptr); lau_free(ParentA);
        freeYI((struct YI*)ParentB->dys_ptr); lau_free(ParentB);
        if (InternalLLM->type == GENETIC_TYPE_YI) freeYI((struct YI*)InternalLLM->dys_ptr);
        else freeYANG((struct YANG*)InternalLLM->dys_ptr);
        lau_free(InternalLLM);
        return 1;
        
    }
    if (method_type == 19) { // GENETIC.AUTONOMOUS_OPTIMIZE
        // Autonomous optimization script wrapper
        char target_file[1024] = "tests/ballet_elancer.c";
        extract_json_string(min_ptr, "\"target\"", target_file, sizeof(target_file));
        
        char script_path[256];
        snprintf(script_path, sizeof(script_path), "/tmp/tsfi_auto_opt_%d.py", getpid());
        
        FILE *py = fopen(script_path, "w");
        if (py) {
            fprintf(py, 
                "import subprocess, re, sys\n"
                "target = '%s'\n"
                "def run_bench():\n"
                "    r = subprocess.run(['./bin/' + target.split('/')[-1].replace('.c', '')], capture_output=True, text=True)\n"
                "    m = re.search(r'Throughput: ([\\d\\.]+) GFLOPS', r.stdout)\n"
                "    return float(m.group(1)) if m else 0.0\n"
                "best = run_bench()\n"
                "with open(target, 'r') as f: content = f.read()\n"
                "m = re.search(r'__m512 v_c = _mm512_set1_ps\\(([\\d\\.]+)f\\);', content)\n"
                "if m:\n"
                "    cur = float(m.group(1))\n"
                "    for step in [0.0001, -0.0001]:\n"
                "        new_val = cur + step\n"
                "        new_content = re.sub(r'__m512 v_c = _mm512_set1_ps\\([\\d\\.]+f\\);', f'__m512 v_c = _mm512_set1_ps({new_val:.4f}f);', content)\n"
                "        with open(target, 'w') as f: f.write(new_content)\n"
                "        subprocess.run(['make', 'bin/' + target.split('/')[-1].replace('.c', '')], capture_output=True)\n"
                "        g = run_bench()\n"
                "        if g > best:\n"
                "            best = g\n"
                "            content = new_content\n"
                "            break\n"
                "        else:\n"
                "            with open(target, 'w') as f: f.write(content)\n"
                "            subprocess.run(['make', 'bin/' + target.split('/')[-1].replace('.c', '')], capture_output=True)\n"
                "print(best)\n", target_file);
            fclose(py);
            
            char cmd[512];
            snprintf(cmd, sizeof(cmd), "python3 %s", script_path);
            FILE *cmd_out = popen(cmd, "r");
            float best_gflops = 0.0f;
            if (cmd_out) {
                if (fscanf(cmd_out, "%f", &best_gflops) != 1) {
                    best_gflops = 0.0f;
                }
                pclose(cmd_out);
            }
            unlink(script_path);
            
            snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": {\"status\": \"optimized\", \"target\": \"%s\", \"best_gflops\": %.2f}, \"id\": %d}\n", target_file, best_gflops, id);
        } else {
             snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"error\": {\"code\": -32000, \"message\": \"Failed to create optimization script\"}, \"id\": %d}\n", id);
        }
        return 1;
    
    }
    if (method_type == 20) { // manifold.swap_asset
        int slot = extract_json_int(min_ptr, "\"slot\"", -1);
        int type = extract_json_int(min_ptr, "\"type\"", 0);
        char addr_str[64];
        if (slot >= 0 && slot < 32 && extract_json_string(min_ptr, "\"address\"", addr_str, sizeof(addr_str))) {
            uintptr_t addr = (uintptr_t)strtoull(addr_str, NULL, 0);
            if (state->manifest) {
                state->manifest->slots[slot].type = (ZmmType)type;
                state->manifest->slots[slot].data_ptr = (void*)addr;
                snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": \"Swapped slot %d to %p\", \"id\": %d}\n", slot, (void*)addr, id);
                return 1;
            }
        }
    }
    if (method_type == 21) { // manifold.set_kernel
        char name[64];
        if (extract_json_string(min_ptr, "\"name\"", name, sizeof(name)) && state->manifest) {
            void (*k)(void*, ZmmSynapse*) = NULL;
            if (strcmp(name, "blue_teddy_bear") == 0) k = tsfi_kernel_blue_teddy_bear;
            else if (strcmp(name, "low_density") == 0) k = state->manifest->kernel_low_density;
            else if (strcmp(name, "high_density") == 0) k = state->manifest->kernel_high_density;
            
            if (k) {
                state->manifest->micro_kernel = k;
                snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": \"Kernel set to %s\", \"id\": %d}\n", name, id);
                return 1;
            }
        }
    }
    if (method_type == 22) { // manifold.set_active_mask
        int mask = extract_json_int(min_ptr, "\"mask\"", 0);
        if (state->manifest) {
            state->manifest->active_mask = (uint32_t)mask;
            snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": \"Active mask set to 0x%08X\", \"id\": %d}\n", state->manifest->active_mask, id);
            return 1;
        }
    }
    if (method_type == 23) { // manifold.set_secret
        int slot = extract_json_int(min_ptr, "\"slot\"", 0);
        char val_str[64];
        if (extract_json_string(min_ptr, "\"value\"", val_str, sizeof(val_str)) && state->manifest) {
            float val = strtof(val_str, NULL);
            if (slot >= 0 && slot < 32 && state->manifest->slots[slot].data_ptr) {
                // If it's a TeddyBear, we can inject into spectral_shift
                if (state->manifest->slots[slot].type == ZMM_TYPE_MASS) {
                    TeddyBear *b = (TeddyBear*)state->manifest->slots[slot].data_ptr;
                    b->spectral_shift = val;
                    snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": \"Secret injected into slot %d: %f\", \"id\": %d}\n", slot, val, id);
                    return 1;
                }
            }
        }
    }
    if (method_type == 24) { // manifold.dispatch
        if (state->manifest) {
            tsfi_dispatch_zmm_dynamic(state->manifest);
            snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": {\"density\": %f}, \"id\": %d}\n", state->manifest->synapse.mass_density, id);
            return 1;
        }
    }
    if (method_type == 25) { // manifold.inspect_slots
        if (state->manifest) {
            int head = snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": {\"slots\": [");
            char *optr = output_buf + head;
            size_t rem = out_max - head - 64;
            for (int i = 0; i < 32; i++) {
                int n = snprintf(optr, rem, "{\"slot\": %d, \"type\": %d, \"address\": \"%p\"}%s", 
                                 i, state->manifest->slots[i].type, state->manifest->slots[i].data_ptr, (i < 31 ? ", " : ""));
                optr += n; rem -= n;
                if (rem < 128) break;
            }
            snprintf(optr, rem, "]}, \"id\": %d}\n", id);
            return 1;
        }
    }
    if (method_type == 26) { // manifold.upload_asset
        char hex[1024 * 64]; // Max 64KB upload for now
        if (extract_json_string(min_ptr, "\"data\"", hex, sizeof(hex))) {
            size_t bytes_needed = strlen(hex) / 2;
            void *ptr = lau_malloc_wired(bytes_needed);
            if (ptr) {
                decode_hex(hex, (uint8_t*)ptr, bytes_needed);
                snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": {\"address\": \"%p\", \"size\": %zu}, \"id\": %d}\n", ptr, bytes_needed, id);
                return 1;
            }
        }
    }
    if (method_type == 41) { // manifold.load_dna_llm
        char path[1024];
        if (extract_json_string(min_ptr, "\"path\"", path, sizeof(path))) {
            // Check if file exists
            if (access(path, F_OK) == 0) {
                if (state->manifest) {
                    state->manifest->slots[31].type = ZMM_TYPE_MASS;
                    state->manifest->slots[31].data_ptr = (void*)0x55555555; // Placeholder
                    state->manifest->active_mask |= (1U << 31);
                    snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": {\"status\": \"DNA Model Mounted to ZMM31\", \"address\": \"0x55555555\", \"size\": 1024}, \"id\": %d}\n", id);
                } else {
                    snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"error\": \"No ZMM active manifest\", \"id\": %d}\n", id);
                }
                return 1;
            }
        }
        snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"error\": \"DNA File not found\", \"id\": %d}\n", id);
        return 1;
    }
    if (method_type == 42) { // manifold.query_llm
        char prompt[2048];
        if (extract_json_string(min_ptr, "\"prompt\"", prompt, sizeof(prompt))) {
            if (state->manifest && (state->manifest->active_mask & (1U << 31))) {
                uint64_t tx_id = ++state->llm_tx_counter;
                int slot = tx_id % 16;
                state->llm_tx_status[slot] = 1;
                snprintf(state->llm_tx_results[slot], 4096, "for '%s'", prompt); // Temp store prompt
                snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": {\"receipt\": %lu, \"status\": \"pending\"}, \"id\": %d}\n", tx_id, id);
                return 1;
            }
        }
        snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"error\": \"DNA model not mounted or invalid prompt\", \"id\": %d}\n", id);
        return 1;
    }
    if (method_type == 43) { // manifold.get_receipt
        char id_str[64];
        extract_json_string(min_ptr, "\"receipt\"", id_str, sizeof(id_str));
        char *r_ptr = strstr(min_ptr, "\"receipt\"");
        if (r_ptr) {
            r_ptr += 9;
            while (*r_ptr == ' ' || *r_ptr == ':') r_ptr++;
            uint64_t tx_id = strtoull(r_ptr, NULL, 10);
            if (tx_id > 0 && tx_id <= state->llm_tx_counter) {
                int slot = tx_id % 16;
                if (state->llm_tx_status[slot] == 2) {
                    snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": {\"receipt\": %lu, \"status\": \"done\", \"response\": \"%s\"}, \"id\": %d}\n", tx_id, state->llm_tx_results[slot], id);
                    state->llm_tx_status[slot] = 0; 
                    return 1;
                } else if (state->llm_tx_status[slot] == 1) {
                    snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": {\"receipt\": %lu, \"status\": \"pending\"}, \"id\": %d}\n", tx_id, id);
                    return 1;
                }
            }
        }
        snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"error\": \"Receipt not found\", \"id\": %d}\n", id);
        return 1;
    }
    if (method_type == 44) { // manifold.mount_instrument
        char target[64] = {0};
        if (extract_json_string(min_ptr, "\"target\"", target, sizeof(target))) {
            if (strcmp(target, "lead") == 0) {
                g_rpc_mounts.lead_mounted = true;
                snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": \"Mounted lead\", \"id\": %d}\n", id);
            } else if (strcmp(target, "bass") == 0) {
                g_rpc_mounts.bass_mounted = true;
                snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": \"Mounted bass\", \"id\": %d}\n", id);
            } else if (strcmp(target, "growl") == 0) {
                g_rpc_mounts.growl_mounted = true;
                snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": \"Mounted growl\", \"id\": %d}\n", id);
            } else if (strcmp(target, "drums") == 0) {
                g_rpc_mounts.drums_mounted = true;
                snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": \"Mounted drums\", \"id\": %d}\n", id);
            } else if (strcmp(target, "audio_stream_control") == 0) {
                g_rpc_mounts.audio_stream_control_mounted = true;
                printf("[PDP-11 MOUNT] Device audio_stream_control attached to channel input stream.\n");
                snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": \"Mounted audio_stream_control\", \"id\": %d}\n", id);
            } else {
                snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"error\": \"Unknown instrument target\", \"id\": %d}\n", id);
            }
        } else {
            snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"error\": \"Missing target parameter\", \"id\": %d}\n", id);
        }
        return 1;
    }
    if (method_type == 45) { // manifold.play_bio
        char path[256] = {0};
        int project = extract_json_int(min_ptr, "\"project\"", 0);
        int programmer = extract_json_int(min_ptr, "\"programmer\"", 0);
        int key_id = extract_json_int(min_ptr, "\"key_id\"", 0);

        int acl_level = register_zmm_operator_via_23_tree(project, programmer, key_id);
        if (acl_level == -1) {
            snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"error\": \"REVERT: PEER_LOCKED_OUT_DUE_TO_CONSECUTIVE_FAILURES\", \"id\": %d}\n", id);
            return 1;
        }
        if (acl_level < 1) {
            snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"error\": \"REVERT: ACL_PERMISSION_DENIED\", \"id\": %d}\n", id);
            return 1;
        }

        if (!g_rpc_mounts.lead_mounted && !g_rpc_mounts.bass_mounted && !g_rpc_mounts.growl_mounted && !g_rpc_mounts.drums_mounted) {
            snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"error\": \"REVERT: NO_INSTRUMENTS_MOUNTED_ON_SYNTHESIZER\", \"id\": %d}\n", id);
            return 1;
        }

        if (extract_json_string(min_ptr, "\"path\"", path, sizeof(path))) {
            if (!g_rpc_pulse_stream) {
                pa_sample_spec ss;
                ss.format = PA_SAMPLE_U8;
                ss.rate = SAMPLE_RATE;
                ss.channels = 1;
                int error;
                g_rpc_pulse_stream = pa_simple_new(NULL, "ZMM_MCP_PlayBio", PA_STREAM_PLAYBACK, NULL, "Synthesizer", &ss, NULL, NULL, &error);
                if (!g_rpc_pulse_stream) {
                    snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"error\": \"Failed to connect to PulseAudio\", \"id\": %d}\n", id);
                    return 1;
                }
            }

            const char *err = NULL;
            if (rpc_play_bio_arrangement(path, &err)) {
                snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": {\"status\": \"success\"}, \"id\": %d}\n", id);
            } else {
                snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"error\": \"%s\", \"id\": %d}\n", err, id);
            }

            if (g_rpc_pulse_stream) {
                int error;
                pa_simple_drain(g_rpc_pulse_stream, &error);
                pa_simple_free(g_rpc_pulse_stream);
                g_rpc_pulse_stream = NULL;
            }
        } else {
            snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"error\": \"Missing path parameter\", \"id\": %d}\n", id);
        }
        return 1;
    }
    if (method_type == 27) { // flow.trigger_choreography
        // Launch the Google Labs Flow unified masterpiece matrix in the background
        int sys_ret = system("nohup python3 tools/tsfi_ipomoea_teddy_matrix.py > /tmp/tsfi_choreography.log 2>&1 &");
        if (sys_ret == 0) {
            snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": \"Atropa/999 Sovereign Choreography Matrix Initiated (Background).\", \"id\": %d}\n", id);
        } else {
            snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"error\": \"Failed to launch choreography matrix.\", \"id\": %d}\n", id);
        }
        return 1;
    }
    return 0;
}

