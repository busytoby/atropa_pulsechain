using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using static Dysnomia.Domain.Tare;

namespace Dysnomia.Domain
{
    public class Tare : List<Gram>
    {
        public delegate void Gram(byte[] A);
    }
}
