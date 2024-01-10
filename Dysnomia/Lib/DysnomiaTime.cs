using System;
using System.Collections.Generic;
using System.Linq;
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

        public int Day { get; }
        public int Hour { get; }
        public int Minute { get; }
        public int Second { get; }

        public DysnomiaTime(DateTime T)
        {
            DateTime DTNow = new DateTime(T.Ticks - Zero);
            Day = Convert.ToInt32(DTNow.Ticks / TimeSpan.TicksPerDay);
            Hour = Convert.ToInt32((DTNow.Ticks - (TimeSpan.TicksPerDay * Day)) / TicksPerHour);
            Minute = Convert.ToInt32((DTNow.Ticks - (TimeSpan.TicksPerDay * Day) - (TicksPerHour * Hour)) / TicksPerMinute);
            Second = Convert.ToInt32((DTNow.Ticks - (TimeSpan.TicksPerDay * Day) - (TicksPerHour * Hour) - (TicksPerMinute * Minute)) / TicksPerSecond);
        }

        public static DysnomiaTime Now()
        {
            return new DysnomiaTime(DateTime.Now);
        } 

        public override String ToString()
        {
            return String.Format("Day: {0} Hour: {1} Minute: {2}", Day, Hour, Minute);
        }
    }
}
