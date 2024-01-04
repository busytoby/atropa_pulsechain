#pragma warning disable CS8981

using Dysnomia.Domain.World;
using System;
using System.Collections.Generic;
using System.ComponentModel.Design;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Dysnomia.Domain.bin
{
    internal class beta : Command
    {
        new public static String Name = "cmd_Beta";
        new public static String Description = "Manually Sets The Block Encryption To State Seeded By Any CMD Arguments";

        protected override void Phi()
        {
            if (Theta == null) throw new Exception("Null Command Theta");
            if (Args == null) throw new Exception("Null Command Args");
            byte[] From = Encoding.Default.GetBytes(Name);
            Theta.In.Enqueue(new Tare.MSG(From, new byte[] { 0x04 }, 6));
            String Eta = string.Join(" ", Args);
            if (Eta.Length == 0)
                Theta.Out.Enqueue(new Tare.MSG(From, Encoding.Default.GetBytes("Beta Command Requires At Least 1 Argument"), 6));
            else
                Theta.In.Enqueue(new Tare.MSG(From, Encoding.Default.GetBytes(Eta), 6));
        }
    }
}
