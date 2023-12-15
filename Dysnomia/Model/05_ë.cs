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
        public int Chi = 0;
        public int Gamma = 1;

        public ë(bool Iota)
        {
            Zuo.Gamma++;
            Rho = new ည(ref Zuo.Psi.Mu, ref Zuo.Mu.Upsilon, Iota);
            Zuo.Mu.Nu.Add(Rho);
            Psi = new ည(ref Zuo.Eta.Mu, ref Zuo.Psi.Rho, !Iota);
            Zuo.Mu.Nu.Add(Psi);
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
            Zuo.Mu.Nu.Add(Eta);
            Sigma = new ညြ(ref Rho, ref Psi, ref Eta);
            Upsilon = Eta.Pi(false);
            Upsilon.Delta.WaitOne();
            Zuo.Mu.Theta.Delta.WaitOne();
            Zuo.Upsilon.Delta.WaitOne();
            Upsilon.Theta(Zuo.Mu.Theta.Omicron);
            Upsilon.Beta(Zuo.Upsilon.Omicron);
            Upsilon.Iota();
            Upsilon.Lambda();
            Upsilon.Alpha();
            Chi = 5;

            Upsilon.Delta.ReleaseMutex();
            Zuo.Mu.Theta.Delta.ReleaseMutex();
            Zuo.Upsilon.Delta.ReleaseMutex();
        }
    }
}
