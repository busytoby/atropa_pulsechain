
using System.Configuration;
using System.Numerics;
using Nethereum.Web3;
using Nethereum.Contracts.Services;
using Nethereum.Hex.HexTypes;
using Nethereum.RPC.Eth;

namespace Wallet
{
    public class LocalWallet
    {
        public string ConnectionString;
        public Web3 w3;
        public IEthApiContractService eth;

        public LocalWallet(string connectionString)
        {
            ConnectionString = connectionString;
            w3 = new Web3(ConnectionString);
            eth = w3.Eth;
        }

        public HexBigInteger EthGetBalance(string Address)
        {
            Task<HexBigInteger> _t = eth.GetBalance.SendRequestAsync(Address);
            _t.Wait();
            return _t.Result;
        }
    }

}
