using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;
using Dysnomia.Lib;
using static Dysnomia.Lib.Tare;

namespace Dysnomia.Domain.World
{
    public class Living
    {
        public delegate void 活();
        public ConcurrentQueue<MSG> In;
        public ConcurrentQueue<MSG> Out;
        public Tare Sigma;
        public Mutex Tau = new Mutex();
        public 活? Omega;
        private Thread? Omicron;

        public Living()
        {
            In = new ConcurrentQueue<MSG>();
            Out = new ConcurrentQueue<MSG>();
            Sigma = new Tare();
        }

        public Living(活 O)
        {
            In = new ConcurrentQueue<MSG>();
            Out = new ConcurrentQueue<MSG>();
            Sigma = new Tare();
            Omega = O;
            Omicron = new Thread(new ThreadStart(Omega));
            Omicron.Start();
        }

        public void Run(活 O)
        {
            Omega = O;
            Omicron = new Thread(new ThreadStart(Omega));
            Omicron.Start();
        }

        public bool Alive() { return Omicron != null && Omicron.IsAlive; }
    }
}
