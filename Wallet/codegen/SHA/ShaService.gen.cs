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
using Dysnomia.Contracts.SHA.ContractDefinition;

namespace Dysnomia.Contracts.SHA
{
    public partial class ShaService: ContractWeb3ServiceBase
    {
        public static Task<TransactionReceipt> DeployContractAndWaitForReceiptAsync(Nethereum.Web3.IWeb3 web3, ShaDeployment shaDeployment, CancellationTokenSource cancellationTokenSource = null)
        {
            return web3.Eth.GetContractDeploymentHandler<ShaDeployment>().SendRequestAndWaitForReceiptAsync(shaDeployment, cancellationTokenSource);
        }

        public static Task<string> DeployContractAsync(Nethereum.Web3.IWeb3 web3, ShaDeployment shaDeployment)
        {
            return web3.Eth.GetContractDeploymentHandler<ShaDeployment>().SendRequestAsync(shaDeployment);
        }

        public static async Task<ShaService> DeployContractAndGetServiceAsync(Nethereum.Web3.IWeb3 web3, ShaDeployment shaDeployment, CancellationTokenSource cancellationTokenSource = null)
        {
            var receipt = await DeployContractAndWaitForReceiptAsync(web3, shaDeployment, cancellationTokenSource);
            return new ShaService(web3, receipt.ContractAddress);
        }

        public ShaService(Nethereum.Web3.IWeb3 web3, string contractAddress) : base(web3, contractAddress)
        {
        }

        public Task<string> AdductRequestAsync(AdductFunction adductFunction)
        {
             return ContractHandler.SendRequestAsync(adductFunction);
        }

        public Task<TransactionReceipt> AdductRequestAndWaitForReceiptAsync(AdductFunction adductFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(adductFunction, cancellationToken);
        }

        public Task<string> AdductRequestAsync(ulong phi)
        {
            var adductFunction = new AdductFunction();
                adductFunction.Phi = phi;
            
             return ContractHandler.SendRequestAsync(adductFunction);
        }

        public Task<TransactionReceipt> AdductRequestAndWaitForReceiptAsync(ulong phi, CancellationTokenSource cancellationToken = null)
        {
            var adductFunction = new AdductFunction();
                adductFunction.Phi = phi;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(adductFunction, cancellationToken);
        }

        public Task<string> AvailRequestAsync(AvailFunction availFunction)
        {
             return ContractHandler.SendRequestAsync(availFunction);
        }

        public Task<TransactionReceipt> AvailRequestAndWaitForReceiptAsync(AvailFunction availFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(availFunction, cancellationToken);
        }

        public Task<string> AvailRequestAsync(ulong xi)
        {
            var availFunction = new AvailFunction();
                availFunction.Xi = xi;
            
             return ContractHandler.SendRequestAsync(availFunction);
        }

        public Task<TransactionReceipt> AvailRequestAndWaitForReceiptAsync(ulong xi, CancellationTokenSource cancellationToken = null)
        {
            var availFunction = new AvailFunction();
                availFunction.Xi = xi;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(availFunction, cancellationToken);
        }

        public Task<string> BondRequestAsync(BondFunction bondFunction)
        {
             return ContractHandler.SendRequestAsync(bondFunction);
        }

        public Task<string> BondRequestAsync()
        {
             return ContractHandler.SendRequestAsync<BondFunction>();
        }

        public Task<TransactionReceipt> BondRequestAndWaitForReceiptAsync(BondFunction bondFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(bondFunction, cancellationToken);
        }

        public Task<TransactionReceipt> BondRequestAndWaitForReceiptAsync(CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync<BondFunction>(null, cancellationToken);
        }

        public Task<string> ConifyRequestAsync(ConifyFunction conifyFunction)
        {
             return ContractHandler.SendRequestAsync(conifyFunction);
        }

        public Task<TransactionReceipt> ConifyRequestAndWaitForReceiptAsync(ConifyFunction conifyFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(conifyFunction, cancellationToken);
        }

        public Task<string> ConifyRequestAsync(ulong beta)
        {
            var conifyFunction = new ConifyFunction();
                conifyFunction.Beta = beta;
            
             return ContractHandler.SendRequestAsync(conifyFunction);
        }

        public Task<TransactionReceipt> ConifyRequestAndWaitForReceiptAsync(ulong beta, CancellationTokenSource cancellationToken = null)
        {
            var conifyFunction = new ConifyFunction();
                conifyFunction.Beta = beta;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(conifyFunction, cancellationToken);
        }

        public Task<string> ConjugateRequestAsync(ConjugateFunction conjugateFunction)
        {
             return ContractHandler.SendRequestAsync(conjugateFunction);
        }

        public Task<TransactionReceipt> ConjugateRequestAndWaitForReceiptAsync(ConjugateFunction conjugateFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(conjugateFunction, cancellationToken);
        }

        public Task<string> ConjugateRequestAsync(ulong chi)
        {
            var conjugateFunction = new ConjugateFunction();
                conjugateFunction.Chi = chi;
            
             return ContractHandler.SendRequestAsync(conjugateFunction);
        }

        public Task<TransactionReceipt> ConjugateRequestAndWaitForReceiptAsync(ulong chi, CancellationTokenSource cancellationToken = null)
        {
            var conjugateFunction = new ConjugateFunction();
                conjugateFunction.Chi = chi;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(conjugateFunction, cancellationToken);
        }

        public Task<ulong> DynamoQueryAsync(DynamoFunction dynamoFunction, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<DynamoFunction, ulong>(dynamoFunction, blockParameter);
        }

        
        public Task<ulong> DynamoQueryAsync(BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<DynamoFunction, ulong>(null, blockParameter);
        }

        public Task<string> FormRequestAsync(FormFunction formFunction)
        {
             return ContractHandler.SendRequestAsync(formFunction);
        }

        public Task<TransactionReceipt> FormRequestAndWaitForReceiptAsync(FormFunction formFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(formFunction, cancellationToken);
        }

        public Task<string> FormRequestAsync(ulong chi)
        {
            var formFunction = new FormFunction();
                formFunction.Chi = chi;
            
             return ContractHandler.SendRequestAsync(formFunction);
        }

        public Task<TransactionReceipt> FormRequestAndWaitForReceiptAsync(ulong chi, CancellationTokenSource cancellationToken = null)
        {
            var formFunction = new FormFunction();
                formFunction.Chi = chi;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(formFunction, cancellationToken);
        }

        public Task<string> FuseRequestAsync(FuseFunction fuseFunction)
        {
             return ContractHandler.SendRequestAsync(fuseFunction);
        }

        public Task<TransactionReceipt> FuseRequestAndWaitForReceiptAsync(FuseFunction fuseFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(fuseFunction, cancellationToken);
        }

        public Task<string> FuseRequestAsync(ulong rho, ulong upsilon, ulong ohm)
        {
            var fuseFunction = new FuseFunction();
                fuseFunction.Rho = rho;
                fuseFunction.Upsilon = upsilon;
                fuseFunction.Ohm = ohm;
            
             return ContractHandler.SendRequestAsync(fuseFunction);
        }

        public Task<TransactionReceipt> FuseRequestAndWaitForReceiptAsync(ulong rho, ulong upsilon, ulong ohm, CancellationTokenSource cancellationToken = null)
        {
            var fuseFunction = new FuseFunction();
                fuseFunction.Rho = rho;
                fuseFunction.Upsilon = upsilon;
                fuseFunction.Ohm = ohm;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(fuseFunction, cancellationToken);
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

        public Task<string> PolarizeRequestAsync(PolarizeFunction polarizeFunction)
        {
             return ContractHandler.SendRequestAsync(polarizeFunction);
        }

        public Task<string> PolarizeRequestAsync()
        {
             return ContractHandler.SendRequestAsync<PolarizeFunction>();
        }

        public Task<TransactionReceipt> PolarizeRequestAndWaitForReceiptAsync(PolarizeFunction polarizeFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(polarizeFunction, cancellationToken);
        }

        public Task<TransactionReceipt> PolarizeRequestAndWaitForReceiptAsync(CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync<PolarizeFunction>(null, cancellationToken);
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

        public Task<string> ReactRequestAsync(ulong pi, ulong theta)
        {
            var reactFunction = new ReactFunction();
                reactFunction.Pi = pi;
                reactFunction.Theta = theta;
            
             return ContractHandler.SendRequestAsync(reactFunction);
        }

        public Task<TransactionReceipt> ReactRequestAndWaitForReceiptAsync(ulong pi, ulong theta, CancellationTokenSource cancellationToken = null)
        {
            var reactFunction = new ReactFunction();
                reactFunction.Pi = pi;
                reactFunction.Theta = theta;
            
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

        public Task<string> SaturateRequestAsync(SaturateFunction saturateFunction)
        {
             return ContractHandler.SendRequestAsync(saturateFunction);
        }

        public Task<TransactionReceipt> SaturateRequestAndWaitForReceiptAsync(SaturateFunction saturateFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(saturateFunction, cancellationToken);
        }

        public Task<string> SaturateRequestAsync(ulong beta, ulong epsilon, ulong theta)
        {
            var saturateFunction = new SaturateFunction();
                saturateFunction.Beta = beta;
                saturateFunction.Epsilon = epsilon;
                saturateFunction.Theta = theta;
            
             return ContractHandler.SendRequestAsync(saturateFunction);
        }

        public Task<TransactionReceipt> SaturateRequestAndWaitForReceiptAsync(ulong beta, ulong epsilon, ulong theta, CancellationTokenSource cancellationToken = null)
        {
            var saturateFunction = new SaturateFunction();
                saturateFunction.Beta = beta;
                saturateFunction.Epsilon = epsilon;
                saturateFunction.Theta = theta;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(saturateFunction, cancellationToken);
        }

        public Task<string> TypeQueryAsync(TypeFunction typeFunction, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<TypeFunction, string>(typeFunction, blockParameter);
        }

        
        public Task<string> TypeQueryAsync(BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<TypeFunction, string>(null, blockParameter);
        }

        public Task<ViewOutputDTO> ViewQueryAsync(ViewFunction viewFunction, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryDeserializingToObjectAsync<ViewFunction, ViewOutputDTO>(viewFunction, blockParameter);
        }

        public Task<ViewOutputDTO> ViewQueryAsync(BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryDeserializingToObjectAsync<ViewFunction, ViewOutputDTO>(null, blockParameter);
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
                typeof(AdductFunction),
                typeof(AvailFunction),
                typeof(BondFunction),
                typeof(ConifyFunction),
                typeof(ConjugateFunction),
                typeof(DynamoFunction),
                typeof(FormFunction),
                typeof(FuseFunction),
                typeof(GetMarketRateFunction),
                typeof(MotzkinPrimeFunction),
                typeof(PolarizeFunction),
                typeof(PurchaseFunction),
                typeof(ReactFunction),
                typeof(RedeemFunction),
                typeof(RenameFunction),
                typeof(SaturateFunction),
                typeof(TypeFunction),
                typeof(ViewFunction),
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
                typeof(OwnableUnauthorizedAccountError),
                typeof(ReactionZeroErrorError)
            };
        }
    }
}
