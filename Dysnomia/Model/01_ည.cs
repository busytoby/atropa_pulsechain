using Dysnomia.Domain;
using System;
using System.Collections.Generic;
using System.Diagnostics.Eventing.Reader;
using System.Linq;
using System.Numerics;
using System.Text;
using System.Threading.Tasks;

namespace Dysnomia
{
    public class ည
    {
        public Faung Mu;
        public Fa Rho;
        public Faung Psi;
        public Faung Nu;
        public Mutex Tau = new Mutex();
        public Living Theta;
        public int Kappa;
        public int Chi = 0;
        public int Gamma = 1;

        private int _r = 0;

        public ည()
        {
            Mu = new Faung();
            Rho = new Fa();
            Kappa = 1;
            Psi = Pi(true);
            Theta = new Living(Phi);
        }

        public void Phi()
        {
            int _sleep = 1;
            while (true)
            {
                Tau.WaitOne();
                Mu.Delta.WaitOne();
                Psi.Delta.WaitOne();
                if ((Mu.Chi == 5 | Mu.Chi == 6) && Kappa != 3)
                    throw new Exception("Unplanned");

                if (Kappa == 1)
                {
                    if (Chi == 0)
                        Mu.Theta(Psi.Cone.Coordinate);
                    else if (Chi == 1)
                        Psi.Theta(Mu.Rod.Coordinate);
                    else if (Chi == 2)
                        Psi.Beta(Mu.Omicron);
                    else if (Chi == 3 && Psi.Chi == 4)
                    {
                        Psi.Alpha();
                        Chi++;
                    }
                    else if (Chi == 4 && Mu.Chi == 4)
                    {
                        Mu.Alpha();
                        Chi++;
                    }
                    if (Chi < 3) Chi++;
                } else if(Kappa == 2)
                {
                    if (Chi == 0)
                    {
                        if (Psi.Omicron.IsZero)
                            Psi.Omicron = Psi.Rod.Identity ^ Psi.Cone.Identity;
                        Mu.Theta(Psi.Omicron);
                    }
                    else if (Chi == 1)
                        Mu.Beta(Psi.Omicron);
                    else if (Chi == 2)
                        Mu.Iota();
                    else if (Chi == 3)
                        Mu.Lambda();
                    else if (Chi == 4)
                        Mu.Alpha();
                    if (Chi < 5) Chi++;
                } else if(Kappa == 3)
                {
                    if (Chi == 0)
                    {
                        if (Mu.Omicron.IsZero)
                            Mu.Omicron = Mu.Rod.Kappa ^ Mu.Cone.Kappa;

                        Psi.Theta(Mu.Omicron);
                    }
                    else if (Chi == 1)
                        Psi.Beta(Mu.Omicron);
                    else if (Chi == 2)
                        Psi.Iota();
                    else if (Chi == 3)
                        Psi.Lambda();
                    else if (Chi == 4)
                        Psi.Alpha();
                    else if (Chi == 5 && Mu.Chi == 5)
                    {
                        Mu.Alpha();
                    }
                    else if (Chi == 6 && Mu.Chi == 6)
                    {
                        if (Nu != null) throw new Exception("Collapse");
                        Nu = new Faung(Mu.Upsilon, Mu.Rho, Mu.Pi, Rho.Barn);
                        Mu.Chi = 1;
                        Mu.Beta(Mu.Omicron);
                        Mu.Iota();
                        Mu.Lambda();
                        Psi.Chi = 1;
                        Psi.Beta(Mu.Omicron);
                        Psi.Iota();
                        Psi.Lambda();
                        Psi.Alpha();
                        Chi++;
                    }
                    else if (Chi == 8)
                    {
                        if (Theta.Chi != null && Theta.Xi.Count == 3 && Theta.Xi[0].Chi == 8 && Theta.Xi[1].Chi == 5 && Theta.Xi[2].Chi == 1)
                        {
                            Theta.Xi[2].Beta(Theta.Chi.Channel);
                            Theta.Xi[2].Iota();
                            Theta.Xi[2].Lambda();
                            Chi++;
                        }
                        _sleep = 200;
                        _r++;
                    }
                    else if (Chi == 9)
                    {
                        if(Theta.Xi.Count == 3 && Theta.Xi[0].Chi == 8)
                        {
                            Theta.Xi[1].Alpha();
                            Theta.Xi[2].Alpha();
                            Chi++;
                        }
                        _sleep = 200;
                        _r++;
                    } else if (Chi == 10)
                    {
                        if (Theta.Xi[0].Chi != 8) throw new Exception("Chi Non 8");
                        Theta.Xi[0].Chi = 0;
                        Chi++;
                    }
                    if (Chi < 5) Chi++;
                    else _sleep = (_sleep >= 4000) ? 4000 : _sleep * 2;
                }
                Mu.Delta.ReleaseMutex();
                Psi.Delta.ReleaseMutex();
                Tau.ReleaseMutex();
                Thread.Sleep(_sleep);
            }
        }

        public ည(ref Faung Beta, ref Fa Omicron, bool Lambda)
        {
            Beta.Rod.Gamma++;
            Beta.Cone.Gamma++;
            Omicron.Gamma++;
            if (Lambda)
                Mu = new Faung();
            else
                Mu = Beta;
            Rho = Omicron;
            if (Lambda)
            {
                Kappa = 2;
                Psi = Beta;
            }
            else
            {
                Kappa = 3;
                Psi = Pi(false);
            }
            Theta = new Living(Phi);
        }

        public Faung Pi(bool Lambda)
        {
            if (Lambda)
                return new Faung(ref Rho, Mu.Cone.Secret, Mu.Cone.Signal, Mu.Cone.Channel, Mu.Cone.Identity);
            else
                return new Faung(Mu.Cone.Dynamo, Mu.Cone.Ring, Mu.Cone.Barn, Mu.Cone.Manifold);
        }

        /*
        List<Faung*> Affinities() { return List<Faung*>({ Mu, Psi }); }

        ~ည()
        {
            if (Mu.Rod.Gamma <= 1 || Mu.Cone.Gamma <= 1) delete Mu; else { Mu.Rod.Gamma--; Mu.Cone.Gamma--; }
            if (Rho.Gamma == 1) delete Rho; else Rho.Gamma--;
            if (Psi.Rod.Gamma <= 1 || Psi.Cone.Gamma <= 1) delete Psi; else { Psi.Rod.Gamma--; Psi.Cone.Gamma--; }
        }
        */
    }
}
