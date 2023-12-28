using static Dysnomia.Domain.Tare;
namespace Dysnomia.Domain
{
    public class Tare : List<Gram>
    {
        public class MSG
        {
            public byte[] From;
            public byte[] Data;
            public short Priority;
            public DateTime TimeStamp;

            public MSG(byte[] from, byte[] data, short priority)
            {
                From = from;
                Data = data;
                Priority = priority;
                TimeStamp = DateTime.Now;
            }
        }

        public delegate void Gram(MSG A);
    }
}