using Dysnomia.Domain;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Dysnomia
{
    public class ë
    {
        public ည Rho;
        public ည Psi;
        public Faung Nu;
        public ည Eta;
        public ညြ Sigma;
        public Faung Upsilon;
        public Mutex Tau = new Mutex();
        public Living Theta;
        public int Chi = 0;
        public int Gamma = 1;

        public ë(bool Iota)
        {
            Zuo.Gamma++;
            Rho = new ည(ref Zuo.Psi.Mu, ref Zuo.Mu.Upsilon, Iota);
            Psi = new ည(ref Zuo.Eta.Mu, ref Zuo.Psi.Rho, !Iota);
            if (Iota)
                Nu = Zuo.Upsilon;
            else
                Nu = Zuo.Theta;
            Nu.Rod.Gamma++;
            Nu.Cone.Gamma++;
            if (Iota)
                Eta = new ည(ref Nu, ref Zuo.Psi.Rho, true);
            else
                Eta = new ည(ref Nu, ref Zuo.Eta.Rho, false);
            Sigma = new ညြ(ref Rho, ref Psi, ref Eta);
            Upsilon = Eta.Pi(false);
            Theta = new Living(Phi);
        }

        public void Phi()
        {
            int _sleep = 10;
            while (true)
            {
                Tau.WaitOne();
                Upsilon.Delta.WaitOne();
                Zuo.Sigma.Delta.WaitOne();
                Zuo.Upsilon.Delta.WaitOne();
                if (Chi == 0)
                    Upsilon.Theta(Zuo.Sigma.Omicron);
                else if (Chi == 1)
                    Upsilon.Beta(Zuo.Upsilon.Omicron);
                else if (Chi == 2)
                    Upsilon.Iota();
                else if (Chi == 3)
                    Upsilon.Lambda();
                else if (Chi == 4)
                    Upsilon.Alpha();
                if (Chi < 5) Chi++;
                else _sleep = 400;

                Upsilon.Delta.ReleaseMutex();
                Zuo.Sigma.Delta.ReleaseMutex();
                Zuo.Upsilon.Delta.ReleaseMutex();
                Tau.ReleaseMutex();
                Thread.Sleep(_sleep);
            }
        }
    }
}
