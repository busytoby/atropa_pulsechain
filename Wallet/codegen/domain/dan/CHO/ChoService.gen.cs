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
using Dysnomia.Contracts.domain.dan.CHO.ContractDefinition;

namespace Dysnomia.Contracts.domain.dan.CHO
{
    public partial class ChoService: ContractWeb3ServiceBase
    {
        public static Task<TransactionReceipt> DeployContractAndWaitForReceiptAsync(Nethereum.Web3.IWeb3 web3, ChoDeployment choDeployment, CancellationTokenSource cancellationTokenSource = null)
        {
            return web3.Eth.GetContractDeploymentHandler<ChoDeployment>().SendRequestAndWaitForReceiptAsync(choDeployment, cancellationTokenSource);
        }

        public static Task<string> DeployContractAsync(Nethereum.Web3.IWeb3 web3, ChoDeployment choDeployment)
        {
            return web3.Eth.GetContractDeploymentHandler<ChoDeployment>().SendRequestAsync(choDeployment);
        }

        public static async Task<ChoService> DeployContractAndGetServiceAsync(Nethereum.Web3.IWeb3 web3, ChoDeployment choDeployment, CancellationTokenSource cancellationTokenSource = null)
        {
            var receipt = await DeployContractAndWaitForReceiptAsync(web3, choDeployment, cancellationTokenSource);
            return new ChoService(web3, receipt.ContractAddress);
        }

        public ChoService(Nethereum.Web3.IWeb3 web3, string contractAddress) : base(web3, contractAddress)
        {
        }

        public Task<string> AddContractOwnerRequestAsync(AddContractOwnerFunction addContractOwnerFunction)
        {
             return ContractHandler.SendRequestAsync(addContractOwnerFunction);
        }

        public Task<TransactionReceipt> AddContractOwnerRequestAndWaitForReceiptAsync(AddContractOwnerFunction addContractOwnerFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(addContractOwnerFunction, cancellationToken);
        }

        public Task<string> AddContractOwnerRequestAsync(string contract, string owner)
        {
            var addContractOwnerFunction = new AddContractOwnerFunction();
                addContractOwnerFunction.Contract = contract;
                addContractOwnerFunction.Owner = owner;
            
             return ContractHandler.SendRequestAsync(addContractOwnerFunction);
        }

        public Task<TransactionReceipt> AddContractOwnerRequestAndWaitForReceiptAsync(string contract, string owner, CancellationTokenSource cancellationToken = null)
        {
            var addContractOwnerFunction = new AddContractOwnerFunction();
                addContractOwnerFunction.Contract = contract;
                addContractOwnerFunction.Owner = owner;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(addContractOwnerFunction, cancellationToken);
        }

        public Task<string> AddLibraryOwnerRequestAsync(AddLibraryOwnerFunction addLibraryOwnerFunction)
        {
             return ContractHandler.SendRequestAsync(addLibraryOwnerFunction);
        }

        public Task<TransactionReceipt> AddLibraryOwnerRequestAndWaitForReceiptAsync(AddLibraryOwnerFunction addLibraryOwnerFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(addLibraryOwnerFunction, cancellationToken);
        }

        public Task<string> AddLibraryOwnerRequestAsync(string what)
        {
            var addLibraryOwnerFunction = new AddLibraryOwnerFunction();
                addLibraryOwnerFunction.What = what;
            
             return ContractHandler.SendRequestAsync(addLibraryOwnerFunction);
        }

        public Task<TransactionReceipt> AddLibraryOwnerRequestAndWaitForReceiptAsync(string what, CancellationTokenSource cancellationToken = null)
        {
            var addLibraryOwnerFunction = new AddLibraryOwnerFunction();
                addLibraryOwnerFunction.What = what;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(addLibraryOwnerFunction, cancellationToken);
        }

        public Task<string> AddSystemAddressRequestAsync(AddSystemAddressFunction addSystemAddressFunction)
        {
             return ContractHandler.SendRequestAsync(addSystemAddressFunction);
        }

        public Task<TransactionReceipt> AddSystemAddressRequestAndWaitForReceiptAsync(AddSystemAddressFunction addSystemAddressFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(addSystemAddressFunction, cancellationToken);
        }

        public Task<string> AddSystemAddressRequestAsync(string alias, string address)
        {
            var addSystemAddressFunction = new AddSystemAddressFunction();
                addSystemAddressFunction.Alias = alias;
                addSystemAddressFunction.Address = address;
            
             return ContractHandler.SendRequestAsync(addSystemAddressFunction);
        }

        public Task<TransactionReceipt> AddSystemAddressRequestAndWaitForReceiptAsync(string alias, string address, CancellationTokenSource cancellationToken = null)
        {
            var addSystemAddressFunction = new AddSystemAddressFunction();
                addSystemAddressFunction.Alias = alias;
                addSystemAddressFunction.Address = address;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(addSystemAddressFunction, cancellationToken);
        }

        public Task<string> AddressesQueryAsync(AddressesFunction addressesFunction, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<AddressesFunction, string>(addressesFunction, blockParameter);
        }

        
        public Task<string> AddressesQueryAsync(string returnValue1, BlockParameter blockParameter = null)
        {
            var addressesFunction = new AddressesFunction();
                addressesFunction.ReturnValue1 = returnValue1;
            
            return ContractHandler.QueryAsync<AddressesFunction, string>(addressesFunction, blockParameter);
        }

        public Task<string> AliasesQueryAsync(AliasesFunction aliasesFunction, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<AliasesFunction, string>(aliasesFunction, blockParameter);
        }

        
        public Task<string> AliasesQueryAsync(BigInteger returnValue1, BlockParameter blockParameter = null)
        {
            var aliasesFunction = new AliasesFunction();
                aliasesFunction.ReturnValue1 = returnValue1;
            
            return ContractHandler.QueryAsync<AliasesFunction, string>(aliasesFunction, blockParameter);
        }

        public Task<string> EnterRequestAsync(EnterFunction enterFunction)
        {
             return ContractHandler.SendRequestAsync(enterFunction);
        }

        public Task<TransactionReceipt> EnterRequestAndWaitForReceiptAsync(EnterFunction enterFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(enterFunction, cancellationToken);
        }

        public Task<string> EnterRequestAsync(string userToken)
        {
            var enterFunction = new EnterFunction();
                enterFunction.UserToken = userToken;
            
             return ContractHandler.SendRequestAsync(enterFunction);
        }

        public Task<TransactionReceipt> EnterRequestAndWaitForReceiptAsync(string userToken, CancellationTokenSource cancellationToken = null)
        {
            var enterFunction = new EnterFunction();
                enterFunction.UserToken = userToken;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(enterFunction, cancellationToken);
        }

        public Task<ulong> EntropyQueryAsync(EntropyFunction entropyFunction, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<EntropyFunction, ulong>(entropyFunction, blockParameter);
        }

        
        public Task<ulong> EntropyQueryAsync(BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<EntropyFunction, ulong>(null, blockParameter);
        }

        public Task<string> GetAddressBySoulQueryAsync(GetAddressBySoulFunction getAddressBySoulFunction, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<GetAddressBySoulFunction, string>(getAddressBySoulFunction, blockParameter);
        }

        
        public Task<string> GetAddressBySoulQueryAsync(ulong soul, BlockParameter blockParameter = null)
        {
            var getAddressBySoulFunction = new GetAddressBySoulFunction();
                getAddressBySoulFunction.Soul = soul;
            
            return ContractHandler.QueryAsync<GetAddressBySoulFunction, string>(getAddressBySoulFunction, blockParameter);
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

        public Task<string> GetUserRequestAsync(GetUserFunction getUserFunction)
        {
             return ContractHandler.SendRequestAsync(getUserFunction);
        }

        public Task<string> GetUserRequestAsync()
        {
             return ContractHandler.SendRequestAsync<GetUserFunction>();
        }

        public Task<TransactionReceipt> GetUserRequestAndWaitForReceiptAsync(GetUserFunction getUserFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(getUserFunction, cancellationToken);
        }

        public Task<TransactionReceipt> GetUserRequestAndWaitForReceiptAsync(CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync<GetUserFunction>(null, cancellationToken);
        }

        public Task<string> GetUserByAddressRequestAsync(GetUserByAddressFunction getUserByAddressFunction)
        {
             return ContractHandler.SendRequestAsync(getUserByAddressFunction);
        }

        public Task<TransactionReceipt> GetUserByAddressRequestAndWaitForReceiptAsync(GetUserByAddressFunction getUserByAddressFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(getUserByAddressFunction, cancellationToken);
        }

        public Task<string> GetUserByAddressRequestAsync(string who)
        {
            var getUserByAddressFunction = new GetUserByAddressFunction();
                getUserByAddressFunction.Who = who;
            
             return ContractHandler.SendRequestAsync(getUserByAddressFunction);
        }

        public Task<TransactionReceipt> GetUserByAddressRequestAndWaitForReceiptAsync(string who, CancellationTokenSource cancellationToken = null)
        {
            var getUserByAddressFunction = new GetUserByAddressFunction();
                getUserByAddressFunction.Who = who;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(getUserByAddressFunction, cancellationToken);
        }

        public Task<string> GetUserBySoulRequestAsync(GetUserBySoulFunction getUserBySoulFunction)
        {
             return ContractHandler.SendRequestAsync(getUserBySoulFunction);
        }

        public Task<TransactionReceipt> GetUserBySoulRequestAndWaitForReceiptAsync(GetUserBySoulFunction getUserBySoulFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(getUserBySoulFunction, cancellationToken);
        }

        public Task<string> GetUserBySoulRequestAsync(ulong soul)
        {
            var getUserBySoulFunction = new GetUserBySoulFunction();
                getUserBySoulFunction.Soul = soul;
            
             return ContractHandler.SendRequestAsync(getUserBySoulFunction);
        }

        public Task<TransactionReceipt> GetUserBySoulRequestAndWaitForReceiptAsync(ulong soul, CancellationTokenSource cancellationToken = null)
        {
            var getUserBySoulFunction = new GetUserBySoulFunction();
                getUserBySoulFunction.Soul = soul;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(getUserBySoulFunction, cancellationToken);
        }

        public Task<ulong> GetUserSoulQueryAsync(GetUserSoulFunction getUserSoulFunction, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<GetUserSoulFunction, ulong>(getUserSoulFunction, blockParameter);
        }

        
        public Task<ulong> GetUserSoulQueryAsync(BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<GetUserSoulFunction, ulong>(null, blockParameter);
        }

        public Task<string> GetUserTokenAddressQueryAsync(GetUserTokenAddressFunction getUserTokenAddressFunction, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<GetUserTokenAddressFunction, string>(getUserTokenAddressFunction, blockParameter);
        }

        
        public Task<string> GetUserTokenAddressQueryAsync(string wallet, BlockParameter blockParameter = null)
        {
            var getUserTokenAddressFunction = new GetUserTokenAddressFunction();
                getUserTokenAddressFunction.Wallet = wallet;
            
            return ContractHandler.QueryAsync<GetUserTokenAddressFunction, string>(getUserTokenAddressFunction, blockParameter);
        }

        public Task<BigInteger> GuaQueryAsync(GuaFunction guaFunction, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<GuaFunction, BigInteger>(guaFunction, blockParameter);
        }

        
        public Task<BigInteger> GuaQueryAsync(BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<GuaFunction, BigInteger>(null, blockParameter);
        }

        public Task<string> LuoRequestAsync(LuoFunction luoFunction)
        {
             return ContractHandler.SendRequestAsync(luoFunction);
        }

        public Task<string> LuoRequestAsync()
        {
             return ContractHandler.SendRequestAsync<LuoFunction>();
        }

        public Task<TransactionReceipt> LuoRequestAndWaitForReceiptAsync(LuoFunction luoFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(luoFunction, cancellationToken);
        }

        public Task<TransactionReceipt> LuoRequestAndWaitForReceiptAsync(CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync<LuoFunction>(null, cancellationToken);
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

        public Task<string> QuQueryAsync(QuFunction quFunction, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<QuFunction, string>(quFunction, blockParameter);
        }

        
        public Task<string> QuQueryAsync(BigInteger returnValue1, BlockParameter blockParameter = null)
        {
            var quFunction = new QuFunction();
                quFunction.ReturnValue1 = returnValue1;
            
            return ContractHandler.QueryAsync<QuFunction, string>(quFunction, blockParameter);
        }

        public Task<string> ReactRequestAsync(ReactFunction reactFunction)
        {
             return ContractHandler.SendRequestAsync(reactFunction);
        }

        public Task<TransactionReceipt> ReactRequestAndWaitForReceiptAsync(ReactFunction reactFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(reactFunction, cancellationToken);
        }

        public Task<string> ReactRequestAsync(ulong eta)
        {
            var reactFunction = new ReactFunction();
                reactFunction.Eta = eta;
            
             return ContractHandler.SendRequestAsync(reactFunction);
        }

        public Task<TransactionReceipt> ReactRequestAndWaitForReceiptAsync(ulong eta, CancellationTokenSource cancellationToken = null)
        {
            var reactFunction = new ReactFunction();
                reactFunction.Eta = eta;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(reactFunction, cancellationToken);
        }

        public Task<string> ReactUserRequestAsync(ReactUserFunction reactUserFunction)
        {
             return ContractHandler.SendRequestAsync(reactUserFunction);
        }

        public Task<TransactionReceipt> ReactUserRequestAndWaitForReceiptAsync(ReactUserFunction reactUserFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(reactUserFunction, cancellationToken);
        }

        public Task<string> ReactUserRequestAsync(ulong soul, ulong epsilon)
        {
            var reactUserFunction = new ReactUserFunction();
                reactUserFunction.Soul = soul;
                reactUserFunction.Epsilon = epsilon;
            
             return ContractHandler.SendRequestAsync(reactUserFunction);
        }

        public Task<TransactionReceipt> ReactUserRequestAndWaitForReceiptAsync(ulong soul, ulong epsilon, CancellationTokenSource cancellationToken = null)
        {
            var reactUserFunction = new ReactUserFunction();
                reactUserFunction.Soul = soul;
                reactUserFunction.Epsilon = epsilon;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(reactUserFunction, cancellationToken);
        }

        public Task<string> ReactorQueryAsync(ReactorFunction reactorFunction, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<ReactorFunction, string>(reactorFunction, blockParameter);
        }

        
        public Task<string> ReactorQueryAsync(BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<ReactorFunction, string>(null, blockParameter);
        }

        public Task<string> RecallRequestAsync(RecallFunction recallFunction)
        {
             return ContractHandler.SendRequestAsync(recallFunction);
        }

        public Task<TransactionReceipt> RecallRequestAndWaitForReceiptAsync(RecallFunction recallFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(recallFunction, cancellationToken);
        }

        public Task<string> RecallRequestAsync(User alpha)
        {
            var recallFunction = new RecallFunction();
                recallFunction.Alpha = alpha;
            
             return ContractHandler.SendRequestAsync(recallFunction);
        }

        public Task<TransactionReceipt> RecallRequestAndWaitForReceiptAsync(User alpha, CancellationTokenSource cancellationToken = null)
        {
            var recallFunction = new RecallFunction();
                recallFunction.Alpha = alpha;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(recallFunction, cancellationToken);
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

        public Task<string> VerifyUserTokenPermissionsRequestAsync(VerifyUserTokenPermissionsFunction verifyUserTokenPermissionsFunction)
        {
             return ContractHandler.SendRequestAsync(verifyUserTokenPermissionsFunction);
        }

        public Task<TransactionReceipt> VerifyUserTokenPermissionsRequestAndWaitForReceiptAsync(VerifyUserTokenPermissionsFunction verifyUserTokenPermissionsFunction, CancellationTokenSource cancellationToken = null)
        {
             return ContractHandler.SendRequestAndWaitForReceiptAsync(verifyUserTokenPermissionsFunction, cancellationToken);
        }

        public Task<string> VerifyUserTokenPermissionsRequestAsync(string userToken)
        {
            var verifyUserTokenPermissionsFunction = new VerifyUserTokenPermissionsFunction();
                verifyUserTokenPermissionsFunction.UserToken = userToken;
            
             return ContractHandler.SendRequestAsync(verifyUserTokenPermissionsFunction);
        }

        public Task<TransactionReceipt> VerifyUserTokenPermissionsRequestAndWaitForReceiptAsync(string userToken, CancellationTokenSource cancellationToken = null)
        {
            var verifyUserTokenPermissionsFunction = new VerifyUserTokenPermissionsFunction();
                verifyUserTokenPermissionsFunction.UserToken = userToken;
            
             return ContractHandler.SendRequestAndWaitForReceiptAsync(verifyUserTokenPermissionsFunction, cancellationToken);
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

        public Task<bool> HasQueryAsync(HasFunction hasFunction, BlockParameter blockParameter = null)
        {
            return ContractHandler.QueryAsync<HasFunction, bool>(hasFunction, blockParameter);
        }

        
        public Task<bool> HasQueryAsync(string contract, string what, BlockParameter blockParameter = null)
        {
            var hasFunction = new HasFunction();
                hasFunction.Contract = contract;
                hasFunction.What = what;
            
            return ContractHandler.QueryAsync<HasFunction, bool>(hasFunction, blockParameter);
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
                typeof(AddContractOwnerFunction),
                typeof(AddLibraryOwnerFunction),
                typeof(AddSystemAddressFunction),
                typeof(AddressesFunction),
                typeof(AliasesFunction),
                typeof(EnterFunction),
                typeof(EntropyFunction),
                typeof(GetAddressBySoulFunction),
                typeof(GetMarketRateFunction),
                typeof(GetUserFunction),
                typeof(GetUserByAddressFunction),
                typeof(GetUserBySoulFunction),
                typeof(GetUserSoulFunction),
                typeof(GetUserTokenAddressFunction),
                typeof(GuaFunction),
                typeof(LuoFunction),
                typeof(MotzkinPrimeFunction),
                typeof(OnFunction),
                typeof(PurchaseFunction),
                typeof(QuFunction),
                typeof(ReactFunction),
                typeof(ReactUserFunction),
                typeof(ReactorFunction),
                typeof(RecallFunction),
                typeof(RedeemFunction),
                typeof(RenameFunction),
                typeof(SaatFunction),
                typeof(TypeFunction),
                typeof(VerifyUserTokenPermissionsFunction),
                typeof(VoidFunction),
                typeof(XiaoFunction),
                typeof(AddOwnerFunction),
                typeof(AllowanceFunction),
                typeof(ApproveFunction),
                typeof(BalanceOfFunction),
                typeof(DecimalsFunction),
                typeof(HasFunction),
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
                typeof(InvalidOwnershipError),
                typeof(InvalidUserError),
                typeof(InvalidUserTokenError),
                typeof(MarketRateNotFoundError),
                typeof(OwnableInvalidOwnerError),
                typeof(OwnableUnauthorizedAccountError),
                typeof(SoulNotEnteredError),
                typeof(SystemAddressError),
                typeof(UserNotEnteredError)
            };
        }
    }
}
