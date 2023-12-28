using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using static Dysnomia.Domain.Tare;

namespace Dysnomia.Domain.World
{
    static public class Logging
    {
        static private Tare Tau;

        static Logging()
        {
            Tau = new Tare();
        }

        static public void Add(Gram G)
        {
            Tau.Add(G);
        }

        static public void Log(byte[] A)
        {
            foreach (Gram G in Tau) G(A);
        }
    }
}
