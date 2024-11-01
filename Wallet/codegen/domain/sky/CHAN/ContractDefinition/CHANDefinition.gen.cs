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

namespace Dysnomia.Contracts.domain.sky.CHAN.ContractDefinition
{


    public partial class ChanDeployment : ChanDeploymentBase
    {
        public ChanDeployment() : base(BYTECODE) { }
        public ChanDeployment(string byteCode) : base(byteCode) { }
    }

    public class ChanDeploymentBase : ContractDeploymentMessage
    {
        public static string BYTECODE = "608060405234801561000f575f5ffd5b506040516123b13803806123b183398101604081905261002e916105a0565b6040518060400160405280600d81526020016c223cb9b737b6b4b09021b430b760991b8152506040518060400160405280600481526020016321a420a760e11b815250826001600160a01b0316636fb7329c6040518163ffffffff1660e01b8152600401602060405180830381865afa1580156100ad573d5f5f3e3d5ffd5b505050506040513d601f19601f820116820180604052508101906100d191906105a0565b338061010357604051633603021960e21b81523260048201525f60248201523060448201526064015b60405180910390fd5b61010e816001610306565b50600461011b848261065a565b506005610128838261065a565b50600680546001600160a01b0319166001600160a01b0383169081179091556040805163604a6fa960e01b815290516201b207929163604a6fa991600480830192602092919082900301815f875af1158015610186573d5f5f3e3d5ffd5b505050506040513d601f19601f820116820180604052508101906101aa9190610714565b6101b4919061074e565b6001600160401b03166007555050600980546001600160a01b0319166001600160a01b038416179055506101e732610359565b6101ef6103a4565b5f600a60075460065f9054906101000a90046001600160a01b03166001600160a01b031663604a6fa96040518163ffffffff1660e01b81526004016020604051808303815f875af1158015610246573d5f5f3e3d5ffd5b505050506040513d601f19601f8201168201806040525081019061026a9190610714565b6001600160401b031661027d919061077c565b61028791906107a3565b90506102a8326102996012600a61089b565b6102a390846108a9565b6103e3565b6102f47324f0154c1dce548adf15da2098fdd8b8a3b8151d60125b6102ce90600a61089b565b6102d99060016108a9565b6001600160a01b039091165f90815260086020526040902055565b6102ff8260126102c3565b50506108d3565b6001600160a01b0382165f81815260208190526040808220805460ff191685151590811790915590519092917f6bfa7e860c3e9bf67b810cb88276b19b27c972812bac04793d369b59a1183a8391a35050565b6103616103f2565b6001600160a01b03811661039657604051633603021960e21b81523260048201525f60248201523060448201526064016100fa565b6103a1816001610306565b50565b6103b06012600a61089b565b6007546103bd91906108a9565b60035410156103e1576103e1306103d66012600a61089b565b6102a39060016108a9565b565b6103ee5f838361044c565b5050565b335f9081526020819052604090205460ff161580156104205750325f9081526020819052604090205460ff16155b156103e157604051630be6bab560e01b81523260048201523360248201523060448201526064016100fa565b6001600160a01b038316610476578060035f82825461046b91906108c0565b909155506105009050565b6001600160a01b0383165f90815260016020526040902054818110156104e257604051630f7d0bbd60e21b81523260048201523360248201526001600160a01b0380861660448301528416606482015230608482015260a4810182905260c4810183905260e4016100fa565b6001600160a01b0384165f9081526001602052604090209082900390555b6001600160a01b03821661051c5760038054829003905561053a565b6001600160a01b0382165f9081526001602052604090208054820190555b816001600160a01b0316836001600160a01b03167fddf252ad1be2c89b69c2b068fc378daa952ba7f163c4a11628f55a4df523b3ef8360405161057f91815260200190565b60405180910390a3505050565b6001600160a01b03811681146103a1575f5ffd5b5f602082840312156105b0575f5ffd5b81516105bb8161058c565b9392505050565b634e487b7160e01b5f52604160045260245ffd5b600181811c908216806105ea57607f821691505b60208210810361060857634e487b7160e01b5f52602260045260245ffd5b50919050565b601f82111561065557805f5260205f20601f840160051c810160208510156106335750805b601f840160051c820191505b81811015610652575f815560010161063f565b50505b505050565b81516001600160401b03811115610673576106736105c2565b6106878161068184546105d6565b8461060e565b6020601f8211600181146106b9575f83156106a25750848201515b5f19600385901b1c1916600184901b178455610652565b5f84815260208120601f198516915b828110156106e857878501518255602094850194600190920191016106c8565b508482101561070557868401515f19600387901b60f8161c191681555b50505050600190811b01905550565b5f60208284031215610724575f5ffd5b81516001600160401b03811681146105bb575f5ffd5b634e487b7160e01b5f52601260045260245ffd5b5f6001600160401b038316806107665761076661073a565b6001600160401b03929092169190910692915050565b5f8261078a5761078a61073a565b500690565b634e487b7160e01b5f52601160045260245ffd5b5f826107b1576107b161073a565b500490565b6001815b60018411156107f1578085048111156107d5576107d561078f565b60018416156107e357908102905b60019390931c9280026107ba565b935093915050565b5f8261080757506001610895565b8161081357505f610895565b816001811461082957600281146108335761084f565b6001915050610895565b60ff8411156108445761084461078f565b50506001821b610895565b5060208310610133831016604e8410600b8410161715610872575081810a610895565b61087e5f1984846107b6565b805f19048211156108915761089161078f565b0290505b92915050565b5f6105bb60ff8416836107f9565b80820281158282048414176108955761089561078f565b808201808211156108955761089561078f565b611ad1806108e05f395ff3fe608060405234801561000f575f5ffd5b50600436106101e7575f3560e01c806370a0823111610109578063d3dc2a0f1161009e578063e4606fde1161006e578063e4606fde146104c5578063f4e6c93f146104d8578063fcfcea45146104e0578063ff762044146104f3575f5ffd5b8063d3dc2a0f14610422578063d5abeb0114610449578063d70d37e614610452578063dd62ed3e1461048d575f5ffd5b80639dcbf7ea116100d95780639dcbf7ea146103d6578063a9059cbb146103e9578063c4a3ab27146103fc578063d0ba60d11461040f575f5ffd5b806370a082311461038d5780637a9bd221146103b55780638da5cb5b146103c857806395d89b41146103ce575f5ffd5b80632b83b1fc1161017f57806338bf3cfa1161014f57806338bf3cfa14610329578063666e1b391461033c5780636fb7329c146103675780637065cb481461037a575f5ffd5b80632b83b1fc146102cc578063313ce567146102f4578063313d246b1461030357806335cb27d714610316575f5ffd5b8063222838db116101ba578063222838db1461027e57806323b872dd146102935780632499a533146102a65780632621e0f8146102b9575f5ffd5b806306fdde03146101eb578063095ea7b31461020957806318025d891461022c57806318160ddd1461026c575b5f5ffd5b6101f3610516565b60405161020091906113e6565b60405180910390f35b61021c61021736600461142f565b6105a6565b6040519015158152602001610200565b61025461023a366004611459565b600a6020525f90815260409020546001600160a01b031681565b6040516001600160a01b039091168152602001610200565b6003545b604051908152602001610200565b61029161028c36600461142f565b6105bf565b005b61021c6102a1366004611474565b6107c6565b6102916102b436600461142f565b6107e9565b6102916102c73660046114b2565b6109a5565b6102706102da366004611459565b6001600160a01b03165f9081526008602052604090205490565b60405160128152602001610200565b600954610254906001600160a01b031681565b6102916103243660046114e9565b610c7e565b610291610337366004611459565b610d99565b61021c61034a366004611459565b6001600160a01b03165f9081526020819052604090205460ff1690565b600654610254906001600160a01b031681565b610291610388366004611459565b610dae565b61027061039b366004611459565b6001600160a01b03165f9081526001602052604090205490565b6102706103c33660046114b2565b610dee565b30610254565b6101f3610e67565b6102916103e43660046115f5565b610e76565b61021c6103f736600461142f565b610e97565b61029161040a3660046114e9565b610ea4565b61029161041d3660046114b2565b610f42565b6104306603632c8eb5af3b81565b60405167ffffffffffffffff9091168152602001610200565b61027060075481565b61021c6104603660046114b2565b6001600160a01b039182165f908152600b6020908152604080832093909416825291909152205460ff1690565b61027061049b3660046114b2565b6001600160a01b039182165f90815260026020908152604080832093909416825291909152205490565b6102916104d3366004611459565b610fdb565b610291611039565b6102916104ee366004611667565b61104b565b6101f36040518060400160405280600481526020016321a420a760e11b81525081565b60606004805461052590611693565b80601f016020809104026020016040519081016040528092919081815260200182805461055190611693565b801561059c5780601f106105735761010080835404028352916020019161059c565b820191905f5260205f20905b81548152906001019060200180831161057f57829003601f168201915b5050505050905090565b5f336105b38185856110d8565b60019150505b92915050565b6001600160a01b0382165f908152600860205260408120549003610606576040516390b2344160e01b81526001600160a01b03831660048201526024015b60405180910390fd5b815f6106146012600a6117c2565b6001600160a01b0385165f9081526008602052604090205461063690856117d0565b61064091906117e7565b6040516323b872dd60e01b81529091505f9030906323b872dd9061066c90339084908990600401611806565b6020604051808303815f875af1158015610688573d5f5f3e3d5ffd5b505050506040513d601f19601f820116820180604052508101906106ac919061182a565b905080306001600160a01b03166306fdde036040518163ffffffff1660e01b81526004015f60405180830381865afa1580156106ea573d5f5f3e3d5ffd5b505050506040513d5f823e601f3d908101601f191682016040526107119190810190611845565b60405160200161072191906118ba565b6040516020818303038152906040529061074e5760405162461bcd60e51b81526004016105fd91906113e6565b5060405163a9059cbb60e01b8152336004820152602481018390526001600160a01b0384169063a9059cbb906044015b6020604051808303815f875af115801561079a573d5f5f3e3d5ffd5b505050506040513d601f19601f820116820180604052508101906107be919061182a565b505050505050565b5f336107d38582856110e5565b6107de85858561114e565b506001949350505050565b6001600160a01b0382165f90815260086020526040812054900361082b576040516390b2344160e01b81526001600160a01b03831660048201526024016105fd565b815f6108396012600a6117c2565b6001600160a01b0385165f9081526008602052604090205461085b90856117d0565b61086591906117e7565b90505f826001600160a01b03166323b872dd3330856040518463ffffffff1660e01b815260040161089893929190611806565b6020604051808303815f875af11580156108b4573d5f5f3e3d5ffd5b505050506040513d601f19601f820116820180604052508101906108d8919061182a565b905080836001600160a01b03166306fdde036040518163ffffffff1660e01b81526004015f60405180830381865afa158015610916573d5f5f3e3d5ffd5b505050506040513d5f823e601f3d908101601f1916820160405261093d9190810190611845565b60405160200161094d91906118ba565b6040516020818303038152906040529061097a5760405162461bcd60e51b81526004016105fd91906113e6565b5060405163a9059cbb60e01b815233600482015260248101859052309063a9059cbb9060440161077e565b6001600160a01b038181165f908152600a60205260409020541615610a59576001600160a01b038082165f908152600a602090815260409182902054825163c11a4b4760e01b8152925193169263c11a4b479260048082019392918290030181865afa158015610a17573d5f5f3e3d5ffd5b505050506040513d601f19601f82011682018060405250810190610a3b91906118ea565b8282604051631b0b10d560e11b81526004016105fd93929190611905565b5f829050326001600160a01b0316816001600160a01b031663c11a4b476040518163ffffffff1660e01b8152600401602060405180830381865afa158015610aa3573d5f5f3e3d5ffd5b505050506040513d601f19601f82011682018060405250810190610ac791906118ea565b6001600160a01b031614610b6057806001600160a01b031663c11a4b476040518163ffffffff1660e01b8152600401602060405180830381865afa158015610b11573d5f5f3e3d5ffd5b505050506040513d601f19601f82011682018060405250810190610b3591906118ea565b60405163049862c160e31b81526001600160a01b0390911660048201523260248201526044016105fd565b5f600a5f836001600160a01b031663c11a4b476040518163ffffffff1660e01b8152600401602060405180830381865afa158015610ba0573d5f5f3e3d5ffd5b505050506040513d601f19601f82011682018060405250810190610bc491906118ea565b6001600160a01b039081168252602082019290925260409081015f2080546001600160a01b03191693831693909317909255905163e2c1014560e01b815283821660048201529082169063e2c10145906024015f604051808303815f87803b158015610c2e575f5ffd5b505af1158015610c40573d5f5f3e3d5ffd5b505050506001600160a01b038281165f908152600a6020526040902080546001600160a01b031916918516919091179055610c79611159565b505050565b610c8661119b565b6001600160a01b0384165f908152600b6020908152604080832033845290915290205460ff16610d2e57836001600160a01b031663c11a4b476040518163ffffffff1660e01b8152600401602060405180830381865afa158015610cec573d5f5f3e3d5ffd5b505050506040513d601f19601f82011682018060405250810190610d1091906118ea565b843360405163608c36ab60e11b81526004016105fd93929190611905565b60405163c05c330960e01b81526001600160a01b0385169063c05c330990610d5e90869086908690600401611806565b5f604051808303815f87803b158015610d75575f5ffd5b505af1158015610d87573d5f5f3e3d5ffd5b50505050610d93611159565b50505050565b610da161119b565b610dab815f6111ed565b50565b610db661119b565b6001600160a01b038116610de357325f30604051633603021960e21b81526004016105fd93929190611905565b610dab8160016111ed565b5f610df761119b565b6040516335e197f160e11b81526001600160a01b038381166004830152841690636bc32fe2906024016020604051808303815f875af1158015610e3c573d5f5f3e3d5ffd5b505050506040513d601f19601f82011682018060405250810190610e609190611928565b9392505050565b60606005805461052590611693565b610e7e61119b565b6004610e8a838261198a565b506005610c79828261198a565b5f336105b381858561114e565b610eac61119b565b6001600160a01b0384165f908152600b6020908152604080832033845290915290205460ff16610f1257836001600160a01b031663c11a4b476040518163ffffffff1660e01b8152600401602060405180830381865afa158015610cec573d5f5f3e3d5ffd5b604051639b1bfa7f60e01b81526001600160a01b03851690639b1bfa7f90610d5e90869086908690600401611806565b610f4a61119b565b6001600160a01b038281165f908152600a60205260409020541615610fa2576001600160a01b038083165f908152600a602052604090819020549051631b0b10d560e11b81526105fd92859216908490600401611905565b6001600160a01b038281165f908152600a6020526040902080546001600160a01b031916918316919091179055610fd7611159565b5050565b610fe361119b565b806001600160a01b031663773495b96040518163ffffffff1660e01b81526004015f604051808303815f87803b15801561101b575f5ffd5b505af115801561102d573d5f5f3e3d5ffd5b50505050610dab611159565b61104161119b565b611049611159565b565b325f818152600a60209081526040808320546001600160a01b03908116808552600b845282852091881680865291845293829020805460ff19168715159081179091558251958652928501849052908401526060830152907ff34394d7fea5330bacb5ff7ca59f9743f2c55d81cd9bce7aa3f4f02a452602469060800160405180910390a1610c79611159565b610c798383836001611240565b6001600160a01b038381165f908152600260209081526040808320938616835292905220545f198114610d935781811015611140573233858530858760405162b83d1960e21b81526004016105fd9796959493929190611a45565b610d9384848484035f611240565b610c798383836112c0565b6111656012600a6117c2565b60075461117291906117d0565b6003541015611049576110493061118b6012600a6117c2565b6111969060016117d0565b6113db565b335f9081526020819052604090205460ff161580156111c95750325f9081526020819052604090205460ff16155b1561104957323330604051630be6bab560e01b81526004016105fd93929190611905565b6001600160a01b0382165f81815260208190526040808220805460ff191685151590811790915590519092917f6bfa7e860c3e9bf67b810cb88276b19b27c972812bac04793d369b59a1183a8391a35050565b6001600160a01b038085165f9081526002602090815260408083209387168352929052208290558015610d9357826001600160a01b0316846001600160a01b03167f8c5be1e5ebec7d5bd14f71427d1e84f3dd0314c0f7b2291e5b200ac8c7c3b925846040516112b291815260200190565b60405180910390a350505050565b6001600160a01b0383166112ea578060035f8282546112df9190611a88565b9091555061134f9050565b6001600160a01b0383165f90815260016020526040902054818110156113315732338585308587604051630f7d0bbd60e21b81526004016105fd9796959493929190611a45565b6001600160a01b0384165f9081526001602052604090209082900390555b6001600160a01b03821661136b57600380548290039055611389565b6001600160a01b0382165f9081526001602052604090208054820190555b816001600160a01b0316836001600160a01b03167fddf252ad1be2c89b69c2b068fc378daa952ba7f163c4a11628f55a4df523b3ef836040516113ce91815260200190565b60405180910390a3505050565b610fd75f83836112c0565b602081525f82518060208401528060208501604085015e5f604082850101526040601f19601f83011684010191505092915050565b6001600160a01b0381168114610dab575f5ffd5b5f5f60408385031215611440575f5ffd5b823561144b8161141b565b946020939093013593505050565b5f60208284031215611469575f5ffd5b8135610e608161141b565b5f5f5f60608486031215611486575f5ffd5b83356114918161141b565b925060208401356114a18161141b565b929592945050506040919091013590565b5f5f604083850312156114c3575f5ffd5b82356114ce8161141b565b915060208301356114de8161141b565b809150509250929050565b5f5f5f5f608085870312156114fc575f5ffd5b84356115078161141b565b935060208501356115178161141b565b925060408501356115278161141b565b9396929550929360600135925050565b634e487b7160e01b5f52604160045260245ffd5b604051601f8201601f1916810167ffffffffffffffff8111828210171561157457611574611537565b604052919050565b5f67ffffffffffffffff82111561159557611595611537565b50601f01601f191660200190565b5f82601f8301126115b2575f5ffd5b81356115c56115c08261157c565b61154b565b8181528460208386010111156115d9575f5ffd5b816020850160208301375f918101602001919091529392505050565b5f5f60408385031215611606575f5ffd5b823567ffffffffffffffff81111561161c575f5ffd5b611628858286016115a3565b925050602083013567ffffffffffffffff811115611644575f5ffd5b611650858286016115a3565b9150509250929050565b8015158114610dab575f5ffd5b5f5f60408385031215611678575f5ffd5b82356116838161141b565b915060208301356114de8161165a565b600181811c908216806116a757607f821691505b6020821081036116c557634e487b7160e01b5f52602260045260245ffd5b50919050565b634e487b7160e01b5f52601160045260245ffd5b6001815b600184111561171a578085048111156116fe576116fe6116cb565b600184161561170c57908102905b60019390931c9280026116e3565b935093915050565b5f82611730575060016105b9565b8161173c57505f6105b9565b8160018114611752576002811461175c57611778565b60019150506105b9565b60ff84111561176d5761176d6116cb565b50506001821b6105b9565b5060208310610133831016604e8410600b841016171561179b575081810a6105b9565b6117a75f1984846116df565b805f19048211156117ba576117ba6116cb565b029392505050565b5f610e6060ff841683611722565b80820281158282048414176105b9576105b96116cb565b5f8261180157634e487b7160e01b5f52601260045260245ffd5b500490565b6001600160a01b039384168152919092166020820152604081019190915260600190565b5f6020828403121561183a575f5ffd5b8151610e608161165a565b5f60208284031215611855575f5ffd5b815167ffffffffffffffff81111561186b575f5ffd5b8201601f8101841361187b575f5ffd5b80516118896115c08261157c565b81815285602083850101111561189d575f5ffd5b8160208401602083015e5f91810160200191909152949350505050565b6d02732b2b21020b8383937bb32b2160951b81525f82518060208501600e85015e5f9201600e0191825250919050565b5f602082840312156118fa575f5ffd5b8151610e608161141b565b6001600160a01b0393841681529183166020830152909116604082015260600190565b5f60208284031215611938575f5ffd5b5051919050565b601f821115610c7957805f5260205f20601f840160051c810160208510156119645750805b601f840160051c820191505b81811015611983575f8155600101611970565b5050505050565b815167ffffffffffffffff8111156119a4576119a4611537565b6119b8816119b28454611693565b8461193f565b6020601f8211600181146119ea575f83156119d35750848201515b5f19600385901b1c1916600184901b178455611983565b5f84815260208120601f198516915b82811015611a1957878501518255602094850194600190920191016119f9565b5084821015611a3657868401515f19600387901b60f8161c191681555b50505050600190811b01905550565b6001600160a01b039788168152958716602087015293861660408601529185166060850152909316608083015260a082019290925260c081019190915260e00190565b808201808211156105b9576105b96116cb56fea2646970667358221220a2c8d363db6ba1be8228baedc37e34c003b7e7b7418e830b7a17bb44aad0cab164736f6c634300081c0033";
        public ChanDeploymentBase() : base(BYTECODE) { }
        public ChanDeploymentBase(string byteCode) : base(byteCode) { }
        [Parameter("address", "XieAddress", 1)]
        public virtual string XieAddress { get; set; }
    }

    public partial class AddYueFunction : AddYueFunctionBase { }

    [Function("AddYue")]
    public class AddYueFunctionBase : FunctionMessage
    {
        [Parameter("address", "Origin", 1)]
        public virtual string Origin { get; set; }
        [Parameter("address", "Yue", 2)]
        public virtual string Yue { get; set; }
    }

    public partial class GetMarketRateFunction : GetMarketRateFunctionBase { }

    [Function("GetMarketRate", "uint256")]
    public class GetMarketRateFunctionBase : FunctionMessage
    {
        [Parameter("address", "_a", 1)]
        public virtual string A { get; set; }
    }

    public partial class MotzkinPrimeFunction : MotzkinPrimeFunctionBase { }

    [Function("MotzkinPrime", "uint64")]
    public class MotzkinPrimeFunctionBase : FunctionMessage
    {

    }

    public partial class OptInFunction : OptInFunctionBase { }

    [Function("OptIn")]
    public class OptInFunctionBase : FunctionMessage
    {
        [Parameter("address", "Contract", 1)]
        public virtual string Contract { get; set; }
        [Parameter("bool", "Allow", 2)]
        public virtual bool Allow { get; set; }
    }

    public partial class OptedInFunction : OptedInFunctionBase { }

    [Function("OptedIn", "bool")]
    public class OptedInFunctionBase : FunctionMessage
    {
        [Parameter("address", "Yue", 1)]
        public virtual string Yue { get; set; }
        [Parameter("address", "Contract", 2)]
        public virtual string Contract { get; set; }
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

    public partial class ReactYueFunction : ReactYueFunctionBase { }

    [Function("ReactYue", "uint256")]
    public class ReactYueFunctionBase : FunctionMessage
    {
        [Parameter("address", "Yue", 1)]
        public virtual string Yue { get; set; }
        [Parameter("address", "Qing", 2)]
        public virtual string Qing { get; set; }
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

    public partial class TransferYueFunction : TransferYueFunctionBase { }

    [Function("TransferYue")]
    public class TransferYueFunctionBase : FunctionMessage
    {
        [Parameter("address", "Yue", 1)]
        public virtual string Yue { get; set; }
        [Parameter("address", "NewOrigin", 2)]
        public virtual string NewOrigin { get; set; }
    }

    public partial class TypeFunction : TypeFunctionBase { }

    [Function("Type", "string")]
    public class TypeFunctionBase : FunctionMessage
    {

    }

    public partial class XiaoFunction : XiaoFunctionBase { }

    [Function("Xiao", "address")]
    public class XiaoFunctionBase : FunctionMessage
    {

    }

    public partial class XieFunction : XieFunctionBase { }

    [Function("Xie", "address")]
    public class XieFunctionBase : FunctionMessage
    {

    }

    public partial class YanFunction : YanFunctionBase { }

    [Function("Yan", "address")]
    public class YanFunctionBase : FunctionMessage
    {
        [Parameter("address", "", 1)]
        public virtual string ReturnValue1 { get; set; }
    }

    public partial class YueForceTransferFunction : YueForceTransferFunctionBase { }

    [Function("YueForceTransfer")]
    public class YueForceTransferFunctionBase : FunctionMessage
    {
        [Parameter("address", "Yue", 1)]
        public virtual string Yue { get; set; }
        [Parameter("address", "From", 2)]
        public virtual string From { get; set; }
        [Parameter("address", "To", 3)]
        public virtual string To { get; set; }
        [Parameter("uint256", "Amount", 4)]
        public virtual BigInteger Amount { get; set; }
    }

    public partial class YueMintToOriginFunction : YueMintToOriginFunctionBase { }

    [Function("YueMintToOrigin")]
    public class YueMintToOriginFunctionBase : FunctionMessage
    {
        [Parameter("address", "Yue", 1)]
        public virtual string Yue { get; set; }
    }

    public partial class YueWithdrawFunction : YueWithdrawFunctionBase { }

    [Function("YueWithdraw")]
    public class YueWithdrawFunctionBase : FunctionMessage
    {
        [Parameter("address", "Yue", 1)]
        public virtual string Yue { get; set; }
        [Parameter("address", "Asset", 2)]
        public virtual string Asset { get; set; }
        [Parameter("address", "To", 3)]
        public virtual string To { get; set; }
        [Parameter("uint256", "Amount", 4)]
        public virtual BigInteger Amount { get; set; }
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

    public partial class NewSpenderContractOptInEventDTO : NewSpenderContractOptInEventDTOBase { }

    [Event("NewSpenderContractOptIn")]
    public class NewSpenderContractOptInEventDTOBase : IEventDTO
    {
        [Parameter("address", "Origin", 1, false )]
        public virtual string Origin { get; set; }
        [Parameter("address", "Yue", 2, false )]
        public virtual string Yue { get; set; }
        [Parameter("address", "Contract", 3, false )]
        public virtual string Contract { get; set; }
        [Parameter("bool", "Allow", 4, false )]
        public virtual bool Allow { get; set; }
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

    public partial class AlreadyAddedError : AlreadyAddedErrorBase { }

    [Error("AlreadyAdded")]
    public class AlreadyAddedErrorBase : IErrorDTO
    {
        [Parameter("address", "Origin", 1)]
        public virtual string Origin { get; set; }
        [Parameter("address", "Yue", 2)]
        public virtual string Yue { get; set; }
        [Parameter("address", "New", 3)]
        public virtual string New { get; set; }
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

    public partial class NotOriginError : NotOriginErrorBase { }

    [Error("NotOrigin")]
    public class NotOriginErrorBase : IErrorDTO
    {
        [Parameter("address", "YueOrigin", 1)]
        public virtual string YueOrigin { get; set; }
        [Parameter("address", "Requestor", 2)]
        public virtual string Requestor { get; set; }
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

    public partial class PlayerMustOptInError : PlayerMustOptInErrorBase { }

    [Error("PlayerMustOptIn")]
    public class PlayerMustOptInErrorBase : IErrorDTO
    {
        [Parameter("address", "Player", 1)]
        public virtual string Player { get; set; }
        [Parameter("address", "Yue", 2)]
        public virtual string Yue { get; set; }
        [Parameter("address", "Contract", 3)]
        public virtual string Contract { get; set; }
    }



    public partial class GetMarketRateOutputDTO : GetMarketRateOutputDTOBase { }

    [FunctionOutput]
    public class GetMarketRateOutputDTOBase : IFunctionOutputDTO 
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



    public partial class OptedInOutputDTO : OptedInOutputDTOBase { }

    [FunctionOutput]
    public class OptedInOutputDTOBase : IFunctionOutputDTO 
    {
        [Parameter("bool", "", 1)]
        public virtual bool ReturnValue1 { get; set; }
    }











    public partial class TypeOutputDTO : TypeOutputDTOBase { }

    [FunctionOutput]
    public class TypeOutputDTOBase : IFunctionOutputDTO 
    {
        [Parameter("string", "", 1)]
        public virtual string ReturnValue1 { get; set; }
    }

    public partial class XiaoOutputDTO : XiaoOutputDTOBase { }

    [FunctionOutput]
    public class XiaoOutputDTOBase : IFunctionOutputDTO 
    {
        [Parameter("address", "", 1)]
        public virtual string ReturnValue1 { get; set; }
    }

    public partial class XieOutputDTO : XieOutputDTOBase { }

    [FunctionOutput]
    public class XieOutputDTOBase : IFunctionOutputDTO 
    {
        [Parameter("address", "", 1)]
        public virtual string ReturnValue1 { get; set; }
    }

    public partial class YanOutputDTO : YanOutputDTOBase { }

    [FunctionOutput]
    public class YanOutputDTOBase : IFunctionOutputDTO 
    {
        [Parameter("address", "Yue", 1)]
        public virtual string Yue { get; set; }
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
