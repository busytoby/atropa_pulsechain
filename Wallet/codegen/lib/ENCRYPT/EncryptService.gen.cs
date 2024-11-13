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
using Dysnomia.Contracts.lib.ENCRYPT.ContractDefinition;

namespace Dysnomia.Contracts.lib.ENCRYPT
{
    public partial class EncryptService: ContractWeb3ServiceBase
    {
        public static Task<TransactionReceipt> DeployContractAndWaitForReceiptAsync(Nethereum.Web3.IWeb3 web3, EncryptDeployment encryptDeployment, CancellationTokenSource cancellationTokenSource = null)
        {
            return web3.Eth.GetContractDeploymentHandler<EncryptDeployment>().SendRequestAndWaitForReceiptAsync(encryptDeployment, cancellationTokenSource);
        }

        public static Task<string> DeployContractAsync(Nethereum.Web3.IWeb3 web3, EncryptDeployment encryptDeployment)
        {
            return web3.Eth.GetContractDeploymentHandler<EncryptDeployment>().SendRequestAsync(encryptDeployment);
        }

        public static async Task<EncryptService> DeployContractAndGetServiceAsync(Nethereum.Web3.IWeb3 web3, EncryptDeployment encryptDeployment, CancellationTokenSource cancellationTokenSource = null)
        {
            var receipt = await DeployContractAndWaitForReceiptAsync(web3, encryptDeployment, cancellationTokenSource);
            return new EncryptService(web3, receipt.ContractAddress);
        }

        public EncryptService(Nethereum.Web3.IWeb3 web3, string contractAddress) : base(web3, contractAddress)
        {
        }

        public Task<string> DecryptRequestAsync(DecryptFunction decryptFunction)
        {
             return ContractHandler.SendRequestAsync(decryptFunction);
        }

        public Task<TransactionReceipt> DecryptRequestAndWaitForReceiptAsync(DecryptFunction decryptFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(decryptFunction, cancellationToken);
        }

        public Task<string> DecryptRequestAsync(ulong from, ulong to, ulong index, byte[] key)
        {
            var decryptFunction = new DecryptFunction();
                decryptFunction.From = from;
                decryptFunction.To = to;
                decryptFunction.Index = index;
                decryptFunction.Key = key;
            
             return ContractHandler.SendRequestAsync(decryptFunction);
        }

        public Task<TransactionReceipt> DecryptRequestAndWaitForReceiptAsync(ulong from, ulong to, ulong index, byte[] key, CancellationTokenSource cancellationToken = null)
        {
            var decryptFunction = new DecryptFunction();
                decryptFunction.From = from;
                decryptFunction.To = to;
                decryptFunction.Index = index;
                decryptFunction.Key = key;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(decryptFunction, cancellationToken);
        }

        public Task<string> DecryptRequestAsync(Decrypt1Function decrypt1Function)
        {
             return ContractHandler.SendRequestAsync(decrypt1Function);
        }

        public Task<TransactionReceipt> DecryptRequestAndWaitForReceiptAsync(Decrypt1Function decrypt1Function, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(decrypt1Function, cancellationToken);
        }

        public Task<string> DecryptRequestAsync(ulong from, ulong to, ulong index, string key)
        {
            var decrypt1Function = new Decrypt1Function();
                decrypt1Function.From = from;
                decrypt1Function.To = to;
                decrypt1Function.Index = index;
                decrypt1Function.Key = key;
            
             return ContractHandler.SendRequestAsync(decrypt1Function);
        }

        public Task<TransactionReceipt> DecryptRequestAndWaitForReceiptAsync(ulong from, ulong to, ulong index, string key, CancellationTokenSource cancellationToken = null)
        {
            var decrypt1Function = new Decrypt1Function();
                decrypt1Function.From = from;
                decrypt1Function.To = to;
                decrypt1Function.Index = index;
                decrypt1Function.Key = key;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(decrypt1Function, cancellationToken);
        }

        public Task<string> EncapsulateRequestAsync(EncapsulateFunction encapsulateFunction)
        {
             return ContractHandler.SendRequestAsync(encapsulateFunction);
        }

        public Task<TransactionReceipt> EncapsulateRequestAndWaitForReceiptAsync(EncapsulateFunction encapsulateFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(encapsulateFunction, cancellationToken);
        }

        public Task<string> EncapsulateRequestAsync(Bao from, ulong gamma, ulong rho, ulong upsilon, ulong ohm)
        {
            var encapsulateFunction = new EncapsulateFunction();
                encapsulateFunction.From = from;
                encapsulateFunction.Gamma = gamma;
                encapsulateFunction.Rho = rho;
                encapsulateFunction.Upsilon = upsilon;
                encapsulateFunction.Ohm = ohm;
            
             return ContractHandler.SendRequestAsync(encapsulateFunction);
        }

        public Task<TransactionReceipt> EncapsulateRequestAndWaitForReceiptAsync(Bao from, ulong gamma, ulong rho, ulong upsilon, ulong ohm, CancellationTokenSource cancellationToken = null)
        {
            var encapsulateFunction = new EncapsulateFunction();
                encapsulateFunction.From = from;
                encapsulateFunction.Gamma = gamma;
                encapsulateFunction.Rho = rho;
                encapsulateFunction.Upsilon = upsilon;
                encapsulateFunction.Ohm = ohm;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(encapsulateFunction, cancellationToken);
        }

        public Task<string> EncryptRequestAsync(EncryptFunction encryptFunction)
        {
             return ContractHandler.SendRequestAsync(encryptFunction);
        }

        public Task<TransactionReceipt> EncryptRequestAndWaitForReceiptAsync(EncryptFunction encryptFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(encryptFunction, cancellationToken);
        }

        public Task<string> EncryptRequestAsync(ulong from, ulong to, string key, string data)
        {
            var encryptFunction = new EncryptFunction();
                encryptFunction.From = from;
                encryptFunction.To = to;
                encryptFunction.Key = key;
                encryptFunction.Data = data;
            
             return ContractHandler.SendRequestAsync(encryptFunction);
        }

        public Task<TransactionReceipt> EncryptRequestAndWaitForReceiptAsync(ulong from, ulong to, string key, string data, CancellationTokenSource cancellationToken = null)
        {
            var encryptFunction = new EncryptFunction();
                encryptFunction.From = from;
                encryptFunction.To = to;
                encryptFunction.Key = key;
                encryptFunction.Data = data;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(encryptFunction, cancellationToken);
        }

        public Task<string> EncryptRequestAsync(Encrypt1Function encrypt1Function)
        {
             return ContractHandler.SendRequestAsync(encrypt1Function);
        }

        public Task<TransactionReceipt> EncryptRequestAndWaitForReceiptAsync(Encrypt1Function encrypt1Function, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(encrypt1Function, cancellationToken);
        }

        public Task<string> EncryptRequestAsync(ulong from, ulong to, byte[] key, byte[] data)
        {
            var encrypt1Function = new Encrypt1Function();
                encrypt1Function.From = from;
                encrypt1Function.To = to;
                encrypt1Function.Key = key;
                encrypt1Function.Data = data;
            
             return ContractHandler.SendRequestAsync(encrypt1Function);
        }

        public Task<TransactionReceipt> EncryptRequestAndWaitForReceiptAsync(ulong from, ulong to, byte[] key, byte[] data, CancellationTokenSource cancellationToken = null)
        {
            var encrypt1Function = new Encrypt1Function();
                encrypt1Function.From = from;
                encrypt1Function.To = to;
                encrypt1Function.Key = key;
                encrypt1Function.Data = data;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(encrypt1Function, cancellationToken);
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

        public Task<string> PruneRequestAsync(PruneFunction pruneFunction)
        {
             return ContractHandler.SendRequestAsync(pruneFunction);
        }

        public Task<string> PruneRequestAsync()
        {
             return ContractHandler.SendRequestAsync<PruneFunction>();
        }

        public Task<TransactionReceipt> PruneRequestAndWaitForReceiptAsync(PruneFunction pruneFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(pruneFunction, cancellationToken);
        }

        public Task<TransactionReceipt> PruneRequestAndWaitForReceiptAsync(CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync<PruneFunction>(null, cancellationToken);
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

        public Task<List<ulong>> SaatQueryAsync(SaatFunction saatFunction, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<SaatFunction, List<ulong>>(saatFunction, blockParameter);
        }

        
        public Task<List<ulong>> SaatQueryAsync(byte[] geng, BlockParameter blockParameter = null)
        {
            var saatFunction = new SaatFunction();
                saatFunction.Geng = geng;
            
            return ContractHandler.QueryAsync<SaatFunction, List<ulong>>(saatFunction, blockParameter);
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

        public Task<string> ZhengQueryAsync(ZhengFunction zhengFunction, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<ZhengFunction, string>(zhengFunction, blockParameter);
        }

        
        public Task<string> ZhengQueryAsync(BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<ZhengFunction, string>(null, blockParameter);
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

        public Task<string> HashKeyRequestAsync(HashKeyFunction hashKeyFunction)
        {
             return ContractHandler.SendRequestAsync(hashKeyFunction);
        }

        public Task<TransactionReceipt> HashKeyRequestAndWaitForReceiptAsync(HashKeyFunction hashKeyFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(hashKeyFunction, cancellationToken);
        }

        public Task<string> HashKeyRequestAsync(byte[] key)
        {
            var hashKeyFunction = new HashKeyFunction();
                hashKeyFunction.Key = key;
            
             return ContractHandler.SendRequestAsync(hashKeyFunction);
        }

        public Task<TransactionReceipt> HashKeyRequestAndWaitForReceiptAsync(byte[] key, CancellationTokenSource cancellationToken = null)
        {
            var hashKeyFunction = new HashKeyFunction();
                hashKeyFunction.Key = key;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(hashKeyFunction, cancellationToken);
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
                typeof(DecryptFunction),
                typeof(Decrypt1Function),
                typeof(EncapsulateFunction),
                typeof(EncryptFunction),
                typeof(Encrypt1Function),
                typeof(GetMarketRateFunction),
                typeof(MotzkinPrimeFunction),
                typeof(PruneFunction),
                typeof(PurchaseFunction),
                typeof(RedeemFunction),
                typeof(RenameFunction),
                typeof(SaatFunction),
                typeof(TypeFunction),
                typeof(VoidFunction),
                typeof(XiaoFunction),
                typeof(ZhengFunction),
                typeof(AddOwnerFunction),
                typeof(AllowanceFunction),
                typeof(ApproveFunction),
                typeof(BalanceOfFunction),
                typeof(DecimalsFunction),
                typeof(HashKeyFunction),
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
                typeof(ChromosomeLengthError),
                typeof(DysnomiaInsufficientAllowanceError),
                typeof(DysnomiaInsufficientBalanceError),
                typeof(MarketRateNotFoundError),
                typeof(NotPartyError),
                typeof(OwnableInvalidOwnerError),
                typeof(OwnableUnauthorizedAccountError),
                typeof(TooManyCryptsError)
            };
        }
    }
}
