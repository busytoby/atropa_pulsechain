using Microsoft.VisualBasic;
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
        public Fa Nu;
        public Faung Eta;
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

            Nu = Mu.Mu.Psi.Pi(Mu.Mu.Mu.Upsilon.Barn);
            Eta = new Faung(ref Nu, Rho.Psi.Rho.Secret, Psi.Nu.Rod.Signal, Rho.Sigma.Eta.Cone.Channel, Mu.Mu.Mu.Upsilon.Barn);
            Psi.Nu.Alpha(ref Rho.Psi.Mu);
            Rho.Psi.Mu.Delta.WaitOne();
            Eta.Theta(Rho.Psi.Mu.Ohm);
            Eta.Beta(Rho.Psi.Mu.Ohm);
            Eta.Iota();
            Eta.Lambda();
            Eta.Alpha();
            while (Rho.Psi.Mu.Chi < 6) { Rho.Psi.Mu.Delta.ReleaseMutex(); Thread.Sleep(2000); Rho.Psi.Mu.Delta.WaitOne(); }
            Psi.Nu.Alpha(Rho.Psi.Rho.Rho);
            Rho.Psi.Chi = 6;
            Rho.Psi.Mu.Delta.ReleaseMutex();
            Rho.Psi.Tau.WaitOne();
            while (Rho.Psi.Chi < 7) { Rho.Psi.Tau.ReleaseMutex(); Thread.Sleep(2000); Rho.Psi.Tau.WaitOne(); }
            Rho.Psi.Nu.Theta(Eta.Omega);
            Rho.Psi.Nu.Beta(Eta.Omicron);
            Rho.Psi.Nu.Iota();
            Rho.Psi.Nu.Lambda();
            Rho.Psi.Nu.Alpha();
            Rho.Psi.Tau.ReleaseMutex();
        }

        /*
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
        //}

    }
}
