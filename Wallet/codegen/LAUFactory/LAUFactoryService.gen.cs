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
using Dysnomia.Contracts.LAUFactory.ContractDefinition;

namespace Dysnomia.Contracts.LAUFactory
{
    public partial class LAUFactoryService: ContractWeb3ServiceBase
    {
        public static Task<TransactionReceipt> DeployContractAndWaitForReceiptAsync(Nethereum.Web3.IWeb3 web3, LAUFactoryDeployment lAUFactoryDeployment, CancellationTokenSource cancellationTokenSource = null)
        {
            return web3.Eth.GetContractDeploymentHandler<LAUFactoryDeployment>().SendRequestAndWaitForReceiptAsync(lAUFactoryDeployment, cancellationTokenSource);
        }

        public static Task<string> DeployContractAsync(Nethereum.Web3.IWeb3 web3, LAUFactoryDeployment lAUFactoryDeployment)
        {
            return web3.Eth.GetContractDeploymentHandler<LAUFactoryDeployment>().SendRequestAsync(lAUFactoryDeployment);
        }

        public static async Task<LAUFactoryService> DeployContractAndGetServiceAsync(Nethereum.Web3.IWeb3 web3, LAUFactoryDeployment lAUFactoryDeployment, CancellationTokenSource cancellationTokenSource = null)
        {
            var receipt = await DeployContractAndWaitForReceiptAsync(web3, lAUFactoryDeployment, cancellationTokenSource);
            return new LAUFactoryService(web3, receipt.ContractAddress);
        }

        public LAUFactoryService(Nethereum.Web3.IWeb3 web3, string contractAddress) : base(web3, contractAddress)
        {
        }

        public Task<string> NewRequestAsync(NewFunction @newFunction)
        {
             return ContractHandler.SendRequestAsync(@newFunction);
        }

        public Task<TransactionReceipt> NewRequestAndWaitForReceiptAsync(NewFunction @newFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(@newFunction, cancellationToken);
        }

        public Task<string> NewRequestAsync(string name, string symbol)
        {
            var @newFunction = new NewFunction();
                @newFunction.Name = name;
                @newFunction.Symbol = symbol;
            
             return ContractHandler.SendRequestAsync(@newFunction);
        }

        public Task<TransactionReceipt> NewRequestAndWaitForReceiptAsync(string name, string symbol, CancellationTokenSource cancellationToken = null)
        {
            var @newFunction = new NewFunction();
                @newFunction.Name = name;
                @newFunction.Symbol = symbol;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(@newFunction, cancellationToken);
        }

        public Task<string> VoidQueryAsync(VoidFunction @voidFunction, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<VoidFunction, string>(@voidFunction, blockParameter);
        }

        
        public Task<string> VoidQueryAsync(BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<VoidFunction, string>(null, blockParameter);
        }

        public override List<Type> GetAllFunctionTypes()
        {
            return new List<Type>
            {
                typeof(NewFunction),
                typeof(VoidFunction)
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
