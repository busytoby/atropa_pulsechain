using System;
using System.Threading.Tasks;
using System.Collections.Generic;
using System.Numerics;
using Nethereum.Hex.HexTypes;
using Nethereum.ABI.FunctionEncoding.Attributes;
using Nethereum.RPC.Eth.DTOs;
using Nethereum.Contracts.CQS;
using Nethereum.Contracts;
using System.Threading;

namespace Dysnomia.Contracts.lib.Hecke.ContractDefinition
{


    public partial class HeckeDeployment : HeckeDeploymentBase
    {
        public HeckeDeployment() : base(BYTECODE) { }
        public HeckeDeployment(string byteCode) : base(byteCode) { }
    }

    public class HeckeDeploymentBase : ContractDeploymentMessage
    {
        public static string BYTECODE = "610bc06040526601b196475ad79b6080908152660b91e58e0ed79b60a05267020341b0a3ead79b60c052676852f6f1149ad79b60e05268153083c807f85ad79b6101005269044dda6e40ae3b5ad79b6101205269dfd05e0ec273d75ad79b610140526a2d76531aa91e97875ad79b610160526b093c08e16a03d5d7475ad79b610180526c01e031cdc988710eca475ad79b6101a0526c618a1dccefb6a0a026475ad79b6101c0526d13d00e0da0b1184a26d6475ad79b6101e0526e040642dac4a3f8eeb98296475ad79b610200526ed145946ff14e907d582596475ad79b610220526f2a822226bd03f55975914196475ad79b610240527008a26eefde64cdd62be12af196475ad79b610260527101c0fe88b92c79cf80e9bc62b196475ad79b61028052715b33b3c59d08be262f7a43b5b196475ad79b6102a052721286808423e5c69fc1a4d5c091b196475ad79b6102c0527303c3521ad74aac5873557b6b1d41b196475ad79b6102e05273c3acad73bb2b01f76d5d11c1f101b196475ad79b610300527427bf133b8204bc664236e79b64f401b196475ad79b61032052750812cfe81668f644c573270b90819001b196475ad79b610340527601a3d23b248d5205f81b63ee595a514001b196475ad79b61036052765546b4036cb4a93665904c6a2658810001b196475ad79b610380527711525c90b214b25f0ca14f858fc9fa340001b196475ad79b6103a052780384bacd642c343b4e90c427213506d2900001b196475ad79b6103c05278b6f5f1b858fa9c0bf567d7f2bec562c5400001b196475ad79b6103e052792529f5197212e7b26dd917dd4ec0181011000001b196475ad79b610400527a078c85c92b2bd7103e5018d8f3ff04e34374000001b196475ad79b610420527b01888b2cdcc4e7af4ca8450c118fccfe29b390000001b196475ad79b610440527b4fbc451cd7ff0f9b922e06739135a3a0787940000001b196475ad79b610460527c10323e09dbdfcf2b99b1594f797ee53c9878a100000001b196475ad79b610480527d034a349a00a97614db38062424adc6904ef880b400000001b196475ad79b6104a0527dab12af48226bfc3c87613f57734c55500a7a249000000001b196475ad79b6104c0527e04c91503d981a92bf7671abc12cd444ea88c46b3e0e69ca0b62f3bdb77350c6104e0527e08e717586ae0e65bb246d438ce273c48010e134331cd39416aace16f93927c610500527e0d0519acfc40238b6d268db589813441598fdfd282b3d5e21f2a8703afefed610520527e11231c018d9f60bb2806473244db2c3ab211ac61d39a7282d3a82c97cc4d5e610540527e15411e561efe9deae2e600af003524340a9378f124810f238825d22be8aace610560527e195f20aab05ddb1a9dc5ba2bbb8f1c2d631545807567abc43ca377c005083f610580527e1d7d22ff41bd184a58a573a876e91426bb97120fc64e4864f1211d542165b06105a0527e219b2553d31c557a13852d2532430c201418de9f1734e505a59ec2e83dc3206105c0527e25b927a8647b92a9ce64e6a1ed9d04196c9aab2e681b81a65a1c687c5a20916105e0527e29d729fcf5dacfd98944a01ea8f6fc12c51c77bdb9021e470e9a0e10767e02610600527e2df52c51873a0d094424599b6450f40c1d9e444d09e8bae7c317b3a492db72610620527e32132ea618994a38ff0413181faaec05762010dc5acf578877955938af38e3610640527e363130faa9f88768b9e3cc94db04e3fecea1dd6babb5f4292c12fecccb9654610660527e3a4f334f3b57c49874c38611965edbf82723a9fafc9c90c9e090a460e7f3c4610680527e3e6d35a3ccb701c82fa33f8e51b8d3f17fa5768a4d832d6a950e49f50451356106a0527e428b37f85e163ef7ea82f90b0d12cbead82743199e69ca0b498bef8920aea66106c0527e46a93a4cef757c27a562b287c86cc3e430a90fa8ef5066abfe09951d3d0c166106e0527e4ac73ca180d4b95760426c0483c6bbdd892adc384037034cb2873ab1596987610700527e4ee53ef61233f6871b2225813f20b3d6e1aca8c7911d9fed6704e04575c6f8610720527e5303414aa39333b6d601defdfa7aabd03a2e7556e2043c8e1b8285d9922468610740527e5721439f34f270e690e1987ab5d4a3c992b041e632ead92ed0002b6dae81d9610760527e5b3f45f3c651ae164bc151f7712e9bc2eb320e7583d175cf847dd101cadf4a610780527e5f5d484857b0eb4606a10b742c8893bc43b3db04d4b8127038fb7695e73cba6107a0527e637b4a9ce9102875c180c4f0e7e28bb59c35a794259eaf10ed791c2a039a2b6107c0527e67994cf17a6f65a57c607e6da33c83aef4b7742376854bb1a1f6c1be1ff79c6107e0527e6bb74f460bcea2d5374037ea5e967ba84d3940b2c76be852567467523c550c610800527e6fd5519a9d2de004f21ff16719f073a1a5bb0d42185284f30af20ce658b27d610820527e73f353ef2e8d1d34acffaae3d54a6b9afe3cd9d169392193bf6fb27a750fee610840527e78115643bfec5a6467df646090a4639456bea660ba1fbe3473ed580e916d5e610860527e7c2f5898514b979422bf1ddd4bfe5b8daf4072f00b065ad5286afda2adcacf610880527e804d5aece2aad4c3dd9ed75a0758538707c23f7f5becf775dce8a336ca28406108a0527e846b5d41740a11f3987e90d6c2b24b8060440c0eacd39416916648cae685b06108c0527e88895f9605694f23535e4a537e0c4379b8c5d89dfdba30b745e3ee5f02e3216108e0527e8ca761ea96c88c530e3e03d039663b731147a52d4ea0cd57fa6193f31f4092610900527e90c5643f2827c982c91dbd4cf4c0336c69c971bc9f8769f8aedf39873b9e02610920527e94e36693b98706b283fd76c9b01a2b65c24b3e4bf06e0699635cdf1b57fb73610940527e990168e84ae643e23edd30466b74235f1acd0adb4154a33a17da84af7458e4610960527e9d1f6b3cdc458111f9bce9c326ce1b58734ed76a923b3fdacc582a4390b654610980527ea13d6d916da4be41b49ca33fe2281351cbd0a3f9e321dc7b80d5cfd7ad13c56109a0527ea55b6fe5ff03fb716f7c5cbc9d820b4b245270893408791c3553756bc971366109c0527ea979723a906338a12a5c163958dc03447cd43d1884ef15bce9d11affe5cea66109e0527ead97748f21c275d0e53bcfb61435fb3dd55609a7d5d5b25d9e4ec094022c17610a00527eb1b576e3b321b300a01b8932cf8ff3372dd7d63726bc4efe52cc66281e8988610a20527eb5d379384480f0305afb42af8ae9eb308659a2c677a2eb9f074a0bbc3ae6f8610a40527eb9f17b8cd5e02d6015dafc2c4643e329dedb6f55c889883fbbc7b150574469610a60527ebe0f7de1673f6a8fd0bab5a9019ddb23375d3be5197024e0704556e473a1da610a80527ec22d8035f89ea7bf8b9a6f25bcf7d31c8fdf08746a56c18124c2fc788fff4a610aa0527ec64b828a89fde4ef467a28a27851cb15e860d503bb3d5e21d940a20cac5cbb610ac0527eca6984df1b5d221f0159e21f33abc30f40e2a1930c23fac28dbe47a0c8ba2c610ae0527ece878733acbc5f4ebc399b9bef05bb0899646e225d0a9763423bed34e5179c610b00527ed2a589883e1b9c7e77195518aa5fb301f1e63ab1adf13403f6b992c901750d610b20527ed6c38bdccf7ad9ae31f90e9565b9aafb4a680740fed7d0a4ab37385d1dd27e610b40527edae18e3160da16ddecd8c8122113a2f4a2e9d3d04fbe6d455fb4ddf13a2fee610b60527edeff9085f239540da7b8818edc6d9aedfb6ba05fa0a509e614328385568d5f610b80527f01bdff210be472a81b4f71031db8db35dbf6d740bf414a13cc2865070aad1abf610ba052610afe90600990605a610ee3565b50348015610b0a575f5ffd5b5060405161270c38038061270c833981016040819052610b2991610f4c565b6040518060400160405280600e81526020016d4479736e6f6d6961204865636b6560901b815250604051806040016040528060058152602001644845434b4560d81b815250826001600160a01b0316636fb7329c6040518163ffffffff1660e01b8152600401602060405180830381865afa158015610baa573d5f5f3e3d5ffd5b505050506040513d601f19601f82011682018060405250810190610bce9190610f4c565b3380610c0057604051633603021960e21b81523260048201525f60248201523060448201526064015b60405180910390fd5b610c0b816001610d41565b506004610c188482611006565b506005610c258382611006565b50600680546001600160a01b0319166001600160a01b0383169081179091556040805163604a6fa960e01b815290516201b207929163604a6fa991600480830192602092919082900301815f875af1158015610c83573d5f5f3e3d5ffd5b505050506040513d601f19601f82011682018060405250810190610ca791906110c0565b610cb191906110e6565b6001600160401b031660075550610ce49150309050610cd26012600a611219565b600754610cdf9190611227565b610d94565b610d307324f0154c1dce548adf15da2098fdd8b8a3b8151d60125b610d0a90600a611219565b610d15906001611227565b6001600160a01b039091165f90815260086020526040902055565b610d3b816012610cff565b50611251565b6001600160a01b0382165f81815260208190526040808220805460ff191685151590811790915590519092917f6bfa7e860c3e9bf67b810cb88276b19b27c972812bac04793d369b59a1183a8391a35050565b610d9f5f8383610da3565b5050565b6001600160a01b038316610dcd578060035f828254610dc2919061123e565b90915550610e579050565b6001600160a01b0383165f9081526001602052604090205481811015610e3957604051630f7d0bbd60e21b81523260048201523360248201526001600160a01b0380861660448301528416606482015230608482015260a4810182905260c4810183905260e401610bf7565b6001600160a01b0384165f9081526001602052604090209082900390555b6001600160a01b038216610e7357600380548290039055610e91565b6001600160a01b0382165f9081526001602052604090208054820190555b816001600160a01b0316836001600160a01b03167fddf252ad1be2c89b69c2b068fc378daa952ba7f163c4a11628f55a4df523b3ef83604051610ed691815260200190565b60405180910390a3505050565b82605a8101928215610f11579160200282015b82811115610f11578251825591602001919060010190610ef6565b50610f1d929150610f21565b5090565b5b80821115610f1d575f8155600101610f22565b6001600160a01b0381168114610f49575f5ffd5b50565b5f60208284031215610f5c575f5ffd5b8151610f6781610f35565b9392505050565b634e487b7160e01b5f52604160045260245ffd5b600181811c90821680610f9657607f821691505b602082108103610fb457634e487b7160e01b5f52602260045260245ffd5b50919050565b601f82111561100157805f5260205f20601f840160051c81016020851015610fdf5750805b601f840160051c820191505b81811015610ffe575f8155600101610feb565b50505b505050565b81516001600160401b0381111561101f5761101f610f6e565b6110338161102d8454610f82565b84610fba565b6020601f821160018114611065575f831561104e5750848201515b5f19600385901b1c1916600184901b178455610ffe565b5f84815260208120601f198516915b828110156110945787850151825560209485019460019092019101611074565b50848210156110b157868401515f19600387901b60f8161c191681555b50505050600190811b01905550565b5f602082840312156110d0575f5ffd5b81516001600160401b0381168114610f67575f5ffd5b5f6001600160401b0383168061110a57634e487b7160e01b5f52601260045260245ffd5b6001600160401b03929092169190910692915050565b634e487b7160e01b5f52601160045260245ffd5b6001815b600184111561116f5780850481111561115357611153611120565b600184161561116157908102905b60019390931c928002611138565b935093915050565b5f8261118557506001611213565b8161119157505f611213565b81600181146111a757600281146111b1576111cd565b6001915050611213565b60ff8411156111c2576111c2611120565b50506001821b611213565b5060208310610133831016604e8410600b84101617156111f0575081810a611213565b6111fc5f198484611134565b805f190482111561120f5761120f611120565b0290505b92915050565b5f610f6760ff841683611177565b808202811582820484141761121357611213611120565b8082018082111561121357611213611120565b6114ae8061125e5f395ff3fe608060405234801561000f575f5ffd5b506004361061016d575f3560e01c80636fb7329c116100d95780639dcbf7ea11610093578063d5abeb011161006e578063d5abeb0114610383578063dd62ed3e1461038c578063f4e6c93f146103c4578063fd9b726a146103cc575f5ffd5b80639dcbf7ea14610336578063a9059cbb14610349578063d3dc2a0f1461035c575f5ffd5b80636fb7329c146102af5780637065cb48146102da57806370a08231146102ed5780638da5cb5b146103155780638dd75fb81461031b57806395d89b411461032e575f5ffd5b80632b83b1fc1161012a5780632b83b1fc146101ff578063313ce567146102275780633353268d1461023657806338bf3cfa1461025e5780634c2ef29114610271578063666e1b3914610284575f5ffd5b806306fdde0314610171578063095ea7b31461018f57806318160ddd146101b2578063222838db146101c457806323b872dd146101d95780632499a533146101ec575b5f5ffd5b6101796103df565b6040516101869190610e10565b60405180910390f35b6101a261019d366004610e60565b61046f565b6040519015158152602001610186565b6003545b604051908152602001610186565b6101d76101d2366004610e60565b610488565b005b6101a26101e7366004610e88565b610694565b6101d76101fa366004610e60565b6106b7565b6101b661020d366004610ec2565b6001600160a01b03165f9081526008602052604090205490565b60405160128152602001610186565b610249610244366004610ee2565b610879565b60408051928352602083019190915201610186565b6101d761026c366004610ec2565b6109a8565b6101b661027f366004610ee2565b6109bd565b6101a2610292366004610ec2565b6001600160a01b03165f9081526020819052604090205460ff1690565b6006546102c2906001600160a01b031681565b6040516001600160a01b039091168152602001610186565b6101d76102e8366004610ec2565b6109d3565b6101b66102fb366004610ec2565b6001600160a01b03165f9081526001602052604090205490565b306102c2565b6101b6610329366004610ee2565b610a1b565b610179610a61565b6101d7610344366004610fb7565b610a70565b6101a2610357366004610e60565b610a96565b61036a6603632c8eb5af3b81565b60405167ffffffffffffffff9091168152602001610186565b6101b660075481565b6101b661039a36600461101c565b6001600160a01b039182165f90815260026020908152604080832093909416825291909152205490565b6101d7610aa3565b6101b66103da366004610ee2565b610ab5565b6060600480546103ee9061104d565b80601f016020809104026020016040519081016040528092919081815260200182805461041a9061104d565b80156104655780601f1061043c57610100808354040283529160200191610465565b820191905f5260205f20905b81548152906001019060200180831161044857829003601f168201915b5050505050905090565b5f3361047c818585610af0565b60019150505b92915050565b6001600160a01b0382165f9081526008602052604081205490036104cf576040516390b2344160e01b81526001600160a01b03831660048201526024015b60405180910390fd5b815f6104dd6012600a611176565b6001600160a01b0385165f908152600860205260409020546104ff9085611184565b61050991906111af565b6040516323b872dd60e01b81523360048201523060248201819052604482018690529192505f91906323b872dd906064016020604051808303815f875af1158015610556573d5f5f3e3d5ffd5b505050506040513d601f19601f8201168201806040525081019061057a91906111c2565b905080306001600160a01b03166306fdde036040518163ffffffff1660e01b81526004015f60405180830381865afa1580156105b8573d5f5f3e3d5ffd5b505050506040513d5f823e601f3d908101601f191682016040526105df91908101906111e1565b6040516020016105ef9190611256565b6040516020818303038152906040529061061c5760405162461bcd60e51b81526004016104c69190610e10565b5060405163a9059cbb60e01b8152336004820152602481018390526001600160a01b0384169063a9059cbb906044015b6020604051808303815f875af1158015610668573d5f5f3e3d5ffd5b505050506040513d601f19601f8201168201806040525081019061068c91906111c2565b505050505050565b5f336106a1858285610afd565b6106ac858585610b6c565b506001949350505050565b6001600160a01b0382165f9081526008602052604081205490036106f9576040516390b2344160e01b81526001600160a01b03831660048201526024016104c6565b815f6107076012600a611176565b6001600160a01b0385165f908152600860205260409020546107299085611184565b61073391906111af565b6040516323b872dd60e01b8152336004820152306024820152604481018290529091505f906001600160a01b038416906323b872dd906064016020604051808303815f875af1158015610788573d5f5f3e3d5ffd5b505050506040513d601f19601f820116820180604052508101906107ac91906111c2565b905080836001600160a01b03166306fdde036040518163ffffffff1660e01b81526004015f60405180830381865afa1580156107ea573d5f5f3e3d5ffd5b505050506040513d5f823e601f3d908101601f1916820160405261081191908101906111e1565b6040516020016108219190611256565b6040516020818303038152906040529061084e5760405162461bcd60e51b81526004016104c69190610e10565b5060405163a9059cbb60e01b815233600482015260248101859052309063a9059cbb9060440161064c565b5f806009605901548311156108905761089061129a565b5f61089a84610ab5565b90505f80600160598490036108c857506062545f906108ba9088906112ae565b96506108c587610ab5565b93505b60615487036108d9575f94506108ea565b6061546108e79088906112ae565b94505b831561091e5760096108fd6001866112ae565b605a811061090d5761090d611286565b0154925061091b83886112ae565b96505b82600985605a811061093257610932611286565b015461093e91906112ae565b915061094b6002836111af565b87101561095a5786955061097d565b6109656002836111af565b61096f90886112ae565b61097a905f196112c1565b95505b61098961014d866112f0565b9450801561099f5761099c855f196112c1565b94505b50505050915091565b6109b0610b77565b6109ba815f610bd1565b50565b600981605a81106109cc575f80fd5b0154905081565b6109db610b77565b6001600160a01b038116610a1057604051633603021960e21b81523260048201525f60248201523060448201526064016104c6565b6109ba816001610bd1565b5f5f821315610a3d57610a308261014d6112c1565b606154610482919061131c565b610a48825f196112c1565b610a549061014d6112c1565b60615461048291906112ae565b6060600580546103ee9061104d565b610a78610b77565b6004610a84838261137a565b506005610a91828261137a565b505050565b5f3361047c818585610b6c565b610aab610b77565b610ab3610c24565b565b5f805b605a811015610aea5782600982605a8110610ad557610ad5611286565b015410610ae25792915050565b600101610ab8565b50919050565b610a918383836001610c66565b6001600160a01b038381165f908152600260209081526040808320938616835292905220545f198114610b665781811015610b58573233858530858760405162b83d1960e21b81526004016104c69796959493929190611435565b610b6684848484035f610c66565b50505050565b610a91838383610ce6565b335f9081526020819052604090205460ff16158015610ba55750325f9081526020819052604090205460ff16155b15610ab357604051630be6bab560e01b81523260048201523360248201523060448201526064016104c6565b6001600160a01b0382165f81815260208190526040808220805460ff191685151590811790915590519092917f6bfa7e860c3e9bf67b810cb88276b19b27c972812bac04793d369b59a1183a8391a35050565b610c306012600a611176565b600754610c3d9190611184565b6003541015610ab357610ab330610c566012600a611176565b610c61906001611184565b610e01565b6001600160a01b038085165f9081526002602090815260408083209387168352929052208290558015610b6657826001600160a01b0316846001600160a01b03167f8c5be1e5ebec7d5bd14f71427d1e84f3dd0314c0f7b2291e5b200ac8c7c3b92584604051610cd891815260200190565b60405180910390a350505050565b6001600160a01b038316610d10578060035f828254610d05919061131c565b90915550610d759050565b6001600160a01b0383165f9081526001602052604090205481811015610d575732338585308587604051630f7d0bbd60e21b81526004016104c69796959493929190611435565b6001600160a01b0384165f9081526001602052604090209082900390555b6001600160a01b038216610d9157600380548290039055610daf565b6001600160a01b0382165f9081526001602052604090208054820190555b816001600160a01b0316836001600160a01b03167fddf252ad1be2c89b69c2b068fc378daa952ba7f163c4a11628f55a4df523b3ef83604051610df491815260200190565b60405180910390a3505050565b610e0c5f8383610ce6565b5050565b602081525f82518060208401528060208501604085015e5f604082850101526040601f19601f83011684010191505092915050565b80356001600160a01b0381168114610e5b575f5ffd5b919050565b5f5f60408385031215610e71575f5ffd5b610e7a83610e45565b946020939093013593505050565b5f5f5f60608486031215610e9a575f5ffd5b610ea384610e45565b9250610eb160208501610e45565b929592945050506040919091013590565b5f60208284031215610ed2575f5ffd5b610edb82610e45565b9392505050565b5f60208284031215610ef2575f5ffd5b5035919050565b634e487b7160e01b5f52604160045260245ffd5b604051601f8201601f1916810167ffffffffffffffff81118282101715610f3657610f36610ef9565b604052919050565b5f67ffffffffffffffff821115610f5757610f57610ef9565b50601f01601f191660200190565b5f82601f830112610f74575f5ffd5b8135610f87610f8282610f3e565b610f0d565b818152846020838601011115610f9b575f5ffd5b816020850160208301375f918101602001919091529392505050565b5f5f60408385031215610fc8575f5ffd5b823567ffffffffffffffff811115610fde575f5ffd5b610fea85828601610f65565b925050602083013567ffffffffffffffff811115611006575f5ffd5b61101285828601610f65565b9150509250929050565b5f5f6040838503121561102d575f5ffd5b61103683610e45565b915061104460208401610e45565b90509250929050565b600181811c9082168061106157607f821691505b602082108103610aea57634e487b7160e01b5f52602260045260245ffd5b634e487b7160e01b5f52601160045260245ffd5b6001815b60018411156110ce578085048111156110b2576110b261107f565b60018416156110c057908102905b60019390931c928002611097565b935093915050565b5f826110e457506001610482565b816110f057505f610482565b816001811461110657600281146111105761112c565b6001915050610482565b60ff8411156111215761112161107f565b50506001821b610482565b5060208310610133831016604e8410600b841016171561114f575081810a610482565b61115b5f198484611093565b805f190482111561116e5761116e61107f565b029392505050565b5f610edb60ff8416836110d6565b80820281158282048414176104825761048261107f565b634e487b7160e01b5f52601260045260245ffd5b5f826111bd576111bd61119b565b500490565b5f602082840312156111d2575f5ffd5b81518015158114610edb575f5ffd5b5f602082840312156111f1575f5ffd5b815167ffffffffffffffff811115611207575f5ffd5b8201601f81018413611217575f5ffd5b8051611225610f8282610f3e565b818152856020838501011115611239575f5ffd5b8160208401602083015e5f91810160200191909152949350505050565b6d02732b2b21020b8383937bb32b2160951b81525f82518060208501600e85015e5f9201600e0191825250919050565b634e487b7160e01b5f52603260045260245ffd5b634e487b7160e01b5f52600160045260245ffd5b818103818111156104825761048261107f565b8082025f8212600160ff1b841416156112dc576112dc61107f565b81810583148215176104825761048261107f565b5f826112fe576112fe61119b565b600160ff1b82145f19841416156113175761131761107f565b500590565b808201808211156104825761048261107f565b601f821115610a9157805f5260205f20601f840160051c810160208510156113545750805b601f840160051c820191505b81811015611373575f8155600101611360565b5050505050565b815167ffffffffffffffff81111561139457611394610ef9565b6113a8816113a2845461104d565b8461132f565b6020601f8211600181146113da575f83156113c35750848201515b5f19600385901b1c1916600184901b178455611373565b5f84815260208120601f198516915b8281101561140957878501518255602094850194600190920191016113e9565b508482101561142657868401515f19600387901b60f8161c191681555b50505050600190811b01905550565b6001600160a01b039788168152958716602087015293861660408601529185166060850152909316608083015260a082019290925260c081019190915260e0019056fea26469706673582212205a6c0a70fb67966ca9c4ea3afe65d4300908b4de2e81b7037be804213d4bf51264736f6c634300081c0033";
        public HeckeDeploymentBase() : base(BYTECODE) { }
        public HeckeDeploymentBase(string byteCode) : base(byteCode) { }
        [Parameter("address", "ChoAddress", 1)]
        public virtual string ChoAddress { get; set; }
    }

    public partial class ComplimentFunction : ComplimentFunctionBase { }

    [Function("Compliment", typeof(ComplimentOutputDTO))]
    public class ComplimentFunctionBase : FunctionMessage
    {
        [Parameter("uint256", "Waat", 1)]
        public virtual BigInteger Waat { get; set; }
    }

    public partial class GetMarketRateFunction : GetMarketRateFunctionBase { }

    [Function("GetMarketRate", "uint256")]
    public class GetMarketRateFunctionBase : FunctionMessage
    {
        [Parameter("address", "_a", 1)]
        public virtual string A { get; set; }
    }

    public partial class GetMeridianFunction : GetMeridianFunctionBase { }

    [Function("GetMeridian", "uint256")]
    public class GetMeridianFunctionBase : FunctionMessage
    {
        [Parameter("uint256", "Waat", 1)]
        public virtual BigInteger Waat { get; set; }
    }

    public partial class GetWaatFunction : GetWaatFunctionBase { }

    [Function("GetWaat", "uint256")]
    public class GetWaatFunctionBase : FunctionMessage
    {
        [Parameter("int256", "Latitude", 1)]
        public virtual BigInteger Latitude { get; set; }
    }

    public partial class MeridiansFunction : MeridiansFunctionBase { }

    [Function("Meridians", "uint256")]
    public class MeridiansFunctionBase : FunctionMessage
    {
        [Parameter("uint256", "", 1)]
        public virtual BigInteger ReturnValue1 { get; set; }
    }

    public partial class MotzkinPrimeFunction : MotzkinPrimeFunctionBase { }

    [Function("MotzkinPrime", "uint64")]
    public class MotzkinPrimeFunctionBase : FunctionMessage
    {

    }

    public partial class PurchaseFunction : PurchaseFunctionBase { }

    [Function("Purchase")]
    public class PurchaseFunctionBase : FunctionMessage
    {
        [Parameter("address", "_t", 1)]
        public virtual string T { get; set; }
        [Parameter("uint256", "_a", 2)]
        public virtual BigInteger A { get; set; }
    }

    public partial class RedeemFunction : RedeemFunctionBase { }

    [Function("Redeem")]
    public class RedeemFunctionBase : FunctionMessage
    {
        [Parameter("address", "_t", 1)]
        public virtual string T { get; set; }
        [Parameter("uint256", "_a", 2)]
        public virtual BigInteger A { get; set; }
    }

    public partial class RenameFunction : RenameFunctionBase { }

    [Function("Rename")]
    public class RenameFunctionBase : FunctionMessage
    {
        [Parameter("string", "newName", 1)]
        public virtual string NewName { get; set; }
        [Parameter("string", "newSymbol", 2)]
        public virtual string NewSymbol { get; set; }
    }

    public partial class XiaoFunction : XiaoFunctionBase { }

    [Function("Xiao", "address")]
    public class XiaoFunctionBase : FunctionMessage
    {

    }

    public partial class AddOwnerFunction : AddOwnerFunctionBase { }

    [Function("addOwner")]
    public class AddOwnerFunctionBase : FunctionMessage
    {
        [Parameter("address", "newOwner", 1)]
        public virtual string NewOwner { get; set; }
    }

    public partial class AllowanceFunction : AllowanceFunctionBase { }

    [Function("allowance", "uint256")]
    public class AllowanceFunctionBase : FunctionMessage
    {
        [Parameter("address", "owner", 1)]
        public virtual string Owner { get; set; }
        [Parameter("address", "spender", 2)]
        public virtual string Spender { get; set; }
    }

    public partial class ApproveFunction : ApproveFunctionBase { }

    [Function("approve", "bool")]
    public class ApproveFunctionBase : FunctionMessage
    {
        [Parameter("address", "spender", 1)]
        public virtual string Spender { get; set; }
        [Parameter("uint256", "value", 2)]
        public virtual BigInteger Value { get; set; }
    }

    public partial class BalanceOfFunction : BalanceOfFunctionBase { }

    [Function("balanceOf", "uint256")]
    public class BalanceOfFunctionBase : FunctionMessage
    {
        [Parameter("address", "account", 1)]
        public virtual string Account { get; set; }
    }

    public partial class DecimalsFunction : DecimalsFunctionBase { }

    [Function("decimals", "uint8")]
    public class DecimalsFunctionBase : FunctionMessage
    {

    }

    public partial class MaxSupplyFunction : MaxSupplyFunctionBase { }

    [Function("maxSupply", "uint256")]
    public class MaxSupplyFunctionBase : FunctionMessage
    {

    }

    public partial class MintToCapFunction : MintToCapFunctionBase { }

    [Function("mintToCap")]
    public class MintToCapFunctionBase : FunctionMessage
    {

    }

    public partial class NameFunction : NameFunctionBase { }

    [Function("name", "string")]
    public class NameFunctionBase : FunctionMessage
    {

    }

    public partial class Owner1Function : Owner1FunctionBase { }

    [Function("owner", "bool")]
    public class Owner1FunctionBase : FunctionMessage
    {
        [Parameter("address", "cOwner", 1)]
        public virtual string COwner { get; set; }
    }

    public partial class OwnerFunction : OwnerFunctionBase { }

    [Function("owner", "address")]
    public class OwnerFunctionBase : FunctionMessage
    {

    }

    public partial class RenounceOwnershipFunction : RenounceOwnershipFunctionBase { }

    [Function("renounceOwnership")]
    public class RenounceOwnershipFunctionBase : FunctionMessage
    {
        [Parameter("address", "toRemove", 1)]
        public virtual string ToRemove { get; set; }
    }

    public partial class SymbolFunction : SymbolFunctionBase { }

    [Function("symbol", "string")]
    public class SymbolFunctionBase : FunctionMessage
    {

    }

    public partial class TotalSupplyFunction : TotalSupplyFunctionBase { }

    [Function("totalSupply", "uint256")]
    public class TotalSupplyFunctionBase : FunctionMessage
    {

    }

    public partial class TransferFunction : TransferFunctionBase { }

    [Function("transfer", "bool")]
    public class TransferFunctionBase : FunctionMessage
    {
        [Parameter("address", "to", 1)]
        public virtual string To { get; set; }
        [Parameter("uint256", "value", 2)]
        public virtual BigInteger Value { get; set; }
    }

    public partial class TransferFromFunction : TransferFromFunctionBase { }

    [Function("transferFrom", "bool")]
    public class TransferFromFunctionBase : FunctionMessage
    {
        [Parameter("address", "from", 1)]
        public virtual string From { get; set; }
        [Parameter("address", "to", 2)]
        public virtual string To { get; set; }
        [Parameter("uint256", "value", 3)]
        public virtual BigInteger Value { get; set; }
    }

    public partial class ApprovalEventDTO : ApprovalEventDTOBase { }

    [Event("Approval")]
    public class ApprovalEventDTOBase : IEventDTO
    {
        [Parameter("address", "owner", 1, true )]
        public virtual string Owner { get; set; }
        [Parameter("address", "spender", 2, true )]
        public virtual string Spender { get; set; }
        [Parameter("uint256", "value", 3, false )]
        public virtual BigInteger Value { get; set; }
    }

    public partial class OwnershipUpdateEventDTO : OwnershipUpdateEventDTOBase { }

    [Event("OwnershipUpdate")]
    public class OwnershipUpdateEventDTOBase : IEventDTO
    {
        [Parameter("address", "newOwner", 1, true )]
        public virtual string NewOwner { get; set; }
        [Parameter("bool", "state", 2, true )]
        public virtual bool State { get; set; }
    }

    public partial class TransferEventDTO : TransferEventDTOBase { }

    [Event("Transfer")]
    public class TransferEventDTOBase : IEventDTO
    {
        [Parameter("address", "from", 1, true )]
        public virtual string From { get; set; }
        [Parameter("address", "to", 2, true )]
        public virtual string To { get; set; }
        [Parameter("uint256", "value", 3, false )]
        public virtual BigInteger Value { get; set; }
    }

    public partial class DysnomiaInsufficientAllowanceError : DysnomiaInsufficientAllowanceErrorBase { }

    [Error("DysnomiaInsufficientAllowance")]
    public class DysnomiaInsufficientAllowanceErrorBase : IErrorDTO
    {
        [Parameter("address", "origin", 1)]
        public virtual string Origin { get; set; }
        [Parameter("address", "sender", 2)]
        public virtual string Sender { get; set; }
        [Parameter("address", "owner", 3)]
        public virtual string Owner { get; set; }
        [Parameter("address", "spender", 4)]
        public virtual string Spender { get; set; }
        [Parameter("address", "what", 5)]
        public virtual string What { get; set; }
        [Parameter("uint256", "allowance", 6)]
        public virtual BigInteger Allowance { get; set; }
        [Parameter("uint256", "needed", 7)]
        public virtual BigInteger Needed { get; set; }
    }

    public partial class DysnomiaInsufficientBalanceError : DysnomiaInsufficientBalanceErrorBase { }

    [Error("DysnomiaInsufficientBalance")]
    public class DysnomiaInsufficientBalanceErrorBase : IErrorDTO
    {
        [Parameter("address", "origin", 1)]
        public virtual string Origin { get; set; }
        [Parameter("address", "sender", 2)]
        public virtual string Sender { get; set; }
        [Parameter("address", "from", 3)]
        public virtual string From { get; set; }
        [Parameter("address", "to", 4)]
        public virtual string To { get; set; }
        [Parameter("address", "what", 5)]
        public virtual string What { get; set; }
        [Parameter("uint256", "balance", 6)]
        public virtual BigInteger Balance { get; set; }
        [Parameter("uint256", "needed", 7)]
        public virtual BigInteger Needed { get; set; }
    }

    public partial class MarketRateNotFoundError : MarketRateNotFoundErrorBase { }

    [Error("MarketRateNotFound")]
    public class MarketRateNotFoundErrorBase : IErrorDTO
    {
        [Parameter("address", "asset", 1)]
        public virtual string Asset { get; set; }
    }

    public partial class OwnableInvalidOwnerError : OwnableInvalidOwnerErrorBase { }

    [Error("OwnableInvalidOwner")]
    public class OwnableInvalidOwnerErrorBase : IErrorDTO
    {
        [Parameter("address", "origin", 1)]
        public virtual string Origin { get; set; }
        [Parameter("address", "owner", 2)]
        public virtual string Owner { get; set; }
        [Parameter("address", "what", 3)]
        public virtual string What { get; set; }
    }

    public partial class OwnableUnauthorizedAccountError : OwnableUnauthorizedAccountErrorBase { }

    [Error("OwnableUnauthorizedAccount")]
    public class OwnableUnauthorizedAccountErrorBase : IErrorDTO
    {
        [Parameter("address", "origin", 1)]
        public virtual string Origin { get; set; }
        [Parameter("address", "account", 2)]
        public virtual string Account { get; set; }
        [Parameter("address", "what", 3)]
        public virtual string What { get; set; }
    }

    public partial class ComplimentOutputDTO : ComplimentOutputDTOBase { }

    [FunctionOutput]
    public class ComplimentOutputDTOBase : IFunctionOutputDTO 
    {
        [Parameter("int256", "Longitude", 1)]
        public virtual BigInteger Longitude { get; set; }
        [Parameter("int256", "Latitude", 2)]
        public virtual BigInteger Latitude { get; set; }
    }

    public partial class GetMarketRateOutputDTO : GetMarketRateOutputDTOBase { }

    [FunctionOutput]
    public class GetMarketRateOutputDTOBase : IFunctionOutputDTO 
    {
        [Parameter("uint256", "", 1)]
        public virtual BigInteger ReturnValue1 { get; set; }
    }

    public partial class GetMeridianOutputDTO : GetMeridianOutputDTOBase { }

    [FunctionOutput]
    public class GetMeridianOutputDTOBase : IFunctionOutputDTO 
    {
        [Parameter("uint256", "Meridian", 1)]
        public virtual BigInteger Meridian { get; set; }
    }

    public partial class GetWaatOutputDTO : GetWaatOutputDTOBase { }

    [FunctionOutput]
    public class GetWaatOutputDTOBase : IFunctionOutputDTO 
    {
        [Parameter("uint256", "Waat", 1)]
        public virtual BigInteger Waat { get; set; }
    }

    public partial class MeridiansOutputDTO : MeridiansOutputDTOBase { }

    [FunctionOutput]
    public class MeridiansOutputDTOBase : IFunctionOutputDTO 
    {
        [Parameter("uint256", "", 1)]
        public virtual BigInteger ReturnValue1 { get; set; }
    }

    public partial class MotzkinPrimeOutputDTO : MotzkinPrimeOutputDTOBase { }

    [FunctionOutput]
    public class MotzkinPrimeOutputDTOBase : IFunctionOutputDTO 
    {
        [Parameter("uint64", "", 1)]
        public virtual ulong ReturnValue1 { get; set; }
    }







    public partial class XiaoOutputDTO : XiaoOutputDTOBase { }

    [FunctionOutput]
    public class XiaoOutputDTOBase : IFunctionOutputDTO 
    {
        [Parameter("address", "", 1)]
        public virtual string ReturnValue1 { get; set; }
    }



    public partial class AllowanceOutputDTO : AllowanceOutputDTOBase { }

    [FunctionOutput]
    public class AllowanceOutputDTOBase : IFunctionOutputDTO 
    {
        [Parameter("uint256", "", 1)]
        public virtual BigInteger ReturnValue1 { get; set; }
    }



    public partial class BalanceOfOutputDTO : BalanceOfOutputDTOBase { }

    [FunctionOutput]
    public class BalanceOfOutputDTOBase : IFunctionOutputDTO 
    {
        [Parameter("uint256", "", 1)]
        public virtual BigInteger ReturnValue1 { get; set; }
    }

    public partial class DecimalsOutputDTO : DecimalsOutputDTOBase { }

    [FunctionOutput]
    public class DecimalsOutputDTOBase : IFunctionOutputDTO 
    {
        [Parameter("uint8", "", 1)]
        public virtual byte ReturnValue1 { get; set; }
    }

    public partial class MaxSupplyOutputDTO : MaxSupplyOutputDTOBase { }

    [FunctionOutput]
    public class MaxSupplyOutputDTOBase : IFunctionOutputDTO 
    {
        [Parameter("uint256", "", 1)]
        public virtual BigInteger ReturnValue1 { get; set; }
    }



    public partial class NameOutputDTO : NameOutputDTOBase { }

    [FunctionOutput]
    public class NameOutputDTOBase : IFunctionOutputDTO 
    {
        [Parameter("string", "", 1)]
        public virtual string ReturnValue1 { get; set; }
    }

    public partial class Owner1OutputDTO : Owner1OutputDTOBase { }

    [FunctionOutput]
    public class Owner1OutputDTOBase : IFunctionOutputDTO 
    {
        [Parameter("bool", "", 1)]
        public virtual bool ReturnValue1 { get; set; }
    }

    public partial class OwnerOutputDTO : OwnerOutputDTOBase { }

    [FunctionOutput]
    public class OwnerOutputDTOBase : IFunctionOutputDTO 
    {
        [Parameter("address", "", 1)]
        public virtual string ReturnValue1 { get; set; }
    }



    public partial class SymbolOutputDTO : SymbolOutputDTOBase { }

    [FunctionOutput]
    public class SymbolOutputDTOBase : IFunctionOutputDTO 
    {
        [Parameter("string", "", 1)]
        public virtual string ReturnValue1 { get; set; }
    }

    public partial class TotalSupplyOutputDTO : TotalSupplyOutputDTOBase { }

    [FunctionOutput]
    public class TotalSupplyOutputDTOBase : IFunctionOutputDTO 
    {
        [Parameter("uint256", "", 1)]
        public virtual BigInteger ReturnValue1 { get; set; }
    }




}
