
using Apparition.Retaliation;
using Dysnomia.Lib;
using ExtensionMethods;
using System.Collections.Concurrent;
using System.Numerics;
using System.Text;
using System.Windows;
using Windows.Foundation.Diagnostics;
using static Dysnomia.Lib.Tare;

namespace Dysnomia
{
    public static class Apparition
    {
        static public bool Stub = true;
        static public BigInteger ID;
        static public RetaliationWindow Window;
        static public Object Tau;
        static public ConcurrentQueue<Logging.MSG> MsgQueue;
        static public short LogLevel = 1;
        static public InterpretationCaller Interpreter;

        static Apparition()
        {
            ID = Math.Random();
            Tau = new Object();
            MsgQueue = new ConcurrentQueue<Logging.MSG>();
        }

        static public Tare? Input(Tare M)
        {
            MsgQueue.Enqueue(M.ToLogMSG());
            return null;
        }

        public delegate void InterpretationCaller(String A);
    }
}
