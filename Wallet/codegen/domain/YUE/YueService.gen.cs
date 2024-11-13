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
using Dysnomia.Contracts.domain.YUE.ContractDefinition;

namespace Dysnomia.Contracts.domain.YUE
{
    public partial class YueService: ContractWeb3ServiceBase
    {
        public static Task<TransactionReceipt> DeployContractAndWaitForReceiptAsync(Nethereum.Web3.IWeb3 web3, YueDeployment yueDeployment, CancellationTokenSource cancellationTokenSource = null)
        {
            return web3.Eth.GetContractDeploymentHandler<YueDeployment>().SendRequestAndWaitForReceiptAsync(yueDeployment, cancellationTokenSource);
        }

        public static Task<string> DeployContractAsync(Nethereum.Web3.IWeb3 web3, YueDeployment yueDeployment)
        {
            return web3.Eth.GetContractDeploymentHandler<YueDeployment>().SendRequestAsync(yueDeployment);
        }

        public static async Task<YueService> DeployContractAndGetServiceAsync(Nethereum.Web3.IWeb3 web3, YueDeployment yueDeployment, CancellationTokenSource cancellationTokenSource = null)
        {
            var receipt = await DeployContractAndWaitForReceiptAsync(web3, yueDeployment, cancellationTokenSource);
            return new YueService(web3, receipt.ContractAddress);
        }

        public YueService(Nethereum.Web3.IWeb3 web3, string contractAddress) : base(web3, contractAddress)
        {
        }

        public Task<BarOutputDTO> BarQueryAsync(BarFunction barFunction, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryDeserializingToObjectAsync<BarFunction, BarOutputDTO>(barFunction, blockParameter);
        }

        public Task<BarOutputDTO> BarQueryAsync(string qing, BlockParameter blockParameter = null)
        {
            var barFunction = new BarFunction();
                barFunction.Qing = qing;
            
            return ContractHandler.QueryDeserializingToObjectAsync<BarFunction, BarOutputDTO>(barFunction, blockParameter);
        }

        public Task<string> ChanQueryAsync(ChanFunction chanFunction, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<ChanFunction, string>(chanFunction, blockParameter);
        }

        
        public Task<string> ChanQueryAsync(BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<ChanFunction, string>(null, blockParameter);
        }

        public Task<string> ChangeOriginRequestAsync(ChangeOriginFunction changeOriginFunction)
        {
             return ContractHandler.SendRequestAsync(changeOriginFunction);
        }

        public Task<TransactionReceipt> ChangeOriginRequestAndWaitForReceiptAsync(ChangeOriginFunction changeOriginFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(changeOriginFunction, cancellationToken);
        }

        public Task<string> ChangeOriginRequestAsync(string newOrigin)
        {
            var changeOriginFunction = new ChangeOriginFunction();
                changeOriginFunction.NewOrigin = newOrigin;
            
             return ContractHandler.SendRequestAsync(changeOriginFunction);
        }

        public Task<TransactionReceipt> ChangeOriginRequestAndWaitForReceiptAsync(string newOrigin, CancellationTokenSource cancellationToken = null)
        {
            var changeOriginFunction = new ChangeOriginFunction();
                changeOriginFunction.NewOrigin = newOrigin;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(changeOriginFunction, cancellationToken);
        }

        public Task<BigInteger> GetAssetRateQueryAsync(GetAssetRateFunction getAssetRateFunction, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<GetAssetRateFunction, BigInteger>(getAssetRateFunction, blockParameter);
        }

        
        public Task<BigInteger> GetAssetRateQueryAsync(string gwatAsset, string integrative, BlockParameter blockParameter = null)
        {
            var getAssetRateFunction = new GetAssetRateFunction();
                getAssetRateFunction.GwatAsset = gwatAsset;
                getAssetRateFunction.Integrative = integrative;
            
            return ContractHandler.QueryAsync<GetAssetRateFunction, BigInteger>(getAssetRateFunction, blockParameter);
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

        public Task<string> HongRequestAsync(HongFunction hongFunction)
        {
             return ContractHandler.SendRequestAsync(hongFunction);
        }

        public Task<TransactionReceipt> HongRequestAndWaitForReceiptAsync(HongFunction hongFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(hongFunction, cancellationToken);
        }

        public Task<string> HongRequestAsync(string spendAsset, string qingAsset, BigInteger purchaseAmount)
        {
            var hongFunction = new HongFunction();
                hongFunction.SpendAsset = spendAsset;
                hongFunction.QingAsset = qingAsset;
                hongFunction.PurchaseAmount = purchaseAmount;
            
             return ContractHandler.SendRequestAsync(hongFunction);
        }

        public Task<TransactionReceipt> HongRequestAndWaitForReceiptAsync(string spendAsset, string qingAsset, BigInteger purchaseAmount, CancellationTokenSource cancellationToken = null)
        {
            var hongFunction = new HongFunction();
                hongFunction.SpendAsset = spendAsset;
                hongFunction.QingAsset = qingAsset;
                hongFunction.PurchaseAmount = purchaseAmount;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(hongFunction, cancellationToken);
        }

        public Task<string> HungRequestAsync(HungFunction hungFunction)
        {
             return ContractHandler.SendRequestAsync(hungFunction);
        }

        public Task<TransactionReceipt> HungRequestAndWaitForReceiptAsync(HungFunction hungFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(hungFunction, cancellationToken);
        }

        public Task<string> HungRequestAsync(string qingAsset, string receiveAsset, BigInteger redeemAmount)
        {
            var hungFunction = new HungFunction();
                hungFunction.QingAsset = qingAsset;
                hungFunction.ReceiveAsset = receiveAsset;
                hungFunction.RedeemAmount = redeemAmount;
            
             return ContractHandler.SendRequestAsync(hungFunction);
        }

        public Task<TransactionReceipt> HungRequestAndWaitForReceiptAsync(string qingAsset, string receiveAsset, BigInteger redeemAmount, CancellationTokenSource cancellationToken = null)
        {
            var hungFunction = new HungFunction();
                hungFunction.QingAsset = qingAsset;
                hungFunction.ReceiveAsset = receiveAsset;
                hungFunction.RedeemAmount = redeemAmount;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(hungFunction, cancellationToken);
        }

        public Task<bool> IsValidAssetQueryAsync(IsValidAssetFunction isValidAssetFunction, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<IsValidAssetFunction, bool>(isValidAssetFunction, blockParameter);
        }

        
        public Task<bool> IsValidAssetQueryAsync(string gwatAsset, string integrative, BlockParameter blockParameter = null)
        {
            var isValidAssetFunction = new IsValidAssetFunction();
                isValidAssetFunction.GwatAsset = gwatAsset;
                isValidAssetFunction.Integrative = integrative;
            
            return ContractHandler.QueryAsync<IsValidAssetFunction, bool>(isValidAssetFunction, blockParameter);
        }

        public Task<string> MintToOriginRequestAsync(MintToOriginFunction mintToOriginFunction)
        {
             return ContractHandler.SendRequestAsync(mintToOriginFunction);
        }

        public Task<string> MintToOriginRequestAsync()
        {
             return ContractHandler.SendRequestAsync<MintToOriginFunction>();
        }

        public Task<TransactionReceipt> MintToOriginRequestAndWaitForReceiptAsync(MintToOriginFunction mintToOriginFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(mintToOriginFunction, cancellationToken);
        }

        public Task<TransactionReceipt> MintToOriginRequestAndWaitForReceiptAsync(CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync<MintToOriginFunction>(null, cancellationToken);
        }

        public Task<ulong> MotzkinPrimeQueryAsync(MotzkinPrimeFunction motzkinPrimeFunction, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<MotzkinPrimeFunction, ulong>(motzkinPrimeFunction, blockParameter);
        }

        
        public Task<ulong> MotzkinPrimeQueryAsync(BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<MotzkinPrimeFunction, ulong>(null, blockParameter);
        }

        public Task<string> OriginQueryAsync(OriginFunction originFunction, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<OriginFunction, string>(originFunction, blockParameter);
        }

        
        public Task<string> OriginQueryAsync(BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<OriginFunction, string>(null, blockParameter);
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

        public Task<string> ReactRequestAsync(string qing)
        {
            var reactFunction = new ReactFunction();
                reactFunction.Qing = qing;
            
             return ContractHandler.SendRequestAsync(reactFunction);
        }

        public Task<TransactionReceipt> ReactRequestAndWaitForReceiptAsync(string qing, CancellationTokenSource cancellationToken = null)
        {
            var reactFunction = new ReactFunction();
                reactFunction.Qing = qing;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(reactFunction, cancellationToken);
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

        public Task<string> WithdrawRequestAsync(WithdrawFunction withdrawFunction)
        {
             return ContractHandler.SendRequestAsync(withdrawFunction);
        }

        public Task<TransactionReceipt> WithdrawRequestAndWaitForReceiptAsync(WithdrawFunction withdrawFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(withdrawFunction, cancellationToken);
        }

        public Task<string> WithdrawRequestAsync(string what, string to, BigInteger amount)
        {
            var withdrawFunction = new WithdrawFunction();
                withdrawFunction.What = what;
                withdrawFunction.To = to;
                withdrawFunction.Amount = amount;
            
             return ContractHandler.SendRequestAsync(withdrawFunction);
        }

        public Task<TransactionReceipt> WithdrawRequestAndWaitForReceiptAsync(string what, string to, BigInteger amount, CancellationTokenSource cancellationToken = null)
        {
            var withdrawFunction = new WithdrawFunction();
                withdrawFunction.What = what;
                withdrawFunction.To = to;
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

        public Task<bool> HasMintQueryAsync(HasMintFunction hasMintFunction, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<HasMintFunction, bool>(hasMintFunction, blockParameter);
        }

        
        public Task<bool> HasMintQueryAsync(string contract, BlockParameter blockParameter = null)
        {
            var hasMintFunction = new HasMintFunction();
                hasMintFunction.Contract = contract;
            
            return ContractHandler.QueryAsync<HasMintFunction, bool>(hasMintFunction, blockParameter);
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
                typeof(BarFunction),
                typeof(ChanFunction),
                typeof(ChangeOriginFunction),
                typeof(GetAssetRateFunction),
                typeof(GetMarketRateFunction),
                typeof(HongFunction),
                typeof(HungFunction),
                typeof(IsValidAssetFunction),
                typeof(MintToOriginFunction),
                typeof(MotzkinPrimeFunction),
                typeof(OriginFunction),
                typeof(PurchaseFunction),
                typeof(ReactFunction),
                typeof(RedeemFunction),
                typeof(RenameFunction),
                typeof(TypeFunction),
                typeof(WithdrawFunction),
                typeof(XiaoFunction),
                typeof(AddOwnerFunction),
                typeof(AllowanceFunction),
                typeof(ApproveFunction),
                typeof(BalanceOfFunction),
                typeof(DecimalsFunction),
                typeof(HasMintFunction),
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
                typeof(ExchangeRateNotFoundError),
                typeof(InvalidPairError),
                typeof(MarketRateNotFoundError),
                typeof(OnlyChanError),
                typeof(OnlyGameTokensError),
                typeof(OwnableInvalidOwnerError),
                typeof(OwnableUnauthorizedAccountError),
                typeof(ZeroHoldingsError)
            };
        }
    }
}
