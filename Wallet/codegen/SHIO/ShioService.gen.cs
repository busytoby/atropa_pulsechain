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
using Dysnomia.Contracts.SHIO.ContractDefinition;

namespace Dysnomia.Contracts.SHIO
{
    public partial class ShioService: ContractWeb3ServiceBase
    {
        public static Task<TransactionReceipt> DeployContractAndWaitForReceiptAsync(Nethereum.Web3.IWeb3 web3, ShioDeployment shioDeployment, CancellationTokenSource cancellationTokenSource = null)
        {
            return web3.Eth.GetContractDeploymentHandler<ShioDeployment>().SendRequestAndWaitForReceiptAsync(shioDeployment, cancellationTokenSource);
        }

        public static Task<string> DeployContractAsync(Nethereum.Web3.IWeb3 web3, ShioDeployment shioDeployment)
        {
            return web3.Eth.GetContractDeploymentHandler<ShioDeployment>().SendRequestAsync(shioDeployment);
        }

        public static async Task<ShioService> DeployContractAndGetServiceAsync(Nethereum.Web3.IWeb3 web3, ShioDeployment shioDeployment, CancellationTokenSource cancellationTokenSource = null)
        {
            var receipt = await DeployContractAndWaitForReceiptAsync(web3, shioDeployment, cancellationTokenSource);
            return new ShioService(web3, receipt.ContractAddress);
        }

        public ShioService(Nethereum.Web3.IWeb3 web3, string contractAddress) : base(web3, contractAddress)
        {
        }

        public Task<string> ConeQueryAsync(ConeFunction coneFunction, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<ConeFunction, string>(coneFunction, blockParameter);
        }

        
        public Task<string> ConeQueryAsync(BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<ConeFunction, string>(null, blockParameter);
        }

        public Task<string> GenerateRequestAsync(GenerateFunction generateFunction)
        {
             return ContractHandler.SendRequestAsync(generateFunction);
        }

        public Task<TransactionReceipt> GenerateRequestAndWaitForReceiptAsync(GenerateFunction generateFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(generateFunction, cancellationToken);
        }

        public Task<string> GenerateRequestAsync(ulong xi, ulong alpha, ulong beta)
        {
            var generateFunction = new GenerateFunction();
                generateFunction.Xi = xi;
                generateFunction.Alpha = alpha;
                generateFunction.Beta = beta;
            
             return ContractHandler.SendRequestAsync(generateFunction);
        }

        public Task<TransactionReceipt> GenerateRequestAndWaitForReceiptAsync(ulong xi, ulong alpha, ulong beta, CancellationTokenSource cancellationToken = null)
        {
            var generateFunction = new GenerateFunction();
                generateFunction.Xi = xi;
                generateFunction.Alpha = alpha;
                generateFunction.Beta = beta;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(generateFunction, cancellationToken);
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

        public Task<string> IsolateRequestAsync(IsolateFunction isolateFunction)
        {
             return ContractHandler.SendRequestAsync(isolateFunction);
        }

        public Task<string> IsolateRequestAsync()
        {
             return ContractHandler.SendRequestAsync<IsolateFunction>();
        }

        public Task<TransactionReceipt> IsolateRequestAndWaitForReceiptAsync(IsolateFunction isolateFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(isolateFunction, cancellationToken);
        }

        public Task<TransactionReceipt> IsolateRequestAndWaitForReceiptAsync(CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync<IsolateFunction>(null, cancellationToken);
        }

        public Task<string> IsomerizeRequestAsync(IsomerizeFunction isomerizeFunction)
        {
             return ContractHandler.SendRequestAsync(isomerizeFunction);
        }

        public Task<string> IsomerizeRequestAsync()
        {
             return ContractHandler.SendRequestAsync<IsomerizeFunction>();
        }

        public Task<TransactionReceipt> IsomerizeRequestAndWaitForReceiptAsync(IsomerizeFunction isomerizeFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(isomerizeFunction, cancellationToken);
        }

        public Task<TransactionReceipt> IsomerizeRequestAndWaitForReceiptAsync(CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync<IsomerizeFunction>(null, cancellationToken);
        }

        public Task<string> LogRequestAsync(LogFunction logFunction)
        {
             return ContractHandler.SendRequestAsync(logFunction);
        }

        public Task<TransactionReceipt> LogRequestAndWaitForReceiptAsync(LogFunction logFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(logFunction, cancellationToken);
        }

        public Task<string> LogRequestAsync(ulong soul, ulong aura, string logLine)
        {
            var logFunction = new LogFunction();
                logFunction.Soul = soul;
                logFunction.Aura = aura;
                logFunction.LogLine = logLine;
            
             return ContractHandler.SendRequestAsync(logFunction);
        }

        public Task<TransactionReceipt> LogRequestAndWaitForReceiptAsync(ulong soul, ulong aura, string logLine, CancellationTokenSource cancellationToken = null)
        {
            var logFunction = new LogFunction();
                logFunction.Soul = soul;
                logFunction.Aura = aura;
                logFunction.LogLine = logLine;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(logFunction, cancellationToken);
        }

        public Task<string> MagnetizeRequestAsync(MagnetizeFunction magnetizeFunction)
        {
             return ContractHandler.SendRequestAsync(magnetizeFunction);
        }

        public Task<string> MagnetizeRequestAsync()
        {
             return ContractHandler.SendRequestAsync<MagnetizeFunction>();
        }

        public Task<TransactionReceipt> MagnetizeRequestAndWaitForReceiptAsync(MagnetizeFunction magnetizeFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(magnetizeFunction, cancellationToken);
        }

        public Task<TransactionReceipt> MagnetizeRequestAndWaitForReceiptAsync(CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync<MagnetizeFunction>(null, cancellationToken);
        }

        public Task<ulong> ManifoldQueryAsync(ManifoldFunction manifoldFunction, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<ManifoldFunction, ulong>(manifoldFunction, blockParameter);
        }

        
        public Task<ulong> ManifoldQueryAsync(BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<ManifoldFunction, ulong>(null, blockParameter);
        }

        public Task<ulong> MonopoleQueryAsync(MonopoleFunction monopoleFunction, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<MonopoleFunction, ulong>(monopoleFunction, blockParameter);
        }

        
        public Task<ulong> MonopoleQueryAsync(BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<MonopoleFunction, ulong>(null, blockParameter);
        }

        public Task<ulong> MotzkinPrimeQueryAsync(MotzkinPrimeFunction motzkinPrimeFunction, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<MotzkinPrimeFunction, ulong>(motzkinPrimeFunction, blockParameter);
        }

        
        public Task<ulong> MotzkinPrimeQueryAsync(BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<MotzkinPrimeFunction, ulong>(null, blockParameter);
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

        public Task<string> ReactRequestAsync(ulong pi)
        {
            var reactFunction = new ReactFunction();
                reactFunction.Pi = pi;
            
             return ContractHandler.SendRequestAsync(reactFunction);
        }

        public Task<TransactionReceipt> ReactRequestAndWaitForReceiptAsync(ulong pi, CancellationTokenSource cancellationToken = null)
        {
            var reactFunction = new ReactFunction();
                reactFunction.Pi = pi;
            
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

        public Task<RhoOutputDTO> RhoQueryAsync(RhoFunction rhoFunction, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryDeserializingToObjectAsync<RhoFunction, RhoOutputDTO>(rhoFunction, blockParameter);
        }

        public Task<RhoOutputDTO> RhoQueryAsync(BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryDeserializingToObjectAsync<RhoFunction, RhoOutputDTO>(null, blockParameter);
        }

        public Task<string> RodQueryAsync(RodFunction rodFunction, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<RodFunction, string>(rodFunction, blockParameter);
        }

        
        public Task<string> RodQueryAsync(BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<RodFunction, string>(null, blockParameter);
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
                typeof(ConeFunction),
                typeof(GenerateFunction),
                typeof(GetMarketRateFunction),
                typeof(IsolateFunction),
                typeof(IsomerizeFunction),
                typeof(LogFunction),
                typeof(MagnetizeFunction),
                typeof(ManifoldFunction),
                typeof(MonopoleFunction),
                typeof(MotzkinPrimeFunction),
                typeof(PurchaseFunction),
                typeof(ReactFunction),
                typeof(RedeemFunction),
                typeof(RenameFunction),
                typeof(RhoFunction),
                typeof(RodFunction),
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
                typeof(LogEventEventDTO),
                typeof(OwnershipUpdateEventDTO),
                typeof(TransferEventDTO)
            };
        }

        public override List<Type> GetAllErrorTypes()
        {
            return new List<Type>
            {
                typeof(BarnInequalityError),
                typeof(DysnomiaInsufficientAllowanceError),
                typeof(DysnomiaInsufficientBalanceError),
                typeof(ManifoldInequalityError),
                typeof(MarketRateNotFoundError),
                typeof(OwnableInvalidOwnerError),
                typeof(OwnableUnauthorizedAccountError),
                typeof(ReactionInequalityErrorError),
                typeof(ReactionZeroErrorError),
                typeof(RingInequalityError)
            };
        }
    }
}
