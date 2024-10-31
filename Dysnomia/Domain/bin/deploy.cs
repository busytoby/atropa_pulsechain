using Dysnomia.Lib;
using Nethereum.Hex.HexTypes;
using Nethereum.Web3;
using System;
using System.Collections.Generic;
using System.ComponentModel.Design;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Wallet;

namespace Dysnomia.Domain.bin
{
    internal class deploy : Command
    {
        new public static String Name = "cmd_Deploy";
        new public static String Description = "Deploy [path/]contract [Account]";

        protected override async void Phi()
        {
            if (Theta == null) throw new Exception("Null Command Theta");
            if (Args == null) throw new Exception("Null Command Args");
            byte[] From = Encoding.Default.GetBytes(Name);

            if(Args.Length == 2) {
                Controller.LocalWallet.SwitchAccount(int.Parse(Args[1]));
                Output(From, Encoding.Default.GetBytes("Deploying From " + Controller.LocalWallet.Account.Address), 6);
            }

            if (Args.Length == 1 || Args.Length == 2)
            {
                string Address = Controller.LocalContracts.Deploy(Args[0]);
                Output(From, Encoding.Default.GetBytes("Deployed " + Args[0] + " Address " + Address), 6);
            }
            else
            {
                Output(From, Encoding.Default.GetBytes("No File Provided"), 6);
            }
        }
    }
}
