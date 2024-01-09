using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Text;
using System.Threading.Tasks;

namespace Dysnomia.Lib
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

        public byte[] NextBytes()
        {
            if (Count == 0) throw new Exception("No Next");
            byte[]? Beta;
            TryDequeue(out Beta);
            if (Beta == null) throw new Exception("Dequeue Failure");
            return Beta;
        }

        public BigInteger Next()
        {
            return new BigInteger(NextBytes());
        }
    }
}
