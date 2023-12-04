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

        public 錨(ref Zuo Beta) : this()
        {
            Mu = Beta;
        }

        public 錨()
        {
            if (Mu == null)
            {
                Task t0 = new Task(() => { Rho = new 锚(); Mu = Rho.Mu; Rho.Pi(); });
                t0.Start();
            }
            else
            {
                Task t0 = new Task(() => { Rho = new 锚(ref Mu); Rho.Pi(); });
                t0.Start();
            }
            
            Task t1 = new Task(() => {
                while (Mu == null) Thread.Sleep(500);
                Psi = new 锚(ref Mu); Psi.Pi(); 
            });
            t1.Start();
            
            Task t2 = new Task(() => {
                while (Mu == null) Thread.Sleep(500);
                Nu = new 锚(ref Mu); Nu.Pi(); 
            });
            t2.Start();

            while(Mu == null || Rho == null || Psi == null || Nu == null) {
                System.Threading.Thread.Sleep(1000);
            }
        }
    }
}
