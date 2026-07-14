#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <time.h>
#include "tsfi_zmm_vm.h"
#include "lau_memory.h"
#include "tsfi_zmm_rpc.h"
#include "lau_telemetry.h"
#include "tsfi_raw.h"
#include "tsfi_io.h"

extern void tsfi_zmm_rpc_step_async_llm(TsfiZmmVmState *state);

// In-Memory MCP Loop (Bridges stdio, Network Port 10042, and SHM Command Channel)

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
    { "qing_2cc_0xe9f5ef3994fe76d83b0faab75062e86457ff96b9", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xe9f5ef3994fe76d83b0faab75062e86457ff96b9" },
    { "qing_bar_0x38f7bbbb1eb74de568c907aa47f3de199c6cefd5", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x38f7bbbb1eb74de568c907aa47f3de199c6cefd5" },
    { "qing_bullion_0x158f8e909d1eb4f7c165807899f1face6b76c12e", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x158f8e909d1eb4f7c165807899f1face6b76c12e" },
    { "qing_berwickshirenaturalistsclub_0x59f6e35871a21c7cb0f23b04ba05700309b79637", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x59f6e35871a21c7cb0f23b04ba05700309b79637" },
    { "qing_campaign_0x42a239df6f5a82cbc579acb653be347176d6b3e7", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x42a239df6f5a82cbc579acb653be347176d6b3e7" },
    { "qing_canasta_0x12891b0b9fac8b5f68fef751e1201c10c72fc422", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x12891b0b9fac8b5f68fef751e1201c10c72fc422" },
    { "qing_cia_0x6e43215f456627d7216e4c88cfcf126e733b4ece", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x6e43215f456627d7216e4c88cfcf126e733b4ece" },
    { "qing_dai_0x644aabeacd96938a7ea5910f0c5240d289b64cd1", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x644aabeacd96938a7ea5910f0c5240d289b64cd1" },
    { "qing_dominance_0x7155dd05a4a9cb58c471448cf875e1c7f302947f", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x7155dd05a4a9cb58c471448cf875e1c7f302947f" },
    { "qing_decivitatedei_0xa846857cad1153bc9ce3ad7391dc7f1d060d2d75", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xa846857cad1153bc9ce3ad7391dc7f1d060d2d75" },
    { "qing_fdc_0x59bfa4b54bbdfa0e55640c52a9fa4eb69891e7a4", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x59bfa4b54bbdfa0e55640c52a9fa4eb69891e7a4" },
    { "qing_fd_0xf628ddf33e97cfe53826e839241200e148394b16", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xf628ddf33e97cfe53826e839241200e148394b16" },
    { "qing_gbaby_0x3a3f249db68a0df584a18098ececc5f6f0ca3944", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x3a3f249db68a0df584a18098ececc5f6f0ca3944" },
    { "qing_grill_0xe0230d9803bd212c71177a2e9a8bafa54dc70671", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xe0230d9803bd212c71177a2e9a8bafa54dc70671" },
    { "qing_hex_0x900ae7e39d13f8d0dbc9bd6f2f117c144efb489a", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x900ae7e39d13f8d0dbc9bd6f2f117c144efb489a" },
    { "qing_inc_0x1e57aede4afb1faad4c6ad3da711fb26e7a44d00", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x1e57aede4afb1faad4c6ad3da711fb26e7a44d00" },
    { "qing_kippah_0x4611f357caf4293269c0c4f00233c921ffe6cc26", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x4611f357caf4293269c0c4f00233c921ffe6cc26" },
    { "qing_knightsofmalta_0xcebcf3d363d6272b18f8708c9844e3c329c002c5", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xcebcf3d363d6272b18f8708c9844e3c329c002c5" },
    { "qing_lcc_0x0972c7cae9857107200397ae1e20f21b59da69c1", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x0972c7cae9857107200397ae1e20f21b59da69c1" },
    { "qing_legal_0x83ec6acdfd04c7fdc98403937f8a18f7a8cd8399", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x83ec6acdfd04c7fdc98403937f8a18f7a8cd8399" },
    { "qing_nba_0x4eb11dcd373c3857e9544a0e6de956c65409f733", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x4eb11dcd373c3857e9544a0e6de956c65409f733" },
    { "qing_pirateswithattitude_0x33f84d5f754c6bcdc55cab4c712fff405895352b", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x33f84d5f754c6bcdc55cab4c712fff405895352b" },
    { "qing_sg_0x606bfffb10a3a562a96c0e82408d0704e69dfe08", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x606bfffb10a3a562a96c0e82408d0704e69dfe08" },
    { "qing_shib_0xf2c4087a18781d5347d4905e86b087b1b9d6ba51", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xf2c4087a18781d5347d4905e86b087b1b9d6ba51" },
    { "qing_stu_0x9f533369e5090c4da895ce9cee28d955096f1215", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x9f533369e5090c4da895ce9cee28d955096f1215" },
    { "qing_treasurybill_0x015a15727d5b21f58b5ab56a86959a3b32849ba9", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x015a15727d5b21f58b5ab56a86959a3b32849ba9" },
    { "qing_trsi_0x04482c949f99177e53befe42bbc20e09ba317428", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x04482c949f99177e53befe42bbc20e09ba317428" },
    { "qing_the5thcolumn_0x6c92dcd185bb2633ea333da02fc78106c7217926", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x6c92dcd185bb2633ea333da02fc78106c7217926" },
    { "qing_usdc_0x39e9e8edf016524757a8202856e366b1d25e0a12", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x39e9e8edf016524757a8202856e366b1d25e0a12" },
    { "qing_wbtc_0x85e14bb47b22cf82a805b424ce605ea025ca751e", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x85e14bb47b22cf82a805b424ce605ea025ca751e" },
    { "qing_wpls_0x8157a5ac755d70218683232b06d6062253104918", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x8157a5ac755d70218683232b06d6062253104918" },
    { "qing_z_0xb0ba7d36b7f0505879179ece7401f24eb653c6e1", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xb0ba7d36b7f0505879179ece7401f24eb653c6e1" },
    { "qing_monatmoney_0x2554a08e8467701f826708095fbd968989fc1da8", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x2554a08e8467701f826708095fbd968989fc1da8" },
    { "qing_qing_008d99_0x008d99c30628d0b94be2076eb62c1e1d8a092048", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x008d99c30628d0b94be2076eb62c1e1d8a092048" },
    { "qing_qing_20ef62_0x20ef620ec63318a84eb5fe6010f6bb2a4f61ade1", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x20ef620ec63318a84eb5fe6010f6bb2a4f61ade1" },
    { "qing_qing_2cd4e9_0x2cd4e92c567604337e09eb3b6265bae7bba9a2ef", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x2cd4e92c567604337e09eb3b6265bae7bba9a2ef" },
    { "qing_qing_49b633_0x49b63324b5288908989fcbe613a6fa5b988fd9ad", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x49b63324b5288908989fcbe613a6fa5b988fd9ad" },
    { "qing_qing_f9d9f7_0xf9d9f7cd9d14c03a00410df7f4be042ae8dfd70a", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xf9d9f7cd9d14c03a00410df7f4be042ae8dfd70a" },
    { "qing_qing_73f2a2_0x73f2a2193abe85ba1290e08b04dce1d64e91827b", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x73f2a2193abe85ba1290e08b04dce1d64e91827b" },
    { "qing_qing_ac97af_0xac97aff0c05ef3840c991d9205f53be80ef8f1f2", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xac97aff0c05ef3840c991d9205f53be80ef8f1f2" },
    { "qing_qing_a67eff_0xa67effbfa307c6c9c6ea63e39654b97a6d8a334b", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xa67effbfa307c6c9c6ea63e39654b97a6d8a334b" },
    { "qing_qing_2245e4_0x2245e41d46f6ebd511bc09e5330dda30fc19ea18", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x2245e41d46f6ebd511bc09e5330dda30fc19ea18" },
    { "qing_qing_b45bd2_0xb45bd2d8728ff0cf320684fbce79c27bca036396", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xb45bd2d8728ff0cf320684fbce79c27bca036396" },
    { "qing_qing_b33523_0xb33523015f92835b3d7d63ccc51fa0bb90507a96", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xb33523015f92835b3d7d63ccc51fa0bb90507a96" },
    { "qing_qing_baae0a_0xbaae0ab51ff6c0ea1ed3cc64f981b662f6a8113f", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xbaae0ab51ff6c0ea1ed3cc64f981b662f6a8113f" },
    { "qing_qing_3990e2_0x3990e20d1a23f7e1bd7cc1956b32e38ea1649b74", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x3990e20d1a23f7e1bd7cc1956b32e38ea1649b74" },
    { "qing_qing_1299bb_0x1299bbac80f7038c19c20843f80608bd80c14a74", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x1299bbac80f7038c19c20843f80608bd80c14a74" },
    { "qing_qing_186dff_0x186dff4a427cf5eb82e087d9605c4e156266b851", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x186dff4a427cf5eb82e087d9605c4e156266b851" },
    { "qing_billburr_0xcaea70828578ed9dc2d68d0aa1f3755edc153385", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xcaea70828578ed9dc2d68d0aa1f3755edc153385" },
    { "qing_qing_e4169c_0xe4169cf239e2fdc6e6e4a3a6fa0a703058ce400b", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xe4169cf239e2fdc6e6e4a3a6fa0a703058ce400b" },
    { "qing_illinois_0xd127839b83cbf537c29b7d268c135f745a0da2e8", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xd127839b83cbf537c29b7d268c135f745a0da2e8" },
    { "qing_qing_e41165_0xe41165bfd2c38a682289b2783d1e6e2e39e3766a", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xe41165bfd2c38a682289b2783d1e6e2e39e3766a" },
    { "qing_qing_bfaa31_0xbfaa31c5eab4fcb21fe7cafcbbacb35593594589", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xbfaa31c5eab4fcb21fe7cafcbbacb35593594589" },
    { "qing_qing_d0ccbb_0xd0ccbb3e2aad138c4e7e8727887557823f3c296c", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xd0ccbb3e2aad138c4e7e8727887557823f3c296c" },
    { "qing_qing_f2efa1_0xf2efa194b5571e407790998b14ed82bced2be3b1", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xf2efa194b5571e407790998b14ed82bced2be3b1" },
    { "qing_qing_f56a48_0xf56a48a8bff503d20745f2245ca1a208c870e787", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xf56a48a8bff503d20745f2245ca1a208c870e787" },
    { "qing_enteh_0xa43f71ac277022a547c56706fbbc5d93f88c3467", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xa43f71ac277022a547c56706fbbc5d93f88c3467" },
    { "qing_qing_5ae65a_0x5ae65a18c06011daab861106983cf72c153ad1fe", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x5ae65a18c06011daab861106983cf72c153ad1fe" },
    { "qing_eris_0x4d8a456a18ae141b04a983389409038ea1fc709d", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x4d8a456a18ae141b04a983389409038ea1fc709d" },
    { "qing_usdc_0xa1ddbbcab4f62e7968cc4adbe8ca0634ee509956", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xa1ddbbcab4f62e7968cc4adbe8ca0634ee509956" },
    { "qing_usdt_0x4c74302e0c64d2f03cf93135ae1d70f3487cd7ee", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x4c74302e0c64d2f03cf93135ae1d70f3487cd7ee" },
    { "qing_qing_bc8faa_0xbc8faad427d17ae6ae386bb073d3ad10b50eb91b", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xbc8faad427d17ae6ae386bb073d3ad10b50eb91b" },
    { "qing_qing_9cfac9_0x9cfac9d4db9a166d2c5aee14b6ab796382a9eda8", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x9cfac9d4db9a166d2c5aee14b6ab796382a9eda8" },
    { "qing_qing_6e1f57_0x6e1f57ece89acdc669017da539baac13c3d952d2", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x6e1f57ece89acdc669017da539baac13c3d952d2" },
    { "qing_scissors_0xcbfca49631570387cb04577b82d55be67b316c74", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xcbfca49631570387cb04577b82d55be67b316c74" },
    { "qing_qing_ec9925_0xec9925a808ddfe4ebd283759489c03cbba9dc181", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xec9925a808ddfe4ebd283759489c03cbba9dc181" },
    { "qing_qing_1ce307_0x1ce307593cbe3c13b2624bc95aa7d10c9fb335b1", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x1ce307593cbe3c13b2624bc95aa7d10c9fb335b1" },
    { "qing_qing_4dd037_0x4dd0371c02631bfd17ad10ab7c0e35a047ff2d20", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x4dd0371c02631bfd17ad10ab7c0e35a047ff2d20" },
    { "qing_qing_0a2f46_0x0a2f46d1d07ea169acea81bdc6d73ed0f5812634", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x0a2f46d1d07ea169acea81bdc6d73ed0f5812634" },
    { "qing_qing_fb1d79_0xfb1d79843b4be8c9fc244f54ee965d051261bb4b", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xfb1d79843b4be8c9fc244f54ee965d051261bb4b" },
    { "qing_phd_0x561854662df671890b8fba538dedbd2cddcf7e32", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x561854662df671890b8fba538dedbd2cddcf7e32" },
    { "qing_math_0x872739df73cad8ec87418bc0b00eb9925bbf6971", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x872739df73cad8ec87418bc0b00eb9925bbf6971" },
    { "qing_qing_a55441_0xa55441e74cb64f9de4a80183d435b05b30b0d9a2", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xa55441e74cb64f9de4a80183d435b05b30b0d9a2" },
    { "qing_qing_ed65c2_0xed65c29cc739402db87b1befe12679402199d5d5", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xed65c29cc739402db87b1befe12679402199d5d5" },
    { "qing_poppy_0x829f778b5857eca60c4c44f187a92395f453b4da", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x829f778b5857eca60c4c44f187a92395f453b4da" },
    { "qing_qing_a65ac1_0xa65ac1bf7e70f8545575c50f40c43f3c898389b4", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xa65ac1bf7e70f8545575c50f40c43f3c898389b4" },
    { "qing_qing_a03d15_0xa03d1569bb76fb23c0d2779866e6e80871d64b58", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xa03d1569bb76fb23c0d2779866e6e80871d64b58" },
    { "qing_usb_0xd70acfbb2a88bfddc2e26f937a138616226552bc", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xd70acfbb2a88bfddc2e26f937a138616226552bc" },
    { "qing_red_0xf398c7aedd6af9f357325d6e402503cbc45ee698", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xf398c7aedd6af9f357325d6e402503cbc45ee698" },
    { "qing_yogi_0xeb818b831dfdfee90e3a036456dfeaa012fce9ef", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xeb818b831dfdfee90e3a036456dfeaa012fce9ef" },
    { "qing_tod_0x776fded4e2fedfa01cde2930bf703d2fe48baae2", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x776fded4e2fedfa01cde2930bf703d2fe48baae2" },
    { "qing_qing_6b1216_0x6b1216d7c563d4a02d2cbe1a21655590a7f670d4", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x6b1216d7c563d4a02d2cbe1a21655590a7f670d4" },
    { "qing_qing_30057b_0x30057be721fb94787ad9c75dc6648dd40c35eae2", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x30057be721fb94787ad9c75dc6648dd40c35eae2" },
    { "qing_qing_8ea4aa_0x8ea4aa2cd5d7e7b5d4f7f7aed7d23a406c12ce8e", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x8ea4aa2cd5d7e7b5d4f7f7aed7d23a406c12ce8e" },
    { "qing_qing_6f6a1b_0x6f6a1b16f98c6c23edcd05b0d34bf1f404a2ca3a", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x6f6a1b16f98c6c23edcd05b0d34bf1f404a2ca3a" },
    { "qing_host_0x903aaa437d8dd3122b362b5f339cd27a53865d2c", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x903aaa437d8dd3122b362b5f339cd27a53865d2c" },
    { "qing_qing_8e52de_0x8e52de27dc8e108f66ebbbed5c47a024384647cb", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x8e52de27dc8e108f66ebbbed5c47a024384647cb" },
    { "qing_qing_23c8a8_0x23c8a89fa859023ff7c7f367df733af14e0719a5", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x23c8a89fa859023ff7c7f367df733af14e0719a5" },
    { "qing_qing_1fb8d9_0x1fb8d93f7d7d49a6f8892137838add45c48793d0", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x1fb8d93f7d7d49a6f8892137838add45c48793d0" },
    { "qing_acab_0x652ba47682c158673be8784a9aa9c125ea8715bb", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x652ba47682c158673be8784a9aa9c125ea8715bb" },
    { "qing_qing_8089b2_0x8089b25f1585c806cc0d9c9dc130665627b0b5da", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x8089b25f1585c806cc0d9c9dc130665627b0b5da" },
    { "qing_qing_e840b6_0xe840b6e56e269eb5b23bf3d1e6b7bbc9fe742c0c", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xe840b6e56e269eb5b23bf3d1e6b7bbc9fe742c0c" },
    { "qing_qing_c24c9e_0xc24c9e44b1dd1d0e5e9402d13163b3bce47436a6", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xc24c9e44b1dd1d0e5e9402d13163b3bce47436a6" },
    { "qing_qing_7f326f_0x7f326f7fd8e308337b9c94f198dc6742d5958752", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x7f326f7fd8e308337b9c94f198dc6742d5958752" },
    { "qing_qing_626337_0x626337c72c8940b965ad48d503c3a7ae6abd1a9d", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x626337c72c8940b965ad48d503c3a7ae6abd1a9d" },
    { "qing_chickendinner_0x9fde2c9e2039c21461825d2504291e7a659a445a", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x9fde2c9e2039c21461825d2504291e7a659a445a" },
    { "qing_tabo_0x6620d4400ff8803fc893a28bb4d69f7300127a3e", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x6620d4400ff8803fc893a28bb4d69f7300127a3e" },
    { "qing_mfti_0x0835072e4f230e81a9c22155034dad78d950cbcd", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x0835072e4f230e81a9c22155034dad78d950cbcd" },
    { "qing_v_0xe8a7ddb2e302385b1d2c9de1ea368023f3e6c115", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xe8a7ddb2e302385b1d2c9de1ea368023f3e6c115" },
    { "qing_larp_0xe5682b404850bc83b8c6836cc6b730776833d104", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xe5682b404850bc83b8c6836cc6b730776833d104" },
    { "qing_qing_ad7cc5_0xad7cc51457ddae47dccb55fdc10168bc1b49c7ee", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xad7cc51457ddae47dccb55fdc10168bc1b49c7ee" },
    { "qing_yt_0xf426280a166dc9797a906d513350a4429d1f765b", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xf426280a166dc9797a906d513350a4429d1f765b" },
    { "qing_ltdefi_0x79f7f57f343da5f3c6d282d837d3847c8a085b35", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x79f7f57f343da5f3c6d282d837d3847c8a085b35" },
    { "qing_cho_0x012b805d8fb0bb53c4e9189328006b9c6d56a44c", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x012b805d8fb0bb53c4e9189328006b9c6d56a44c" },
    { "qing_qing_dd0118_0xdd0118dffcccc1ce809b35b97a69dcd56a4a4a04", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xdd0118dffcccc1ce809b35b97a69dcd56a4a4a04" },
    { "qing_toot_test_0xc23219d2d454383cd6b81bf6f21fd058510e2d87", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xc23219d2d454383cd6b81bf6f21fd058510e2d87" },
    { "qing_yue_test_0x2dad80bccd63ae144aa84ce5cd110e24afd55fd7", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x2dad80bccd63ae144aa84ce5cd110e24afd55fd7" },
    { "qing_qing_d11f68_0xd11f6892e2d7df8422fba01680ec8c7cd7d28457", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xd11f6892e2d7df8422fba01680ec8c7cd7d28457" },
    { "qing_lyphe_0x73f884b83303320872ddff25aab056e748597d89", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x73f884b83303320872ddff25aab056e748597d89" },
    { "qing_qing_d79fd2_0xd79fd2fff5dad1c330d757d16dc3283b8b4ac6f2", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xd79fd2fff5dad1c330d757d16dc3283b8b4ac6f2" },
    { "qing_teh711_0x902fb3ed4298f70b14c0fb32fbf3e6f299c82ad3", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x902fb3ed4298f70b14c0fb32fbf3e6f299c82ad3" },
    { "qing_qing_55c8bb_0x55c8bb6e3b2eb74ff4e17894139f7e91d77fee06", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x55c8bb6e3b2eb74ff4e17894139f7e91d77fee06" },
    { "qing_qing_10d085_0x10d0855f1bb9d3a239a7953317cdd38419c93996", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x10d0855f1bb9d3a239a7953317cdd38419c93996" },
    { "qing_qing_b9fbe2_0xb9fbe2404fe67dc1498f90930c722e49fa9ba828", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xb9fbe2404fe67dc1498f90930c722e49fa9ba828" },
    { "qing_jabbie_0x8d2c98f0e8071dc875d771dd38af5b3025dc519c", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x8d2c98f0e8071dc875d771dd38af5b3025dc519c" },
    { "qing_qing_90b724_0x90b724bb34c6f6e629caeffb98b66b62f57d4450", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x90b724bb34c6f6e629caeffb98b66b62f57d4450" },
    { "qing_eoe_0x2baa47c72d416553761d1dc74761ded7dc89f38e", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x2baa47c72d416553761d1dc74761ded7dc89f38e" },
    { "qing_m8_0x61c54cbabe5e4b64164e26fcd73de83556a4311e", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x61c54cbabe5e4b64164e26fcd73de83556a4311e" },
    { "qing_qing_6152e1_0x6152e1b78a4f428bf26348b658e7107c6bcf747c", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x6152e1b78a4f428bf26348b658e7107c6bcf747c" },
    { "qing_zg_0x4c6fc9ef5f930b7b0a23978ec73f13d44cfde82e", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x4c6fc9ef5f930b7b0a23978ec73f13d44cfde82e" },
    { "qing_qing_3c6047_0x3c6047c8637a24ff2dbeca1260190f1e1c5b1861", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x3c6047c8637a24ff2dbeca1260190f1e1c5b1861" },
    { "qing_hyperlight_0x9fca2363838960f75735ad35ebc34d6f25afd164", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x9fca2363838960f75735ad35ebc34d6f25afd164" },
    { "qing_qing_1f11e9_0x1f11e98298d93da20cbc9b7acb7993dcf1ac5e21", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x1f11e98298d93da20cbc9b7acb7993dcf1ac5e21" },
    { "qing_map_0x58065d1351972d9358665602c967ea58c13dc744", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x58065d1351972d9358665602c967ea58c13dc744" },
    { "qing_pfy_0x558fd2240d65b2cb72098444f5b888d8792ca8c5", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x558fd2240d65b2cb72098444f5b888d8792ca8c5" },
    { "qing_ricrih_0xbc1b23d5aa37fa3a7cbaa1b8967d7b2dc28374a1", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xbc1b23d5aa37fa3a7cbaa1b8967d7b2dc28374a1" },
    { "qing_pfy_0x214bbeb1b93d476ec173833827a2ddcd40762fb7", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x214bbeb1b93d476ec173833827a2ddcd40762fb7" },
    { "qing_qing_b91892_0xb91892d4d500551fcb6408144a0ba0eb9f00a042", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xb91892d4d500551fcb6408144a0ba0eb9f00a042" },
    { "qing_spike_0x18ea4acea0be7dc27099291416e3e2dba6380518", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x18ea4acea0be7dc27099291416e3e2dba6380518" },
    { "qing_cookies_0x809f3eece0998fdd698ae41581bfa8c2e0411b78", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x809f3eece0998fdd698ae41581bfa8c2e0411b78" },
    { "qing_qing_1cd36f_0x1cd36f0607fd28ca5c386f7fc9ac3032f9b19801", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x1cd36f0607fd28ca5c386f7fc9ac3032f9b19801" },
    { "qing_ups_0x5e97b5e8ad1621fe79a7dbedf890c6b03b085233", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x5e97b5e8ad1621fe79a7dbedf890c6b03b085233" },
    { "qing_nsdq_0xd898c85c071128396fb6082221d545ca01803ed5", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xd898c85c071128396fb6082221d545ca01803ed5" },
    { "qing_dwjnes_0x601f9a779b6f23814df81698d72e13eb017b413d", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x601f9a779b6f23814df81698d72e13eb017b413d" },
    { "qing_qing_810596_0x8105962336bc3c7e7b662d41b3a20411c7cf7481", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x8105962336bc3c7e7b662d41b3a20411c7cf7481" },
    { "qing_qing_8b78d1_0x8b78d18751050b35045c4a65edda9909b2c1f819", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x8b78d18751050b35045c4a65edda9909b2c1f819" },
    { "qing_koan_0xc56a3e96cb971be4119cdf3094ecd6b8bbb4f622", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xc56a3e96cb971be4119cdf3094ecd6b8bbb4f622" },
    { "qing_t3v3n_0x923a2c6d923486ee4cb1d24f6f9ccaa702032ca5", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x923a2c6d923486ee4cb1d24f6f9ccaa702032ca5" },
    { "qing_qing_1b8132_0x1b813252b66583487cd2af1dbb2f79f6944e462f", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x1b813252b66583487cd2af1dbb2f79f6944e462f" },
    { "qing_qing_6ee5fb_0x6ee5fbced6e7b5df816ff0502b90e97cb7ae7d27", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x6ee5fbced6e7b5df816ff0502b90e97cb7ae7d27" },
    { "qing_jdd_0x2e20de33b21a96db0c6554155967ba3701143d81", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x2e20de33b21a96db0c6554155967ba3701143d81" },
    { "qing_qing_33d7ca_0x33d7ca680f3ded76414f82d9ffc0ad13e946a45d", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x33d7ca680f3ded76414f82d9ffc0ad13e946a45d" },
    { "qing_idgaf_0x8d293a6728b76938d8f3ba8ca4c3ff5f70b059b1", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x8d293a6728b76938d8f3ba8ca4c3ff5f70b059b1" },
    { "qing_qing_e3d57c_0xe3d57c5e482ec49758218b09317986fc7507777c", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xe3d57c5e482ec49758218b09317986fc7507777c" },
    { "qing_qing_92dfb0_0x92dfb0ed3d95644784f9d428c65b642899d6914a", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x92dfb0ed3d95644784f9d428c65b642899d6914a" },
    { "qing_ideall_0x72ed037419af06c07051420dde440141232cc886", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x72ed037419af06c07051420dde440141232cc886" },
    { "qing_qing_b91786_0xb9178657dca756acbb1dbc7885bd773947f74ff4", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xb9178657dca756acbb1dbc7885bd773947f74ff4" },
    { "qing_choke_0x8cf5f218113ea5c23abedf58787da58e4c8c91a6", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x8cf5f218113ea5c23abedf58787da58e4c8c91a6" },
    { "qing_shield_0x43f5fb4b95a345b18ee41961a83af44e828983a3", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x43f5fb4b95a345b18ee41961a83af44e828983a3" },
    { "qing_ppusdt_0x12f094196783b993b86080bcfa48f7a96773427e", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x12f094196783b993b86080bcfa48f7a96773427e" },
    { "qing_pp_0x139298dbccc50e837d8a23e3d77ed5a3150d78fd", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x139298dbccc50e837d8a23e3d77ed5a3150d78fd" },
    { "qing_ppcrows_0x32503eaf0009b15b85e3a35caa7981045fdf5b2f", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x32503eaf0009b15b85e3a35caa7981045fdf5b2f" },
    { "qing_ppfinal_0x06e8e1250eb000ed1bed35e8106de0809154ae76", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x06e8e1250eb000ed1bed35e8106de0809154ae76" },
    { "qing_massacrefreebies_0xd60186918acb0e2117869dae864d8cde1d388e71", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xd60186918acb0e2117869dae864d8cde1d388e71" },
    { "qing_qing_301ba1_0x301ba124be42a0e7a8f0d583c532ad8d887065f9", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x301ba124be42a0e7a8f0d583c532ad8d887065f9" },
    { "qing_ppmassacrefreebies_0xe39690a43c3f979a413192a8cb5965602fda826b", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xe39690a43c3f979a413192a8cb5965602fda826b" },
    { "qing_pp_0x7343d8afa9d6e3376873ea24ccba7c7230aab14b", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x7343d8afa9d6e3376873ea24ccba7c7230aab14b" },
    { "qing_wagmi_0xf2a0636c05716cf4ce4937cb88348c8e2eccf8e7", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xf2a0636c05716cf4ce4937cb88348c8e2eccf8e7" },
    { "qing_shio_0xf8b9487137ac32c9f68c044b29b06705a4a6b836", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xf8b9487137ac32c9f68c044b29b06705a4a6b836" },
    { "qing_qing_1661d4_0x1661d42f37273f17c155fbf946c6dbc813fc8c21", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x1661d42f37273f17c155fbf946c6dbc813fc8c21" },
    { "qing_4pp_0xfa767ed66e6868e726c78e59b8adc94732172f13", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xfa767ed66e6868e726c78e59b8adc94732172f13" },
    { "qing_qing_66c769_0x66c76936fc07456b662346351c26948655934c3f", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x66c76936fc07456b662346351c26948655934c3f" },
    { "qing_qing_4226a7_0x4226a7802fb3281d690134060bd36bf31854c57b", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x4226a7802fb3281d690134060bd36bf31854c57b" },
    { "qing_snack_0x81b74e9a2eaf3c024ecbb19ee6d592363f206a1b", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x81b74e9a2eaf3c024ecbb19ee6d592363f206a1b" },
    { "qing_weth_0x875360521d23456af5cf561bd0b1c1fdbf47c4cb", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x875360521d23456af5cf561bd0b1c1fdbf47c4cb" },
    { "qing_dai_0x693424a8a747823c4f682d9bd2aa59d3c1cfee6a", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x693424a8a747823c4f682d9bd2aa59d3c1cfee6a" },
    { "qing_usdt_0x86be06efbebb6cba9d2303c2175037da66477794", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x86be06efbebb6cba9d2303c2175037da66477794" },
    { "qing_wbtc_0xeed783edc584aa1e796f28e3a2b3e37e8c3393c7", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xeed783edc584aa1e796f28e3a2b3e37e8c3393c7" },
    { "qing_qing_1eb1a9_0x1eb1a9dc288ce4cf75b6cd8e4525ad3f4f763f43", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x1eb1a9dc288ce4cf75b6cd8e4525ad3f4f763f43" },
    { "qing_qing_ae5845_0xae584548961b80adc3b3eccbe4fa37031725bf80", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xae584548961b80adc3b3eccbe4fa37031725bf80" },
    { "qing_qing_b97fab_0xb97fabd3588284f2848f804650092c0a47bdfec2", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xb97fabd3588284f2848f804650092c0a47bdfec2" },
    { "qing_qing_9a18d0_0x9a18d0ee9332e3357b99764936057265ccdf5fb1", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x9a18d0ee9332e3357b99764936057265ccdf5fb1" },
    { "qing_qing_9fbcf0_0x9fbcf0d8f908edac06e2922d199449a27f0904b1", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x9fbcf0d8f908edac06e2922d199449a27f0904b1" },
    { "qing_qing_8909ec_0x8909ecfce761c8f15e8ba3c81c19ae0213df58ab", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x8909ecfce761c8f15e8ba3c81c19ae0213df58ab" },
    { "qing_qing_813663_0x813663a7ed4b4b6001ae6e0ccfcf1d38cff63afa", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x813663a7ed4b4b6001ae6e0ccfcf1d38cff63afa" },
    { "qing_frock_0x8985267bbfe0e8c4da9f6eabbd3341dcd88e975d", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x8985267bbfe0e8c4da9f6eabbd3341dcd88e975d" },
    { "qing_qing_41f926_0x41f9260b49facb67eb0072cb0f75094cd94e8b80", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x41f9260b49facb67eb0072cb0f75094cd94e8b80" },
    { "qing_qing_53b1c8_0x53b1c89fe423afe3095662a253bd1b0776eb40ac", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x53b1c89fe423afe3095662a253bd1b0776eb40ac" },
    { "qing_aave_0xc5b901c1fad5ef392e8812ab134b46d3bc1acf31", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xc5b901c1fad5ef392e8812ab134b46d3bc1acf31" },
    { "qing_bal_0xccd1fed551fe2c34e0876509e323a4f430922b6d", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xccd1fed551fe2c34e0876509e323a4f430922b6d" },
    { "qing_btt_0x3ea1b5181543b8f793943e9503091d463c6b18c7", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x3ea1b5181543b8f793943e9503091d463c6b18c7" },
    { "qing_cdai_0xe0e3928c901491339bff312224d172aaafe3d6d4", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xe0e3928c901491339bff312224d172aaafe3d6d4" },
    { "qing_crv_0xea530865dbca4fa985a9881e746c3d3e84dbe745", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xea530865dbca4fa985a9881e746c3d3e84dbe745" },
    { "qing_doge_0xe83852e0574765f5d4d1d314c1c63e98a8efa2c0", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xe83852e0574765f5d4d1d314c1c63e98a8efa2c0" },
    { "qing_ldo_0x5949c6d1b226ad35b4e040a43e6d36ad4d3c99d5", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x5949c6d1b226ad35b4e040a43e6d36ad4d3c99d5" },
    { "qing_link_0xa3263c6d3f76979f7078345ae528dce928bdfd38", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xa3263c6d3f76979f7078345ae528dce928bdfd38" },
    { "qing_paxg_0x15eb61bc2ee18caacce042432abfba44f0760254", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x15eb61bc2ee18caacce042432abfba44f0760254" },
    { "qing_sand_0xa1f27c6035783540d1f5f0a9163601e6233f9573", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xa1f27c6035783540d1f5f0a9163601e6233f9573" },
    { "qing_steth_0x8975c4663045d5e92df191d5441c42b4fa613016", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x8975c4663045d5e92df191d5441c42b4fa613016" },
    { "qing_sushi_0x2fda4e4e39b2366b8505e6d5490fb0fefb7e9ab1", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x2fda4e4e39b2366b8505e6d5490fb0fefb7e9ab1" },
    { "qing_uni_0x4b3e90c711c4a6cb98bc742a9acd7f75793a02d6", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x4b3e90c711c4a6cb98bc742a9acd7f75793a02d6" },
    { "qing_usdp_0x06a7935f7e9dee9340948762a86b7a89bf634dd0", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x06a7935f7e9dee9340948762a86b7a89bf634dd0" },
    { "qing_wsteth_0x12ede01a1bb388ff5be75c0c545fba123ffe8ad3", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x12ede01a1bb388ff5be75c0c545fba123ffe8ad3" },
    { "qing_qing_f43d21_0xf43d21f1efd219b68fad48005e63573ec8103bcc", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xf43d21f1efd219b68fad48005e63573ec8103bcc" },
    { "qing_cst_0xb742345b9d59cb529145e9788110ddc1107469ed", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xb742345b9d59cb529145e9788110ddc1107469ed" },
    { "qing_irc_0x5de6076d5d5ea616f0d03a7f3bbce0ea324dd69e", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x5de6076d5d5ea616f0d03a7f3bbce0ea324dd69e" },
    { "qing_bail_0x67a8275cc8ca1ea4965b63e4c2f24dc5a0c882d4", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x67a8275cc8ca1ea4965b63e4c2f24dc5a0c882d4" },
    { "qing_scrollz_0xfebab3fb779b7384eae9f76e48118660f34f0a7b", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xfebab3fb779b7384eae9f76e48118660f34f0a7b" },
    { "qing_qing_93a615_0x93a615cef158f5bb04c29080c87f020ca1a0f3f9", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x93a615cef158f5bb04c29080c87f020ca1a0f3f9" },
    { "qing_qing_ae97e1_0xae97e10b24d855c998d71beeeabae27b61e953b4", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xae97e10b24d855c998d71beeeabae27b61e953b4" },
    { "qing_qing_f89912_0xf8991225bb81388897f643ddc73659c77aa88f1e", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xf8991225bb81388897f643ddc73659c77aa88f1e" },
    { "qing_qing_1f483b_0x1f483bf3b2c6023965803aaf7de6f91b9f6a361d", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x1f483bf3b2c6023965803aaf7de6f91b9f6a361d" },
    { "qing_dfm_0x1c46b59ad3cf2fe51329cedfc026ebc284caa68c", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x1c46b59ad3cf2fe51329cedfc026ebc284caa68c" },
    { "qing_parade_0xc18043852397719061214badc23d8e00971dc59e", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xc18043852397719061214badc23d8e00971dc59e" },
    { "qing_tlrz_0xd1aec2fd25a2ac396eb4e0c31e06bae391de6664", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xd1aec2fd25a2ac396eb4e0c31e06bae391de6664" },
    { "qing_qing_2682c3_0x2682c397c5d1a847ce1ed1684b465a6f111cf921", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x2682c397c5d1a847ce1ed1684b465a6f111cf921" },
    { "qing_ssa_0x9689279e40eb743d48bfe93ffed9e1035537b1ca", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x9689279e40eb743d48bfe93ffed9e1035537b1ca" },
    { "qing_scoiety_0x849d46776952e67652c0343193f765f78ba38c6b", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x849d46776952e67652c0343193f765f78ba38c6b" },
    { "qing_qing_bb751e_0xbb751e0e54000a95105611b585a455ecc946838c", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xbb751e0e54000a95105611b585a455ecc946838c" },
    { "qing_tehater_0x69d483c279d94e277302223d5be23e30d2a12c87", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x69d483c279d94e277302223d5be23e30d2a12c87" },
    { "qing_qing_90bc7f_0x90bc7fa7e6003868c2f462e0a00810b9292a0d73", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x90bc7fa7e6003868c2f462e0a00810b9292a0d73" },
    { "qing_ozzy_0xb9d9498242246006faa556dbdd52ec56199bf19b", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xb9d9498242246006faa556dbdd52ec56199bf19b" },
    { "qing_parsimony_0x58f9619937015d753a891652bd6d61f1d9e11470", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x58f9619937015d753a891652bd6d61f1d9e11470" },
    { "qing_ardz_0x84ce9c99363dda0e6195d9208351e987f09733d0", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x84ce9c99363dda0e6195d9208351e987f09733d0" },
    { "qing_metal_0xd2bc1720f80f2c0fc62489c239d1c716a74681e9", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xd2bc1720f80f2c0fc62489c239d1c716a74681e9" },
    { "qing_quit_0xbf07341aabdd2695d2deb595d06a415de7f606fe", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xbf07341aabdd2695d2deb595d06a415de7f606fe" },
    { "qing_efficient_0x66ba4c28beb62f2fe9caf7e9bb7751fccf334e2b", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x66ba4c28beb62f2fe9caf7e9bb7751fccf334e2b" },
    { "qing_electricgokart_0xfd6eb20487d90bfd3001679be521e73ef28e0abd", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xfd6eb20487d90bfd3001679be521e73ef28e0abd" },
    { "qing_embassy_0x0af47b8ff2dad920efc1e8fe02d8511820762aba", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x0af47b8ff2dad920efc1e8fe02d8511820762aba" },
    { "qing_clementine_0xee2d8b94c47242e96c91909770e14cb8fcf4e11c", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xee2d8b94c47242e96c91909770e14cb8fcf4e11c" },
    { "qing_memez_0xf3916fc739348138c3a47f8829c7e3edf8f96801", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xf3916fc739348138c3a47f8829c7e3edf8f96801" },
    { "qing_safety_0xdc832d80173b5e15c6baa55415cf5b551002adf0", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xdc832d80173b5e15c6baa55415cf5b551002adf0" },
    { "qing_whoopi_0xbdf654404196d209c7524ab59e639a2522665741", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xbdf654404196d209c7524ab59e639a2522665741" },
    { "qing_spam_0x2b5b22f88ea0108f8c2472b95cc27127f4b48387", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x2b5b22f88ea0108f8c2472b95cc27127f4b48387" },
    { "qing_act_0xa1b5fb4604210cd55e5f82f50b4f46f0e7b3b051", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xa1b5fb4604210cd55e5f82f50b4f46f0e7b3b051" },
    { "qing_counterfit_0x4b98be930a6509944c2747bde14f8774d811bde0", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x4b98be930a6509944c2747bde14f8774d811bde0" },
    { "qing_national_0x7c6e6e9522bb9308a638b93802cca6943828c5f0", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x7c6e6e9522bb9308a638b93802cca6943828c5f0" },
    { "qing_svbpc_0xee5aadc0ef3e7265a63859ba4e24263e6bb3ed68", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xee5aadc0ef3e7265a63859ba4e24263e6bb3ed68" },
    { "qing_ronin_0x53294c8572be0f965ecbcd7e7a3a90c38d523c2e", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x53294c8572be0f965ecbcd7e7a3a90c38d523c2e" },
    { "qing_pulpit_0x1c9a2a138e779cd34bd1ff832cbb4426d2bfe0cc", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x1c9a2a138e779cd34bd1ff832cbb4426d2bfe0cc" },
    { "qing_dismissed_0x547ee112a01b5c21e92c8cf753e13e9f64d87afe", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x547ee112a01b5c21e92c8cf753e13e9f64d87afe" },
    { "qing_rfc_0x4567da9c07b68516f033476a333860a82a2897b8", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x4567da9c07b68516f033476a333860a82a2897b8" },
    { "qing_qing_ce93b1_0xce93b125dfbbf8b5be4e5fe80795afa40135943a", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xce93b125dfbbf8b5be4e5fe80795afa40135943a" },
    { "qing_regal_0x28c691728e06e7671e7d3f0ec906d2a2fdd1936e", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x28c691728e06e7671e7d3f0ec906d2a2fdd1936e" },
    { "qing_puffybff_0x2d013f27781672705869a1695f53f99a4d800b66", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x2d013f27781672705869a1695f53f99a4d800b66" },
    { "qing_hmb_0xa6786ee02625292fe49615e50cc537c3cd9d4076", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xa6786ee02625292fe49615e50cc537c3cd9d4076" },
    { "qing_sotdpr_0xeed3eb66e91507e34580971705a23ddcee980ba9", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xeed3eb66e91507e34580971705a23ddcee980ba9" },
    { "qing_mf_0x4a38a1767a48f3ca02c9336cfcb844701b80f268", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x4a38a1767a48f3ca02c9336cfcb844701b80f268" },
    { "qing_bf_0xa267e90a2a7347d8d751cd6409595988ef0c4696", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xa267e90a2a7347d8d751cd6409595988ef0c4696" },
    { "qing_anmesty_0x02cca891e5959e8f11c64cf88356450154258524", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x02cca891e5959e8f11c64cf88356450154258524" },
    { "qing_tigard_0xa2aeb94ce78fbc0041a47051456844ebc2e22d08", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xa2aeb94ce78fbc0041a47051456844ebc2e22d08" },
    { "qing_qing_63c385_0x63c38523ae7ef5a1bd791b248a87de1c47977606", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x63c38523ae7ef5a1bd791b248a87de1c47977606" },
    { "qing_qing_e7ac4d_0xe7ac4d11de373ac57e24c912992292643e53a02e", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xe7ac4d11de373ac57e24c912992292643e53a02e" },
    { "qing_dork_0xae204a68f7fb18796dd5c294548f05bc832853e0", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xae204a68f7fb18796dd5c294548f05bc832853e0" },
    { "qing_tehcode_0x57a925aedab2484b1b060ed37c5ece61fa0a06df", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x57a925aedab2484b1b060ed37c5ece61fa0a06df" },
    { "qing_test_0x895b912698cf8dd3b7a19635cd7fa850772cee94", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x895b912698cf8dd3b7a19635cd7fa850772cee94" },
    { "qing_qing_299c56_0x299c56d1e1efa1d662823faa7fe7ccda6a4f35eb", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x299c56d1e1efa1d662823faa7fe7ccda6a4f35eb" },
    { "qing_hastey_0x53fd70a6fb15ee55033ee8b6e4917ff39bcddb54", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x53fd70a6fb15ee55033ee8b6e4917ff39bcddb54" },
    { "qing_y_0x55fd3be287ec145c791539ff1257b7536d8a390e", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x55fd3be287ec145c791539ff1257b7536d8a390e" },
    { "qing_qing_f0f5e1_0xf0f5e1144dc80eae45a52c041411a830f5b07b1f", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xf0f5e1144dc80eae45a52c041411a830f5b07b1f" },
    { "qing_qing_363a38_0x363a383b4b2b687883d5f95450f2f837140b2a6b", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x363a383b4b2b687883d5f95450f2f837140b2a6b" },
    { "qing_qing_84b634_0x84b634ff2d9e47e819fde10998bd30a6bf2b2ed6", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x84b634ff2d9e47e819fde10998bd30a6bf2b2ed6" },
    { "qing_qing_cbdef1_0xcbdef17697e0145075574152f65aafb6a0c2785d", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xcbdef17697e0145075574152f65aafb6a0c2785d" },
    { "qing_fig_0x0a1b27f86655a93f00e84707ba217e63788d8c6c", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x0a1b27f86655a93f00e84707ba217e63788d8c6c" },
    { "qing_qing_0f1853_0x0f1853ee374c0c9c52172cea7269162d26508e88", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x0f1853ee374c0c9c52172cea7269162d26508e88" },
    { "qing_qing_c1729d_0xc1729d35eb6a3bb3cca4f902a3b3568deec4ac29", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xc1729d35eb6a3bb3cca4f902a3b3568deec4ac29" },
    { "qing_qing_a8434a_0xa8434a7e4af760757b7c50a0a8efda52e4248dbb", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xa8434a7e4af760757b7c50a0a8efda52e4248dbb" },
    { "qing_plp_0xa3bfdb59e9d4167f56b47424531ddb31992afc57", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xa3bfdb59e9d4167f56b47424531ddb31992afc57" },
    { "qing_ruj_0xd38c924e478cba40bb192f500789f0623a45e87f", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xd38c924e478cba40bb192f500789f0623a45e87f" },
    { "qing_77_0x408966184a1be877dbaab63f5dd59ec85afc2840", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x408966184a1be877dbaab63f5dd59ec85afc2840" },
    { "qing_qing_1769dc_0x1769dc19ce382f7f0b50578786b3fe9bd0603de9", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x1769dc19ce382f7f0b50578786b3fe9bd0603de9" },
    { "qing_qing_57a7d9_0x57a7d955ba5df1d291bd4887ae05c3eb3510ee74", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x57a7d955ba5df1d291bd4887ae05c3eb3510ee74" },
    { "qing_durekverrett_0xdf1af19695098758dab1f307176991664e94ea67", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xdf1af19695098758dab1f307176991664e94ea67" },
    { "qing_sand_0xa8b91a415b248bb042497196666a6b26fa757e9e", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xa8b91a415b248bb042497196666a6b26fa757e9e" },
    { "qing_qing_0c9ebc_0x0c9ebce7d4e20ee00bcd003cfa6c155c23eda253", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x0c9ebce7d4e20ee00bcd003cfa6c155c23eda253" },
    { "qing_bell_0x870bce7fa25de214841fe37a2cd11f78334ce9c8", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x870bce7fa25de214841fe37a2cd11f78334ce9c8" },
    { "qing_plutos_0xb5a778ecdaad74c2e27dd2edbeebd6daa98ff9a7", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xb5a778ecdaad74c2e27dd2edbeebd6daa98ff9a7" },
    { "qing_qing_b7ecdb_0xb7ecdb308a92cf9837e677cc07da51b649a19fd6", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xb7ecdb308a92cf9837e677cc07da51b649a19fd6" },
    { "qing_hecke_0x46fd470d0aded2bb93e913843797d99855f90d8d", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x46fd470d0aded2bb93e913843797d99855f90d8d" },
    { "qing_glob_0xa5add4f8d0c60fd82e779e1c0505caef21df3452", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xa5add4f8d0c60fd82e779e1c0505caef21df3452" },
    { "qing_msa_0x8eeb2ed87cae753d3589c56cd8bc5788c62cd919", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x8eeb2ed87cae753d3589c56cd8bc5788c62cd919" },
    { "qing_qing_c4a299_0xc4a2994e5c1ccc3db0b5a8a30f2ba349aba8f037", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xc4a2994e5c1ccc3db0b5a8a30f2ba349aba8f037" },
    { "qing_qing_efacd8_0xefacd8ccb0f39a5e6219b902cd81b85f984d19ca", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xefacd8ccb0f39a5e6219b902cd81b85f984d19ca" },
    { "qing_tdolla_0x46d7db3f3e88ebb25b4b23cbc1ebba21568d973d", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x46d7db3f3e88ebb25b4b23cbc1ebba21568d973d" },
    { "qing_gibs_0x1b8774c0d0ba2a814a592be7978dfe78b0e86e35", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x1b8774c0d0ba2a814a592be7978dfe78b0e86e35" },
    { "qing_qing_c0e7db_0xc0e7db8219d27fc77a30689017fa0f557b05e023", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xc0e7db8219d27fc77a30689017fa0f557b05e023" },
    { "qing_shio_0xb6b83367fa8bd49e369b70615030f28a095fe0d8", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xb6b83367fa8bd49e369b70615030f28a095fe0d8" },
    { "qing_shio_0xadf5bec5c4b8469bd9fbdd4e51077736d857a660", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xadf5bec5c4b8469bd9fbdd4e51077736d857a660" },
    { "qing_meta_0xc70a1047e7b9ac9f0ad74000dab1ab471b49c154", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xc70a1047e7b9ac9f0ad74000dab1ab471b49c154" },
    { "qing_pang_0x1213094216a8726bb8d27a1a8d50078c5234a4f3", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x1213094216a8726bb8d27a1a8d50078c5234a4f3" },
    { "qing_void_0xeccdfd5fa13b40ec76b9c45a5d20463b47de6c9c", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xeccdfd5fa13b40ec76b9c45a5d20463b47de6c9c" },
    { "qing_choa_0xf8f8af1f48152e1f0c93f38fa783783530ffff40", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xf8f8af1f48152e1f0c93f38fa783783530ffff40" },
    { "qing_yi_0x170259c2640e221772b94c8037b24da766b3bcfc", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x170259c2640e221772b94c8037b24da766b3bcfc" },
    { "qing_cheon_0x7206172b419354dfd25fb9ae732f936c2c5c01fb", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x7206172b419354dfd25fb9ae732f936c2c5c01fb" },
    { "qing_yau_0x06b2bb1d1ffcd92b70462b32e5878a0b88ee9897", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x06b2bb1d1ffcd92b70462b32e5878a0b88ee9897" },
    { "qing_yang_0x6c49021d0aeac563c0a16de1c3f70ae5f87ca768", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x6c49021d0aeac563c0a16de1c3f70ae5f87ca768" },
    { "qing_ring_0xc9083d2a55536ac0c96b5235d585a4d5638b90f3", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xc9083d2a55536ac0c96b5235d585a4d5638b90f3" },
    { "qing_qing_d4e52f_0xd4e52fed9d967b093bb36c91f5aa07b75b31e047", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xd4e52fed9d967b093bb36c91f5aa07b75b31e047" },
    { "qing_had_0x6bfd5a5416c30ec99be17d4f7426542b9eb32bba", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x6bfd5a5416c30ec99be17d4f7426542b9eb32bba" },
    { "qing_phnx_0x45c569abee5b95b2564cca9c55dbe6e1d5e5de93", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x45c569abee5b95b2564cca9c55dbe6e1d5e5de93" },
    { "qing_delta_0x60231492ea7a40f72119da9e6c8bf738961679f3", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x60231492ea7a40f72119da9e6c8bf738961679f3" },
    { "qing_qing_8f591e_0x8f591ef073d9ffa8b4d1c592f4ff3011151d6f12", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x8f591ef073d9ffa8b4d1c592f4ff3011151d6f12" },
    { "qing_scftus_0xf9249a4de17b50257b5462682853047661c2e77e", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xf9249a4de17b50257b5462682853047661c2e77e" },
    { "qing_iau_0x54d82a58519de030a778673e106be53817b96aaa", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x54d82a58519de030a778673e106be53817b96aaa" },
    { "qing_telover_0x35a5adc16adb50aaa74dec369a500715567dd22e", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x35a5adc16adb50aaa74dec369a500715567dd22e" },
    { "qing_qing_5d0d83_0x5d0d839b31c5243519392b615ac1b56a5ac0e4b7", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x5d0d839b31c5243519392b615ac1b56a5ac0e4b7" },
    { "qing_jy_0x396f37a0f109567c7c34f51710f7ab798aa4b5bb", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x396f37a0f109567c7c34f51710f7ab798aa4b5bb" },
    { "qing_drll_0x50089a1813d07d9df278976d2f4af7f034d04b44", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x50089a1813d07d9df278976d2f4af7f034d04b44" },
    { "qing_cihm_0x2be03ca2ff702ebf3a3d44ff8b5e0013bffaf080", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x2be03ca2ff702ebf3a3d44ff8b5e0013bffaf080" },
    { "qing_mn_0x8ec2785d749dd2866ccf000edaf2fde4958b5387", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x8ec2785d749dd2866ccf000edaf2fde4958b5387" },
    { "qing_clrkz_0x8704f58e0066b8b375cc43cce8473419668085ca", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x8704f58e0066b8b375cc43cce8473419668085ca" },
    { "qing_sants_0x77744b0a0b09c2860e709cff729e2891b7fa95ef", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x77744b0a0b09c2860e709cff729e2891b7fa95ef" },
    { "qing_df_0xed7a46819427de6cc83ceeab63a150b45f88de1c", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xed7a46819427de6cc83ceeab63a150b45f88de1c" },
    { "qing_fedex_0xc50ce07770d9715f23ade9ff9bb88d9c328ce9a8", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xc50ce07770d9715f23ade9ff9bb88d9c328ce9a8" },
    { "qing_atlas_0x77c14dd7ab31a4c223deb8ef182d372a7cfae5c7", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x77c14dd7ab31a4c223deb8ef182d372a7cfae5c7" },
    { "qing_napalm_0x93a517de9a47abe235c86c5bb6955c54985a1b87", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x93a517de9a47abe235c86c5bb6955c54985a1b87" },
    { "qing_physos_0xe21b843ca882095ee091a4a8b45b74a03928990a", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xe21b843ca882095ee091a4a8b45b74a03928990a" },
    { "qing_hbk_0xbc63ccd32c1b9515653144ff0bf31382a162466a", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xbc63ccd32c1b9515653144ff0bf31382a162466a" },
    { "qing_aquax_0x986b85c00d2db24676e73515c5a8cb5fee0f4b0d", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x986b85c00d2db24676e73515c5a8cb5fee0f4b0d" },
    { "qing_bloomx_0xc90e4cc1213ce5050805ff66494b050569241003", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xc90e4cc1213ce5050805ff66494b050569241003" },
    { "qing_emc2_0x69420aed2adcb8d43f4c0716e41a8d06c62555f1", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x69420aed2adcb8d43f4c0716e41a8d06c62555f1" },
    { "qing_lght_0x5d781b46292195781229322d538bd25d644ad653", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x5d781b46292195781229322d538bd25d644ad653" },
    { "qing_nrgy_0xe57209022c3c96ec9918af9e5127195f1d50738c", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xe57209022c3c96ec9918af9e5127195f1d50738c" },
    { "qing_mss_0xa2ed574f07e5bba9e97eafccdb4c50b3e21bde16", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xa2ed574f07e5bba9e97eafccdb4c50b3e21bde16" },
    { "qing_prvx_0x99bde85f89fbcb76dbfc9666d79e1cd530935d67", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x99bde85f89fbcb76dbfc9666d79e1cd530935d67" },
    { "qing_ccne_0x39fcd5e0f1146717c0f45b8837632314ad5b73d7", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x39fcd5e0f1146717c0f45b8837632314ad5b73d7" },
    { "qing_prismo_0xdd208d9037c41041c91211978cfb1ba5ff0a1ffc", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xdd208d9037c41041c91211978cfb1ba5ff0a1ffc" },
    { "qing_nous_0x363c28f698b6507171b02a90e5a2b43f832eb8cd", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x363c28f698b6507171b02a90e5a2b43f832eb8cd" },
    { "qing_man432_0xf71423b0e4e029604af70db45f7c78890081d74a", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xf71423b0e4e029604af70db45f7c78890081d74a" },
    { "qing_qing_2b78b3_0x2b78b345880807ac9be28c1309ab55d6fa8796de", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x2b78b345880807ac9be28c1309ab55d6fa8796de" },
    { "qing_qing_72dbc8_0x72dbc884541980e36012becb054065a57f3f0e43", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x72dbc884541980e36012becb054065a57f3f0e43" },
    { "qing_mp_0x9c4379235ebf3989c8a90e567cc8019a2010a461", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x9c4379235ebf3989c8a90e567cc8019a2010a461" },
    { "qing_cfs_0x35e2765d39b3c89b186b282ddd27abbb4d3e4805", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x35e2765d39b3c89b186b282ddd27abbb4d3e4805" },
    { "qing_jinks_0xd836526feebeb7bd23e095a551a84d873830b35a", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xd836526feebeb7bd23e095a551a84d873830b35a" },
    { "qing_qing_553c41_0x553c41d0ef132c4e3ad993b6d74d871e2238cbb4", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x553c41d0ef132c4e3ad993b6d74d871e2238cbb4" },
    { "qing_mp_0x94ecc8e13e87e981bd69d35b57a738790690fd4c", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x94ecc8e13e87e981bd69d35b57a738790690fd4c" },
    { "qing_qing_081183_0x081183aa299463a4e75e37dc629688f0e7bff470", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x081183aa299463a4e75e37dc629688f0e7bff470" },
    { "qing_qing_f02f06_0xf02f060c797fa9ebf6a72cdc3e072b3ee2c003c6", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xf02f060c797fa9ebf6a72cdc3e072b3ee2c003c6" },
    { "qing_qing_f86fb6_0xf86fb6c69f4d1386976fb22d87e442448163cfcc", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xf86fb6c69f4d1386976fb22d87e442448163cfcc" },
    { "qing_mc_0xb5c3370f3a8db25a3c30df7c409afea0d5dec93c", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xb5c3370f3a8db25a3c30df7c409afea0d5dec93c" },
    { "qing_qing_ba82a0_0xba82a0119c32e8f86476b6077c72fb2699da75f0", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xba82a0119c32e8f86476b6077c72fb2699da75f0" },
    { "qing_qing_7b10d6_0x7b10d6133b0a66a49a0f1c0902e9693633b9d294", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x7b10d6133b0a66a49a0f1c0902e9693633b9d294" },
    { "qing_qing_81f108_0x81f1088ed450655bddfab444cdcff24d817cc897", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x81f1088ed450655bddfab444cdcff24d817cc897" },
    { "qing_qing_b99839_0xb998396be6bf0324c46c6b489390a411d7852b40", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xb998396be6bf0324c46c6b489390a411d7852b40" },
    { "qing_qing_a7b2c9_0xa7b2c902a9139e75738889f8f53cb0f5df688816", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xa7b2c902a9139e75738889f8f53cb0f5df688816" },
    { "qing_qing_aee268_0xaee268a215e81dd3a4ff965978dec6106859ea38", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xaee268a215e81dd3a4ff965978dec6106859ea38" },
    { "qing_qing_edc0ef_0xedc0ef784e50671debe3c31ae01c19ac4170fb1b", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xedc0ef784e50671debe3c31ae01c19ac4170fb1b" },
    { "qing_qing_a6268e_0xa6268ea1256f530a759769802bf7153ba46011fa", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xa6268ea1256f530a759769802bf7153ba46011fa" },
    { "qing_qing_d09e21_0xd09e21d3b66354687b562dbe7e13977f08644d85", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xd09e21d3b66354687b562dbe7e13977f08644d85" },
    { "qing_cfs_0xec665d480ad538df65457fdd37cd30a8ed1331ce", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xec665d480ad538df65457fdd37cd30a8ed1331ce" },
    { "qing_qing_d0eabe_0xd0eabe17031f975fbad155ae435d0708ed8572be", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xd0eabe17031f975fbad155ae435d0708ed8572be" },
    { "qing_cashx_0xc6992845d889aea0e7630880bcca3f725a2d6ca0", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xc6992845d889aea0e7630880bcca3f725a2d6ca0" },
    { "qing_qing_30a3_0x30a315d4a69f1077b423e5b73e262e1d9da604df", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x30a315d4a69f1077b423e5b73e262e1d9da604df" },
    { "qing_eedn_0xeb83f838fd42b496409f996b2895e2ca415b3106", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xeb83f838fd42b496409f996b2895e2ca415b3106" },
    { "qing_ruarc_0x992029c95d913ba8b4ac831b89932e2a628d94a6", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x992029c95d913ba8b4ac831b89932e2a628d94a6" },
    { "qing_who_0x64979bca9251b2bafa5e3d62967c8e3057f68c0d", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x64979bca9251b2bafa5e3d62967c8e3057f68c0d" },
    { "qing_sirin_0x651fcbae1fba264ce7aef0b9646c73b14a98bf3e", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x651fcbae1fba264ce7aef0b9646c73b14a98bf3e" },
    { "qing_ichidai_0xfbd0aa20acc2335bf29f6bf7f07881dc5300a391", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xfbd0aa20acc2335bf29f6bf7f07881dc5300a391" },
    { "qing_daiichi_0xe6f1658841b688b1f777b9e15eaebc19527b940e", "../solidity/dysnomia/domain/dan/03_qing.sol", "0xe6f1658841b688b1f777b9e15eaebc19527b940e" },
    { "qing_qing_80d6c7_0x80d6c7e47d9cdfae23fdc9869778873f1ea2dbcf", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x80d6c7e47d9cdfae23fdc9869778873f1ea2dbcf" },
    { "qing_qing_7392a8_0x7392a8c55f93749b4b4182da70e79141041c31e0", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x7392a8c55f93749b4b4182da70e79141041c31e0" },
    { "qing_qing_04add0_0x04add092a0315cfb66ef2dbffa603d5fe55fc1d8", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x04add092a0315cfb66ef2dbffa603d5fe55fc1d8" },
    { "qing_qing_50da29_0x50da29cffe5a124ee05287b4af95c80b417a1607", "../solidity/dysnomia/domain/dan/03_qing.sol", "0x50da29cffe5a124ee05287b4af95c80b417a1607" },
    { "qi",           "../solidity/dysnomia/domain/soeng/01_qi.sol",     "0x91F3ca10050862eccdbb7b265cc6fB49EeDD802C" },
    { "mai",          "../solidity/dysnomia/domain/soeng/02_mai.sol",     "0x2cc7e492a98cb211287A1506A55245dBa200E52a" },
    { "xia",          "../solidity/dysnomia/domain/soeng/03_xia.sol",     "0x6f7D105670842d39ACc32B5AeF0C5238AA0B9F3D" },
    { "xie",          "../solidity/dysnomia/domain/soeng/04_xie.sol",     "0xfa242E3A4FBAa0773559560B49bDeA35C975cBaf" },
    { "chan",         "../solidity/dysnomia/domain/sky/01_chan.sol",     "0xCb39Cc1afd2CEe8E8C82F9A330aBF37F4933d850" },
    { "sei",          "../solidity/dysnomia/domain/tang/01_sei.sol",     "0x8f4798DfDA3BC507954d768Bc33f3c456A407F30" },
    { "choa",         "../solidity/dysnomia/domain/sky/02_choa.sol",     "0xd8949704cc5acAf1600D908a2607AbeB668fAc6b" },
    { "cheon",        "../solidity/dysnomia/domain/tang/02_cheon.sol",    "0xcF54035Be7F6eC2664FEeaAcfbA4470BbB23E0A0" },
    { "cabsMarketMachine", "../solidity/dysnomia/domain/sky/CABSMarketMachine.sol", "0x74ef2B06A1D2035C33244A4a263FF00B84504865" },
    { "zmachine",       "../solidity/bin/zmachine.yul",             "5" },
    { "zmachineParser", "../solidity/bin/zmachineParser.yul",       "6" }
};

int main() {
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stdin, NULL, _IONBF, 0);
    srand((unsigned int)time(NULL) ^ (unsigned int)getpid());
    
    extern void tsfi_wire_firmware_init(void);
    tsfi_wire_firmware_init();

    TsfiZmmVmState state;
    tsfi_zmm_vm_init(&state);

    const char *id = getenv("TSFI_TELEMETRY_ID");
    if (id) {
        tsfi_zmm_vm_attach_telemetry(&state, id);
        tsfi_io_printf(stderr, "[MCP] Attached to Telemetry ID: %s\n", id);
    }

    tsfi_io_printf(stderr, "[ZMM_VM] Autoloading Dysnomia contracts...\n");
    int count = sizeof(g_dysnomia_system) / sizeof(g_dysnomia_system[0]);
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
            for (int r = 0; r < 5; r++) {
                tsfi_zmm_vm_exec(&state, "YULEXEC \"vmreq\", \"604a6fa9\"");
            }
        }
        snprintf(cmd, sizeof(cmd), "YULINIT \"%s\", \"%s\", %lu", 
                 g_dysnomia_system[i].name, 
                 g_dysnomia_system[i].path, 
                 addr64);
        tsfi_zmm_vm_exec(&state, cmd);
    }
    
    // Create default User LAU token on startup:
    const char *calldata_new = "8c6dc5ec00000000000000000000000000000000000000000000000000000000000000400000000000000000000000000000000000000000000000000000000000000080000000000000000000000000000000000000000000000000000000000000000c5573657220546f6b656e20310000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000a55534552544f4b454e3100000000000000000000000000000000000000000000";
    char exec_cmd[1024];
    snprintf(exec_cmd, sizeof(exec_cmd), "YULEXEC \"laufactory\", \"%s\"", calldata_new);
    tsfi_io_printf(stderr, "[ZMM_VM] Creating default LAU token: %s\n", exec_cmd);
    tsfi_zmm_vm_exec(&state, exec_cmd);
    
    // Bind zmachineParser (6) to zmachine (5) to enable parseCommand thunking
    tsfi_io_printf(stderr, "[ZMM_VM] Binding zmachineParser to zmachine...\n");
    tsfi_zmm_vm_exec(&state, "YULEXEC \"zmachine\", \"7e1ef7e90000000000000000000000000000000000000000000000000000000000000006\"");

    // Rehydrate Quadtree ledger storage states from node ledger files
    extern void blue_box_rehydrate_quadtree_states(void);
    blue_box_rehydrate_quadtree_states();

    size_t cap = 1024 * 1024 * 4; 
    char *input = (char*)lau_malloc(cap);
    char *output = (char*)lau_malloc(cap);
    if (!input || !output) return 1;

    // --- Initialize Network Bridge (Google Flow Ingest) ---
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd >= 0) {
        int opt = 1;
        setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        fcntl(server_fd, F_SETFL, O_NONBLOCK);
        
        struct sockaddr_in address;
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        int port = 10042;
        const char *p_env = getenv("TSFI_MCP_PORT");
        if (p_env) port = atoi(p_env);
        address.sin_port = htons(port);
        
        if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
            tsfi_io_printf(stderr, "[MCP] Network Bind Failed on %d\n", port);
            close(server_fd);
            server_fd = -1;
        } else {
            listen(server_fd, 3);
            tsfi_io_printf(stderr, "[MCP] Google Flow Bridge Listening on Port %d\n", port);
        }
    }

    struct pollfd fds[2];
    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;
    fds[1].fd = server_fd;
    fds[1].events = (server_fd >= 0) ? POLLIN : 0;

    // Stream Loop
    while (1) {
        // 1. Check SHM Command Channel
        if (state.telem && state.telem->request_cmd[0] == '{') {
            char cmd[4096];
            snprintf(cmd, sizeof(cmd), "%.4095s", (char*)state.telem->request_cmd);
            state.telem->request_cmd[0] = 0; // Clear it

            if (tsfi_zmm_rpc_dispatch(&state, cmd, output, cap)) {
                // Write response to SHM for Cockpit to see
                snprintf((char*)state.telem->zmm_msg, sizeof(state.telem->zmm_msg), "%s", output);
                // Also log to stdout for debugging
                tsfi_io_printf(stderr, "[MCP SHM] %s", output);
            }
        }

        // 2. Check Input Sources (Non-blocking poll)
        if (poll(fds, 2, 10) > 0) {
            // Check STDIN
            if (fds[0].revents & POLLIN) {
                if (fgets(input, cap, stdin)) {
                    if (strstr(input, "\"method\": \"exit\"") || strstr(input, "\"method\":\"exit\"")) {
                        break;
                    }
                    if (tsfi_zmm_rpc_dispatch(&state, input, output, cap)) {
                        tsfi_io_printf(stdout, "%s", output);
                    }
                }
            }
            
            // Check Network (Accept and handle one request at a time)
            if (server_fd >= 0 && (fds[1].revents & POLLIN)) {
                struct sockaddr_in client_addr;
                socklen_t addrlen = sizeof(client_addr);
                int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addrlen);
                if (client_fd >= 0) {
                    static time_t g_lut = 0;
                    static int g_urc = 0;
                    time_t now = time(NULL);
                    if (now - g_lut >= 3600) { g_urc = 0; }
                    if (g_urc >= 1000) {
                        const char *err_msg = "{\"jsonrpc\":\"2.0\",\"error\":{\"code\":-32001,\"message\":\"RATE_LIMIT_EXCEEDED_MAX_1_PER_HOUR\"}}";
                        ssize_t nw = write(client_fd, err_msg, strlen(err_msg));
                        (void)nw;
                        close(client_fd);
                        continue;
                    }
                    g_lut = now;
                    g_urc++;

                    ssize_t n = read(client_fd, input, cap - 1);
                    if (n > 0) {
                        input[n] = 0;
                        if (tsfi_zmm_rpc_dispatch(&state, input, output, cap)) {
                            ssize_t nw = write(client_fd, output, strlen(output));
                            if (nw < 0) perror("write failed");
                        }
                    }
                    close(client_fd);
                }
            }
        }
        
        tsfi_zmm_rpc_step_async_llm(&state);
        tsfi_raw_usleep(1000); // Prevent 100% CPU
    }
    
    lau_free(input);
    lau_free(output);
    
    // Decompose sovereign execution geometrieZ spawned by VM execution
    extern void tsfi_wire_firmware_teardown(void);
    extern void tsfi_dysnomia_drain_pools(void);
    extern void tsfi_dl_thunks_teardown(void);
    tsfi_wire_firmware_teardown();
    tsfi_dysnomia_drain_pools();
    tsfi_dl_thunks_teardown();
    
    tsfi_zmm_vm_destroy(&state);
    
    // Explicitly enforce geometric bounds verification
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    
    return 0;
}
