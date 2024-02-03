using Dysnomia.Domain.World;
using Dysnomia.Lib;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Dysnomia.Domain.bin
{
    internal class esay : Command
    {
        new public static String Name = "cmd_Esay";
        new public static String Description = "Send An Encrypted Chat Message To The Selected Server";

        protected override void Phi()
        {
            if (Theta == null) throw new Exception("Null Command Theta");
            if (Args == null) throw new Exception("Null Command Args");
            byte[] From = Encoding.Default.GetBytes(Name);

            if (Controller.Fi.Nu.IsZero)
            {
                Output(From, Encoding.Default.GetBytes("No Connected Server Selected"), 6);
            }
            else
            {
                Greed Server = Controller.Fi.Psi[Controller.Fi.Nu];
                Server.Input("COMMAND", "ESAY", new byte[] { 0x13 }, 1);
                Server.Input("COMMAND", "ESAY Message", Encoding.Default.GetBytes(String.Join(" ", Args)), 1);
            }
        }
    }
}
