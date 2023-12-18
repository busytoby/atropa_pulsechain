
using Apparition.Retaliation;
using System.Windows;

namespace Dysnomia
{
    public static class Apparition
    {
        static public bool Stub = true;
        static public RetaliationWindow Window;
        static public Mutex Tau = new Mutex();
        static public Queue<Tuple<String, String>> MsgQueue;

        static Apparition()
        {
            MsgQueue = new Queue<Tuple<String, String>>();
        }
    }
}
