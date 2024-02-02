#pragma warning disable CS8981

using Dysnomia.Domain.World;
using Dysnomia.Lib;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Text;
using System.Threading.Tasks;

namespace Dysnomia.Domain.bin
{
    internal class select : Command
    {
        new public static String Name = "cmd_Select";
        new public static String Description = "Select A Connection To Activate Or List Open Connections";

        protected override void Phi()
        {
            if (Theta == null) throw new Exception("Null Command Theta");
            if (Args == null) throw new Exception("Null Command Args");
            byte[] From = Encoding.Default.GetBytes(Name);

            if (Args.Length == 0)
            {
                if(Controller.Fi.Psi.Count == 0) Output(From, Encoding.Default.GetBytes("No Open Connections"), 6);
                foreach (Greed G in Controller.Fi.Psi.Values)
                {
                    if (G.Cone == false)
                        Output(From, Encoding.Default.GetBytes(String.Format("{0} :: {1}[{2}]", G.ClientId.ToString(), G.Host, G.Port)), 6);
                }
                return;
            }

            
            /*
            byte[] From = Encoding.Default.GetBytes(Name);
            Input(From, new byte[] { 0x03 }, 6);
            Input(From, new byte[] { 0x00 }, 6);
            */
        }
    }
}
