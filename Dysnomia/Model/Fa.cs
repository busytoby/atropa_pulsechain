using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Text;
using System.Threading.Tasks;

namespace Dysnomia
{
    public class Fa
    {
        public BigInteger Base, Secret, Signal, Channel, Pole, Identity, Foundation, Element, Dynamo, Manifold, Ring, Barn, Coordinate;
        public BigInteger Tau, Eta, Kappa, Rho, Beta, Phi;
        public short Nu;
        public int Gamma = 1;
        public delegate BigInteger Modulator(BigInteger A, BigInteger B, BigInteger C);
        Modulator Mu;

        public Fa(bool Omicron = false)
        {
            if (Omicron)
                Mu = Math.ModXOR;
            else
                Mu = Math.ModPow;
            Tau = 0;
            Initialize();
            Seed();
            Tune();
        }

        public void Initialize()
        {
            Base = Secret = Signal = Channel = Pole = 0;
            Identity = Foundation = Element = 0;
            Dynamo = 0;
            Manifold = 0;
            Ring = 0;
            Barn = Ring;
            Eta = Kappa = Rho = Beta = Phi = 0;
            Nu = 0;
            Coordinate = 0;
        }

        public void Seed()
        {
            Base = Math.Random();
            if (Base < 0) Base = Base * -1;
            Secret = Math.Random();
            Signal = Math.Random();
            if (Signal < 0) Signal = Signal * -1;
        }

        public void Tune()
        {
            Channel = Mu(Base, Signal, Math.Prime);
            if (Channel < 0) throw new Exception("Negative Channel");
        }

        public void Fuse(BigInteger Rho, BigInteger Upsilon, BigInteger Ohm)
        {
            Base = Upsilon;
            if (Base < 0)
                throw new Exception("Negative Base");
            Secret = Ohm;
            Signal = Rho;
        }

        public BigInteger Avail(BigInteger Xi)
        {
            return Mu(Xi, Secret, Math.Prime);
        }

        public void Form(BigInteger Chi)
        {
            Base = Mu(Chi, Secret, Math.Prime);
            if (Base < 0) Base = Base * -1;
            Tune();
        }

        public void Polarize()
        {
            Pole = Mu(Base, Secret, Math.Prime);
        }

        public void Conjugate(ref BigInteger Chi)
        {
            Coordinate = Mu(Chi, Secret, Math.Prime);
            Chi = 0;
        }

        public void Conify()
        {
            if (Nu != 0) throw new Exception("Nu Non Zero");
            Identity = Math.Random();
            if (Identity < 0) Identity = Identity * -1;
            Foundation = Mu(Base, Identity, Math.Prime);
            if (Foundation < 0) throw new Exception("Negative Foundation");
            Nu = 1;
        }

        public BigInteger Saturate(BigInteger Epsilon, BigInteger Theta)
        {
            if (Nu == 0)
            {
                Identity = Math.Random();
                Foundation = Mu(Base, Identity, Math.Prime);
            }
            else if (Nu != 1) throw new Exception("Nu Non One");

            Beta = Mu(Epsilon, Identity, Math.Prime);
            Rho = Mu(Theta, Identity, Math.Prime);
            Eta = Mu(Epsilon, Signal, Math.Prime);

            Phi = Rho + Eta;
            Element = Beta + Phi;

            // Principal Uncertainty
            Dynamo = Mu(Theta, Signal, Math.Prime);
            Manifold = Element + Dynamo;

            if (Nu != 0 && Nu != 1) throw new Exception("Invalid Nu");

            return Eta;
        }

        public void Bond()
        {
            Dynamo = Mu(Base, Signal, Element);
            Pole = 0;
        }

        public void Adduct(BigInteger Phi)
        {
            Manifold = Mu(Phi, Signal, Element);
        }

        public void Open()
        {
            Ring = Mu(Coordinate, Manifold, Element);
            Barn = Mu(Ring, Manifold, Element);
        }

        public bool ManifoldCompare(ref Fa Rod)
        {
            return (Manifold == Rod.Manifold &&
                Ring == Rod.Ring &&
                Barn == Rod.Barn);
        }

        public BigInteger Charge(BigInteger Psi)
        {
            return Mu(Barn, Psi, Ring);
        }

        public BigInteger Induce(BigInteger Sigma)
        {
            return Mu(Sigma, Manifold, Ring);
        }

        public BigInteger Torque(BigInteger Sigma)
        {
            return Mu(Sigma, Element, Channel);
        }

        public BigInteger Amplify(BigInteger Upsilon)
        {
            return Torque(Upsilon);
        }

        public BigInteger Sustain(BigInteger Ohm)
        {
            return Torque(Ohm);
        }

        public void React(BigInteger Pi, BigInteger Theta)
        {
            Eta = Mu(Pi, Channel, Theta);
            Kappa = Mu(Pi, Theta, Channel);
            if (Eta == 0 || Kappa == 0) throw new Exception("Fault");
        }
    }
}
