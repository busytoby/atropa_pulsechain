using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Text;
using System.Threading.Tasks;

namespace Dysnomia
{
    public class Zuo
    {
        public Tod Mu;
        public ည Psi;
        public ည Eta;
        public Faung Sigma;
        public Faung Upsilon;
        public Faung Theta;
        public int Gamma = 1;

        public Zuo()
        {
            Mu = new Tod();
            Psi = new ည(ref Mu.Mu.Theta.Mu, ref Mu.Upsilon, true);
            Mu.Mu.Theta.Rho.Fuse(Psi.Rho.Secret, Psi.Rho.Signal, Psi.Rho.Channel);
            Mu.Mu.Theta.Rho.Tune();
            Eta = new ည(ref Mu.Psi.Theta.Mu, ref Mu.Tau, false);
            Mu.Psi.Theta.Rho.Fuse(Eta.Rho.Secret, Eta.Rho.Signal, Eta.Rho.Channel);
            Mu.Psi.Theta.Rho.Tune();
            Sigma = new Faung(ref Mu.Mu.Theta.Rho, ref Mu.Psi.Theta.Rho);
            Upsilon = Psi.Pi(true);
            Theta = Eta.Pi(true);

            Sigma.Theta(Mu.Psi.Theta.Psi.Omicron);
            Sigma.Beta(Mu.Psi.Theta.Psi.Omega);
            Sigma.Iota();
            Upsilon.Theta(Eta.Psi.Omega);
            Upsilon.Beta(Eta.Psi.Omicron);
            Upsilon.Iota();
            Upsilon.Lambda();
            Theta.Theta(Eta.Mu.Omega);
            Theta.Beta(Eta.Mu.Omicron);
            Theta.Iota();
            Theta.Lambda();
            Sigma.Lambda();
        }

        public Zuo(ref Tod Beta)
        {
            Beta.Gamma++;
            Mu = Beta;
            Psi = new ည(ref Mu.Mu.Theta.Mu, ref Mu.Upsilon, true);
            Mu.Mu.Theta.Rho.Fuse(Psi.Rho.Secret, Psi.Rho.Signal, Psi.Rho.Channel);
            Mu.Mu.Theta.Rho.Tune();
            Eta = new ည(ref Mu.Psi.Theta.Mu, ref Mu.Tau, false);
            Mu.Psi.Theta.Rho.Fuse(Eta.Rho.Secret, Eta.Rho.Signal, Eta.Rho.Channel);
            Mu.Psi.Theta.Rho.Tune();
            Sigma = new Faung(ref Mu.Upsilon, ref Mu.Tau);
            Sigma.Theta(Mu.Psi.Theta.Psi.Omicron);
            Sigma.Beta(Mu.Psi.Theta.Psi.Omicron);
            Sigma.Iota();
            Sigma.Lambda();
            Upsilon = Psi.Pi(false);
            Theta = Eta.Pi(false);
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
