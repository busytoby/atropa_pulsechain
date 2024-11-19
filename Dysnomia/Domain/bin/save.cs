using Dysnomia.Lib;
using Nethereum.Hex.HexTypes;
using Nethereum.Web3;
using Org.BouncyCastle.Utilities;
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
    internal class save : Command {
        new public static String Name = "cmd_Save";
        new public static String Description = "Save (alias)";

        protected override async void Phi() {
            if(Theta == null) throw new Exception("Null Command Theta");
            if(Args == null) throw new Exception("Null Command Args");
            byte[] From = Encoding.Default.GetBytes(Name);

            if(Aliases.Forward.Count == 0) return;

            uint no = 0;
            if(Args.Length == 0) {
                using(StreamWriter outputFile = new StreamWriter(Path.Combine(Wallet.Contracts.RootFolder, Wallet.Wallet._base))) {
                    string _alias = Wallet.Wallet._base;
                    string _cx = Aliases.Forward[Wallet.Wallet._base];
                    outputFile.WriteLine(Wallet.Wallet._base + 0x0 + Wallet.Wallet._base + 0x0 + _cx);
                    no++;
                    while(Aliases.Reverse.ContainsKey(_alias)) {
                        _cx = Aliases.Reverse[_alias];
                        outputFile.WriteLine(_cx + 0x0 + Wallet.Wallet._base + 0x0 + _alias);
                        no++;
                    }
                }
                Output(From, Encoding.Default.GetBytes("Saved " + no + " Aliases"), 6);
                return;
            }
            if(Args[Args.Length - 1].Length == 0) Args = Args.Take(Args.Length - 1).ToArray();

            for(int i = 0; i < Args.Length; i++) {
                string _cx = Controller.LocalContracts.ResolveAlias(Args[i]);
                _ = await Controller.LocalContracts.ExecuteWithAliases(Wallet.Wallet._base, "Set", (string)Args[i], _cx);
                no++;
            }
            Output(From, Encoding.Default.GetBytes("Saved " + no + " Aliases"), 6);
        }
    }
}
