using Dysnomia.Lib;
using Nethereum.Hex.HexTypes;
using Nethereum.Web3;
using System;
using System.Collections.Generic;
using System.ComponentModel.Design;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Dysnomia.Domain.bin
{
    internal class balance : Command
    {
        new public static String Name = "cmd_Balance";
        new public static String Description = "Balance (account) [(token)], Get Account Balance [for token]";

        protected override async void Phi()
        {
            if (Theta == null) throw new Exception("Null Command Theta");
            if (Args == null) throw new Exception("Null Command Args");
            byte[] From = Encoding.Default.GetBytes(Name);

            if (Args.Length == 1)
            {
                HexBigInteger Balance = Controller.LocalWallet.EthGetBalance(Args[0]);
                decimal ETHBalance = Web3.Convert.FromWei(Balance);
                Output(From, Encoding.Default.GetBytes("Balance Test: " + Balance), 6);
            } else if (Args.Length == 2)
            {
                Output(From, Encoding.Default.GetBytes("Not Yet Supported"), 6);
            }
            else
            {
                Output(From, Encoding.Default.GetBytes("Balance Only Accepts Up To 2 Arguments"), 6);
            }
        }
    }
}
