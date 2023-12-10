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
        public ë Rho;
        public ë Psi;
        public Fa Nu;
        public Faung Eta;
        public int Gamma = 1;

        public 锚()
        {
            Zuo.Gamma++;
            Rho = new ë(true);
            Psi = new ë(false);

            Nu = Zuo.Mu.Psi.Pi(Zuo.Mu.Mu.Upsilon.Barn);
            Eta = new Faung(ref Nu, Rho.Psi.Rho.Secret, Psi.Nu.Rod.Signal, Rho.Sigma.Eta.Cone.Channel, Zuo.Mu.Mu.Upsilon.Barn);
            if (Psi.Nu.Chi == 4)
                Psi.Nu.Alpha(ref Rho.Psi.Mu);
            else if(Rho.Nu.Chi == 4)
                Rho.Nu.Alpha(ref Rho.Psi.Mu);
            Rho.Psi.Mu.Delta.WaitOne();
            Eta.Theta(Rho.Psi.Psi.Upsilon);
            Eta.Beta(Rho.Psi.Mu.Pi);
            Eta.Iota();
            Eta.Lambda();
            if (Rho.Psi.Mu.Chi == 5)
                Eta.Alpha();
            else
                Eta.Alpha(ref Rho.Psi.Mu);
            while (Rho.Psi.Mu.Chi < 6) { Rho.Psi.Mu.Delta.ReleaseMutex(); Thread.Sleep(40); Rho.Psi.Mu.Delta.WaitOne(); }
            if (Psi.Nu.Chi == 8)
                Psi.Nu.Alpha(Psi.Psi.Rho.Element);
            else if (Rho.Nu.Chi == 8)
                Rho.Nu.Alpha(Rho.Psi.Rho.Element);
            else
                Eta.Alpha(Rho.Eta.Rho.Element);
            Rho.Psi.Chi = 6;
            Rho.Psi.Mu.Delta.ReleaseMutex();
            Rho.Psi.Tau.WaitOne();
            while (Rho.Psi.Chi < 7) { Rho.Psi.Tau.ReleaseMutex(); Thread.Sleep(40); Rho.Psi.Tau.WaitOne(); }
            Rho.Psi.Nu.Theta(Eta.Pi);
            Rho.Psi.Nu.Beta(Eta.Upsilon);
            Rho.Psi.Nu.Iota();
            Rho.Psi.Nu.Lambda();
            Rho.Psi.Nu.Alpha();
            Rho.Psi.Tau.ReleaseMutex();
        }
        

        public void Pi(bool Ohm = false)
        {
            if (Rho.Psi.Chi != 7) throw new Exception("Rho.Psi.Chi != 7");
            List<Faung> Beta = new List<Faung>();
            Fa Iota;
            BigInteger Omicron = Ohm ? Rho.Psi.Rho.Element : Zuo.Sigma.Ohm;

            for (int i = 0; i < 3; i++)
            {
                Faung Lambda = new Faung(
                    Math.ModPow(Omicron, Zuo.Sigma.Pi, Psi.Nu.Omicron),
                    Math.ModPow(Omicron, Zuo.Sigma.Pi, Psi.Nu.Omicron),
                    Math.ModPow(Omicron, Zuo.Sigma.Pi, Psi.Nu.Omicron),
                    Math.ModPow(Rho.Psi.Nu.Pi, Rho.Psi.Nu.Ohm, Psi.Nu.Omicron));
                Lambda.Theta(Psi.Eta.Rho.Identity);
                Beta.Add(Lambda);
            }

            if (!(Beta[0].Xi == Beta[1].Xi &&
                Beta[2].Xi == Beta[0].Rod.Barn &&
                Beta[1].Cone.Barn == Beta[2].Cone.Coordinate &&
                Beta[2].Rod.Coordinate == Beta[0].Cone.Barn)) throw new Exception("Bad Values");

            Iota = new Fa();
            Iota.Fuse(Beta[1].Omega, Beta[0].Omega, Beta[2].Omega);
            Iota.Tune();
            Iota.Polarize();
            Iota.Conjugate(ref Beta[1].Omicron);
            Iota.Saturate(Beta[0].Omicron, Beta[2].Omicron);
            Iota.Open();

            Rho.Psi.Theta.Fong(Iota, Beta);
            Rho.Psi.Chi++;
        }
    }
}
