﻿using Dysnomia.Domain;
using Dysnomia.Domain.World;
using Nethereum.Web3.Accounts;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Wallet;
using static System.Runtime.InteropServices.JavaScript.JSType;

namespace Dysnomia
{
    static public class Controller
    {
        static public Oracle Oracle;
        static public Wallet.Wallet LocalWallet;
        static public Wallet.Contracts LocalContracts;

        static Controller()
        {
            Oracle = new Oracle();
            LocalWallet = new Wallet.Wallet("http://127.0.0.1:8545/");
            //LocalWallet = new Wallet.Wallet("http://dysnomia.airforce:8545/");
            LocalWallet.SetOracleProcessString(Oracle.ProcessStringAndWait);

            //Wallet.Wallet Wallet2 = new Wallet.Wallet("http://dysnomia.airforce:8545/");
            LocalWallet.SwitchAccount(0);
            Nethereum.Hex.HexTypes.HexBigInteger Test = LocalWallet.EthGetBalance("0x2358B9d8111a0544C79A2Ba53436078eF887c547");
            Test = LocalWallet.EthGetBalance("0x70997970C51812dc3A010C7d01b50e0d17dc79C8");
            Test = LocalWallet.EthGetBalance("0x7a20189B297343CF26d8548764b04891f37F3414");
            //LocalWallet.SwitchAccount(0);
            //Test = Wallet2.EthGetBalance("0x2358B9d8111a0544C79A2Ba53436078eF887c547");
            Wallet.Contracts.Init(@"C:\Users\mariarahel\source\solc.exe",
                           @"C:\Users\mariarahel\source\repos\atropa_pulsechain");
            LocalContracts = new Wallet.Contracts(LocalWallet);
            Oracle.Enqueue(new byte[] { 0x00 });
            Oracle.Enqueue(new byte[] { 0x00 });
        }
    }
}
