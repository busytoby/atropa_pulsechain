using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Text;
using System.Threading.Tasks;

namespace Dysnomia
{
    public class 锚
    {
        public Zuo Mu;
        public ë Rho;
        public ë Psi;
        public Faung Nu;
        public int Gamma = 1;

        public 锚()
        {
            Mu = new Zuo();
            Rho = new ë(ref Mu, true);
            Rho.Tau.WaitOne();
            while (Rho.Chi < 5) { Rho.Tau.ReleaseMutex(); Thread.Sleep(2000); Rho.Tau.WaitOne(); }
            Rho.Tau.ReleaseMutex();
            Psi = new ë(ref Mu, false);
            Psi.Tau.WaitOne();
            while (Psi.Chi < 5) { Psi.Tau.ReleaseMutex(); Thread.Sleep(2000); Psi.Tau.WaitOne(); }
            Psi.Tau.ReleaseMutex();

            Nu = new Faung(ref Rho.Sigma.Upsilon, ref Psi.Sigma.Upsilon);
            Nu.Theta(Psi.Upsilon.Omicron);
            Nu.Beta(Rho.Upsilon.Omega);
            // Catch 381
            Nu.Iota();
            Nu.Lambda();
            Nu.Alpha();
        }

        public Fa Pi()
        {
            List<Faung> Beta = new List<Faung>();
            Fa Iota;

            for (int i = 0; i < 3; i++)
            {
                Faung Lambda = new Faung(
                    Math.ModPow(Mu.Sigma.Ohm, Mu.Sigma.Pi, Psi.Nu.Omicron),
                    Math.ModPow(Psi.Mu.Sigma.Ohm, Psi.Mu.Sigma.Pi, Psi.Nu.Omicron),
                    Math.ModPow(Rho.Mu.Sigma.Ohm, Rho.Mu.Sigma.Pi, Psi.Nu.Omicron),
                    Math.ModPow(Nu.Pi, Nu.Ohm, Psi.Nu.Omicron));
                Lambda.Theta(Psi.Eta.Rho.Identity);
                Beta.Add(Lambda);
            }

            int b = 99;

            return new Fa();
            /*
            if (!(Beta[0].Xi == Beta[1].Xi &&
                Beta[2].Xi == Beta[0].Rod.Barn &&
                Beta[1].Cone.Barn == Beta[2].Cone.Coordinate &&
                Beta[2].Rod.Coordinate == Beta[0].Cone.Barn)) throw 101;

            Iota = new Fa();
            Iota.Fuse(Beta[1].Omega, Beta[0].Omega, Beta[2].Omega);
            Iota.Tune();
            Iota.Polarize();
            Iota.Conjugate(&Beta[1].Omicron);
            Iota.Saturate(Beta[0].Omicron, Beta[2].Omicron);
            Iota.Open();

            for (int i = 0; i < Beta.size(); i++) Tod::Eta.Water.push_front(Beta[i]);
            Beta.clear();
            return Iota;
            */
        }
    }
}
