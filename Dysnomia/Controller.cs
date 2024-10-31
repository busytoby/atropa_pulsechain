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
        static public Fi Fi;
        static public Wallet.Wallet LocalWallet;
        static public Contracts LocalContracts;
        //static public Serialization Serialization;

        static Controller()
        {
            Oracle = new Oracle();
            Fi = new Fi();
            Fi.Rho.Subscribers.Add(Oracle.Fi);
            LocalWallet = new Wallet.Wallet("http://127.0.0.1:8545/");
            LocalWallet.SwitchAccount(0);
            Contracts.Init(@"C:\Users\mariarahel\source\solc.exe",
                           @"C:\Users\mariarahel\source\repos\atropa_pulsechain\solidity");
            LocalContracts = new Contracts(LocalWallet);
            //Serialization = new Serialization();
        }
    }
}
