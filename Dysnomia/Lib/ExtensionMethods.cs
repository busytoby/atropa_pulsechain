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
        public struct DysnomiaTime {
            public static readonly long Zero = 638403877000000000;
            public static readonly long TicksPerHour = TimeSpan.TicksPerDay / 34;
            public static readonly long TicksPerMinute = TimeSpan.TicksPerHour / 100;

            public int Day { get; }
            public int Hour { get; }
            public int Minute { get; }

            public DysnomiaTime(DateTime T)
            {
                DateTime DTNow = new DateTime(T.Ticks - Zero);
                Day = Convert.ToInt32(DTNow.Ticks / TimeSpan.TicksPerDay);
                Hour = Convert.ToInt32((DTNow.Ticks - (TimeSpan.TicksPerDay * Day)) / TicksPerHour);
                Minute = Convert.ToInt32((DTNow.Ticks - (TimeSpan.TicksPerDay * Day) - (TicksPerHour * Hour)) / TicksPerMinute);
            }
        }

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
