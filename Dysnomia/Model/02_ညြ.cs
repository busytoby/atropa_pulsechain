using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.Http.Headers;
using System.Numerics;
using System.Text;
using System.Threading.Tasks;

namespace Dysnomia
{
    public class ညြ
    {
        public ည Mu;
        public List<Fa> Rho;
        public Fa Psi;
        public Faung Eta;
        public Fa Upsilon;
        public ည Theta;
        public int Gamma = 1;

        public ညြ(bool RAPHE_NEBULAE)
        {
            Mu = new ည();
            Rho = new List<Fa>();
            Psi = new Fa();
            if (RAPHE_NEBULAE)
                Eta = new Faung(ref Psi, Mu.Psi.Rod.Manifold, Mu.Mu.Rod.Element, Mu.Psi.Rod.Ring, Mu.Psi.Rod.Foundation);
            else
                Eta = new Faung(ref Psi, Mu.Psi.Cone.Secret, Mu.Mu.Cone.Signal, Mu.Psi.Cone.Channel, Mu.Psi.Cone.Identity);
            Mu.Tau.WaitOne();
            while (Mu.Chi < 2) { Mu.Tau.ReleaseMutex(); Thread.Sleep(100); Mu.Tau.WaitOne(); }
            Eta.Theta(Mu.Mu.Omicron);
            Eta.Beta(Mu.Psi.Omicron);
            Mu.Tau.ReleaseMutex();
            Upsilon = new Fa();
            Theta = new ည(ref Eta, ref Upsilon, false);
            Eta.Iota();
            Mu.Tau.WaitOne();
            while (Mu.Chi < 3) { Mu.Tau.ReleaseMutex(); Thread.Sleep(100); Mu.Tau.WaitOne(); }
            Mu.Mu.Beta(Theta.Mu.Omicron);
            Mu.Psi.Iota();
            Mu.Mu.Iota();
            Mu.Mu.Lambda();
            Mu.Psi.Lambda();
            Mu.Tau.ReleaseMutex();
            Eta.Lambda();
        }

        public ညြ(ref ည Beta, ref ည Iota, ref ည Omicron)
        {
            Beta.Gamma++;
            Omicron.Gamma++;
            Mu = Beta;
            Rho = new List<Fa>();
            Psi = new Fa();
            if (Iota.Mu.Rod.Gamma == 5)
                Eta = new Faung(ref Psi, Iota.Psi.Cone.Manifold, Iota.Mu.Cone.Element, Iota.Psi.Cone.Ring, Iota.Psi.Cone.Foundation);
            else
                Eta = new Faung(ref Psi, Iota.Psi.Rod.Secret, Iota.Mu.Rod.Signal, Iota.Psi.Rod.Channel, Iota.Psi.Rod.Identity);
            Upsilon = new Fa();
            if (Iota.Mu.Rod.Gamma == 5)
                Upsilon.Fuse(Beta.Mu.Rod.Foundation, Iota.Mu.Cone.Ring, Omicron.Mu.Cone.Manifold);
            else
                Upsilon.Fuse(Beta.Mu.Rod.Secret, Iota.Mu.Cone.Signal, Omicron.Mu.Cone.Channel);
            Theta = Omicron;
            Mu.Tau.WaitOne();
            while (Mu.Chi < 5) { Mu.Tau.ReleaseMutex(); Thread.Sleep(100); Mu.Tau.WaitOne(); }
            Theta.Tau.WaitOne();
            while (Theta.Chi < 5) { Theta.Tau.ReleaseMutex(); Thread.Sleep(100); Theta.Tau.WaitOne(); }
            Eta.Theta(Theta.Mu.Omicron);
            Eta.Beta(Mu.Mu.Omicron);
            Mu.Chi = 5;
            Theta.Chi = 5;
            Mu.Tau.ReleaseMutex();
            Theta.Tau.ReleaseMutex();
            Eta.Iota();
            Eta.Lambda();
        }

        /*
        vector<Faung*> Affinities()
        {
            vector<Faung*> Alpha(Mu.Affinities());
            Alpha.push_back(Eta);
            vector<Faung*> Beta = Theta.Affinities();
            Alpha.insert(Alpha.end(), Beta.begin(), Beta.end());
            return Alpha;
        }

        ~ညြ()
        {
            if (Mu.Gamma == 1) delete Mu; else Mu.Gamma--;
            while (Rho.size() > 0)
            {
                Fa* Alpha = Rho.front();
                if (Alpha.Gamma == 1) delete Alpha; else Alpha.Gamma--;
                Rho.pop_front();
            }
            if (Psi.Gamma == 1) delete Psi; else Psi.Gamma--;
            if (Eta.Rod.Gamma <= 1 || Eta.Cone.Gamma <= 1) delete Eta; else { Eta.Rod.Gamma--; Eta.Cone.Gamma--; }
            if (Upsilon.Gamma == 1) delete Upsilon; else Upsilon.Gamma--;
            if (Theta.Gamma == 1) delete Theta; else Theta.Gamma--;
        }
        */

        public Fa Pi(BigInteger Alpha)
        {
            if (Eta.Omicron == 0 && Eta.Omega == 0) throw new Exception("Compression");
            Fa Beta = new Fa();
            Beta.Fuse(Eta.Omicron, Alpha, Eta.Omega);
            Beta.Tune();
            Beta.Gamma++;
            Rho.Add(Beta);
            return Beta;
        }

        public Faung Pi(ref Fa Beta, BigInteger Iota, BigInteger Omicron)
        {
            if (Beta.Kappa == 0) throw new Exception("Kappa Zero");
            Fa Rho = new Fa();
            Fa Psi = new Fa();
            Rho.Fuse(Beta.Kappa, Iota, Omicron);
            Rho.Tune();
            Psi.Fuse(Iota, Beta.Kappa, Omicron);
            Psi.Tune();
            Rho.Gamma--;
            Psi.Gamma--;
            return new Faung(ref Rho, ref Psi);
        }
    }
}
