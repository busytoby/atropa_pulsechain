using System;
using System.Threading.Tasks;
using System.Collections.Generic;
using System.Numerics;
using Nethereum.Hex.HexTypes;
using Nethereum.ABI.FunctionEncoding.Attributes;
using Nethereum.Web3;
using Nethereum.RPC.Eth.DTOs;
using Nethereum.Contracts.CQS;
using Nethereum.Contracts.ContractHandlers;
using Nethereum.Contracts;
using System.Threading;
using Dysnomia.Contracts.domain.dan.setaddresses.ContractDefinition;

namespace Dysnomia.Contracts.domain.dan.setaddresses
{
    public partial class SetaddressesService: ContractWeb3ServiceBase
    {
        public static Task<TransactionReceipt> DeployContractAndWaitForReceiptAsync(Nethereum.Web3.IWeb3 web3, SetaddressesDeployment setaddressesDeployment, CancellationTokenSource cancellationTokenSource = null)
        {
            return web3.Eth.GetContractDeploymentHandler<SetaddressesDeployment>().SendRequestAndWaitForReceiptAsync(setaddressesDeployment, cancellationTokenSource);
        }

        public static Task<string> DeployContractAsync(Nethereum.Web3.IWeb3 web3, SetaddressesDeployment setaddressesDeployment)
        {
            return web3.Eth.GetContractDeploymentHandler<SetaddressesDeployment>().SendRequestAsync(setaddressesDeployment);
        }

        public static async Task<SetaddressesService> DeployContractAndGetServiceAsync(Nethereum.Web3.IWeb3 web3, SetaddressesDeployment setaddressesDeployment, CancellationTokenSource cancellationTokenSource = null)
        {
            var receipt = await DeployContractAndWaitForReceiptAsync(web3, setaddressesDeployment, cancellationTokenSource);
            return new SetaddressesService(web3, receipt.ContractAddress);
        }

        public SetaddressesService(Nethereum.Web3.IWeb3 web3, string contractAddress) : base(web3, contractAddress)
        {
        }

        public override List<Type> GetAllFunctionTypes()
        {
            return new List<Type>
            {

            };
        }

        public override List<Type> GetAllEventTypes()
        {
            return new List<Type>
            {

            };
        }

        public override List<Type> GetAllErrorTypes()
        {
            return new List<Type>
            {

            };
        }
    }
}
