#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <iostream>
#include "llama.h"

int main(int argc, char **argv) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <model_path> <prompt>\n";
        return 1;
    }
    const std::string model_path = argv[1];
    const std::string prompt = argv[2];

    llama_backend_init();
    llama_numa_init(GGML_NUMA_STRATEGY_DISABLED);

    llama_model_params model_params = llama_model_default_params();
    model_params.n_gpu_layers = 99; // Offload all layers to GPU

    llama_model *model = llama_load_model_from_file(model_path.c_str(), model_params);
    if (!model) {
        std::cerr << "[FRACTURE] Failed to load model from: " << model_path << "\n";
        return 1;
    }

    llama_context_params ctx_params = llama_context_default_params();
    ctx_params.n_ctx = 4096;
    ctx_params.n_threads = 16;

    llama_context *ctx = llama_new_context_with_model(model, ctx_params);
    if (!ctx) {
        std::cerr << "[FRACTURE] Failed to create llama context.\n";
        llama_free_model(model);
        return 1;
    }

    const struct llama_vocab *vocab = llama_model_get_vocab(model);

    // Format prompt according to DeepSeek Coder V2 template
    std::string formatted_prompt = "### Instruction:\n" + prompt + "\n\n### Response:\n";

    // Tokenize prompt
    std::vector<llama_token> tokens;
    tokens.resize(formatted_prompt.size() + 16);
    int n_tokens = llama_tokenize(vocab, formatted_prompt.c_str(), formatted_prompt.size(), tokens.data(), tokens.size(), true, true);
    if (n_tokens < 0) {
        tokens.resize(-n_tokens);
        n_tokens = llama_tokenize(vocab, formatted_prompt.c_str(), formatted_prompt.size(), tokens.data(), tokens.size(), true, true);
    }
    tokens.resize(n_tokens);

    // Initialize KV cache/decode loop
    llama_batch batch = llama_batch_get_one(tokens.data(), tokens.size());
    if (llama_decode(ctx, batch) != 0) {
        std::cerr << "[FRACTURE] Decode failed.\n";
        llama_free(ctx);
        llama_free_model(model);
        return 1;
    }

    // Initialize sampler
    llama_sampler *smpl = llama_sampler_init_greedy();

    // Generation loop
    const int max_tokens = 512;
    for (int i = 0; i < max_tokens; i++) {
        llama_token token_id = llama_sampler_sample(smpl, ctx, -1);
        llama_sampler_accept(smpl, token_id);

        if (llama_vocab_is_eog(vocab, token_id)) {
            break;
        }

        // Convert token to piece and print
        char buf[128];
        int n_chars = llama_token_to_piece(vocab, token_id, buf, sizeof(buf), 0, false);
        if (n_chars > 0) {
            std::cout << std::string(buf, n_chars);
            std::cout.flush();
        }

        // Decode next token
        batch = llama_batch_get_one(&token_id, 1);
        if (llama_decode(ctx, batch) != 0) {
            std::cerr << "\n[FRACTURE] Decode failed during generation.\n";
            break;
        }
    }

    std::cout << "\n";
    llama_sampler_free(smpl);
    llama_free(ctx);
    llama_free_model(model);
    llama_backend_free();
    return 0;
}
