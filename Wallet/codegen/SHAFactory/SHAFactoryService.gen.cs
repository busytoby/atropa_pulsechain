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
using Dysnomia.Contracts.SHAFactory.ContractDefinition;

namespace Dysnomia.Contracts.SHAFactory
{
    public partial class SHAFactoryService: ContractWeb3ServiceBase
    {
        public static Task<TransactionReceipt> DeployContractAndWaitForReceiptAsync(Nethereum.Web3.IWeb3 web3, SHAFactoryDeployment sHAFactoryDeployment, CancellationTokenSource cancellationTokenSource = null)
        {
            return web3.Eth.GetContractDeploymentHandler<SHAFactoryDeployment>().SendRequestAndWaitForReceiptAsync(sHAFactoryDeployment, cancellationTokenSource);
        }

        public static Task<string> DeployContractAsync(Nethereum.Web3.IWeb3 web3, SHAFactoryDeployment sHAFactoryDeployment)
        {
            return web3.Eth.GetContractDeploymentHandler<SHAFactoryDeployment>().SendRequestAsync(sHAFactoryDeployment);
        }

        public static async Task<SHAFactoryService> DeployContractAndGetServiceAsync(Nethereum.Web3.IWeb3 web3, SHAFactoryDeployment sHAFactoryDeployment, CancellationTokenSource cancellationTokenSource = null)
        {
            var receipt = await DeployContractAndWaitForReceiptAsync(web3, sHAFactoryDeployment, cancellationTokenSource);
            return new SHAFactoryService(web3, receipt.ContractAddress);
        }

        public SHAFactoryService(Nethereum.Web3.IWeb3 web3, string contractAddress) : base(web3, contractAddress)
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

        public Task<string> NewRequestAsync(string name, string symbol, string mathLib)
        {
            var @newFunction = new NewFunction();
                @newFunction.Name = name;
                @newFunction.Symbol = symbol;
                @newFunction.MathLib = mathLib;
            
             return ContractHandler.SendRequestAsync(@newFunction);
        }

        public Task<TransactionReceipt> NewRequestAndWaitForReceiptAsync(string name, string symbol, string mathLib, CancellationTokenSource cancellationToken = null)
        {
            var @newFunction = new NewFunction();
                @newFunction.Name = name;
                @newFunction.Symbol = symbol;
                @newFunction.MathLib = mathLib;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(@newFunction, cancellationToken);
        }

        public override List<Type> GetAllFunctionTypes()
        {
            return new List<Type>
            {
                typeof(NewFunction)
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
