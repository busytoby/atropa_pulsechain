using static Dysnomia.Domain.Tare;
namespace Dysnomia.Domain
{
    public class Tare : List<Gram>
    {
        public delegate void Gram(byte[] A);
    }
}