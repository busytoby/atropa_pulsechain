﻿#pragma warning disable CS8981

using Dysnomia.Lib;
using System;
using System.Collections.Generic;
using System.ComponentModel.Design;
using System.Linq;
using System.Reflection;
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
            if (Theta == null) throw new Exception("Null Command Theta");
            if (Args == null) throw new Exception("Null Command Args");
            byte[] From = Encoding.Default.GetBytes(Name);

            if (Args.Length == 0)
            {
                Output(From, Encoding.Default.GetBytes("Help Commands Available:"), 6);
                String CmdList = "";
                foreach (Type T in GetCommands())
                    CmdList += T.Name + " ";
                Output(From, Encoding.Default.GetBytes(CmdList), 6);
            } else
            {
                foreach(String A in Args)
                {
                    bool found = false;
                    foreach (Type T in GetCommands())
                        if (A == T.Name)
                        {
                            FieldInfo? DF = T.GetField("Description");
                            if (DF == null) throw new Exception("No Description");
                            object? DFV = DF.GetValue(null);
                            if (DFV == null) throw new Exception("Null Description");
                            Output(From, Encoding.Default.GetBytes(A + ": " + DFV.ToString()), 6);
                            found = true;
                            break;
                        }
                    if(!found)
                        Output(From, Encoding.Default.GetBytes("Help For " + A + " Not Found"), 6);
                }
            }
        }
    }
}
