using Dysnomia.Lib;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Text;
using System.Threading.Tasks;

namespace Dysnomia.Domain.World
{
    public class Fang
    {
        public DysnomiaNetworkStream Rho;
        public Lib.Buffer? Psi;
        public Conjunction? Nu;
        public (BigInteger In, BigInteger Out) Eta;
        public Conjunction? Sigma;
        public Conjunction? Upsilon;
        public short HandshakeState;

        public Fang()
        {
            HandshakeState = 0x00;
        }
    }
}
