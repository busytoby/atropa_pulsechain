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

namespace Dysnomia.Contracts.domain.assets.VITUS.ContractDefinition
{


    public partial class VitusDeployment : VitusDeploymentBase
    {
        public VitusDeployment() : base(BYTECODE) { }
        public VitusDeployment(string byteCode) : base(byteCode) { }
    }

    public class VitusDeploymentBase : ContractDeploymentMessage
    {
        public static string BYTECODE = "608060405234801561000f575f5ffd5b5060405161244538038061244583398101604081905261002e91610622565b604051806040016040528060118152602001704479736e6f6d69612043726561746f727360781b81525060405180604001604052806005815260200164564954555360d81b815250826001600160a01b0316636fb7329c6040518163ffffffff1660e01b8152600401602060405180830381865afa1580156100b2573d5f5f3e3d5ffd5b505050506040513d601f19601f820116820180604052508101906100d69190610622565b338061010857604051633603021960e21b81523260048201525f60248201523060448201526064015b60405180910390fd5b61011381600161042a565b50600461012084826106dc565b50600561012d83826106dc565b50600680546001600160a01b0319166001600160a01b0383169081179091556040805163604a6fa960e01b815290516201b207929163604a6fa991600480830192602092919082900301815f875af115801561018b573d5f5f3e3d5ffd5b505050506040513d601f19601f820116820180604052508101906101af9190610796565b6101b991906107bc565b50505f6007555050600980546001600160a01b0319166001600160a01b038316908117909155604080516351d9550760e01b815290516351d95507916004808201926020929091908290030181865afa158015610218573d5f5f3e3d5ffd5b505050506040513d601f19601f8201168201806040525081019061023c9190610622565b6001600160a01b0316636456147b6040518163ffffffff1660e01b8152600401602060405180830381865afa158015610277573d5f5f3e3d5ffd5b505050506040513d601f19601f8201168201806040525081019061029b9190610622565b6001600160a01b031663031d11f46040518163ffffffff1660e01b8152600401602060405180830381865afa1580156102d6573d5f5f3e3d5ffd5b505050506040513d601f19601f820116820180604052508101906102fa9190610622565b604051630e0cb96960e31b81523060048201526001600160a01b039190911690637065cb48906024015f604051808303815f87803b15801561033a575f5ffd5b505af115801561034c573d5f5f3e3d5ffd5b505050505f73a1bee1dae9af77dac73aa0459ed63b4d93fc6d2990506103d130826001600160a01b03166318160ddd6040518163ffffffff1660e01b8152600401602060405180830381865afa1580156103a8573d5f5f3e3d5ffd5b505050506040513d601f19601f820116820180604052508101906103cc91906107f6565b61047d565b61041b73a1bee1dae9af77dac73aa0459ed63b4d93fc6d296103f56012600a610906565b610400906001610914565b6001600160a01b039091165f90815260086020526040902055565b61042361048c565b505061093e565b6001600160a01b0382165f81815260208190526040808220805460ff191685151590811790915590519092917f6bfa7e860c3e9bf67b810cb88276b19b27c972812bac04793d369b59a1183a8391a35050565b6104885f83836104cb565b5050565b6104986012600a610906565b6007546104a59190610914565b60035410156104c9576104c9306104be6012600a610906565b6103cc906001610914565b565b6001600160a01b0383166104f5578060035f8282546104ea919061092b565b9091555061057f9050565b6001600160a01b0383165f908152600160205260409020548181101561056157604051630f7d0bbd60e21b81523260048201523360248201526001600160a01b0380861660448301528416606482015230608482015260a4810182905260c4810183905260e4016100ff565b6001600160a01b0384165f9081526001602052604090209082900390555b6001600160a01b03821661059b576003805482900390556105b9565b6001600160a01b0382165f9081526001602052604090208054820190555b816001600160a01b0316836001600160a01b03167fddf252ad1be2c89b69c2b068fc378daa952ba7f163c4a11628f55a4df523b3ef836040516105fe91815260200190565b60405180910390a3505050565b6001600160a01b038116811461061f575f5ffd5b50565b5f60208284031215610632575f5ffd5b815161063d8161060b565b9392505050565b634e487b7160e01b5f52604160045260245ffd5b600181811c9082168061066c57607f821691505b60208210810361068a57634e487b7160e01b5f52602260045260245ffd5b50919050565b601f8211156106d757805f5260205f20601f840160051c810160208510156106b55750805b601f840160051c820191505b818110156106d4575f81556001016106c1565b50505b505050565b81516001600160401b038111156106f5576106f5610644565b610709816107038454610658565b84610690565b6020601f82116001811461073b575f83156107245750848201515b5f19600385901b1c1916600184901b1784556106d4565b5f84815260208120601f198516915b8281101561076a578785015182556020948501946001909201910161074a565b508482101561078757868401515f19600387901b60f8161c191681555b50505050600190811b01905550565b5f602082840312156107a6575f5ffd5b81516001600160401b038116811461063d575f5ffd5b5f6001600160401b038316806107e057634e487b7160e01b5f52601260045260245ffd5b6001600160401b03929092169190910692915050565b5f60208284031215610806575f5ffd5b5051919050565b634e487b7160e01b5f52601160045260245ffd5b6001815b600184111561085c578085048111156108405761084061080d565b600184161561084e57908102905b60019390931c928002610825565b935093915050565b5f8261087257506001610900565b8161087e57505f610900565b8160018114610894576002811461089e576108ba565b6001915050610900565b60ff8411156108af576108af61080d565b50506001821b610900565b5060208310610133831016604e8410600b84101617156108dd575081810a610900565b6108e95f198484610821565b805f19048211156108fc576108fc61080d565b0290505b92915050565b5f61063d60ff841683610864565b80820281158282048414176109005761090061080d565b808201808211156109005761090061080d565b611afa8061094b5f395ff3fe608060405234801561000f575f5ffd5b5060043610610187575f3560e01c8063666e1b39116100d95780639dcbf7ea11610093578063d5abeb011161006e578063d5abeb0114610394578063dd62ed3e1461039d578063f4e6c93f146103d5578063ff762044146103dd575f5ffd5b80639dcbf7ea14610347578063a9059cbb1461035a578063d3dc2a0f1461036d575f5ffd5b8063666e1b39146102c05780636fb7329c146102eb5780637065cb48146102fe57806370a08231146103115780638da5cb5b1461033957806395d89b411461033f575f5ffd5b806323b872dd116101445780632d67bb911161011f5780632d67bb9114610260578063313ce5671461028b57806338bf3cfa1461029a5780635b6b431d146102ad575f5ffd5b806323b872dd146102125780632499a533146102255780632b83b1fc14610238575f5ffd5b806306fdde031461018b578063095ea7b3146101a95780630ef67887146101cc5780630f6798a5146101e257806318160ddd146101f7578063222838db146101ff575b5f5ffd5b610193610401565b6040516101a0919061147b565b60405180910390f35b6101bc6101b73660046114c4565b610491565b60405190151581526020016101a0565b6101d46104aa565b6040519081526020016101a0565b6101f56101f03660046114c4565b61064d565b005b6003546101d4565b6101f561020d3660046114c4565b610a03565b6101bc6102203660046114ee565b610c07565b6101f56102333660046114c4565b610c2a565b6101d461024636600461152c565b6001600160a01b03165f9081526008602052604090205490565b600954610273906001600160a01b031681565b6040516001600160a01b0390911681526020016101a0565b604051601281526020016101a0565b6101f56102a836600461152c565b610dec565b6101f56102bb36600461154e565b610e01565b6101bc6102ce36600461152c565b6001600160a01b03165f9081526020819052604090205460ff1690565b600654610273906001600160a01b031681565b6101f561030c36600461152c565b6110c5565b6101d461031f36600461152c565b6001600160a01b03165f9081526001602052604090205490565b30610273565b61019361110d565b6101f5610355366004611623565b61111c565b6101bc6103683660046114c4565b611142565b61037b6603632c8eb5af3b81565b60405167ffffffffffffffff90911681526020016101a0565b6101d460075481565b6101d46103ab366004611688565b6001600160a01b039182165f90815260026020908152604080832093909416825291909152205490565b6101f561114f565b61019360405180604001604052806005815260200164564954555360d81b81525081565b606060048054610410906116bf565b80601f016020809104026020016040519081016040528092919081815260200182805461043c906116bf565b80156104875780601f1061045e57610100808354040283529160200191610487565b820191905f5260205f20905b81548152906001019060200180831161046a57829003601f168201915b5050505050905090565b5f3361049e818585611161565b60019150505b92915050565b5f5f60095f9054906101000a90046001600160a01b03166001600160a01b03166351d955076040518163ffffffff1660e01b8152600401602060405180830381865afa1580156104fc573d5f5f3e3d5ffd5b505050506040513d601f19601f8201168201806040525081019061052091906116f7565b6001600160a01b0316636456147b6040518163ffffffff1660e01b8152600401602060405180830381865afa15801561055b573d5f5f3e3d5ffd5b505050506040513d601f19601f8201168201806040525081019061057f91906116f7565b6001600160a01b0316631cb77ea76040518163ffffffff1660e01b81526004016040805180830381865afa1580156105b9573d5f5f3e3d5ffd5b505050506040513d601f19601f820116820180604052508101906105dd9190611712565b506040516370a0823160e01b81526001600160a01b038216600482015290915030906370a0823190602401602060405180830381865afa158015610623573d5f5f3e3d5ffd5b505050506040513d601f19601f82011682018060405250810190610647919061173f565b91505090565b61065561116e565b61065f82826111c8565b6040516370a0823160e01b81526001600160a01b03831660048201525f9030906370a0823190602401602060405180830381865afa1580156106a3573d5f5f3e3d5ffd5b505050506040513d601f19601f820116820180604052508101906106c7919061173f565b90505f60095f9054906101000a90046001600160a01b03166001600160a01b031663ec557f406040518163ffffffff1660e01b8152600401602060405180830381865afa15801561071a573d5f5f3e3d5ffd5b505050506040513d601f19601f8201168201806040525081019061073e91906116f7565b6001600160a01b031663ec557f406040518163ffffffff1660e01b8152600401602060405180830381865afa158015610779573d5f5f3e3d5ffd5b505050506040513d601f19601f8201168201806040525081019061079d91906116f7565b604051634c2ef29160e01b8152600d60048201526001600160a01b039190911690634c2ef29190602401602060405180830381865afa1580156107e2573d5f5f3e3d5ffd5b505050506040513d601f19601f82011682018060405250810190610806919061173f565b9050808211156109fd576009546040516370a0823160e01b81526001600160a01b0390911660048201525f9030906370a0823190602401602060405180830381865afa158015610858573d5f5f3e3d5ffd5b505050506040513d601f19601f8201168201806040525081019061087c919061173f565b90505f60095f9054906101000a90046001600160a01b03166001600160a01b031663ec557f406040518163ffffffff1660e01b8152600401602060405180830381865afa1580156108cf573d5f5f3e3d5ffd5b505050506040513d601f19601f820116820180604052508101906108f391906116f7565b6001600160a01b031663ec557f406040518163ffffffff1660e01b8152600401602060405180830381865afa15801561092e573d5f5f3e3d5ffd5b505050506040513d601f19601f8201168201806040525081019061095291906116f7565b604051634c2ef29160e01b8152601460048201526001600160a01b039190911690634c2ef29190602401602060405180830381865afa158015610997573d5f5f3e3d5ffd5b505050506040513d601f19601f820116820180604052508101906109bb919061173f565b9050808210156109eb576009546109e69087906001600160a01b03166109e1868861176a565b6111d7565b6109fa565b6109fa865f6109e1868861176a565b50505b50505050565b6001600160a01b0382165f908152600860205260408120549003610a4a576040516390b2344160e01b81526001600160a01b03831660048201526024015b60405180910390fd5b815f610a586012600a611860565b6001600160a01b0385165f90815260086020526040902054610a7a908561186e565b610a849190611885565b6040516323b872dd60e01b81523360048201523060248201819052604482018690529192505f91906323b872dd906064016020604051808303815f875af1158015610ad1573d5f5f3e3d5ffd5b505050506040513d601f19601f82011682018060405250810190610af591906118a4565b905080306001600160a01b03166306fdde036040518163ffffffff1660e01b81526004015f60405180830381865afa158015610b33573d5f5f3e3d5ffd5b505050506040513d5f823e601f3d908101601f19168201604052610b5a91908101906118c3565b604051602001610b6a9190611938565b60405160208183030381529060405290610b975760405162461bcd60e51b8152600401610a41919061147b565b5060405163a9059cbb60e01b8152336004820152602481018390526001600160a01b0384169063a9059cbb906044015b6020604051808303815f875af1158015610be3573d5f5f3e3d5ffd5b505050506040513d601f19601f820116820180604052508101906109fa91906118a4565b5f33610c148582856111e2565b610c1f8585856111d7565b506001949350505050565b6001600160a01b0382165f908152600860205260408120549003610c6c576040516390b2344160e01b81526001600160a01b0383166004820152602401610a41565b815f610c7a6012600a611860565b6001600160a01b0385165f90815260086020526040902054610c9c908561186e565b610ca69190611885565b6040516323b872dd60e01b8152336004820152306024820152604481018290529091505f906001600160a01b038416906323b872dd906064016020604051808303815f875af1158015610cfb573d5f5f3e3d5ffd5b505050506040513d601f19601f82011682018060405250810190610d1f91906118a4565b905080836001600160a01b03166306fdde036040518163ffffffff1660e01b81526004015f60405180830381865afa158015610d5d573d5f5f3e3d5ffd5b505050506040513d5f823e601f3d908101601f19168201604052610d8491908101906118c3565b604051602001610d949190611938565b60405160208183030381529060405290610dc15760405162461bcd60e51b8152600401610a41919061147b565b5060405163a9059cbb60e01b815233600482015260248101859052309063a9059cbb90604401610bc7565b610df461116e565b610dfe815f61124b565b50565b600954604080516351d9550760e01b815290515f926001600160a01b0316916351d955079160048083019260209291908290030181865afa158015610e48573d5f5f3e3d5ffd5b505050506040513d601f19601f82011682018060405250810190610e6c91906116f7565b6001600160a01b0316636456147b6040518163ffffffff1660e01b8152600401602060405180830381865afa158015610ea7573d5f5f3e3d5ffd5b505050506040513d601f19601f82011682018060405250810190610ecb91906116f7565b6001600160a01b0316631cb77ea76040518163ffffffff1660e01b81526004016040805180830381865afa158015610f05573d5f5f3e3d5ffd5b505050506040513d601f19601f82011682018060405250810190610f299190611712565b50905060095f9054906101000a90046001600160a01b03166001600160a01b03166351d955076040518163ffffffff1660e01b8152600401602060405180830381865afa158015610f7c573d5f5f3e3d5ffd5b505050506040513d601f19601f82011682018060405250810190610fa091906116f7565b6001600160a01b0316636456147b6040518163ffffffff1660e01b8152600401602060405180830381865afa158015610fdb573d5f5f3e3d5ffd5b505050506040513d601f19601f82011682018060405250810190610fff91906116f7565b6001600160a01b031663031d11f46040518163ffffffff1660e01b8152600401602060405180830381865afa15801561103a573d5f5f3e3d5ffd5b505050506040513d601f19601f8201168201806040525081019061105e91906116f7565b60405163c4a3ab2760e01b81526001600160a01b03838116600483015230602483015233604483015260648201859052919091169063c4a3ab27906084015f604051808303815f87803b1580156110b3575f5ffd5b505af11580156109fa573d5f5f3e3d5ffd5b6110cd61116e565b6001600160a01b03811661110257604051633603021960e21b81523260048201525f6024820152306044820152606401610a41565b610dfe81600161124b565b606060058054610410906116bf565b61112461116e565b600461113083826119b3565b50600561113d82826119b3565b505050565b5f3361049e8185856111d7565b61115761116e565b61115f61129e565b565b61113d83838360016112e0565b335f9081526020819052604090205460ff1615801561119c5750325f9081526020819052604090205460ff16155b1561115f57604051630be6bab560e01b8152326004820152336024820152306044820152606401610a41565b6111d35f8383611360565b5050565b61113d838383611360565b6001600160a01b038381165f908152600260209081526040808320938616835292905220545f1981146109fd578181101561123d573233858530858760405162b83d1960e21b8152600401610a419796959493929190611a6e565b6109fd84848484035f6112e0565b6001600160a01b0382165f81815260208190526040808220805460ff191685151590811790915590519092917f6bfa7e860c3e9bf67b810cb88276b19b27c972812bac04793d369b59a1183a8391a35050565b6112aa6012600a611860565b6007546112b7919061186e565b600354101561115f5761115f306112d06012600a611860565b6112db90600161186e565b6111c8565b6001600160a01b038085165f90815260026020908152604080832093871683529290522082905580156109fd57826001600160a01b0316846001600160a01b03167f8c5be1e5ebec7d5bd14f71427d1e84f3dd0314c0f7b2291e5b200ac8c7c3b9258460405161135291815260200190565b60405180910390a350505050565b6001600160a01b03831661138a578060035f82825461137f9190611ab1565b909155506113ef9050565b6001600160a01b0383165f90815260016020526040902054818110156113d15732338585308587604051630f7d0bbd60e21b8152600401610a419796959493929190611a6e565b6001600160a01b0384165f9081526001602052604090209082900390555b6001600160a01b03821661140b57600380548290039055611429565b6001600160a01b0382165f9081526001602052604090208054820190555b816001600160a01b0316836001600160a01b03167fddf252ad1be2c89b69c2b068fc378daa952ba7f163c4a11628f55a4df523b3ef8360405161146e91815260200190565b60405180910390a3505050565b602081525f82518060208401528060208501604085015e5f604082850101526040601f19601f83011684010191505092915050565b6001600160a01b0381168114610dfe575f5ffd5b5f5f604083850312156114d5575f5ffd5b82356114e0816114b0565b946020939093013593505050565b5f5f5f60608486031215611500575f5ffd5b833561150b816114b0565b9250602084013561151b816114b0565b929592945050506040919091013590565b5f6020828403121561153c575f5ffd5b8135611547816114b0565b9392505050565b5f6020828403121561155e575f5ffd5b5035919050565b634e487b7160e01b5f52604160045260245ffd5b604051601f8201601f1916810167ffffffffffffffff811182821017156115a2576115a2611565565b604052919050565b5f67ffffffffffffffff8211156115c3576115c3611565565b50601f01601f191660200190565b5f82601f8301126115e0575f5ffd5b81356115f36115ee826115aa565b611579565b818152846020838601011115611607575f5ffd5b816020850160208301375f918101602001919091529392505050565b5f5f60408385031215611634575f5ffd5b823567ffffffffffffffff81111561164a575f5ffd5b611656858286016115d1565b925050602083013567ffffffffffffffff811115611672575f5ffd5b61167e858286016115d1565b9150509250929050565b5f5f60408385031215611699575f5ffd5b82356116a4816114b0565b915060208301356116b4816114b0565b809150509250929050565b600181811c908216806116d357607f821691505b6020821081036116f157634e487b7160e01b5f52602260045260245ffd5b50919050565b5f60208284031215611707575f5ffd5b8151611547816114b0565b5f5f60408385031215611723575f5ffd5b825161172e816114b0565b60208401519092506116b4816114b0565b5f6020828403121561174f575f5ffd5b5051919050565b634e487b7160e01b5f52601160045260245ffd5b818103818111156104a4576104a4611756565b6001815b60018411156117b85780850481111561179c5761179c611756565b60018416156117aa57908102905b60019390931c928002611781565b935093915050565b5f826117ce575060016104a4565b816117da57505f6104a4565b81600181146117f057600281146117fa57611816565b60019150506104a4565b60ff84111561180b5761180b611756565b50506001821b6104a4565b5060208310610133831016604e8410600b8410161715611839575081810a6104a4565b6118455f19848461177d565b805f190482111561185857611858611756565b029392505050565b5f61154760ff8416836117c0565b80820281158282048414176104a4576104a4611756565b5f8261189f57634e487b7160e01b5f52601260045260245ffd5b500490565b5f602082840312156118b4575f5ffd5b81518015158114611547575f5ffd5b5f602082840312156118d3575f5ffd5b815167ffffffffffffffff8111156118e9575f5ffd5b8201601f810184136118f9575f5ffd5b80516119076115ee826115aa565b81815285602083850101111561191b575f5ffd5b8160208401602083015e5f91810160200191909152949350505050565b6d02732b2b21020b8383937bb32b2160951b81525f82518060208501600e85015e5f9201600e0191825250919050565b601f82111561113d57805f5260205f20601f840160051c8101602085101561198d5750805b601f840160051c820191505b818110156119ac575f8155600101611999565b5050505050565b815167ffffffffffffffff8111156119cd576119cd611565565b6119e1816119db84546116bf565b84611968565b6020601f821160018114611a13575f83156119fc5750848201515b5f19600385901b1c1916600184901b1784556119ac565b5f84815260208120601f198516915b82811015611a425787850151825560209485019460019092019101611a22565b5084821015611a5f57868401515f19600387901b60f8161c191681555b50505050600190811b01905550565b6001600160a01b039788168152958716602087015293861660408601529185166060850152909316608083015260a082019290925260c081019190915260e00190565b808201808211156104a4576104a461175656fea264697066735822122040a0e415d21585fbb1a2b8d2982cc07d8a2b13f73e0f0688615d816403aa561564736f6c634300081c0033";
        public VitusDeploymentBase() : base(BYTECODE) { }
        public VitusDeploymentBase(string byteCode) : base(byteCode) { }
        [Parameter("address", "WorldAddress", 1)]
        public virtual string WorldAddress { get; set; }
    }

    public partial class BalanceFunction : BalanceFunctionBase { }

    [Function("Balance", "uint256")]
    public class BalanceFunctionBase : FunctionMessage
    {

    }

    public partial class GetMarketRateFunction : GetMarketRateFunctionBase { }

    [Function("GetMarketRate", "uint256")]
    public class GetMarketRateFunctionBase : FunctionMessage
    {
        [Parameter("address", "_a", 1)]
        public virtual string A { get; set; }
    }

    public partial class MintFunction : MintFunctionBase { }

    [Function("Mint")]
    public class MintFunctionBase : FunctionMessage
    {
        [Parameter("address", "To", 1)]
        public virtual string To { get; set; }
        [Parameter("uint256", "Amount", 2)]
        public virtual BigInteger Amount { get; set; }
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

    public partial class TypeFunction : TypeFunctionBase { }

    [Function("Type", "string")]
    public class TypeFunctionBase : FunctionMessage
    {

    }

    public partial class WithdrawFunction : WithdrawFunctionBase { }

    [Function("Withdraw")]
    public class WithdrawFunctionBase : FunctionMessage
    {
        [Parameter("uint256", "Amount", 1)]
        public virtual BigInteger Amount { get; set; }
    }

    public partial class WorldFunction : WorldFunctionBase { }

    [Function("World", "address")]
    public class WorldFunctionBase : FunctionMessage
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

    public partial class BalanceOutputDTO : BalanceOutputDTOBase { }

    [FunctionOutput]
    public class BalanceOutputDTOBase : IFunctionOutputDTO 
    {
        [Parameter("uint256", "", 1)]
        public virtual BigInteger ReturnValue1 { get; set; }
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







    public partial class TypeOutputDTO : TypeOutputDTOBase { }

    [FunctionOutput]
    public class TypeOutputDTOBase : IFunctionOutputDTO 
    {
        [Parameter("string", "", 1)]
        public virtual string ReturnValue1 { get; set; }
    }



    public partial class WorldOutputDTO : WorldOutputDTOBase { }

    [FunctionOutput]
    public class WorldOutputDTOBase : IFunctionOutputDTO 
    {
        [Parameter("address", "", 1)]
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
