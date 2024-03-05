using System.Numerics;
using System.Text;
using static Dysnomia.Lib.Tare;
using static System.Runtime.InteropServices.JavaScript.JSType;

namespace Dysnomia.Lib
{
    public class Tare : Conjunction
    {
        public List<Gram> Subscribers;

        public Tare()
        {
            Subscribers = new List<Gram>();
        }

        public void Enqueue(BigInteger N) { Enqueue(N.ToByteArray()); }
        public void Enqueue(string S) { Enqueue(Encoding.Default.GetBytes(S)); }
        public void Enqueue(long N)
        {
            if (N < 255) Enqueue(new byte[] { Convert.ToByte(N) });
            else Enqueue(BitConverter.GetBytes(N));
        }

        public void Enqueue(string From, string Data, short Priority)
        {
            if (Count != 0) throw new Exception("Already Tare");
            Enqueue(0x10);
            Enqueue(DysnomiaTime.Now.Ticks);
            Enqueue(From);
            Enqueue(Data);
            Enqueue(Priority);
        }

        public void Enqueue(string From, string Subject, byte[] Data, short Priority)
        {
            if (Count != 0) throw new Exception("Already Tare");
            Enqueue(0x11);
            Enqueue(DysnomiaTime.Now.Ticks);
            Enqueue(From);
            Enqueue(Subject);
            Enqueue(Data);
            Enqueue(Priority);
        }

        public void Enqueue(string From, byte[] Subject, byte[] Data, short Priority)
        {
            if (Count != 0) throw new Exception("Already Tare");
            Enqueue(0x17);
            Enqueue(DysnomiaTime.Now.Ticks);
            Enqueue(From);
            Enqueue(Subject);
            Enqueue(Data);
            Enqueue(Priority);
        }

        public void Enqueue(byte[] From, byte[] Data, short Priority)
        {
            if (Count != 0) throw new Exception("Already Tare");
            Enqueue(0x10);
            Enqueue(DysnomiaTime.Now.Ticks);
            Enqueue(From);
            Enqueue(Data);
            Enqueue(Priority);
        }

        public bool IsData()
        {
            if (Count != 4) return false;
            byte[]? OpCode;
            if (TryPeek(out OpCode))
                if (OpCode.Length == 1 && OpCode[0] == 0x10)
                    return true;
            return false;
        }

        public Logging.MSG ToLogMSG()
        {
            if (Count < 5) throw new Exception("Tare Short");
            byte _OpCode = OpCode();
            byte[]? Ticks, From, Subject, Data;
            short Priority;
            Subject = null;

            if (_OpCode == 0x10 || _OpCode == 0x11)
            {
                Ticks = NextBytes();
                From = NextBytes();
                if (_OpCode == 0x11) Subject = NextBytes();
                Data = NextBytes();
                Priority = OpCode();
                return new Logging.MSG(From, Subject, Data, Priority);
            }

            else throw new Exception("Unknown OpCode");
        }

        public delegate Tare? Gram(Tare M);
    }
}