using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Reflection.Emit;
using System.Text;
using System.Threading.Channels;
using System.Threading.Tasks;

namespace Dysnomia.Domain.World
{
    public class Fan
    {
        public List<Fang> Items;
        public Dictionary<BigInteger, int> Indexes;
        public BigInteger ProxyId;

        public Fan()
        {
            ProxyId = 0;
            Items = new List<Fang>();
            Indexes = new Dictionary<BigInteger, int>();
            Add(0);
        }

        public Fan(BigInteger ClientId, BigInteger Proxy)
        {
            ProxyId = Proxy;
            Items = new List<Fang>();
            Indexes = new Dictionary<BigInteger, int>();
            Add(Proxy);
        }

        public void Add(BigInteger Proxy)
        {
            Indexes.Add(Proxy, Items.Count);
            Items.Add(new Fang());
        }

        public Fang this[BigInteger index]
        {
            get => Items[Indexes[index]];
        }
    }
}
