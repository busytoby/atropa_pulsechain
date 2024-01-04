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
    internal class connect : Command
    {
        new public static String Name = "cmd_Connect";
        new public static String Description = "Initiate A New Connection connect [Host] [Port]";

        protected override void Phi()
        {
            byte[] From = Encoding.Default.GetBytes(Name);

            if (Args.Length == 0)
                Theta.Out.Enqueue(new Tare.MSG(From, Encoding.Default.GetBytes("Connect Command Requires At Least 1 Argument"), 6));
            else
            {
                Theta.In.Enqueue(new Tare.MSG(From, new byte[] { 0x05 }, 6));
                Theta.In.Enqueue(new Tare.MSG(From, Encoding.Default.GetBytes(Args[0]), 6));
                if(Args.Length < 2) Theta.In.Enqueue(new Tare.MSG(From, BitConverter.GetBytes(Int16.Parse("5555")), 6)); // 0x15B3 = 5555
                else Theta.In.Enqueue(new Tare.MSG(From, BitConverter.GetBytes(Int16.Parse(Args[1])), 6));
            }
        }
    }
}
