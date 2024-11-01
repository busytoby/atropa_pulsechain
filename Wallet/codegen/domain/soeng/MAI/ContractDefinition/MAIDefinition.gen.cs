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

namespace Dysnomia.Contracts.domain.soeng.MAI.ContractDefinition
{


    public partial class MaiDeployment : MaiDeploymentBase
    {
        public MaiDeployment() : base(BYTECODE) { }
        public MaiDeployment(string byteCode) : base(byteCode) { }
    }

    public class MaiDeploymentBase : ContractDeploymentMessage
    {
        public static string BYTECODE = "608060405234801561000f575f5ffd5b50604051611fbc380380611fbc83398101604081905261002e9161059e565b6040518060400160405280600c81526020016b4479736e6f6d6961204d414960a01b815250604051806040016040528060038152602001624d414960e81b815250826001600160a01b0316636fb7329c6040518163ffffffff1660e01b8152600401602060405180830381865afa1580156100ab573d5f5f3e3d5ffd5b505050506040513d601f19601f820116820180604052508101906100cf919061059e565b338061010157604051633603021960e21b81523260048201525f60248201523060448201526064015b60405180910390fd5b61010c816001610304565b5060046101198482610658565b5060056101268382610658565b50600680546001600160a01b0319166001600160a01b0383169081179091556040805163604a6fa960e01b815290516201b207929163604a6fa991600480830192602092919082900301815f875af1158015610184573d5f5f3e3d5ffd5b505050506040513d601f19601f820116820180604052508101906101a89190610712565b6101b2919061074c565b6001600160401b03166007555050600980546001600160a01b0319166001600160a01b038416179055506101e532610357565b5f600a60075460065f9054906101000a90046001600160a01b03166001600160a01b031663604a6fa96040518163ffffffff1660e01b81526004016020604051808303815f875af115801561023c573d5f5f3e3d5ffd5b505050506040513d601f19601f820116820180604052508101906102609190610712565b6001600160401b0316610273919061077a565b61027d91906107a1565b905061029e3261028f6012600a610899565b61029990846108a7565b6103a2565b6102ea7324f0154c1dce548adf15da2098fdd8b8a3b8151d60125b6102c490600a610899565b6102cf9060016108a7565b6001600160a01b039091165f90815260086020526040902055565b6102f58260126102b9565b6102fd6103b1565b50506108d1565b6001600160a01b0382165f81815260208190526040808220805460ff191685151590811790915590519092917f6bfa7e860c3e9bf67b810cb88276b19b27c972812bac04793d369b59a1183a8391a35050565b61035f6103f0565b6001600160a01b03811661039457604051633603021960e21b81523260048201525f60248201523060448201526064016100f8565b61039f816001610304565b50565b6103ad5f838361044a565b5050565b6103bd6012600a610899565b6007546103ca91906108a7565b60035410156103ee576103ee306103e36012600a610899565b6102999060016108a7565b565b335f9081526020819052604090205460ff1615801561041e5750325f9081526020819052604090205460ff16155b156103ee57604051630be6bab560e01b81523260048201523360248201523060448201526064016100f8565b6001600160a01b038316610474578060035f82825461046991906108be565b909155506104fe9050565b6001600160a01b0383165f90815260016020526040902054818110156104e057604051630f7d0bbd60e21b81523260048201523360248201526001600160a01b0380861660448301528416606482015230608482015260a4810182905260c4810183905260e4016100f8565b6001600160a01b0384165f9081526001602052604090209082900390555b6001600160a01b03821661051a57600380548290039055610538565b6001600160a01b0382165f9081526001602052604090208054820190555b816001600160a01b0316836001600160a01b03167fddf252ad1be2c89b69c2b068fc378daa952ba7f163c4a11628f55a4df523b3ef8360405161057d91815260200190565b60405180910390a3505050565b6001600160a01b038116811461039f575f5ffd5b5f602082840312156105ae575f5ffd5b81516105b98161058a565b9392505050565b634e487b7160e01b5f52604160045260245ffd5b600181811c908216806105e857607f821691505b60208210810361060657634e487b7160e01b5f52602260045260245ffd5b50919050565b601f82111561065357805f5260205f20601f840160051c810160208510156106315750805b601f840160051c820191505b81811015610650575f815560010161063d565b50505b505050565b81516001600160401b03811115610671576106716105c0565b6106858161067f84546105d4565b8461060c565b6020601f8211600181146106b7575f83156106a05750848201515b5f19600385901b1c1916600184901b178455610650565b5f84815260208120601f198516915b828110156106e657878501518255602094850194600190920191016106c6565b508482101561070357868401515f19600387901b60f8161c191681555b50505050600190811b01905550565b5f60208284031215610722575f5ffd5b81516001600160401b03811681146105b9575f5ffd5b634e487b7160e01b5f52601260045260245ffd5b5f6001600160401b0383168061076457610764610738565b6001600160401b03929092169190910692915050565b5f8261078857610788610738565b500690565b634e487b7160e01b5f52601160045260245ffd5b5f826107af576107af610738565b500490565b6001815b60018411156107ef578085048111156107d3576107d361078d565b60018416156107e157908102905b60019390931c9280026107b8565b935093915050565b5f8261080557506001610893565b8161081157505f610893565b816001811461082757600281146108315761084d565b6001915050610893565b60ff8411156108425761084261078d565b50506001821b610893565b5060208310610133831016604e8410600b8410161715610870575081810a610893565b61087c5f1984846107b4565b805f190482111561088f5761088f61078d565b0290505b92915050565b5f6105b960ff8416836107f7565b80820281158282048414176108935761089361078d565b808201808211156108935761089361078d565b6116de806108de5f395ff3fe608060405234801561000f575f5ffd5b5060043610610153575f3560e01c806370a08231116100bf578063a9059cbb11610079578063a9059cbb14610307578063d3dc2a0f1461031a578063d5abeb0114610340578063dd62ed3e14610349578063f4e6c93f14610381578063ff76204414610389575f5ffd5b806370a0823114610298578063743a3e5e146102c057806384d0643e146102d35780638da5cb5b146102e657806395d89b41146102ec5780639dcbf7ea146102f4575f5ffd5b80632b83b1fc116101105780632b83b1fc146101e5578063313ce5671461020d57806338bf3cfa1461021c578063666e1b391461022f5780636fb7329c1461025a5780637065cb4814610285575f5ffd5b806306fdde0314610157578063095ea7b31461017557806318160ddd14610198578063222838db146101aa57806323b872dd146101bf5780632499a533146101d2575b5f5ffd5b61015f6103ab565b60405161016c9190610ed6565b60405180910390f35b610188610183366004610f1f565b61043b565b604051901515815260200161016c565b6003545b60405190815260200161016c565b6101bd6101b8366004610f1f565b610454565b005b6101886101cd366004610f49565b610660565b6101bd6101e0366004610f1f565b610683565b61019c6101f3366004610f87565b6001600160a01b03165f9081526008602052604090205490565b6040516012815260200161016c565b6101bd61022a366004610f87565b610845565b61018861023d366004610f87565b6001600160a01b03165f9081526020819052604090205460ff1690565b60065461026d906001600160a01b031681565b6040516001600160a01b03909116815260200161016c565b6101bd610293366004610f87565b61085a565b61019c6102a6366004610f87565b6001600160a01b03165f9081526001602052604090205490565b61019c6102ce366004610fbd565b6108a2565b60095461026d906001600160a01b031681565b3061026d565b61015f610b62565b6101bd6103023660046110e0565b610b71565b610188610315366004610f1f565b610b97565b6103286603632c8eb5af3b81565b6040516001600160401b03909116815260200161016c565b61019c60075481565b61019c610357366004611143565b6001600160a01b039182165f90815260026020908152604080832093909416825291909152205490565b6101bd610ba4565b61015f604051806040016040528060038152602001624d414960e81b81525081565b6060600480546103ba9061117a565b80601f01602080910402602001604051908101604052809291908181526020018280546103e69061117a565b80156104315780601f1061040857610100808354040283529160200191610431565b820191905f5260205f20905b81548152906001019060200180831161041457829003601f168201915b5050505050905090565b5f33610448818585610bb6565b60019150505b92915050565b6001600160a01b0382165f90815260086020526040812054900361049b576040516390b2344160e01b81526001600160a01b03831660048201526024015b60405180910390fd5b815f6104a96012600a6112a9565b6001600160a01b0385165f908152600860205260409020546104cb90856112b7565b6104d591906112ce565b6040516323b872dd60e01b81523360048201523060248201819052604482018690529192505f91906323b872dd906064016020604051808303815f875af1158015610522573d5f5f3e3d5ffd5b505050506040513d601f19601f8201168201806040525081019061054691906112ed565b905080306001600160a01b03166306fdde036040518163ffffffff1660e01b81526004015f60405180830381865afa158015610584573d5f5f3e3d5ffd5b505050506040513d5f823e601f3d908101601f191682016040526105ab9190810190611359565b6040516020016105bb9190611392565b604051602081830303815290604052906105e85760405162461bcd60e51b81526004016104929190610ed6565b5060405163a9059cbb60e01b8152336004820152602481018390526001600160a01b0384169063a9059cbb906044015b6020604051808303815f875af1158015610634573d5f5f3e3d5ffd5b505050506040513d601f19601f8201168201806040525081019061065891906112ed565b505050505050565b5f3361066d858285610bc3565b610678858585610c32565b506001949350505050565b6001600160a01b0382165f9081526008602052604081205490036106c5576040516390b2344160e01b81526001600160a01b0383166004820152602401610492565b815f6106d36012600a6112a9565b6001600160a01b0385165f908152600860205260409020546106f590856112b7565b6106ff91906112ce565b6040516323b872dd60e01b8152336004820152306024820152604481018290529091505f906001600160a01b038416906323b872dd906064016020604051808303815f875af1158015610754573d5f5f3e3d5ffd5b505050506040513d601f19601f8201168201806040525081019061077891906112ed565b905080836001600160a01b03166306fdde036040518163ffffffff1660e01b81526004015f60405180830381865afa1580156107b6573d5f5f3e3d5ffd5b505050506040513d5f823e601f3d908101601f191682016040526107dd9190810190611359565b6040516020016107ed9190611392565b6040516020818303038152906040529061081a5760405162461bcd60e51b81526004016104929190610ed6565b5060405163a9059cbb60e01b815233600482015260248101859052309063a9059cbb90604401610618565b61084d610c3d565b610857815f610c97565b50565b610862610c3d565b6001600160a01b03811661089757604051633603021960e21b81523260048201525f6024820152306044820152606401610492565b610857816001610c97565b5f6108ab610cea565b60095460408051635d7ec6a560e11b815290515f926001600160a01b03169163bafd8d4a9160048083019260209291908290030181865afa1580156108f2573d5f5f3e3d5ffd5b505050506040513d601f19601f8201168201806040525081019061091691906113c2565b6001600160a01b0316636872fc3c6040518163ffffffff1660e01b8152600401602060405180830381865afa158015610951573d5f5f3e3d5ffd5b505050506040513d601f19601f8201168201806040525081019061097591906113c2565b604051631bc1073d60e11b81526001600160401b03861660048201526001600160a01b0391909116906337820e7a906024015f604051808303815f875af11580156109c2573d5f5f3e3d5ffd5b505050506040513d5f823e601f3d908101601f191682016040526109e991908101906113f8565b90505f60095f9054906101000a90046001600160a01b03166001600160a01b031663bafd8d4a6040518163ffffffff1660e01b8152600401602060405180830381865afa158015610a3c573d5f5f3e3d5ffd5b505050506040513d601f19601f82011682018060405250810190610a6091906113c2565b6001600160a01b03166320bc1aa2856040518263ffffffff1660e01b8152600401610a8d91815260200190565b602060405180830381865afa158015610aa8573d5f5f3e3d5ffd5b505050506040513d601f19601f82011682018060405250810190610acc91906113c2565b60608301516020840151516040516370a0823160e01b81526001600160a01b0391821660048201529293506001600160401b0390911691908316906370a0823190602401602060405180830381865afa158015610b2b573d5f5f3e3d5ffd5b505050506040513d601f19601f82011682018060405250810190610b4f9190611536565b610b5991906112ce565b95945050505050565b6060600580546103ba9061117a565b610b79610c3d565b6004610b858382611598565b506005610b928282611598565b505050565b5f33610448818585610c32565b610bac610c3d565b610bb4610cea565b565b610b928383836001610d2c565b6001600160a01b038381165f908152600260209081526040808320938616835292905220545f198114610c2c5781811015610c1e573233858530858760405162b83d1960e21b81526004016104929796959493929190611652565b610c2c84848484035f610d2c565b50505050565b610b92838383610dac565b335f9081526020819052604090205460ff16158015610c6b5750325f9081526020819052604090205460ff16155b15610bb457604051630be6bab560e01b8152326004820152336024820152306044820152606401610492565b6001600160a01b0382165f81815260208190526040808220805460ff191685151590811790915590519092917f6bfa7e860c3e9bf67b810cb88276b19b27c972812bac04793d369b59a1183a8391a35050565b610cf66012600a6112a9565b600754610d0391906112b7565b6003541015610bb457610bb430610d1c6012600a6112a9565b610d279060016112b7565b610ec7565b6001600160a01b038085165f9081526002602090815260408083209387168352929052208290558015610c2c57826001600160a01b0316846001600160a01b03167f8c5be1e5ebec7d5bd14f71427d1e84f3dd0314c0f7b2291e5b200ac8c7c3b92584604051610d9e91815260200190565b60405180910390a350505050565b6001600160a01b038316610dd6578060035f828254610dcb9190611695565b90915550610e3b9050565b6001600160a01b0383165f9081526001602052604090205481811015610e1d5732338585308587604051630f7d0bbd60e21b81526004016104929796959493929190611652565b6001600160a01b0384165f9081526001602052604090209082900390555b6001600160a01b038216610e5757600380548290039055610e75565b6001600160a01b0382165f9081526001602052604090208054820190555b816001600160a01b0316836001600160a01b03167fddf252ad1be2c89b69c2b068fc378daa952ba7f163c4a11628f55a4df523b3ef83604051610eba91815260200190565b60405180910390a3505050565b610ed25f8383610dac565b5050565b602081525f82518060208401528060208501604085015e5f604082850101526040601f19601f83011684010191505092915050565b6001600160a01b0381168114610857575f5ffd5b5f5f60408385031215610f30575f5ffd5b8235610f3b81610f0b565b946020939093013593505050565b5f5f5f60608486031215610f5b575f5ffd5b8335610f6681610f0b565b92506020840135610f7681610f0b565b929592945050506040919091013590565b5f60208284031215610f97575f5ffd5b8135610fa281610f0b565b9392505050565b6001600160401b0381168114610857575f5ffd5b5f5f60408385031215610fce575f5ffd5b8235610f3b81610fa9565b634e487b7160e01b5f52604160045260245ffd5b604051608081016001600160401b038111828210171561100f5761100f610fd9565b60405290565b60405161010081016001600160401b038111828210171561100f5761100f610fd9565b604051601f8201601f191681016001600160401b038111828210171561106057611060610fd9565b604052919050565b5f6001600160401b0382111561108057611080610fd9565b50601f01601f191660200190565b5f82601f83011261109d575f5ffd5b81356110b06110ab82611068565b611038565b8181528460208386010111156110c4575f5ffd5b816020850160208301375f918101602001919091529392505050565b5f5f604083850312156110f1575f5ffd5b82356001600160401b03811115611106575f5ffd5b6111128582860161108e565b92505060208301356001600160401b0381111561112d575f5ffd5b6111398582860161108e565b9150509250929050565b5f5f60408385031215611154575f5ffd5b823561115f81610f0b565b9150602083013561116f81610f0b565b809150509250929050565b600181811c9082168061118e57607f821691505b6020821081036111ac57634e487b7160e01b5f52602260045260245ffd5b50919050565b634e487b7160e01b5f52601160045260245ffd5b6001815b6001841115611201578085048111156111e5576111e56111b2565b60018416156111f357908102905b60019390931c9280026111ca565b935093915050565b5f826112175750600161044e565b8161122357505f61044e565b816001811461123957600281146112435761125f565b600191505061044e565b60ff841115611254576112546111b2565b50506001821b61044e565b5060208310610133831016604e8410600b8410161715611282575081810a61044e565b61128e5f1984846111c6565b805f19048211156112a1576112a16111b2565b029392505050565b5f610fa260ff841683611209565b808202811582820484141761044e5761044e6111b2565b5f826112e857634e487b7160e01b5f52601260045260245ffd5b500490565b5f602082840312156112fd575f5ffd5b81518015158114610fa2575f5ffd5b5f82601f83011261131b575f5ffd5b81516113296110ab82611068565b81815284602083860101111561133d575f5ffd5b8160208501602083015e5f918101602001919091529392505050565b5f60208284031215611369575f5ffd5b81516001600160401b0381111561137e575f5ffd5b61138a8482850161130c565b949350505050565b6d02732b2b21020b8383937bb32b2160951b81525f82518060208501600e85015e5f9201600e0191825250919050565b5f602082840312156113d2575f5ffd5b8151610fa281610f0b565b80516113e881610fa9565b919050565b80516113e881610f0b565b5f60208284031215611408575f5ffd5b81516001600160401b0381111561141d575f5ffd5b8201808403610160811215611430575f5ffd5b611438610fed565b825161144381610fa9565b8152610100601f1983011215611457575f5ffd5b61145f611015565b9150602083015161146f81610f0b565b825261147d604084016113ed565b602083015261148e606084016113dd565b604083015261149f608084016113dd565b60608301526114b060a084016113ed565b60808301526114c160c084016113dd565b60a08301526114d260e084016113dd565b60c08301526114e461010084016113dd565b60e083015281602082015261012083015191506001600160401b0382111561150a575f5ffd5b6115168683850161130c565b604082015261152861014084016113dd565b606082015295945050505050565b5f60208284031215611546575f5ffd5b5051919050565b601f821115610b9257805f5260205f20601f840160051c810160208510156115725750805b601f840160051c820191505b81811015611591575f815560010161157e565b5050505050565b81516001600160401b038111156115b1576115b1610fd9565b6115c5816115bf845461117a565b8461154d565b6020601f8211600181146115f7575f83156115e05750848201515b5f19600385901b1c1916600184901b178455611591565b5f84815260208120601f198516915b828110156116265787850151825560209485019460019092019101611606565b508482101561164357868401515f19600387901b60f8161c191681555b50505050600190811b01905550565b6001600160a01b039788168152958716602087015293861660408601529185166060850152909316608083015260a082019290925260c081019190915260e00190565b8082018082111561044e5761044e6111b256fea2646970667358221220e2d7077e20117921204296b999856192aa7c9b64ab186852eea8d7a425d7637a64736f6c634300081c0033";
        public MaiDeploymentBase() : base(BYTECODE) { }
        public MaiDeploymentBase(string byteCode) : base(byteCode) { }
        [Parameter("address", "QiAddress", 1)]
        public virtual string QiAddress { get; set; }
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

    public partial class PurchaseFunction : PurchaseFunctionBase { }

    [Function("Purchase")]
    public class PurchaseFunctionBase : FunctionMessage
    {
        [Parameter("address", "_t", 1)]
        public virtual string T { get; set; }
        [Parameter("uint256", "_a", 2)]
        public virtual BigInteger A { get; set; }
    }

    public partial class QiFunction : QiFunctionBase { }

    [Function("Qi", "address")]
    public class QiFunctionBase : FunctionMessage
    {

    }

    public partial class ReactFunction : ReactFunctionBase { }

    [Function("React", "uint256")]
    public class ReactFunctionBase : FunctionMessage
    {
        [Parameter("uint64", "Soul", 1)]
        public virtual ulong Soul { get; set; }
        [Parameter("uint256", "QingWaat", 2)]
        public virtual BigInteger QingWaat { get; set; }
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



    public partial class QiOutputDTO : QiOutputDTOBase { }

    [FunctionOutput]
    public class QiOutputDTOBase : IFunctionOutputDTO 
    {
        [Parameter("address", "", 1)]
        public virtual string ReturnValue1 { get; set; }
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
