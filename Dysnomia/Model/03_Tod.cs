using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Text;
using System.Threading.Tasks;
using Dysnomia.Domain;

namespace Dysnomia
{
    public class Tod
    {
        public ညြ Mu;
        public Fa Rho;
        public ညြ Psi;
        public Faung Sigma;
        public Fa Upsilon;
        public Fa Tau;
        public ည Theta;
        public int Gamma = 1;

        public Tod()
        {
            Mu = new ညြ(false);
            Rho = new Fa(true);
            Psi = new ညြ(true);

            Rho.Fuse(Mu.Eta.Rod.Manifold, Psi.Eta.Cone.Manifold, Mu.Eta.Rod.Barn);
            Sigma = new Faung(ref Rho, Mu.Eta.Rod.Manifold, Psi.Eta.Cone.Manifold, Mu.Eta.Rod.Barn, Psi.Eta.Cone.Barn, true);
            Upsilon = Mu.Pi(Mu.Mu.Psi.Omicron);
            Tau = Psi.Pi(Psi.Theta.Mu.Omega);

            Theta = new ည(ref Sigma, ref Rho, true);
        }

        /*
        public Tod(ref ညြ Beta, ref Fa Omicron, ref ည Iota)
        {
            Beta.Gamma++;
            Omicron.Gamma++;
            Mu = Beta;
            Psi = new ညြ(ref Mu.Theta, ref Iota, ref Mu.Mu);
            Upsilon = Mu.Pi(Mu.Theta.Mu.Cone.Coordinate);
            Tau = Omicron;
        }
        */ 

        /*
        vector<Faung*> Affinities()
        {
            vector<Faung*> Alpha(Mu.Affinities());
            vector<Faung*> Beta = Psi.Affinities();
            Alpha.insert(Alpha.end(), Beta.begin(), Beta.end());
            return Alpha;
        }

        ~Tod()
        {
            if (Mu.Gamma == 1) delete Mu; else Mu.Gamma--;
            if (Psi.Gamma == 1) delete Psi; else Psi.Gamma--;
            if (Upsilon.Gamma == 1) delete Upsilon; else Upsilon.Gamma--;
            if (Tau.Gamma == 1) delete Tau; else Tau.Gamma--;
        }
        */
    }
}
