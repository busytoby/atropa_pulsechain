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
using Dysnomia.Contracts.SHIOFactory.ContractDefinition;

namespace Dysnomia.Contracts.SHIOFactory
{
    public partial class SHIOFactoryService: ContractWeb3ServiceBase
    {
        public static Task<TransactionReceipt> DeployContractAndWaitForReceiptAsync(Nethereum.Web3.IWeb3 web3, SHIOFactoryDeployment sHIOFactoryDeployment, CancellationTokenSource cancellationTokenSource = null)
        {
            return web3.Eth.GetContractDeploymentHandler<SHIOFactoryDeployment>().SendRequestAndWaitForReceiptAsync(sHIOFactoryDeployment, cancellationTokenSource);
        }

        public static Task<string> DeployContractAsync(Nethereum.Web3.IWeb3 web3, SHIOFactoryDeployment sHIOFactoryDeployment)
        {
            return web3.Eth.GetContractDeploymentHandler<SHIOFactoryDeployment>().SendRequestAsync(sHIOFactoryDeployment);
        }

        public static async Task<SHIOFactoryService> DeployContractAndGetServiceAsync(Nethereum.Web3.IWeb3 web3, SHIOFactoryDeployment sHIOFactoryDeployment, CancellationTokenSource cancellationTokenSource = null)
        {
            var receipt = await DeployContractAndWaitForReceiptAsync(web3, sHIOFactoryDeployment, cancellationTokenSource);
            return new SHIOFactoryService(web3, receipt.ContractAddress);
        }

        public SHIOFactoryService(Nethereum.Web3.IWeb3 web3, string contractAddress) : base(web3, contractAddress)
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

        public Task<string> NewRequestAsync(string rod, string cone, string mathLib)
        {
            var @newFunction = new NewFunction();
                @newFunction.Rod = rod;
                @newFunction.Cone = cone;
                @newFunction.MathLib = mathLib;
            
             return ContractHandler.SendRequestAsync(@newFunction);
        }

        public Task<TransactionReceipt> NewRequestAndWaitForReceiptAsync(string rod, string cone, string mathLib, CancellationTokenSource cancellationToken = null)
        {
            var @newFunction = new NewFunction();
                @newFunction.Rod = rod;
                @newFunction.Cone = cone;
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
