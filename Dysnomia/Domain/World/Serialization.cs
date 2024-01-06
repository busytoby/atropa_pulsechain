using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Dysnomia.Domain.World
{
    public class Serialization : Daemon
    {
        new public static string Name = "Serialization";
        new public static String Description = "Serialization Daemon";

        public void Serialize(byte[] OpCode, byte[] Bytes)
        {
            Enqueue(OpCode);
            Enqueue(Bytes);
        }
    }
}
