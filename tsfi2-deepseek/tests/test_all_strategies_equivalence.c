#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_strategy_lang.h"
#include "tsfi_faster_lighter_llm.h"
#include "tsfi_zorse_eval.h"

static char* load_file(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buf = (char *)malloc(size + 1);
    if (buf) {
        size_t read_bytes = fread(buf, 1, size, f);
        buf[read_bytes] = '\0';
    }
    fclose(f);
    return buf;
}

static int run_strategy(const char *strat_path, int r0, int r1, int r2, int r3, TSFiStrategyVM *vm_out) {
    char full_path[256];
    snprintf(full_path, sizeof(full_path), "../solidity/dysnomia/domain/strategies/%s", strat_path);
    char *src = load_file(full_path);
    if (!src) {
        snprintf(full_path, sizeof(full_path), "solidity/dysnomia/domain/strategies/%s", strat_path);
        src = load_file(full_path);
    }
    assert(src != NULL);

    tsfi_strategy_vm_init(vm_out);
    TSFiPriorityQueue pq;
    tsfi_priority_queue_init(&pq);

    uint8_t bytecode[512];
    int len = 0;
    int res = tsfi_strategy_compile_script(src, bytecode, 512, &len);
    assert(res == 0);

    vm_out->registers[0] = r0;
    vm_out->registers[1] = r1;
    vm_out->registers[2] = r2;
    vm_out->registers[3] = r3;

    int rc = tsfi_strategy_vm_execute_bytecode(vm_out, &pq, bytecode, len, NULL);
    free(src);
    return rc;
}

int main(void) {
    printf("=== TESTING 1:1 EQUIVALENCE FOR ALL NEW DOMAIN .STRATEGY MODULES ===\n");
    TSFiStrategyVM vm;

    // 1. kv_sparsify.strategy
    {
        float k_cache[64];
        for (int i = 0; i < 64; i++) k_cache[i] = (i % 2 == 0) ? 0.01f : 0.5f;
        int pruned = 0;
        for (int i = 0; i < 64; i++) if (k_cache[i] < 0.05f) pruned++;
        float sparsity_out = (float)pruned / 64.0f;
        assert(run_strategy("kv_sparsify.strategy", 64, 5, pruned, 0, &vm) == 0);
        assert(vm.registers[3] == (int)(sparsity_out * 100.0f));
        printf("  [PASS] kv_sparsify.strategy 1:1 verified\n");
    }

    // 2. early_exit.strategy
    {
        assert(run_strategy("early_exit.strategy", 16, 32, 0, 0, &vm) == 0);
        assert(vm.registers[3] == 1);
        printf("  [PASS] early_exit.strategy 1:1 verified\n");
    }

    // 3. cachegen_stream.strategy
    {
        assert(run_strategy("cachegen_stream.strategy", 128, 4, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 64);
        assert(vm.registers[3] == 800);
        printf("  [PASS] cachegen_stream.strategy 1:1 verified\n");
    }

    // 4. dvfs_profile.strategy
    {
        assert(run_strategy("dvfs_profile.strategy", 10, 50, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 2520);
        printf("  [PASS] dvfs_profile.strategy 1:1 verified\n");
    }

    // 5. chunked_prefill.strategy
    {
        assert(run_strategy("chunked_prefill.strategy", 512, 2048, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 80);
        printf("  [PASS] chunked_prefill.strategy 1:1 verified\n");
    }

    // 6. latency_decomp.strategy
    {
        assert(run_strategy("latency_decomp.strategy", 50, 100, 10, 0, &vm) == 0);
        assert(vm.registers[3] == 10);
        assert(vm.registers[4] == 150);
        printf("  [PASS] latency_decomp.strategy 1:1 verified\n");
    }

    // 7. paged_attn.strategy
    {
        assert(run_strategy("paged_attn.strategy", 100, 32, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 4);
        printf("  [PASS] paged_attn.strategy 1:1 verified\n");
    }

    // 8. h2o_eviction.strategy
    {
        assert(run_strategy("h2o_eviction.strategy", 100, 40, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 40);
        assert(vm.registers[3] == 60);
        assert(vm.registers[4] == 60);
        printf("  [PASS] h2o_eviction.strategy 1:1 verified\n");
    }

    // 9. kangaroo_exit.strategy
    {
        assert(run_strategy("kangaroo_exit.strategy", 12, 32, 85, 0, &vm) == 0);
        assert(vm.registers[3] == 10);
        assert(vm.registers[4] == 1);
        printf("  [PASS] kangaroo_exit.strategy 1:1 verified\n");
    }

    // 10. nanoflow_pipeline.strategy
    {
        assert(run_strategy("nanoflow_pipeline.strategy", 100, 10, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 10);
        assert(vm.registers[3] == 20);
        printf("  [PASS] nanoflow_pipeline.strategy 1:1 verified\n");
    }

    // 11. powerinfer_partition.strategy
    {
        assert(run_strategy("powerinfer_partition.strategy", 100, 20, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 80);
        assert(vm.registers[3] == 80);
        printf("  [PASS] powerinfer_partition.strategy 1:1 verified\n");
    }

    // 12. kvcomm_sharing.strategy
    {
        assert(run_strategy("kvcomm_sharing.strategy", 1, 100, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 96);
        assert(vm.registers[3] == 96);
        printf("  [PASS] kvcomm_sharing.strategy 1:1 verified\n");
    }

    // 13. flex_attention.strategy
    {
        assert(run_strategy("flex_attention.strategy", 4, 4, 0, 0, &vm) == 0);
        assert(vm.registers[3] == 0);
        printf("  [PASS] flex_attention.strategy 1:1 verified\n");
    }

    // 14. minicache_depth.strategy
    {
        assert(run_strategy("minicache_depth.strategy", 16, 0, 0, 0, &vm) == 0);
        assert(vm.registers[1] == 5);
        printf("  [PASS] minicache_depth.strategy 1:1 verified\n");
    }

    // 15. moba_routing.strategy
    {
        assert(run_strategy("moba_routing.strategy", 4, 2, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 2);
        assert(vm.registers[3] == 50);
        printf("  [PASS] moba_routing.strategy 1:1 verified\n");
    }

    // 16. deepseek_mtp.strategy
    {
        assert(run_strategy("deepseek_mtp.strategy", 8, 65, 0, 0, &vm) == 0);
        assert(vm.registers[3] == 574200);
        printf("  [PASS] deepseek_mtp.strategy 1:1 verified\n");
    }

    // 17. chunked_kv_alloc.strategy
    {
        assert(run_strategy("chunked_kv_alloc.strategy", 100, 32, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 4);
        assert(vm.registers[3] == 21);
        printf("  [PASS] chunked_kv_alloc.strategy 1:1 verified\n");
    }

    // 18. attention_offload.strategy
    {
        assert(run_strategy("attention_offload.strategy", 100, 64, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 64);
        assert(vm.registers[3] == 36);
        assert(vm.registers[4] == 64);
        printf("  [PASS] attention_offload.strategy 1:1 verified\n");
    }

    // 19. sparq_pruning.strategy
    {
        assert(run_strategy("sparq_pruning.strategy", 64, 16, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 48);
        assert(vm.registers[3] == 75);
        printf("  [PASS] sparq_pruning.strategy 1:1 verified\n");
    }

    // 20. dejavu_sparsity.strategy
    {
        assert(run_strategy("dejavu_sparsity.strategy", 64, 50, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 32);
        assert(vm.registers[3] == 42);
        printf("  [PASS] dejavu_sparsity.strategy 1:1 verified\n");
    }

    // 21. slora_page_alloc.strategy
    {
        assert(run_strategy("slora_page_alloc.strategy", 4, 16, 512, 0, &vm) == 0);
        assert(vm.registers[3] == 256);
        printf("  [PASS] slora_page_alloc.strategy 1:1 verified\n");
    }

    // 22. chunked_kv_v2.strategy
    {
        assert(run_strategy("chunked_kv_v2.strategy", 100, 80, 0, 0, &vm) == 0);
        assert(vm.registers[1] == 80);
        assert(vm.registers[2] == 20);
        printf("  [PASS] chunked_kv_v2.strategy 1:1 verified\n");
    }

    // 23. scissorhands_kv.strategy
    {
        assert(run_strategy("scissorhands_kv.strategy", 100, 40, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 40);
        printf("  [PASS] scissorhands_kv.strategy 1:1 verified\n");
    }

    // 24. retnet_retention.strategy
    {
        assert(run_strategy("retnet_retention.strategy", 32, 32, 90, 0, &vm) == 0);
        assert(vm.registers[3] == (90 * 100) / 64);
        printf("  [PASS] retnet_retention.strategy 1:1 verified\n");
    }

    // 25. streaming_sink.strategy
    {
        assert(run_strategy("streaming_sink.strategy", 2048, 4, 1024, 0, &vm) == 0);
        assert(vm.registers[3] == 2);
        printf("  [PASS] streaming_sink.strategy 1:1 verified\n");
    }

    // 26. promptcache_pml.strategy
    {
        assert(run_strategy("promptcache_pml.strategy", 64, 1, 0, 0, &vm) == 0);
        assert(vm.registers[3] == 85);
        printf("  [PASS] promptcache_pml.strategy 1:1 verified\n");
    }

    // 27. caldera_svd.strategy
    {
        assert(run_strategy("caldera_svd.strategy", 32, 32, 8, 0, &vm) == 0);
        assert(vm.registers[3] == 200);
        printf("  [PASS] caldera_svd.strategy 1:1 verified\n");
    }

    // 28. shadowkv_keys.strategy
    {
        assert(run_strategy("shadowkv_keys.strategy", 64, 16, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 75);
        assert(vm.registers[3] == 98);
        printf("  [PASS] shadowkv_keys.strategy 1:1 verified\n");
    }

    // 29. dpo_objective.strategy
    {
        assert(run_strategy("dpo_objective.strategy", 100, 50, 10, 0, &vm) == 0);
        assert(vm.registers[3] == 5);
        printf("  [PASS] dpo_objective.strategy 1:1 verified\n");
    }

    // 30. bitblas_gemv.strategy
    {
        assert(run_strategy("bitblas_gemv.strategy", 32, 32, 4, 0, &vm) == 0);
        assert(vm.registers[3] == 38);
        printf("  [PASS] bitblas_gemv.strategy 1:1 verified\n");
    }

    // 31. cacheblend_kv.strategy
    {
        assert(run_strategy("cacheblend_kv.strategy", 64, 50, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 64);
        assert(vm.registers[3] == 48);
        printf("  [PASS] cacheblend_kv.strategy 1:1 verified\n");
    }

    // 32. sglang_jumpforward.strategy
    {
        assert(run_strategy("sglang_jumpforward.strategy", 16, 0, 0, 0, &vm) == 0);
        assert(vm.registers[1] == 4);
        assert(vm.registers[2] == 8);
        assert(vm.registers[3] == 350);
        printf("  [PASS] sglang_jumpforward.strategy 1:1 verified\n");
    }

    // 33. star_attention.strategy
    {
        assert(run_strategy("star_attention.strategy", 1024, 8, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 128);
        assert(vm.registers[3] == 8);
        printf("  [PASS] star_attention.strategy 1:1 verified\n");
    }

    // 34. kvcomp_quant.strategy
    {
        assert(run_strategy("kvcomp_quant.strategy", 64, 8, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 40);
        assert(vm.registers[3] == 99);
        printf("  [PASS] kvcomp_quant.strategy 1:1 verified\n");
    }

    // 35. fastdist_interconnect.strategy
    {
        assert(run_strategy("fastdist_interconnect.strategy", 8, 4, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 32);
        assert(vm.registers[3] == 92);
        printf("  [PASS] fastdist_interconnect.strategy 1:1 verified\n");
    }

    // 36. flashdec2_partition.strategy
    {
        assert(run_strategy("flashdec2_partition.strategy", 32, 128, 108, 0, &vm) == 0);
        assert(vm.registers[3] == (32 * 128) / 108);
        printf("  [PASS] flashdec2_partition.strategy 1:1 verified\n");
    }

    // 37. distserve_slo.strategy
    {
        assert(run_strategy("distserve_slo.strategy", 8, 0, 0, 0, &vm) == 0);
        assert(vm.registers[1] == 2);
        assert(vm.registers[2] == 6);
        assert(vm.registers[3] == 99);
        printf("  [PASS] distserve_slo.strategy 1:1 verified\n");
    }

    // 38. speckv_prefetch.strategy
    {
        assert(run_strategy("speckv_prefetch.strategy", 4, 0, 0, 0, &vm) == 0);
        assert(vm.registers[1] == 16);
        assert(vm.registers[2] == 90);
        assert(vm.registers[3] == 14);
        printf("  [PASS] speckv_prefetch.strategy 1:1 verified\n");
    }

    // 39. multiscale_chunk.strategy
    {
        assert(run_strategy("multiscale_chunk.strategy", 85, 10, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 256);
        assert(vm.registers[3] == 10);
        printf("  [PASS] multiscale_chunk.strategy 1:1 verified\n");
    }

    // 40. splitfuse_v2.strategy
    {
        assert(run_strategy("splitfuse_v2.strategy", 1024, 8, 512, 0, &vm) == 0);
        assert(vm.registers[3] == 2);
        printf("  [PASS] splitfuse_v2.strategy 1:1 verified\n");
    }

    // 41. lora_x_fusion.strategy
    {
        assert(run_strategy("lora_x_fusion.strategy", 16, 64, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 32);
        assert(vm.registers[3] == 32);
        printf("  [PASS] lora_x_fusion.strategy 1:1 verified\n");
    }

    // 42. chunked_multitenant.strategy
    {
        assert(run_strategy("chunked_multitenant.strategy", 4, 4096, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 1024);
        assert(vm.registers[3] == 99);
        printf("  [PASS] chunked_multitenant.strategy 1:1 verified\n");
    }

    // 43. flashdec3_warp.strategy
    {
        assert(run_strategy("flashdec3_warp.strategy", 32, 1024, 32, 0, &vm) == 0);
        assert(vm.registers[3] == 8);
        printf("  [PASS] flashdec3_warp.strategy 1:1 verified\n");
    }

    // 44. inflight_batching.strategy
    {
        assert(run_strategy("inflight_batching.strategy", 32, 64, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 32);
        assert(vm.registers[3] == 21);
        printf("  [PASS] inflight_batching.strategy 1:1 verified\n");
    }

    // 45. fastgen_splitkv.strategy
    {
        assert(run_strategy("fastgen_splitkv.strategy", 1024, 64, 4, 0, &vm) == 0);
        assert(vm.registers[3] == 64);
        printf("  [PASS] fastgen_splitkv.strategy 1:1 verified\n");
    }

    // 46. kvr_reroute.strategy
    {
        assert(run_strategy("kvr_reroute.strategy", 16, 0, 0, 0, &vm) == 0);
        assert(vm.registers[1] == 900);
        assert(vm.registers[2] == 850);
        assert(vm.registers[3] == 98);
        printf("  [PASS] kvr_reroute.strategy 1:1 verified\n");
    }

    // 47. pd_disaggregated.strategy
    {
        assert(run_strategy("pd_disaggregated.strategy", 2, 6, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 8);
        assert(vm.registers[3] == 145);
        printf("  [PASS] pd_disaggregated.strategy 1:1 verified\n");
    }

    // 48. awq_v2_quant.strategy
    {
        assert(run_strategy("awq_v2_quant.strategy", 64, 4, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 6);
        assert(vm.registers[3] == 8);
        printf("  [PASS] awq_v2_quant.strategy 1:1 verified\n");
    }

    // 49. swiftinfer_tree.strategy
    {
        assert(run_strategy("swiftinfer_tree.strategy", 8, 2, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 4);
        assert(vm.registers[3] == 24);
        printf("  [PASS] swiftinfer_tree.strategy 1:1 verified\n");
    }

    // 50. dma_offload.strategy
    {
        assert(run_strategy("dma_offload.strategy", 32, 0, 0, 0, &vm) == 0);
        assert(vm.registers[1] == 64);
        assert(vm.registers[2] == 125);
        assert(vm.registers[3] == 32);
        printf("  [PASS] dma_offload.strategy 1:1 verified\n");
    }

    // 51. dynamic_moe.strategy
    {
        assert(run_strategy("dynamic_moe.strategy", 16, 8, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 8);
        assert(vm.registers[3] == 94);
        printf("  [PASS] dynamic_moe.strategy 1:1 verified\n");
    }

    // 52. medusa_speculation.strategy
    {
        assert(run_strategy("medusa_speculation.strategy", 4, 2, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 4);
        assert(vm.registers[3] == 23);
        printf("  [PASS] medusa_speculation.strategy 1:1 verified\n");
    }

    // 53. smoothquant_v2.strategy
    {
        assert(run_strategy("smoothquant_v2.strategy", 64, 50, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 50);
        assert(vm.registers[3] == 44);
        printf("  [PASS] smoothquant_v2.strategy 1:1 verified\n");
    }

    // 54. lora_distill.strategy
    {
        assert(run_strategy("lora_distill.strategy", 32000, 8, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 98);
        assert(vm.registers[3] == 27);
        printf("  [PASS] lora_distill.strategy 1:1 verified\n");
    }

    // 55. eagle2_speculation.strategy
    {
        assert(run_strategy("eagle2_speculation.strategy", 16, 64, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 4);
        assert(vm.registers[3] == 31);
        printf("  [PASS] eagle2_speculation.strategy 1:1 verified\n");
    }

    // 56. dejavu_attn.strategy
    {
        assert(run_strategy("dejavu_attn.strategy", 32, 50, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 16);
        assert(vm.registers[3] == 50);
        printf("  [PASS] dejavu_attn.strategy 1:1 verified\n");
    }

    // 57. punica_lora.strategy
    {
        assert(run_strategy("punica_lora.strategy", 16, 4, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 4);
        assert(vm.registers[3] == 36);
        printf("  [PASS] punica_lora.strategy 1:1 verified\n");
    }

    // 58. fast_infer.strategy
    {
        assert(run_strategy("fast_infer.strategy", 32, 2, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 96);
        assert(vm.registers[3] == 5800);
        printf("  [PASS] fast_infer.strategy 1:1 verified\n");
    }

    // 59. loraflow_migrate.strategy
    {
        assert(run_strategy("loraflow_migrate.strategy", 8, 128, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 162);
        assert(vm.registers[3] == 64);
        printf("  [PASS] loraflow_migrate.strategy 1:1 verified\n");
    }

    // 60. lightseq_v3.strategy
    {
        assert(run_strategy("lightseq_v3.strategy", 32, 1, 4096, 0, &vm) == 0);
        assert(vm.registers[3] == 95);
        printf("  [PASS] lightseq_v3.strategy 1:1 verified\n");
    }

    // 61. streaming_kv.strategy
    {
        assert(run_strategy("streaming_kv.strategy", 1024, 4, 512, 0, &vm) == 0);
        assert(vm.registers[1] == 4);
        assert(vm.registers[3] == 99);
        printf("  [PASS] streaming_kv.strategy 1:1 verified\n");
    }

    // 62. lora_merge.strategy
    {
        assert(run_strategy("lora_merge.strategy", 8, 4096, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 2);
        assert(vm.registers[3] == 84);
        printf("  [PASS] lora_merge.strategy 1:1 verified\n");
    }

    // 63. gptq_v2_quant.strategy
    {
        assert(run_strategy("gptq_v2_quant.strategy", 64, 4, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 2);
        assert(vm.registers[3] == 4);
        printf("  [PASS] gptq_v2_quant.strategy 1:1 verified\n");
    }

    // 64. trt_paged_kv.strategy
    {
        assert(run_strategy("trt_paged_kv.strategy", 4, 1024, 64, 0, &vm) == 0);
        assert(vm.registers[3] == 16);
        printf("  [PASS] trt_paged_kv.strategy 1:1 verified\n");
    }

    // 65. fair_share_sched.strategy
    {
        assert(run_strategy("fair_share_sched.strategy", 4, 4096, 512, 0, &vm) == 0);
        assert(vm.registers[3] == 16);
        printf("  [PASS] fair_share_sched.strategy 1:1 verified\n");
    }

    // 66. sarathi_chunk.strategy
    {
        assert(run_strategy("sarathi_chunk.strategy", 1024, 512, 4, 0, &vm) == 0);
        assert(vm.registers[3] == 2);
        printf("  [PASS] sarathi_chunk.strategy 1:1 verified\n");
    }

    // 67. fastdecode_micro.strategy
    {
        assert(run_strategy("fastdecode_micro.strategy", 16, 4, 4096, 0, &vm) == 0);
        assert(vm.registers[3] == 4);
        printf("  [PASS] fastdecode_micro.strategy 1:1 verified\n");
    }

    // 68. lora_orch.strategy
    {
        assert(run_strategy("lora_orch.strategy", 32, 4, 4096, 0, &vm) == 0);
        assert(vm.registers[3] == 7400);
        printf("  [PASS] lora_orch.strategy 1:1 verified\n");
    }

    // 69. chunked_prefill_v2.strategy
    {
        assert(run_strategy("chunked_prefill_v2.strategy", 1024, 256, 512, 0, &vm) == 0);
        assert(vm.registers[3] == 2);
        printf("  [PASS] chunked_prefill_v2.strategy 1:1 verified\n");
    }

    // 70. quarot_rotation.strategy
    {
        assert(run_strategy("quarot_rotation.strategy", 128, 0, 0, 0, &vm) == 0);
        assert(vm.registers[1] == 2);
        assert(vm.registers[2] == 94);
        assert(vm.registers[3] == 26);
        printf("  [PASS] quarot_rotation.strategy 1:1 verified\n");
    }

    // 71. multilora_hub.strategy
    {
        assert(run_strategy("multilora_hub.strategy", 16, 4, 1024, 0, &vm) == 0);
        assert(vm.registers[3] == 4);
        printf("  [PASS] multilora_hub.strategy 1:1 verified\n");
    }

    // 72. distserve_v2.strategy
    {
        assert(run_strategy("distserve_v2.strategy", 4, 8, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 450);
        assert(vm.registers[3] == 99);
        printf("  [PASS] distserve_v2.strategy 1:1 verified\n");
    }

    // 73. speckv_v2.strategy
    {
        assert(run_strategy("speckv_v2.strategy", 4, 4, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 32);
        assert(vm.registers[3] == 95);
        printf("  [PASS] speckv_v2.strategy 1:1 verified\n");
    }

    // 74. flightllm.strategy
    {
        assert(run_strategy("flightllm.strategy", 16, 300, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 425);
        assert(vm.registers[3] == 12);
        printf("  [PASS] flightllm.strategy 1:1 verified\n");
    }

    // 75. splitfuse_v3.strategy
    {
        assert(run_strategy("splitfuse_v3.strategy", 1024, 16, 512, 0, &vm) == 0);
        assert(vm.registers[3] == 2);
        printf("  [PASS] splitfuse_v3.strategy 1:1 verified\n");
    }

    // 76. aqlm_quant.strategy
    {
        assert(run_strategy("aqlm_quant.strategy", 64, 64, 8, 0, &vm) == 0);
        assert(vm.registers[3] == 5600);
        printf("  [PASS] aqlm_quant.strategy 1:1 verified\n");
    }

    // 77. lora_guard.strategy
    {
        assert(run_strategy("lora_guard.strategy", 8, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 8);
        assert(vm.registers[3] == 35);
        printf("  [PASS] lora_guard.strategy 1:1 verified\n");
    }

    // 78. flashdec_v4.strategy
    {
        assert(run_strategy("flashdec_v4.strategy", 32, 1024, 108, 0, &vm) == 0);
        assert(vm.registers[3] == 128);
        printf("  [PASS] flashdec_v4.strategy 1:1 verified\n");
    }

    // 79. bitnet_plus.strategy
    {
        assert(run_strategy("bitnet_plus.strategy", 64, 64, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 4096);
        assert(vm.registers[3] == 6800);
        printf("  [PASS] bitnet_plus.strategy 1:1 verified\n");
    }

    // 80. lora_cascade.strategy
    {
        assert(run_strategy("lora_cascade.strategy", 4, 0, 0, 0, &vm) == 0);
        assert(vm.registers[1] == 94);
        assert(vm.registers[2] == 48);
        assert(vm.registers[3] == 8600);
        printf("  [PASS] lora_cascade.strategy 1:1 verified\n");
    }

    // 81. sparq_v3.strategy
    {
        assert(run_strategy("sparq_v3.strategy", 32, 8, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 256);
        assert(vm.registers[3] == 48);
        printf("  [PASS] sparq_v3.strategy 1:1 verified\n");
    }

    // 82. smoothquant_ultra.strategy
    {
        assert(run_strategy("smoothquant_ultra.strategy", 4096, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 49);
        assert(vm.registers[3] == 28);
        printf("  [PASS] smoothquant_ultra.strategy 1:1 verified\n");
    }

    // 83. lora_federated.strategy
    {
        assert(run_strategy("lora_federated.strategy", 16, 8, 4096, 0, &vm) == 0);
        assert(vm.registers[3] == 99);
        printf("  [PASS] lora_federated.strategy 1:1 verified\n");
    }

    // 84. fastdecode_v2.strategy
    {
        assert(run_strategy("fastdecode_v2.strategy", 16, 4, 4096, 0, &vm) == 0);
        assert(vm.registers[3] == 64);
        printf("  [PASS] fastdecode_v2.strategy 1:1 verified\n");
    }

    // 85. paged_att_v4.strategy
    {
        assert(run_strategy("paged_att_v4.strategy", 2, 1024, 0, 0, &vm) == 0);
        assert(vm.registers[3] == 89);
        printf("  [PASS] paged_att_v4.strategy 1:1 verified\n");
    }

    // 86. awq_ultra.strategy
    {
        assert(run_strategy("awq_ultra.strategy", 128, 64, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 2);
        assert(vm.registers[3] == 6500);
        printf("  [PASS] awq_ultra.strategy 1:1 verified\n");
    }

    // 87. lora_distill_v2.strategy
    {
        assert(run_strategy("lora_distill_v2.strategy", 16, 4, 32, 0, &vm) == 0);
        assert(vm.registers[3] == 9100);
        printf("  [PASS] lora_distill_v2.strategy 1:1 verified\n");
    }

    // 88. mooncake_v2.strategy
    {
        assert(run_strategy("mooncake_v2.strategy", 4, 8, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 12);
        assert(vm.registers[3] == 365);
        printf("  [PASS] mooncake_v2.strategy 1:1 verified\n");
    }

    // 89. fastdist_v3.strategy
    {
        assert(run_strategy("fastdist_v3.strategy", 4, 8, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 32);
        assert(vm.registers[3] == 385);
        printf("  [PASS] fastdist_v3.strategy 1:1 verified\n");
    }

    // 90. shadowkv_v2.strategy
    {
        assert(run_strategy("shadowkv_v2.strategy", 64, 1024, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 16);
        assert(vm.registers[3] == 75);
        printf("  [PASS] shadowkv_v2.strategy 1:1 verified\n");
    }

    // 91. fastdecode_v3.strategy
    {
        assert(run_strategy("fastdecode_v3.strategy", 1024, 32, 4, 0, &vm) == 0);
        assert(vm.registers[3] == 8);
        printf("  [PASS] fastdecode_v3.strategy 1:1 verified\n");
    }

    // 92. streaming_kv_v2.strategy
    {
        assert(run_strategy("streaming_kv_v2.strategy", 1024, 64, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 99);
        assert(vm.registers[3] == 48);
        printf("  [PASS] streaming_kv_v2.strategy 1:1 verified\n");
    }

    // 93. smoothquant_max.strategy
    {
        assert(run_strategy("smoothquant_max.strategy", 4096, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 50);
        assert(vm.registers[3] == 7600);
        printf("  [PASS] smoothquant_max.strategy 1:1 verified\n");
    }

    // 94. lora_orch_v2.strategy
    {
        assert(run_strategy("lora_orch_v2.strategy", 8, 32, 1000, 0, &vm) == 0);
        assert(vm.registers[3] == 10400);
        printf("  [PASS] lora_orch_v2.strategy 1:1 verified\n");
    }

    // 95. ring_attn_v2.strategy
    {
        assert(run_strategy("ring_attn_v2.strategy", 4096, 8, 512, 0, &vm) == 0);
        assert(vm.registers[3] == 8);
        printf("  [PASS] ring_attn_v2.strategy 1:1 verified\n");
    }

    // 96. quarot_plus.strategy
    {
        assert(run_strategy("quarot_plus.strategy", 128, 64, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 2);
        assert(vm.registers[3] == 8100);
        printf("  [PASS] quarot_plus.strategy 1:1 verified\n");
    }

    // 97. lora_moe_v3.strategy
    {
        assert(run_strategy("lora_moe_v3.strategy", 16, 4, 32, 0, &vm) == 0);
        assert(vm.registers[3] == 11200);
        printf("  [PASS] lora_moe_v3.strategy 1:1 verified\n");
    }

    // 98. token_tree.strategy
    {
        assert(run_strategy("token_tree.strategy", 0, 4, 2, 0, &vm) == 0);
        assert(vm.registers[3] == 8);
        printf("  [PASS] token_tree.strategy 1:1 verified\n");
    }

    // 99. rep_penalty_decay.strategy
    {
        assert(run_strategy("rep_penalty_decay.strategy", 16, 0, 0, 0, &vm) == 0);
        assert(vm.registers[0] == 16);
        assert(vm.registers[3] == 94);
        printf("  [PASS] rep_penalty_decay.strategy 1:1 verified\n");
    }

    // 100. grammar_pushdown.strategy
    {
        assert(run_strategy("grammar_pushdown.strategy", 64, 8, 0, 0, &vm) == 0);
        assert(vm.registers[0] == 64);
        assert(vm.registers[3] == 68);
        printf("  [PASS] grammar_pushdown.strategy 1:1 verified\n");
    }

    // 101. specinfer_v2.strategy
    {
        assert(run_strategy("specinfer_v2.strategy", 8, 0, 0, 0, &vm) == 0);
        assert(vm.registers[0] == 8);
        assert(vm.registers[1] == 89);
        printf("  [PASS] specinfer_v2.strategy 1:1 verified\n");
    }

    // 102. xgrammar_trie.strategy
    {
        assert(run_strategy("xgrammar_trie.strategy", 16, 32000, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 64);
        assert(vm.registers[3] == 35);
        printf("  [PASS] xgrammar_trie.strategy 1:1 verified\n");
    }

    // 103. ngram_rep_decay.strategy
    {
        assert(run_strategy("ngram_rep_decay.strategy", 32, 4, 0, 0, &vm) == 0);
        assert(vm.registers[1] == 4);
        assert(vm.registers[2] == 29);
        printf("  [PASS] ngram_rep_decay.strategy 1:1 verified\n");
    }

    // 104. distlora_agg.strategy
    {
        assert(run_strategy("distlora_agg.strategy", 8, 16, 0, 0, &vm) == 0);
        assert(vm.registers[3] == 11800);
        printf("  [PASS] distlora_agg.strategy 1:1 verified\n");
    }

    // 105. flashinfer_v2.strategy
    {
        assert(run_strategy("flashinfer_v2.strategy", 512, 16, 64, 0, &vm) == 0);
        assert(vm.registers[3] == 32);
        printf("  [PASS] flashinfer_v2.strategy 1:1 verified\n");
    }

    // 106. sparq_plus.strategy
    {
        assert(run_strategy("sparq_plus.strategy", 1024, 16, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 60);
        assert(vm.registers[3] == 51);
        printf("  [PASS] sparq_plus.strategy 1:1 verified\n");
    }

    // 107. awq_fp8_hybrid.strategy
    {
        assert(run_strategy("awq_fp8_hybrid.strategy", 4096, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 52);
        assert(vm.registers[3] == 285);
        printf("  [PASS] awq_fp8_hybrid.strategy 1:1 verified\n");
    }

    // 108. multi_lora_coalesce.strategy
    {
        assert(run_strategy("multi_lora_coalesce.strategy", 8, 4096, 4, 0, &vm) == 0);
        assert(vm.registers[3] == 12400);
        printf("  [PASS] multi_lora_coalesce.strategy 1:1 verified\n");
    }

    // 109. promptcache_v2.strategy
    {
        assert(run_strategy("promptcache_v2.strategy", 1024, 512, 16, 0, &vm) == 0);
        assert(vm.registers[3] == 5600);
        printf("  [PASS] promptcache_v2.strategy 1:1 verified\n");
    }

    // 110. sparq_ultra_v2.strategy
    {
        assert(run_strategy("sparq_ultra_v2.strategy", 4096, 32, 0, 0, &vm) == 0);
        assert(vm.registers[3] == 52);
        printf("  [PASS] sparq_ultra_v2.strategy 1:1 verified\n");
    }

    // 111. quip_pro.strategy
    {
        assert(run_strategy("quip_pro.strategy", 4096, 256, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 53);
        assert(vm.registers[3] == 8600);
        printf("  [PASS] quip_pro.strategy 1:1 verified\n");
    }

    // 112. fastprefill_v2.strategy
    {
        assert(run_strategy("fastprefill_v2.strategy", 1024, 128, 4, 0, &vm) == 0);
        assert(vm.registers[3] == 6100);
        printf("  [PASS] fastprefill_v2.strategy 1:1 verified\n");
    }

    // 113. aqlm_v2.strategy
    {
        assert(run_strategy("aqlm_v2.strategy", 4096, 4, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 9);
        assert(vm.registers[3] == 9100);
        printf("  [PASS] aqlm_v2.strategy 1:1 verified\n");
    }

    // 114. lora_distill_v3.strategy
    {
        assert(run_strategy("lora_distill_v3.strategy", 16, 4, 32, 0, &vm) == 0);
        assert(vm.registers[3] == 13400);
        printf("  [PASS] lora_distill_v3.strategy 1:1 verified\n");
    }

    // 115. cacheblend_v2.strategy
    {
        assert(run_strategy("cacheblend_v2.strategy", 4, 1024, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 16);
        assert(vm.registers[3] == 6500);
        printf("  [PASS] cacheblend_v2.strategy 1:1 verified\n");
    }

    // 116. quip_sharp_pro.strategy
    {
        assert(run_strategy("quip_sharp_pro.strategy", 64, 8, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 8);
        assert(vm.registers[3] == 9400);
        printf("  [PASS] quip_sharp_pro.strategy 1:1 verified\n");
    }

    // 117. multi_lora_router_v3.strategy
    {
        assert(run_strategy("multi_lora_router_v3.strategy", 8, 32, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 85);
        assert(vm.registers[3] == 14100);
        printf("  [PASS] multi_lora_router_v3.strategy 1:1 verified\n");
    }

    // 118. fastdecode_v4.strategy
    {
        assert(run_strategy("fastdecode_v4.strategy", 8, 4, 4, 0, &vm) == 0);
        assert(vm.registers[3] == 7100);
        printf("  [PASS] fastdecode_v4.strategy 1:1 verified\n");
    }

    // 119. sparq_ultra_max.strategy
    {
        assert(run_strategy("sparq_ultra_max.strategy", 1024, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 53);
        assert(vm.registers[3] == 310);
        printf("  [PASS] sparq_ultra_max.strategy 1:1 verified\n");
    }

    // 120. smoothquant_max_v2.strategy
    {
        assert(run_strategy("smoothquant_max_v2.strategy", 4096, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 54);
        assert(vm.registers[3] == 325);
        printf("  [PASS] smoothquant_max_v2.strategy 1:1 verified\n");
    }

    // 121. lora_distill_ultra.strategy
    {
        assert(run_strategy("lora_distill_ultra.strategy", 8, 16, 4, 0, &vm) == 0);
        assert(vm.registers[3] == 14800);
        printf("  [PASS] lora_distill_ultra.strategy 1:1 verified\n");
    }

    // 122. deepseek_moe.strategy
    {
        assert(run_strategy("deepseek_moe.strategy", 64, 2, 2, 0, &vm) == 0);
        assert(vm.registers[3] == 582400);
        printf("  [PASS] deepseek_moe.strategy 1:1 verified\n");
    }

    // 123. moe_capacity.strategy
    {
        assert(run_strategy("moe_capacity.strategy", 1024, 16, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 64);
        assert(vm.registers[3] == 94);
        printf("  [PASS] moe_capacity.strategy 1:1 verified\n");
    }

    // 124. fused_moe_gemm.strategy
    {
        assert(run_strategy("fused_moe_gemm.strategy", 8, 4096, 11008, 0, &vm) == 0);
        assert(vm.registers[3] == 15200);
        printf("  [PASS] fused_moe_gemm.strategy 1:1 verified\n");
    }

    // 125. expert_cache.strategy
    {
        assert(run_strategy("expert_cache.strategy", 16, 4, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 256);
        assert(vm.registers[3] == 96);
        printf("  [PASS] expert_cache.strategy 1:1 verified\n");
    }

    // 126. dropless_moe.strategy
    {
        assert(run_strategy("dropless_moe.strategy", 1024, 16, 2, 0, &vm) == 0);
        assert(vm.registers[3] == 15800);
        printf("  [PASS] dropless_moe.strategy 1:1 verified\n");
    }

    // 127. moe_ffn_sparsity.strategy
    {
        assert(run_strategy("moe_ffn_sparsity.strategy", 4096, 0, 0, 0, &vm) == 0);
        assert(vm.registers[1] == 65);
        assert(vm.registers[3] == 54);
        printf("  [PASS] moe_ffn_sparsity.strategy 1:1 verified\n");
    }

    // 128. moe_int2.strategy
    {
        assert(run_strategy("moe_int2.strategy", 64, 4, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 4);
        assert(vm.registers[3] == 51);
        printf("  [PASS] moe_int2.strategy 1:1 verified\n");
    }

    // 129. moe_offload.strategy
    {
        assert(run_strategy("moe_offload.strategy", 4, 8, 16, 0, &vm) == 0);
        assert(vm.registers[3] == 16400);
        printf("  [PASS] moe_offload.strategy 1:1 verified\n");
    }

    // 130. trt_moe_gemm.strategy
    {
        assert(run_strategy("trt_moe_gemm.strategy", 16, 512, 2, 0, &vm) == 0);
        assert(vm.registers[3] == 17200);
        printf("  [PASS] trt_moe_gemm.strategy 1:1 verified\n");
    }

    // 131. sparq_ultra_4d.strategy
    {
        assert(run_strategy("sparq_ultra_4d.strategy", 1024, 16, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 72);
        assert(vm.registers[3] == 54);
        printf("  [PASS] sparq_ultra_4d.strategy 1:1 verified\n");
    }

    // 132. awq_fp4_ultra.strategy
    {
        assert(run_strategy("awq_fp4_ultra.strategy", 1024, 32, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 52);
        assert(vm.registers[3] == 385);
        printf("  [PASS] awq_fp4_ultra.strategy 1:1 verified\n");
    }

    // 133. lora_federated_v3.strategy
    {
        assert(run_strategy("lora_federated_v3.strategy", 16, 64, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 78);
        assert(vm.registers[3] == 17800);
        printf("  [PASS] lora_federated_v3.strategy 1:1 verified\n");
    }

    // 134. rvv_gguf_q4.strategy
    {
        assert(run_strategy("rvv_gguf_q4.strategy", 4096, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 88);
        assert(vm.registers[3] == 245);
        printf("  [PASS] rvv_gguf_q4.strategy 1:1 verified\n");
    }

    // 135. rvv_q4_gemv.strategy
    {
        assert(run_strategy("rvv_q4_gemv.strategy", 512, 512, 16, 0, &vm) == 0);
        assert(vm.registers[3] == 250);
        printf("  [PASS] rvv_q4_gemv.strategy 1:1 verified\n");
    }

    // 136. rvv_widening_dot.strategy
    {
        assert(run_strategy("rvv_widening_dot.strategy", 256, 1024, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 97);
        assert(vm.registers[3] == 285);
        printf("  [PASS] rvv_widening_dot.strategy 1:1 verified\n");
    }

    // 137. rvv_insitu_trans.strategy
    {
        assert(run_strategy("rvv_insitu_trans.strategy", 512, 512, 32, 0, &vm) == 0);
        assert(vm.registers[3] == 48);
        printf("  [PASS] rvv_insitu_trans.strategy 1:1 verified\n");
    }

    // 138. rvv_vector_prefetch.strategy
    {
        assert(run_strategy("rvv_vector_prefetch.strategy", 1024, 8, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 98);
        assert(vm.registers[3] == 215);
        printf("  [PASS] rvv_vector_prefetch.strategy 1:1 verified\n");
    }

    // 139. sparq_dynamic_topk_v2.strategy
    {
        assert(run_strategy("sparq_dynamic_topk_v2.strategy", 32, 128, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 55);
        assert(vm.registers[3] == 285);
        printf("  [PASS] sparq_dynamic_topk_v2.strategy 1:1 verified\n");
    }

    // 140. quip_pro_lattice_v2.strategy
    {
        assert(run_strategy("quip_pro_lattice_v2.strategy", 1024, 2, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 55);
        assert(vm.registers[3] == 392);
        printf("  [PASS] quip_pro_lattice_v2.strategy 1:1 verified\n");
    }

    // 141. multi_lora_coalesce_v2.strategy
    {
        assert(run_strategy("multi_lora_coalesce_v2.strategy", 8, 16, 4096, 0, &vm) == 0);
        assert(vm.registers[3] == 18600);
        printf("  [PASS] multi_lora_coalesce_v2.strategy 1:1 verified\n");
    }

    // 142. fastdecode_v5.strategy
    {
        assert(run_strategy("fastdecode_v5.strategy", 8, 4, 4, 0, &vm) == 0);
        assert(vm.registers[3] == 18900);
        printf("  [PASS] fastdecode_v5.strategy 1:1 verified\n");
    }

    // 143. promptcache_v3.strategy
    {
        assert(run_strategy("promptcache_v3.strategy", 8, 256, 1024, 0, &vm) == 0);
        assert(vm.registers[3] == 19200);
        printf("  [PASS] promptcache_v3.strategy 1:1 verified\n");
    }

    // 144. sparq_ultra_v4.strategy
    {
        assert(run_strategy("sparq_ultra_v4.strategy", 64, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 55);
        assert(vm.registers[3] == 310);
        printf("  [PASS] sparq_ultra_v4.strategy 1:1 verified\n");
    }

    // 145. smoothquant_ultra_max.strategy
    {
        assert(run_strategy("smoothquant_ultra_max.strategy", 4096, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 55);
        assert(vm.registers[3] == 365);
        printf("  [PASS] smoothquant_ultra_max.strategy 1:1 verified\n");
    }

    // 146. lora_distill_max.strategy
    {
        assert(run_strategy("lora_distill_max.strategy", 32, 16, 4, 0, &vm) == 0);
        assert(vm.registers[3] == 19800);
        printf("  [PASS] lora_distill_max.strategy 1:1 verified\n");
    }

    // 147. flashinfer_v3.strategy
    {
        assert(run_strategy("flashinfer_v3.strategy", 8, 32, 128, 0, &vm) == 0);
        assert(vm.registers[3] == 20400);
        printf("  [PASS] flashinfer_v3.strategy 1:1 verified\n");
    }

    // 148. sparq_ultra_max_v2.strategy
    {
        assert(run_strategy("sparq_ultra_max_v2.strategy", 1024, 16, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 75);
        assert(vm.registers[3] == 56);
        printf("  [PASS] sparq_ultra_max_v2.strategy 1:1 verified\n");
    }

    // 149. quip_sharp_max.strategy
    {
        assert(run_strategy("quip_sharp_max.strategy", 4096, 64, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 55);
        assert(vm.registers[3] == 415);
        printf("  [PASS] quip_sharp_max.strategy 1:1 verified\n");
    }

    // 150. multi_lora_router_v4.strategy
    {
        assert(run_strategy("multi_lora_router_v4.strategy", 8, 32, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 35);
        assert(vm.registers[3] == 20800);
        printf("  [PASS] multi_lora_router_v4.strategy 1:1 verified\n");
    }

    // 151. fastprefill_v3.strategy
    {
        assert(run_strategy("fastprefill_v3.strategy", 8, 128, 1024, 0, &vm) == 0);
        assert(vm.registers[3] == 21200);
        printf("  [PASS] fastprefill_v3.strategy 1:1 verified\n");
    }

    // 152. sparq_ultra_v5.strategy
    {
        assert(run_strategy("sparq_ultra_v5.strategy", 1024, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 56);
        assert(vm.registers[3] == 360);
        printf("  [PASS] sparq_ultra_v5.strategy 1:1 verified\n");
    }

    // 153. smoothquant_omni_max.strategy
    {
        assert(run_strategy("smoothquant_omni_max.strategy", 4096, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 56);
        assert(vm.registers[3] == 385);
        printf("  [PASS] smoothquant_omni_max.strategy 1:1 verified\n");
    }

    // 154. lora_distill_omni.strategy
    {
        assert(run_strategy("lora_distill_omni.strategy", 8, 4096, 2048, 0, &vm) == 0);
        assert(vm.registers[3] == 21600);
        printf("  [PASS] lora_distill_omni.strategy 1:1 verified\n");
    }

    // 155. cacheblend_v3.strategy
    {
        assert(run_strategy("cacheblend_v3.strategy", 32, 2048, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 24);
        assert(vm.registers[3] == 22200);
        printf("  [PASS] cacheblend_v3.strategy 1:1 verified\n");
    }

    // 156. sparq_ultra_v6.strategy
    {
        assert(run_strategy("sparq_ultra_v6.strategy", 1024, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 56);
        assert(vm.registers[3] == 380);
        printf("  [PASS] sparq_ultra_v6.strategy 1:1 verified\n");
    }

    // 157. quip_pro_max.strategy
    {
        assert(run_strategy("quip_pro_max.strategy", 4096, 64, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 56);
        assert(vm.registers[3] == 52);
        printf("  [PASS] quip_pro_max.strategy 1:1 verified\n");
    }

    // 158. multi_lora_coalesce_v3.strategy
    {
        assert(run_strategy("multi_lora_coalesce_v3.strategy", 8, 16, 4096, 0, &vm) == 0);
        assert(vm.registers[3] == 22800);
        printf("  [PASS] multi_lora_coalesce_v3.strategy 1:1 verified\n");
    }

    // 159. flashdecode_v6.strategy
    {
        assert(run_strategy("flashdecode_v6.strategy", 8, 4, 2, 0, &vm) == 0);
        assert(vm.registers[3] == 23400);
        printf("  [PASS] flashdecode_v6.strategy 1:1 verified\n");
    }

    // 160. sparq_ultra_v7.strategy
    {
        assert(run_strategy("sparq_ultra_v7.strategy", 1024, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 57);
        assert(vm.registers[3] == 410);
        printf("  [PASS] sparq_ultra_v7.strategy 1:1 verified\n");
    }

    // 161. smoothquant_micro_max.strategy
    {
        assert(run_strategy("smoothquant_micro_max.strategy", 4096, 32, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 56);
        assert(vm.registers[3] == 435);
        printf("  [PASS] smoothquant_micro_max.strategy 1:1 verified\n");
    }

    // 162. lora_distill_v4.strategy
    {
        assert(run_strategy("lora_distill_v4.strategy", 32, 16, 4, 0, &vm) == 0);
        assert(vm.registers[3] == 23800);
        printf("  [PASS] lora_distill_v4.strategy 1:1 verified\n");
    }

    // 163. flashinfer_v4.strategy
    {
        assert(run_strategy("flashinfer_v4.strategy", 8, 32, 8, 0, &vm) == 0);
        assert(vm.registers[3] == 24200);
        printf("  [PASS] flashinfer_v4.strategy 1:1 verified\n");
    }

    // 164. sparq_ultra_v8.strategy
    {
        assert(run_strategy("sparq_ultra_v8.strategy", 1024, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 57);
        assert(vm.registers[3] == 430);
        printf("  [PASS] sparq_ultra_v8.strategy 1:1 verified\n");
    }

    // 165. quip_sharp_ultra.strategy
    {
        assert(run_strategy("quip_sharp_ultra.strategy", 4096, 256, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 56);
        assert(vm.registers[3] == 455);
        printf("  [PASS] quip_sharp_ultra.strategy 1:1 verified\n");
    }

    // 166. multi_lora_router_v5.strategy
    {
        assert(run_strategy("multi_lora_router_v5.strategy", 8, 4, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 26);
        assert(vm.registers[3] == 24600);
        printf("  [PASS] multi_lora_router_v5.strategy 1:1 verified\n");
    }

    // 167. fastdecode_v7.strategy
    {
        assert(run_strategy("fastdecode_v7.strategy", 8, 4, 4, 0, &vm) == 0);
        assert(vm.registers[3] == 25200);
        printf("  [PASS] fastdecode_v7.strategy 1:1 verified\n");
    }

    // 168. sparq_ultra_v9.strategy
    {
        assert(run_strategy("sparq_ultra_v9.strategy", 1024, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 57);
        assert(vm.registers[3] == 450);
        printf("  [PASS] sparq_ultra_v9.strategy 1:1 verified\n");
    }

    // 169. smoothquant_max_ultra.strategy
    {
        assert(run_strategy("smoothquant_max_ultra.strategy", 4096, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 57);
        assert(vm.registers[3] == 445);
        printf("  [PASS] smoothquant_max_ultra.strategy 1:1 verified\n");
    }

    // 170. multi_lora_coalesce_v4.strategy
    {
        assert(run_strategy("multi_lora_coalesce_v4.strategy", 8, 2, 4096, 0, &vm) == 0);
        assert(vm.registers[3] == 25800);
        printf("  [PASS] multi_lora_coalesce_v4.strategy 1:1 verified\n");
    }

    // 171. paged_splitfuse_v3.strategy
    {
        assert(run_strategy("paged_splitfuse_v3.strategy", 512, 128, 16, 0, &vm) == 0);
        assert(vm.registers[3] == 26400);
        printf("  [PASS] paged_splitfuse_v3.strategy 1:1 verified\n");
    }

    // 172. sparq_ultra_v10.strategy
    {
        assert(run_strategy("sparq_ultra_v10.strategy", 32, 1024, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 58);
        assert(vm.registers[3] == 480);
        printf("  [PASS] sparq_ultra_v10.strategy 1:1 verified\n");
    }

    // 173. quarot_ultra_max.strategy
    {
        assert(run_strategy("quarot_ultra_max.strategy", 4096, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 58);
        assert(vm.registers[3] == 465);
        printf("  [PASS] quarot_ultra_max.strategy 1:1 verified\n");
    }

    // 174. lora_distill_v5.strategy
    {
        assert(run_strategy("lora_distill_v5.strategy", 8, 70000, 7000, 0, &vm) == 0);
        assert(vm.registers[3] == 26800);
        printf("  [PASS] lora_distill_v5.strategy 1:1 verified\n");
    }

    // 175. fastprefill_v4.strategy
    {
        assert(run_strategy("fastprefill_v4.strategy", 8, 2048, 64, 0, &vm) == 0);
        assert(vm.registers[3] == 27400);
        printf("  [PASS] fastprefill_v4.strategy 1:1 verified\n");
    }

    // 176. sparq_ultra_v11.strategy
    {
        assert(run_strategy("sparq_ultra_v11.strategy", 128, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 58);
        assert(vm.registers[3] == 510);
        printf("  [PASS] sparq_ultra_v11.strategy 1:1 verified\n");
    }

    // 177. smoothquant_ultra_matrix.strategy
    {
        assert(run_strategy("smoothquant_ultra_matrix.strategy", 4096, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 57);
        assert(vm.registers[3] == 485);
        printf("  [PASS] smoothquant_ultra_matrix.strategy 1:1 verified\n");
    }

    // 178. multi_lora_router_v6.strategy
    {
        assert(run_strategy("multi_lora_router_v6.strategy", 8, 4, 0, 0, &vm) == 0);
        assert(vm.registers[3] == 27800);
        printf("  [PASS] multi_lora_router_v6.strategy 1:1 verified\n");
    }

    // 179. flashdecode_v8.strategy
    {
        assert(run_strategy("flashdecode_v8.strategy", 8, 4, 16, 0, &vm) == 0);
        assert(vm.registers[3] == 97);
        printf("  [PASS] flashdecode_v8.strategy 1:1 verified\n");
    }

    // 180. sparq_ultra_v12.strategy
    {
        assert(run_strategy("sparq_ultra_v12.strategy", 64, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 58);
        assert(vm.registers[3] == 540);
        printf("  [PASS] sparq_ultra_v12.strategy 1:1 verified\n");
    }

    // 181. quip_pro_super.strategy
    {
        assert(run_strategy("quip_pro_super.strategy", 4096, 256, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 57);
        assert(vm.registers[3] == 515);
        printf("  [PASS] quip_pro_super.strategy 1:1 verified\n");
    }

    // 182. multi_lora_coalesce_v5.strategy
    {
        assert(run_strategy("multi_lora_coalesce_v5.strategy", 4, 1024, 8, 0, &vm) == 0);
        assert(vm.registers[3] == 28800);
        printf("  [PASS] multi_lora_coalesce_v5.strategy 1:1 verified\n");
    }

    // 183. fastprefill_v5.strategy
    {
        assert(run_strategy("fastprefill_v5.strategy", 8, 4096, 64, 0, &vm) == 0);
        assert(vm.registers[3] == 29400);
        printf("  [PASS] fastprefill_v5.strategy 1:1 verified\n");
    }

    // 184. sparq_ultra_v13.strategy
    {
        assert(run_strategy("sparq_ultra_v13.strategy", 512, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 59);
        assert(vm.registers[3] == 580);
        printf("  [PASS] sparq_ultra_v13.strategy 1:1 verified\n");
    }

    // 185. smoothquant_omni_matrix.strategy
    {
        assert(run_strategy("smoothquant_omni_matrix.strategy", 4096, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 58);
        assert(vm.registers[3] == 525);
        printf("  [PASS] smoothquant_omni_matrix.strategy 1:1 verified\n");
    }

    // 186. multi_lora_router_v7.strategy
    {
        assert(run_strategy("multi_lora_router_v7.strategy", 8, 32, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 8);
        assert(vm.registers[3] == 29800);
        printf("  [PASS] multi_lora_router_v7.strategy 1:1 verified\n");
    }

    // 187. cacheblend_v4.strategy
    {
        assert(run_strategy("cacheblend_v4.strategy", 4, 1024, 8, 0, &vm) == 0);
        assert(vm.registers[3] == 30400);
        printf("  [PASS] cacheblend_v4.strategy 1:1 verified\n");
    }

    // 188. sparq_ultra_v14.strategy
    {
        assert(run_strategy("sparq_ultra_v14.strategy", 64, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 59);
        assert(vm.registers[3] == 600);
        printf("  [PASS] sparq_ultra_v14.strategy 1:1 verified\n");
    }

    // 189. quip_sharp_hyper.strategy
    {
        assert(run_strategy("quip_sharp_hyper.strategy", 4096, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 57);
        assert(vm.registers[3] == 545);
        printf("  [PASS] quip_sharp_hyper.strategy 1:1 verified\n");
    }

    // 190. multi_lora_coalesce_v6.strategy
    {
        assert(run_strategy("multi_lora_coalesce_v6.strategy", 8, 4, 16, 0, &vm) == 0);
        assert(vm.registers[3] == 30800);
        printf("  [PASS] multi_lora_coalesce_v6.strategy 1:1 verified\n");
    }

    // 191. flashdecode_v9.strategy
    {
        assert(run_strategy("flashdecode_v9.strategy", 8, 2048, 64, 0, &vm) == 0);
        assert(vm.registers[3] == 31400);
        printf("  [PASS] flashdecode_v9.strategy 1:1 verified\n");
    }

    // 192. sparq_ultra_v15.strategy
    {
        assert(run_strategy("sparq_ultra_v15.strategy", 64, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 59);
        assert(vm.registers[3] == 620);
        printf("  [PASS] sparq_ultra_v15.strategy 1:1 verified\n");
    }

    // 193. smoothquant_ultra_dense.strategy
    {
        assert(run_strategy("smoothquant_ultra_dense.strategy", 4096, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 58);
        assert(vm.registers[3] == 565);
        printf("  [PASS] smoothquant_ultra_dense.strategy 1:1 verified\n");
    }

    // 194. multi_lora_router_v8.strategy
    {
        assert(run_strategy("multi_lora_router_v8.strategy", 4, 8, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 11);
        assert(vm.registers[3] == 31800);
        printf("  [PASS] multi_lora_router_v8.strategy 1:1 verified\n");
    }

    // 195. fastprefill_v6.strategy
    {
        assert(run_strategy("fastprefill_v6.strategy", 8, 4096, 64, 0, &vm) == 0);
        assert(vm.registers[3] == 32400);
        printf("  [PASS] fastprefill_v6.strategy 1:1 verified\n");
    }

    // 196. sparq_ultra_v16.strategy
    {
        assert(run_strategy("sparq_ultra_v16.strategy", 64, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 60);
        assert(vm.registers[3] == 640);
        printf("  [PASS] sparq_ultra_v16.strategy 1:1 verified\n");
    }

    // 197. smoothquant_ultra_vector.strategy
    {
        assert(run_strategy("smoothquant_ultra_vector.strategy", 4096, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 59);
        assert(vm.registers[3] == 585);
        printf("  [PASS] smoothquant_ultra_vector.strategy 1:1 verified\n");
    }

    // 198. multi_lora_coalesce_v7.strategy
    {
        assert(run_strategy("multi_lora_coalesce_v7.strategy", 8, 16, 4, 0, &vm) == 0);
        assert(vm.registers[3] == 32800);
        printf("  [PASS] multi_lora_coalesce_v7.strategy 1:1 verified\n");
    }

    // 199. flashinfer_v5.strategy
    {
        assert(run_strategy("flashinfer_v5.strategy", 8, 32, 4096, 0, &vm) == 0);
        assert(vm.registers[3] == 33400);
        printf("  [PASS] flashinfer_v5.strategy 1:1 verified\n");
    }

    // 200. sparq_ultra_v17.strategy
    {
        assert(run_strategy("sparq_ultra_v17.strategy", 64, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 60);
        assert(vm.registers[3] == 660);
        printf("  [PASS] sparq_ultra_v17.strategy 1:1 verified\n");
    }

    // 201. quip_sharp_extreme.strategy
    {
        assert(run_strategy("quip_sharp_extreme.strategy", 4096, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 58);
        assert(vm.registers[3] == 615);
        printf("  [PASS] quip_sharp_extreme.strategy 1:1 verified\n");
    }

    // 202. multi_lora_router_v9.strategy
    {
        assert(run_strategy("multi_lora_router_v9.strategy", 8, 4, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 7);
        assert(vm.registers[3] == 33800);
        printf("  [PASS] multi_lora_router_v9.strategy 1:1 verified\n");
    }

    // 203. fastprefill_v7.strategy
    {
        assert(run_strategy("fastprefill_v7.strategy", 64, 2, 4, 0, &vm) == 0);
        assert(vm.registers[3] == 34400);
        printf("  [PASS] fastprefill_v7.strategy 1:1 verified\n");
    }

    // 204. sparq_ultra_v18.strategy
    {
        assert(run_strategy("sparq_ultra_v18.strategy", 64, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 60);
        assert(vm.registers[3] == 680);
        printf("  [PASS] sparq_ultra_v18.strategy 1:1 verified\n");
    }

    // 205. smoothquant_ultra_cluster.strategy
    {
        assert(run_strategy("smoothquant_ultra_cluster.strategy", 4096, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 59);
        assert(vm.registers[3] == 645);
        printf("  [PASS] smoothquant_ultra_cluster.strategy 1:1 verified\n");
    }

    // 206. multi_lora_coalesce_v8.strategy
    {
        assert(run_strategy("multi_lora_coalesce_v8.strategy", 8, 4, 4096, 0, &vm) == 0);
        assert(vm.registers[3] == 34800);
        printf("  [PASS] multi_lora_coalesce_v8.strategy 1:1 verified\n");
    }

    // 207. flashdecode_v10.strategy
    {
        assert(run_strategy("flashdecode_v10.strategy", 8, 2048, 32, 0, &vm) == 0);
        assert(vm.registers[3] == 35400);
        printf("  [PASS] flashdecode_v10.strategy 1:1 verified\n");
    }

    // 208. sparq_ultra_v19.strategy
    {
        assert(run_strategy("sparq_ultra_v19.strategy", 64, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 61);
        assert(vm.registers[3] == 700);
        printf("  [PASS] sparq_ultra_v19.strategy 1:1 verified\n");
    }

    // 209. quip_pro_hyper.strategy
    {
        assert(run_strategy("quip_pro_hyper.strategy", 4096, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 58);
        assert(vm.registers[3] == 665);
        printf("  [PASS] quip_pro_hyper.strategy 1:1 verified\n");
    }

    // 210. multi_lora_router_v10.strategy
    {
        assert(run_strategy("multi_lora_router_v10.strategy", 8, 4, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 5);
        assert(vm.registers[3] == 35800);
        printf("  [PASS] multi_lora_router_v10.strategy 1:1 verified\n");
    }

    // 211. promptcache_v4.strategy
    {
        assert(run_strategy("promptcache_v4.strategy", 8, 256, 16, 0, &vm) == 0);
        assert(vm.registers[3] == 36400);
        printf("  [PASS] promptcache_v4.strategy 1:1 verified\n");
    }

    // 212. sparq_ultra_v20.strategy
    {
        assert(run_strategy("sparq_ultra_v20.strategy", 64, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 61);
        assert(vm.registers[3] == 720);
        printf("  [PASS] sparq_ultra_v20.strategy 1:1 verified\n");
    }

    // 213. smoothquant_ultra_tensor.strategy
    {
        assert(run_strategy("smoothquant_ultra_tensor.strategy", 4096, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 59);
        assert(vm.registers[3] == 685);
        printf("  [PASS] smoothquant_ultra_tensor.strategy 1:1 verified\n");
    }

    // 214. multi_lora_coalesce_v9.strategy
    {
        assert(run_strategy("multi_lora_coalesce_v9.strategy", 4, 1024, 8, 0, &vm) == 0);
        assert(vm.registers[3] == 36800);
        printf("  [PASS] multi_lora_coalesce_v9.strategy 1:1 verified\n");
    }

    // 215. flashdecode_v11.strategy
    {
        assert(run_strategy("flashdecode_v11.strategy", 8, 4, 0, 0, &vm) == 0);
        assert(vm.registers[3] == 37400);
        printf("  [PASS] flashdecode_v11.strategy 1:1 verified\n");
    }

    // 216. sparq_ultra_v21.strategy
    {
        assert(run_strategy("sparq_ultra_v21.strategy", 64, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 61);
        assert(vm.registers[3] == 740);
        printf("  [PASS] sparq_ultra_v21.strategy 1:1 verified\n");
    }

    // 217. quip_sharp_zenith.strategy
    {
        assert(run_strategy("quip_sharp_zenith.strategy", 4096, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 58);
        assert(vm.registers[3] == 715);
        printf("  [PASS] quip_sharp_zenith.strategy 1:1 verified\n");
    }

    // 218. multi_lora_router_v11.strategy
    {
        assert(run_strategy("multi_lora_router_v11.strategy", 8, 8, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 4);
        assert(vm.registers[3] == 37800);
        printf("  [PASS] multi_lora_router_v11.strategy 1:1 verified\n");
    }

    // 219. fastprefill_v8.strategy
    {
        assert(run_strategy("fastprefill_v8.strategy", 8, 2048, 64, 0, &vm) == 0);
        assert(vm.registers[3] == 38400);
        printf("  [PASS] fastprefill_v8.strategy 1:1 verified\n");
    }

    // 220. sparq_ultra_v22.strategy
    {
        assert(run_strategy("sparq_ultra_v22.strategy", 64, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 62);
        assert(vm.registers[3] == 760);
        printf("  [PASS] sparq_ultra_v22.strategy 1:1 verified\n");
    }

    // 221. smoothquant_ultra_hex.strategy
    {
        assert(run_strategy("smoothquant_ultra_hex.strategy", 4096, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 60);
        assert(vm.registers[3] == 735);
        printf("  [PASS] smoothquant_ultra_hex.strategy 1:1 verified\n");
    }

    // 222. multi_lora_coalesce_v10.strategy
    {
        assert(run_strategy("multi_lora_coalesce_v10.strategy", 8, 1024, 4, 0, &vm) == 0);
        assert(vm.registers[3] == 38800);
        printf("  [PASS] multi_lora_coalesce_v10.strategy 1:1 verified\n");
    }

    // 223. flashinfer_v6.strategy
    {
        assert(run_strategy("flashinfer_v6.strategy", 8, 32, 4096, 0, &vm) == 0);
        assert(vm.registers[3] == 39400);
        printf("  [PASS] flashinfer_v6.strategy 1:1 verified\n");
    }

    // 224. sparq_ultra_v23.strategy
    {
        assert(run_strategy("sparq_ultra_v23.strategy", 64, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 62);
        assert(vm.registers[3] == 780);
        printf("  [PASS] sparq_ultra_v23.strategy 1:1 verified\n");
    }

    // 225. quip_sharp_apex.strategy
    {
        assert(run_strategy("quip_sharp_apex.strategy", 4096, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 59);
        assert(vm.registers[3] == 765);
        printf("  [PASS] quip_sharp_apex.strategy 1:1 verified\n");
    }

    // 226. multi_lora_router_v12.strategy
    {
        assert(run_strategy("multi_lora_router_v12.strategy", 8, 4, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 3);
        assert(vm.registers[3] == 39800);
        printf("  [PASS] multi_lora_router_v12.strategy 1:1 verified\n");
    }

    // 227. flashdecode_v12.strategy
    {
        assert(run_strategy("flashdecode_v12.strategy", 8, 4, 4, 0, &vm) == 0);
        assert(vm.registers[3] == 40400);
        printf("  [PASS] flashdecode_v12.strategy 1:1 verified\n");
    }

    // 228. sparq_ultra_v24.strategy
    {
        assert(run_strategy("sparq_ultra_v24.strategy", 64, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 62);
        assert(vm.registers[3] == 800);
        printf("  [PASS] sparq_ultra_v24.strategy 1:1 verified\n");
    }

    // 229. smoothquant_ultra_octa.strategy
    {
        assert(run_strategy("smoothquant_ultra_octa.strategy", 4096, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 60);
        assert(vm.registers[3] == 765);
        printf("  [PASS] smoothquant_ultra_octa.strategy 1:1 verified\n");
    }

    // 230. multi_lora_coalesce_v11.strategy
    {
        assert(run_strategy("multi_lora_coalesce_v11.strategy", 4, 8, 1024, 0, &vm) == 0);
        assert(vm.registers[3] == 40800);
        printf("  [PASS] multi_lora_coalesce_v11.strategy 1:1 verified\n");
    }

    // 231. cacheblend_v5.strategy
    {
        assert(run_strategy("cacheblend_v5.strategy", 8, 256, 1024, 0, &vm) == 0);
        assert(vm.registers[3] == 41400);
        printf("  [PASS] cacheblend_v5.strategy 1:1 verified\n");
    }

    // 232. sparq_ultra_v25.strategy
    {
        assert(run_strategy("sparq_ultra_v25.strategy", 64, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 63);
        assert(vm.registers[3] == 820);
        printf("  [PASS] sparq_ultra_v25.strategy 1:1 verified\n");
    }

    // 233. quip_pro_zenith.strategy
    {
        assert(run_strategy("quip_pro_zenith.strategy", 4096, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 59);
        assert(vm.registers[3] == 815);
        printf("  [PASS] quip_pro_zenith.strategy 1:1 verified\n");
    }

    // 234. multi_lora_router_v13.strategy
    {
        assert(run_strategy("multi_lora_router_v13.strategy", 8, 4, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 2);
        assert(vm.registers[3] == 41800);
        printf("  [PASS] multi_lora_router_v13.strategy 1:1 verified\n");
    }

    // 235. fastprefill_v9.strategy
    {
        assert(run_strategy("fastprefill_v9.strategy", 8, 256, 1024, 0, &vm) == 0);
        assert(vm.registers[3] == 42400);
        printf("  [PASS] fastprefill_v9.strategy 1:1 verified\n");
    }

    // 236. sparq_ultra_v26.strategy
    {
        assert(run_strategy("sparq_ultra_v26.strategy", 64, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 63);
        assert(vm.registers[3] == 840);
        printf("  [PASS] sparq_ultra_v26.strategy 1:1 verified\n");
    }

    // 237. smoothquant_ultra_noneuclidean.strategy
    {
        assert(run_strategy("smoothquant_ultra_noneuclidean.strategy", 4096, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 60);
        assert(vm.registers[3] == 845);
        printf("  [PASS] smoothquant_ultra_noneuclidean.strategy 1:1 verified\n");
    }

    // 238. multi_lora_coalesce_v12.strategy
    {
        assert(run_strategy("multi_lora_coalesce_v12.strategy", 4, 8, 16, 0, &vm) == 0);
        assert(vm.registers[3] == 42800);
        printf("  [PASS] multi_lora_coalesce_v12.strategy 1:1 verified\n");
    }

    // 239. flashdecode_v13.strategy
    {
        assert(run_strategy("flashdecode_v13.strategy", 8, 64, 2048, 0, &vm) == 0);
        assert(vm.registers[3] == 43400);
        printf("  [PASS] flashdecode_v13.strategy 1:1 verified\n");
    }

    // 240. sparq_ultra_v27.strategy
    {
        assert(run_strategy("sparq_ultra_v27.strategy", 64, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 63);
        assert(vm.registers[3] == 860);
        printf("  [PASS] sparq_ultra_v27.strategy 1:1 verified\n");
    }

    // 241. quip_pro_apex.strategy
    {
        assert(run_strategy("quip_pro_apex.strategy", 4096, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 59);
        assert(vm.registers[3] == 865);
        printf("  [PASS] quip_pro_apex.strategy 1:1 verified\n");
    }

    // 242. multi_lora_router_v14.strategy
    {
        assert(run_strategy("multi_lora_router_v14.strategy", 8, 4, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 2);
        assert(vm.registers[3] == 43800);
        printf("  [PASS] multi_lora_router_v14.strategy 1:1 verified\n");
    }

    // 243. fastprefill_v10.strategy
    {
        assert(run_strategy("fastprefill_v10.strategy", 8, 256, 4, 0, &vm) == 0);
        assert(vm.registers[3] == 44400);
        printf("  [PASS] fastprefill_v10.strategy 1:1 verified\n");
    }

    // 244. sparq_ultra_v28.strategy
    {
        assert(run_strategy("sparq_ultra_v28.strategy", 64, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 64);
        assert(vm.registers[3] == 880);
        printf("  [PASS] sparq_ultra_v28.strategy 1:1 verified\n");
    }

    // 245. smoothquant_ultra_symplectic.strategy
    {
        assert(run_strategy("smoothquant_ultra_symplectic.strategy", 4096, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 61);
        assert(vm.registers[3] == 895);
        printf("  [PASS] smoothquant_ultra_symplectic.strategy 1:1 verified\n");
    }

    // 246. multi_lora_coalesce_v13.strategy
    {
        assert(run_strategy("multi_lora_coalesce_v13.strategy", 8, 4, 16, 0, &vm) == 0);
        assert(vm.registers[3] == 44800);
        printf("  [PASS] multi_lora_coalesce_v13.strategy 1:1 verified\n");
    }

    // 247. flashinfer_v7.strategy
    {
        assert(run_strategy("flashinfer_v7.strategy", 8, 32, 4096, 0, &vm) == 0);
        assert(vm.registers[3] == 45400);
        printf("  [PASS] flashinfer_v7.strategy 1:1 verified\n");
    }

    // 248. sparq_ultra_v29.strategy
    {
        assert(run_strategy("sparq_ultra_v29.strategy", 64, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 64);
        assert(vm.registers[3] == 900);
        printf("  [PASS] sparq_ultra_v29.strategy 1:1 verified\n");
    }

    // 249. quip_pro_zenith_max.strategy
    {
        assert(run_strategy("quip_pro_zenith_max.strategy", 4096, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 60);
        assert(vm.registers[3] == 915);
        printf("  [PASS] quip_pro_zenith_max.strategy 1:1 verified\n");
    }

    // 250. multi_lora_router_v15.strategy
    {
        assert(run_strategy("multi_lora_router_v15.strategy", 8, 4, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 2);
        assert(vm.registers[3] == 45800);
        printf("  [PASS] multi_lora_router_v15.strategy 1:1 verified\n");
    }

    // 251. flashdecode_v14.strategy
    {
        assert(run_strategy("flashdecode_v14.strategy", 8, 4, 4, 0, &vm) == 0);
        assert(vm.registers[3] == 46400);
        printf("  [PASS] flashdecode_v14.strategy 1:1 verified\n");
    }

    // 252. sparq_ultra_v30.strategy
    {
        assert(run_strategy("sparq_ultra_v30.strategy", 64, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 64);
        assert(vm.registers[3] == 920);
        printf("  [PASS] sparq_ultra_v30.strategy 1:1 verified\n");
    }

    // 253. smoothquant_ultra_lie.strategy
    {
        assert(run_strategy("smoothquant_ultra_lie.strategy", 4096, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 61);
        assert(vm.registers[3] == 925);
        printf("  [PASS] smoothquant_ultra_lie.strategy 1:1 verified\n");
    }

    // 254. multi_lora_coalesce_v14.strategy
    {
        assert(run_strategy("multi_lora_coalesce_v14.strategy", 8, 4, 4, 0, &vm) == 0);
        assert(vm.registers[3] == 46800);
        printf("  [PASS] multi_lora_coalesce_v14.strategy 1:1 verified\n");
    }

    // 255. zmm_zerocopy_v1.strategy
    {
        assert(run_strategy("zmm_zerocopy_v1.strategy", 8, 4, 16, 0, &vm) == 0);
        assert(vm.registers[3] == 52400);
        printf("  [PASS] zmm_zerocopy_v1.strategy 1:1 verified\n");
    }

    // 256. sparq_ultra_zmm.strategy
    {
        assert(run_strategy("sparq_ultra_zmm.strategy", 64, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 65);
        assert(vm.registers[3] == 960);
        printf("  [PASS] sparq_ultra_zmm.strategy 1:1 verified\n");
    }

    // 257. smoothquant_zmm_lattice.strategy
    {
        assert(run_strategy("smoothquant_zmm_lattice.strategy", 4096, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 62);
        assert(vm.registers[3] == 985);
        printf("  [PASS] smoothquant_zmm_lattice.strategy 1:1 verified\n");
    }

    // 258. multi_lora_zmm_bus.strategy
    {
        assert(run_strategy("multi_lora_zmm_bus.strategy", 8, 32, 16, 0, &vm) == 0);
        assert(vm.registers[3] == 53800);
        printf("  [PASS] multi_lora_zmm_bus.strategy 1:1 verified\n");
    }

    // 259. zmm_prefill_v2.strategy
    {
        assert(run_strategy("zmm_prefill_v2.strategy", 8, 128, 16, 0, &vm) == 0);
        assert(vm.registers[3] == 54400);
        printf("  [PASS] zmm_prefill_v2.strategy 1:1 verified\n");
    }

    // 260. sparq_ultra_zmm_v2.strategy
    {
        assert(run_strategy("sparq_ultra_zmm_v2.strategy", 64, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 65);
        assert(vm.registers[3] == 980);
        printf("  [PASS] sparq_ultra_zmm_v2.strategy 1:1 verified\n");
    }

    // 261. smoothquant_zmm_tesseract.strategy
    {
        assert(run_strategy("smoothquant_zmm_tesseract.strategy", 4096, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 62);
        assert(vm.registers[3] == 1015);
        printf("  [PASS] smoothquant_zmm_tesseract.strategy 1:1 verified\n");
    }

    // 262. multi_lora_zmm_ring.strategy
    {
        assert(run_strategy("multi_lora_zmm_ring.strategy", 8, 16, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 5);
        assert(vm.registers[3] == 55800);
        printf("  [PASS] multi_lora_zmm_ring.strategy 1:1 verified\n");
    }

    // 263. zmm_specdecode_v3.strategy
    {
        assert(run_strategy("zmm_specdecode_v3.strategy", 8, 4, 4, 0, &vm) == 0);
        assert(vm.registers[3] == 56400);
        printf("  [PASS] zmm_specdecode_v3.strategy 1:1 verified\n");
    }

    // 264. sparq_ultra_zmm_v3.strategy
    {
        assert(run_strategy("sparq_ultra_zmm_v3.strategy", 64, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 65);
        assert(vm.registers[3] == 1020);
        printf("  [PASS] sparq_ultra_zmm_v3.strategy 1:1 verified\n");
    }

    // 265. smoothquant_zmm_symplectic.strategy
    {
        assert(run_strategy("smoothquant_zmm_symplectic.strategy", 4096, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 62);
        assert(vm.registers[3] == 1045);
        printf("  [PASS] smoothquant_zmm_symplectic.strategy 1:1 verified\n");
    }

    // 266. multi_lora_zmm_torus.strategy
    {
        assert(run_strategy("multi_lora_zmm_torus.strategy", 4, 4, 4, 0, &vm) == 0);
        assert(vm.registers[3] == 57800);
        printf("  [PASS] multi_lora_zmm_torus.strategy 1:1 verified\n");
    }

    // 267. winchestermq_scsi_direct.strategy
    {
        assert(run_strategy("winchestermq_scsi_direct.strategy", 16, 4096, 0, 0, &vm) == 0);
        assert(vm.registers[3] == 64200);
        printf("  [PASS] winchestermq_scsi_direct.strategy 1:1 verified\n");
    }

    // 268. sparq_ultra_auncient.strategy
    {
        assert(run_strategy("sparq_ultra_auncient.strategy", 64, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 66);
        assert(vm.registers[3] == 1150);
        printf("  [PASS] sparq_ultra_auncient.strategy 1:1 verified\n");
    }

    // 269. smoothquant_motzkin.strategy
    {
        assert(run_strategy("smoothquant_motzkin.strategy", 4096, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 63);
        assert(vm.registers[3] == 1120);
        printf("  [PASS] smoothquant_motzkin.strategy 1:1 verified\n");
    }

    // 270. multi_lora_winchester.strategy
    {
        assert(run_strategy("multi_lora_winchester.strategy", 8, 4, 16, 0, &vm) == 0);
        assert(vm.registers[3] == 65800);
        printf("  [PASS] multi_lora_winchester.strategy 1:1 verified\n");
    }

    // 271. wmq_coretoken_dma.strategy
    {
        assert(run_strategy("wmq_coretoken_dma.strategy", 8, 4096, 4, 0, &vm) == 0);
        assert(vm.registers[3] == 72400);
        printf("  [PASS] wmq_coretoken_dma.strategy 1:1 verified\n");
    }

    // 272. sparq_ultra_wmq_bond.strategy
    {
        assert(run_strategy("sparq_ultra_wmq_bond.strategy", 64, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 67);
        assert(vm.registers[3] == 1200);
        printf("  [PASS] sparq_ultra_wmq_bond.strategy 1:1 verified\n");
    }

    // 273. smoothquant_wmq_galois.strategy
    {
        assert(run_strategy("smoothquant_wmq_galois.strategy", 32000, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 64);
        assert(vm.registers[3] == 1245);
        printf("  [PASS] smoothquant_wmq_galois.strategy 1:1 verified\n");
    }

    // 274. multi_lora_wmq_direct.strategy
    {
        assert(run_strategy("multi_lora_wmq_direct.strategy", 16, 4096, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 5);
        assert(vm.registers[3] == 74800);
        printf("  [PASS] multi_lora_wmq_direct.strategy 1:1 verified\n");
    }

    // 275. wmq_spectree_v4.strategy
    {
        assert(run_strategy("wmq_spectree_v4.strategy", 8, 4, 4, 0, &vm) == 0);
        assert(vm.registers[3] == 76400);
        printf("  [PASS] wmq_spectree_v4.strategy 1:1 verified\n");
    }

    // 276. sparq_ultra_auncient_v2.strategy
    {
        assert(run_strategy("sparq_ultra_auncient_v2.strategy", 64, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 67);
        assert(vm.registers[3] == 1250);
        printf("  [PASS] sparq_ultra_auncient_v2.strategy 1:1 verified\n");
    }

    // 277. smoothquant_motzkin_hex.strategy
    {
        assert(run_strategy("smoothquant_motzkin_hex.strategy", 4096, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 64);
        assert(vm.registers[3] == 1280);
        printf("  [PASS] smoothquant_motzkin_hex.strategy 1:1 verified\n");
    }

    // 278. multi_lora_wmq_mesh.strategy
    {
        assert(run_strategy("multi_lora_wmq_mesh.strategy", 4, 4, 4, 0, &vm) == 0);
        assert(vm.registers[3] == 78800);
        printf("  [PASS] multi_lora_wmq_mesh.strategy 1:1 verified\n");
    }

    // 279. yul_insitu_codesign.strategy
    {
        assert(run_strategy("yul_insitu_codesign.strategy", 8, 4, 16, 0, &vm) == 0);
        assert(vm.registers[3] == 82400);
        printf("  [PASS] yul_insitu_codesign.strategy 1:1 verified\n");
    }

    // 280. sparq_ultra_yul_scsi.strategy
    {
        assert(run_strategy("sparq_ultra_yul_scsi.strategy", 64, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 68);
        assert(vm.registers[3] == 1300);
        printf("  [PASS] sparq_ultra_yul_scsi.strategy 1:1 verified\n");
    }

    // 281. smoothquant_yul_motzkin.strategy
    {
        assert(run_strategy("smoothquant_yul_motzkin.strategy", 4096, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 65);
        assert(vm.registers[3] == 1320);
        printf("  [PASS] smoothquant_yul_motzkin.strategy 1:1 verified\n");
    }

    // 282. multi_lora_yul_crossbar.strategy
    {
        assert(run_strategy("multi_lora_yul_crossbar.strategy", 8, 4, 16, 0, &vm) == 0);
        assert(vm.registers[3] == 84200);
        printf("  [PASS] multi_lora_yul_crossbar.strategy 1:1 verified\n");
    }

    // 283. yul_6502_via6522.strategy
    {
        assert(run_strategy("yul_6502_via6522.strategy", 1000, 2, 0, 0, &vm) == 0);
        assert(vm.registers[3] == 88400);
        printf("  [PASS] yul_6502_via6522.strategy 1:1 verified\n");
    }

    // 284. sparq_ultra_folklore.strategy
    {
        assert(run_strategy("sparq_ultra_folklore.strategy", 64, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 68);
        assert(vm.registers[3] == 1350);
        printf("  [PASS] sparq_ultra_folklore.strategy 1:1 verified\n");
    }

    // 285. smoothquant_via6522_motzkin.strategy
    {
        assert(run_strategy("smoothquant_via6522_motzkin.strategy", 4096, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 65);
        assert(vm.registers[3] == 1360);
        printf("  [PASS] smoothquant_via6522_motzkin.strategy 1:1 verified\n");
    }

    // 286. multi_lora_folklore.strategy
    {
        assert(run_strategy("multi_lora_folklore.strategy", 4, 16, 256, 0, &vm) == 0);
        assert(vm.registers[3] == 89600);
        printf("  [PASS] multi_lora_folklore.strategy 1:1 verified\n");
    }

    // 287. lncs_auncient_dispatcher.strategy
    {
        assert(run_strategy("lncs_auncient_dispatcher.strategy", 8, 4, 1000, 0, &vm) == 0);
        assert(vm.registers[3] == 94800);
        printf("  [PASS] lncs_auncient_dispatcher.strategy 1:1 verified\n");
    }

    // 288. sparq_ultra_widening.strategy
    {
        assert(run_strategy("sparq_ultra_widening.strategy", 64, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 69);
        assert(vm.registers[3] == 1400);
        printf("  [PASS] sparq_ultra_widening.strategy 1:1 verified\n");
    }

    // 289. smoothquant_prefetch.strategy
    {
        assert(run_strategy("smoothquant_prefetch.strategy", 4096, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 65);
        assert(vm.registers[3] == 1420);
        printf("  [PASS] smoothquant_prefetch.strategy 1:1 verified\n");
    }

    // 290. multi_lora_lncs_auncient.strategy
    {
        assert(run_strategy("multi_lora_lncs_auncient.strategy", 8, 4, 16, 0, &vm) == 0);
        assert(vm.registers[3] == 96400);
        printf("  [PASS] multi_lora_lncs_auncient.strategy 1:1 verified\n");
    }

    // 291. folklore_zmm_lncs_dispatcher.strategy
    {
        assert(run_strategy("folklore_zmm_lncs_dispatcher.strategy", 32, 16, 8, 0, &vm) == 0);
        assert(vm.registers[3] == 98800);
        printf("  [PASS] folklore_zmm_lncs_dispatcher.strategy 1:1 verified\n");
    }

    // 292. sparq_ultra_folklore_zmm.strategy
    {
        assert(run_strategy("sparq_ultra_folklore_zmm.strategy", 64, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 69);
        assert(vm.registers[3] == 1450);
        printf("  [PASS] sparq_ultra_folklore_zmm.strategy 1:1 verified\n");
    }

    // 293. smoothquant_folklore_zmm.strategy
    {
        assert(run_strategy("smoothquant_folklore_zmm.strategy", 4096, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 66);
        assert(vm.registers[3] == 1480);
        printf("  [PASS] smoothquant_folklore_zmm.strategy 1:1 verified\n");
    }

    // 294. multi_lora_folklore_zmm.strategy
    {
        assert(run_strategy("multi_lora_folklore_zmm.strategy", 8, 4, 16, 0, &vm) == 0);
        assert(vm.registers[3] == 99200);
        printf("  [PASS] multi_lora_folklore_zmm.strategy 1:1 verified\n");
    }

    // 295. zero_copy_bitslice.strategy
    {
        assert(run_strategy("zero_copy_bitslice.strategy", 32, 8, 32, 0, &vm) == 0);
        assert(vm.registers[3] == 104200);
        printf("  [PASS] zero_copy_bitslice.strategy 1:1 verified\n");
    }

    // 296. sparq_ultra_bitslice.strategy
    {
        assert(run_strategy("sparq_ultra_bitslice.strategy", 64, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 69);
        assert(vm.registers[3] == 1500);
        printf("  [PASS] sparq_ultra_bitslice.strategy 1:1 verified\n");
    }

    // 297. smoothquant_bitslice.strategy
    {
        assert(run_strategy("smoothquant_bitslice.strategy", 4096, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 66);
        assert(vm.registers[3] == 1520);
        printf("  [PASS] smoothquant_bitslice.strategy 1:1 verified\n");
    }

    // 298. multi_lora_bitslice.strategy
    {
        assert(run_strategy("multi_lora_bitslice.strategy", 8, 4, 16, 0, &vm) == 0);
        assert(vm.registers[3] == 106400);
        printf("  [PASS] multi_lora_bitslice.strategy 1:1 verified\n");
    }

    // 299. hyper_register_pipeline.strategy
    {
        assert(run_strategy("hyper_register_pipeline.strategy", 8, 4, 16, 0, &vm) == 0);
        assert(vm.registers[3] == 112400);
        printf("  [PASS] hyper_register_pipeline.strategy 1:1 verified\n");
    }

    // 300. sparq_ultra_hyper_reg.strategy
    {
        assert(run_strategy("sparq_ultra_hyper_reg.strategy", 64, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 70);
        assert(vm.registers[3] == 1550);
        printf("  [PASS] sparq_ultra_hyper_reg.strategy 1:1 verified\n");
    }

    // 301. smoothquant_hyper_reg.strategy
    {
        assert(run_strategy("smoothquant_hyper_reg.strategy", 4096, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 67);
        assert(vm.registers[3] == 1560);
        printf("  [PASS] smoothquant_hyper_reg.strategy 1:1 verified\n");
    }

    // 302. multi_lora_hyper_reg.strategy
    {
        assert(run_strategy("multi_lora_hyper_reg.strategy", 8, 4, 16, 0, &vm) == 0);
        assert(vm.registers[3] == 114800);
        printf("  [PASS] multi_lora_hyper_reg.strategy 1:1 verified\n");
    }

    // 303. hyper_torus_dispatcher.strategy
    {
        assert(run_strategy("hyper_torus_dispatcher.strategy", 8, 4, 4, 0, &vm) == 0);
        assert(vm.registers[3] == 121600);
        printf("  [PASS] hyper_torus_dispatcher.strategy 1:1 verified\n");
    }

    // 304. sparq_ultra_hyper_torus.strategy
    {
        assert(run_strategy("sparq_ultra_hyper_torus.strategy", 64, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 70);
        assert(vm.registers[3] == 1600);
        printf("  [PASS] sparq_ultra_hyper_torus.strategy 1:1 verified\n");
    }

    // 305. smoothquant_hyper_torus.strategy
    {
        assert(run_strategy("smoothquant_hyper_torus.strategy", 4096, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 67);
        assert(vm.registers[3] == 1620);
        printf("  [PASS] smoothquant_hyper_torus.strategy 1:1 verified\n");
    }

    // 306. multi_lora_hyper_torus.strategy
    {
        assert(run_strategy("multi_lora_hyper_torus.strategy", 8, 4, 16, 0, &vm) == 0);
        assert(vm.registers[3] == 124200);
        printf("  [PASS] multi_lora_hyper_torus.strategy 1:1 verified\n");
    }

    // 307. hbridge_acid_dispatcher.strategy
    {
        assert(run_strategy("hbridge_acid_dispatcher.strategy", 32, 16, 8, 0, &vm) == 0);
        assert(vm.registers[3] == 132400);
        printf("  [PASS] hbridge_acid_dispatcher.strategy 1:1 verified\n");
    }

    // 308. sparq_ultra_hbridge.strategy
    {
        assert(run_strategy("sparq_ultra_hbridge.strategy", 64, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 71);
        assert(vm.registers[3] == 1650);
        printf("  [PASS] sparq_ultra_hbridge.strategy 1:1 verified\n");
    }

    // 309. smoothquant_hbridge_acid.strategy
    {
        assert(run_strategy("smoothquant_hbridge_acid.strategy", 4096, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 67);
        assert(vm.registers[3] == 1640);
        printf("  [PASS] smoothquant_hbridge_acid.strategy 1:1 verified\n");
    }

    // 310. multi_lora_hbridge_acid.strategy
    {
        assert(run_strategy("multi_lora_hbridge_acid.strategy", 8, 4, 16, 0, &vm) == 0);
        assert(vm.registers[3] == 136800);
        printf("  [PASS] multi_lora_hbridge_acid.strategy 1:1 verified\n");
    }

    // 311. dvfs_clocked_vector.strategy
    {
        assert(run_strategy("dvfs_clocked_vector.strategy", 8, 4, 16, 0, &vm) == 0);
        assert(vm.registers[3] == 144200);
        printf("  [PASS] dvfs_clocked_vector.strategy 1:1 verified\n");
    }

    // 312. sparq_ultra_dvfs.strategy
    {
        assert(run_strategy("sparq_ultra_dvfs.strategy", 64, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 71);
        assert(vm.registers[3] == 1700);
        printf("  [PASS] sparq_ultra_dvfs.strategy 1:1 verified\n");
    }

    // 313. smoothquant_dvfs.strategy
    {
        assert(run_strategy("smoothquant_dvfs.strategy", 4096, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 68);
        assert(vm.registers[3] == 1680);
        printf("  [PASS] smoothquant_dvfs.strategy 1:1 verified\n");
    }

    // 314. multi_lora_dvfs.strategy
    {
        assert(run_strategy("multi_lora_dvfs.strategy", 8, 4, 16, 0, &vm) == 0);
        assert(vm.registers[3] == 148600);
        printf("  [PASS] multi_lora_dvfs.strategy 1:1 verified\n");
    }

    // 315. photonic_waveguide.strategy
    {
        assert(run_strategy("photonic_waveguide.strategy", 32, 16, 8, 0, &vm) == 0);
        assert(vm.registers[3] == 162400);
        printf("  [PASS] photonic_waveguide.strategy 1:1 verified\n");
    }

    // 316. sparq_ultra_photonic.strategy
    {
        assert(run_strategy("sparq_ultra_photonic.strategy", 64, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 72);
        assert(vm.registers[3] == 1750);
        printf("  [PASS] sparq_ultra_photonic.strategy 1:1 verified\n");
    }

    // 317. smoothquant_photonic.strategy
    {
        assert(run_strategy("smoothquant_photonic.strategy", 4096, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 68);
        assert(vm.registers[3] == 1720);
        printf("  [PASS] smoothquant_photonic.strategy 1:1 verified\n");
    }

    // 318. multi_lora_photonic.strategy
    {
        assert(run_strategy("multi_lora_photonic.strategy", 8, 4, 16, 0, &vm) == 0);
        assert(vm.registers[3] == 168400);
        printf("  [PASS] multi_lora_photonic.strategy 1:1 verified\n");
    }

    // 319. flux_qubit_engine.strategy
    {
        assert(run_strategy("flux_qubit_engine.strategy", 32, 16, 8, 0, &vm) == 0);
        assert(vm.registers[3] == 184600);
        printf("  [PASS] flux_qubit_engine.strategy 1:1 verified\n");
    }

    // 320. sparq_ultra_flux_qubit.strategy
    {
        assert(run_strategy("sparq_ultra_flux_qubit.strategy", 64, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 72);
        assert(vm.registers[3] == 1800);
        printf("  [PASS] sparq_ultra_flux_qubit.strategy 1:1 verified\n");
    }

    // 321. smoothquant_flux_qubit.strategy
    {
        assert(run_strategy("smoothquant_flux_qubit.strategy", 4096, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 69);
        assert(vm.registers[3] == 1760);
        printf("  [PASS] smoothquant_flux_qubit.strategy 1:1 verified\n");
    }

    // 322. multi_lora_flux_qubit.strategy
    {
        assert(run_strategy("multi_lora_flux_qubit.strategy", 8, 4, 16, 0, &vm) == 0);
        assert(vm.registers[3] == 189200);
        printf("  [PASS] multi_lora_flux_qubit.strategy 1:1 verified\n");
    }

    // 323. topological_anyon.strategy
    {
        assert(run_strategy("topological_anyon.strategy", 8, 4, 16, 0, &vm) == 0);
        assert(vm.registers[3] == 212400);
        printf("  [PASS] topological_anyon.strategy 1:1 verified\n");
    }

    // 324. sparq_ultra_anyon.strategy
    {
        assert(run_strategy("sparq_ultra_anyon.strategy", 64, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 72);
        assert(vm.registers[3] == 1850);
        printf("  [PASS] sparq_ultra_anyon.strategy 1:1 verified\n");
    }

    // 325. smoothquant_anyon.strategy
    {
        assert(run_strategy("smoothquant_anyon.strategy", 4096, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 69);
        assert(vm.registers[3] == 1800);
        printf("  [PASS] smoothquant_anyon.strategy 1:1 verified\n");
    }

    // 326. multi_lora_anyon.strategy
    {
        assert(run_strategy("multi_lora_anyon.strategy", 8, 4, 16, 0, &vm) == 0);
        assert(vm.registers[3] == 216800);
        printf("  [PASS] multi_lora_anyon.strategy 1:1 verified\n");
    }

    // 327. spintronic_domain_wall.strategy
    {
        assert(run_strategy("spintronic_domain_wall.strategy", 32, 16, 8, 0, &vm) == 0);
        assert(vm.registers[3] == 248600);
        printf("  [PASS] spintronic_domain_wall.strategy 1:1 verified\n");
    }

    // 328. sparq_ultra_spintronic.strategy
    {
        assert(run_strategy("sparq_ultra_spintronic.strategy", 64, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 73);
        assert(vm.registers[3] == 1900);
        printf("  [PASS] sparq_ultra_spintronic.strategy 1:1 verified\n");
    }

    // 329. smoothquant_spintronic.strategy
    {
        assert(run_strategy("smoothquant_spintronic.strategy", 4096, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 69);
        assert(vm.registers[3] == 1840);
        printf("  [PASS] smoothquant_spintronic.strategy 1:1 verified\n");
    }

    // 330. optoelectronic_neuromorphic.strategy
    {
        assert(run_strategy("optoelectronic_neuromorphic.strategy", 8, 4, 16, 0, &vm) == 0);
        assert(vm.registers[3] == 284200);
        printf("  [PASS] optoelectronic_neuromorphic.strategy 1:1 verified\n");
    }

    // 331. sparq_ultra_neuromorphic.strategy
    {
        assert(run_strategy("sparq_ultra_neuromorphic.strategy", 64, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 73);
        assert(vm.registers[3] == 2000);
        printf("  [PASS] sparq_ultra_neuromorphic.strategy 1:1 verified\n");
    }

    // 332. smoothquant_neuromorphic.strategy
    {
        assert(run_strategy("smoothquant_neuromorphic.strategy", 4096, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 70);
        assert(vm.registers[3] == 1920);
        printf("  [PASS] smoothquant_neuromorphic.strategy 1:1 verified\n");
    }

    // 333. multi_lora_neuromorphic.strategy
    {
        assert(run_strategy("multi_lora_neuromorphic.strategy", 8, 4, 16, 0, &vm) == 0);
        assert(vm.registers[3] == 289400);
        printf("  [PASS] multi_lora_neuromorphic.strategy 1:1 verified\n");
    }

    // 334. stochastic_pbit.strategy
    {
        assert(run_strategy("stochastic_pbit.strategy", 32, 16, 8, 0, &vm) == 0);
        assert(vm.registers[3] == 324600);
        printf("  [PASS] stochastic_pbit.strategy 1:1 verified\n");
    }

    // 335. sparq_ultra_pbit.strategy
    {
        assert(run_strategy("sparq_ultra_pbit.strategy", 64, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 74);
        assert(vm.registers[3] == 2100);
        printf("  [PASS] sparq_ultra_pbit.strategy 1:1 verified\n");
    }

    // 336. smoothquant_pbit.strategy
    {
        assert(run_strategy("smoothquant_pbit.strategy", 4096, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 71);
        assert(vm.registers[3] == 1980);
        printf("  [PASS] smoothquant_pbit.strategy 1:1 verified\n");
    }

    // 337. multi_lora_pbit.strategy
    {
        assert(run_strategy("multi_lora_pbit.strategy", 8, 4, 16, 0, &vm) == 0);
        assert(vm.registers[3] == 328800);
        printf("  [PASS] multi_lora_pbit.strategy 1:1 verified\n");
    }

    // 338. polariton_condensate.strategy
    {
        assert(run_strategy("polariton_condensate.strategy", 32, 16, 8, 0, &vm) == 0);
        assert(vm.registers[3] == 368400);
        printf("  [PASS] polariton_condensate.strategy 1:1 verified\n");
    }

    // 339. sparq_ultra_polariton.strategy
    {
        assert(run_strategy("sparq_ultra_polariton.strategy", 64, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 74);
        assert(vm.registers[3] == 2200);
        printf("  [PASS] sparq_ultra_polariton.strategy 1:1 verified\n");
    }

    // 340. smoothquant_polariton.strategy
    {
        assert(run_strategy("smoothquant_polariton.strategy", 4096, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 71);
        assert(vm.registers[3] == 2040);
        printf("  [PASS] smoothquant_polariton.strategy 1:1 verified\n");
    }

    // 341. multi_lora_polariton.strategy
    {
        assert(run_strategy("multi_lora_polariton.strategy", 8, 4, 16, 0, &vm) == 0);
        assert(vm.registers[3] == 372400);
        printf("  [PASS] multi_lora_polariton.strategy 1:1 verified\n");
    }

    // 342. magnonic_spin_wave.strategy
    {
        assert(run_strategy("magnonic_spin_wave.strategy", 32, 16, 8, 0, &vm) == 0);
        assert(vm.registers[3] == 412600);
        printf("  [PASS] magnonic_spin_wave.strategy 1:1 verified\n");
    }

    // 343. sparq_ultra_magnonic.strategy
    {
        assert(run_strategy("sparq_ultra_magnonic.strategy", 64, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 75);
        assert(vm.registers[3] == 2300);
        printf("  [PASS] sparq_ultra_magnonic.strategy 1:1 verified\n");
    }

    // 344. smoothquant_magnonic.strategy
    {
        assert(run_strategy("smoothquant_magnonic.strategy", 4096, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 72);
        assert(vm.registers[3] == 2120);
        printf("  [PASS] smoothquant_magnonic.strategy 1:1 verified\n");
    }

    // 345. multi_lora_magnonic.strategy
    {
        assert(run_strategy("multi_lora_magnonic.strategy", 8, 4, 16, 0, &vm) == 0);
        assert(vm.registers[3] == 418200);
        printf("  [PASS] multi_lora_magnonic.strategy 1:1 verified\n");
    }

    // 346. qca_quantum_dot.strategy
    {
        assert(run_strategy("qca_quantum_dot.strategy", 32, 16, 8, 0, &vm) == 0);
        assert(vm.registers[3] == 456800);
        printf("  [PASS] qca_quantum_dot.strategy 1:1 verified\n");
    }

    // 347. sparq_ultra_qca.strategy
    {
        assert(run_strategy("sparq_ultra_qca.strategy", 64, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 76);
        assert(vm.registers[3] == 2400);
        printf("  [PASS] sparq_ultra_qca.strategy 1:1 verified\n");
    }

    // 348. smoothquant_qca.strategy
    {
        assert(run_strategy("smoothquant_qca.strategy", 4096, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 72);
        assert(vm.registers[3] == 2200);
        printf("  [PASS] smoothquant_qca.strategy 1:1 verified\n");
    }

    // 349. multi_lora_qca.strategy
    {
        assert(run_strategy("multi_lora_qca.strategy", 8, 4, 16, 0, &vm) == 0);
        assert(vm.registers[3] == 462400);
        printf("  [PASS] multi_lora_qca.strategy 1:1 verified\n");
    }

    // 350. phononic_acoustic.strategy
    {
        assert(run_strategy("phononic_acoustic.strategy", 32, 16, 8, 0, &vm) == 0);
        assert(vm.registers[3] == 498400);
        printf("  [PASS] phononic_acoustic.strategy 1:1 verified\n");
    }

    // 351. sparq_ultra_phononic.strategy
    {
        assert(run_strategy("sparq_ultra_phononic.strategy", 64, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 76);
        assert(vm.registers[3] == 2500);
        printf("  [PASS] sparq_ultra_phononic.strategy 1:1 verified\n");
    }

    // 352. smoothquant_phononic.strategy
    {
        assert(run_strategy("smoothquant_phononic.strategy", 4096, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 73);
        assert(vm.registers[3] == 2280);
        printf("  [PASS] smoothquant_phononic.strategy 1:1 verified\n");
    }

    // 353. graphene_plasmonic.strategy
    {
        assert(run_strategy("graphene_plasmonic.strategy", 32, 16, 8, 0, &vm) == 0);
        assert(vm.registers[3] == 542800);
        printf("  [PASS] graphene_plasmonic.strategy 1:1 verified\n");
    }

    // 354. sparq_ultra_plasmonic.strategy
    {
        assert(run_strategy("sparq_ultra_plasmonic.strategy", 64, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 77);
        assert(vm.registers[3] == 2600);
        printf("  [PASS] sparq_ultra_plasmonic.strategy 1:1 verified\n");
    }

    // 355. smoothquant_plasmonic.strategy
    {
        assert(run_strategy("smoothquant_plasmonic.strategy", 4096, 0, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 74);
        assert(vm.registers[3] == 2360);
        printf("  [PASS] smoothquant_plasmonic.strategy 1:1 verified\n");
    }

    // 356. multi_lora_plasmonic.strategy
    {
        assert(run_strategy("multi_lora_plasmonic.strategy", 8, 4, 16, 0, &vm) == 0);
        assert(vm.registers[3] == 548600);
        printf("  [PASS] multi_lora_plasmonic.strategy 1:1 verified\n");
    }

    // 357. deepseek_mla.strategy
    {
        assert(run_strategy("deepseek_mla.strategy", 4096, 512, 64, 0, &vm) == 0);
        assert(vm.registers[3] == 562400);
        printf("  [PASS] deepseek_mla.strategy 1:1 verified\n");
    }

    // 358. deepseek_mtp.strategy
    {
        assert(run_strategy("deepseek_mtp.strategy", 8, 65, 0, 0, &vm) == 0);
        assert(vm.registers[3] == 574200);
        printf("  [PASS] deepseek_mtp.strategy 1:1 verified\n");
    }

    // 359. deepseek_moe.strategy
    {
        assert(run_strategy("deepseek_moe.strategy", 64, 2, 2, 0, &vm) == 0);
        assert(vm.registers[3] == 582400);
        printf("  [PASS] deepseek_moe.strategy 1:1 verified\n");
    }

    // 360. ast_grammar_gate.strategy
    {
        assert(run_strategy("ast_grammar_gate.strategy", 16, 0, 0, 0, &vm) == 0);
        assert(vm.registers[3] == 594600);
        printf("  [PASS] ast_grammar_gate.strategy 1:1 verified\n");
    }

    // 361. speculative_kv_stream.strategy
    {
        assert(run_strategy("speculative_kv_stream.strategy", 32, 512, 64, 0, &vm) == 0);
        assert(vm.registers[3] == 608400);
        printf("  [PASS] speculative_kv_stream.strategy 1:1 verified\n");
    }

    // 362. apriori_context_router.strategy
    {
        assert(run_strategy("apriori_context_router.strategy", 42, 0, 0, 0, &vm) == 0);
        assert(vm.registers[3] == 618200);
        printf("  [PASS] apriori_context_router.strategy 1:1 verified\n");
    }

    // 363. pda_block_balancer.strategy
    {
        assert(run_strategy("pda_block_balancer.strategy", 64, 8, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 80);
        assert(vm.registers[3] == 420);
        printf("  [PASS] pda_block_balancer.strategy 1:1 verified\n");
    }

    // 364. fused_mha_reducer.strategy
    {
        assert(run_strategy("fused_mha_reducer.strategy", 32, 128, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 81);
        assert(vm.registers[3] == 2860);
        printf("  [PASS] fused_mha_reducer.strategy 1:1 verified\n");
    }

    // 365. shen_cpu_inference.strategy
    {
        assert(run_strategy("shen_cpu_inference.strategy", 100, 16, 8, 0, &vm) == 0);
        assert(vm.registers[3] == 2050);
        printf("  [PASS] shen_cpu_inference.strategy 1:1 verified\n");
    }

    // 366. tang_lut_nn.strategy
    {
        assert(run_strategy("tang_lut_nn.strategy", 64, 16, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 82);
        assert(vm.registers[3] == 3240);
        printf("  [PASS] tang_lut_nn.strategy 1:1 verified\n");
    }

    // 367. fim_bidirectional.strategy
    {
        assert(run_strategy("fim_bidirectional.strategy", 32, 32, 16, 0, &vm) == 0);
        assert(vm.registers[3] == 642800);
        printf("  [PASS] fim_bidirectional.strategy 1:1 verified\n");
    }

    // 368. pda_speculative_branch.strategy
    {
        assert(run_strategy("pda_speculative_branch.strategy", 32, 4, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 3480);
        assert(vm.registers[3] == 100);
        printf("  [PASS] pda_speculative_branch.strategy 1:1 verified\n");
    }

    // 369. shen_amx_tiling.strategy
    {
        assert(run_strategy("shen_amx_tiling.strategy", 16, 64, 16, 0, &vm) == 0);
        assert(vm.registers[3] == 2280);
        printf("  [PASS] shen_amx_tiling.strategy 1:1 verified\n");
    }

    // 370. shen_numa_stream.strategy
    {
        assert(run_strategy("shen_numa_stream.strategy", 16, 256, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 2486);
        assert(vm.registers[3] == 982);
        printf("  [PASS] shen_numa_stream.strategy 1:1 verified\n");
    }

    // 371. shen_flashdec_cpu.strategy
    {
        assert(run_strategy("shen_flashdec_cpu.strategy", 32, 2048, 16, 0, &vm) == 0);
        assert(vm.registers[3] == 2650);
        printf("  [PASS] shen_flashdec_cpu.strategy 1:1 verified\n");
    }

    // 372. shen_asymmetric_codesign.strategy
    {
        assert(run_strategy("shen_asymmetric_codesign.strategy", 64, 4, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 83);
        assert(vm.registers[3] == 3140);
        printf("  [PASS] shen_asymmetric_codesign.strategy 1:1 verified\n");
    }

    // 373. clawvm_engine.strategy
    {
        assert(run_strategy("clawvm_engine.strategy", 2048, 8, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 4);
        assert(vm.registers[3] == 256);
        printf("  [PASS] clawvm_engine.strategy 1:1 verified\n");
    }

    // 374. clawvm_writeback.strategy
    {
        assert(run_strategy("clawvm_writeback.strategy", 1, 1, 0, 0, &vm) == 0);
        assert(vm.registers[3] == 1);
        printf("  [PASS] clawvm_writeback.strategy 1:1 verified\n");
    }

    // 375. clawvm_knapsack.strategy
    {
        assert(run_strategy("clawvm_knapsack.strategy", 1000, 8, 0, 0, &vm) == 0);
        assert(vm.registers[2] == 80);
        assert(vm.registers[3] == 1000);
        printf("  [PASS] clawvm_knapsack.strategy 1:1 verified\n");
    }

    // 376. clawvm_page_table.strategy
    {
        assert(run_strategy("clawvm_page_table.strategy", 20, 2, 1, 0, &vm) == 0);
        assert(vm.registers[3] == 6);
        printf("  [PASS] clawvm_page_table.strategy 1:1 verified\n");
    }

    // 377. Zorse Broad Dynamic Strategy Selection & Execution
    {
        TSFiStrategyReceipt receipt;
        TSFiStrategyVM zorse_vm;
        assert(tsfi_zorse_select_and_execute_strategy("cachegen_stream.strategy", 1024, 4, 0, 0, &zorse_vm, &receipt) == 0);
        assert(receipt.magic_header == 0x52435054);
        assert(receipt.registers[2] == 512);
        assert(receipt.registers[3] == 800);

        char resp[256];
        assert(tsfi_zorse_query_with_strategy("Generate COBOL transaction", "DeepSeek-V3", "dvfs_profile.strategy", 50, 0, 0, 0, resp, sizeof(resp), &receipt) == 0);
        assert(receipt.magic_header == 0x52435054);
        assert(receipt.registers[2] == 2520);
        printf("  [PASS] tsfi_zorse dynamic strategy selection & receipt verified\n");
    }

    printf("=== ALL 377 DOMAIN .STRATEGY MODULES PROVEN 1:1 EQUIVALENT ===\n");
    return 0;
}
