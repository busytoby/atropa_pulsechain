using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Text;
using System.Threading.Tasks;

namespace Dysnomia.Domain.World
{
    public class Fan
    {
        public List<Fang> Items;
        public Dictionary<BigInteger, int> Indexes;

        public Fan()
        {
            Items = new List<Fang>();
            Indexes = new Dictionary<BigInteger, int>();
            Add(0, 0);
        }

        public Fan(BigInteger Proxy)
        {
            Items = new List<Fang>();
            Indexes = new Dictionary<BigInteger, int>();
            Add(0, Proxy);
        }

        public Fan(BigInteger ClientId, BigInteger Proxy)
        {
            Items = new List<Fang>();
            Indexes = new Dictionary<BigInteger, int>();
            Add(ClientId, Proxy);
        }

        public void Add(BigInteger clientId, BigInteger Proxy)
        {
            Indexes.Add(clientId, Items.Count);
            Items.Add(new Fang(Proxy));
        }

        public Fang this[BigInteger index]
        {
            get => Items[Indexes[index]];
        }
    }
}
