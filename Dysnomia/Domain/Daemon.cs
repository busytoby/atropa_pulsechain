using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;

namespace Dysnomia.Domain
{
    abstract public class Daemon : ConcurrentQueue<byte[]>
    {
        static public string Name = "Daemon";
        static public string Description = "Not Set Description";
        public Object Tau;
        public Living? Theta;

        public Daemon()
        {
            Tau = new Object();
            Theta = null;
        }

        public byte[] NextBytes()
        {
            if (Count == 0) throw new Exception("No Next");
            byte[]? Beta;
            TryDequeue(out Beta);
            if (Beta == null) throw new Exception("Dequeue Failure");
            return Beta;
        }

        public BigInteger Next()
        {
            return new BigInteger(NextBytes());
        }

        protected virtual void Phi()
        {
            throw new Exception("Not Implemented");
        }
    }
}
