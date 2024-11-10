using Dysnomia.Domain;
using Dysnomia.Domain.World;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Wallet;

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
            LocalWallet.SwitchAccount(0);
            Wallet.Contracts.Init(@"C:\Users\mariarahel\source\solc.exe",
                           @"C:\Users\mariarahel\source\repos\atropa_pulsechain\solidity");
            LocalContracts = new Wallet.Contracts(LocalWallet);
        }
    }
}
