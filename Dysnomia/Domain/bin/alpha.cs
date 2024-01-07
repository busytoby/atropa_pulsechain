#pragma warning disable CS8981

using Dysnomia.Lib;
using System;
using System.Collections.Generic;
using System.ComponentModel.Design;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Dysnomia.Domain.bin
{
    internal class alpha : Command
    {
        new public static String Name = "cmd_Alpha";
        new public static String Description = "Manually Sets The System Encryption To State Seeded By Any CMD Arguments";

        protected override void Phi()
        {
            if (Theta == null) throw new Exception("Null Command Theta");
            if (Args == null) throw new Exception("Null Command Args");
            byte[] From = Encoding.Default.GetBytes(Name);
            Theta.In.Enqueue(new Tare.MSG(From, new byte[] { 0x03 }, 6));
            String Eta = string.Join(" ", Args);
            if (Eta.Length == 0)
                Theta.In.Enqueue(new Tare.MSG(From, new byte[] { 0x00 }, 6));
            else
                Theta.In.Enqueue(new Tare.MSG(From, Encoding.Default.GetBytes(Eta), 6));
        }
    }
}
