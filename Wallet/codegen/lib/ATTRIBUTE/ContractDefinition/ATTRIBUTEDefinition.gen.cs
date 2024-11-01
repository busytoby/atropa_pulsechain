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

namespace Dysnomia.Contracts.lib.ATTRIBUTE.ContractDefinition
{


    public partial class AttributeDeployment : AttributeDeploymentBase
    {
        public AttributeDeployment() : base(BYTECODE) { }
        public AttributeDeployment(string byteCode) : base(byteCode) { }
    }

    public class AttributeDeploymentBase : ContractDeploymentMessage
    {
        public static string BYTECODE = "608060405234801561000f575f5ffd5b5060405161256e38038061256e83398101604081905261002e916106d9565b6040518060400160405280601481526020017f434841544c4f47204c69624174747269627574650000000000000000000000008152506040518060400160405280600c81526020016b4c696241747472696275746560a01b815250826001600160a01b0316636fb7329c6040518163ffffffff1660e01b8152600401602060405180830381865afa1580156100c5573d5f5f3e3d5ffd5b505050506040513d601f19601f820116820180604052508101906100e991906106d9565b338061011b57604051633603021960e21b81523260048201525f60248201523060448201526064015b60405180910390fd5b61012681600161040f565b5060046101338482610793565b5060056101408382610793565b50600680546001600160a01b0319166001600160a01b0383169081179091556040805163604a6fa960e01b815290516201b207929163604a6fa991600480830192602092919082900301815f875af115801561019e573d5f5f3e3d5ffd5b505050506040513d601f19601f820116820180604052508101906101c2919061084d565b6101cc9190610887565b6001600160401b03166007819055505f600a60075460065f9054906101000a90046001600160a01b03166001600160a01b031663604a6fa96040518163ffffffff1660e01b81526004016020604051808303815f875af1158015610232573d5f5f3e3d5ffd5b505050506040513d601f19601f82011682018060405250810190610256919061084d565b6001600160401b031661026991906108b5565b61027391906108dc565b9050610294326102856012600a6109d4565b61028f90846109e2565b610462565b6102de7324f0154c1dce548adf15da2098fdd8b8a3b8151d6102b86012600a6109d4565b6102c39060016109e2565b6001600160a01b039091165f90815260086020526040902055565b5050600980546001600160a01b0319166001600160a01b03851617905550610307905081610471565b600954604080516339dd2ce960e21b81526004810191909152600c60448201526b6c696261747472696275746560a01b60648201523060248201526001600160a01b039091169063e774b3a4906084015f604051808303815f87803b15801561036e575f5ffd5b505af1158015610380573d5f5f3e3d5ffd5b505050506103b560405180604001604052806008815260200167557365726e616d6560c01b815250600c6104bc60201b60201c565b60408051808201909152600d81526c5465737441747472696275746560981b60208201526103e490608c6104bc565b6103ec6104fc565b6104093260125b6103fe90600a6109d4565b61028f9060016109e2565b50610a22565b6001600160a01b0382165f81815260208190526040808220805460ff191685151590811790915590519092917f6bfa7e860c3e9bf67b810cb88276b19b27c972812bac04793d369b59a1183a8391a35050565b61046d5f838361052b565b5050565b61047961066b565b6001600160a01b0381166104ae57604051633603021960e21b81523260048201525f6024820152306044820152606401610112565b6104b981600161040f565b50565b6104c461066b565b80600a836040516104d591906109f9565b908152604051908190036020019020805460ff9290921660ff1990921691909117905561046d5b6105086012600a6109d4565b60075461051591906109e2565b6003541015610529576105293060126103f3565b565b6001600160a01b038316610555578060035f82825461054a9190610a0f565b909155506105df9050565b6001600160a01b0383165f90815260016020526040902054818110156105c157604051630f7d0bbd60e21b81523260048201523360248201526001600160a01b0380861660448301528416606482015230608482015260a4810182905260c4810183905260e401610112565b6001600160a01b0384165f9081526001602052604090209082900390555b6001600160a01b0382166105fb57600380548290039055610619565b6001600160a01b0382165f9081526001602052604090208054820190555b816001600160a01b0316836001600160a01b03167fddf252ad1be2c89b69c2b068fc378daa952ba7f163c4a11628f55a4df523b3ef8360405161065e91815260200190565b60405180910390a3505050565b335f9081526020819052604090205460ff161580156106995750325f9081526020819052604090205460ff16155b1561052957604051630be6bab560e01b8152326004820152336024820152306044820152606401610112565b6001600160a01b03811681146104b9575f5ffd5b5f602082840312156106e9575f5ffd5b81516106f4816106c5565b9392505050565b634e487b7160e01b5f52604160045260245ffd5b600181811c9082168061072357607f821691505b60208210810361074157634e487b7160e01b5f52602260045260245ffd5b50919050565b601f82111561078e57805f5260205f20601f840160051c8101602085101561076c5750805b601f840160051c820191505b8181101561078b575f8155600101610778565b50505b505050565b81516001600160401b038111156107ac576107ac6106fb565b6107c0816107ba845461070f565b84610747565b6020601f8211600181146107f2575f83156107db5750848201515b5f19600385901b1c1916600184901b17845561078b565b5f84815260208120601f198516915b828110156108215787850151825560209485019460019092019101610801565b508482101561083e57868401515f19600387901b60f8161c191681555b50505050600190811b01905550565b5f6020828403121561085d575f5ffd5b81516001600160401b03811681146106f4575f5ffd5b634e487b7160e01b5f52601260045260245ffd5b5f6001600160401b0383168061089f5761089f610873565b6001600160401b03929092169190910692915050565b5f826108c3576108c3610873565b500690565b634e487b7160e01b5f52601160045260245ffd5b5f826108ea576108ea610873565b500490565b6001815b600184111561092a5780850481111561090e5761090e6108c8565b600184161561091c57908102905b60019390931c9280026108f3565b935093915050565b5f82610940575060016109ce565b8161094c57505f6109ce565b8160018114610962576002811461096c57610988565b60019150506109ce565b60ff84111561097d5761097d6108c8565b50506001821b6109ce565b5060208310610133831016604e8410600b84101617156109ab575081810a6109ce565b6109b75f1984846108ef565b805f19048211156109ca576109ca6108c8565b0290505b92915050565b5f6106f460ff841683610932565b80820281158282048414176109ce576109ce6108c8565b5f82518060208501845e5f920191825250919050565b808201808211156109ce576109ce6108c8565b611b3f80610a2f5f395ff3fe608060405234801561000f575f5ffd5b50600436106101dc575f3560e01c80637bde874311610109578063d25f5a5d1161009e578063ec2542231161006e578063ec25422314610469578063f4e6c93f1461047c578063f4f6c22c14610484578063ff76204414610497575f5ffd5b8063d25f5a5d146103ef578063d3dc2a0f14610402578063d5abeb0114610428578063dd62ed3e14610431575f5ffd5b80639dcbf7ea116100d95780639dcbf7ea146103a3578063a9059cbb146103b6578063aafd31cf146103c9578063be193345146103dc575f5ffd5b80637bde87431461036f5780638da5cb5b146103825780639139ab2c1461038857806395d89b411461039b575f5ffd5b80632b83b1fc1161017f578063666e1b391161014f578063666e1b39146102de5780636fb7329c146103095780637065cb481461033457806370a0823114610347575f5ffd5b80632b83b1fc14610281578063313ce567146102a957806337714208146102b857806338bf3cfa146102cb575f5ffd5b806318160ddd116101ba57806318160ddd14610234578063222838db1461024657806323b872dd1461025b5780632499a5331461026e575f5ffd5b806306be4133146101e057806306fdde0314610209578063095ea7b314610211575b5f5ffd5b6101f36101ee36600461135e565b6104c2565b60405161020091906113bf565b60405180910390f35b6101f36105bd565b61022461021f3660046113d8565b61064d565b6040519015158152602001610200565b6003545b604051908152602001610200565b6102596102543660046113d8565b610664565b005b610224610269366004611402565b61086b565b61025961027c3660046113d8565b61088e565b61023861028f366004611440565b6001600160a01b03165f9081526008602052604090205490565b60405160128152602001610200565b6102596102c63660046114d3565b610a50565b6102596102d9366004611440565b610a97565b6102246102ec366004611440565b6001600160a01b03165f9081526020819052604090205460ff1690565b60065461031c906001600160a01b031681565b6040516001600160a01b039091168152602001610200565b610259610342366004611440565b610aa9565b610238610355366004611440565b6001600160a01b03165f9081526001602052604090205490565b61025961037d36600461150c565b610af1565b3061031c565b6101f361039636600461157e565b610bb8565b6101f3610ca4565b6102596103b13660046115b3565b610cb3565b6102246103c43660046113d8565b610cd4565b6102596103d7366004611616565b610ce1565b6102596103ea36600461165c565b610d49565b6102596103fd3660046116a4565b610d91565b6104106603632c8eb5af3b81565b6040516001600160401b039091168152602001610200565b61023860075481565b61023861043f3660046116e9565b6001600160a01b039182165f90815260026020908152604080832093909416825291909152205490565b6101f3610477366004611705565b610df8565b610259610ee4565b60095461031c906001600160a01b031681565b6101f36040518060400160405280600c81526020016b4c696241747472696275746560a01b81525081565b60606104cc610ef6565b6001600160401b0383165f908152600c6020908152604080832085516001600160a01b031684529091529020805461050390611739565b80601f016020809104026020016040519081016040528092919081815260200182805461052f90611739565b801561057a5780601f106105515761010080835404028352916020019161057a565b820191905f5260205f20905b81548152906001019060200180831161055d57829003601f168201915b505050505090506001815110156105b7578151604051635f2c73e560e01b81526001600160a01b0390911660048201526024015b60405180910390fd5b92915050565b6060600480546105cc90611739565b80601f01602080910402602001604051908101604052809291908181526020018280546105f890611739565b80156106435780601f1061061a57610100808354040283529160200191610643565b820191905f5260205f20905b81548152906001019060200180831161062657829003601f168201915b5050505050905090565b5f3361065a818585610f50565b5060019392505050565b6001600160a01b0382165f9081526008602052604081205490036106a6576040516390b2344160e01b81526001600160a01b03831660048201526024016105ae565b815f6106b46012600a611868565b6001600160a01b0385165f908152600860205260409020546106d69085611876565b6106e0919061188d565b6040516323b872dd60e01b81523360048201523060248201819052604482018690529192505f91906323b872dd906064016020604051808303815f875af115801561072d573d5f5f3e3d5ffd5b505050506040513d601f19601f8201168201806040525081019061075191906118ac565b905080306001600160a01b03166306fdde036040518163ffffffff1660e01b81526004015f60405180830381865afa15801561078f573d5f5f3e3d5ffd5b505050506040513d5f823e601f3d908101601f191682016040526107b691908101906118cb565b6040516020016107c69190611956565b604051602081830303815290604052906107f35760405162461bcd60e51b81526004016105ae91906113bf565b5060405163a9059cbb60e01b8152336004820152602481018390526001600160a01b0384169063a9059cbb906044015b6020604051808303815f875af115801561083f573d5f5f3e3d5ffd5b505050506040513d601f19601f8201168201806040525081019061086391906118ac565b505050505050565b5f33610878858285610f5d565b610883858585610fcc565b506001949350505050565b6001600160a01b0382165f9081526008602052604081205490036108d0576040516390b2344160e01b81526001600160a01b03831660048201526024016105ae565b815f6108de6012600a611868565b6001600160a01b0385165f908152600860205260409020546109009085611876565b61090a919061188d565b6040516323b872dd60e01b8152336004820152306024820152604481018290529091505f906001600160a01b038416906323b872dd906064016020604051808303815f875af115801561095f573d5f5f3e3d5ffd5b505050506040513d601f19601f8201168201806040525081019061098391906118ac565b905080836001600160a01b03166306fdde036040518163ffffffff1660e01b81526004015f60405180830381865afa1580156109c1573d5f5f3e3d5ffd5b505050506040513d5f823e601f3d908101601f191682016040526109e891908101906118cb565b6040516020016109f89190611956565b60405160208183030381529060405290610a255760405162461bcd60e51b81526004016105ae91906113bf565b5060405163a9059cbb60e01b815233600482015260248101859052309063a9059cbb90604401610823565b610a58610ef6565b5f600a82604051610a699190611978565b908152604051908190036020019020805460ff9290921660ff19909216919091179055610a94610fd7565b50565b610a9f610ef6565b610a94815f611019565b610ab1610ef6565b6001600160a01b038116610ae657604051633603021960e21b81523260048201525f60248201523060448201526064016105ae565b610a94816001611019565b610af9610ef6565b600a82604051610b099190611978565b90815260405190819003602001902054815160ff9091161015610b6757808151600a84604051610b399190611978565b908152604051908190036020018120546378a014e160e11b82526105ae93929160ff90911690600401611983565b6001600160401b0383165f908152600b6020526040908190209051829190610b90908590611978565b90815260200160405180910390209081610baa91906119f9565b50610bb3610fd7565b505050565b6060610bc2610ef6565b6001600160401b0383165f908152600c602090815260408083206001600160a01b038616845290915290208054610bf890611739565b80601f0160208091040260200160405190810160405280929190818152602001828054610c2490611739565b8015610c6f5780601f10610c4657610100808354040283529160200191610c6f565b820191905f5260205f20905b815481529060010190602001808311610c5257829003601f168201915b505050505090506001815110156105b757604051635f2c73e560e01b81526001600160a01b03831660048201526024016105ae565b6060600580546105cc90611739565b610cbb610ef6565b6004610cc783826119f9565b506005610bb382826119f9565b5f3361065a818585610fcc565b610ce9610ef6565b602081511115610d135780516040516378a014e160e11b81526105ae918391602090600401611983565b6001600160401b0383165f908152600c6020908152604080832085516001600160a01b031684529091529020610baa82826119f9565b610d51610ef6565b80600a83604051610d629190611978565b908152604051908190036020019020805460ff9290921660ff19909216919091179055610d8d610fd7565b5050565b610d99610ef6565b602081511115610dc35780516040516378a014e160e11b81526105ae918391602090600401611983565b6001600160401b0383165f908152600c602090815260408083206001600160a01b03861684529091529020610baa82826119f9565b6060610e02610ef6565b6001600160401b0383165f908152600b6020526040908190209051610e28908490611978565b90815260200160405180910390208054610e4190611739565b80601f0160208091040260200160405190810160405280929190818152602001828054610e6d90611739565b8015610eb85780601f10610e8f57610100808354040283529160200191610eb8565b820191905f5260205f20905b815481529060010190602001808311610e9b57829003601f168201915b505050505090506001815110156105b75781604051634a63062560e11b81526004016105ae91906113bf565b610eec610ef6565b610ef4610fd7565b565b335f9081526020819052604090205460ff16158015610f245750325f9081526020819052604090205460ff16155b15610ef457604051630be6bab560e01b81523260048201523360248201523060448201526064016105ae565b610bb3838383600161106c565b6001600160a01b038381165f908152600260209081526040808320938616835292905220545f198114610fc65781811015610fb8573233858530858760405162b83d1960e21b81526004016105ae9796959493929190611ab3565b610fc684848484035f61106c565b50505050565b610bb38383836110ec565b610fe36012600a611868565b600754610ff09190611876565b6003541015610ef457610ef4306110096012600a611868565b611014906001611876565b611207565b6001600160a01b0382165f81815260208190526040808220805460ff191685151590811790915590519092917f6bfa7e860c3e9bf67b810cb88276b19b27c972812bac04793d369b59a1183a8391a35050565b6001600160a01b038085165f9081526002602090815260408083209387168352929052208290558015610fc657826001600160a01b0316846001600160a01b03167f8c5be1e5ebec7d5bd14f71427d1e84f3dd0314c0f7b2291e5b200ac8c7c3b925846040516110de91815260200190565b60405180910390a350505050565b6001600160a01b038316611116578060035f82825461110b9190611af6565b9091555061117b9050565b6001600160a01b0383165f908152600160205260409020548181101561115d5732338585308587604051630f7d0bbd60e21b81526004016105ae9796959493929190611ab3565b6001600160a01b0384165f9081526001602052604090209082900390555b6001600160a01b038216611197576003805482900390556111b5565b6001600160a01b0382165f9081526001602052604090208054820190555b816001600160a01b0316836001600160a01b03167fddf252ad1be2c89b69c2b068fc378daa952ba7f163c4a11628f55a4df523b3ef836040516111fa91815260200190565b60405180910390a3505050565b610d8d5f83836110ec565b80356001600160401b0381168114611228575f5ffd5b919050565b634e487b7160e01b5f52604160045260245ffd5b60405161010081016001600160401b03811182821017156112645761126461122d565b60405290565b604051601f8201601f191681016001600160401b03811182821017156112925761129261122d565b604052919050565b6001600160a01b0381168114610a94575f5ffd5b80356112288161129a565b5f61010082840312156112ca575f5ffd5b6112d2611241565b905081356112df8161129a565b81526112ed602083016112ae565b60208201526112fe60408301611212565b604082015261130f60608301611212565b6060820152611320608083016112ae565b608082015261133160a08301611212565b60a082015261134260c08301611212565b60c082015261135360e08301611212565b60e082015292915050565b5f5f6101208385031215611370575f5ffd5b61137983611212565b915061138884602085016112b9565b90509250929050565b5f81518084528060208401602086015e5f602082860101526020601f19601f83011685010191505092915050565b602081525f6113d16020830184611391565b9392505050565b5f5f604083850312156113e9575f5ffd5b82356113f48161129a565b946020939093013593505050565b5f5f5f60608486031215611414575f5ffd5b833561141f8161129a565b9250602084013561142f8161129a565b929592945050506040919091013590565b5f60208284031215611450575f5ffd5b81356113d18161129a565b5f6001600160401b038211156114735761147361122d565b50601f01601f191660200190565b5f82601f830112611490575f5ffd5b81356114a361149e8261145b565b61126a565b8181528460208386010111156114b7575f5ffd5b816020850160208301375f918101602001919091529392505050565b5f602082840312156114e3575f5ffd5b81356001600160401b038111156114f8575f5ffd5b61150484828501611481565b949350505050565b5f5f5f6060848603121561151e575f5ffd5b61152784611212565b925060208401356001600160401b03811115611541575f5ffd5b61154d86828701611481565b92505060408401356001600160401b03811115611568575f5ffd5b61157486828701611481565b9150509250925092565b5f5f6040838503121561158f575f5ffd5b61159883611212565b915060208301356115a88161129a565b809150509250929050565b5f5f604083850312156115c4575f5ffd5b82356001600160401b038111156115d9575f5ffd5b6115e585828601611481565b92505060208301356001600160401b03811115611600575f5ffd5b61160c85828601611481565b9150509250929050565b5f5f5f6101408486031215611629575f5ffd5b61163284611212565b925061164185602086016112b9565b91506101208401356001600160401b03811115611568575f5ffd5b5f5f6040838503121561166d575f5ffd5b82356001600160401b03811115611682575f5ffd5b61168e85828601611481565b925050602083013560ff811681146115a8575f5ffd5b5f5f5f606084860312156116b6575f5ffd5b6116bf84611212565b925060208401356116cf8161129a565b915060408401356001600160401b03811115611568575f5ffd5b5f5f604083850312156116fa575f5ffd5b82356115988161129a565b5f5f60408385031215611716575f5ffd5b61171f83611212565b915060208301356001600160401b03811115611600575f5ffd5b600181811c9082168061174d57607f821691505b60208210810361176b57634e487b7160e01b5f52602260045260245ffd5b50919050565b634e487b7160e01b5f52601160045260245ffd5b6001815b60018411156117c0578085048111156117a4576117a4611771565b60018416156117b257908102905b60019390931c928002611789565b935093915050565b5f826117d6575060016105b7565b816117e257505f6105b7565b81600181146117f857600281146118025761181e565b60019150506105b7565b60ff84111561181357611813611771565b50506001821b6105b7565b5060208310610133831016604e8410600b8410161715611841575081810a6105b7565b61184d5f198484611785565b805f190482111561186057611860611771565b029392505050565b5f6113d160ff8416836117c8565b80820281158282048414176105b7576105b7611771565b5f826118a757634e487b7160e01b5f52601260045260245ffd5b500490565b5f602082840312156118bc575f5ffd5b815180151581146113d1575f5ffd5b5f602082840312156118db575f5ffd5b81516001600160401b038111156118f0575f5ffd5b8201601f81018413611900575f5ffd5b805161190e61149e8261145b565b818152856020838501011115611922575f5ffd5b8160208401602083015e5f91810160200191909152949350505050565b5f81518060208401855e5f93019283525090919050565b6d02732b2b21020b8383937bb32b2160951b81525f6113d1600e83018461193f565b5f6113d1828461193f565b606081525f6119956060830186611391565b905083602083015260ff83166040830152949350505050565b601f821115610bb357805f5260205f20601f840160051c810160208510156119d35750805b601f840160051c820191505b818110156119f2575f81556001016119df565b5050505050565b81516001600160401b03811115611a1257611a1261122d565b611a2681611a208454611739565b846119ae565b6020601f821160018114611a58575f8315611a415750848201515b5f19600385901b1c1916600184901b1784556119f2565b5f84815260208120601f198516915b82811015611a875787850151825560209485019460019092019101611a67565b5084821015611aa457868401515f19600387901b60f8161c191681555b50505050600190811b01905550565b6001600160a01b039788168152958716602087015293861660408601529185166060850152909316608083015260a082019290925260c081019190915260e00190565b808201808211156105b7576105b761177156fea264697066735822122041e0d8f23c9cccaaa15c75d0f143c06a1ceab77f64013dbb37500236a3ab2b7664736f6c634300081c0033";
        public AttributeDeploymentBase() : base(BYTECODE) { }
        public AttributeDeploymentBase(string byteCode) : base(byteCode) { }
        [Parameter("address", "VoidAddress", 1)]
        public virtual string VoidAddress { get; set; }
    }

    public partial class AliasFunction : AliasFunctionBase { }

    [Function("Alias", "string")]
    public class AliasFunctionBase : FunctionMessage
    {
        [Parameter("uint64", "Soul", 1)]
        public virtual ulong Soul { get; set; }
        [Parameter("tuple", "entity", 2)]
        public virtual Bao Entity { get; set; }
    }

    public partial class Alias1Function : Alias1FunctionBase { }

    [Function("Alias", "string")]
    public class Alias1FunctionBase : FunctionMessage
    {
        [Parameter("uint64", "Soul", 1)]
        public virtual ulong Soul { get; set; }
        [Parameter("address", "name", 2)]
        public virtual string Name { get; set; }
    }

    public partial class Alias2Function : Alias2FunctionBase { }

    [Function("Alias")]
    public class Alias2FunctionBase : FunctionMessage
    {
        [Parameter("uint64", "Soul", 1)]
        public virtual ulong Soul { get; set; }
        [Parameter("tuple", "entity", 2)]
        public virtual Bao Entity { get; set; }
        [Parameter("string", "value", 3)]
        public virtual string Value { get; set; }
    }

    public partial class Alias3Function : Alias3FunctionBase { }

    [Function("Alias")]
    public class Alias3FunctionBase : FunctionMessage
    {
        [Parameter("uint64", "Soul", 1)]
        public virtual ulong Soul { get; set; }
        [Parameter("address", "name", 2)]
        public virtual string Name { get; set; }
        [Parameter("string", "value", 3)]
        public virtual string Value { get; set; }
    }

    public partial class GetFunction : GetFunctionBase { }

    [Function("Get", "string")]
    public class GetFunctionBase : FunctionMessage
    {
        [Parameter("uint64", "Soul", 1)]
        public virtual ulong Soul { get; set; }
        [Parameter("string", "name", 2)]
        public virtual string Name { get; set; }
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

    public partial class SetFunction : SetFunctionBase { }

    [Function("Set")]
    public class SetFunctionBase : FunctionMessage
    {
        [Parameter("uint64", "Soul", 1)]
        public virtual ulong Soul { get; set; }
        [Parameter("string", "name", 2)]
        public virtual string Name { get; set; }
        [Parameter("string", "value", 3)]
        public virtual string Value { get; set; }
    }

    public partial class TypeFunction : TypeFunctionBase { }

    [Function("Type", "string")]
    public class TypeFunctionBase : FunctionMessage
    {

    }

    public partial class VoidFunction : VoidFunctionBase { }

    [Function("Void", "address")]
    public class VoidFunctionBase : FunctionMessage
    {

    }

    public partial class XiaoFunction : XiaoFunctionBase { }

    [Function("Xiao", "address")]
    public class XiaoFunctionBase : FunctionMessage
    {

    }

    public partial class AddAttributeFunction : AddAttributeFunctionBase { }

    [Function("addAttribute")]
    public class AddAttributeFunctionBase : FunctionMessage
    {
        [Parameter("string", "name", 1)]
        public virtual string Name { get; set; }
        [Parameter("uint8", "maxLength", 2)]
        public virtual byte MaxLength { get; set; }
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

    public partial class RemoveAttributeFunction : RemoveAttributeFunctionBase { }

    [Function("removeAttribute")]
    public class RemoveAttributeFunctionBase : FunctionMessage
    {
        [Parameter("string", "name", 1)]
        public virtual string Name { get; set; }
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

    public partial class AliasNotFoundError : AliasNotFoundErrorBase { }

    [Error("AliasNotFound")]
    public class AliasNotFoundErrorBase : IErrorDTO
    {
        [Parameter("address", "name", 1)]
        public virtual string Name { get; set; }
    }

    public partial class AttributeNotFoundError : AttributeNotFoundErrorBase { }

    [Error("AttributeNotFound")]
    public class AttributeNotFoundErrorBase : IErrorDTO
    {
        [Parameter("string", "name", 1)]
        public virtual string Name { get; set; }
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

    public partial class MaximumLengthError : MaximumLengthErrorBase { }

    [Error("MaximumLength")]
    public class MaximumLengthErrorBase : IErrorDTO
    {
        [Parameter("string", "value", 1)]
        public virtual string Value { get; set; }
        [Parameter("uint256", "length", 2)]
        public virtual BigInteger Length { get; set; }
        [Parameter("uint8", "maxLength", 3)]
        public virtual byte MaxLength { get; set; }
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

    public partial class AliasOutputDTO : AliasOutputDTOBase { }

    [FunctionOutput]
    public class AliasOutputDTOBase : IFunctionOutputDTO 
    {
        [Parameter("string", "_a", 1)]
        public virtual string A { get; set; }
    }

    public partial class Alias1OutputDTO : Alias1OutputDTOBase { }

    [FunctionOutput]
    public class Alias1OutputDTOBase : IFunctionOutputDTO 
    {
        [Parameter("string", "_a", 1)]
        public virtual string A { get; set; }
    }





    public partial class GetOutputDTO : GetOutputDTOBase { }

    [FunctionOutput]
    public class GetOutputDTOBase : IFunctionOutputDTO 
    {
        [Parameter("string", "_a", 1)]
        public virtual string A { get; set; }
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

    public partial class VoidOutputDTO : VoidOutputDTOBase { }

    [FunctionOutput]
    public class VoidOutputDTOBase : IFunctionOutputDTO 
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
