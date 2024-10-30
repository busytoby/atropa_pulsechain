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
                        Output(From, Encoding.Default.GetBytes(String.Format("{0} :: {1}[{2}]", G.ClientId, G.Host, G.Port)), 6);
                    }
                }
            }
            else
            {
                if(!TryParse(Args[0]))
                    Output(From, Encoding.Default.GetBytes(String.Format("ClientId Not Found: {0}", Args[0])), 6);
            }

            byte[] To;

            Controller.Fi.Nu.TryPeek(out To);
            BigInteger Beta = new BigInteger(To);

            if (!Beta.IsZero)
            {
                Output(From, Encoding.Default.GetBytes(String.Format("Active: {0} :: {1}[{2}]", Beta, Controller.Fi.Psi[Beta].Host, Controller.Fi.Psi[Beta].Port)), 6);
                byte[] Nu;
                Controller.Fi.Nu.TryPeek(out Nu);
                BigInteger Epsilon = new BigInteger(Nu);
                if (!Controller.Fi.Psi.ContainsKey(Epsilon)) throw new Exception("Invalid Query Host");
                Controller.Fi.Psi[Epsilon].Handshake("Query", 0x14, Controller.Fi.Psi[Epsilon].Rho[0]);
                Controller.Fi.Psi[Epsilon].Handshake("Query", Controller.Fi.Nu.Serialize(1), Controller.Fi.Psi[Epsilon].Rho[Controller.Fi.Psi[Epsilon].Rho.ProxyId]);

                while (Controller.Fi.Psi[Epsilon].Rho[Controller.Fi.Psi[Epsilon].Rho.ProxyId].Sigma.Count < 2) Thread.Sleep(200);
                while (Controller.Fi.Psi[Epsilon].Rho[Controller.Fi.Psi[Epsilon].Rho.ProxyId].Sigma.Count > 0)
                {
                    byte[] OpCode = Controller.Fi.Psi[Epsilon].Rho[Controller.Fi.Psi[Epsilon].Rho.ProxyId].Sigma.NextBytes();
                    BigInteger SubId = Controller.Fi.Psi[Epsilon].Rho[Controller.Fi.Psi[Epsilon].Rho.ProxyId].Sigma.Next();
#if !DEBUG
                    if (Epsilon == SubId) continue;
#endif
                    if (SubId != 0)
                        Output(From, Encoding.Default.GetBytes(String.Format("Available Path: {0}.{1} :: {2}[{3}]", Epsilon, SubId, Controller.Fi.Psi[Epsilon].Host, Controller.Fi.Psi[Epsilon].Port)), 6);
                }
            }
        }

        public bool TryParse(String Dotted)
        {
            try
            {
                Conjunction Eta = new Conjunction();
                foreach (String Beta in Dotted.Split(".", StringSplitOptions.RemoveEmptyEntries))
                {
                    BigInteger _sel;
                    BigInteger.TryParse(Beta, out _sel);
                    if (Controller.Fi.Psi.ContainsKey(_sel))
                        Eta.Enqueue(_sel.ToByteArray());
                    else
                        return false;
                }

                Controller.Fi.Nu.Clear();
                while (Eta.Count > 0)
                    Controller.Fi.Nu.Enqueue(Eta.NextBytes());

                return true;
            }
            catch { }
            return false;
        }
    }
}
