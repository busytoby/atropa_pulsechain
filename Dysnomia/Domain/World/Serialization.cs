using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Text;
using System.Threading.Tasks;

namespace Dysnomia.Domain.World
{
    public class Serialization : ConcurrentQueue<byte[]>
    {
        public void Serialize(byte[] OpCode, byte[] Bytes)
        {
            Enqueue(OpCode);
            Enqueue(Bytes);
        }

        public void Serialize(byte[] OpCode, BigInteger Beta)
        {
            Serialize(OpCode, Beta.ToByteArray());
        }
    }
}
