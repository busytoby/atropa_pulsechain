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
        private BigInteger Base, Element, Manifold, Ring, Barn, Cone_Channel, Rod_Channel, Cone_Dynamo, Rod_Dynamo;

        public Fi(ref Faung Beta)
        {
            Mu = Beta.Rod.Mu;
            if (Beta.Nu.Count != 32) throw new Exception("Count Expected 32");

            Base = new BigInteger(Beta.Nu[0]);
            Element = new BigInteger(Beta.Nu[1]);
            Manifold = new BigInteger(Beta.Nu[2]);
            Ring = new BigInteger(Beta.Nu[3]);
            Barn = new BigInteger(Beta.Nu[4]);

            BigInteger Delta = new BigInteger(Beta.Nu[5]);
            BigInteger Pi = new BigInteger(Beta.Nu[6]);
            if (Mu(Barn, Delta, Ring) != Pi) throw new Exception("Invalid Charge");

            Delta = new BigInteger(Beta.Nu[7]);
            if (Mu(Pi, Manifold, Ring) != Delta) throw new Exception("Invalid Induction");

            Cone_Channel = new BigInteger(Beta.Nu[8]);
            Pi = new BigInteger(Beta.Nu[9]);
            if (Mu(Delta, Element, Cone_Channel) != Pi) throw new Exception("Invalid Torque");

            Delta = new BigInteger(Beta.Nu[10]);
            if (Mu(Pi, Element, Cone_Channel) != Delta) throw new Exception("Invalid Amp");

            Pi = new BigInteger(Beta.Nu[11]);
            if (Mu(Delta, Element, Cone_Channel) != Pi) throw new Exception("Invalid Sustain");

            Rod_Channel = new BigInteger(Beta.Nu[12]);
            Delta = new BigInteger(Beta.Nu[13]);
            if (Mu(Pi, Cone_Channel, Rod_Channel) != Delta) throw new Exception("Invalid Rod Kappa");

            Delta = new BigInteger(Beta.Nu[14]);
            if (Mu(Pi, Rod_Channel, Cone_Channel) != Delta) throw new Exception("Invalid Cone Kappa");

            Delta = new BigInteger(Beta.Nu[15]);
            Pi = new BigInteger(Beta.Nu[16]);
            if (Mu(Delta, Element, Rod_Channel) != Pi) throw new Exception("Invalid Beta Torque");

            Delta = new BigInteger(Beta.Nu[17]);
            if (Mu(Pi, Element, Rod_Channel) != Delta) throw new Exception("Invalid Beta Amp");

            Cone_Dynamo = new BigInteger(Beta.Nu[18]);
            Rod_Dynamo = new BigInteger(Beta.Nu[19]);
            Pi = new BigInteger(Beta.Nu[20]);
            if (Mu(Delta, Element, Rod_Channel) != Pi) throw new Exception("Invalid Beta Sustain");

            Delta = new BigInteger(Beta.Nu[21]);
            if(Mu(Pi, Rod_Channel, Cone_Dynamo) != Delta) throw new Exception("Invalid Rod Eta");

            Delta = new BigInteger(Beta.Nu[22]);
            if (Mu(Pi, Cone_Channel, Rod_Dynamo) != Delta) throw new Exception("Invalid Cone Eta");

            throw new Exception("Stubbed");
            int i = 99;
        }
    }
}
