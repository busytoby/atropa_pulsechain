#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_zmm_vm.h"
#include "tsfi_wire_firmware.h"

typedef struct {
    const char *name;
    const char *path;
    const char *virt_addr;
} DysnomiaContract;

static const DysnomiaContract g_dysnomia_system[] = {
    { "vmreq",        "../solidity/dysnomia/00b_vmreq.sol",              "0x10" },
    { "shafactory",   "../solidity/dysnomia/02c_shafactory.sol",          "0x11" },
    { "shiofactory",  "../solidity/dysnomia/03c_shiofactory.sol",         "0x12" },
    { "yi",           "../solidity/dysnomia/04_yi.sol",                  "0x83a918056aB9316837Dc48a216119D679D561d91" },
    { "zheng",        "../solidity/dysnomia/05_zheng.sol",               "0x14" },
    { "zhou",         "../solidity/dysnomia/06_zhou.sol",                "0xe5d3A6e88590fc2A8037D9CCbd816C05B1ff5f11" },
    { "yau",          "../solidity/dysnomia/07_yau.sol",                 "0xb9A44De20f26a027e467CB6c2F98766F01904189" },
    { "yang",         "../solidity/dysnomia/08_yang.sol",                "0xFE9b99eCC43cb423408b975cc5ff439e5ABaCb61" },
    { "siu",          "../solidity/dysnomia/09_siu.sol",                 "0xb4C1248812dAbF72cb2e82175b4c0aCffE4D2b10" },
    { "void",         "../solidity/dysnomia/10_void.sol",                "0xCd19062a6d3019b02A676D72e51D8de7A398dE25" },
    { "laufactory",   "../solidity/dysnomia/11c_laufactory.sol",         "0x0EB4EE7d5Ff28cbF68565A174f7E5e186c36B4b3" },
    { "lau",          "../solidity/dysnomia/11_lau.sol",                  "0x25" },
    { "cho",          "../solidity/dysnomia/01_cho.sol",                 "0x1b" },
    { "map",          "../solidity/dysnomia/domain/map.sol",             "0x1c" },
    { "qi",           "../solidity/dysnomia/domain/soeng/01_qi.sol",     "0x1b8F9E19360D1dc94295D984b7Ca7eA9b810D9ee" },
    { "mai",          "../solidity/dysnomia/domain/soeng/02_mai.sol",     "0xf69e9f943674027Cedf05564A8D5A01041d07c62" },
    { "xia",          "../solidity/dysnomia/domain/soeng/03_xia.sol",     "0x347BC40503E0CE23fE0F5587F232Cd2D07D4Eb89" },
    { "xie",          "../solidity/dysnomia/domain/soeng/04_xie.sol",     "0x2556F7f8d82EbcdD7b821b0981C38D9dA9439CdD" },
    { "chan",         "../solidity/dysnomia/domain/sky/01_chan.sol",     "0x7674516ad438dd67A057fBc1119168d9A7d2a9B1" },
    { "sei",          "../solidity/dysnomia/domain/tang/01_sei.sol",     "0x8B090509eAe0fEB4A0B934de1b4345161fA9a62d" },
    { "choa",         "../solidity/dysnomia/domain/sky/02_choa.sol",     "0xA63F8061A67ecdbf147Cd1B60f91Cf95464E868D" },
    { "cheon",        "../solidity/dysnomia/domain/tang/02_cheon.sol",    "0x840CBD20A70774BECAc4e932Fff6fb1f5417997F" }
};

int main() {
    printf("=== TSFi ZMM VM Full Dysnomia Systems Integration Test ===\n");
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
        snprintf(cmd, sizeof(cmd), "YULINIT \"%s\", \"%s\", %lu", 
                 g_dysnomia_system[i].name, 
                 g_dysnomia_system[i].path, 
                 addr64);
        tsfi_zmm_vm_exec(&vm, cmd);
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

    tsfi_zmm_vm_destroy(&vm);
    printf("=== ALL FULL DYSNOMIA SYSTEM INTEGRATION TESTS COMPLETED ===\n");
    return 0;
}
