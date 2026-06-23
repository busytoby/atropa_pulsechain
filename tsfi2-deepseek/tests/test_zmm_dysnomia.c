#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <unistd.h>
#include "tsfi_zmm_vm.h"
#include "tsfi_wire_firmware.h"

typedef struct {
    const char *name;
    const char *path;
    const char *virt_addr;
} DysnomiaContract;

static const DysnomiaContract g_dysnomia_system[] = {
    { "vmreq",        "../solidity/dysnomia/00b_vmreq.sol",              "0x9766a8d4fCe69F8b5544b99C13D1dA85bBBE762D" },
    { "shafactory",   "../solidity/dysnomia/02c_shafactory.sol",          "0xA865c31c6333CA7C1CBBF87F3D854D420797994F" },
    { "shiofactory",  "../solidity/dysnomia/03c_shiofactory.sol",         "0x737Df6B1d40A5f52fC5203a4E245A23725731769" },
    { "yi",           "../solidity/dysnomia/04_yi.sol",                  "0xff2A5Cc6f6c07d71933fccD5a09b893CD44F4Db7" },
    { "zheng",        "../solidity/dysnomia/05_zheng.sol",               "0x707389622103f0C5fC438FFd217f90663fFbdF12" },
    { "zhou",         "../solidity/dysnomia/06_zhou.sol",                "0x8dD417c945DA2a6E282237bbB0D1A55c2b95b467" },
    { "yau",          "../solidity/dysnomia/07_yau.sol",                 "0x40055f13377cfe5017e8F34167D0e404e050f06E" },
    { "yang",         "../solidity/dysnomia/08_yang.sol",                "0xe6B7baF9A4e09bb1D956Bb1b4af9Bd13539DC24f" },
    { "siu",          "../solidity/dysnomia/09_siu.sol",                 "0xab55A1628d908e9Ae55fdf5dd2cb0dDC043920Fc" },
    { "void",         "../solidity/dysnomia/10_void.sol",                "0xf65a7EF723B8a91755080bd8C1c70779dB77bBb5" },
    { "strings",      "../solidity/dysnomia/lib/stringlib.sol",          "0xf7a3B55128ebA848D4DdeAB28DfcebfE6D1FDA10" },
    { "libattribute", "../solidity/dysnomia/lib/attribute.sol",          "0xa30a7a10ED34722453c30da1083407a790f1D874" },
    { "corereactions","../solidity/dysnomia/lib/reactions_core.sol",      "0xFEC485Ad87db95099110C47FCCc8eB691A6FCf7c" },
    { "laufactory",   "../solidity/dysnomia/11c_laufactory.sol",         "0x4D05Cc16B140347fb67c427456791F67df6CAb39" },
    { "lau",          "../solidity/dysnomia/11_lau.sol",                  "0xBC366E9E93FcF0C741377Db15D938e9cAe96B18e" },
    { "cho",          "../solidity/dysnomia/domain/dan/01_cho.sol",      "0xB98E414af1fC02d6B5a6bF1CD08B844864B87d4f" },
    { "map",          "../solidity/dysnomia/domain/map.sol",             "0x60C7215A1C325Fc5fc91cd301DD94642142Baf80" },
    { "qing",         "../solidity/dysnomia/domain/dan/03_qing.sol",     "0xccb077a0" },
    { "qi",           "../solidity/dysnomia/domain/soeng/01_qi.sol",     "0x91F3ca10050862eccdbb7b265cc6fB49EeDD802C" },
    { "mai",          "../solidity/dysnomia/domain/soeng/02_mai.sol",     "0x2cc7e492a98cb211287A1506A55245dBa200E52a" },
    { "xia",          "../solidity/dysnomia/domain/soeng/03_xia.sol",     "0x6f7D105670842d39ACc32B5AeF0C5238AA0B9F3D" },
    { "xie",          "../solidity/dysnomia/domain/soeng/04_xie.sol",     "0xfa242E3A4FBAa0773559560B49bDeA35C975cBaf" },
    { "chan",         "../solidity/dysnomia/domain/sky/01_chan.sol",     "0xCb39Cc1afd2CEe8E8C82F9A330aBF37F4933d850" },
    { "sei",          "../solidity/dysnomia/domain/tang/01_sei.sol",     "0x8f4798DfDA3BC507954d768Bc33f3c456A407F30" },
    { "choa",         "../solidity/dysnomia/domain/sky/02_choa.sol",     "0xd8949704cc5acAf1600D908a2607AbeB668fAc6b" },
    { "cheon",        "../solidity/dysnomia/domain/tang/02_cheon.sol",    "0xcF54035Be7F6eC2664FEeaAcfbA4470BbB23E0A0" }
};

#include "lau_yul_thunk.h"

static uint64_t decode_uint256_scaled_by_1e18(const uint8_t *bytes) {
    uint64_t rem = 0;
    uint64_t quotient[4] = {0};
    uint64_t divisor = 1000000000000000000ULL;
    uint64_t digits[4];
    for (int i = 0; i < 4; i++) {
        digits[i] = 0;
        for (int b = 0; b < 8; b++) {
            digits[i] = (digits[i] << 8) | bytes[i * 8 + b];
        }
    }
    for (int i = 0; i < 4; i++) {
        unsigned __int128 dividend = ((unsigned __int128)rem << 64) | digits[i];
        quotient[i] = (uint64_t)(dividend / divisor);
        rem = (uint64_t)(dividend % divisor);
    }
    return quotient[3];
}

int main() {
    srand((unsigned int)time(NULL) ^ (unsigned int)getpid());
    printf("=== TSFi ZMM VM Full Dysnomia Systems Integration Test ===\n");
    remove("evm_storage.json");
    remove("tsfi2-deepseek/evm_storage.json");
    FILE *clear_fp = fopen("evm_storage.json", "w");
    if (clear_fp) {
        fclose(clear_fp);
    }
    clear_fp = fopen("tsfi2-deepseek/evm_storage.json", "w");
    if (clear_fp) {
        fclose(clear_fp);
    }
    tsfi_wire_firmware_init();

    TsfiZmmVmState vm;
    tsfi_zmm_vm_init(&vm);

    int count = sizeof(g_dysnomia_system) / sizeof(g_dysnomia_system[0]);
    printf("[INIT] Deploying %d Dysnomia contracts to ZMM VM...\n", count);

    for (int i = 0; i < count; i++) {
        char cmd[1024];
        uint64_t addr64 = 0;
        const char *addr_str = g_dysnomia_system[i].virt_addr;
        if (addr_str[0] == '0' && (addr_str[1] == 'x' || addr_str[1] == 'X')) addr_str += 2;
        size_t len = strlen(addr_str);
        if (len > 16) {
            sscanf(addr_str + len - 16, "%lx", &addr64);
        } else {
            sscanf(addr_str, "%lx", &addr64);
        }
        if (strcmp(g_dysnomia_system[i].name, "shafactory") == 0) {
            printf("\n -> [SEEDING] Calling vmreq.Random() 5 times to advance the PRNG...\n");
            for (int r = 0; r < 5; r++) {
                tsfi_zmm_vm_exec(&vm, "YULEXEC \"vmreq\", \"604a6fa9\"");
            }
            printf("\n");
        }
        snprintf(cmd, sizeof(cmd), "YULINIT \"%s\", \"%s\", %lu", 
                 g_dysnomia_system[i].name, 
                 g_dysnomia_system[i].path, 
                 addr64);
        tsfi_zmm_vm_exec(&vm, cmd);
    }

    printf("\n=== [CONSTRUCTOR SETUP] Verifying constructor configurations via execution loop ===\n");
    
    // Default wallet address context (0x4CC)
    const char *default_wallet = "0x00000000000000000000000000000000000004CC";
    
    // Query actual token balances held in the balance on the contract thunks
    for (int i = 0; i < count; i++) {
        uint64_t distribution_amount = 0;
        if (strcmp(g_dysnomia_system[i].name, "shafactory") != 0 &&
            strcmp(g_dysnomia_system[i].name, "shiofactory") != 0 &&
            strcmp(g_dysnomia_system[i].name, "laufactory") != 0 &&
            strcmp(g_dysnomia_system[i].name, "strings") != 0 &&
            strcmp(g_dysnomia_system[i].name, "libattribute") != 0 &&
            strcmp(g_dysnomia_system[i].name, "corereactions") != 0) {
            
            // Selector for balanceOf(address) is 70a08231
            uint8_t calldata[36] = {0};
            calldata[0] = 0x70;
            calldata[1] = 0xa0;
            calldata[2] = 0x82;
            calldata[3] = 0x31;
            calldata[34] = 0x04;
            calldata[35] = 0xcc;
            
            uint8_t retval[32] = {0};
            size_t retval_len = sizeof(retval);
            bool success = lau_yul_thunk_execute(g_dysnomia_system[i].name, calldata, sizeof(calldata), retval, &retval_len);
            if (success && retval_len == 32) {
                distribution_amount = decode_uint256_scaled_by_1e18(retval);
            }
        }
        printf(" -> [INITCODE] Executed constructor for tier: %s. Minted %lu.000000000000000000 tokens to %s\n", 
               g_dysnomia_system[i].name, 
               distribution_amount, 
               default_wallet);
    }

    printf("\n=== [TEST 1] Creating a new User LAU token from LAUFactory on ZMM VM ===\n");
    // Calldata for New("User Token 1", "USERTOKEN1"):
    // Selector: 8c6dc5ec
    const char *calldata_new = "8c6dc5ec00000000000000000000000000000000000000000000000000000000000000400000000000000000000000000000000000000000000000000000000000000080000000000000000000000000000000000000000000000000000000000000000c5573657220546f6b656e20310000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000a55534552544f4b454e3100000000000000000000000000000000000000000000";

    char exec_cmd[1024];
    snprintf(exec_cmd, sizeof(exec_cmd), "YULEXEC \"laufactory\", \"%s\"", calldata_new);
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    printf(" -> Executing: %s\n", exec_cmd);
    tsfi_zmm_vm_exec(&vm, exec_cmd);

    printf("\n=== [TEST 2] Chatting on the Void with LAU token on ZMM VM ===\n");
    // Calldata for Chat("hello void"):
    // Selector: 21516fc4
    const char *calldata_chat = "21516fc40000000000000000000000000000000000000000000000000000000000000020000000000000000000000000000000000000000000000000000000000000000a68656c6c6f20766f696400000000000000000000000000000000000000000000";
    
    char chat_cmd[1024];
    snprintf(chat_cmd, sizeof(chat_cmd), "YULEXEC \"lau\", \"%s\"", calldata_chat);
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    printf(" -> Executing: %s\n", chat_cmd);
    tsfi_zmm_vm_exec(&vm, chat_cmd);

    int logs_count = lau_yul_thunk_get_log_count();
    printf(" -> Manual Event Verification: Emitted %d events in last transaction\n", logs_count);
    for (int l = 0; l < logs_count; l++) {
        uint64_t addr = 0;
        int num_topics = 0;
        u256_t topics[4];
        uint8_t data[2048] = {0};
        size_t d_size = sizeof(data);
        if (lau_yul_thunk_get_log(l, &addr, &num_topics, topics, data, &d_size)) {
            printf("      * Log %d: contract=0x%lx, topics=%d", l, addr, num_topics);
            for (int t = 0; t < num_topics; t++) {
                printf(", topic%d=0x%016lx%016lx%016lx%016lx", t, topics[t].d[3], topics[t].d[2], topics[t].d[1], topics[t].d[0]);
            }
            printf("\n");
        }
    }

    tsfi_zmm_vm_destroy(&vm);
    printf("=== ALL FULL DYSNOMIA SYSTEM INTEGRATION TESTS COMPLETED ===\n");
    return 0;
}
