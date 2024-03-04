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
        public Lib.Buffer? Psi;
        public Conjunction? Nu;
        public (BigInteger In, BigInteger Out) Eta;
        public Conjunction? Sigma;
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

        public Fang(BigInteger _proxy)
        {
            Mu = new Fa();
            PeerFoundation = 0;
            PeerChannel = 0;
            PeerDynamo = 0;
            Proxy = _proxy;
            HandshakeState = 0x00;
        }
    }
}
