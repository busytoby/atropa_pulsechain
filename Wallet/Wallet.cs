
using System.Configuration;
using System.Numerics;
using Nethereum.Web3;
using Nethereum.Contracts.Services;
using Nethereum.Hex.HexTypes;
using Nethereum.RPC.Eth;
using Nethereum.Web3.Accounts;
using Nethereum.Contracts;
using Nethereum.RPC.Eth.DTOs;
using System.Reflection.Metadata;

namespace Wallet
{
    public class Wallet
    {
        static public string _base = "þ";
        public string ConnectionString;
        public Web3 w3;
        public IEthApiContractService eth;
        public Account Account;
        internal OracleProcessString ProcessString;
        public delegate dynamic OracleProcessString(String A);

        public Wallet(string connectionString)
        {
            ConnectionString = connectionString;
            Account = new Account(Accounts.pkeys[0]);
            w3 = new Web3(ConnectionString);
            eth = w3.Eth;

            Task<HexBigInteger> _b = w3.Eth.Blocks.GetBlockNumber.SendRequestAsync();
            _b.Wait();
            HexBigInteger latestBlock = new HexBigInteger(_b.Result.ToUlong() + 1);
            Event<Events.TransferEvent> TransferEvent = w3.Eth.GetEvent<Events.TransferEvent>();
            NewFilterInput _n = TransferEvent.CreateFilterInput();
            _n.FromBlock = new BlockParameter(latestBlock);
            Contracts.Logs.Add(new wEvent(TransferEvent, _n, "TransferEvent"));
        }

        public void SetOracleProcessString(OracleProcessString _o) {
            ProcessString = _o;
        }

        public void SwitchAccount(int Number)
        {
            Account = new Account(Accounts.pkeys[Number]);
            w3 = new Web3(Account, ConnectionString);
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
