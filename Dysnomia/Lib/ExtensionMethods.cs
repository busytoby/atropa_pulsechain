using Dysnomia;
using Dysnomia.Lib;
using System;
using System.Buffers.Text;
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
        public static DysnomiaTime ToDysnomia(this DateTime Mu)
        {
            return new DysnomiaTime(Mu);
        }

        public static Logging.MSG ToLogMSG(this Tare M)
        {
            if (M.Count < 5) throw new Exception("Tare Short");
            byte OpCode;
            byte[]? Ticks, From, Subject, Data;
            short Priority;
            Subject = null;
            OpCode = M.OpCode();
            if (OpCode == 0x10 || OpCode == 0x11)
            {
                Ticks = M.NextBytes();
                From = M.NextBytes();
                if (OpCode == 0x11) Subject = M.NextBytes();
                Data = M.NextBytes();
                Priority = M.OpCode();
                return new Logging.MSG(From, Subject, Data, Priority);
            }

            else throw new Exception("Unknown OpCode");
        }
    }
}
