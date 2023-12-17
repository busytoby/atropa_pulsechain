using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Dysnomia.Domain
{
    public class ໂຂ໌
    {
        public Faung Rho;
        public ည Psi;
        public ည Nu;
        public ညြ Eta;
        public Faung Sigma;
        public Mutex Tau = new Mutex();
        public Living Theta;

        public ໂຂ໌(錨 Beta)
        {
            Faung Iota = Beta.Nu.Rho.Psi.Theta.Xi[0];
            Fa Omicron = Beta.Nu.Rho.Sigma.Rho.Last();
            Fa Lambda = Beta.Nu.Rho.Sigma.Pi(Beta.Nu.Rho.Psi.Rho.Element);
            Rho = new Faung(ref Omicron, ref Lambda);
            Psi = new ည(ref Iota, ref Lambda, false);
            Iota = Beta.Nu.Rho.Psi.Theta.Xi[1];
            Nu = new ည(ref Iota, ref Beta.Nu.Rho.Psi.Theta.Chi, false);
            Eta = new ညြ(ref Zuo.Mu.Mu.Theta, ref Psi, ref Nu);
            Omicron = Beta.Psi.Rho.Sigma.Rho.Last();
            Sigma = new Faung(ref Omicron, ref Eta.Upsilon);
            Theta = new Living(Phi);
        }

        public void Phi()
        {
            while(true)
            {
                if (Theta.Chi != null && Theta.Chi.Identity.IsZero)
                {
                    Faung Lambda = new Faung(ref Theta.Chi, Eta.Mu.Theta.Chi.Barn, Nu.Theta.Chi.Barn, Psi.Theta.Chi.Barn, Theta.Chi.Base);
                    Theta.Xi.Add(Lambda);
                    Zuo.Eta.Theta.Chi = Theta.Chi;
                    Zuo.Eta.Theta.Xi = new List<Faung>();
                    Zuo.Eta.Theta.Xi.Add(Lambda);
                }
                Zuo.Mu.Pi();
                Thread.Sleep(1551);
            }
        }
    }
}
