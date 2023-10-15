using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Dysnomia.Domain
{
    public class Atmos
    {
        public Fa Mu;
        public List<Faung> Rho;
        public List<Faung> Water;

        public Atmos(ref Fa Eta)
        {
            Eta.Gamma++;
            Mu = Eta;
        }
    }
}
