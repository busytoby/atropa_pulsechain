using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using static Dysnomia.Lib.Tare;

namespace Dysnomia.Lib
{
    static public class Logging
    {
        /* for now
         * 1 - data
         * 2 - crypt
         * 3 - operational
         * 4 - opcode
         * 5 - function
         * 6 - msg
         * 7 - error
         */
        public class MSG
        {
            public byte[] From;
            public byte[]? Subject;
            public byte[] Data;
            public short Priority;
            public DysnomiaTime TimeStamp;

            public MSG(byte[] from, byte[]? subject, byte[] data, short priority)
            {
                From = from;
                TimeStamp = DysnomiaTime.Now;
                Subject = subject;
                Data = data;
                Priority = priority;
            }

            public MSG(byte[] from, byte[] data, short priority) : this(from, null, data, priority) { }
        }


        static private Tare Tau;
        static private object Theta;

        static Logging()
        {
            Tau = new Tare();
            Theta = new object();
        }

        static public void Add(Gram G)
        {
            Tau.Subscribers.Add(G);
        }

        static public void Log(Tare A)
        {
            new Task(() => { foreach (Gram G in Tau.Subscribers) G(A); }).Start();
        }

        static public void Log(string From, string Data, short Priority = 6)
        {
            lock (Theta)
            {
                Tau.Enqueue(From, Data, Priority);
                foreach (Gram G in Tau.Subscribers) G(Tau);
                Tau.Clear();
            }
        }
    }
}
