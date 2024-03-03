using Dysnomia.Domain.bin;
using ExtensionMethods;
using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Text;
using System.Threading.Tasks;

namespace Dysnomia.Lib
{
    public class Conjunction : ConcurrentQueue<byte[]>
    {
        public void Join(byte[] OpCode, byte[] Bytes)
        {
            Enqueue(OpCode);
            Enqueue(Bytes);
        }

        public void Join(byte[] OpCode, BigInteger Beta) { Join(OpCode, Beta.ToByteArray()); }

        public BigInteger Next() { return new BigInteger(NextBytes()); }
        public string NextString() { return Encoding.Default.GetString(NextBytes()); }

        public byte[] NextBytes()
        {
            if (Count == 0) throw new Exception("No Next");
            byte[]? Beta;
            TryDequeue(out Beta);
            if (Beta == null) throw new Exception("Dequeue Failure");
            return Beta;
        }

        public byte OpCode()
        {
            if (Count == 0) throw new Exception("No Next");
            byte[]? Beta;
            TryPeek(out Beta);
            if (Beta == null || Beta.Length != 1) throw new Exception("Invalid OpCode");
            TryDequeue(out Beta);
            if (Beta == null) throw new Exception("Invalid OpCode");
            return Beta[0];
        }

        public byte[] Serialize(int skip = 0)
        {
            byte[] Delta = new byte[] { };
            foreach (byte[] Alpha in ToArray())
            {
                if (skip-- > 0) continue;
                byte[] Lambda = BitConverter.GetBytes(Alpha.Length);
                Delta = Delta.Concat(Lambda).Concat(Alpha).ToArray();
            }
            return Delta.Concat(new byte[] {0x00}).ToArray();
        }

        public static Conjunction Deserialize(byte[] Gamma)
        {
            int _size;
            byte[] _next;
            Conjunction Mu = new Conjunction();

            try
            {
                for (int i = 0; i < Gamma.Length;)
                {
                    _size = BitConverter.ToInt32(Gamma, i);
                    _next = Gamma.Skip(i + 4).Take(_size).ToArray();
                    Mu.Enqueue(_next);
                    if (Gamma.Skip(i + 4 + _size).Take(1).ToArray()[0] == 0) return Mu;
                }
            }
            catch { }
            return null;
        }
    }
}
