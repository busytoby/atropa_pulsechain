using Dysnomia.Domain.World;
using System;
using System.Collections.Generic;
using System.ComponentModel.Design;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Dysnomia.Domain.bin
{
    internal class help : Command
    {
        protected override void Phi()
        {
            byte[] From = Encoding.Default.GetBytes("cmd_Help");

            if (Args.Length == 0)
            {
                Theta.Out.Enqueue(new Tare.MSG(From, Encoding.Default.GetBytes("Help Commands Available:"), 6));
                String CmdList = "";
                foreach (Type T in GetCommands())
                    CmdList += T.Name + " ";
                Theta.Out.Enqueue(new Tare.MSG(From, Encoding.Default.GetBytes(CmdList), 6));
            } else
            {
                throw new Exception("Not Implemented");
            }
        }
    }
}
