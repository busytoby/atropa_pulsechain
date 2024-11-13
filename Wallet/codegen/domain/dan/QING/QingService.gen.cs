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
using Dysnomia.Contracts.domain.dan.QING.ContractDefinition;

namespace Dysnomia.Contracts.domain.dan.QING
{
    public partial class QingService: ContractWeb3ServiceBase
    {
        public static Task<TransactionReceipt> DeployContractAndWaitForReceiptAsync(Nethereum.Web3.IWeb3 web3, QingDeployment qingDeployment, CancellationTokenSource cancellationTokenSource = null)
        {
            return web3.Eth.GetContractDeploymentHandler<QingDeployment>().SendRequestAndWaitForReceiptAsync(qingDeployment, cancellationTokenSource);
        }

        public static Task<string> DeployContractAsync(Nethereum.Web3.IWeb3 web3, QingDeployment qingDeployment)
        {
            return web3.Eth.GetContractDeploymentHandler<QingDeployment>().SendRequestAsync(qingDeployment);
        }

        public static async Task<QingService> DeployContractAndGetServiceAsync(Nethereum.Web3.IWeb3 web3, QingDeployment qingDeployment, CancellationTokenSource cancellationTokenSource = null)
        {
            var receipt = await DeployContractAndWaitForReceiptAsync(web3, qingDeployment, cancellationTokenSource);
            return new QingService(web3, receipt.ContractAddress);
        }

        public QingService(Nethereum.Web3.IWeb3 web3, string contractAddress) : base(web3, contractAddress)
        {
        }

        public Task<string> AddMarketRateRequestAsync(AddMarketRateFunction addMarketRateFunction)
        {
             return ContractHandler.SendRequestAsync(addMarketRateFunction);
        }

        public Task<TransactionReceipt> AddMarketRateRequestAndWaitForReceiptAsync(AddMarketRateFunction addMarketRateFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(addMarketRateFunction, cancellationToken);
        }

        public Task<string> AddMarketRateRequestAsync(string contract, BigInteger rate)
        {
            var addMarketRateFunction = new AddMarketRateFunction();
                addMarketRateFunction.Contract = contract;
                addMarketRateFunction.Rate = rate;
            
             return ContractHandler.SendRequestAsync(addMarketRateFunction);
        }

        public Task<TransactionReceipt> AddMarketRateRequestAndWaitForReceiptAsync(string contract, BigInteger rate, CancellationTokenSource cancellationToken = null)
        {
            var addMarketRateFunction = new AddMarketRateFunction();
                addMarketRateFunction.Contract = contract;
                addMarketRateFunction.Rate = rate;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(addMarketRateFunction, cancellationToken);
        }

        public Task<bool> AdmittedQueryAsync(AdmittedFunction admittedFunction, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<AdmittedFunction, bool>(admittedFunction, blockParameter);
        }

        
        public Task<bool> AdmittedQueryAsync(string userToken, BlockParameter blockParameter = null)
        {
            var admittedFunction = new AdmittedFunction();
                admittedFunction.UserToken = userToken;
            
            return ContractHandler.QueryAsync<AdmittedFunction, bool>(admittedFunction, blockParameter);
        }

        public Task<string> AllowCROWSRequestAsync(AllowCROWSFunction allowCROWSFunction)
        {
             return ContractHandler.SendRequestAsync(allowCROWSFunction);
        }

        public Task<TransactionReceipt> AllowCROWSRequestAndWaitForReceiptAsync(AllowCROWSFunction allowCROWSFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(allowCROWSFunction, cancellationToken);
        }

        public Task<string> AllowCROWSRequestAsync(bool b)
        {
            var allowCROWSFunction = new AllowCROWSFunction();
                allowCROWSFunction.B = b;
            
             return ContractHandler.SendRequestAsync(allowCROWSFunction);
        }

        public Task<TransactionReceipt> AllowCROWSRequestAndWaitForReceiptAsync(bool b, CancellationTokenSource cancellationToken = null)
        {
            var allowCROWSFunction = new AllowCROWSFunction();
                allowCROWSFunction.B = b;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(allowCROWSFunction, cancellationToken);
        }

        public Task<string> AssetQueryAsync(AssetFunction assetFunction, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<AssetFunction, string>(assetFunction, blockParameter);
        }

        
        public Task<string> AssetQueryAsync(BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<AssetFunction, string>(null, blockParameter);
        }

        public Task<ushort> BouncerDivisorQueryAsync(BouncerDivisorFunction bouncerDivisorFunction, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<BouncerDivisorFunction, ushort>(bouncerDivisorFunction, blockParameter);
        }

        
        public Task<ushort> BouncerDivisorQueryAsync(BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<BouncerDivisorFunction, ushort>(null, blockParameter);
        }

        public Task<string> ChatRequestAsync(ChatFunction chatFunction)
        {
             return ContractHandler.SendRequestAsync(chatFunction);
        }

        public Task<TransactionReceipt> ChatRequestAndWaitForReceiptAsync(ChatFunction chatFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(chatFunction, cancellationToken);
        }

        public Task<string> ChatRequestAsync(string userToken, string msg)
        {
            var chatFunction = new ChatFunction();
                chatFunction.UserToken = userToken;
                chatFunction.Msg = msg;
            
             return ContractHandler.SendRequestAsync(chatFunction);
        }

        public Task<TransactionReceipt> ChatRequestAndWaitForReceiptAsync(string userToken, string msg, CancellationTokenSource cancellationToken = null)
        {
            var chatFunction = new ChatFunction();
                chatFunction.UserToken = userToken;
                chatFunction.Msg = msg;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(chatFunction, cancellationToken);
        }

        public Task<string> ChoQueryAsync(ChoFunction choFunction, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<ChoFunction, string>(choFunction, blockParameter);
        }

        
        public Task<string> ChoQueryAsync(BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<ChoFunction, string>(null, blockParameter);
        }

        public Task<BigInteger> CoverChargeQueryAsync(CoverChargeFunction coverChargeFunction, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<CoverChargeFunction, BigInteger>(coverChargeFunction, blockParameter);
        }

        
        public Task<BigInteger> CoverChargeQueryAsync(BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<CoverChargeFunction, BigInteger>(null, blockParameter);
        }

        public Task<ulong> EntropyQueryAsync(EntropyFunction entropyFunction, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<EntropyFunction, ulong>(entropyFunction, blockParameter);
        }

        
        public Task<ulong> EntropyQueryAsync(BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<EntropyFunction, ulong>(null, blockParameter);
        }

        public Task<bool> GwatQueryAsync(GwatFunction gwatFunction, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<GwatFunction, bool>(gwatFunction, blockParameter);
        }

        
        public Task<bool> GwatQueryAsync(BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<GwatFunction, bool>(null, blockParameter);
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

        public Task<string> GetQingQueryAsync(GetQingFunction getQingFunction, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<GetQingFunction, string>(getQingFunction, blockParameter);
        }

        
        public Task<string> GetQingQueryAsync(BigInteger qingWaat, BlockParameter blockParameter = null)
        {
            var getQingFunction = new GetQingFunction();
                getQingFunction.QingWaat = qingWaat;
            
            return ContractHandler.QueryAsync<GetQingFunction, string>(getQingFunction, blockParameter);
        }

        public Task<string> JoinRequestAsync(JoinFunction joinFunction)
        {
             return ContractHandler.SendRequestAsync(joinFunction);
        }

        public Task<TransactionReceipt> JoinRequestAndWaitForReceiptAsync(JoinFunction joinFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(joinFunction, cancellationToken);
        }

        public Task<string> JoinRequestAsync(string userToken)
        {
            var joinFunction = new JoinFunction();
                joinFunction.UserToken = userToken;
            
             return ContractHandler.SendRequestAsync(joinFunction);
        }

        public Task<TransactionReceipt> JoinRequestAndWaitForReceiptAsync(string userToken, CancellationTokenSource cancellationToken = null)
        {
            var joinFunction = new JoinFunction();
                joinFunction.UserToken = userToken;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(joinFunction, cancellationToken);
        }

        public Task<string> MapQueryAsync(MapFunction mapFunction, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<MapFunction, string>(mapFunction, blockParameter);
        }

        
        public Task<string> MapQueryAsync(BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<MapFunction, string>(null, blockParameter);
        }

        public Task<ulong> MotzkinPrimeQueryAsync(MotzkinPrimeFunction motzkinPrimeFunction, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<MotzkinPrimeFunction, ulong>(motzkinPrimeFunction, blockParameter);
        }

        
        public Task<ulong> MotzkinPrimeQueryAsync(BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<MotzkinPrimeFunction, ulong>(null, blockParameter);
        }

        public Task<bool> NoCROWSQueryAsync(NoCROWSFunction noCROWSFunction, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<NoCROWSFunction, bool>(noCROWSFunction, blockParameter);
        }

        
        public Task<bool> NoCROWSQueryAsync(BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<NoCROWSFunction, bool>(null, blockParameter);
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

        public Task<string> TypeQueryAsync(TypeFunction typeFunction, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<TypeFunction, string>(typeFunction, blockParameter);
        }

        
        public Task<string> TypeQueryAsync(BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<TypeFunction, string>(null, blockParameter);
        }

        public Task<BigInteger> WaatQueryAsync(WaatFunction waatFunction, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<WaatFunction, BigInteger>(waatFunction, blockParameter);
        }

        
        public Task<BigInteger> WaatQueryAsync(BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<WaatFunction, BigInteger>(null, blockParameter);
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

        public Task<bool> BouncerQueryAsync(BouncerFunction bouncerFunction, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<BouncerFunction, bool>(bouncerFunction, blockParameter);
        }

        
        public Task<bool> BouncerQueryAsync(string cBouncer, BlockParameter blockParameter = null)
        {
            var bouncerFunction = new BouncerFunction();
                bouncerFunction.CBouncer = cBouncer;
            
            return ContractHandler.QueryAsync<BouncerFunction, bool>(bouncerFunction, blockParameter);
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

        public Task<string> RemoveGuestRequestAsync(RemoveGuestFunction removeGuestFunction)
        {
             return ContractHandler.SendRequestAsync(removeGuestFunction);
        }

        public Task<TransactionReceipt> RemoveGuestRequestAndWaitForReceiptAsync(RemoveGuestFunction removeGuestFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(removeGuestFunction, cancellationToken);
        }

        public Task<string> RemoveGuestRequestAsync(string a)
        {
            var removeGuestFunction = new RemoveGuestFunction();
                removeGuestFunction.A = a;
            
             return ContractHandler.SendRequestAsync(removeGuestFunction);
        }

        public Task<TransactionReceipt> RemoveGuestRequestAndWaitForReceiptAsync(string a, CancellationTokenSource cancellationToken = null)
        {
            var removeGuestFunction = new RemoveGuestFunction();
                removeGuestFunction.A = a;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(removeGuestFunction, cancellationToken);
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

        public Task<string> SetBouncerDivisorRequestAsync(SetBouncerDivisorFunction setBouncerDivisorFunction)
        {
             return ContractHandler.SendRequestAsync(setBouncerDivisorFunction);
        }

        public Task<TransactionReceipt> SetBouncerDivisorRequestAndWaitForReceiptAsync(SetBouncerDivisorFunction setBouncerDivisorFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(setBouncerDivisorFunction, cancellationToken);
        }

        public Task<string> SetBouncerDivisorRequestAsync(ushort d)
        {
            var setBouncerDivisorFunction = new SetBouncerDivisorFunction();
                setBouncerDivisorFunction.D = d;
            
             return ContractHandler.SendRequestAsync(setBouncerDivisorFunction);
        }

        public Task<TransactionReceipt> SetBouncerDivisorRequestAndWaitForReceiptAsync(ushort d, CancellationTokenSource cancellationToken = null)
        {
            var setBouncerDivisorFunction = new SetBouncerDivisorFunction();
                setBouncerDivisorFunction.D = d;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(setBouncerDivisorFunction, cancellationToken);
        }

        public Task<string> SetCoverChargeRequestAsync(SetCoverChargeFunction setCoverChargeFunction)
        {
             return ContractHandler.SendRequestAsync(setCoverChargeFunction);
        }

        public Task<TransactionReceipt> SetCoverChargeRequestAndWaitForReceiptAsync(SetCoverChargeFunction setCoverChargeFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(setCoverChargeFunction, cancellationToken);
        }

        public Task<string> SetCoverChargeRequestAsync(BigInteger c)
        {
            var setCoverChargeFunction = new SetCoverChargeFunction();
                setCoverChargeFunction.C = c;
            
             return ContractHandler.SendRequestAsync(setCoverChargeFunction);
        }

        public Task<TransactionReceipt> SetCoverChargeRequestAndWaitForReceiptAsync(BigInteger c, CancellationTokenSource cancellationToken = null)
        {
            var setCoverChargeFunction = new SetCoverChargeFunction();
                setCoverChargeFunction.C = c;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(setCoverChargeFunction, cancellationToken);
        }

        public Task<string> SetGuestlistRequestAsync(SetGuestlistFunction setGuestlistFunction)
        {
             return ContractHandler.SendRequestAsync(setGuestlistFunction);
        }

        public Task<TransactionReceipt> SetGuestlistRequestAndWaitForReceiptAsync(SetGuestlistFunction setGuestlistFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(setGuestlistFunction, cancellationToken);
        }

        public Task<string> SetGuestlistRequestAsync(string a)
        {
            var setGuestlistFunction = new SetGuestlistFunction();
                setGuestlistFunction.A = a;
            
             return ContractHandler.SendRequestAsync(setGuestlistFunction);
        }

        public Task<TransactionReceipt> SetGuestlistRequestAndWaitForReceiptAsync(string a, CancellationTokenSource cancellationToken = null)
        {
            var setGuestlistFunction = new SetGuestlistFunction();
                setGuestlistFunction.A = a;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(setGuestlistFunction, cancellationToken);
        }

        public Task<string> SetStaffRequestAsync(SetStaffFunction setStaffFunction)
        {
             return ContractHandler.SendRequestAsync(setStaffFunction);
        }

        public Task<TransactionReceipt> SetStaffRequestAndWaitForReceiptAsync(SetStaffFunction setStaffFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(setStaffFunction, cancellationToken);
        }

        public Task<string> SetStaffRequestAsync(string a, bool active)
        {
            var setStaffFunction = new SetStaffFunction();
                setStaffFunction.A = a;
                setStaffFunction.Active = active;
            
             return ContractHandler.SendRequestAsync(setStaffFunction);
        }

        public Task<TransactionReceipt> SetStaffRequestAndWaitForReceiptAsync(string a, bool active, CancellationTokenSource cancellationToken = null)
        {
            var setStaffFunction = new SetStaffFunction();
                setStaffFunction.A = a;
                setStaffFunction.Active = active;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(setStaffFunction, cancellationToken);
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
                typeof(AddMarketRateFunction),
                typeof(AdmittedFunction),
                typeof(AllowCROWSFunction),
                typeof(AssetFunction),
                typeof(BouncerDivisorFunction),
                typeof(ChatFunction),
                typeof(ChoFunction),
                typeof(CoverChargeFunction),
                typeof(EntropyFunction),
                typeof(GwatFunction),
                typeof(GetMarketRateFunction),
                typeof(GetQingFunction),
                typeof(JoinFunction),
                typeof(MapFunction),
                typeof(MotzkinPrimeFunction),
                typeof(NoCROWSFunction),
                typeof(PurchaseFunction),
                typeof(RedeemFunction),
                typeof(RenameFunction),
                typeof(TypeFunction),
                typeof(WaatFunction),
                typeof(WithdrawFunction),
                typeof(XiaoFunction),
                typeof(AddOwnerFunction),
                typeof(AllowanceFunction),
                typeof(ApproveFunction),
                typeof(BalanceOfFunction),
                typeof(BouncerFunction),
                typeof(DecimalsFunction),
                typeof(MaxSupplyFunction),
                typeof(MintToCapFunction),
                typeof(NameFunction),
                typeof(Owner1Function),
                typeof(OwnerFunction),
                typeof(RemoveGuestFunction),
                typeof(RenounceOwnershipFunction),
                typeof(SetBouncerDivisorFunction),
                typeof(SetCoverChargeFunction),
                typeof(SetGuestlistFunction),
                typeof(SetStaffFunction),
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
                typeof(LogEventEventDTO),
                typeof(OwnershipUpdateEventDTO),
                typeof(TransferEventDTO),
                typeof(WithdrawalEventDTO)
            };
        }

        public override List<Type> GetAllErrorTypes()
        {
            return new List<Type>
            {
                typeof(AlreadyJoinedError),
                typeof(BouncerUnauthorizedError),
                typeof(CoverChargeUnauthorizedError),
                typeof(DysnomiaInsufficientAllowanceError),
                typeof(DysnomiaInsufficientBalanceError),
                typeof(ForbiddenError),
                typeof(MarketRateCanOnlyBeIncreasedError),
                typeof(MarketRateNotFoundError),
                typeof(NotAdmittedError),
                typeof(OwnableInvalidOwnerError),
                typeof(OwnableUnauthorizedAccountError),
                typeof(PayCoverError),
                typeof(TokenMaximumRateError)
            };
        }
    }
}
