using Dysnomia.Domain.World;
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
        public Faung Mu;
        public byte[]? Bytes;

        public Buffer(Daemon Beta)
        {
            if (Beta.Count < 5) throw new Exception("Daemon Queue Count < 5");
            BigInteger Rho, Upsilon, Ohm, Xi, Alpha;
            Rho = Beta.Next();
            Upsilon = Beta.Next();
            Ohm = Beta.Next();
            Xi = Beta.Next();
            Alpha = Beta.Next();
            Mu = new Faung(Rho, Upsilon, Ohm, Xi, Alpha);
        }

        public Buffer(BigInteger Rho, BigInteger Upsilon, BigInteger Ohm, BigInteger Xi, BigInteger Alpha)
        {
            Mu = new Faung(Rho, Upsilon, Ohm, Xi, Alpha);
        }

        public Buffer(Faung _Mu, byte[] Rho) {
            Mu = _Mu;
            Encode(Rho);
        }

        public void Clear()
        {
            Bytes = null;
        }

        public void Alpha(BigInteger Omicron)
        {
            if (Mu.Rod == null) throw new Exception("Null Rod");
            if (Mu.Cone == null) throw new Exception("Null Cone");
            Logging.Log("Buffer", "Alpha: " + Omicron.ToString(), 2);
            if (Omicron == 0) throw new Exception("Omicron Zero");

            Mu.Charge(Omicron);
            if (Mu.Sigma < 4)
                throw new Exception("Sigma < 4");
            Mu.Induce();
            Mu.Torque();
            Mu.Amplify();
            Mu.Sustain();
            Mu.React(false);
            if (Mu.Omega != Mu.Omicron) throw new Exception("NonParallel");
        }

        public void Beta(BigInteger Omicron)
        {
            if (Mu.Rod == null) throw new Exception("Null Rod");
            if (Mu.Cone == null) throw new Exception("Null Cone");
            Logging.Log("Buffer", "Beta: " + Omicron.ToString(), 4);
            if (Mu.Omicron == 0) throw new Exception("Mu Omicron Zero");
            if (Omicron == 0) throw new Exception("Iota Zero");

            BigInteger Lambda = Mu.Rod.Torque(Omicron);
            Lambda = Mu.Rod.Amplify(Lambda);
            Lambda = Mu.Rod.Sustain(Lambda);
            Mu.Rod.React(Lambda, Mu.Cone.Dynamo);
            Mu.Cone.React(Lambda, Mu.Rod.Dynamo);
        }

        private void Encode(byte[] Rho)
        {
            int _size = Rho.Length;
            if(Bytes == null) Bytes = new byte[_size];

            if (Mu.Rod == null) throw new Exception("Null Rod");
            if (Mu.Cone == null) throw new Exception("Null Cone");

            BigInteger Beta = Mu.Rod.Mu(Mu.Cone.Kappa, Mu.Rod.Kappa, Mu.Pi);
            for (int i = 0; i < _size;)
            {
                byte[] Iota = Beta.ToByteArray();
                for (int j = 0; j < Iota.Length && i < _size; j++, i++)
                    Bytes[i] = (byte)(Iota[j] ^ Rho[i]);
                Beta = Mu.Rod.Mu(Beta, Mu.Rod.Kappa, Mu.Pi);
            }
        }

        public void Recode()
        {
            if (Bytes == null) throw new Exception("Cannot Recode Null Buffer");
            Encode(Bytes);
        }

        public override String ToString()
        {
            if (Bytes == null) return "";
            return new String(Bytes.Select(b => (char)b).ToArray());
        }
    }
}
