using Dysnomia;
using Dysnomia.Lib;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Reflection.Emit;
using System.Text;
using System.Threading.Tasks;

namespace ExtensionMethods
{
    public static class ExtensionMethods
    {
        public static Serialization OpenSerialization(this Fa Rho)
        {
            Serialization Nu = new Serialization();
            Nu.Enqueue(new byte[] { 0x07 });
            Nu.Enqueue(Rho.Ring.ToByteArray());
            Nu.Enqueue(Rho.Coordinate.ToByteArray());
            Nu.Enqueue(Rho.Manifold.ToByteArray());
            Nu.Enqueue(Rho.Barn.ToByteArray());
            Nu.Enqueue(Rho.Element.ToByteArray());
            return Nu;
        }

        public static DysnomiaTime ToDysnomia(this DateTime Mu)
        {
            return new DysnomiaTime(Mu);
        }
    }
}
