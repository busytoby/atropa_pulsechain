using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Text;
using System.Threading.Tasks;

namespace Dysnomia.Domain
{
    public class Buffer
    {
        public byte[] Bytes;

        public Buffer(Faung Mu, byte[] Rho) {
            int _size = Rho.Length;
            Bytes = new byte[_size];
            Encode(Mu, Rho);
        }

        private void Encode(Faung Mu, byte[] Rho)
        {
            int _size = Rho.Length;

            BigInteger Beta = Mu.Rod.Mu(Mu.Rod.Kappa, Mu.Cone.Kappa, Mu.Pi);
            for (int i = 0; i < _size;)
            {
                byte[] Iota = Beta.ToByteArray();
                for (int j = 0; j < Iota.Length && i < _size; j++, i++)
                    Bytes[i] = (byte)(Iota[j] ^ Rho[i]);
                Beta = Mu.Rod.Mu(Beta, Mu.Cone.Kappa, Mu.Pi);
            }
        }

        public void Decode(Faung Mu)
        {
            Encode(Mu, Bytes);
        }

        public override String ToString()
        {
            return new String(Bytes.Select(b => (char)b).ToArray());
        }
    }
}
