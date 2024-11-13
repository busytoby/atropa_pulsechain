using Dysnomia.Lib;
using Nethereum.Hex.HexTypes;
using Nethereum.Web3;
using System;
using System.Collections.Generic;
using System.ComponentModel.Design;
using System.Dynamic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Wallet;

namespace Dysnomia.Domain.bin
{
    internal class deploy : Command
    {
        new public static String Name = "cmd_Deploy";
        new public static String Description = "Deploy [name] [account] [[path/]contract] [options...]";

        protected override async void Phi()
        {
            if (Theta == null) throw new Exception("Null Command Theta");
            if (Args == null) throw new Exception("Null Command Args");
            byte[] From = Encoding.Default.GetBytes(Name);

            /*
            ExpandoObject _callargs = new ExpandoObject();
            for(int i = 1; i < Args.Length; i++) {
                _callargs.Prepend()

            }*/
            if(Args.Length >= 2) {
                Controller.LocalWallet.SwitchAccount(int.Parse(Args[1]));
                Output(From, Encoding.Default.GetBytes("Deploying From " + Controller.LocalWallet.Account.Address), 6);
            }

            if (Args.Length >= 3)
            {
                string Address = Controller.LocalContracts.Deploy(Args[0], Args[2], Args.Skip(3).ToArray());
                Output(From, Encoding.Default.GetBytes("Deployed " + Args[2] + " Address " + Address), 6);
            }
            else
            {
                Task Install = Controller.LocalContracts.Install(Output);
                Install.Wait();
            }
        }
    }
}
