﻿using System;
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
        public Mutex Tau = new Mutex();
        public 活 Omega;
        public Fa Chi;
        public List<Faung> Xi;
        private Thread Omicron;

        public Living(活 O) 
        {
            Omega = O;
            Omicron = new Thread(new ThreadStart(Omega));
            Omicron.Start();
        }

        public void Fong(Fa Beta, List<Faung> Iota)
        {
            Tau.WaitOne();
            if (Chi != null || Xi != null) throw new Exception("Non Null");
            Chi = Beta;
            Xi = Iota;
            Tau.ReleaseMutex();
        }
    }
}