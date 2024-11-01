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
using Dysnomia.Contracts.lib.ATTRIBUTE.ContractDefinition;

namespace Dysnomia.Contracts.lib.ATTRIBUTE
{
    public partial class AttributeService: ContractWeb3ServiceBase
    {
        public static Task<TransactionReceipt> DeployContractAndWaitForReceiptAsync(Nethereum.Web3.IWeb3 web3, AttributeDeployment attributeDeployment, CancellationTokenSource cancellationTokenSource = null)
        {
            return web3.Eth.GetContractDeploymentHandler<AttributeDeployment>().SendRequestAndWaitForReceiptAsync(attributeDeployment, cancellationTokenSource);
        }

        public static Task<string> DeployContractAsync(Nethereum.Web3.IWeb3 web3, AttributeDeployment attributeDeployment)
        {
            return web3.Eth.GetContractDeploymentHandler<AttributeDeployment>().SendRequestAsync(attributeDeployment);
        }

        public static async Task<AttributeService> DeployContractAndGetServiceAsync(Nethereum.Web3.IWeb3 web3, AttributeDeployment attributeDeployment, CancellationTokenSource cancellationTokenSource = null)
        {
            var receipt = await DeployContractAndWaitForReceiptAsync(web3, attributeDeployment, cancellationTokenSource);
            return new AttributeService(web3, receipt.ContractAddress);
        }

        public AttributeService(Nethereum.Web3.IWeb3 web3, string contractAddress) : base(web3, contractAddress)
        {
        }

        public Task<string> AliasQueryAsync(AliasFunction aliasFunction, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<AliasFunction, string>(aliasFunction, blockParameter);
        }

        
        public Task<string> AliasQueryAsync(ulong soul, Bao entity, BlockParameter blockParameter = null)
        {
            var aliasFunction = new AliasFunction();
                aliasFunction.Soul = soul;
                aliasFunction.Entity = entity;
            
            return ContractHandler.QueryAsync<AliasFunction, string>(aliasFunction, blockParameter);
        }

        public Task<string> AliasQueryAsync(Alias1Function alias1Function, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<Alias1Function, string>(alias1Function, blockParameter);
        }

        
        public Task<string> AliasQueryAsync(ulong soul, string name, BlockParameter blockParameter = null)
        {
            var alias1Function = new Alias1Function();
                alias1Function.Soul = soul;
                alias1Function.Name = name;
            
            return ContractHandler.QueryAsync<Alias1Function, string>(alias1Function, blockParameter);
        }

        public Task<string> AliasRequestAsync(Alias2Function alias2Function)
        {
             return ContractHandler.SendRequestAsync(alias2Function);
        }

        public Task<TransactionReceipt> AliasRequestAndWaitForReceiptAsync(Alias2Function alias2Function, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(alias2Function, cancellationToken);
        }

        public Task<string> AliasRequestAsync(ulong soul, Bao entity, string value)
        {
            var alias2Function = new Alias2Function();
                alias2Function.Soul = soul;
                alias2Function.Entity = entity;
                alias2Function.Value = value;
            
             return ContractHandler.SendRequestAsync(alias2Function);
        }

        public Task<TransactionReceipt> AliasRequestAndWaitForReceiptAsync(ulong soul, Bao entity, string value, CancellationTokenSource cancellationToken = null)
        {
            var alias2Function = new Alias2Function();
                alias2Function.Soul = soul;
                alias2Function.Entity = entity;
                alias2Function.Value = value;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(alias2Function, cancellationToken);
        }

        public Task<string> AliasRequestAsync(Alias3Function alias3Function)
        {
             return ContractHandler.SendRequestAsync(alias3Function);
        }

        public Task<TransactionReceipt> AliasRequestAndWaitForReceiptAsync(Alias3Function alias3Function, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(alias3Function, cancellationToken);
        }

        public Task<string> AliasRequestAsync(ulong soul, string name, string value)
        {
            var alias3Function = new Alias3Function();
                alias3Function.Soul = soul;
                alias3Function.Name = name;
                alias3Function.Value = value;
            
             return ContractHandler.SendRequestAsync(alias3Function);
        }

        public Task<TransactionReceipt> AliasRequestAndWaitForReceiptAsync(ulong soul, string name, string value, CancellationTokenSource cancellationToken = null)
        {
            var alias3Function = new Alias3Function();
                alias3Function.Soul = soul;
                alias3Function.Name = name;
                alias3Function.Value = value;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(alias3Function, cancellationToken);
        }

        public Task<string> GetQueryAsync(GetFunction getFunction, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<GetFunction, string>(getFunction, blockParameter);
        }

        
        public Task<string> GetQueryAsync(ulong soul, string name, BlockParameter blockParameter = null)
        {
            var getFunction = new GetFunction();
                getFunction.Soul = soul;
                getFunction.Name = name;
            
            return ContractHandler.QueryAsync<GetFunction, string>(getFunction, blockParameter);
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

        public Task<string> SetRequestAsync(SetFunction setFunction)
        {
             return ContractHandler.SendRequestAsync(setFunction);
        }

        public Task<TransactionReceipt> SetRequestAndWaitForReceiptAsync(SetFunction setFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(setFunction, cancellationToken);
        }

        public Task<string> SetRequestAsync(ulong soul, string name, string value)
        {
            var setFunction = new SetFunction();
                setFunction.Soul = soul;
                setFunction.Name = name;
                setFunction.Value = value;
            
             return ContractHandler.SendRequestAsync(setFunction);
        }

        public Task<TransactionReceipt> SetRequestAndWaitForReceiptAsync(ulong soul, string name, string value, CancellationTokenSource cancellationToken = null)
        {
            var setFunction = new SetFunction();
                setFunction.Soul = soul;
                setFunction.Name = name;
                setFunction.Value = value;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(setFunction, cancellationToken);
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

        public Task<string> AddAttributeRequestAsync(AddAttributeFunction addAttributeFunction)
        {
             return ContractHandler.SendRequestAsync(addAttributeFunction);
        }

        public Task<TransactionReceipt> AddAttributeRequestAndWaitForReceiptAsync(AddAttributeFunction addAttributeFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(addAttributeFunction, cancellationToken);
        }

        public Task<string> AddAttributeRequestAsync(string name, byte maxLength)
        {
            var addAttributeFunction = new AddAttributeFunction();
                addAttributeFunction.Name = name;
                addAttributeFunction.MaxLength = maxLength;
            
             return ContractHandler.SendRequestAsync(addAttributeFunction);
        }

        public Task<TransactionReceipt> AddAttributeRequestAndWaitForReceiptAsync(string name, byte maxLength, CancellationTokenSource cancellationToken = null)
        {
            var addAttributeFunction = new AddAttributeFunction();
                addAttributeFunction.Name = name;
                addAttributeFunction.MaxLength = maxLength;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(addAttributeFunction, cancellationToken);
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

        public Task<string> RemoveAttributeRequestAsync(RemoveAttributeFunction removeAttributeFunction)
        {
             return ContractHandler.SendRequestAsync(removeAttributeFunction);
        }

        public Task<TransactionReceipt> RemoveAttributeRequestAndWaitForReceiptAsync(RemoveAttributeFunction removeAttributeFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(removeAttributeFunction, cancellationToken);
        }

        public Task<string> RemoveAttributeRequestAsync(string name)
        {
            var removeAttributeFunction = new RemoveAttributeFunction();
                removeAttributeFunction.Name = name;
            
             return ContractHandler.SendRequestAsync(removeAttributeFunction);
        }

        public Task<TransactionReceipt> RemoveAttributeRequestAndWaitForReceiptAsync(string name, CancellationTokenSource cancellationToken = null)
        {
            var removeAttributeFunction = new RemoveAttributeFunction();
                removeAttributeFunction.Name = name;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(removeAttributeFunction, cancellationToken);
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
                typeof(AliasFunction),
                typeof(Alias1Function),
                typeof(Alias2Function),
                typeof(Alias3Function),
                typeof(GetFunction),
                typeof(GetMarketRateFunction),
                typeof(MotzkinPrimeFunction),
                typeof(PurchaseFunction),
                typeof(RedeemFunction),
                typeof(RenameFunction),
                typeof(SetFunction),
                typeof(TypeFunction),
                typeof(VoidFunction),
                typeof(XiaoFunction),
                typeof(AddAttributeFunction),
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
                typeof(RemoveAttributeFunction),
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
                typeof(AliasNotFoundError),
                typeof(AttributeNotFoundError),
                typeof(DysnomiaInsufficientAllowanceError),
                typeof(DysnomiaInsufficientBalanceError),
                typeof(MarketRateNotFoundError),
                typeof(MaximumLengthError),
                typeof(OwnableInvalidOwnerError),
                typeof(OwnableUnauthorizedAccountError)
            };
        }
    }
}
