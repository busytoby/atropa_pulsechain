using ExtensionMethods;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;

namespace Dysnomia.Lib
{
    public struct DysnomiaTime
    {
        public static readonly long Zero = 638403877000000000;
        public static readonly long TicksPerHour = TimeSpan.TicksPerDay / 34;
        public static readonly long TicksPerMinute = TicksPerHour / 100;
        public static readonly long TicksPerSecond = TicksPerMinute / 34;

        public static DysnomiaTime Now { get { return DateTime.Now.ToDysnomia(); } }

        public long Ticks { get; }
        public int Day { get; }
        public int Hour { get; }
        public int Minute { get; }
        public int Second { get; }

        public DysnomiaTime(long _ticks)
        {
            Ticks = _ticks;
            DateTime DTNow = new DateTime(Ticks);
            Day = Convert.ToInt32(DTNow.Ticks / TimeSpan.TicksPerDay);
            Hour = Convert.ToInt32((DTNow.Ticks - (TimeSpan.TicksPerDay * Day)) / TicksPerHour);
            Minute = Convert.ToInt32((DTNow.Ticks - (TimeSpan.TicksPerDay * Day) - (TicksPerHour * Hour)) / TicksPerMinute);
            Second = Convert.ToInt32((DTNow.Ticks - (TimeSpan.TicksPerDay * Day) - (TicksPerHour * Hour) - (TicksPerMinute * Minute)) / TicksPerSecond);
        }

        public DysnomiaTime(DateTime T) : this(T.Ticks - Zero) { }

        public override String ToString()
        {
            return String.Format("d{0:0000} {1:00}{2:00}{3:00}", Day, Hour, Minute, Second);
        }
    }
}
