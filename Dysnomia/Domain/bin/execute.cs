using Dysnomia.Lib;
using Nethereum.Hex.HexTypes;
using Nethereum.Web3;
using System;
using System.Collections.Generic;
using System.ComponentModel.Design;
using System.Dynamic;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using Wallet;

namespace Dysnomia.Domain.bin {
    internal class execute : Command {
        new public static String Name = "cmd_Execute";
        new public static String Description = "Execute [account] [contract or alias] [function]  [options...]";

        protected override async void Phi() {
            if(Theta == null) throw new Exception("Null Command Theta");
            if(Args == null) throw new Exception("Null Command Args");
            byte[] From = Encoding.Default.GetBytes(Name);

            if(Args.Length == 0) {
                Output(From, Encoding.Default.GetBytes("Aliases " + Alias + " = " + Controller.LocalContracts.ResolveAlias(Alias)), 6);
                return;
            }
            if(Args[Args.Length - 1].Length == 0) Args = Args.Take(Args.Length - 1).ToArray();

            int _arg = 0;
            bool account = int.TryParse(Args[0], out int _accountnumber);
            bool account2 = false;
            if(!account) account2 = int.TryParse(Alias, out _accountnumber);
            if(account || account2) {
                Alias = Args[_arg++];
                Controller.LocalWallet.SwitchAccount(_accountnumber);
                Output(From, Encoding.Default.GetBytes("Executing From " + Controller.LocalWallet.Account.Address), 6);
            }

            try {
                Task<dynamic> _exe;
                if(Alias == "execute") _exe = Controller.LocalContracts.ExecuteWithAliases(Output, (string)Args[_arg], (string)Args.Skip(_arg + 1).Take(1).ToArray()[0], Args.Skip(_arg + 2).ToArray());
                else _exe = Controller.LocalContracts.ExecuteWithAliases(Output, Alias, (string)Args[_arg], Args.Skip(_arg + 1).ToArray());
                _exe.Wait();
                dynamic _result = _exe.Result;
                if(_result != null) Output(From, Encoding.Default.GetBytes(Alias + " " + string.Join(" ", Args.Skip(_arg).ToArray()) + ": " + _result), 6);
                else Output(From, Encoding.Default.GetBytes(Alias + " " + string.Join(" ", Args.Skip(_arg).ToArray())), 6);
            } catch { }
        }
    }
}
