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
    internal class compile : Command
    {
        new public static String Name = "cmd_Compile";
        new public static String Description = "Compile [path/[contract]]";

        protected override async void Phi()
        {
            if (Theta == null) throw new Exception("Null Command Theta");
            if (Args == null) throw new Exception("Null Command Args");
            byte[] From = Encoding.Default.GetBytes(Name);

            if (Args.Length == 1)
            {
                Output(From, Encoding.Default.GetBytes("Not Yet Supported"), 6);
            }
            else
            {
                Contracts.Compile();
                Output(From, Encoding.Default.GetBytes("Done, See: " + Contracts.OutputFolder), 6);
            }
        }
    }
}
