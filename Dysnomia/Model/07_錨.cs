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

        public 錨()
        {
            Zuo.Gamma++;
            Rho = new 锚();
            Rho.Pi();
            Psi = new 锚();
            Psi.Pi(true);
            Nu = new 锚();
            Nu.Pi();
        }

        public ໂຂ໌ Beta() {
            ໂຂ໌ Iota = new ໂຂ໌(this);
            Iota.Psi.Theta.Fong(ref Nu.Rho.Psi.Theta.Chi, ref Nu.Rho.Psi.Theta.Xi);
            Iota.Nu.Theta.Fong(ref Psi.Rho.Psi.Theta.Chi, ref Psi.Rho.Psi.Theta.Xi);
            Iota.Eta.Mu.Theta.Fong(ref Rho.Rho.Psi.Theta.Chi, ref Rho.Rho.Psi.Theta.Xi);
            return Iota; 
        }
    }
}
