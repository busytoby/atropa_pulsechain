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
        public void Beta(ref Faung Beta, BigInteger Iota)
        {
            if (Iota == 0) throw new Exception("Iota Zero");
            if (Count < 16) throw new Exception("Count < 16");

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
            Enqueue(null);
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
