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
using Dysnomia.Contracts.VOID.ContractDefinition;

namespace Dysnomia.Contracts.VOID
{
    public partial class VoidService: ContractWeb3ServiceBase
    {
        public static Task<TransactionReceipt> DeployContractAndWaitForReceiptAsync(Nethereum.Web3.IWeb3 web3, VoidDeployment @voidDeployment, CancellationTokenSource cancellationTokenSource = null)
        {
            return web3.Eth.GetContractDeploymentHandler<VoidDeployment>().SendRequestAndWaitForReceiptAsync(@voidDeployment, cancellationTokenSource);
        }

        public static Task<string> DeployContractAsync(Nethereum.Web3.IWeb3 web3, VoidDeployment @voidDeployment)
        {
            return web3.Eth.GetContractDeploymentHandler<VoidDeployment>().SendRequestAsync(@voidDeployment);
        }

        public static async Task<VoidService> DeployContractAndGetServiceAsync(Nethereum.Web3.IWeb3 web3, VoidDeployment @voidDeployment, CancellationTokenSource cancellationTokenSource = null)
        {
            var receipt = await DeployContractAndWaitForReceiptAsync(web3, @voidDeployment, cancellationTokenSource);
            return new VoidService(web3, receipt.ContractAddress);
        }

        public VoidService(Nethereum.Web3.IWeb3 web3, string contractAddress) : base(web3, contractAddress)
        {
        }

        public Task<string> AddLibraryRequestAsync(AddLibraryFunction addLibraryFunction)
        {
             return ContractHandler.SendRequestAsync(addLibraryFunction);
        }

        public Task<TransactionReceipt> AddLibraryRequestAndWaitForReceiptAsync(AddLibraryFunction addLibraryFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(addLibraryFunction, cancellationToken);
        }

        public Task<string> AddLibraryRequestAsync(string name, string a)
        {
            var addLibraryFunction = new AddLibraryFunction();
                addLibraryFunction.Name = name;
                addLibraryFunction.A = a;
            
             return ContractHandler.SendRequestAsync(addLibraryFunction);
        }

        public Task<TransactionReceipt> AddLibraryRequestAndWaitForReceiptAsync(string name, string a, CancellationTokenSource cancellationToken = null)
        {
            var addLibraryFunction = new AddLibraryFunction();
                addLibraryFunction.Name = name;
                addLibraryFunction.A = a;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(addLibraryFunction, cancellationToken);
        }

        public Task<string> AliasRequestAsync(Alias2Function alias2Function)
        {
             return ContractHandler.SendRequestAsync(alias2Function);
        }

        public Task<TransactionReceipt> AliasRequestAndWaitForReceiptAsync(Alias2Function alias2Function, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(alias2Function, cancellationToken);
        }

        public Task<string> AliasRequestAsync(string name, string value)
        {
            var alias2Function = new Alias2Function();
                alias2Function.Name = name;
                alias2Function.Value = value;
            
             return ContractHandler.SendRequestAsync(alias2Function);
        }

        public Task<TransactionReceipt> AliasRequestAndWaitForReceiptAsync(string name, string value, CancellationTokenSource cancellationToken = null)
        {
            var alias2Function = new Alias2Function();
                alias2Function.Name = name;
                alias2Function.Value = value;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(alias2Function, cancellationToken);
        }

        public Task<string> AliasQueryAsync(AliasFunction aliasFunction, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<AliasFunction, string>(aliasFunction, blockParameter);
        }

        
        public Task<string> AliasQueryAsync(Bao theta, BlockParameter blockParameter = null)
        {
            var aliasFunction = new AliasFunction();
                aliasFunction.Theta = theta;
            
            return ContractHandler.QueryAsync<AliasFunction, string>(aliasFunction, blockParameter);
        }

        public Task<string> AliasRequestAsync(Alias3Function alias3Function)
        {
             return ContractHandler.SendRequestAsync(alias3Function);
        }

        public Task<TransactionReceipt> AliasRequestAndWaitForReceiptAsync(Alias3Function alias3Function, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(alias3Function, cancellationToken);
        }

        public Task<string> AliasRequestAsync(Bao theta, string value)
        {
            var alias3Function = new Alias3Function();
                alias3Function.Theta = theta;
                alias3Function.Value = value;
            
             return ContractHandler.SendRequestAsync(alias3Function);
        }

        public Task<TransactionReceipt> AliasRequestAndWaitForReceiptAsync(Bao theta, string value, CancellationTokenSource cancellationToken = null)
        {
            var alias3Function = new Alias3Function();
                alias3Function.Theta = theta;
                alias3Function.Value = value;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(alias3Function, cancellationToken);
        }

        public Task<string> AliasQueryAsync(Alias1Function alias1Function, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<Alias1Function, string>(alias1Function, blockParameter);
        }

        
        public Task<string> AliasQueryAsync(string name, BlockParameter blockParameter = null)
        {
            var alias1Function = new Alias1Function();
                alias1Function.Name = name;
            
            return ContractHandler.QueryAsync<Alias1Function, string>(alias1Function, blockParameter);
        }

        public Task<string> ChatRequestAsync(ChatFunction chatFunction)
        {
             return ContractHandler.SendRequestAsync(chatFunction);
        }

        public Task<TransactionReceipt> ChatRequestAndWaitForReceiptAsync(ChatFunction chatFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(chatFunction, cancellationToken);
        }

        public Task<string> ChatRequestAsync(string chatline)
        {
            var chatFunction = new ChatFunction();
                chatFunction.Chatline = chatline;
            
             return ContractHandler.SendRequestAsync(chatFunction);
        }

        public Task<TransactionReceipt> ChatRequestAndWaitForReceiptAsync(string chatline, CancellationTokenSource cancellationToken = null)
        {
            var chatFunction = new ChatFunction();
                chatFunction.Chatline = chatline;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(chatFunction, cancellationToken);
        }

        public Task<string> EnterRequestAsync(EnterFunction enterFunction)
        {
             return ContractHandler.SendRequestAsync(enterFunction);
        }

        public Task<string> EnterRequestAsync()
        {
             return ContractHandler.SendRequestAsync<EnterFunction>();
        }

        public Task<TransactionReceipt> EnterRequestAndWaitForReceiptAsync(EnterFunction enterFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(enterFunction, cancellationToken);
        }

        public Task<TransactionReceipt> EnterRequestAndWaitForReceiptAsync(CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync<EnterFunction>(null, cancellationToken);
        }

        public Task<string> EnterRequestAsync(Enter1Function enter1Function)
        {
             return ContractHandler.SendRequestAsync(enter1Function);
        }

        public Task<TransactionReceipt> EnterRequestAndWaitForReceiptAsync(Enter1Function enter1Function, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(enter1Function, cancellationToken);
        }

        public Task<string> EnterRequestAsync(string name, string symbol)
        {
            var enter1Function = new Enter1Function();
                enter1Function.Name = name;
                enter1Function.Symbol = symbol;
            
             return ContractHandler.SendRequestAsync(enter1Function);
        }

        public Task<TransactionReceipt> EnterRequestAndWaitForReceiptAsync(string name, string symbol, CancellationTokenSource cancellationToken = null)
        {
            var enter1Function = new Enter1Function();
                enter1Function.Name = name;
                enter1Function.Symbol = symbol;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(enter1Function, cancellationToken);
        }

        public Task<string> GetAttributeQueryAsync(GetAttributeFunction getAttributeFunction, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<GetAttributeFunction, string>(getAttributeFunction, blockParameter);
        }

        
        public Task<string> GetAttributeQueryAsync(string name, BlockParameter blockParameter = null)
        {
            var getAttributeFunction = new GetAttributeFunction();
                getAttributeFunction.Name = name;
            
            return ContractHandler.QueryAsync<GetAttributeFunction, string>(getAttributeFunction, blockParameter);
        }

        public Task<string> GetLibraryAddressQueryAsync(GetLibraryAddressFunction getLibraryAddressFunction, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<GetLibraryAddressFunction, string>(getLibraryAddressFunction, blockParameter);
        }

        
        public Task<string> GetLibraryAddressQueryAsync(string name, BlockParameter blockParameter = null)
        {
            var getLibraryAddressFunction = new GetLibraryAddressFunction();
                getLibraryAddressFunction.Name = name;
            
            return ContractHandler.QueryAsync<GetLibraryAddressFunction, string>(getLibraryAddressFunction, blockParameter);
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

        public Task<string> LogRequestAsync(Log1Function log1Function)
        {
             return ContractHandler.SendRequestAsync(log1Function);
        }

        public Task<TransactionReceipt> LogRequestAndWaitForReceiptAsync(Log1Function log1Function, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(log1Function, cancellationToken);
        }

        public Task<string> LogRequestAsync(string sigma, string logLine)
        {
            var log1Function = new Log1Function();
                log1Function.Sigma = sigma;
                log1Function.LogLine = logLine;
            
             return ContractHandler.SendRequestAsync(log1Function);
        }

        public Task<TransactionReceipt> LogRequestAndWaitForReceiptAsync(string sigma, string logLine, CancellationTokenSource cancellationToken = null)
        {
            var log1Function = new Log1Function();
                log1Function.Sigma = sigma;
                log1Function.LogLine = logLine;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(log1Function, cancellationToken);
        }

        public Task<string> LogRequestAsync(LogFunction logFunction)
        {
             return ContractHandler.SendRequestAsync(logFunction);
        }

        public Task<TransactionReceipt> LogRequestAndWaitForReceiptAsync(LogFunction logFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(logFunction, cancellationToken);
        }

        public Task<string> LogRequestAsync(string logLine)
        {
            var logFunction = new LogFunction();
                logFunction.LogLine = logLine;
            
             return ContractHandler.SendRequestAsync(logFunction);
        }

        public Task<TransactionReceipt> LogRequestAndWaitForReceiptAsync(string logLine, CancellationTokenSource cancellationToken = null)
        {
            var logFunction = new LogFunction();
                logFunction.LogLine = logLine;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(logFunction, cancellationToken);
        }

        public Task<string> LogRequestAsync(Log2Function log2Function)
        {
             return ContractHandler.SendRequestAsync(log2Function);
        }

        public Task<TransactionReceipt> LogRequestAndWaitForReceiptAsync(Log2Function log2Function, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(log2Function, cancellationToken);
        }

        public Task<string> LogRequestAsync(ulong sigma, string logLine)
        {
            var log2Function = new Log2Function();
                log2Function.Sigma = sigma;
                log2Function.LogLine = logLine;
            
             return ContractHandler.SendRequestAsync(log2Function);
        }

        public Task<TransactionReceipt> LogRequestAndWaitForReceiptAsync(ulong sigma, string logLine, CancellationTokenSource cancellationToken = null)
        {
            var log2Function = new Log2Function();
                log2Function.Sigma = sigma;
                log2Function.LogLine = logLine;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(log2Function, cancellationToken);
        }

        public Task<ulong> MotzkinPrimeQueryAsync(MotzkinPrimeFunction motzkinPrimeFunction, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<MotzkinPrimeFunction, ulong>(motzkinPrimeFunction, blockParameter);
        }

        
        public Task<ulong> MotzkinPrimeQueryAsync(BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<MotzkinPrimeFunction, ulong>(null, blockParameter);
        }

        public Task<string> NuQueryAsync(NuFunction nuFunction, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<NuFunction, string>(nuFunction, blockParameter);
        }

        
        public Task<string> NuQueryAsync(BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<NuFunction, string>(null, blockParameter);
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

        public Task<string> SetAttributeRequestAsync(SetAttributeFunction setAttributeFunction)
        {
             return ContractHandler.SendRequestAsync(setAttributeFunction);
        }

        public Task<TransactionReceipt> SetAttributeRequestAndWaitForReceiptAsync(SetAttributeFunction setAttributeFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(setAttributeFunction, cancellationToken);
        }

        public Task<string> SetAttributeRequestAsync(string name, string value)
        {
            var setAttributeFunction = new SetAttributeFunction();
                setAttributeFunction.Name = name;
                setAttributeFunction.Value = value;
            
             return ContractHandler.SendRequestAsync(setAttributeFunction);
        }

        public Task<TransactionReceipt> SetAttributeRequestAndWaitForReceiptAsync(string name, string value, CancellationTokenSource cancellationToken = null)
        {
            var setAttributeFunction = new SetAttributeFunction();
                setAttributeFunction.Name = name;
                setAttributeFunction.Value = value;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(setAttributeFunction, cancellationToken);
        }

        public Task<string> TypeQueryAsync(TypeFunction typeFunction, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<TypeFunction, string>(typeFunction, blockParameter);
        }

        
        public Task<string> TypeQueryAsync(BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<TypeFunction, string>(null, blockParameter);
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
                typeof(AddLibraryFunction),
                typeof(Alias2Function),
                typeof(AliasFunction),
                typeof(Alias3Function),
                typeof(Alias1Function),
                typeof(ChatFunction),
                typeof(EnterFunction),
                typeof(Enter1Function),
                typeof(GetAttributeFunction),
                typeof(GetLibraryAddressFunction),
                typeof(GetMarketRateFunction),
                typeof(Log1Function),
                typeof(LogFunction),
                typeof(Log2Function),
                typeof(MotzkinPrimeFunction),
                typeof(NuFunction),
                typeof(PurchaseFunction),
                typeof(RedeemFunction),
                typeof(RenameFunction),
                typeof(SetAttributeFunction),
                typeof(TypeFunction),
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
                typeof(InvalidLogXiError),
                typeof(MarketRateNotFoundError),
                typeof(NoUserEntryError),
                typeof(NoUserNameError),
                typeof(NotShioOwnerError),
                typeof(OwnableInvalidOwnerError),
                typeof(OwnableUnauthorizedAccountError),
                typeof(UserAlreadyCreatedError)
            };
        }
    }
}
