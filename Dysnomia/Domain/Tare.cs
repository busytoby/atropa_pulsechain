using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Dysnomia.Domain
{
    static public class Tare
    {
        public delegate void Gram(byte[] A);
        static public List<Gram> Grams;

        static Tare()
        {
            Grams = new List<Gram>();
        }

        static public void AddCustom(Gram A)
        {
            Grams.Add(A);
        }

        static public void Log(byte[] A)
        {
            foreach (Gram G in Grams) G(A);
        }
    }
}
