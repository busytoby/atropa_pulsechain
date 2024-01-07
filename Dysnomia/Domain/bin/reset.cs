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
    internal class reset : Command
    {
        new public static String Name = "cmd_Reset";
        new public static String Description = "Resets The System Encryption To Original State";

        protected override void Phi()
        {
            if (Theta == null) throw new Exception("Null Command Theta");
            byte[] From = Encoding.Default.GetBytes(Name);
            Theta.In.Enqueue(new Tare.MSG(From, new byte[] { 0x03 }, 6));
            Theta.In.Enqueue(new Tare.MSG(From, new byte[] { 0x00 }, 6));
        }
    }
}
