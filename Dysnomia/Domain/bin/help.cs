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
    internal class help : Command
    {
        new public static String Name = "cmd_Help";
        new public static String Description = "Prints The List Of Available Commands Or Provides Information On A Specific Command With help [cmd]";

        protected override void Phi()
        {
            byte[] From = Encoding.Default.GetBytes(Name);

            if (Args.Length == 0)
            {
                Theta.Out.Enqueue(new Tare.MSG(From, Encoding.Default.GetBytes("Help Commands Available:"), 6));
                String CmdList = "";
                foreach (Type T in GetCommands())
                    CmdList += T.Name + " ";
                Theta.Out.Enqueue(new Tare.MSG(From, Encoding.Default.GetBytes(CmdList), 6));
            } else
            {
                foreach(String A in Args)
                {
                    bool found = false;
                    foreach (Type T in GetCommands())
                        if (A == T.Name)
                        {
                            Theta.Out.Enqueue(new Tare.MSG(From, Encoding.Default.GetBytes(A + ": " + T.GetField("Description").GetValue(null).ToString()), 6));
                            found = true;
                            break;
                        }
                    if(!found)
                        Theta.Out.Enqueue(new Tare.MSG(From, Encoding.Default.GetBytes("Help For " + A + " Not Found"), 6));
                }
            }
        }
    }
}
