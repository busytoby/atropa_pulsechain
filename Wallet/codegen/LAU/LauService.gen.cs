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
using Dysnomia.Contracts.LAU.ContractDefinition;

namespace Dysnomia.Contracts.LAU
{
    public partial class LauService: ContractWeb3ServiceBase
    {
        public static Task<TransactionReceipt> DeployContractAndWaitForReceiptAsync(Nethereum.Web3.IWeb3 web3, LauDeployment lauDeployment, CancellationTokenSource cancellationTokenSource = null)
        {
            return web3.Eth.GetContractDeploymentHandler<LauDeployment>().SendRequestAndWaitForReceiptAsync(lauDeployment, cancellationTokenSource);
        }

        public static Task<string> DeployContractAsync(Nethereum.Web3.IWeb3 web3, LauDeployment lauDeployment)
        {
            return web3.Eth.GetContractDeploymentHandler<LauDeployment>().SendRequestAsync(lauDeployment);
        }

        public static async Task<LauService> DeployContractAndGetServiceAsync(Nethereum.Web3.IWeb3 web3, LauDeployment lauDeployment, CancellationTokenSource cancellationTokenSource = null)
        {
            var receipt = await DeployContractAndWaitForReceiptAsync(web3, lauDeployment, cancellationTokenSource);
            return new LauService(web3, receipt.ContractAddress);
        }

        public LauService(Nethereum.Web3.IWeb3 web3, string contractAddress) : base(web3, contractAddress)
        {
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

        public Task<string> CurrentAreaQueryAsync(CurrentAreaFunction currentAreaFunction, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<CurrentAreaFunction, string>(currentAreaFunction, blockParameter);
        }

        
        public Task<string> CurrentAreaQueryAsync(BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<CurrentAreaFunction, string>(null, blockParameter);
        }

        public Task<string> EtaQueryAsync(EtaFunction etaFunction, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<EtaFunction, string>(etaFunction, blockParameter);
        }

        
        public Task<string> EtaQueryAsync(BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<EtaFunction, string>(null, blockParameter);
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

        public Task<string> LeaveRequestAsync(LeaveFunction leaveFunction)
        {
             return ContractHandler.SendRequestAsync(leaveFunction);
        }

        public Task<string> LeaveRequestAsync()
        {
             return ContractHandler.SendRequestAsync<LeaveFunction>();
        }

        public Task<TransactionReceipt> LeaveRequestAndWaitForReceiptAsync(LeaveFunction leaveFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(leaveFunction, cancellationToken);
        }

        public Task<TransactionReceipt> LeaveRequestAndWaitForReceiptAsync(CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync<LeaveFunction>(null, cancellationToken);
        }

        public Task<ulong> MotzkinPrimeQueryAsync(MotzkinPrimeFunction motzkinPrimeFunction, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<MotzkinPrimeFunction, ulong>(motzkinPrimeFunction, blockParameter);
        }

        
        public Task<ulong> MotzkinPrimeQueryAsync(BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<MotzkinPrimeFunction, ulong>(null, blockParameter);
        }

        public Task<OnOutputDTO> OnQueryAsync(OnFunction onFunction, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryDeserializingToObjectAsync<OnFunction, OnOutputDTO>(onFunction, blockParameter);
        }

        public Task<OnOutputDTO> OnQueryAsync(BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryDeserializingToObjectAsync<OnFunction, OnOutputDTO>(null, blockParameter);
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

        public Task<ulong> SaatQueryAsync(SaatFunction saatFunction, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<SaatFunction, ulong>(saatFunction, blockParameter);
        }

        
        public Task<ulong> SaatQueryAsync(BigInteger returnValue1, BlockParameter blockParameter = null)
        {
            var saatFunction = new SaatFunction();
                saatFunction.ReturnValue1 = returnValue1;
            
            return ContractHandler.QueryAsync<SaatFunction, ulong>(saatFunction, blockParameter);
        }

        public Task<string> TypeQueryAsync(TypeFunction typeFunction, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<TypeFunction, string>(typeFunction, blockParameter);
        }

        
        public Task<string> TypeQueryAsync(BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<TypeFunction, string>(null, blockParameter);
        }

        public Task<string> UsernameQueryAsync(UsernameFunction usernameFunction, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<UsernameFunction, string>(usernameFunction, blockParameter);
        }

        
        public Task<string> UsernameQueryAsync(BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<UsernameFunction, string>(null, blockParameter);
        }

        public Task<string> UsernameRequestAsync(Username1Function username1Function)
        {
             return ContractHandler.SendRequestAsync(username1Function);
        }

        public Task<TransactionReceipt> UsernameRequestAndWaitForReceiptAsync(Username1Function username1Function, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(username1Function, cancellationToken);
        }

        public Task<string> UsernameRequestAsync(string newUsername)
        {
            var username1Function = new Username1Function();
                username1Function.NewUsername = newUsername;
            
             return ContractHandler.SendRequestAsync(username1Function);
        }

        public Task<TransactionReceipt> UsernameRequestAndWaitForReceiptAsync(string newUsername, CancellationTokenSource cancellationToken = null)
        {
            var username1Function = new Username1Function();
                username1Function.NewUsername = newUsername;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(username1Function, cancellationToken);
        }

        public Task<string> VoidRequestAsync(VoidFunction @voidFunction)
        {
             return ContractHandler.SendRequestAsync(@voidFunction);
        }

        public Task<TransactionReceipt> VoidRequestAndWaitForReceiptAsync(VoidFunction @voidFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(@voidFunction, cancellationToken);
        }

        public Task<string> VoidRequestAsync(bool really1, bool really2)
        {
            var @voidFunction = new VoidFunction();
                @voidFunction.Really1 = really1;
                @voidFunction.Really2 = really2;
            
             return ContractHandler.SendRequestAsync(@voidFunction);
        }

        public Task<TransactionReceipt> VoidRequestAndWaitForReceiptAsync(bool really1, bool really2, CancellationTokenSource cancellationToken = null)
        {
            var @voidFunction = new VoidFunction();
                @voidFunction.Really1 = really1;
                @voidFunction.Really2 = really2;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(@voidFunction, cancellationToken);
        }

        public Task<string> WithdrawRequestAsync(WithdrawFunction withdrawFunction)
        {
             return ContractHandler.SendRequestAsync(withdrawFunction);
        }

        public Task<TransactionReceipt> WithdrawRequestAndWaitForReceiptAsync(WithdrawFunction withdrawFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(withdrawFunction, cancellationToken);
        }

        public Task<string> WithdrawRequestAsync(string what, BigInteger amount)
        {
            var withdrawFunction = new WithdrawFunction();
                withdrawFunction.What = what;
                withdrawFunction.Amount = amount;
            
             return ContractHandler.SendRequestAsync(withdrawFunction);
        }

        public Task<TransactionReceipt> WithdrawRequestAndWaitForReceiptAsync(string what, BigInteger amount, CancellationTokenSource cancellationToken = null)
        {
            var withdrawFunction = new WithdrawFunction();
                withdrawFunction.What = what;
                withdrawFunction.Amount = amount;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(withdrawFunction, cancellationToken);
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
                typeof(Alias2Function),
                typeof(AliasFunction),
                typeof(Alias3Function),
                typeof(Alias1Function),
                typeof(ChatFunction),
                typeof(CurrentAreaFunction),
                typeof(EtaFunction),
                typeof(GetMarketRateFunction),
                typeof(LeaveFunction),
                typeof(MotzkinPrimeFunction),
                typeof(OnFunction),
                typeof(PurchaseFunction),
                typeof(RedeemFunction),
                typeof(RenameFunction),
                typeof(SaatFunction),
                typeof(TypeFunction),
                typeof(UsernameFunction),
                typeof(Username1Function),
                typeof(VoidFunction),
                typeof(WithdrawFunction),
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
