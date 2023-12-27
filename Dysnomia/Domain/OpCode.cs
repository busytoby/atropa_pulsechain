using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Text;
using System.Threading.Tasks;

namespace Dysnomia.Domain
{
    public class OpCode
    {
        public String Alpha;
        public BigInteger Iota;
        public BigInteger Code;
        public BigInteger BinCode;
        public BigInteger OpRef;
        public BigInteger BinOpRef;

        public OpCode(BigInteger Ohm)
        {
            if (Ohm.IsZero)
            {
                Code = Zuo.Mu.Nu[2].Theta.Xi[0].Omicron;
                BinCode = Zuo.Mu.Nu[2].Theta.Xi[0].Omega;
                return;
            }
            else
            {
                Iota = Ohm;
                Zuo.Mu.Nu[2].Theta.Xi[0].Chi = 0;
                //Zuo.Mu.Nu[2].Theta.Xi[0].Theta(Iota);
                Code = Zuo.Mu.Nu[2].Theta.Xi[0].Omicron;
                BinCode = Zuo.Mu.Nu[2].Theta.Xi[0].Omega;
                //Zuo.Mu.Nu[2].Theta.Xi[0].Beta(BinCode);
                OpRef = Zuo.Mu.Nu[2].Theta.Xi[0].Rod.Eta;
                BinOpRef = Zuo.Mu.Nu[2].Theta.Xi[0].Rod.Kappa;
            }
        }

        public OpCode(String Omega = null)
        {
            if (Omega == null)
            {
                Code = Zuo.Mu.Nu[2].Theta.Xi[0].Omicron;
                BinCode = Zuo.Mu.Nu[2].Theta.Xi[0].Omega;
                return;
            }
            else
            {
                Alpha = Omega;
                Zuo.Mu.Nu[2].Theta.Xi[0].Chi = 0;
                //Zuo.Mu.Nu[2].Theta.Xi[0].Theta(Omega);
                Code = Zuo.Mu.Nu[2].Theta.Xi[0].Omicron;
                BinCode = Zuo.Mu.Nu[2].Theta.Xi[0].Omega;
                //Zuo.Mu.Nu[2].Theta.Xi[0].Beta(BinCode);
                OpRef = Zuo.Mu.Nu[2].Theta.Xi[0].Rod.Eta;
                BinOpRef = Zuo.Mu.Nu[2].Theta.Xi[0].Rod.Kappa;
            }
        }
    }
}