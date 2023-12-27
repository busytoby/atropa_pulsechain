using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Dysnomia.Domain
{
    public class Nit : ConcurrentQueue<byte[]>
    {
    }
}
