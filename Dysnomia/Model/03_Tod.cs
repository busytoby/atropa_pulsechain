using System;
using System.Collections.Concurrent;
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
        public ConcurrentQueue<Fi> Rho;
        public ညြ Psi;
        public List<ည> Nu;
        public Fa Upsilon;
        public Fa Tau;
        public Faung Theta;
        public int Gamma = 1;

        public Tod()
        {
            Mu = new ညြ(false);
            Rho = new ConcurrentQueue<Fi>();
            Psi = new ညြ(true);
            Nu = [Mu.Mu, Mu.Theta, Psi.Mu, Psi.Theta];
            Upsilon = Mu.Pi(Mu.Mu.Psi.Omicron);
            Tau = Psi.Pi(Psi.Theta.Mu.Omega);

            Theta = new Faung(ref Mu.Theta.Rho, ref Psi.Theta.Rho);
        }

        public void Pi()
        {
            if (Rho.Count > 0)
            {
                int i = 99;
            }

            /*
            if (Nu[12].Theta.Chi != null && Nu[4].Theta.Chi == null)
            {
                Nu[4].Theta.Chi = Nu[12].Theta.Chi;
                Nu[4].Theta.Xi = Nu[12].Theta.Xi;
                //Nu[12].Chi = 12;
            }

            if (Nu[19].Theta.Chi != null && Nu[11].Theta.Chi == null)
            {
                Nu[11].Theta.Chi = Nu[19].Theta.Chi;
                Nu[11].Theta.Xi = Nu[19].Theta.Xi;
            }

            if (Nu[4].Theta.Chi == null || Nu[11].Theta.Chi == null)
                throw new Exception("Incomplete");

            if (Nu[1].Theta.Chi != null && Nu[1].Theta.Xi.Count == 3 && Nu[1].Theta.Xi[0].Chi == 1)
            {
                Faung Beta = Nu[1].Theta.Xi[1];
                Beta.Beta(Nu[5].Theta.Chi.Channel);
                Beta.Iota();
                Beta.Lambda();
                Nu[1].Theta.Xi[0].Beta(Nu[5].Theta.Chi.Channel);
                Nu[1].Theta.Xi[0].Iota();
                Nu[1].Theta.Xi[0].Lambda();
                Nu[1].Theta.Xi[0].Alpha(ref Beta);
            }
            */
            /*
            if (Nu[5].Chi == 11)
            {
                Nu[2].Theta.Chi = Nu[5].Theta.Chi;
                Nu[2].Theta.Xi = Nu[5].Theta.Xi;
                Nu[5].Theta.Xi = new List<Faung>();
                Nu[5].Mu.Alpha();
                Nu[5].Chi++;
            }
            */

            /*
            foreach(ည Beta in Nu)
            {
                continue;
            }
            */
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
