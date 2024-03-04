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
            Add(0);
        }

        public void Add(BigInteger clientId)
        {
            Indexes.Add(clientId, Items.Count);
            Items.Add(new Fang());
        }

        public Fang this[BigInteger index]
        {
            get => Items[Indexes[index]];
        }
    }
}
