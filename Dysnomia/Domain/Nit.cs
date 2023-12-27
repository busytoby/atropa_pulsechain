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

        public void Alpha(BigInteger Iota)
        {
            Enqueue(new byte[] { 0x06 });
            Enqueue(Iota.ToByteArray());
            Upsilon = Upsilon ^ Iota;
        }

        public void Alpha(ref Faung Beta)
        {
            Enqueue(new byte[] { 0x05 });
            Enqueue(Beta.Upsilon.ToByteArray());
            // stub additional record from beta
            Upsilon = Upsilon ^ Ohm ^ Pi ^ Beta.Upsilon;
            Enqueue(Upsilon.ToByteArray());
            Beta.Ohm = Beta.Ohm ^ Upsilon;
            Enqueue(Beta.Ohm.ToByteArray());
        }

        public void Alpha(bool Phi = true)
        {
            Enqueue(new byte[] { 0x04 });
            Upsilon = Phi ? Upsilon ^ Ohm ^ Pi : Upsilon ^ Ohm;
            Enqueue(Upsilon.ToByteArray());
        }

        public void Lambda(ref Faung Beta)
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

        public void Iota(ref Faung Beta)
        {
            Enqueue(new byte[] { 0x02 });
            BigInteger Lambda = Beta.Cone.Torque(Beta.Rod.Kappa);
            Enqueue(Lambda.ToByteArray());
            Lambda = Beta.Cone.Amplify(Lambda);
            Enqueue(Lambda.ToByteArray());
            Lambda = Beta.Cone.Sustain(Lambda);
            Enqueue(Lambda.ToByteArray());
            Beta.Rod.React(Lambda, Beta.Rod.Channel);
            Enqueue(Beta.Rod.Kappa.ToByteArray());
            Beta.Cone.React(Lambda, Beta.Cone.Channel);
            Enqueue(Beta.Cone.Kappa.ToByteArray());
        }

        public void Beta(ref Faung Beta, BigInteger Iota)
        {
            if (Iota == 0) throw new Exception("Iota Zero");

            Enqueue(new byte[] { 0x01 });
            Enqueue(Iota.ToByteArray());
            BigInteger Lambda = Beta.Rod.Torque(Iota);
            Enqueue(Lambda.ToByteArray());
            Lambda = Beta.Rod.Amplify(Lambda);
            Enqueue(Lambda.ToByteArray());
            Lambda = Beta.Rod.Sustain(Lambda);
            Enqueue(Beta.Cone.Dynamo.ToByteArray());
            Enqueue(Beta.Rod.Dynamo.ToByteArray());
            Enqueue(Lambda.ToByteArray());
            Beta.Rod.React(Lambda, Beta.Cone.Dynamo);
            Enqueue(Beta.Rod.Eta.ToByteArray());
            Beta.Cone.React(Lambda, Beta.Rod.Dynamo);
            Enqueue(Beta.Cone.Eta.ToByteArray());
        }

        public void Theta(ref Faung Beta, BigInteger Omicron)
        {
            if (Omicron == 0) throw new Exception("Omicron Zero");
            Enqueue(new byte[] {0x00});
            Enqueue(Beta.Rod.Base.ToByteArray());
            Enqueue(Beta.Rod.Element.ToByteArray());
            Enqueue(Beta.Rod.Manifold.ToByteArray());
            Enqueue(Beta.Rod.Ring.ToByteArray());
            Enqueue(Beta.Rod.Barn.ToByteArray());
            Enqueue(Omicron.ToByteArray());
            Beta.Charge(Omicron);
            Enqueue(Beta.Sigma.ToByteArray());
            if (Beta.Sigma < 4)
                throw new Exception("Sigma < 4");
            Beta.Induce();
            Enqueue(Beta.Rho.ToByteArray());
            Enqueue(Beta.Cone.Channel.ToByteArray());
            Beta.Torque();
            Enqueue(Beta.Upsilon.ToByteArray());
            Beta.Amplify();
            Enqueue(Beta.Ohm.ToByteArray());
            Beta.Sustain();
            Enqueue(Beta.Pi.ToByteArray());
            Beta.React();
            Enqueue(Beta.Rod.Channel.ToByteArray());
            Enqueue(Beta.Rod.Kappa.ToByteArray());
            Enqueue(Beta.Cone.Kappa.ToByteArray());
        }

        public void Theta(ref Faung Beta)
        {
            Theta(ref Beta, Beta.Pi > 0 ? Beta.Pi : Beta.Rod.Signal);
        }

        public void Theta(ref Faung Beta, String Omicron)
        {
            //if (Chi != 0) throw new Exception("Chi Non Zero");
            byte[] Iota = Encoding.Unicode.GetBytes(Omicron);
            BigInteger Kappa = new BigInteger(Iota);
            Theta(ref Beta, Kappa);
        }
    }
}
