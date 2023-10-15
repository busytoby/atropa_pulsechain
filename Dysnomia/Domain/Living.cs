using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;

namespace Dysnomia.Domain
{
    public class Living
    {
        public delegate void 活();
        public 活 Omega;
        private Thread Omicron;

        public Living(活 O) 
        {
            Omega = O;
            Omicron = new Thread(new ThreadStart(Omega));
            Omicron.Start();
        }
    }
}
