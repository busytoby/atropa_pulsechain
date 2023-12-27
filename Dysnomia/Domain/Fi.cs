using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Text;
using System.Threading.Channels;
using System.Threading.Tasks;

namespace Dysnomia.Domain
{
    public class Fi
    {
        private Fa.Modulator Mu;
        private Faung Rho;
        private BigInteger Base, Element, Manifold, Ring, Barn, Cone_Channel, Rod_Channel, Cone_Dynamo, Rod_Dynamo;

        public Fi(ref Faung Beta)
        {
            Mu = Beta.Rod.Mu;
            if (Beta.Nu.Count != 32) throw new Exception("Count Expected 32");

            Rho = Beta;

            Base = Next();
            Element = Next();
            Manifold = Next();
            Ring = Next();
            Barn = Next();

            BigInteger Delta = Next();
            BigInteger Pi = Next();
            if (Mu(Barn, Delta, Ring) != Pi) throw new Exception("Invalid Charge");

            Delta = Next();
            if (Mu(Pi, Manifold, Ring) != Delta) throw new Exception("Invalid Induction");

            Cone_Channel = Next();
            Pi = Next();
            if (Mu(Delta, Element, Cone_Channel) != Pi) throw new Exception("Invalid Torque");

            Delta = Next();
            if (Mu(Pi, Element, Cone_Channel) != Delta) throw new Exception("Invalid Amp");

            Pi = Next();
            if (Mu(Delta, Element, Cone_Channel) != Pi) throw new Exception("Invalid Sustain");

            Rod_Channel = Next();
            Delta = Next();
            if (Mu(Pi, Cone_Channel, Rod_Channel) != Delta) throw new Exception("Invalid Rod Kappa");

            Delta = Next();
            if (Mu(Pi, Rod_Channel, Cone_Channel) != Delta) throw new Exception("Invalid Cone Kappa");

            Delta = Next();
            Pi = Next();
            if (Mu(Delta, Element, Rod_Channel) != Pi) throw new Exception("Invalid Beta Torque");

            Delta = Next();
            if (Mu(Pi, Element, Rod_Channel) != Delta) throw new Exception("Invalid Beta Amp");

            Cone_Dynamo = Next();
            Rod_Dynamo = Next();
            Pi = Next();
            if (Mu(Delta, Element, Rod_Channel) != Pi) throw new Exception("Invalid Beta Sustain");

            Delta = Next();
            if (Mu(Pi, Rod_Channel, Cone_Dynamo) != Delta) throw new Exception("Invalid Rod Eta");

            Delta = Next();
            if (Mu(Pi, Cone_Channel, Rod_Dynamo) != Delta) throw new Exception("Invalid Cone Eta");
            throw new Exception("Stubbed");
            int i = 99;
        }

        public BigInteger Next()
        {
            byte[] Beta;
            Rho.Nu.TryDequeue(out Beta);
            return new BigInteger(Beta);
        }
    }
}
