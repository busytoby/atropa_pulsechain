#pragma warning disable CS8602
#pragma warning disable CS8603
#pragma warning disable CS8981

using Dysnomia.Domain.World;
using Dysnomia.Lib;
using ExtensionMethods;
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
                if (Controller.Fi.Psi.Count == 0) Output(From, Encoding.Default.GetBytes("No Open Connections"), 6);
                foreach (Greed G in Controller.Fi.Psi.Values)
                {
                    if (G.Cone == false)
                    {
                        Output(From, Encoding.Default.GetBytes(String.Format("{0} :: {1}[{2}]", G.ClientId.B64(), G.Host, G.Port)), 6);
                    }
                }
            }
            else
            {
                BigInteger _sel = new BigInteger(Convert.FromBase64String(Args[0]));
                if (Controller.Fi.Psi.ContainsKey(_sel))
                {
                    Controller.Fi.Nu.Clear();
                    Controller.Fi.Nu.Enqueue(_sel.ToByteArray());
                }
                else
                    Output(From, Encoding.Default.GetBytes(String.Format("ClientId Not Found: {0}", _sel.B64())), 6);
            }

            byte[] To;

            Controller.Fi.Nu.TryPeek(out To);
            BigInteger Beta = new BigInteger(To);

            if (!Beta.IsZero)
            {
                Output(From, Encoding.Default.GetBytes(String.Format("Active: {0}", Beta.B64())), 6);
                byte[] Nu;
                Controller.Fi.Nu.TryPeek(out Nu);
                BigInteger Epsilon = new BigInteger(Nu);
                if (!Controller.Fi.Psi.ContainsKey(Epsilon)) throw new Exception("Invalid Query Host");
                Controller.Fi.Psi[Epsilon].Handshake("Query", 0x14);
                Controller.Fi.Psi[Epsilon].Handshake("Query", Controller.Fi.Nu.Serialize(1));

                while (Controller.Fi.Psi[Epsilon].Sigma.Count < 2) Thread.Sleep(200);
                while (Controller.Fi.Psi[Epsilon].Sigma.Count > 0)
                {
                    byte[] OpCode = Controller.Fi.Psi[Epsilon].Sigma.NextBytes();
                    BigInteger SubId = Controller.Fi.Psi[Epsilon].Sigma.Next();
#if !DEBUG
                    if (Epsilon == SubId) continue;
#endif
                    if (SubId != 0)
                        Output(From, Encoding.Default.GetBytes(String.Format("Path: {0}{1}", Epsilon.B64(), SubId.B64())), 6);
                }
            }
        }
    }
}
