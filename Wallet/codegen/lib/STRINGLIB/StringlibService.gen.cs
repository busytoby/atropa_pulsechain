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
using Dysnomia.Contracts.lib.STRINGLIB.ContractDefinition;

namespace Dysnomia.Contracts.lib.STRINGLIB
{
    public partial class StringlibService: ContractWeb3ServiceBase
    {
        public static Task<TransactionReceipt> DeployContractAndWaitForReceiptAsync(Nethereum.Web3.IWeb3 web3, StringlibDeployment stringlibDeployment, CancellationTokenSource cancellationTokenSource = null)
        {
            return web3.Eth.GetContractDeploymentHandler<StringlibDeployment>().SendRequestAndWaitForReceiptAsync(stringlibDeployment, cancellationTokenSource);
        }

        public static Task<string> DeployContractAsync(Nethereum.Web3.IWeb3 web3, StringlibDeployment stringlibDeployment)
        {
            return web3.Eth.GetContractDeploymentHandler<StringlibDeployment>().SendRequestAsync(stringlibDeployment);
        }

        public static async Task<StringlibService> DeployContractAndGetServiceAsync(Nethereum.Web3.IWeb3 web3, StringlibDeployment stringlibDeployment, CancellationTokenSource cancellationTokenSource = null)
        {
            var receipt = await DeployContractAndWaitForReceiptAsync(web3, stringlibDeployment, cancellationTokenSource);
            return new StringlibService(web3, receipt.ContractAddress);
        }

        public StringlibService(Nethereum.Web3.IWeb3 web3, string contractAddress) : base(web3, contractAddress)
        {
        }

        public Task<bool> CaseInsensitiveCompareQueryAsync(CaseInsensitiveCompareFunction caseInsensitiveCompareFunction, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<CaseInsensitiveCompareFunction, bool>(caseInsensitiveCompareFunction, blockParameter);
        }

        
        public Task<bool> CaseInsensitiveCompareQueryAsync(byte[] a, byte[] b, BlockParameter blockParameter = null)
        {
            var caseInsensitiveCompareFunction = new CaseInsensitiveCompareFunction();
                caseInsensitiveCompareFunction.A = a;
                caseInsensitiveCompareFunction.B = b;
            
            return ContractHandler.QueryAsync<CaseInsensitiveCompareFunction, bool>(caseInsensitiveCompareFunction, blockParameter);
        }

        public Task<bool> CheckAcronymQueryAsync(CheckAcronymFunction checkAcronymFunction, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<CheckAcronymFunction, bool>(checkAcronymFunction, blockParameter);
        }

        
        public Task<bool> CheckAcronymQueryAsync(string a, string b, BlockParameter blockParameter = null)
        {
            var checkAcronymFunction = new CheckAcronymFunction();
                checkAcronymFunction.A = a;
                checkAcronymFunction.B = b;
            
            return ContractHandler.QueryAsync<CheckAcronymFunction, bool>(checkAcronymFunction, blockParameter);
        }

        public Task<bool> CheckAcronymQueryAsync(CheckAcronym1Function checkAcronym1Function, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<CheckAcronym1Function, bool>(checkAcronym1Function, blockParameter);
        }

        
        public Task<bool> CheckAcronymQueryAsync(byte[] acronym, string beta, BlockParameter blockParameter = null)
        {
            var checkAcronym1Function = new CheckAcronym1Function();
                checkAcronym1Function.Acronym = acronym;
                checkAcronym1Function.Beta = beta;
            
            return ContractHandler.QueryAsync<CheckAcronym1Function, bool>(checkAcronym1Function, blockParameter);
        }

        public Task<bool> CheckPalindromeQueryAsync(CheckPalindromeFunction checkPalindromeFunction, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<CheckPalindromeFunction, bool>(checkPalindromeFunction, blockParameter);
        }

        
        public Task<bool> CheckPalindromeQueryAsync(string s, BlockParameter blockParameter = null)
        {
            var checkPalindromeFunction = new CheckPalindromeFunction();
                checkPalindromeFunction.S = s;
            
            return ContractHandler.QueryAsync<CheckPalindromeFunction, bool>(checkPalindromeFunction, blockParameter);
        }

        public Task<bool> CheckPalindromeQueryAsync(CheckPalindrome1Function checkPalindrome1Function, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<CheckPalindrome1Function, bool>(checkPalindrome1Function, blockParameter);
        }

        
        public Task<bool> CheckPalindromeQueryAsync(byte[] s, BlockParameter blockParameter = null)
        {
            var checkPalindrome1Function = new CheckPalindrome1Function();
                checkPalindrome1Function.S = s;
            
            return ContractHandler.QueryAsync<CheckPalindrome1Function, bool>(checkPalindrome1Function, blockParameter);
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

        public Task<string> HexQueryAsync(HexFunction hexFunction, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<HexFunction, string>(hexFunction, blockParameter);
        }

        
        public Task<string> HexQueryAsync(byte[] value, BlockParameter blockParameter = null)
        {
            var hexFunction = new HexFunction();
                hexFunction.Value = value;
            
            return ContractHandler.QueryAsync<HexFunction, string>(hexFunction, blockParameter);
        }

        public Task<string> HexQueryAsync(Hex1Function hex1Function, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<Hex1Function, string>(hex1Function, blockParameter);
        }

        
        public Task<string> HexQueryAsync(BigInteger value, BlockParameter blockParameter = null)
        {
            var hex1Function = new Hex1Function();
                hex1Function.Value = value;
            
            return ContractHandler.QueryAsync<Hex1Function, string>(hex1Function, blockParameter);
        }

        public Task<string> HexQueryAsync(Hex2Function hex2Function, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<Hex2Function, string>(hex2Function, blockParameter);
        }

        public Task<string> HexQueryAsync(Hex3Function hex3Function, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<Hex3Function, string>(hex3Function, blockParameter);
        }

        
        public Task<string> HexQueryAsync(string account, BlockParameter blockParameter = null)
        {
            var hex3Function = new Hex3Function();
                hex3Function.Account = account;
            
            return ContractHandler.QueryAsync<Hex3Function, string>(hex3Function, blockParameter);
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

        public Task<string> RandomAcronymRequestAsync(RandomAcronymFunction randomAcronymFunction)
        {
             return ContractHandler.SendRequestAsync(randomAcronymFunction);
        }

        public Task<TransactionReceipt> RandomAcronymRequestAndWaitForReceiptAsync(RandomAcronymFunction randomAcronymFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(randomAcronymFunction, cancellationToken);
        }

        public Task<string> RandomAcronymRequestAsync(byte maxLength)
        {
            var randomAcronymFunction = new RandomAcronymFunction();
                randomAcronymFunction.MaxLength = maxLength;
            
             return ContractHandler.SendRequestAsync(randomAcronymFunction);
        }

        public Task<TransactionReceipt> RandomAcronymRequestAndWaitForReceiptAsync(byte maxLength, CancellationTokenSource cancellationToken = null)
        {
            var randomAcronymFunction = new RandomAcronymFunction();
                randomAcronymFunction.MaxLength = maxLength;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(randomAcronymFunction, cancellationToken);
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

        public Task<byte[]> ReverseQueryAsync(ReverseFunction reverseFunction, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<ReverseFunction, byte[]>(reverseFunction, blockParameter);
        }

        
        public Task<byte[]> ReverseQueryAsync(byte[] s, BlockParameter blockParameter = null)
        {
            var reverseFunction = new ReverseFunction();
                reverseFunction.S = s;
            
            return ContractHandler.QueryAsync<ReverseFunction, byte[]>(reverseFunction, blockParameter);
        }

        public Task<string> ReverseQueryAsync(Reverse1Function reverse1Function, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<Reverse1Function, string>(reverse1Function, blockParameter);
        }

        
        public Task<string> ReverseQueryAsync(string s, BlockParameter blockParameter = null)
        {
            var reverse1Function = new Reverse1Function();
                reverse1Function.S = s;
            
            return ContractHandler.QueryAsync<Reverse1Function, string>(reverse1Function, blockParameter);
        }

        public Task<string> StringQueryAsync(StringFunction @stringFunction, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<StringFunction, string>(@stringFunction, blockParameter);
        }

        
        public Task<string> StringQueryAsync(BigInteger value, BlockParameter blockParameter = null)
        {
            var @stringFunction = new StringFunction();
                @stringFunction.Value = value;
            
            return ContractHandler.QueryAsync<StringFunction, string>(@stringFunction, blockParameter);
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

        public Task<BigInteger> Log10QueryAsync(Log10Function log10Function, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<Log10Function, BigInteger>(log10Function, blockParameter);
        }

        
        public Task<BigInteger> Log10QueryAsync(BigInteger value, BlockParameter blockParameter = null)
        {
            var log10Function = new Log10Function();
                log10Function.Value = value;
            
            return ContractHandler.QueryAsync<Log10Function, BigInteger>(log10Function, blockParameter);
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
                typeof(CaseInsensitiveCompareFunction),
                typeof(CheckAcronymFunction),
                typeof(CheckAcronym1Function),
                typeof(CheckPalindromeFunction),
                typeof(CheckPalindrome1Function),
                typeof(GetMarketRateFunction),
                typeof(HexFunction),
                typeof(Hex1Function),
                typeof(Hex2Function),
                typeof(Hex3Function),
                typeof(MotzkinPrimeFunction),
                typeof(PurchaseFunction),
                typeof(RandomAcronymFunction),
                typeof(RedeemFunction),
                typeof(RenameFunction),
                typeof(ReverseFunction),
                typeof(Reverse1Function),
                typeof(StringFunction),
                typeof(TypeFunction),
                typeof(XiaoFunction),
                typeof(AddOwnerFunction),
                typeof(AllowanceFunction),
                typeof(ApproveFunction),
                typeof(BalanceOfFunction),
                typeof(DecimalsFunction),
                typeof(Log10Function),
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
                typeof(MinimumLength3Error),
                typeof(OwnableInvalidOwnerError),
                typeof(OwnableUnauthorizedAccountError)
            };
        }
    }
}
