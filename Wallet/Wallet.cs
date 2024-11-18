﻿
using System.Configuration;
using System.Numerics;
using Nethereum.Web3;
using Nethereum.Contracts.Services;
using Nethereum.Hex.HexTypes;
using Nethereum.RPC.Eth;
using Nethereum.Web3.Accounts;

namespace Wallet
{
    public class Wallet
    {
        public string ConnectionString;
        public Web3 w3;
        public IEthApiContractService eth;
        public Account Account;
        internal OracleProcessString ProcessString;
        public delegate void OracleProcessString(String A);

        public Wallet(string connectionString)
        {
            ConnectionString = connectionString;
            Account = new Account(Accounts.pkeys[0]);
            w3 = new Web3(ConnectionString);
            eth = w3.Eth;
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