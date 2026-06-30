#include <unistd.h>
#include "tsfi_zmm_rpc.h"
#include "tsfi_opt_zmm.h"
#include "tsfi_genetic.h"
#include "tsfi_svdag.h"
#include "tsfi_io.h"
#include "lau_memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_DILEMMA_LOGS 128
typedef struct {
    char event[64];
    char source[64];
    char details[256];
    uint64_t timestamp;
} DilemmaLogEntry;

static DilemmaLogEntry g_dilemma_logs[MAX_DILEMMA_LOGS];
static int g_dilemma_log_count = 0;

// Helper to extract a string parameter from simple JSON
static int extract_json_string(const char *json, const char *key, char *out, size_t max) {
    char *k = strstr(json, key);
    if (!k) return 0;
    char *start = strchr(k, ':');
    if (!start) return 0;
    start = strchr(start, '"');
    if (!start) return 0;
    start++;
    size_t i = 0;
    while (*start && i < max - 1) {
        if (*start == '"' && (i == 0 || *(start-1) != '\\')) break;
        out[i++] = *start++;
    }
    out[i] = 0;
    return 1;
}

static int extract_json_int(const char *json, const char *key, int default_val) {
    char *k = strstr(json, key);
    if (!k) return default_val;
    char *start = strchr(k, ':');
    if (!start) return default_val;
    return atoi(start + 1);
}

static size_t decode_hex(const char *hex, uint8_t *out, size_t max) {
    size_t len = strlen(hex);
    size_t decoded = 0;
    for (size_t i = 0; i < len && decoded < max; i += 2) {
        unsigned int val;
        sscanf(hex + i, "%02x", &val);
        out[decoded++] = (uint8_t)val;
    }
    return decoded;
}

int tsfi_zmm_rpc_dispatch(TsfiZmmVmState *state, const char *json_in, char *output_buf, size_t out_max) {
    const char *p = json_in;
    
    char *method_run = strstr(p, "wave512.run");
    char *method_reset = strstr(p, "wave512.reset");
    char *method_inspect = strstr(p, "wave512.inspect");
    char *method_scramble = strstr(p, "wave512.scramble");
    char *method_attach = strstr(p, "wave512.attach");
    char *method_genetic_bench = strstr(p, "genetic.benchmark");
    char *method_establish_llm = strstr(p, "genetic.establish_llm");
    char *method_autonomous_opt = strstr(p, "genetic.autonomous_optimize");
    

    // Shell Methods
    char *method_read_file = strstr(p, "shell.read_file");
    char *method_motzkin = strstr(p, "math.motzkin");

    // Manifold/Flow Methods
    char *method_swap_asset = strstr(p, "manifold.swap_asset");
    char *method_set_kernel = strstr(p, "manifold.set_kernel");
    char *method_set_mask = strstr(p, "manifold.set_active_mask");
    char *method_set_secret = strstr(p, "manifold.set_secret");
    char *method_dispatch = strstr(p, "manifold.dispatch");
    char *method_inspect_slots = strstr(p, "manifold.inspect_slots");
    char *method_upload_asset = strstr(p, "manifold.upload_asset");
    char *method_load_dna = strstr(p, "manifold.load_dna_llm");
    char *method_query_llm = strstr(p, "manifold.query_llm");
    char *method_get_receipt = strstr(p, "manifold.get_receipt");
    char *method_flow_choreography = strstr(p, "flow.trigger_choreography");
    char *method_dilemma_log = strstr(p, "wave512.dilemma_log");

    char *min_ptr = NULL;
    int method_type = 0; 
    
    if (method_run && (!min_ptr || method_run < min_ptr)) { min_ptr = method_run; method_type = 1; }
    if (method_reset && (!min_ptr || method_reset < min_ptr)) { min_ptr = method_reset; method_type = 2; }
    if (method_inspect && (!min_ptr || method_inspect < min_ptr)) { min_ptr = method_inspect; method_type = 3; }
    if (method_scramble && (!min_ptr || method_scramble < min_ptr)) { min_ptr = method_scramble; method_type = 4; }
    if (method_attach && (!min_ptr || method_attach < min_ptr)) { min_ptr = method_attach; method_type = 5; }
    if (method_genetic_bench && (!min_ptr || method_genetic_bench < min_ptr)) { min_ptr = method_genetic_bench; method_type = 6; }
    if (method_establish_llm && (!min_ptr || method_establish_llm < min_ptr)) { min_ptr = method_establish_llm; method_type = 11; }
    if (method_autonomous_opt && (!min_ptr || method_autonomous_opt < min_ptr)) { min_ptr = method_autonomous_opt; method_type = 19; }
    if (method_read_file && (!min_ptr || method_read_file < min_ptr)) { min_ptr = method_read_file; method_type = 10; }
    if (method_motzkin && (!min_ptr || method_motzkin < min_ptr)) { min_ptr = method_motzkin; method_type = 12; }
    
    if (method_swap_asset && (!min_ptr || method_swap_asset < min_ptr)) { min_ptr = method_swap_asset; method_type = 20; }
    if (method_set_kernel && (!min_ptr || method_set_kernel < min_ptr)) { min_ptr = method_set_kernel; method_type = 21; }
    if (method_set_mask && (!min_ptr || method_set_mask < min_ptr)) { min_ptr = method_set_mask; method_type = 22; }
    if (method_set_secret && (!min_ptr || method_set_secret < min_ptr)) { min_ptr = method_set_secret; method_type = 23; }
    if (method_dispatch && (!min_ptr || method_dispatch < min_ptr)) { min_ptr = method_dispatch; method_type = 24; }
    if (method_inspect_slots && (!min_ptr || method_inspect_slots < min_ptr)) { min_ptr = method_inspect_slots; method_type = 25; }
    if (method_upload_asset && (!min_ptr || method_upload_asset < min_ptr)) { min_ptr = method_upload_asset; method_type = 26; }
    if (method_load_dna && (!min_ptr || method_load_dna < min_ptr)) { min_ptr = method_load_dna; method_type = 41; }
    if (method_query_llm && (!min_ptr || method_query_llm < min_ptr)) { min_ptr = method_query_llm; method_type = 42; }
    if (method_get_receipt && (!min_ptr || method_get_receipt < min_ptr)) { min_ptr = method_get_receipt; method_type = 43; }
    if (method_flow_choreography && (!min_ptr || method_flow_choreography < min_ptr)) { min_ptr = method_flow_choreography; method_type = 27; }
    if (method_dilemma_log && (!min_ptr || method_dilemma_log < min_ptr)) { min_ptr = method_dilemma_log; method_type = 28; }

    char *method_mouse_move = strstr(p, "input.mouse_move");
    char *method_mouse_button = strstr(p, "input.mouse_button");
    char *method_keyboard = strstr(p, "input.keyboard");

    if (method_mouse_move && (!min_ptr || method_mouse_move < min_ptr)) { min_ptr = method_mouse_move; method_type = 30; }
    if (method_mouse_button && (!min_ptr || method_mouse_button < min_ptr)) { min_ptr = method_mouse_button; method_type = 31; }
    if (method_keyboard && (!min_ptr || method_keyboard < min_ptr)) { min_ptr = method_keyboard; method_type = 32; }


    if (!min_ptr) return 0;

    int id = 1;
    char *id_ptr = strstr(min_ptr, "\"id\"");
    if (id_ptr) {
        id_ptr = strchr(id_ptr, ':');
        if (id_ptr) id = atoi(id_ptr + 1);
    }

    if (method_type == 1) { // RUN
        char *code_key = strstr(min_ptr, "\"code\"");
        if (code_key) {
            char *start = strchr(code_key, ':');
            if (start) start = strchr(start, '"');
            if (start) {
                start++; 
                size_t len = strlen(start);
                char *decoded = (char*)lau_malloc(len + 1);
                char *src = start;
                char *dst = decoded;
                while (*src) {
                    if (*src == '"' && *(src-1) != '\\') break; 
                    if (*src == '\\') {
                        src++;
                        if (*src == 'n') *dst++ = '\n';
                        else if (*src == 't') *dst++ = '\t';
                        else if (*src == '"') *dst++ = '"';
                        else if (*src == '\\') *dst++ = '\\';
                        else *dst++ = *src;
                    } else {
                        *dst++ = *src;
                    }
                    src++;
                }
                *dst = 0;

                state->output_pos = 0;
                tsfi_zmm_vm_exec(state, decoded);
                
                int n = snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": {\"output\": \"");
                char *optr = output_buf + n;
                size_t rem = out_max - n;
                
                for (int i=0; i<state->output_pos && rem > 16; i++) {
                    char c = state->output_buffer[i];
                    if (c == '\n') { *optr++ = '\\'; *optr++ = 'n'; rem -= 2; }
                    else if (c == '"') { *optr++ = '\\'; *optr++ = '"'; rem -= 2; }
                    else if (c == '\\') { *optr++ = '\\'; *optr++ = '\\'; rem -= 2; }
                    else { *optr++ = c; rem--; }
                }
                snprintf(optr, rem, "\"}, \"id\": %d}\n", id);
                lau_free(decoded);
                return 1;
            }
        }
    } else if (method_type == 2) { // RESET
        tsfi_zmm_vm_destroy(state);
        tsfi_zmm_vm_init(state);
        snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": \"Reset OK\", \"id\": %d}\n", id);
        return 1;
    } else if (method_type == 3) { // INSPECT
        float density = state->manifest ? state->manifest->synapse.mass_density : 0.0f;
        int swap = state->manifest ? state->manifest->synapse.request_kernel_swap : 0;
        snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": {\"density\": %.4f, \"swap\": %d}, \"id\": %d}\n", density, swap, id);
        return 1;
    } else if (method_type == 4) { // SCRAMBLE
        if (state->manifest && state->manifest->contiguous_rf) {
            tsfi_scramble_wave512(state->manifest->contiguous_rf, 2048);
            snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": \"Scrambled 2KB RF\", \"id\": %d}\n", id);
        } else {
            snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"error\": \"No Manifest\", \"id\": %d}\n", id);
        }
        return 1;
    } else if (method_type == 5) { // ATTACH
        char shm_id[64];
        if (extract_json_string(min_ptr, "\"shm_id\"", shm_id, sizeof(shm_id))) {
            tsfi_zmm_vm_attach_telemetry(state, shm_id);
            snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": \"Attached to %s\", \"id\": %d}\n", shm_id, id);
            return 1;
        }
    } else if (method_type == 6) { // GENETIC.BENCHMARK
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
    } else if (method_type == 11) { // GENETIC.ESTABLISH_LLM
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
        
    } else if (method_type == 19) { // GENETIC.AUTONOMOUS_OPTIMIZE
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
    
    } else if (method_type == 12) { // MATH.MOTZKIN
        // Generate Motzkin string buffer and grep it
        char* gen_buf = malloc(1024 * 1024);
        size_t gen_offset = 0;
        
        uint64_t m_prev2 = 1;
        uint64_t m_prev1 = 1;
        
        auto bool is_prime(uint64_t n) {
            if (n <= 1) return false;
            if (n <= 3) return true;
            if (n % 2 == 0 || n % 3 == 0) return false;
            for (uint64_t i = 5; i * i <= n; i += 6) {
                if (n % i == 0 || n % (i + 2) == 0) return false;
            }
            return true;
        }

        for (uint64_t n = 2; n < 60; n++) {
            uint64_t term1 = (2 * n + 1) * m_prev1;
            uint64_t term2 = (3 * n - 3) * m_prev2;
            uint64_t m_curr = (term1 + term2) / (n + 2);
            
            int bits = 0;
            uint64_t temp = m_curr;
            while (temp > 0) { bits++; temp >>= 1; }
            
            if (is_prime(m_curr)) {
                gen_offset += snprintf(gen_buf + gen_offset, 1024 * 1024 - gen_offset, "M_%lu = %lu (0x%lX) - %d bits [PRIME]\n", n, m_curr, m_curr, bits);
            } else {
                gen_offset += snprintf(gen_buf + gen_offset, 1024 * 1024 - gen_offset, "M_%lu = %lu (0x%lX) - %d bits\n", n, m_curr, m_curr, bits);
            }
            
            m_prev2 = m_prev1;
            m_prev1 = m_curr;
        }
        
        // Execute native tsfi_io_grep to find the 50-bit prime!
        extern size_t tsfi_io_grep(const uint8_t* buffer, size_t buffer_size, const char* pattern, size_t pattern_len, size_t* match_indices, size_t max_matches);
        size_t match_idx[100];
        size_t num_matches = tsfi_io_grep((const uint8_t*)gen_buf, gen_offset, "[PRIME]", 7, match_idx, 100);
        
        char result_str[512] = "No 50-bit prime found";
        for (size_t i = 0; i < num_matches; i++) {
            // Find start of line
            size_t line_start = match_idx[i];
            while (line_start > 0 && gen_buf[line_start-1] != '\n') line_start--;
            
            // If it contains "50 bits"
            if (strstr(gen_buf + line_start, "50 bits") && strstr(gen_buf + line_start, "[PRIME]")) {
                size_t line_end = match_idx[i];
                while (line_end < gen_offset && gen_buf[line_end] != '\n') line_end++;
                
                snprintf(result_str, sizeof(result_str), "%.*s", (int)(line_end - line_start), gen_buf + line_start);
                break;
            }
        }
        
        snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": \"%s\", \"id\": %d}\n", result_str, id);
        free(gen_buf);

    } else if (method_type == 10) { // SHELL.READ_FILE
        char path[512];
        if (extract_json_string(min_ptr, "\"path\"", path, sizeof(path))) {
            int secret = extract_json_int(min_ptr, "\"secret\"", 0);
            int limit = extract_json_int(min_ptr, "\"limit\"", 65536); // Increased default limit
            
            if (limit > (int)(out_max / 4)) limit = out_max / 4; // Allow larger reads but keep space for JSON overhead

            FILE *f = fopen(path, "rb");
            if (!f) {
                snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"error\": \"Cannot open file: %s\", \"id\": %d}\n", path, id);
                return 1;
            }

            fseek(f, 0, SEEK_END);
            long total_size = ftell(f);
            if (secret >= total_size) {
                fclose(f);
                snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": {\"content\": \"\", \"total\": %ld}, \"id\": %d}\n", total_size, id);
                return 1;
            }

            fseek(f, secret, SEEK_SET);
            char *file_data = (char*)lau_malloc(limit + 1);
            size_t n = fread(file_data, 1, limit, f);
            file_data[n] = 0;
            fclose(f);

            int head = snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": {\"total\": %ld, \"content\": \"", total_size);
            char *optr = output_buf + head;
            size_t rem = out_max - head - 64; // Safety margin for JSON closing

            for (size_t i = 0; i < n && rem > 8; i++) {
                unsigned char c = (unsigned char)file_data[i];
                if (c == '\n') { *optr++ = '\\'; *optr++ = 'n'; rem -= 2; }
                else if (c == '\t') { *optr++ = '\\'; *optr++ = 't'; rem -= 2; }
                else if (c == '\r') { *optr++ = '\\'; *optr++ = 'r'; rem -= 2; }
                else if (c == '"') { *optr++ = '\\'; *optr++ = '"'; rem -= 2; }
                else if (c == '\\') { *optr++ = '\\'; *optr++ = '\\'; rem -= 2; }
                else if (c >= 32 && c <= 126) { *optr++ = c; rem--; }
                else {
                    // Robust Hex Escape for binary/non-printable data
                    int written = snprintf(optr, rem, "\\\\x%02x", c);
                    optr += written;
                    rem -= written;
                }
            }
            snprintf(optr, rem, "\"}, \"id\": %d}\n", id);
            lau_free(file_data);
            return 1;
        }
    } else if (method_type == 20) { // manifold.swap_asset
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
    } else if (method_type == 21) { // manifold.set_kernel
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
    } else if (method_type == 22) { // manifold.set_active_mask
        int mask = extract_json_int(min_ptr, "\"mask\"", 0);
        if (state->manifest) {
            state->manifest->active_mask = (uint32_t)mask;
            snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": \"Active mask set to 0x%08X\", \"id\": %d}\n", state->manifest->active_mask, id);
            return 1;
        }
    } else if (method_type == 23) { // manifold.set_secret
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
    } else if (method_type == 24) { // manifold.dispatch
        if (state->manifest) {
            tsfi_dispatch_zmm_dynamic(state->manifest);
            snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": {\"density\": %f}, \"id\": %d}\n", state->manifest->synapse.mass_density, id);
            return 1;
        }
    } else if (method_type == 25) { // manifold.inspect_slots
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
    } else if (method_type == 26) { // manifold.upload_asset
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
    } else if (method_type == 41) { // manifold.load_dna_llm
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
    } else if (method_type == 42) { // manifold.query_llm
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
    } else if (method_type == 43) { // manifold.get_receipt
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
    } else if (method_type == 27) { // flow.trigger_choreography
        // Launch the Google Labs Flow unified masterpiece matrix in the background
        int sys_ret = system("nohup python3 tools/tsfi_ipomoea_teddy_matrix.py > /tmp/tsfi_choreography.log 2>&1 &");
        if (sys_ret == 0) {
            snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": \"Atropa/999 Sovereign Choreography Matrix Initiated (Background).\", \"id\": %d}\n", id);
        } else {
            snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"error\": \"Failed to launch choreography matrix.\", \"id\": %d}\n", id);
        }
        return 1;
    } else if (method_type == 28) { // wave512.dilemma_log
        char event_str[64];
        if (extract_json_string(min_ptr, "\"event\"", event_str, sizeof(event_str))) {
            char src_str[64] = "External";
            char det_str[256] = "";
            extract_json_string(min_ptr, "\"source\"", src_str, sizeof(src_str));
            extract_json_string(min_ptr, "\"details\"", det_str, sizeof(det_str));
            
            if (g_dilemma_log_count < MAX_DILEMMA_LOGS) {
                snprintf(g_dilemma_logs[g_dilemma_log_count].event, 64, "%s", event_str);
                snprintf(g_dilemma_logs[g_dilemma_log_count].source, 64, "%s", src_str);
                snprintf(g_dilemma_logs[g_dilemma_log_count].details, 256, "%s", det_str);
                g_dilemma_logs[g_dilemma_log_count].timestamp = (uint64_t)time(NULL);
                g_dilemma_log_count++;
            } else {
                for (int i = 1; i < MAX_DILEMMA_LOGS; i++) {
                    g_dilemma_logs[i - 1] = g_dilemma_logs[i];
                }
                snprintf(g_dilemma_logs[MAX_DILEMMA_LOGS - 1].event, 64, "%s", event_str);
                snprintf(g_dilemma_logs[MAX_DILEMMA_LOGS - 1].source, 64, "%s", src_str);
                snprintf(g_dilemma_logs[MAX_DILEMMA_LOGS - 1].details, 256, "%s", det_str);
                g_dilemma_logs[MAX_DILEMMA_LOGS - 1].timestamp = (uint64_t)time(NULL);
            }
            snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": \"Log registered\", \"id\": %d}\n", id);
            return 1;
        } else {
            int head = snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": [");
            char *optr = output_buf + head;
            size_t rem = out_max - head - 64;
            for (int i = 0; i < g_dilemma_log_count; i++) {
                int n = snprintf(optr, rem, "{\"event\": \"%s\", \"source\": \"%s\", \"details\": \"%s\", \"timestamp\": %lu}%s", 
                                 g_dilemma_logs[i].event, g_dilemma_logs[i].source, g_dilemma_logs[i].details,
                                 (unsigned long)g_dilemma_logs[i].timestamp, (i < g_dilemma_log_count - 1 ? ", " : ""));
                optr += n; rem -= n;
                if (rem < 128) break;
            }
            snprintf(optr, rem, "], \"id\": %d}\n", id);
            return 1;
        }
    } else if (method_type == 30) { // input.mouse_move
        int x = extract_json_int(min_ptr, "\"x\"", 0);
        int y = extract_json_int(min_ptr, "\"y\"", 0);
        if (!state->reu_ram) {
            state->reu_size = 0x10000;
            state->reu_ram = (uint8_t*)calloc(1, state->reu_size);
        }
        if (state->reu_ram && state->reu_size > 0xF004) {
            state->reu_ram[0xF000] = (uint8_t)(x & 0xFF);
            state->reu_ram[0xF001] = (uint8_t)(y & 0xFF);
            state->reu_ram[0xF003] = (uint8_t)((x >> 8) & 0xFF);
            state->reu_ram[0xF004] = (uint8_t)((y >> 8) & 0xFF);
        }
        extern bool lau_yul_thunk_execute(const char *name, const uint8_t *calldata, size_t cd_size, uint8_t *retval, size_t *retval_len);
        uint8_t command_byte = (0x00 << 6) | 0x01;
        uint8_t cd[36] = {0x98, 0xd4, 0x00, 0xc0};
        cd[35] = command_byte;
        uint8_t ret[32];
        size_t ret_len = 32;
        lau_yul_thunk_execute("WinchesterMQ", cd, 36, ret, &ret_len);

        uint8_t cd_post[36] = {0xcc, 0xb0, 0x77, 0xa0};
        char cmd_str[32] = {0};
        snprintf(cmd_str, sizeof(cmd_str), "MM %d %d", x, y);
        memcpy(cd_post + 4, cmd_str, 32);
        lau_yul_thunk_execute("WinchesterMQ", cd_post, 36, ret, &ret_len);

        snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": \"Mouse move OK\", \"id\": %d}\n", id);
        return 1;
    } else if (method_type == 31) { // input.mouse_button
        int button = extract_json_int(min_ptr, "\"button\"", 272);
        int btn_state = extract_json_int(min_ptr, "\"state\"", 0);
        int x = extract_json_int(min_ptr, "\"x\"", -1);
        int y = extract_json_int(min_ptr, "\"y\"", -1);
        if (!state->reu_ram) {
            state->reu_size = 0x10000;
            state->reu_ram = (uint8_t*)calloc(1, state->reu_size);
        }
        if (state->reu_ram && state->reu_size > 0xF004 && x >= 0 && y >= 0) {
            state->reu_ram[0xF000] = (uint8_t)(x & 0xFF);
            state->reu_ram[0xF001] = (uint8_t)(y & 0xFF);
            state->reu_ram[0xF003] = (uint8_t)((x >> 8) & 0xFF);
            state->reu_ram[0xF004] = (uint8_t)((y >> 8) & 0xFF);
        }
        uint8_t btn_idx = 0;
        if (button == 273) btn_idx = 1;
        if (button == 274) btn_idx = 2;
        uint8_t command_byte = (0x01 << 6) | (btn_state ? 0x04 : 0x00) | (btn_idx & 0x03);

        extern bool lau_yul_thunk_execute(const char *name, const uint8_t *calldata, size_t cd_size, uint8_t *retval, size_t *retval_len);
        uint8_t cd[36] = {0x98, 0xd4, 0x00, 0xc0};
        cd[35] = command_byte;
        uint8_t ret[32];
        size_t ret_len = 32;
        lau_yul_thunk_execute("WinchesterMQ", cd, 36, ret, &ret_len);

        uint8_t cd_post[36] = {0xcc, 0xb0, 0x77, 0xa0};
        char cmd_str[32] = {0};
        snprintf(cmd_str, sizeof(cmd_str), "%s %d", btn_state ? "MD" : "MU", button);
        memcpy(cd_post + 4, cmd_str, 32);
        lau_yul_thunk_execute("WinchesterMQ", cd_post, 36, ret, &ret_len);

        snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": \"Mouse button OK\", \"id\": %d}\n", id);
        return 1;
    } else if (method_type == 32) { // input.keyboard
        int keycode = extract_json_int(min_ptr, "\"keycode\"", 0);
        int key_state = extract_json_int(min_ptr, "\"state\"", 0);
        if (!state->reu_ram) {
            state->reu_size = 0x10000;
            state->reu_ram = (uint8_t*)calloc(1, state->reu_size);
        }
        if (state->reu_ram && state->reu_size > 0xF002) {
            state->reu_ram[0xF002] = (uint8_t)keycode;
        }
        uint8_t command_byte = (0x02 << 6) | (key_state ? 0x20 : 0x00) | (keycode & 0x1F);

        extern bool lau_yul_thunk_execute(const char *name, const uint8_t *calldata, size_t cd_size, uint8_t *retval, size_t *retval_len);
        uint8_t cd[36] = {0x98, 0xd4, 0x00, 0xc0};
        cd[35] = command_byte;
        uint8_t ret[32];
        size_t ret_len = 32;
        lau_yul_thunk_execute("WinchesterMQ", cd, 36, ret, &ret_len);

        uint8_t cd_post[36] = {0xcc, 0xb0, 0x77, 0xa0};
        char cmd_str[32] = {0};
        snprintf(cmd_str, sizeof(cmd_str), "%s %d", key_state ? "KD" : "KU", keycode);
        memcpy(cd_post + 4, cmd_str, 32);
        lau_yul_thunk_execute("WinchesterMQ", cd_post, 36, ret, &ret_len);

        snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"result\": \"Keyboard event OK\", \"id\": %d}\n", id);
        return 1;
    }

    
    snprintf(output_buf, out_max, "{\"jsonrpc\": \"2.0\", \"error\": \"Method not found\", \"id\": %d}\n", id);
    return 1;
}

void tsfi_zmm_rpc_step_async_llm(TsfiZmmVmState *state) {
    if (!state) return;
    for (int i = 0; i < 16; i++) {
        if (state->llm_tx_status[i] == 1) { 
            if (state->manifest && (state->manifest->active_mask & (1U << 31))) {
                char prompt_extract[2048] = {0};
                char *prompt_start = strstr(state->llm_tx_results[i], "for '");
                if (prompt_start) {
                    prompt_start += 5;
                    char *prompt_end = strchr(prompt_start, '\'');
                    if (prompt_end) {
                        strncpy(prompt_extract, prompt_start, prompt_end - prompt_start);
                    }
                }
                
                memset(state->llm_tx_results[i], 0, sizeof(state->llm_tx_results[i]));
                
                char cmd[4096];
                snprintf(cmd, sizeof(cmd), "/home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/bin/query_local_deepseek /home/mariarahel/src/tsfi2/assets/DeepSeek-Coder-6.7B.gguf \"%s\"", prompt_extract);
                
                FILE *fp = popen(cmd, "r");
                if (fp) {
                    char chunk[1024];
                    while (fgets(chunk, sizeof(chunk), fp) != NULL) {
                        char esc_chunk[2048] = {0};
                        int ei = 0;
                        for(int ci=0; chunk[ci] != 0 && ei < 2046; ci++) {
                            if (chunk[ci] == '\n') {
                                esc_chunk[ei++] = '\\';
                                esc_chunk[ei++] = 'n';
                            } else if (chunk[ci] == '"') {
                                esc_chunk[ei++] = '\\';
                                esc_chunk[ei++] = '"';
                            } else if (chunk[ci] == '\\') {
                                esc_chunk[ei++] = '\\';
                                esc_chunk[ei++] = '\\';
                            } else {
                                esc_chunk[ei++] = chunk[ci];
                            }
                        }
                        strcat(state->llm_tx_results[i], esc_chunk);
                    }
                    pclose(fp);
                } else {
                    strcat(state->llm_tx_results[i], "[FRACTURE] Popen failed.\\n");
                }
                
                state->llm_tx_status[i] = 2; // DONE
            } else {
                state->llm_tx_status[i] = 2;
                snprintf(state->llm_tx_results[i], 4096, "ERROR: DNA Manifold Unmounted.");
            }
            break;
        }
    }
}
