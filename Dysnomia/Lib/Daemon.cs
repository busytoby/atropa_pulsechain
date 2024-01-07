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
    abstract public class Daemon : Serialization
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
