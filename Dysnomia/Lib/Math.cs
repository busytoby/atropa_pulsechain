using System;
using System.Globalization;
using System.Numerics;

namespace Dysnomia
{
    public class Math
    {
        public static BigInteger Prime;
        public static BigInteger MotzkinPrime = 953467954114363;
        public static BigInteger BellPrime = BigInteger.Parse("35742549198872617291353508656626642567");
        public static BigInteger RepunitPrime = BigInteger.Parse(
            "3AF1BF72E45FFA4B0C58C8EFD853EC16D0CEF373229C476B51B61F54D30E04D8B76C" +
            "21AB7D561AFFBAF86D54B30F0801D3C469F90CA32AC22DE4B8D67628A6F8C7AFE84F" +
            "A17B62600A6DE053A7EA0D55E2F90E3052467ABBEF4DDFF671C71C71C71C71C71C71" +
            "C71C71C71C71C71C71C71C71C71C71C71C71C71C71C71C71C71C71C71C7", NumberStyles.AllowHexSpecifier);

        public static BigInteger CatalanPrime = BigInteger.Parse(
            "01A74B82867AB14262C6E0153A32170696AB92141212C6C478D195A94DE8FACB8AD0" +
            "901ABFE89B6129C427EBFA03AD511242C79FD93E89B20429DB33BEC356978F703875" +
            "07A95AD2068E195E9B852B6BF884AB9A714BA75CFB10A7F9CFE865A0764B6B6E8648" +
            "C98C8252C2B2AC42E7CAE4E8360", NumberStyles.AllowHexSpecifier);

        public static BigInteger MarsennePrime = BigInteger.Parse(
            "01FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF" +
            "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF", NumberStyles.AllowHexSpecifier);

        private static Random rand;
        private static Object randLock;

        static Math()
        {
            rand = new Random(); 
            randLock = new Object();
            Prime = MotzkinPrime;
        }

        public static BigInteger Random()
        {
            byte[] b = Prime.ToByteArray();
            lock(randLock)
            {
                rand.NextBytes(b);
            }
            BigInteger R = new BigInteger(b);
            if (R < 0) R = R * -1;
            return R % Prime;
        }

        public static BigInteger ModXOR(BigInteger A, BigInteger B, BigInteger C)
        {
            BigInteger M = A ^ B % C;
            return M;
        }

        public static BigInteger ModPow(BigInteger A, BigInteger B, BigInteger C)
        {
            BigInteger M = BigInteger.ModPow(A, B, C);
            return M;
        }
    }

}
