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

namespace Dysnomia.Contracts.SHIOFactory.ContractDefinition
{


    public partial class SHIOFactoryDeployment : SHIOFactoryDeploymentBase
    {
        public SHIOFactoryDeployment() : base(BYTECODE) { }
        public SHIOFactoryDeployment(string byteCode) : base(byteCode) { }
    }

    public class SHIOFactoryDeploymentBase : ContractDeploymentMessage
    {
        public static string BYTECODE = "6080604052348015600e575f5ffd5b5061371b8061001c5f395ff3fe608060405234801561000f575f5ffd5b5060043610610029575f3560e01c8063485830ae1461002d575b5f5ffd5b61004061003b366004610280565b61005c565b6040516001600160a01b03909116815260200160405180910390f35b5f83838360405161006c90610258565b6001600160a01b03938416815291831660208301529091166040820152606001604051809103905ff0801580156100a5573d5f5f3e3d5ffd5b50604051630e0cb96960e31b81523360048201529091506001600160a01b03821690637065cb48906024015f604051808303815f87803b1580156100e7575f5ffd5b505af11580156100f9573d5f5f3e3d5ffd5b5050604051630e0cb96960e31b81523260048201526001600160a01b0384169250637065cb4891506024015f604051808303815f87803b15801561013b575f5ffd5b505af115801561014d573d5f5f3e3d5ffd5b5050604051631c5f9e7d60e11b81523060048201526001600160a01b03841692506338bf3cfa91506024015f604051808303815f87803b15801561018f575f5ffd5b505af11580156101a1573d5f5f3e3d5ffd5b5050604051630e0cb96960e31b81526001600160a01b03848116600483015287169250637065cb4891506024015f604051808303815f87803b1580156101e5575f5ffd5b505af11580156101f7573d5f5f3e3d5ffd5b5050604051630e0cb96960e31b81526001600160a01b03848116600483015286169250637065cb4891506024015f604051808303815f87803b15801561023b575f5ffd5b505af115801561024d573d5f5f3e3d5ffd5b505050509392505050565b613425806102c183390190565b80356001600160a01b038116811461027b575f5ffd5b919050565b5f5f5f60608486031215610292575f5ffd5b61029b84610265565b92506102a960208501610265565b91506102b760408501610265565b9050925092509256fe608060405234801561000f575f5ffd5b5060405161342538038061342583398101604081905261002e916104ac565b604080518082018252600c81526b434841544c4f47205368696f60a01b602080830191909152825180840190935260048352635348494f60e01b90830152908233806100a057604051633603021960e21b81523260048201525f60248201523060448201526064015b60405180910390fd5b6100ab8160016102a8565b5060046100b88482610584565b5060056100c58382610584565b50600680546001600160a01b0319166001600160a01b0383169081179091556040805163604a6fa960e01b815290516201b207929163604a6fa991600480830192602092919082900301815f875af1158015610123573d5f5f3e3d5ffd5b505050506040513d601f19601f82011682018060405250810190610147919061063e565b610151919061067f565b6001600160401b03166007819055505f600a60075460065f9054906101000a90046001600160a01b03166001600160a01b031663604a6fa96040518163ffffffff1660e01b81526004016020604051808303815f875af11580156101b7573d5f5f3e3d5ffd5b505050506040513d601f19601f820116820180604052508101906101db919061063e565b6001600160401b03166101ee91906106ad565b6101f891906106d4565b90506102193261020a6012600a6107cc565b61021490846107da565b6102fb565b6102637324f0154c1dce548adf15da2098fdd8b8a3b8151d61023d6012600a6107cc565b6102489060016107da565b6001600160a01b039091165f90815260086020526040902055565b5050600980546001600160a01b038088166001600160a01b031992831617909255600a805492871692909116919091179055506102a0905061030a565b505050610804565b6001600160a01b0382165f81815260208190526040808220805460ff191685151590811790915590519092917f6bfa7e860c3e9bf67b810cb88276b19b27c972812bac04793d369b59a1183a8391a35050565b6103065f8383610314565b5050565b610312610454565b565b6001600160a01b03831661033e578060035f82825461033391906107f1565b909155506103c89050565b6001600160a01b0383165f90815260016020526040902054818110156103aa57604051630f7d0bbd60e21b81523260048201523360248201526001600160a01b0380861660448301528416606482015230608482015260a4810182905260c4810183905260e401610097565b6001600160a01b0384165f9081526001602052604090209082900390555b6001600160a01b0382166103e457600380548290039055610402565b6001600160a01b0382165f9081526001602052604090208054820190555b816001600160a01b0316836001600160a01b03167fddf252ad1be2c89b69c2b068fc378daa952ba7f163c4a11628f55a4df523b3ef8360405161044791815260200190565b60405180910390a3505050565b6104606012600a6107cc565b60075461046d91906107da565b600354101561031257610312306104866012600a6107cc565b6102149060016107da565b80516001600160a01b03811681146104a7575f5ffd5b919050565b5f5f5f606084860312156104be575f5ffd5b6104c784610491565b92506104d560208501610491565b91506104e360408501610491565b90509250925092565b634e487b7160e01b5f52604160045260245ffd5b600181811c9082168061051457607f821691505b60208210810361053257634e487b7160e01b5f52602260045260245ffd5b50919050565b601f82111561057f57805f5260205f20601f840160051c8101602085101561055d5750805b601f840160051c820191505b8181101561057c575f8155600101610569565b50505b505050565b81516001600160401b0381111561059d5761059d6104ec565b6105b1816105ab8454610500565b84610538565b6020601f8211600181146105e3575f83156105cc5750848201515b5f19600385901b1c1916600184901b17845561057c565b5f84815260208120601f198516915b8281101561061257878501518255602094850194600190920191016105f2565b508482101561062f57868401515f19600387901b60f8161c191681555b50505050600190811b01905550565b5f6020828403121561064e575f5ffd5b81516001600160401b0381168114610664575f5ffd5b9392505050565b634e487b7160e01b5f52601260045260245ffd5b5f6001600160401b038316806106975761069761066b565b6001600160401b03929092169190910692915050565b5f826106bb576106bb61066b565b500690565b634e487b7160e01b5f52601160045260245ffd5b5f826106e2576106e261066b565b500490565b6001815b600184111561072257808504811115610706576107066106c0565b600184161561071457908102905b60019390931c9280026106eb565b935093915050565b5f82610738575060016107c6565b8161074457505f6107c6565b816001811461075a576002811461076457610780565b60019150506107c6565b60ff841115610775576107756106c0565b50506001821b6107c6565b5060208310610133831016604e8410600b84101617156107a3575081810a6107c6565b6107af5f1984846106e7565b805f19048211156107c2576107c26106c0565b0290505b92915050565b5f61066460ff84168361072a565b80820281158282048414176107c6576107c66106c0565b808201808211156107c6576107c66106c0565b612c14806108115f395ff3fe608060405234801561000f575f5ffd5b50600436106101f2575f3560e01c80636fb7329c11610114578063a4566950116100a9578063daeee48211610079578063daeee482146104bb578063dd62ed3e146104ce578063ece1af6b14610506578063f4e6c93f1461050e578063ff76204414610516575f5ffd5b8063a456695014610435578063a9059cbb14610491578063d3dc2a0f146104a4578063d5abeb01146104b2575f5ffd5b80638da5cb5b116100e45780638da5cb5b1461040c57806395d89b411461041257806395fb81901461041a5780639dcbf7ea14610422575f5ffd5b80636fb7329c146103ab5780637065cb48146103be57806370a08231146103d157806385da8ff8146103f9575f5ffd5b806325a28a8f1161018a5780633b8052c91161015a5780633b8052c9146103225780633cf93051146103555780634970204f1461036f578063666e1b3914610380575f5ffd5b806325a28a8f146102b35780632b83b1fc146102d8578063313ce5671461030057806338bf3cfa1461030f575f5ffd5b806318160ddd116101c557806318160ddd14610268578063222838db1461027a57806323b872dd1461028d5780632499a533146102a0575f5ffd5b806303443a15146101f657806306fdde0314610226578063089c10b01461023b578063095ea7b314610245575b5f5ffd5b600b54610209906001600160401b031681565b6040516001600160401b0390911681526020015b60405180910390f35b61022e610539565b60405161021d91906123b7565b6102436105c9565b005b6102586102533660046123eb565b610635565b604051901515815260200161021d565b6003545b60405190815260200161021d565b6102436102883660046123eb565b61064e565b61025861029b366004612413565b61085a565b6102436102ae3660046123eb565b61087d565b6009546001600160a01b03165b6040516001600160a01b03909116815260200161021d565b61026c6102e636600461244d565b6001600160a01b03165f9081526008602052604090205490565b6040516012815260200161021d565b61024361031d36600461244d565b610a3f565b61033561033036600461247a565b610a54565b604080516001600160401b0393841681529290911660208301520161021d565b600b5461020990600160401b90046001600160401b031681565b600a546001600160a01b03166102c0565b61025861038e36600461244d565b6001600160a01b03165f9081526020819052604090205460ff1690565b6006546102c0906001600160a01b031681565b6102436103cc36600461244d565b610d20565b61026c6103df36600461244d565b6001600160a01b03165f9081526001602052604090205490565b610243610407366004612495565b610d68565b306102c0565b61022e61123a565b610209611249565b6102436104303660046125c2565b611c73565b600954600a54610462916001600160a01b039081169190811690600160a01b90046001600160401b031683565b604080516001600160a01b0394851681529390921660208401526001600160401b03169082015260600161021d565b61025861049f3660046123eb565b611c94565b6102096603632c8eb5af3b81565b61026c60075481565b6102436104c9366004612625565b611ca1565b61026c6104dc366004612682565b6001600160a01b039182165f90815260026020908152604080832093909416825291909152205490565b610243611ce9565b610243611d35565b61022e604051806040016040528060048152602001635348494f60e01b81525081565b606060048054610548906126b3565b80601f0160208091040260200160405190810160405280929190818152602001828054610574906126b3565b80156105bf5780601f10610596576101008083540402835291602001916105bf565b820191905f5260205f20905b8154815290600101906020018083116105a257829003601f168201915b5050505050905090565b6105d1611d45565b600a54604080516346dd440160e01b815290516001600160a01b03909216916346dd4401916004808201925f9290919082900301818387803b158015610615575f5ffd5b505af1158015610627573d5f5f3e3d5ffd5b50505050610633611d9f565b565b5f33610642818585611de1565b60019150505b92915050565b6001600160a01b0382165f908152600860205260408120549003610695576040516390b2344160e01b81526001600160a01b03831660048201526024015b60405180910390fd5b815f6106a36012600a6127e2565b6001600160a01b0385165f908152600860205260409020546106c590856127f0565b6106cf9190612807565b6040516323b872dd60e01b81523360048201523060248201819052604482018690529192505f91906323b872dd906064016020604051808303815f875af115801561071c573d5f5f3e3d5ffd5b505050506040513d601f19601f820116820180604052508101906107409190612826565b905080306001600160a01b03166306fdde036040518163ffffffff1660e01b81526004015f60405180830381865afa15801561077e573d5f5f3e3d5ffd5b505050506040513d5f823e601f3d908101601f191682016040526107a59190810190612845565b6040516020016107b591906128b9565b604051602081830303815290604052906107e25760405162461bcd60e51b815260040161068c91906123b7565b5060405163a9059cbb60e01b8152336004820152602481018390526001600160a01b0384169063a9059cbb906044015b6020604051808303815f875af115801561082e573d5f5f3e3d5ffd5b505050506040513d601f19601f820116820180604052508101906108529190612826565b505050505050565b5f33610867858285611dee565b610872858585611e5d565b506001949350505050565b6001600160a01b0382165f9081526008602052604081205490036108bf576040516390b2344160e01b81526001600160a01b038316600482015260240161068c565b815f6108cd6012600a6127e2565b6001600160a01b0385165f908152600860205260409020546108ef90856127f0565b6108f99190612807565b6040516323b872dd60e01b8152336004820152306024820152604481018290529091505f906001600160a01b038416906323b872dd906064016020604051808303815f875af115801561094e573d5f5f3e3d5ffd5b505050506040513d601f19601f820116820180604052508101906109729190612826565b905080836001600160a01b03166306fdde036040518163ffffffff1660e01b81526004015f60405180830381865afa1580156109b0573d5f5f3e3d5ffd5b505050506040513d5f823e601f3d908101601f191682016040526109d79190810190612845565b6040516020016109e791906128b9565b60405160208183030381529060405290610a145760405162461bcd60e51b815260040161068c91906123b7565b5060405163a9059cbb60e01b815233600482015260248101859052309063a9059cbb90604401610812565b610a47611d45565b610a51815f611e68565b50565b600b54600954600a5460408051631686f26560e01b81529051600160401b9094046001600160401b031694909418935f938493849384936001600160a01b039283169363f302d97f938a931691631686f265916004808301926101a092919082900301818a875af1158015610acb573d5f5f3e3d5ffd5b505050506040513d601f19601f82011682018060405250810190610aef91906128f4565b606001516040516001600160e01b031960e085901b1681526001600160401b0392831660048201529116602482015260440160408051808303815f875af1158015610b3c573d5f5f3e3d5ffd5b505050506040513d601f19601f82011682018060405250810190610b6091906129f7565b600a5460095460408051631686f26560e01b815290519496509294505f9384936001600160a01b039384169363f302d97f938c93911691631686f265916004808301926101a092919082900301818a875af1158015610bc1573d5f5f3e3d5ffd5b505050506040513d601f19601f82011682018060405250810190610be591906128f4565b606001516040516001600160e01b031960e085901b1681526001600160401b0392831660048201529116602482015260440160408051808303815f875af1158015610c32573d5f5f3e3d5ffd5b505050506040513d601f19601f82011682018060405250810190610c5691906129f7565b91509150826001600160401b0316826001600160401b0316141580610c8d5750836001600160401b0316816001600160401b031614155b15610cbe576040516364b8b9c560e11b81526001600160401b0380861660048301528416602482015260440161068c565b6001600160401b0381161580610cdb57506001600160401b038216155b15610d0c5760405163b5067c2760e01b81526001600160401b0380861660048301528416602482015260440161068c565b610d14611d35565b90969095509350505050565b610d28611d45565b6001600160a01b038116610d5d57604051633603021960e21b81523260048201525f602482015230604482015260640161068c565b610a51816001611e68565b610d70611d45565b610d7983611ebb565b600954600a5460408051631686f26560e01b815290516001600160a01b03938416936372d229fd931691631686f265916004808301926101a092919082900301815f875af1158015610dcd573d5f5f3e3d5ffd5b505050506040513d601f19601f82011682018060405250810190610df191906128f4565b60a001516040516001600160e01b031960e084901b1681526001600160401b0390911660048201526024015f604051808303815f87803b158015610e33575f5ffd5b505af1158015610e45573d5f5f3e3d5ffd5b5050600a5460095460408051631686f26560e01b815290516001600160a01b0393841695506372d229fd94509190921691631686f265916004808301926101a092919082900301815f875af1158015610ea0573d5f5f3e3d5ffd5b505050506040513d601f19601f82011682018060405250810190610ec491906128f4565b60a001516040516001600160e01b031960e084901b1681526001600160401b0390911660048201526024015f604051808303815f87803b158015610f06575f5ffd5b505af1158015610f18573d5f5f3e3d5ffd5b5050600a5460405163467e251f60e01b81526001600160401b03861660048201526001600160a01b03909116925063467e251f91506024015f604051808303815f87803b158015610f67575f5ffd5b505af1158015610f79573d5f5f3e3d5ffd5b5050600954600a5460408051631686f26560e01b815290516001600160a01b039384169550633e9f64449450879390921691631686f265916004808201926101a09290919082900301815f875af1158015610fd6573d5f5f3e3d5ffd5b505050506040513d601f19601f82011682018060405250810190610ffa91906128f4565b60e00151600a5460408051631686f26560e01b815290516001600160a01b0390921691631686f265916004808201926101a09290919082900301815f875af1158015611048573d5f5f3e3d5ffd5b505050506040513d601f19601f8201168201806040525081019061106c91906128f4565b606001516040516001600160e01b031960e086901b1681526001600160401b03938416600482015291831660248301529190911660448201526064015f604051808303815f87803b1580156110bf575f5ffd5b505af11580156110d1573d5f5f3e3d5ffd5b5050600a5460095460408051631686f26560e01b815290516001600160a01b039384169550633e9f64449450869390921691631686f265916004808201926101a09290919082900301815f875af115801561112e573d5f5f3e3d5ffd5b505050506040513d601f19601f8201168201806040525081019061115291906128f4565b60e0015160095460408051631686f26560e01b815290516001600160a01b0390921691631686f265916004808201926101a09290919082900301815f875af11580156111a0573d5f5f3e3d5ffd5b505050506040513d601f19601f820116820180604052508101906111c491906128f4565b606001516040516001600160e01b031960e086901b1681526001600160401b03938416600482015291831660248301529190911660448201526064015f604051808303815f87803b158015611217575f5ffd5b505af1158015611229573d5f5f3e3d5ffd5b50505050611235611d9f565b505050565b606060058054610548906126b3565b5f611252611d45565b600954600a54604080516326cf553960e21b815290516001600160a01b039384169363f6671de4931691639b3d54e491600480830192602092919082900301815f875af11580156112a5573d5f5f3e3d5ffd5b505050506040513d601f19601f820116820180604052508101906112c99190612a2f565b6040516001600160e01b031960e084901b1681526001600160401b0390911660048201526024016020604051808303815f875af115801561130c573d5f5f3e3d5ffd5b505050506040513d601f19601f820116820180604052508101906113309190612a2f565b600b805467ffffffffffffffff19166001600160401b0392909216919091179055600a54600954604080516326cf553960e21b815290516001600160a01b039384169363f6671de4931691639b3d54e491600480830192602092919082900301815f875af11580156113a4573d5f5f3e3d5ffd5b505050506040513d601f19601f820116820180604052508101906113c89190612a2f565b6040516001600160e01b031960e084901b1681526001600160401b0390911660048201526024016020604051808303815f875af115801561140b573d5f5f3e3d5ffd5b505050506040513d601f19601f8201168201806040525081019061142f9190612a2f565b600b546001600160401b0390811691161461146c57600b546040516326cc173d60e01b81526001600160401b03909116600482015260240161068c565b60065460095460408051631686f26560e01b815290515f936001600160a01b03908116936353df6e3593911691631686f265916004808201926101a092909190829003018189875af11580156114c4573d5f5f3e3d5ffd5b505050506040513d601f19601f820116820180604052508101906114e891906128f4565b6101200151600b5460095460408051631686f26560e01b815290516001600160401b03909316926001600160a01b0390921691631686f265916004808201926101a09290919082900301815f875af1158015611546573d5f5f3e3d5ffd5b505050506040513d601f19601f8201168201806040525081019061156a91906128f4565b61010001516040516001600160e01b031960e086901b1681526001600160401b03938416600482015291831660248301529190911660448201526064016020604051808303815f875af11580156115c3573d5f5f3e3d5ffd5b505050506040513d601f19601f820116820180604052508101906115e79190612a2f565b600654600a5460408051631686f26560e01b815290519394506001600160a01b03928316936353df6e359390921691631686f265916004808201926101a09290919082900301815f875af1158015611641573d5f5f3e3d5ffd5b505050506040513d601f19601f8201168201806040525081019061166591906128f4565b6101200151600b54600a5460408051631686f26560e01b815290516001600160401b03909316926001600160a01b0390921691631686f265916004808201926101a09290919082900301815f875af11580156116c3573d5f5f3e3d5ffd5b505050506040513d601f19601f820116820180604052508101906116e791906128f4565b61010001516040516001600160e01b031960e086901b1681526001600160401b03938416600482015291831660248301529190911660448201526064016020604051808303815f875af1158015611740573d5f5f3e3d5ffd5b505050506040513d601f19601f820116820180604052508101906117649190612a2f565b6001600160401b0316816001600160401b0316146117a057604051631472a7a760e01b81526001600160401b038216600482015260240161068c565b600654600b5460095460408051631686f26560e01b815290516001600160a01b03948516946353df6e359487946001600160401b0390911693911691631686f265916004808201926101a09290919082900301815f875af1158015611807573d5f5f3e3d5ffd5b505050506040513d601f19601f8201168201806040525081019061182b91906128f4565b61010001516040516001600160e01b031960e086901b1681526001600160401b03938416600482015291831660248301529190911660448201526064016020604051808303815f875af1158015611884573d5f5f3e3d5ffd5b505050506040513d601f19601f820116820180604052508101906118a89190612a2f565b600a80546001600160401b03928316600160a01b0267ffffffffffffffff60a01b1982168117909255600654600b5460408051631686f26560e01b815290516001600160a01b03938416966353df6e35968996949091169490811693169290921791631686f265916004808201926101a09290919082900301815f875af1158015611935573d5f5f3e3d5ffd5b505050506040513d601f19601f8201168201806040525081019061195991906128f4565b61010001516040516001600160e01b031960e086901b1681526001600160401b03938416600482015291831660248301529190911660448201526064016020604051808303815f875af11580156119b2573d5f5f3e3d5ffd5b505050506040513d601f19601f820116820180604052508101906119d69190612a2f565b600a54600160a01b90046001600160401b03908116911614611a2057600a5460405163d1c1e0b960e01b8152600160a01b9091046001600160401b0316600482015260240161068c565b60065460095460408051631686f26560e01b815290516001600160a01b03938416936353df6e35931691631686f265916004808301926101a092919082900301815f875af1158015611a74573d5f5f3e3d5ffd5b505050506040513d601f19601f82011682018060405250810190611a9891906128f4565b6101600151600a5460408051631686f26560e01b815290516001600160a01b0390921691631686f265916004808201926101a09290919082900301815f875af1158015611ae7573d5f5f3e3d5ffd5b505050506040513d601f19601f82011682018060405250810190611b0b91906128f4565b61016001516040516001600160e01b031960e085901b1681526001600160401b039283166004820152911660248201526603632c8eb5af3b60448201526064016020604051808303815f875af1158015611b67573d5f5f3e3d5ffd5b505050506040513d601f19601f82011682018060405250810190611b8b9190612a2f565b600b60086101000a8154816001600160401b0302191690836001600160401b03160217905550611bb9611d9f565b6009546040805163f4e6c93f60e01b815290516001600160a01b039092169163f4e6c93f916004808201925f9290919082900301818387803b158015611bfd575f5ffd5b505af1158015611c0f573d5f5f3e3d5ffd5b5050600a546040805163f4e6c93f60e01b815290516001600160a01b03909216935063f4e6c93f92506004808201925f9290919082900301818387803b158015611c57575f5ffd5b505af1158015611c69573d5f5f3e3d5ffd5b5092935050505090565b611c7b611d45565b6004611c878382612a95565b5060056112358282612a95565b5f33610642818585611e5d565b611ca9611d45565b7f6b81130c485ac9b98332fa40c2e57900867815b0fe1497e1a168caf930fc9c9d838383604051611cdc93929190612b4f565b60405180910390a1505050565b611cf1611d45565b600954604080516346dd440160e01b815290516001600160a01b03909216916346dd4401916004808201925f9290919082900301818387803b158015610615575f5ffd5b611d3d611d45565b610633611d9f565b335f9081526020819052604090205460ff16158015611d735750325f9081526020819052604090205460ff16155b1561063357604051630be6bab560e01b815232600482015233602482015230604482015260640161068c565b611dab6012600a6127e2565b600754611db891906127f0565b60035410156106335761063330611dd16012600a6127e2565b611ddc9060016127f0565b6121df565b61123583838360016121ee565b6001600160a01b038381165f908152600260209081526040808320938616835292905220545f198114611e575781811015611e49573233858530858760405162b83d1960e21b815260040161068c9796959493929190612b88565b611e5784848484035f6121ee565b50505050565b61123583838361226e565b6001600160a01b0382165f81815260208190526040808220805460ff191685151590811790915590519092917f6bfa7e860c3e9bf67b810cb88276b19b27c972812bac04793d369b59a1183a8391a35050565b60095460405163c2bea95f60e01b81526001600160401b03831660048201526001600160a01b039091169063c2bea95f906024015f604051808303815f87803b158015611f06575f5ffd5b505af1158015611f18573d5f5f3e3d5ffd5b5050600a5460405163c2bea95f60e01b81526001600160401b03851660048201526001600160a01b03909116925063c2bea95f91506024015f604051808303815f87803b158015611f67575f5ffd5b505af1158015611f79573d5f5f3e3d5ffd5b5050600954600a5460408051631686f26560e01b815290516001600160a01b03938416955063b40619de94509190921691631686f265916004808301926101a092919082900301815f875af1158015611fd4573d5f5f3e3d5ffd5b505050506040513d601f19601f82011682018060405250810190611ff891906128f4565b608001516040516001600160e01b031960e084901b1681526001600160401b0390911660048201526024015f604051808303815f87803b15801561203a575f5ffd5b505af115801561204c573d5f5f3e3d5ffd5b5050600a5460095460408051631686f26560e01b815290516001600160a01b03938416955063b40619de94509190921691631686f265916004808301926101a092919082900301815f875af11580156120a7573d5f5f3e3d5ffd5b505050506040513d601f19601f820116820180604052508101906120cb91906128f4565b608001516040516001600160e01b031960e084901b1681526001600160401b0390911660048201526024015f604051808303815f87803b15801561210d575f5ffd5b505af115801561211f573d5f5f3e3d5ffd5b505060095460408051633edca88760e11b815290516001600160a01b039092169350637db9510e92506004808201925f9290919082900301818387803b158015612167575f5ffd5b505af1158015612179573d5f5f3e3d5ffd5b5050600a5460408051633edca88760e11b815290516001600160a01b039092169350637db9510e92506004808201925f9290919082900301818387803b1580156121c1575f5ffd5b505af11580156121d3573d5f5f3e3d5ffd5b50505050610a51611d9f565b6121ea5f838361226e565b5050565b6001600160a01b038085165f9081526002602090815260408083209387168352929052208290558015611e5757826001600160a01b0316846001600160a01b03167f8c5be1e5ebec7d5bd14f71427d1e84f3dd0314c0f7b2291e5b200ac8c7c3b9258460405161226091815260200190565b60405180910390a350505050565b6001600160a01b038316612298578060035f82825461228d9190612bcb565b909155506122fd9050565b6001600160a01b0383165f90815260016020526040902054818110156122df5732338585308587604051630f7d0bbd60e21b815260040161068c9796959493929190612b88565b6001600160a01b0384165f9081526001602052604090209082900390555b6001600160a01b03821661231957600380548290039055612337565b6001600160a01b0382165f9081526001602052604090208054820190555b816001600160a01b0316836001600160a01b03167fddf252ad1be2c89b69c2b068fc378daa952ba7f163c4a11628f55a4df523b3ef8360405161237c91815260200190565b60405180910390a3505050565b5f81518084528060208401602086015e5f602082860101526020601f19601f83011685010191505092915050565b602081525f6123c96020830184612389565b9392505050565b80356001600160a01b03811681146123e6575f5ffd5b919050565b5f5f604083850312156123fc575f5ffd5b612405836123d0565b946020939093013593505050565b5f5f5f60608486031215612425575f5ffd5b61242e846123d0565b925061243c602085016123d0565b929592945050506040919091013590565b5f6020828403121561245d575f5ffd5b6123c9826123d0565b6001600160401b0381168114610a51575f5ffd5b5f6020828403121561248a575f5ffd5b81356123c981612466565b5f5f5f606084860312156124a7575f5ffd5b83356124b281612466565b925060208401356124c281612466565b915060408401356124d281612466565b809150509250925092565b634e487b7160e01b5f52604160045260245ffd5b6040516101a081016001600160401b0381118282101715612514576125146124dd565b60405290565b604051601f8201601f191681016001600160401b0381118282101715612542576125426124dd565b604052919050565b5f6001600160401b03821115612562576125626124dd565b50601f01601f191660200190565b5f82601f83011261257f575f5ffd5b813561259261258d8261254a565b61251a565b8181528460208386010111156125a6575f5ffd5b816020850160208301375f918101602001919091529392505050565b5f5f604083850312156125d3575f5ffd5b82356001600160401b038111156125e8575f5ffd5b6125f485828601612570565b92505060208301356001600160401b0381111561260f575f5ffd5b61261b85828601612570565b9150509250929050565b5f5f5f60608486031215612637575f5ffd5b833561264281612466565b9250602084013561265281612466565b915060408401356001600160401b0381111561266c575f5ffd5b61267886828701612570565b9150509250925092565b5f5f60408385031215612693575f5ffd5b61269c836123d0565b91506126aa602084016123d0565b90509250929050565b600181811c908216806126c757607f821691505b6020821081036126e557634e487b7160e01b5f52602260045260245ffd5b50919050565b634e487b7160e01b5f52601160045260245ffd5b6001815b600184111561273a5780850481111561271e5761271e6126eb565b600184161561272c57908102905b60019390931c928002612703565b935093915050565b5f8261275057506001610648565b8161275c57505f610648565b8160018114612772576002811461277c57612798565b6001915050610648565b60ff84111561278d5761278d6126eb565b50506001821b610648565b5060208310610133831016604e8410600b84101617156127bb575081810a610648565b6127c75f1984846126ff565b805f19048211156127da576127da6126eb565b029392505050565b5f6123c960ff841683612742565b8082028115828204841417610648576106486126eb565b5f8261282157634e487b7160e01b5f52601260045260245ffd5b500490565b5f60208284031215612836575f5ffd5b815180151581146123c9575f5ffd5b5f60208284031215612855575f5ffd5b81516001600160401b0381111561286a575f5ffd5b8201601f8101841361287a575f5ffd5b805161288861258d8261254a565b81815285602083850101111561289c575f5ffd5b8160208401602083015e5f91810160200191909152949350505050565b6d02732b2b21020b8383937bb32b2160951b81525f82518060208501600e85015e5f9201600e0191825250919050565b80516123e681612466565b5f6101a0828403128015612906575f5ffd5b5061290f6124f1565b612918836128e9565b8152612926602084016128e9565b6020820152612937604084016128e9565b6040820152612948606084016128e9565b6060820152612959608084016128e9565b608082015261296a60a084016128e9565b60a082015261297b60c084016128e9565b60c082015261298c60e084016128e9565b60e082015261299e61010084016128e9565b6101008201526129b161012084016128e9565b6101208201526129c461014084016128e9565b6101408201526129d761016084016128e9565b6101608201526129ea61018084016128e9565b6101808201529392505050565b5f5f60408385031215612a08575f5ffd5b8251612a1381612466565b6020840151909250612a2481612466565b809150509250929050565b5f60208284031215612a3f575f5ffd5b81516123c981612466565b601f82111561123557805f5260205f20601f840160051c81016020851015612a6f5750805b601f840160051c820191505b81811015612a8e575f8155600101612a7b565b5050505050565b81516001600160401b03811115612aae57612aae6124dd565b612ac281612abc84546126b3565b84612a4a565b6020601f821160018114612af4575f8315612add5750848201515b5f19600385901b1c1916600184901b178455612a8e565b5f84815260208120601f198516915b82811015612b235787850151825560209485019460019092019101612b03565b5084821015612b4057868401515f19600387901b60f8161c191681555b50505050600190811b01905550565b6001600160401b03841681526001600160401b0383166020820152606060408201525f612b7f6060830184612389565b95945050505050565b6001600160a01b039788168152958716602087015293861660408601529185166060850152909316608083015260a082019290925260c081019190915260e00190565b80820180821115610648576106486126eb56fea2646970667358221220998e925a52b722f29751e28f1e5b046dd65555356a30ee55e3a400ca461fbea464736f6c634300081c0033a2646970667358221220547407ceda54348876519e599e983c214edf868f00095f16e026a8efd51ddf8164736f6c634300081c0033";
        public SHIOFactoryDeploymentBase() : base(BYTECODE) { }
        public SHIOFactoryDeploymentBase(string byteCode) : base(byteCode) { }

    }

    public partial class NewFunction : NewFunctionBase { }

    [Function("New", "address")]
    public class NewFunctionBase : FunctionMessage
    {
        [Parameter("address", "Rod", 1)]
        public virtual string Rod { get; set; }
        [Parameter("address", "Cone", 2)]
        public virtual string Cone { get; set; }
        [Parameter("address", "MathLib", 3)]
        public virtual string MathLib { get; set; }
    }


}
