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
        new public static String Description = "Deploy [account] [[path/]contract] [options...]";

        protected override async void Phi()
        {
            if (Theta == null) throw new Exception("Null Command Theta");
            if (Args == null) throw new Exception("Null Command Args");
            byte[] From = Encoding.Default.GetBytes(Name);

            int _arg = 0;
            bool account = int.TryParse(Args[0], out int _accountnumber);
            if(account) {
                _arg++;
                Controller.LocalWallet.SwitchAccount(_accountnumber);
                Output(From, Encoding.Default.GetBytes("Deploying From " + Controller.LocalWallet.Account.Address), 6);
            }

            Task<string> _t = Controller.LocalContracts.Deploy(Output, (string)Args[_arg], Args.Skip(_arg + 1).ToArray());
            string Address = _t.Result;
            Output(From, Encoding.Default.GetBytes("Deployment Address: " + Address), 6);
        }
    }
}
