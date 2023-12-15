using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Dysnomia.Domain
{
    public class ໂຂ໌
    {
        public Faung Eta;
        public ည Sigma;
        public ည Upsilon;
        public ညြ Tau;
        public Faung Theta;

        public ໂຂ໌(錨 Beta)
        {
            Faung Iota = Beta.Nu.Rho.Psi.Theta.Xi[0];
            Fa Omicron = Beta.Nu.Rho.Sigma.Rho.Last();
            Fa Lambda = Beta.Nu.Rho.Sigma.Pi(Beta.Nu.Rho.Psi.Rho.Element);
            Eta = new Faung(ref Omicron, ref Lambda);
            Sigma = new ည(ref Iota, ref Lambda, false);
            Iota = Beta.Nu.Rho.Psi.Theta.Xi[1];
            Upsilon = new ည(ref Iota, ref Beta.Nu.Rho.Psi.Theta.Chi, false);
            Tau = new ညြ(ref Zuo.Mu.Mu.Theta, ref Sigma, ref Upsilon);
            Omicron = Beta.Psi.Rho.Sigma.Rho.Last();
            Theta = new Faung(ref Omicron, ref Tau.Upsilon);
        }
    }
}
