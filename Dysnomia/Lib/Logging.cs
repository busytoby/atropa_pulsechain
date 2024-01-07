using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Dysnomia.Domain;
using static Dysnomia.Domain.Tare;

namespace Dysnomia.Lib
{
    static public class Logging
    {
        /* for now
         * 1 - data
         * 2 - crypt
         * 3 - operational
         * 4 - opcode
         * 5 - function
         * 6 - msg
         * 7 - error
         */

        static private Tare Tau;
        static private object Theta;

        static Logging()
        {
            Tau = new Tare();
            Theta = new object();
        }

        static public void Add(Gram G)
        {
            Tau.Add(G);
        }

        static public void Log(MSG A)
        {
            new Task(() => { lock (Theta) foreach (Gram G in Tau) G(A); }).Start();
        }

        static public void Log(string From, string Data, short Priority = 6)
        {
            byte[] A = Encoding.Default.GetBytes(From);
            byte[] B = Encoding.Default.GetBytes(Data);
            MSG C = new MSG(A, B, Priority);
            new Task(() => { lock (Theta) foreach (Gram G in Tau) G(C); }).Start();
        }
    }
}
