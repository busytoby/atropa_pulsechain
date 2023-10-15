using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Dysnomia
{
    public class ë
    {
        public Zuo Mu;
        public ည Rho;
        public ည Psi;
        public Faung Nu;
        public ည Eta;
        public ညြ Sigma;
        public int Gamma = 1;

        public ë(ref Zuo Beta, bool Iota)
        {
            Beta.Gamma++;
            Mu = Beta;
            Rho = new ည(ref Mu.Psi.Mu, ref Mu.Mu.Upsilon, Iota);
            Psi = new ည(ref Mu.Eta.Mu, ref Mu.Psi.Rho, !Iota);
            if (Iota)
                Nu = Mu.Upsilon;
            else
                Nu = Mu.Theta;
            Nu.Rod.Gamma++;
            Nu.Cone.Gamma++;
            if (Iota)
                Eta = new ည(ref Nu, ref Mu.Psi.Rho, true);
            else
                Eta = new ည(ref Nu, ref Mu.Eta.Rho, false);
            Sigma = new ညြ(ref Rho, ref Psi, ref Eta);
        }
    }
}
