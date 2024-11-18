#pragma warning disable CS8981

using Dysnomia.Lib;
using Nethereum.ABI.Model;
using Nethereum.Contracts;
using System;
using System.Collections.Generic;
using System.ComponentModel.Design;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Input;

namespace Dysnomia.Domain.bin {
    internal class alias : Command {
        new public static String Name = "cmd_Alias";
        new public static String Description = "List All Aliases, or [alias] or set [alias] [value]";

        protected override void Phi() {
            if(Theta == null) throw new Exception("Null Command Theta");
            if(Args == null) throw new Exception("Null Command Args");
            byte[] From = Encoding.Default.GetBytes(Name);

            if(Args.Length == 0) {
                Output(From, Encoding.Default.GetBytes("Aliases:"), 6);
                foreach(string k in Wallet.Aliases.Forward.Keys)
                    Output(From, Encoding.Default.GetBytes(k + ": " + Wallet.Aliases.Forward[k]), 6);
            } else if(Args.Length == 1) {
                string _alias = Controller.LocalContracts.ResolveAlias(Args[0]);
                if(_alias != Args[0])
                    Output(From, Encoding.Default.GetBytes("Aliases " + Args[0] + " = " + Controller.LocalContracts.ResolveAlias(Args[0])), 6);
                else
                    Output(From, Encoding.Default.GetBytes("Unknown Alias " + Args[0]), 6);
            } else {
                Wallet.Aliases.Forward[Args[0]] = string.Join(" ", Args.Skip(1).ToArray());
                Output(From, Encoding.Default.GetBytes("Aliases " + Args[0] + " = " + Controller.LocalContracts.ResolveAlias(Args[0])), 6);
            }
        }
    }
}
