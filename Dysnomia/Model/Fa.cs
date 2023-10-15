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

        public Fa()
        {
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
            Channel = Math.ModPow(Base, Signal, Math.Prime);
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
            return Math.ModPow(Xi, Secret, Math.Prime);
        }

        public void Form(BigInteger Chi)
        {
            Base = Math.ModPow(Chi, Secret, Math.Prime);
            if (Base < 0) Base = Base * -1;
            Tune();
        }

        public void Polarize()
        {
            Pole = Math.ModPow(Base, Secret, Math.Prime);
        }

        public void Conjugate(ref BigInteger Chi)
        {
            Coordinate = Math.ModPow(Chi, Secret, Math.Prime);
            Chi = 0;
        }

        public void Conify()
        {
            if (Nu != 0) throw new Exception("Nu Non Zero");
            Identity = Math.Random();
            if (Identity < 0) Identity = Identity * -1;
            Foundation = Math.ModPow(Base, Identity, Math.Prime);
            if (Foundation < 0) throw new Exception("Negative Foundation");
            Nu = 1;
        }

        public BigInteger Saturate(BigInteger Epsilon, BigInteger Theta)
        {
            if (Nu == 0)
            {
                Identity = Math.Random();
                Foundation = Math.ModPow(Base, Identity, Math.Prime);
            }
            else if (Nu != 1) throw new Exception("Nu Non One");

            Beta = Math.ModPow(Epsilon, Identity, Math.Prime);
            Rho = Math.ModPow(Theta, Identity, Math.Prime);
            Eta = Math.ModPow(Epsilon, Signal, Math.Prime);

            Phi = Rho + Eta;
            Element = Beta + Phi;

            // Principal Uncertainty
            Dynamo = Math.ModPow(Theta, Signal, Math.Prime);
            Manifold = Element + Dynamo;

            if (Nu != 0 && Nu != 1) throw new Exception("Invalid Nu");

            return Eta;
        }

        public void Bond()
        {
            Dynamo = Math.ModPow(Base, Signal, Element);
            Pole = 0;
        }

        public void Adduct(BigInteger Phi)
        {
            Manifold = Math.ModPow(Phi, Signal, Element);
        }

        public void Open()
        {
            Ring = Math.ModPow(Coordinate, Manifold, Element);
            Barn = Math.ModPow(Ring, Manifold, Element);
        }

        public bool ManifoldCompare(ref Fa Rod)
        {
            return (Manifold == Rod.Manifold &&
                Ring == Rod.Ring &&
                Barn == Rod.Barn);
        }

        public BigInteger Charge(BigInteger Psi)
        {
            return Math.ModPow(Barn, Psi, Ring);
        }

        public BigInteger Induce(BigInteger Sigma)
        {
            return Math.ModPow(Sigma, Manifold, Ring);
        }

        public BigInteger Torque(BigInteger Sigma)
        {
            return Math.ModPow(Sigma, Element, Channel);
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
            Eta = Math.ModPow(Pi, Channel, Theta);
            Kappa = Math.ModPow(Pi, Theta, Channel);
            if (Eta == 0 || Kappa == 0) throw new Exception("Fault");
        }
    }
}
