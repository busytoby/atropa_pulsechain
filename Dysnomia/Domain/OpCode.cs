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
        public String Iota;
        public BigInteger Code;
        public BigInteger BinCode;
        public BigInteger OpRef;
        public BigInteger BinOpRef;

        public OpCode(Faung Beta, String Omega = null) { 
            if(Omega == null)
            {
                Code = Beta.Omicron;
                BinCode = Beta.Omega;
                return;
            } else
            {
                Iota = Omega;
                Beta.Chi = 0;
                Beta.Theta(Omega);
                Code = Beta.Omicron;
                BinCode = Beta.Omega;
                Beta.Beta(BinCode);
                OpRef = Beta.Rod.Eta;
                BinOpRef = Beta.Rod.Kappa;
            }
        }
    }
}
