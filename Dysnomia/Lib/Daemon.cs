using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using Dysnomia.Domain.World;

namespace Dysnomia.Lib
{
    abstract public class Daemon : Conjunction
    {
        static public string Name = "Daemon";
        static public string Description = "Not Set Description";
        public object Tau;
        public Living? Theta;

        public Daemon()
        {
            Tau = new object();
            Theta = null;
        }

        public void Input(byte[] From, byte[] Data, short Priority)
        {
            if (Theta == null) throw new Exception("Null Theta");
            Tare M = new Tare();
            M.Enqueue(From);
            M.Enqueue(Data);
            M.Enqueue(Priority);
            Theta.In.Enqueue(M);
        }

        public void Input(String From, String Subject, byte[] Data, short Priority)
        {
            if (Theta == null) throw new Exception("Null Theta");
            Tare M = new Tare();
            M.Enqueue(From);
            M.Enqueue(Subject);
            M.Enqueue(Data);
            M.Enqueue(Priority);
            Theta.In.Enqueue(M);
        }

        public void Output(byte[] From, byte[] Data, short Priority)
        {
            if (Theta == null) throw new Exception("Null Theta");
            Tare M = new Tare();
            M.Enqueue(From);
            M.Enqueue(Data);
            M.Enqueue(Priority);
            Theta.Out.Enqueue(M);
        }

        public void Output(String From, String Subject, byte[] Data, short Priority)
        {
            if (Theta == null) throw new Exception("Null Theta");
            Tare M = new Tare();
            M.Enqueue(From);
            M.Enqueue(Subject);
            M.Enqueue(Data);
            M.Enqueue(Priority);
            Theta.Out.Enqueue(M);
        }

        protected virtual void Phi()
        {
            throw new Exception("Not Implemented");
        }
    }
}
