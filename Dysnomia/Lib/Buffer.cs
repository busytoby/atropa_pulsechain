using Dysnomia.Domain.bin;
using ExtensionMethods;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Text;
using System.Threading.Tasks;

namespace Dysnomia.Lib
{
    public class Buffer : Conjunction
    {
        public byte[]? Bytes;

        public Buffer(Conjunction Beta)
        {
            if (Beta.Count < 5) throw new Exception("Conjunction Queue Count < 5");
            BigInteger Rho, Upsilon, Ohm, Xi, Alpha;
            Rho = Beta.Next();
            Upsilon = Beta.Next();
            Ohm = Beta.Next();
            Xi = Beta.Next();
            Alpha = Beta.Next();

            Enqueue(Rho.ToByteArray());
            Enqueue(Upsilon.ToByteArray());
            Enqueue(Ohm.ToByteArray());
            Enqueue(Xi.ToByteArray());
        }

        public Buffer(BigInteger Rho, BigInteger Upsilon, BigInteger Ohm, BigInteger Xi, BigInteger Alpha)
        {
            Enqueue(Rho.ToByteArray());
            Enqueue(Upsilon.ToByteArray());
            Enqueue(Ohm.ToByteArray());
            Enqueue(Xi.ToByteArray());
        }

        /*
        public void Clear()
        {
            Bytes = null;
        }
        */

        public void Encode(BigInteger Rho, ref BigInteger Eta)
        {
            Encode(Rho.ToByteArray(), ref Eta);
        }

        public void Encode(String Rho, ref BigInteger Eta)
        {
            Encode(Encoding.Default.GetBytes(Rho), ref Eta);
        }

        public void Encode(byte[] Rho, ref BigInteger Eta)
        {
            int _size = Rho.Length;
            Bytes = new byte[_size];
            Bytes = Rho;
        }

        public override string ToString()
        {
            if (Bytes == null) return "";
            return Encoding.Default.GetString(Bytes);
        }
    }
}
