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
                if (Controller.Fi.Psi.Count == 0) Output(From, Encoding.Default.GetBytes("No Open Connections"), 6);
                foreach (Greed G in Controller.Fi.Psi.Values)
                {
                    if (G.Cone == false)
                    {
                        Output(From, Encoding.Default.GetBytes(String.Format("{0} :: {1}[{2}]", G.ClientId.ToString(), G.Host, G.Port)), 6);
                    }
                }
            }
            else
            {
                BigInteger _sel = BigInteger.Parse(Args[0]);
                if (Controller.Fi.Psi.ContainsKey(_sel)) Controller.Fi.Nu = _sel;
                else Output(From, Encoding.Default.GetBytes(String.Format("ClientId Not Found: ", _sel)), 6);
            }

            if (!Controller.Fi.Nu.IsZero)
            {
                Output(From, Encoding.Default.GetBytes(String.Format("Active: {0}", Controller.Fi.Nu.ToString())), 6);
                Conjunction Q = new Conjunction();
                Q.Enqueue(Controller.Fi.Nu.ToByteArray());
                Q.Enqueue(new byte[] { 0x00 });
                Conjunction R = Query(Q).GetAwaiter().GetResult();
            }
        }

        protected static async Task<Conjunction> Query(Conjunction Q)
        {
            return await Task.Run(() =>
            {
                BigInteger Epsilon = Q.Next();
                Controller.Fi.Psi[Epsilon].Handshake("Query", 0x14);
                Controller.Fi.Psi[Epsilon].Handshake("Query", Q.NextBytes());
                Conjunction R = new Conjunction();
                R.Enqueue(new byte[] { 0x01, 0x02, 0x00 });
                return R;
            });
        }
    }
}
