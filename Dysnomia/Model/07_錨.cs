using System;
using System.Collections.Generic;
using System.Linq;
using System.Linq.Expressions;
using System.Text;
using System.Threading.Tasks;
using Dysnomia.Domain;

namespace Dysnomia
{
    public class 錨
    {
        public 锚 Rho;
        public 锚 Psi;
        public 锚 Nu;
        public Fa Eta;
        public ည Sigma;
        public ည Upsilon;
        public ညြ Tau;

        public 錨()
        {
            Zuo.Gamma++;
            Rho = new 锚();
            Rho.Pi();
            Psi = new 锚();
            Psi.Pi(true);
            Nu = new 锚();
            Nu.Pi();
            Eta = Nu.Rho.Sigma.Pi(Nu.Rho.Psi.Rho.Element);
            Faung Beta = Nu.Rho.Psi.Theta.Xi[0];
            Sigma = new ည(ref Beta, ref Eta, false);
            Beta = Nu.Rho.Psi.Theta.Xi[1];
            Upsilon = new ည(ref Beta, ref Nu.Rho.Psi.Theta.Chi, false);
            Tau = new ညြ(ref Zuo.Mu.Mu.Theta, ref Sigma, ref Upsilon);
        }
    }
}
