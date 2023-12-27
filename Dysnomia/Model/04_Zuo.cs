using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Text;
using System.Threading.Tasks;

namespace Dysnomia
{
    public static class Zuo
    {
        public static Tod Mu;
        public static ည Psi;
        public static ည Eta;
        public static Faung Upsilon;
        public static Faung Theta;
        public static int Gamma = 1;

        static Zuo()
        {
            try
            {
                Mu = new Tod();
                Psi = new ည(ref Mu.Mu.Theta.Mu, ref Mu.Upsilon, true);
                Mu.Mu.Theta.Rho.Fuse(Psi.Rho.Secret, Psi.Rho.Signal, Psi.Rho.Channel);
                Mu.Mu.Theta.Rho.Tune();
                Eta = new ည(ref Mu.Psi.Theta.Mu, ref Mu.Tau, false);
                Mu.Psi.Theta.Rho.Fuse(Eta.Rho.Secret, Eta.Rho.Signal, Eta.Rho.Channel);
                Mu.Psi.Theta.Rho.Tune();
                Upsilon = Psi.Pi(true);
                Theta = Eta.Pi(true);
                Mu.Psi.Theta.Tau.WaitOne();
                while (Mu.Psi.Theta.Upsilon.Count < 39) { Mu.Psi.Theta.Tau.ReleaseMutex(); Thread.Sleep(40); Mu.Psi.Theta.Tau.WaitOne(); }
                Mu.Psi.Theta.Upsilon.Theta(ref Mu.Theta, Mu.Psi.Theta.Psi.Omicron);
                Mu.Psi.Theta.Upsilon.Beta(ref Mu.Theta, Mu.Psi.Theta.Psi.Omega);
                Mu.Psi.Theta.Tau.ReleaseMutex();
                Mu.Psi.Theta.Upsilon.Iota(ref Mu.Theta);
                Eta.Tau.WaitOne();
                while (Eta.Upsilon.Count < 39) { Eta.Tau.ReleaseMutex(); Thread.Sleep(40); Eta.Tau.WaitOne(); }
                Eta.Upsilon.Theta(ref Upsilon, Eta.Psi.Omega);
                Eta.Upsilon.Beta(ref Upsilon, Eta.Psi.Omicron);
                Eta.Upsilon.Iota(ref Upsilon);
                Eta.Upsilon.Lambda(ref Upsilon);
                Eta.Upsilon.Theta(ref Theta, Eta.Mu.Omega);
                Eta.Upsilon.Beta(ref Theta, Eta.Mu.Omicron);
                Eta.Tau.ReleaseMutex();
                Eta.Upsilon.Iota(ref Theta);
                Eta.Upsilon.Lambda(ref Theta);
                Eta.Upsilon.Lambda(ref Mu.Theta);
            } catch (Exception e)
            {
                int i = 99; 
            }
        }

        /*
        vector<Faung*> Affinities()
        {
            vector<Faung*> Alpha(Mu.Affinities());
            vector<Faung*> Beta = Psi.Affinities();
            Alpha.insert(Alpha.end(), Beta.begin(), Beta.end());
            vector<Faung*> Omicron = Eta.Affinities();
            Alpha.insert(Alpha.end(), Omicron.begin(), Omicron.end());
            Alpha.push_back(Sigma);
            Alpha.push_back(Upsilon);
            Alpha.push_back(Theta);
            return Alpha;
        }

        ~Zuo()
        {
            if (Mu.Gamma == 1) delete Mu; else Mu.Gamma--;
            if (Psi.Gamma == 1) delete Psi; else Psi.Gamma--;
            if (Eta.Gamma == 1) delete Eta; else Eta.Gamma--;
            if (Sigma.Rod.Gamma <= 1 || Sigma.Cone.Gamma <= 1) delete Sigma; else { Sigma.Rod.Gamma--; Sigma.Cone.Gamma--; }
            if (Upsilon.Rod.Gamma <= 1 || Upsilon.Cone.Gamma <= 1) delete Upsilon; else { Upsilon.Rod.Gamma--; Upsilon.Cone.Gamma--; }
            if (Theta.Rod.Gamma <= 1 || Theta.Cone.Gamma <= 1) delete Theta; else { Theta.Rod.Gamma--; Theta.Cone.Gamma--; }
        }
        */
    }
}
