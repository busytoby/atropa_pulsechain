#pragma warning disable CS8602

using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Security.Policy;
using System.Text;
using System.Threading.Tasks;

namespace Dysnomia.Domain
{
    public class Nit : ConcurrentQueue<byte[]>
    {
        BigInteger Upsilon, Ohm, Pi, Kappa, Eta;

        public void Alpha(BigInteger Iota) // Upsilon
        {
            Enqueue(new byte[] { 0x06 });
            Enqueue(Iota.ToByteArray());
            Upsilon = Upsilon ^ Iota;
        }

        public void Alpha(ref Faung Beta) // Upsilon
        {
            Enqueue(new byte[] { 0x05 });
            Enqueue(Beta.Upsilon.ToByteArray());
            // stub additional record from beta
            Upsilon = Upsilon ^ Ohm ^ Pi ^ Beta.Upsilon;
            Enqueue(Upsilon.ToByteArray());
            Beta.Ohm = Beta.Ohm ^ Upsilon;
            Enqueue(Beta.Ohm.ToByteArray());
        }

        public void Alpha(bool Phi = true) // Upsilon
        {
            Enqueue(new byte[] { 0x04 });
            Upsilon = Phi ? Upsilon ^ Ohm ^ Pi : Upsilon ^ Ohm;
            Enqueue(Upsilon.ToByteArray());
        }

        public void Lambda(ref Faung Beta) // Rho
        {
            Enqueue(new byte[] { 0x03 });
            Upsilon = Beta.Cone.Torque(Beta.Rod.Eta);
            Enqueue(Upsilon.ToByteArray());
            Ohm = Beta.Cone.Amplify(Upsilon);
            Enqueue(Ohm.ToByteArray());
            Pi = Beta.Cone.Sustain(Ohm);
            Enqueue(Pi.ToByteArray());
            Beta.Cone.React(Pi, Beta.Cone.Dynamo);
            Kappa = Beta.Cone.Kappa;
            Beta.Rod.React(Pi, Beta.Rod.Dynamo);
            Eta = Beta.Rod.Kappa;
        }
    }
}
