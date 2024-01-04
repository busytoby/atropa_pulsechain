using static Dysnomia.Domain.Tare;
namespace Dysnomia.Domain
{
    public class Tare : List<Gram>
    {
        public class MSG
        {
            public byte[] From;
            public byte[]? Subject;
            public byte[] Data;
            public short Priority;
            public DateTime TimeStamp;

            public MSG(byte[] from, byte[]? subject, byte[] data, short priority)
            {
                From = from;
                Subject = subject;
                Data = data;
                Priority = priority;
                TimeStamp = DateTime.Now;
            }

            public MSG(byte[] from, byte[] data, short priority) : this(from, null, data, priority){}
        }

        public delegate void Gram(MSG A);
    }
}