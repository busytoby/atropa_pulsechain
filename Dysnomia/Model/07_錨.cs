using System;
using System.Collections.Generic;
using System.Linq;
using System.Linq.Expressions;
using System.Text;
using System.Threading.Tasks;
using Dysnomia.Domain;

namespace Dysnomia
{
    public class 錨
    {
        public Zuo Mu;
        public 锚 Rho;
        public 锚 Psi;
        public 锚 Nu;

        public 錨(ref Zuo Beta, ref 锚 Iota, ref 锚 Omicron, ref 锚 Lambda) 
        {
            Mu = Beta;
            Rho = Iota;
            Nu = Omicron;
            Psi = Lambda;
        }

        public 錨(ref Zuo Beta, ref 锚 Iota, ref 锚 Omicron) : this()
        {
            Mu = Beta;
            Rho = Iota;
            Nu = Omicron;
        }

        public 錨(ref Zuo Beta, ref 锚 Iota) : this()
        {
            Mu = Beta;
            Rho = Iota;
        }

        public 錨(ref Zuo Beta) : this()
        {
            Mu = Beta;
            Rho = new 锚(ref Mu); 
            Rho.Pi();
        }

        public 錨()
        {
            if (Mu == null)
            {
                Task t0 = new Task(() =>
                {
                    Rho = new 锚(); Mu = Rho.Mu; Rho.Pi();
                    Psi = new 锚(ref Mu); Psi.Pi();
                    Nu = new 锚(ref Mu); Nu.Pi();
                });
                t0.Start();
                while (Mu == null) Thread.Sleep(200);
                Task t1 = new Task(() => { Psi = new 锚(ref Mu); Psi.Pi(); });
                t1.Start();
                Task t2 = new Task(() => { Nu = new 锚(ref Mu); Nu.Pi(); });
                t2.Start();
            }
            else if (Nu == null)
            {
                Task t1 = new Task(() => { Psi = new 锚(ref Mu); Psi.Pi(); });
                t1.Start();
                Task t2 = new Task(() => { Nu = new 锚(ref Mu); Nu.Pi(); });
                t2.Start();
            } else
            {
                Task t1 = new Task(() => { Psi = new 锚(ref Mu); Psi.Pi(); });
                t1.Start();
            }

            while(Mu == null || Rho == null || Psi == null || Nu == null) {
                System.Threading.Thread.Sleep(400);
            }
        }
    }
}
