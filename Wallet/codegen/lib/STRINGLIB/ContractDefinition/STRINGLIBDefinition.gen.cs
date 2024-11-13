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

namespace Dysnomia.Contracts.lib.STRINGLIB.ContractDefinition
{


    public partial class StringlibDeployment : StringlibDeploymentBase
    {
        public StringlibDeployment() : base(BYTECODE) { }
        public StringlibDeployment(string byteCode) : base(byteCode) { }
    }

    public class StringlibDeploymentBase : ContractDeploymentMessage
    {
        public static string BYTECODE = "608060405234801561000f575f5ffd5b5060405161266f38038061266f83398101604081905261002e9161056c565b6040518060400160405280601381526020017f4459534e4f4d4941204c6962537472696e6773000000000000000000000000008152506040518060400160405280600a8152602001694c6962537472696e677360b01b815250826001600160a01b0316636fb7329c6040518163ffffffff1660e01b8152600401602060405180830381865afa1580156100c3573d5f5f3e3d5ffd5b505050506040513d601f19601f820116820180604052508101906100e7919061056c565b338061011957604051633603021960e21b81523260048201525f60248201523060448201526064015b60405180910390fd5b6101248160016102c8565b5060046101318482610626565b50600561013e8382610626565b50600680546001600160a01b0319166001600160a01b0383169081179091556040805163604a6fa960e01b815290516201b207929163604a6fa991600480830192602092919082900301815f875af115801561019c573d5f5f3e3d5ffd5b505050506040513d601f19601f820116820180604052508101906101c091906106e0565b6101ca9190610706565b6001600160401b0316600755508291506101e590508161031b565b6101ee3261031b565b604080516339dd2ce960e21b815260048101919091526007604482015266737472696e677360c81b60648201523060248201526001600160a01b0382169063e774b3a4906084015f604051808303815f87803b15801561024c575f5ffd5b505af115801561025e573d5f5f3e3d5ffd5b505050506102b97324f0154c1dce548adf15da2098fdd8b8a3b8151d61028861036660201b60201c565b61029390600a610839565b61029e906001610847565b6001600160a01b039091165f90815260086020526040902055565b6102c161036b565b5050610871565b6001600160a01b0382165f81815260208190526040808220805460ff191685151590811790915590519092917f6bfa7e860c3e9bf67b810cb88276b19b27c972812bac04793d369b59a1183a8391a35050565b6103236103af565b6001600160a01b03811661035857604051633603021960e21b81523260048201525f6024820152306044820152606401610110565b6103638160016102c8565b50565b601290565b6103776012600a610839565b6007546103849190610847565b60035410156103ad576103ad3061039d6012600a610839565b6103a8906001610847565b610409565b565b335f9081526020819052604090205460ff161580156103dd5750325f9081526020819052604090205460ff16155b156103ad57604051630be6bab560e01b8152326004820152336024820152306044820152606401610110565b6104145f8383610418565b5050565b6001600160a01b038316610442578060035f828254610437919061085e565b909155506104cc9050565b6001600160a01b0383165f90815260016020526040902054818110156104ae57604051630f7d0bbd60e21b81523260048201523360248201526001600160a01b0380861660448301528416606482015230608482015260a4810182905260c4810183905260e401610110565b6001600160a01b0384165f9081526001602052604090209082900390555b6001600160a01b0382166104e857600380548290039055610506565b6001600160a01b0382165f9081526001602052604090208054820190555b816001600160a01b0316836001600160a01b03167fddf252ad1be2c89b69c2b068fc378daa952ba7f163c4a11628f55a4df523b3ef8360405161054b91815260200190565b60405180910390a3505050565b6001600160a01b0381168114610363575f5ffd5b5f6020828403121561057c575f5ffd5b815161058781610558565b9392505050565b634e487b7160e01b5f52604160045260245ffd5b600181811c908216806105b657607f821691505b6020821081036105d457634e487b7160e01b5f52602260045260245ffd5b50919050565b601f82111561062157805f5260205f20601f840160051c810160208510156105ff5750805b601f840160051c820191505b8181101561061e575f815560010161060b565b50505b505050565b81516001600160401b0381111561063f5761063f61058e565b6106538161064d84546105a2565b846105da565b6020601f821160018114610685575f831561066e5750848201515b5f19600385901b1c1916600184901b17845561061e565b5f84815260208120601f198516915b828110156106b45787850151825560209485019460019092019101610694565b50848210156106d157868401515f19600387901b60f8161c191681555b50505050600190811b01905550565b5f602082840312156106f0575f5ffd5b81516001600160401b0381168114610587575f5ffd5b5f6001600160401b0383168061072a57634e487b7160e01b5f52601260045260245ffd5b6001600160401b03929092169190910692915050565b634e487b7160e01b5f52601160045260245ffd5b6001815b600184111561078f5780850481111561077357610773610740565b600184161561078157908102905b60019390931c928002610758565b935093915050565b5f826107a557506001610833565b816107b157505f610833565b81600181146107c757600281146107d1576107ed565b6001915050610833565b60ff8411156107e2576107e2610740565b50506001821b610833565b5060208310610133831016604e8410600b8410161715610810575081810a610833565b61081c5f198484610754565b805f190482111561082f5761082f610740565b0290505b92915050565b5f61058760ff841683610797565b808202811582820484141761083357610833610740565b8082018082111561083357610833610740565b611df18061087e5f395ff3fe608060405234801561000f575f5ffd5b5060043610610213575f3560e01c80636fb7329c1161011f578063a9059cbb116100a9578063e05a627c11610079578063e05a627c146104d5578063e3cc97b3146104e8578063ebdae5f9146104fb578063f4e6c93f1461050e578063ff76204414610516575f5ffd5b8063a9059cbb1461045b578063d3dc2a0f1461046e578063d5abeb0114610494578063dd62ed3e1461049d575f5ffd5b806387f7ea3d116100ef57806387f7ea3d146104145780638da5cb5b1461042757806395d89b411461042d5780639dcbf7ea14610435578063a4a9556614610448575f5ffd5b80636fb7329c1461039b5780637065cb48146103c657806370a08231146103d95780637dbbd60f14610401575f5ffd5b8063313ce567116101a05780634637b7a2116101705780634637b7a21461032457806348ca15c114610337578063583079ba1461034a5780636560aee31461035d578063666e1b3914610370575f5ffd5b8063313ce567146102ef57806338bf3cfa146102fe5780633b6ca1871461031157806344ecc3f914610324575f5ffd5b8063222838db116101e6578063222838db1461027957806323b872dd1461028e5780632499a533146102a157806327fd3ae2146102b45780632b83b1fc146102c7575f5ffd5b806301158b0c1461021757806306fdde031461023f578063095ea7b31461025457806318160ddd14610267575b5f5ffd5b61022a61022536600461171d565b61053f565b60405190151581526020015b60405180910390f35b610247610553565b60405161023691906117ae565b61022a6102623660046117db565b6105e3565b6003545b604051908152602001610236565b61028c6102873660046117db565b6105fa565b005b61022a61029c366004611803565b610806565b61028c6102af3660046117db565b610829565b6102476102c236600461183d565b6109eb565b61026b6102d5366004611854565b6001600160a01b03165f9081526008602052604090205490565b60405160128152602001610236565b61028c61030c366004611854565b610a79565b61024761031f36600461186d565b610a8e565b61024761033236600461183d565b610b48565b61022a61034536600461186d565b610b74565b61022a61035836600461186d565b610b7a565b61022a61036b3660046118bd565b610c01565b61022a61037e366004611854565b6001600160a01b03165f9081526020819052604090205460ff1690565b6006546103ae906001600160a01b031681565b6040516001600160a01b039091168152602001610236565b61028c6103d4366004611854565b610c7b565b61026b6103e7366004611854565b6001600160a01b03165f9081526001602052604090205490565b61024761040f3660046118ee565b610cc3565b61022a61042236600461171d565b610efe565b306103ae565b61024761105d565b61028c61044336600461171d565b61106c565b61024761045636600461186d565b611092565b61022a6104693660046117db565b611299565b61047c6603632c8eb5af3b81565b6040516001600160401b039091168152602001610236565b61026b60075481565b61026b6104ab366004611915565b6001600160a01b039182165f90815260026020908152604080832093909416825291909152205490565b6102476104e3366004611854565b6112a6565b6102476104f636600461186d565b6112cf565b61026b61050936600461183d565b6112da565b61028c61132d565b6102476040518060400160405280600a8152602001694c6962537472696e677360b01b81525081565b5f61054a8383610efe565b90505b92915050565b6060600480546105629061193d565b80601f016020809104026020016040519081016040528092919081815260200182805461058e9061193d565b80156105d95780601f106105b0576101008083540402835291602001916105d9565b820191905f5260205f20905b8154815290600101906020018083116105bc57829003601f168201915b5050505050905090565b5f336105f081858561133f565b5060019392505050565b6001600160a01b0382165f908152600860205260408120549003610641576040516390b2344160e01b81526001600160a01b03831660048201526024015b60405180910390fd5b815f61064f6012600a611a6c565b6001600160a01b0385165f908152600860205260409020546106719085611a7a565b61067b9190611aa5565b6040516323b872dd60e01b81523360048201523060248201819052604482018690529192505f91906323b872dd906064016020604051808303815f875af11580156106c8573d5f5f3e3d5ffd5b505050506040513d601f19601f820116820180604052508101906106ec9190611ab8565b905080306001600160a01b03166306fdde036040518163ffffffff1660e01b81526004015f60405180830381865afa15801561072a573d5f5f3e3d5ffd5b505050506040513d5f823e601f3d908101601f191682016040526107519190810190611ad7565b6040516020016107619190611b4b565b6040516020818303038152906040529061078e5760405162461bcd60e51b815260040161063891906117ae565b5060405163a9059cbb60e01b8152336004820152602481018390526001600160a01b0384169063a9059cbb906044015b6020604051808303815f875af11580156107da573d5f5f3e3d5ffd5b505050506040513d601f19601f820116820180604052508101906107fe9190611ab8565b505050505050565b5f3361081385828561134c565b61081e8585856113bb565b506001949350505050565b6001600160a01b0382165f90815260086020526040812054900361086b576040516390b2344160e01b81526001600160a01b0383166004820152602401610638565b815f6108796012600a611a6c565b6001600160a01b0385165f9081526008602052604090205461089b9085611a7a565b6108a59190611aa5565b6040516323b872dd60e01b8152336004820152306024820152604481018290529091505f906001600160a01b038416906323b872dd906064016020604051808303815f875af11580156108fa573d5f5f3e3d5ffd5b505050506040513d601f19601f8201168201806040525081019061091e9190611ab8565b905080836001600160a01b03166306fdde036040518163ffffffff1660e01b81526004015f60405180830381865afa15801561095c573d5f5f3e3d5ffd5b505050506040513d5f823e601f3d908101601f191682016040526109839190810190611ad7565b6040516020016109939190611b4b565b604051602081830303815290604052906109c05760405162461bcd60e51b815260040161063891906117ae565b5060405163a9059cbb60e01b815233600482015260248101859052309063a9059cbb906044016107be565b60605f6109f7836112da565b6001019050806001600160401b03811115610a1457610a1461165f565b6040519080825280601f01601f191660200182016040528015610a3e576020820181803683370190505b5091508082016020015b5f19016f181899199a1a9b1b9c1cb0b131b232b360811b600a85061a8153600a8404935083610a48575b5050919050565b610a816113c6565b610a8b815f611420565b50565b606081516001600160401b03811115610aa957610aa961165f565b6040519080825280601f01601f191660200182016040528015610ad3576020820181803683370190505b5082519091505f5b8351811015610a72578381610aef81611b7b565b925081518110610b0157610b01611b93565b01602001516001600160f81b03191683610b1a84611ba7565b93508381518110610b2d57610b2d611b93565b60200101906001600160f81b03191690815f1a905350610adb565b606061054d82604051602001610b6091815260200190565b604051602081830303815290604052611092565b5f61054d825b5f5f610b8583610a8e565b83519091505f5b845181101561081e578481610ba081611b7b565b925081518110610bb257610bb2611b93565b01602001516001600160f81b03191683610bcb84611ba7565b93508381518110610bde57610bde611b93565b01602001516001600160f81b03191614610bfc57505f949350505050565b610b8c565b5f605a60f883901c11610c46576001600160f81b03198381169083161480610c3f5750610c3360f883901c6020611bbc565b60ff168360f81c60ff16145b905061054d565b6001600160f81b03198381169083161480610c3f575060f882811c90610c709085901c6020611bbc565b60ff1614905061054d565b610c836113c6565b6001600160a01b038116610cb857604051633603021960e21b81523260048201525f6024820152306044820152606401610638565b610a8b816001611420565b606060038260ff161015610cea57604051639564899b60e01b815260040160405180910390fd5b60408051808201909152601a81527f4142434445464748494a4b4c4d4e4f505152535455565758595a00000000000060208201525f610d2a600285611bd5565b60ff1660065f9054906101000a90046001600160a01b03166001600160a01b031663604a6fa96040518163ffffffff1660e01b81526004016020604051808303815f875af1158015610d7e573d5f5f3e3d5ffd5b505050506040513d601f19601f82011682018060405250810190610da29190611bee565b610dac9190611c14565b610db7906002611c41565b9050806001600160401b03166001600160401b03811115610dda57610dda61165f565b6040519080825280601f01601f191660200182016040528015610e04576020820181803683370190505b5092505f5b816001600160401b0316811015610ef5575f601a60065f9054906101000a90046001600160a01b03166001600160a01b031663604a6fa96040518163ffffffff1660e01b81526004016020604051808303815f875af1158015610e6e573d5f5f3e3d5ffd5b505050506040513d601f19601f82011682018060405250810190610e929190611bee565b610e9c9190611c14565b905083816001600160401b031681518110610eb957610eb9611b93565b602001015160f81c60f81b858381518110610ed657610ed6611b93565b60200101906001600160f81b03191690815f1a90535050600101610e09565b50610a72611473565b81515f9082901580610f0f57508051155b15610f1d575f91505061054d565b610f63815f81518110610f3257610f32611b93565b602001015160f81c60f81b855f81518110610f4f57610f4f611b93565b01602001516001600160f81b031916610c01565b610f70575f91505061054d565b6001805b825181101561104757828181518110610f8f57610f8f611b93565b0160209081015160f81c900361101e5785518260ff1610610fb5575f935050505061054d565b610ff483610fc4836001611c60565b81518110610fd457610fd4611b93565b602001015160f81c60f81b878460ff1681518110610f4f57610f4f611b93565b611003575f935050505061054d565b61100e826001611bbc565b915061101b816001611c60565b90505b85518260ff161115611035575f935050505061054d565b8061103f81611b7b565b915050610f74565b5084518160ff161461081e575f9250505061054d565b6060600580546105629061193d565b6110746113c6565b60046110808382611cbe565b50600561108d8282611cbe565b505050565b60408051808201909152601081526f181899199a1a9b1b9c1cb0b131b232b360811b60208201528151606091905f906110cc906002611a7a565b6110d7906002611c60565b6001600160401b038111156110ee576110ee61165f565b6040519080825280601f01601f191660200182016040528015611118576020820181803683370190505b509050600360fc1b815f8151811061113257611132611b93565b60200101906001600160f81b03191690815f1a905350600f60fb1b8160018151811061116057611160611b93565b60200101906001600160f81b03191690815f1a9053505f5b84518110156112915782600486838151811061119657611196611b93565b016020015182516001600160f81b031990911690911c60f81c9081106111be576111be611b93565b01602001516001600160f81b031916826111d9836002611a7a565b6111e4906002611c60565b815181106111f4576111f4611b93565b60200101906001600160f81b03191690815f1a9053508285828151811061121d5761121d611b93565b602091010151815160f89190911c600f1690811061123d5761123d611b93565b01602001516001600160f81b03191682611258836002611a7a565b611263906003611c60565b8151811061127357611273611b93565b60200101906001600160f81b03191690815f1a905350600101611178565b509392505050565b5f336105f08185856113bb565b604051606082811b6bffffffffffffffffffffffff191660208301529061054d90603401610b60565b606061054d82610a8e565b5f8060405b60018160ff1610611326578060ff16600a0a8410611318578060ff16600a0a848161130c5761130c611a91565b0493508060ff16820191505b600260ff82160490506112df565b5092915050565b6113356113c6565b61133d611473565b565b61108d83838360016114b5565b6001600160a01b038381165f908152600260209081526040808320938616835292905220545f1981146113b557818110156113a7573233858530858760405162b83d1960e21b81526004016106389796959493929190611d78565b6113b584848484035f6114b5565b50505050565b61108d838383611535565b335f9081526020819052604090205460ff161580156113f45750325f9081526020819052604090205460ff16155b1561133d57604051630be6bab560e01b8152326004820152336024820152306044820152606401610638565b6001600160a01b0382165f81815260208190526040808220805460ff191685151590811790915590519092917f6bfa7e860c3e9bf67b810cb88276b19b27c972812bac04793d369b59a1183a8391a35050565b61147f6012600a611a6c565b60075461148c9190611a7a565b600354101561133d5761133d306114a56012600a611a6c565b6114b0906001611a7a565b611650565b6001600160a01b038085165f90815260026020908152604080832093871683529290522082905580156113b557826001600160a01b0316846001600160a01b03167f8c5be1e5ebec7d5bd14f71427d1e84f3dd0314c0f7b2291e5b200ac8c7c3b9258460405161152791815260200190565b60405180910390a350505050565b6001600160a01b03831661155f578060035f8282546115549190611c60565b909155506115c49050565b6001600160a01b0383165f90815260016020526040902054818110156115a65732338585308587604051630f7d0bbd60e21b81526004016106389796959493929190611d78565b6001600160a01b0384165f9081526001602052604090209082900390555b6001600160a01b0382166115e0576003805482900390556115fe565b6001600160a01b0382165f9081526001602052604090208054820190555b816001600160a01b0316836001600160a01b03167fddf252ad1be2c89b69c2b068fc378daa952ba7f163c4a11628f55a4df523b3ef8360405161164391815260200190565b60405180910390a3505050565b61165b5f8383611535565b5050565b634e487b7160e01b5f52604160045260245ffd5b604051601f8201601f191681016001600160401b038111828210171561169b5761169b61165f565b604052919050565b5f6001600160401b038211156116bb576116bb61165f565b50601f01601f191660200190565b5f82601f8301126116d8575f5ffd5b8135602083015f6116f06116eb846116a3565b611673565b9050828152858383011115611703575f5ffd5b828260208301375f92810160200192909252509392505050565b5f5f6040838503121561172e575f5ffd5b82356001600160401b03811115611743575f5ffd5b61174f858286016116c9565b92505060208301356001600160401b0381111561176a575f5ffd5b611776858286016116c9565b9150509250929050565b5f81518084528060208401602086015e5f602082860101526020601f19601f83011685010191505092915050565b602081525f61054a6020830184611780565b80356001600160a01b03811681146117d6575f5ffd5b919050565b5f5f604083850312156117ec575f5ffd5b6117f5836117c0565b946020939093013593505050565b5f5f5f60608486031215611815575f5ffd5b61181e846117c0565b925061182c602085016117c0565b929592945050506040919091013590565b5f6020828403121561184d575f5ffd5b5035919050565b5f60208284031215611864575f5ffd5b61054a826117c0565b5f6020828403121561187d575f5ffd5b81356001600160401b03811115611892575f5ffd5b61189e848285016116c9565b949350505050565b80356001600160f81b0319811681146117d6575f5ffd5b5f5f604083850312156118ce575f5ffd5b6118d7836118a6565b91506118e5602084016118a6565b90509250929050565b5f602082840312156118fe575f5ffd5b813560ff8116811461190e575f5ffd5b9392505050565b5f5f60408385031215611926575f5ffd5b61192f836117c0565b91506118e5602084016117c0565b600181811c9082168061195157607f821691505b60208210810361196f57634e487b7160e01b5f52602260045260245ffd5b50919050565b634e487b7160e01b5f52601160045260245ffd5b6001815b60018411156119c4578085048111156119a8576119a8611975565b60018416156119b657908102905b60019390931c92800261198d565b935093915050565b5f826119da5750600161054d565b816119e657505f61054d565b81600181146119fc5760028114611a0657611a22565b600191505061054d565b60ff841115611a1757611a17611975565b50506001821b61054d565b5060208310610133831016604e8410600b8410161715611a45575081810a61054d565b611a515f198484611989565b805f1904821115611a6457611a64611975565b029392505050565b5f61054a60ff8416836119cc565b808202811582820484141761054d5761054d611975565b634e487b7160e01b5f52601260045260245ffd5b5f82611ab357611ab3611a91565b500490565b5f60208284031215611ac8575f5ffd5b8151801515811461190e575f5ffd5b5f60208284031215611ae7575f5ffd5b81516001600160401b03811115611afc575f5ffd5b8201601f81018413611b0c575f5ffd5b8051611b1a6116eb826116a3565b818152856020838501011115611b2e575f5ffd5b8160208401602083015e5f91810160200191909152949350505050565b6d02732b2b21020b8383937bb32b2160951b81525f82518060208501600e85015e5f9201600e0191825250919050565b5f60018201611b8c57611b8c611975565b5060010190565b634e487b7160e01b5f52603260045260245ffd5b5f81611bb557611bb5611975565b505f190190565b60ff818116838216019081111561054d5761054d611975565b60ff828116828216039081111561054d5761054d611975565b5f60208284031215611bfe575f5ffd5b81516001600160401b038116811461190e575f5ffd5b5f6001600160401b03831680611c2c57611c2c611a91565b806001600160401b0384160691505092915050565b6001600160401b03818116838216019081111561054d5761054d611975565b8082018082111561054d5761054d611975565b601f82111561108d57805f5260205f20601f840160051c81016020851015611c985750805b601f840160051c820191505b81811015611cb7575f8155600101611ca4565b5050505050565b81516001600160401b03811115611cd757611cd761165f565b611ceb81611ce5845461193d565b84611c73565b6020601f821160018114611d1d575f8315611d065750848201515b5f19600385901b1c1916600184901b178455611cb7565b5f84815260208120601f198516915b82811015611d4c5787850151825560209485019460019092019101611d2c565b5084821015611d6957868401515f19600387901b60f8161c191681555b50505050600190811b01905550565b6001600160a01b039788168152958716602087015293861660408601529185166060850152909316608083015260a082019290925260c081019190915260e0019056fea26469706673582212207e919f47fb2ea4a92a6ac48dfc0bfe0eca25be418b65d82bb264c25b341da0a764736f6c634300081c0033";
        public StringlibDeploymentBase() : base(BYTECODE) { }
        public StringlibDeploymentBase(string byteCode) : base(byteCode) { }
        [Parameter("address", "VoidAddress", 1)]
        public virtual string VoidAddress { get; set; }
    }

    public partial class CaseInsensitiveCompareFunction : CaseInsensitiveCompareFunctionBase { }

    [Function("CaseInsensitiveCompare", "bool")]
    public class CaseInsensitiveCompareFunctionBase : FunctionMessage
    {
        [Parameter("bytes1", "A", 1)]
        public virtual byte[] A { get; set; }
        [Parameter("bytes1", "B", 2)]
        public virtual byte[] B { get; set; }
    }

    public partial class CheckAcronymFunction : CheckAcronymFunctionBase { }

    [Function("CheckAcronym", "bool")]
    public class CheckAcronymFunctionBase : FunctionMessage
    {
        [Parameter("string", "_A", 1)]
        public virtual string A { get; set; }
        [Parameter("string", "_B", 2)]
        public virtual string B { get; set; }
    }

    public partial class CheckAcronym1Function : CheckAcronym1FunctionBase { }

    [Function("CheckAcronym", "bool")]
    public class CheckAcronym1FunctionBase : FunctionMessage
    {
        [Parameter("bytes", "_acronym", 1)]
        public virtual byte[] Acronym { get; set; }
        [Parameter("string", "_Beta", 2)]
        public virtual string Beta { get; set; }
    }

    public partial class CheckPalindromeFunction : CheckPalindromeFunctionBase { }

    [Function("CheckPalindrome", "bool")]
    public class CheckPalindromeFunctionBase : FunctionMessage
    {
        [Parameter("string", "S", 1)]
        public virtual string S { get; set; }
    }

    public partial class CheckPalindrome1Function : CheckPalindrome1FunctionBase { }

    [Function("CheckPalindrome", "bool")]
    public class CheckPalindrome1FunctionBase : FunctionMessage
    {
        [Parameter("bytes", "S", 1)]
        public virtual byte[] S { get; set; }
    }

    public partial class GetMarketRateFunction : GetMarketRateFunctionBase { }

    [Function("GetMarketRate", "uint256")]
    public class GetMarketRateFunctionBase : FunctionMessage
    {
        [Parameter("address", "_a", 1)]
        public virtual string A { get; set; }
    }

    public partial class HexFunction : HexFunctionBase { }

    [Function("Hex", "string")]
    public class HexFunctionBase : FunctionMessage
    {
        [Parameter("bytes32", "value", 1)]
        public virtual byte[] Value { get; set; }
    }

    public partial class Hex1Function : Hex1FunctionBase { }

    [Function("Hex", "string")]
    public class Hex1FunctionBase : FunctionMessage
    {
        [Parameter("uint256", "value", 1)]
        public virtual BigInteger Value { get; set; }
    }

    public partial class Hex2Function : Hex2FunctionBase { }

    [Function("Hex", "string")]
    public class Hex2FunctionBase : FunctionMessage
    {
        [Parameter("bytes", "data", 1)]
        public virtual byte[] Data { get; set; }
    }

    public partial class Hex3Function : Hex3FunctionBase { }

    [Function("Hex", "string")]
    public class Hex3FunctionBase : FunctionMessage
    {
        [Parameter("address", "account", 1)]
        public virtual string Account { get; set; }
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

    public partial class RandomAcronymFunction : RandomAcronymFunctionBase { }

    [Function("RandomAcronym", "bytes")]
    public class RandomAcronymFunctionBase : FunctionMessage
    {
        [Parameter("uint8", "MaxLength", 1)]
        public virtual byte MaxLength { get; set; }
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

    public partial class ReverseFunction : ReverseFunctionBase { }

    [Function("Reverse", "bytes")]
    public class ReverseFunctionBase : FunctionMessage
    {
        [Parameter("bytes", "S", 1)]
        public virtual byte[] S { get; set; }
    }

    public partial class Reverse1Function : Reverse1FunctionBase { }

    [Function("Reverse", "string")]
    public class Reverse1FunctionBase : FunctionMessage
    {
        [Parameter("string", "S", 1)]
        public virtual string S { get; set; }
    }

    public partial class StringFunction : StringFunctionBase { }

    [Function("String", "string")]
    public class StringFunctionBase : FunctionMessage
    {
        [Parameter("uint256", "value", 1)]
        public virtual BigInteger Value { get; set; }
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

    public partial class Log10Function : Log10FunctionBase { }

    [Function("log10", "uint256")]
    public class Log10FunctionBase : FunctionMessage
    {
        [Parameter("uint256", "value", 1)]
        public virtual BigInteger Value { get; set; }
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

    public partial class MinimumLength3Error : MinimumLength3ErrorBase { }
    [Error("MinimumLength3")]
    public class MinimumLength3ErrorBase : IErrorDTO
    {
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

    public partial class CaseInsensitiveCompareOutputDTO : CaseInsensitiveCompareOutputDTOBase { }

    [FunctionOutput]
    public class CaseInsensitiveCompareOutputDTOBase : IFunctionOutputDTO 
    {
        [Parameter("bool", "", 1)]
        public virtual bool ReturnValue1 { get; set; }
    }

    public partial class CheckAcronymOutputDTO : CheckAcronymOutputDTOBase { }

    [FunctionOutput]
    public class CheckAcronymOutputDTOBase : IFunctionOutputDTO 
    {
        [Parameter("bool", "", 1)]
        public virtual bool ReturnValue1 { get; set; }
    }

    public partial class CheckAcronym1OutputDTO : CheckAcronym1OutputDTOBase { }

    [FunctionOutput]
    public class CheckAcronym1OutputDTOBase : IFunctionOutputDTO 
    {
        [Parameter("bool", "", 1)]
        public virtual bool ReturnValue1 { get; set; }
    }

    public partial class CheckPalindromeOutputDTO : CheckPalindromeOutputDTOBase { }

    [FunctionOutput]
    public class CheckPalindromeOutputDTOBase : IFunctionOutputDTO 
    {
        [Parameter("bool", "", 1)]
        public virtual bool ReturnValue1 { get; set; }
    }

    public partial class CheckPalindrome1OutputDTO : CheckPalindrome1OutputDTOBase { }

    [FunctionOutput]
    public class CheckPalindrome1OutputDTOBase : IFunctionOutputDTO 
    {
        [Parameter("bool", "", 1)]
        public virtual bool ReturnValue1 { get; set; }
    }

    public partial class GetMarketRateOutputDTO : GetMarketRateOutputDTOBase { }

    [FunctionOutput]
    public class GetMarketRateOutputDTOBase : IFunctionOutputDTO 
    {
        [Parameter("uint256", "", 1)]
        public virtual BigInteger ReturnValue1 { get; set; }
    }

    public partial class HexOutputDTO : HexOutputDTOBase { }

    [FunctionOutput]
    public class HexOutputDTOBase : IFunctionOutputDTO 
    {
        [Parameter("string", "", 1)]
        public virtual string ReturnValue1 { get; set; }
    }

    public partial class Hex1OutputDTO : Hex1OutputDTOBase { }

    [FunctionOutput]
    public class Hex1OutputDTOBase : IFunctionOutputDTO 
    {
        [Parameter("string", "", 1)]
        public virtual string ReturnValue1 { get; set; }
    }

    public partial class Hex2OutputDTO : Hex2OutputDTOBase { }

    [FunctionOutput]
    public class Hex2OutputDTOBase : IFunctionOutputDTO 
    {
        [Parameter("string", "", 1)]
        public virtual string ReturnValue1 { get; set; }
    }

    public partial class Hex3OutputDTO : Hex3OutputDTOBase { }

    [FunctionOutput]
    public class Hex3OutputDTOBase : IFunctionOutputDTO 
    {
        [Parameter("string", "", 1)]
        public virtual string ReturnValue1 { get; set; }
    }

    public partial class MotzkinPrimeOutputDTO : MotzkinPrimeOutputDTOBase { }

    [FunctionOutput]
    public class MotzkinPrimeOutputDTOBase : IFunctionOutputDTO 
    {
        [Parameter("uint64", "", 1)]
        public virtual ulong ReturnValue1 { get; set; }
    }









    public partial class ReverseOutputDTO : ReverseOutputDTOBase { }

    [FunctionOutput]
    public class ReverseOutputDTOBase : IFunctionOutputDTO 
    {
        [Parameter("bytes", "Reversed", 1)]
        public virtual byte[] Reversed { get; set; }
    }

    public partial class Reverse1OutputDTO : Reverse1OutputDTOBase { }

    [FunctionOutput]
    public class Reverse1OutputDTOBase : IFunctionOutputDTO 
    {
        [Parameter("string", "Reversed", 1)]
        public virtual string Reversed { get; set; }
    }

    public partial class StringOutputDTO : StringOutputDTOBase { }

    [FunctionOutput]
    public class StringOutputDTOBase : IFunctionOutputDTO 
    {
        [Parameter("string", "buffer", 1)]
        public virtual string Buffer { get; set; }
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

    public partial class Log10OutputDTO : Log10OutputDTOBase { }

    [FunctionOutput]
    public class Log10OutputDTOBase : IFunctionOutputDTO 
    {
        [Parameter("uint256", "", 1)]
        public virtual BigInteger ReturnValue1 { get; set; }
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
