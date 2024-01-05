using Dysnomia.Domain;
using Dysnomia.Domain.World;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Dysnomia
{
    static public class Beta
    {
        static public Oracle Oracle;
        static public Fi Fi;

        static Beta()
        {
            Oracle = new Oracle();
            Fi = new Fi();
            Fi.Rho.Add(Oracle.Fi);
        }
    }
}
