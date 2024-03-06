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
        public Fa Mu;
        public DysnomiaNetworkStream Rho;
        public Lib.Buffer? Psi;
        public Conjunction? Nu;
        public (BigInteger In, BigInteger Out) Eta;
        public Conjunction? Sigma;
        public Conjunction? Upsilon;
        public BigInteger PeerFoundation;
        public BigInteger PeerChannel;
        public BigInteger PeerDynamo;
        public BigInteger Proxy;
        public short HandshakeState;

        public Fang()
        {
            Mu = new Fa();
            PeerFoundation = 0;
            PeerChannel = 0;
            PeerDynamo = 0;
            Proxy = 0;
            HandshakeState = 0x00;
        }
    }
}
