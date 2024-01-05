using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;

namespace Dysnomia.Domain
{
    public class Daemon : ConcurrentQueue<byte[]>
    {
        static public string Name = "Daemon";
        static public string Description = "Not Set Description";
        public Living? Theta;

        public Daemon()
        {
            Theta = null;
        }

        protected virtual void Phi()
        {
            throw new Exception("Not Implemented");
        }
    }
}
