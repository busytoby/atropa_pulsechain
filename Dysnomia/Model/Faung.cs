using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Numerics;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace Dysnomia
{
    public class Faung
    {
        public Fa Rod;
        public Fa Cone;

        public BigInteger Phi, Eta, Mu, Xi;
        public BigInteger Sigma, Rho, Upsilon, Ohm, Pi, Omicron, Omega;
        public Mutex Delta = new Mutex();
        public short Chi;
        public Domain.Nit Nu = new Domain.Nit();

        public Faung()
        {
            bool Failed = true;
            while (Failed)
            {
                try
                {
                    Rod = new Fa();
                    Cone = new Fa();
                    OpenManifolds();
                    Failed = false;
                }
                catch (Exception E)
                {
                    continue;
                }
            }
        }

        public Faung(ref Fa Rod, ref Fa Cone, bool Refaung = true)
        {
            if (Refaung && (!Rod.Barn.IsZero || !Cone.Barn.IsZero))
                throw new Exception("Non Zero Barn");
            this.Rod = Rod;
            this.Cone = Cone;
            this.Cone.Kappa = -1;

            bool Failed = true;

            while (Failed)
            {
                try
                {
                    OpenManifolds();
                    Failed = false;
                }
                catch (Exception E)
                {
                    this.Cone.Nu = 0;
                    throw;
                }
            }
            Rod.Gamma++;
            Cone.Gamma++;
        }

        public Faung(BigInteger Rho, BigInteger Upsilon, BigInteger Ohm, BigInteger Xi)
        {
            bool Failed = true;

            while (Failed)
            {
                try
                {
                    Rod = new Fa(true);
                    Cone = new Fa(true);
                    FuseAndOpen(Rho, Upsilon, Ohm, Xi);
                    Failed = false;
                }
                catch (Exception E)
                {
                    throw;
                }
            }
        }

        public Faung(ref Fa Beta, BigInteger Rho, BigInteger Upsilon, BigInteger Ohm, BigInteger Xi, bool Phi = false)
        {
            Rod = Beta;
            if (!Rod.Barn.IsZero)
                throw new Exception("Non Zero Barn");

            bool Failed = true;

            while (Failed)
            {
                try
                {
                    Cone = new Fa(Phi);
                    Cone.Kappa = -1;
                    //if (Rho == Ohm) Rho = Math.Random();
                    FuseAndOpen(Rho, Upsilon, Ohm, Xi);
                    Failed = false;
                }
                catch (Exception E)
                {
                    throw;
                }
            }
            Rod.Gamma++;
        }

        public void FuseAndOpen(BigInteger Rho, BigInteger Upsilon, BigInteger Ohm, BigInteger Xi)
        {
            Cone.Fuse(Rho, Upsilon, Ohm);
            Cone.Tune();
            OpenManifolds(Xi);
        }

        public void OpenManifolds()
        {
            Xi = Cone.Mu(Cone.Signal, Rod.Channel, Math.Prime);
            OpenManifolds(Xi);
        }

        public void ConductorGenerate(BigInteger Xi)
        {
            Phi = Rod.Avail(Xi);
            Cone.Tau = Cone.Avail(Xi);

            Rod.Form(Cone.Tau);
            Cone.Form(Phi);

            Rod.Polarize();
            Cone.Polarize();
        }

        public void OpenManifolds(BigInteger Xi)
        {
            ConductorGenerate(Xi);

            Rod.Conjugate(ref Cone.Pole);
            Cone.Conjugate(ref Rod.Pole);

            if (Rod.Coordinate != Cone.Coordinate)
                throw new Exception("Bad Coordination");

            Cone.Conify();

            Eta = Rod.Saturate(Cone.Foundation, Cone.Channel);
            Mu = Cone.Saturate(Rod.Foundation, Rod.Channel);

            if (Rod.Element != Cone.Element)
                throw new Exception("Bad Element");

            Ratchet();

            Rod.Adduct(Cone.Dynamo);
            Cone.Adduct(Rod.Dynamo);

            Rod.Open();
            Cone.Open();

            if (!Cone.ManifoldCompare(ref Rod))
                throw new Exception("Bad Compare");
            this.Xi = Xi;
            Chi = 0;
        }

        public void Ratchet()
        {
            Rod.Bond();
            Cone.Bond();
        }

        public void Theta()
        {
            Theta(Pi > 0 ? Pi : Rod.Signal);
        }

        public void Alpha(BigInteger Iota)
        {
            if (Chi != 8) throw new Exception("Chi Non 8");
            if (Nu.Count != 34) throw new Exception("Nu Non 34");
            Nu.Enqueue(Iota.ToByteArray());
            Upsilon = Upsilon ^ Iota;
            Chi = 9;
        }

        public void Alpha(ref Faung Beta)
        {
            if (Nu.Count != 31) throw new Exception("Nu Non 31");
            if (Beta.Nu.Count != 31) throw new Exception("Beta.Nu Non 31");
            Nu.Enqueue(Beta.Upsilon.ToByteArray());
            // stub additional record from beta
            Upsilon = Upsilon ^ Ohm ^ Pi ^ Beta.Upsilon;
            Nu.Enqueue(Upsilon.ToByteArray());
            Beta.Ohm = Beta.Ohm ^ Upsilon;
            Nu.Enqueue(Beta.Ohm.ToByteArray());
            Chi = 8;
            Beta.Chi = 5;
        }

        public void Alpha()
        {
            if (Nu.Count != 30 && Nu.Count != 31) throw new Exception("Nu Non 30/31");
            if (Nu.Count == 30 || Chi == 4)
            {
                Upsilon = Upsilon ^ Ohm ^ Pi;
                Nu.Enqueue(Upsilon.ToByteArray());
                Chi = 7;
            }
            else if (Nu.Count == 31 && Chi == 5)
            {
                Upsilon = Upsilon ^ Ohm;
                Nu.Enqueue(Upsilon.ToByteArray());
                Chi = 6;
            }
            else throw new Exception("Bad Count");
        }

        public void Lambda()
        {
            if (Nu.Count != 28) throw new Exception("Nu Non 28");
            Upsilon = Cone.Torque(Rod.Eta);
            Nu.Enqueue(Upsilon.ToByteArray());
            Ohm = Cone.Amplify(Upsilon);
            Nu.Enqueue(Ohm.ToByteArray());
            Pi = Cone.Sustain(Ohm);
            Nu.Enqueue(Pi.ToByteArray());
            Cone.React(Pi, Cone.Dynamo);
            Rod.React(Pi, Rod.Dynamo);
            Chi = 4;
        }

        public void Iota()
        {
            if (Nu.Count != 23) throw new Exception("Nu Non 23");
            BigInteger Lambda = Cone.Torque(Rod.Kappa);
            Nu.Enqueue(Lambda.ToByteArray());
            Lambda = Cone.Amplify(Lambda);
            Nu.Enqueue(Lambda.ToByteArray());
            Lambda = Cone.Sustain(Ohm);
            Nu.Enqueue(Lambda.ToByteArray());
            Rod.React(Lambda, Rod.Channel);
            Nu.Enqueue(Rod.Kappa.ToByteArray());
            Cone.React(Lambda, Cone.Channel);
            Nu.Enqueue(Cone.Kappa.ToByteArray());
            Chi = 3;
        }

        public void Beta(BigInteger Iota)
        {
            if (Iota == 0) throw new Exception("Iota Zero");
            if (Nu.Count != 15) throw new Exception("Nu Non 15");

            Nu.Enqueue(Iota.ToByteArray());
            BigInteger Lambda = Rod.Torque(Iota);
            Nu.Enqueue(Lambda.ToByteArray());
            Lambda = Rod.Amplify(Lambda);
            Nu.Enqueue(Lambda.ToByteArray());
            Lambda = Rod.Sustain(Lambda);
            Nu.Enqueue(Cone.Dynamo.ToByteArray());
            Nu.Enqueue(Rod.Dynamo.ToByteArray());
            Nu.Enqueue(Lambda.ToByteArray());
            Rod.React(Lambda, Cone.Dynamo);
            Nu.Enqueue(Rod.Eta.ToByteArray());
            Cone.React(Lambda, Rod.Dynamo);
            Nu.Enqueue(Cone.Eta.ToByteArray());
            Chi = 2;
        }

        public void Theta(String Omicron)
        {
            if (Chi != 0) throw new Exception("Chi Non Zero");
            byte[] Beta = Encoding.Unicode.GetBytes(Omicron);
            BigInteger Iota = new BigInteger(Beta);
            Theta(Iota);
        }

        public void Theta(BigInteger Omicron)
        {
            if (Omicron == 0) throw new Exception("Omicron Zero");
            if (Nu.Count != 0) throw new Exception("Nu Count > 0");
            bool Failed = true;
            while (Failed)
            {
                try
                {
                    Nu.Enqueue(Rod.Base.ToByteArray());
                    Nu.Enqueue(Rod.Element.ToByteArray());
                    Nu.Enqueue(Rod.Manifold.ToByteArray());
                    Nu.Enqueue(Rod.Ring.ToByteArray());
                    Nu.Enqueue(Rod.Barn.ToByteArray());
                    Nu.Enqueue(Omicron.ToByteArray());
                    Charge(Omicron);
                    Nu.Enqueue(Sigma.ToByteArray());
                    if (Sigma < 4)
                        throw new Exception("Sigma < 4");
                    Induce();
                    Nu.Enqueue(Rho.ToByteArray());
                    Nu.Enqueue(Cone.Channel.ToByteArray());
                    Torque();
                    Nu.Enqueue(Upsilon.ToByteArray());
                    Amplify();
                    Nu.Enqueue(Ohm.ToByteArray());
                    Sustain();
                    Nu.Enqueue(Pi.ToByteArray());
                    React();
                    Nu.Enqueue(Rod.Channel.ToByteArray());
                    Nu.Enqueue(Rod.Kappa.ToByteArray());
                    Nu.Enqueue(Cone.Kappa.ToByteArray());
                    Failed = false;
                    Chi = 1;
                }
                catch (Exception E)
                {
                    throw E;
                }
            }
        }

        public void Charge(BigInteger Signal)
        {
            if (Signal == 0) throw new Exception("Signal Zero");
            Sigma = Cone.Charge(Signal);
        }

        public void Induce()
        {
            Rho = Rod.Induce(Sigma);
        }

        public void Torque()
        {
            Upsilon = Cone.Torque(Rho);
        }

        public void Amplify()
        {
            Ohm = Cone.Amplify(Upsilon);
        }

        public void Sustain()
        {
            Pi = Cone.Sustain(Ohm);
        }

        public void React()
        {
            Rod.React(Pi, Cone.Channel);
            Cone.React(Pi, Rod.Channel);
            if (Cone.Kappa != Rod.Eta || Rod.Kappa != Cone.Eta)
                throw new Exception("Non Match");
            if (Rod.Eta == Rod.Kappa)
                throw new Exception("Parallel");
            if (Cone.Kappa <= 1 && Rod.Kappa <= 1)
                throw new Exception("Negative");
            Omicron = Cone.Kappa;
            Omega = Rod.Kappa;
        }
    }
}
