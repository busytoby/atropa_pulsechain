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
using Dysnomia.Contracts.lib.COREREACTIONSLIB.ContractDefinition;

namespace Dysnomia.Contracts.lib.COREREACTIONSLIB
{
    public partial class CorereactionslibService: ContractWeb3ServiceBase
    {
        public static Task<TransactionReceipt> DeployContractAndWaitForReceiptAsync(Nethereum.Web3.IWeb3 web3, CorereactionslibDeployment corereactionslibDeployment, CancellationTokenSource cancellationTokenSource = null)
        {
            return web3.Eth.GetContractDeploymentHandler<CorereactionslibDeployment>().SendRequestAndWaitForReceiptAsync(corereactionslibDeployment, cancellationTokenSource);
        }

        public static Task<string> DeployContractAsync(Nethereum.Web3.IWeb3 web3, CorereactionslibDeployment corereactionslibDeployment)
        {
            return web3.Eth.GetContractDeploymentHandler<CorereactionslibDeployment>().SendRequestAsync(corereactionslibDeployment);
        }

        public static async Task<CorereactionslibService> DeployContractAndGetServiceAsync(Nethereum.Web3.IWeb3 web3, CorereactionslibDeployment corereactionslibDeployment, CancellationTokenSource cancellationTokenSource = null)
        {
            var receipt = await DeployContractAndWaitForReceiptAsync(web3, corereactionslibDeployment, cancellationTokenSource);
            return new CorereactionslibService(web3, receipt.ContractAddress);
        }

        public CorereactionslibService(Nethereum.Web3.IWeb3 web3, string contractAddress) : base(web3, contractAddress)
        {
        }

        public Task<string> ChoQueryAsync(ChoFunction choFunction, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<ChoFunction, string>(choFunction, blockParameter);
        }

        
        public Task<string> ChoQueryAsync(BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<ChoFunction, string>(null, blockParameter);
        }

        public Task<string> EntropyRequestAsync(EntropyFunction entropyFunction)
        {
             return ContractHandler.SendRequestAsync(entropyFunction);
        }

        public Task<TransactionReceipt> EntropyRequestAndWaitForReceiptAsync(EntropyFunction entropyFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(entropyFunction, cancellationToken);
        }

        public Task<string> EntropyRequestAsync(Bao on)
        {
            var entropyFunction = new EntropyFunction();
                entropyFunction.On = on;
            
             return ContractHandler.SendRequestAsync(entropyFunction);
        }

        public Task<TransactionReceipt> EntropyRequestAndWaitForReceiptAsync(Bao on, CancellationTokenSource cancellationToken = null)
        {
            var entropyFunction = new EntropyFunction();
                entropyFunction.On = on;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(entropyFunction, cancellationToken);
        }

        public Task<BigInteger> GetMarketRateQueryAsync(GetMarketRateFunction getMarketRateFunction, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<GetMarketRateFunction, BigInteger>(getMarketRateFunction, blockParameter);
        }

        
        public Task<BigInteger> GetMarketRateQueryAsync(string a, BlockParameter blockParameter = null)
        {
            var getMarketRateFunction = new GetMarketRateFunction();
                getMarketRateFunction.A = a;
            
            return ContractHandler.QueryAsync<GetMarketRateFunction, BigInteger>(getMarketRateFunction, blockParameter);
        }

        public Task<string> InitializeRequestAsync(InitializeFunction initializeFunction)
        {
             return ContractHandler.SendRequestAsync(initializeFunction);
        }

        public Task<TransactionReceipt> InitializeRequestAndWaitForReceiptAsync(InitializeFunction initializeFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(initializeFunction, cancellationToken);
        }

        public Task<string> InitializeRequestAsync(Bao on)
        {
            var initializeFunction = new InitializeFunction();
                initializeFunction.On = on;
            
             return ContractHandler.SendRequestAsync(initializeFunction);
        }

        public Task<TransactionReceipt> InitializeRequestAndWaitForReceiptAsync(Bao on, CancellationTokenSource cancellationToken = null)
        {
            var initializeFunction = new InitializeFunction();
                initializeFunction.On = on;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(initializeFunction, cancellationToken);
        }

        public Task<ulong> MotzkinPrimeQueryAsync(MotzkinPrimeFunction motzkinPrimeFunction, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<MotzkinPrimeFunction, ulong>(motzkinPrimeFunction, blockParameter);
        }

        
        public Task<ulong> MotzkinPrimeQueryAsync(BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<MotzkinPrimeFunction, ulong>(null, blockParameter);
        }

        public Task<string> OperatorReactRequestAsync(OperatorReactFunction operatorReactFunction)
        {
             return ContractHandler.SendRequestAsync(operatorReactFunction);
        }

        public Task<TransactionReceipt> OperatorReactRequestAndWaitForReceiptAsync(OperatorReactFunction operatorReactFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(operatorReactFunction, cancellationToken);
        }

        public Task<string> OperatorReactRequestAsync(Bao on, ulong omega)
        {
            var operatorReactFunction = new OperatorReactFunction();
                operatorReactFunction.On = on;
                operatorReactFunction.Omega = omega;
            
             return ContractHandler.SendRequestAsync(operatorReactFunction);
        }

        public Task<TransactionReceipt> OperatorReactRequestAndWaitForReceiptAsync(Bao on, ulong omega, CancellationTokenSource cancellationToken = null)
        {
            var operatorReactFunction = new OperatorReactFunction();
                operatorReactFunction.On = on;
                operatorReactFunction.Omega = omega;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(operatorReactFunction, cancellationToken);
        }

        public Task<string> PurchaseRequestAsync(PurchaseFunction purchaseFunction)
        {
             return ContractHandler.SendRequestAsync(purchaseFunction);
        }

        public Task<TransactionReceipt> PurchaseRequestAndWaitForReceiptAsync(PurchaseFunction purchaseFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(purchaseFunction, cancellationToken);
        }

        public Task<string> PurchaseRequestAsync(string t, BigInteger a)
        {
            var purchaseFunction = new PurchaseFunction();
                purchaseFunction.T = t;
                purchaseFunction.A = a;
            
             return ContractHandler.SendRequestAsync(purchaseFunction);
        }

        public Task<TransactionReceipt> PurchaseRequestAndWaitForReceiptAsync(string t, BigInteger a, CancellationTokenSource cancellationToken = null)
        {
            var purchaseFunction = new PurchaseFunction();
                purchaseFunction.T = t;
                purchaseFunction.A = a;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(purchaseFunction, cancellationToken);
        }

        public Task<string> ReactRequestAsync(ReactFunction reactFunction)
        {
             return ContractHandler.SendRequestAsync(reactFunction);
        }

        public Task<TransactionReceipt> ReactRequestAndWaitForReceiptAsync(ReactFunction reactFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(reactFunction, cancellationToken);
        }

        public Task<string> ReactRequestAsync(Bao on, ulong omicron, ulong omega)
        {
            var reactFunction = new ReactFunction();
                reactFunction.On = on;
                reactFunction.Omicron = omicron;
                reactFunction.Omega = omega;
            
             return ContractHandler.SendRequestAsync(reactFunction);
        }

        public Task<TransactionReceipt> ReactRequestAndWaitForReceiptAsync(Bao on, ulong omicron, ulong omega, CancellationTokenSource cancellationToken = null)
        {
            var reactFunction = new ReactFunction();
                reactFunction.On = on;
                reactFunction.Omicron = omicron;
                reactFunction.Omega = omega;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(reactFunction, cancellationToken);
        }

        public Task<string> ReactBangRequestAsync(ReactBangFunction reactBangFunction)
        {
             return ContractHandler.SendRequestAsync(reactBangFunction);
        }

        public Task<TransactionReceipt> ReactBangRequestAndWaitForReceiptAsync(ReactBangFunction reactBangFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(reactBangFunction, cancellationToken);
        }

        public Task<string> ReactBangRequestAsync(ulong eta)
        {
            var reactBangFunction = new ReactBangFunction();
                reactBangFunction.Eta = eta;
            
             return ContractHandler.SendRequestAsync(reactBangFunction);
        }

        public Task<TransactionReceipt> ReactBangRequestAndWaitForReceiptAsync(ulong eta, CancellationTokenSource cancellationToken = null)
        {
            var reactBangFunction = new ReactBangFunction();
                reactBangFunction.Eta = eta;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(reactBangFunction, cancellationToken);
        }

        public Task<string> ReactErisRequestAsync(ReactErisFunction reactErisFunction)
        {
             return ContractHandler.SendRequestAsync(reactErisFunction);
        }

        public Task<TransactionReceipt> ReactErisRequestAndWaitForReceiptAsync(ReactErisFunction reactErisFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(reactErisFunction, cancellationToken);
        }

        public Task<string> ReactErisRequestAsync(ulong iota)
        {
            var reactErisFunction = new ReactErisFunction();
                reactErisFunction.Iota = iota;
            
             return ContractHandler.SendRequestAsync(reactErisFunction);
        }

        public Task<TransactionReceipt> ReactErisRequestAndWaitForReceiptAsync(ulong iota, CancellationTokenSource cancellationToken = null)
        {
            var reactErisFunction = new ReactErisFunction();
                reactErisFunction.Iota = iota;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(reactErisFunction, cancellationToken);
        }

        public Task<string> ReactFomalhauteRequestAsync(ReactFomalhauteFunction reactFomalhauteFunction)
        {
             return ContractHandler.SendRequestAsync(reactFomalhauteFunction);
        }

        public Task<TransactionReceipt> ReactFomalhauteRequestAndWaitForReceiptAsync(ReactFomalhauteFunction reactFomalhauteFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(reactFomalhauteFunction, cancellationToken);
        }

        public Task<string> ReactFomalhauteRequestAsync(ulong mu)
        {
            var reactFomalhauteFunction = new ReactFomalhauteFunction();
                reactFomalhauteFunction.Mu = mu;
            
             return ContractHandler.SendRequestAsync(reactFomalhauteFunction);
        }

        public Task<TransactionReceipt> ReactFomalhauteRequestAndWaitForReceiptAsync(ulong mu, CancellationTokenSource cancellationToken = null)
        {
            var reactFomalhauteFunction = new ReactFomalhauteFunction();
                reactFomalhauteFunction.Mu = mu;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(reactFomalhauteFunction, cancellationToken);
        }

        public Task<string> ReactFornaxRequestAsync(ReactFornaxFunction reactFornaxFunction)
        {
             return ContractHandler.SendRequestAsync(reactFornaxFunction);
        }

        public Task<TransactionReceipt> ReactFornaxRequestAndWaitForReceiptAsync(ReactFornaxFunction reactFornaxFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(reactFornaxFunction, cancellationToken);
        }

        public Task<string> ReactFornaxRequestAsync(ulong rho)
        {
            var reactFornaxFunction = new ReactFornaxFunction();
                reactFornaxFunction.Rho = rho;
            
             return ContractHandler.SendRequestAsync(reactFornaxFunction);
        }

        public Task<TransactionReceipt> ReactFornaxRequestAndWaitForReceiptAsync(ulong rho, CancellationTokenSource cancellationToken = null)
        {
            var reactFornaxFunction = new ReactFornaxFunction();
                reactFornaxFunction.Rho = rho;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(reactFornaxFunction, cancellationToken);
        }

        public Task<string> ReactLaiRequestAsync(ReactLaiFunction reactLaiFunction)
        {
             return ContractHandler.SendRequestAsync(reactLaiFunction);
        }

        public Task<TransactionReceipt> ReactLaiRequestAndWaitForReceiptAsync(ReactLaiFunction reactLaiFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(reactLaiFunction, cancellationToken);
        }

        public Task<string> ReactLaiRequestAsync(ulong gamma)
        {
            var reactLaiFunction = new ReactLaiFunction();
                reactLaiFunction.Gamma = gamma;
            
             return ContractHandler.SendRequestAsync(reactLaiFunction);
        }

        public Task<TransactionReceipt> ReactLaiRequestAndWaitForReceiptAsync(ulong gamma, CancellationTokenSource cancellationToken = null)
        {
            var reactLaiFunction = new ReactLaiFunction();
                reactLaiFunction.Gamma = gamma;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(reactLaiFunction, cancellationToken);
        }

        public Task<string> ReactLeRequestAsync(ReactLeFunction reactLeFunction)
        {
             return ContractHandler.SendRequestAsync(reactLeFunction);
        }

        public Task<TransactionReceipt> ReactLeRequestAndWaitForReceiptAsync(ReactLeFunction reactLeFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(reactLeFunction, cancellationToken);
        }

        public Task<string> ReactLeRequestAsync(ulong delta)
        {
            var reactLeFunction = new ReactLeFunction();
                reactLeFunction.Delta = delta;
            
             return ContractHandler.SendRequestAsync(reactLeFunction);
        }

        public Task<TransactionReceipt> ReactLeRequestAndWaitForReceiptAsync(ulong delta, CancellationTokenSource cancellationToken = null)
        {
            var reactLeFunction = new ReactLeFunction();
                reactLeFunction.Delta = delta;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(reactLeFunction, cancellationToken);
        }

        public Task<string> ReactShioConeRequestAsync(ReactShioConeFunction reactShioConeFunction)
        {
             return ContractHandler.SendRequestAsync(reactShioConeFunction);
        }

        public Task<TransactionReceipt> ReactShioConeRequestAndWaitForReceiptAsync(ReactShioConeFunction reactShioConeFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(reactShioConeFunction, cancellationToken);
        }

        public Task<string> ReactShioConeRequestAsync(string beta, ulong theta)
        {
            var reactShioConeFunction = new ReactShioConeFunction();
                reactShioConeFunction.Beta = beta;
                reactShioConeFunction.Theta = theta;
            
             return ContractHandler.SendRequestAsync(reactShioConeFunction);
        }

        public Task<TransactionReceipt> ReactShioConeRequestAndWaitForReceiptAsync(string beta, ulong theta, CancellationTokenSource cancellationToken = null)
        {
            var reactShioConeFunction = new ReactShioConeFunction();
                reactShioConeFunction.Beta = beta;
                reactShioConeFunction.Theta = theta;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(reactShioConeFunction, cancellationToken);
        }

        public Task<string> ReactShioRodRequestAsync(ReactShioRodFunction reactShioRodFunction)
        {
             return ContractHandler.SendRequestAsync(reactShioRodFunction);
        }

        public Task<TransactionReceipt> ReactShioRodRequestAndWaitForReceiptAsync(ReactShioRodFunction reactShioRodFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(reactShioRodFunction, cancellationToken);
        }

        public Task<string> ReactShioRodRequestAsync(string beta, ulong theta)
        {
            var reactShioRodFunction = new ReactShioRodFunction();
                reactShioRodFunction.Beta = beta;
                reactShioRodFunction.Theta = theta;
            
             return ContractHandler.SendRequestAsync(reactShioRodFunction);
        }

        public Task<TransactionReceipt> ReactShioRodRequestAndWaitForReceiptAsync(string beta, ulong theta, CancellationTokenSource cancellationToken = null)
        {
            var reactShioRodFunction = new ReactShioRodFunction();
                reactShioRodFunction.Beta = beta;
                reactShioRodFunction.Theta = theta;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(reactShioRodFunction, cancellationToken);
        }

        public Task<string> ReactToBangRequestAsync(ReactToBangFunction reactToBangFunction)
        {
             return ContractHandler.SendRequestAsync(reactToBangFunction);
        }

        public Task<TransactionReceipt> ReactToBangRequestAndWaitForReceiptAsync(ReactToBangFunction reactToBangFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(reactToBangFunction, cancellationToken);
        }

        public Task<string> ReactToBangRequestAsync(Bao on, ulong omicron, ulong omega)
        {
            var reactToBangFunction = new ReactToBangFunction();
                reactToBangFunction.On = on;
                reactToBangFunction.Omicron = omicron;
                reactToBangFunction.Omega = omega;
            
             return ContractHandler.SendRequestAsync(reactToBangFunction);
        }

        public Task<TransactionReceipt> ReactToBangRequestAndWaitForReceiptAsync(Bao on, ulong omicron, ulong omega, CancellationTokenSource cancellationToken = null)
        {
            var reactToBangFunction = new ReactToBangFunction();
                reactToBangFunction.On = on;
                reactToBangFunction.Omicron = omicron;
                reactToBangFunction.Omega = omega;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(reactToBangFunction, cancellationToken);
        }

        public Task<string> ReactToLaiRequestAsync(ReactToLaiFunction reactToLaiFunction)
        {
             return ContractHandler.SendRequestAsync(reactToLaiFunction);
        }

        public Task<TransactionReceipt> ReactToLaiRequestAndWaitForReceiptAsync(ReactToLaiFunction reactToLaiFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(reactToLaiFunction, cancellationToken);
        }

        public Task<string> ReactToLaiRequestAsync(Bao on, ulong omicron, ulong omega)
        {
            var reactToLaiFunction = new ReactToLaiFunction();
                reactToLaiFunction.On = on;
                reactToLaiFunction.Omicron = omicron;
                reactToLaiFunction.Omega = omega;
            
             return ContractHandler.SendRequestAsync(reactToLaiFunction);
        }

        public Task<TransactionReceipt> ReactToLaiRequestAndWaitForReceiptAsync(Bao on, ulong omicron, ulong omega, CancellationTokenSource cancellationToken = null)
        {
            var reactToLaiFunction = new ReactToLaiFunction();
                reactToLaiFunction.On = on;
                reactToLaiFunction.Omicron = omicron;
                reactToLaiFunction.Omega = omega;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(reactToLaiFunction, cancellationToken);
        }

        public Task<string> ReactToLeRequestAsync(ReactToLeFunction reactToLeFunction)
        {
             return ContractHandler.SendRequestAsync(reactToLeFunction);
        }

        public Task<TransactionReceipt> ReactToLeRequestAndWaitForReceiptAsync(ReactToLeFunction reactToLeFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(reactToLeFunction, cancellationToken);
        }

        public Task<string> ReactToLeRequestAsync(Bao on, ulong omicron, ulong omega)
        {
            var reactToLeFunction = new ReactToLeFunction();
                reactToLeFunction.On = on;
                reactToLeFunction.Omicron = omicron;
                reactToLeFunction.Omega = omega;
            
             return ContractHandler.SendRequestAsync(reactToLeFunction);
        }

        public Task<TransactionReceipt> ReactToLeRequestAndWaitForReceiptAsync(Bao on, ulong omicron, ulong omega, CancellationTokenSource cancellationToken = null)
        {
            var reactToLeFunction = new ReactToLeFunction();
                reactToLeFunction.On = on;
                reactToLeFunction.Omicron = omicron;
                reactToLeFunction.Omega = omega;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(reactToLeFunction, cancellationToken);
        }

        public Task<string> ReactToNewRequestAsync(ReactToNewFunction reactToNewFunction)
        {
             return ContractHandler.SendRequestAsync(reactToNewFunction);
        }

        public Task<TransactionReceipt> ReactToNewRequestAndWaitForReceiptAsync(ReactToNewFunction reactToNewFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(reactToNewFunction, cancellationToken);
        }

        public Task<string> ReactToNewRequestAsync(Bao on, List<ulong> saat)
        {
            var reactToNewFunction = new ReactToNewFunction();
                reactToNewFunction.On = on;
                reactToNewFunction.Saat = saat;
            
             return ContractHandler.SendRequestAsync(reactToNewFunction);
        }

        public Task<TransactionReceipt> ReactToNewRequestAndWaitForReceiptAsync(Bao on, List<ulong> saat, CancellationTokenSource cancellationToken = null)
        {
            var reactToNewFunction = new ReactToNewFunction();
                reactToNewFunction.On = on;
                reactToNewFunction.Saat = saat;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(reactToNewFunction, cancellationToken);
        }

        public Task<string> ReactToTalkRequestAsync(ReactToTalkFunction reactToTalkFunction)
        {
             return ContractHandler.SendRequestAsync(reactToTalkFunction);
        }

        public Task<TransactionReceipt> ReactToTalkRequestAndWaitForReceiptAsync(ReactToTalkFunction reactToTalkFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(reactToTalkFunction, cancellationToken);
        }

        public Task<string> ReactToTalkRequestAsync(User alpha)
        {
            var reactToTalkFunction = new ReactToTalkFunction();
                reactToTalkFunction.Alpha = alpha;
            
             return ContractHandler.SendRequestAsync(reactToTalkFunction);
        }

        public Task<TransactionReceipt> ReactToTalkRequestAndWaitForReceiptAsync(User alpha, CancellationTokenSource cancellationToken = null)
        {
            var reactToTalkFunction = new ReactToTalkFunction();
                reactToTalkFunction.Alpha = alpha;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(reactToTalkFunction, cancellationToken);
        }

        public Task<string> RedeemRequestAsync(RedeemFunction redeemFunction)
        {
             return ContractHandler.SendRequestAsync(redeemFunction);
        }

        public Task<TransactionReceipt> RedeemRequestAndWaitForReceiptAsync(RedeemFunction redeemFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(redeemFunction, cancellationToken);
        }

        public Task<string> RedeemRequestAsync(string t, BigInteger a)
        {
            var redeemFunction = new RedeemFunction();
                redeemFunction.T = t;
                redeemFunction.A = a;
            
             return ContractHandler.SendRequestAsync(redeemFunction);
        }

        public Task<TransactionReceipt> RedeemRequestAndWaitForReceiptAsync(string t, BigInteger a, CancellationTokenSource cancellationToken = null)
        {
            var redeemFunction = new RedeemFunction();
                redeemFunction.T = t;
                redeemFunction.A = a;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(redeemFunction, cancellationToken);
        }

        public Task<string> RegisterChoForTalkRequestAsync(RegisterChoForTalkFunction registerChoForTalkFunction)
        {
             return ContractHandler.SendRequestAsync(registerChoForTalkFunction);
        }

        public Task<TransactionReceipt> RegisterChoForTalkRequestAndWaitForReceiptAsync(RegisterChoForTalkFunction registerChoForTalkFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(registerChoForTalkFunction, cancellationToken);
        }

        public Task<string> RegisterChoForTalkRequestAsync(string choAddress)
        {
            var registerChoForTalkFunction = new RegisterChoForTalkFunction();
                registerChoForTalkFunction.ChoAddress = choAddress;
            
             return ContractHandler.SendRequestAsync(registerChoForTalkFunction);
        }

        public Task<TransactionReceipt> RegisterChoForTalkRequestAndWaitForReceiptAsync(string choAddress, CancellationTokenSource cancellationToken = null)
        {
            var registerChoForTalkFunction = new RegisterChoForTalkFunction();
                registerChoForTalkFunction.ChoAddress = choAddress;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(registerChoForTalkFunction, cancellationToken);
        }

        public Task<string> RenameRequestAsync(RenameFunction renameFunction)
        {
             return ContractHandler.SendRequestAsync(renameFunction);
        }

        public Task<TransactionReceipt> RenameRequestAndWaitForReceiptAsync(RenameFunction renameFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(renameFunction, cancellationToken);
        }

        public Task<string> RenameRequestAsync(string newName, string newSymbol)
        {
            var renameFunction = new RenameFunction();
                renameFunction.NewName = newName;
                renameFunction.NewSymbol = newSymbol;
            
             return ContractHandler.SendRequestAsync(renameFunction);
        }

        public Task<TransactionReceipt> RenameRequestAndWaitForReceiptAsync(string newName, string newSymbol, CancellationTokenSource cancellationToken = null)
        {
            var renameFunction = new RenameFunction();
                renameFunction.NewName = newName;
                renameFunction.NewSymbol = newSymbol;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(renameFunction, cancellationToken);
        }

        public Task<string> TypeQueryAsync(TypeFunction typeFunction, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<TypeFunction, string>(typeFunction, blockParameter);
        }

        
        public Task<string> TypeQueryAsync(BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<TypeFunction, string>(null, blockParameter);
        }

        public Task<string> VoidQueryAsync(VoidFunction @voidFunction, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<VoidFunction, string>(@voidFunction, blockParameter);
        }

        
        public Task<string> VoidQueryAsync(BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<VoidFunction, string>(null, blockParameter);
        }

        public Task<string> XiaoQueryAsync(XiaoFunction xiaoFunction, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<XiaoFunction, string>(xiaoFunction, blockParameter);
        }

        
        public Task<string> XiaoQueryAsync(BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<XiaoFunction, string>(null, blockParameter);
        }

        public Task<string> AddOwnerRequestAsync(AddOwnerFunction addOwnerFunction)
        {
             return ContractHandler.SendRequestAsync(addOwnerFunction);
        }

        public Task<TransactionReceipt> AddOwnerRequestAndWaitForReceiptAsync(AddOwnerFunction addOwnerFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(addOwnerFunction, cancellationToken);
        }

        public Task<string> AddOwnerRequestAsync(string newOwner)
        {
            var addOwnerFunction = new AddOwnerFunction();
                addOwnerFunction.NewOwner = newOwner;
            
             return ContractHandler.SendRequestAsync(addOwnerFunction);
        }

        public Task<TransactionReceipt> AddOwnerRequestAndWaitForReceiptAsync(string newOwner, CancellationTokenSource cancellationToken = null)
        {
            var addOwnerFunction = new AddOwnerFunction();
                addOwnerFunction.NewOwner = newOwner;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(addOwnerFunction, cancellationToken);
        }

        public Task<BigInteger> AllowanceQueryAsync(AllowanceFunction allowanceFunction, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<AllowanceFunction, BigInteger>(allowanceFunction, blockParameter);
        }

        
        public Task<BigInteger> AllowanceQueryAsync(string owner, string spender, BlockParameter blockParameter = null)
        {
            var allowanceFunction = new AllowanceFunction();
                allowanceFunction.Owner = owner;
                allowanceFunction.Spender = spender;
            
            return ContractHandler.QueryAsync<AllowanceFunction, BigInteger>(allowanceFunction, blockParameter);
        }

        public Task<string> ApproveRequestAsync(ApproveFunction approveFunction)
        {
             return ContractHandler.SendRequestAsync(approveFunction);
        }

        public Task<TransactionReceipt> ApproveRequestAndWaitForReceiptAsync(ApproveFunction approveFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(approveFunction, cancellationToken);
        }

        public Task<string> ApproveRequestAsync(string spender, BigInteger value)
        {
            var approveFunction = new ApproveFunction();
                approveFunction.Spender = spender;
                approveFunction.Value = value;
            
             return ContractHandler.SendRequestAsync(approveFunction);
        }

        public Task<TransactionReceipt> ApproveRequestAndWaitForReceiptAsync(string spender, BigInteger value, CancellationTokenSource cancellationToken = null)
        {
            var approveFunction = new ApproveFunction();
                approveFunction.Spender = spender;
                approveFunction.Value = value;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(approveFunction, cancellationToken);
        }

        public Task<BigInteger> BalanceOfQueryAsync(BalanceOfFunction balanceOfFunction, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<BalanceOfFunction, BigInteger>(balanceOfFunction, blockParameter);
        }

        
        public Task<BigInteger> BalanceOfQueryAsync(string account, BlockParameter blockParameter = null)
        {
            var balanceOfFunction = new BalanceOfFunction();
                balanceOfFunction.Account = account;
            
            return ContractHandler.QueryAsync<BalanceOfFunction, BigInteger>(balanceOfFunction, blockParameter);
        }

        public Task<byte> DecimalsQueryAsync(DecimalsFunction decimalsFunction, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<DecimalsFunction, byte>(decimalsFunction, blockParameter);
        }

        
        public Task<byte> DecimalsQueryAsync(BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<DecimalsFunction, byte>(null, blockParameter);
        }

        public Task<BigInteger> MaxSupplyQueryAsync(MaxSupplyFunction maxSupplyFunction, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<MaxSupplyFunction, BigInteger>(maxSupplyFunction, blockParameter);
        }

        
        public Task<BigInteger> MaxSupplyQueryAsync(BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<MaxSupplyFunction, BigInteger>(null, blockParameter);
        }

        public Task<string> MintToCapRequestAsync(MintToCapFunction mintToCapFunction)
        {
             return ContractHandler.SendRequestAsync(mintToCapFunction);
        }

        public Task<string> MintToCapRequestAsync()
        {
             return ContractHandler.SendRequestAsync<MintToCapFunction>();
        }

        public Task<TransactionReceipt> MintToCapRequestAndWaitForReceiptAsync(MintToCapFunction mintToCapFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(mintToCapFunction, cancellationToken);
        }

        public Task<TransactionReceipt> MintToCapRequestAndWaitForReceiptAsync(CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync<MintToCapFunction>(null, cancellationToken);
        }

        public Task<string> NameQueryAsync(NameFunction nameFunction, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<NameFunction, string>(nameFunction, blockParameter);
        }

        
        public Task<string> NameQueryAsync(BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<NameFunction, string>(null, blockParameter);
        }

        public Task<bool> OwnerQueryAsync(Owner1Function owner1Function, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<Owner1Function, bool>(owner1Function, blockParameter);
        }

        
        public Task<bool> OwnerQueryAsync(string cOwner, BlockParameter blockParameter = null)
        {
            var owner1Function = new Owner1Function();
                owner1Function.COwner = cOwner;
            
            return ContractHandler.QueryAsync<Owner1Function, bool>(owner1Function, blockParameter);
        }

        public Task<string> OwnerQueryAsync(OwnerFunction ownerFunction, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<OwnerFunction, string>(ownerFunction, blockParameter);
        }

        
        public Task<string> OwnerQueryAsync(BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<OwnerFunction, string>(null, blockParameter);
        }

        public Task<string> RenounceOwnershipRequestAsync(RenounceOwnershipFunction renounceOwnershipFunction)
        {
             return ContractHandler.SendRequestAsync(renounceOwnershipFunction);
        }

        public Task<TransactionReceipt> RenounceOwnershipRequestAndWaitForReceiptAsync(RenounceOwnershipFunction renounceOwnershipFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(renounceOwnershipFunction, cancellationToken);
        }

        public Task<string> RenounceOwnershipRequestAsync(string toRemove)
        {
            var renounceOwnershipFunction = new RenounceOwnershipFunction();
                renounceOwnershipFunction.ToRemove = toRemove;
            
             return ContractHandler.SendRequestAsync(renounceOwnershipFunction);
        }

        public Task<TransactionReceipt> RenounceOwnershipRequestAndWaitForReceiptAsync(string toRemove, CancellationTokenSource cancellationToken = null)
        {
            var renounceOwnershipFunction = new RenounceOwnershipFunction();
                renounceOwnershipFunction.ToRemove = toRemove;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(renounceOwnershipFunction, cancellationToken);
        }

        public Task<string> SymbolQueryAsync(SymbolFunction symbolFunction, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<SymbolFunction, string>(symbolFunction, blockParameter);
        }

        
        public Task<string> SymbolQueryAsync(BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<SymbolFunction, string>(null, blockParameter);
        }

        public Task<BigInteger> TotalSupplyQueryAsync(TotalSupplyFunction totalSupplyFunction, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<TotalSupplyFunction, BigInteger>(totalSupplyFunction, blockParameter);
        }

        
        public Task<BigInteger> TotalSupplyQueryAsync(BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<TotalSupplyFunction, BigInteger>(null, blockParameter);
        }

        public Task<string> TransferRequestAsync(TransferFunction transferFunction)
        {
             return ContractHandler.SendRequestAsync(transferFunction);
        }

        public Task<TransactionReceipt> TransferRequestAndWaitForReceiptAsync(TransferFunction transferFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(transferFunction, cancellationToken);
        }

        public Task<string> TransferRequestAsync(string to, BigInteger value)
        {
            var transferFunction = new TransferFunction();
                transferFunction.To = to;
                transferFunction.Value = value;
            
             return ContractHandler.SendRequestAsync(transferFunction);
        }

        public Task<TransactionReceipt> TransferRequestAndWaitForReceiptAsync(string to, BigInteger value, CancellationTokenSource cancellationToken = null)
        {
            var transferFunction = new TransferFunction();
                transferFunction.To = to;
                transferFunction.Value = value;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(transferFunction, cancellationToken);
        }

        public Task<string> TransferFromRequestAsync(TransferFromFunction transferFromFunction)
        {
             return ContractHandler.SendRequestAsync(transferFromFunction);
        }

        public Task<TransactionReceipt> TransferFromRequestAndWaitForReceiptAsync(TransferFromFunction transferFromFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(transferFromFunction, cancellationToken);
        }

        public Task<string> TransferFromRequestAsync(string from, string to, BigInteger value)
        {
            var transferFromFunction = new TransferFromFunction();
                transferFromFunction.From = from;
                transferFromFunction.To = to;
                transferFromFunction.Value = value;
            
             return ContractHandler.SendRequestAsync(transferFromFunction);
        }

        public Task<TransactionReceipt> TransferFromRequestAndWaitForReceiptAsync(string from, string to, BigInteger value, CancellationTokenSource cancellationToken = null)
        {
            var transferFromFunction = new TransferFromFunction();
                transferFromFunction.From = from;
                transferFromFunction.To = to;
                transferFromFunction.Value = value;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(transferFromFunction, cancellationToken);
        }

        public override List<Type> GetAllFunctionTypes()
        {
            return new List<Type>
            {
                typeof(ChoFunction),
                typeof(EntropyFunction),
                typeof(GetMarketRateFunction),
                typeof(InitializeFunction),
                typeof(MotzkinPrimeFunction),
                typeof(OperatorReactFunction),
                typeof(PurchaseFunction),
                typeof(ReactFunction),
                typeof(ReactBangFunction),
                typeof(ReactErisFunction),
                typeof(ReactFomalhauteFunction),
                typeof(ReactFornaxFunction),
                typeof(ReactLaiFunction),
                typeof(ReactLeFunction),
                typeof(ReactShioConeFunction),
                typeof(ReactShioRodFunction),
                typeof(ReactToBangFunction),
                typeof(ReactToLaiFunction),
                typeof(ReactToLeFunction),
                typeof(ReactToNewFunction),
                typeof(ReactToTalkFunction),
                typeof(RedeemFunction),
                typeof(RegisterChoForTalkFunction),
                typeof(RenameFunction),
                typeof(TypeFunction),
                typeof(VoidFunction),
                typeof(XiaoFunction),
                typeof(AddOwnerFunction),
                typeof(AllowanceFunction),
                typeof(ApproveFunction),
                typeof(BalanceOfFunction),
                typeof(DecimalsFunction),
                typeof(MaxSupplyFunction),
                typeof(MintToCapFunction),
                typeof(NameFunction),
                typeof(Owner1Function),
                typeof(OwnerFunction),
                typeof(RenounceOwnershipFunction),
                typeof(SymbolFunction),
                typeof(TotalSupplyFunction),
                typeof(TransferFunction),
                typeof(TransferFromFunction)
            };
        }

        public override List<Type> GetAllEventTypes()
        {
            return new List<Type>
            {
                typeof(ApprovalEventDTO),
                typeof(OwnershipUpdateEventDTO),
                typeof(TransferEventDTO)
            };
        }

        public override List<Type> GetAllErrorTypes()
        {
            return new List<Type>
            {
                typeof(DysnomiaInsufficientAllowanceError),
                typeof(DysnomiaInsufficientBalanceError),
                typeof(MarketRateNotFoundError),
                typeof(OwnableInvalidOwnerError),
                typeof(OwnableUnauthorizedAccountError)
            };
        }
    }
}
