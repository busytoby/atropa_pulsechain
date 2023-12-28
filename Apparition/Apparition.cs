
using Apparition.Retaliation;
using System.Text;
using System.Windows;
using Windows.Foundation.Diagnostics;

namespace Dysnomia
{
    public static class Apparition
    {
        static public bool Stub = true;
        static public RetaliationWindow Window;
        static public Object Tau;
        static public Queue<Tuple<String, String>> MsgQueue;
        static public short LogLevel = 3;

        static Apparition()
        {
            Tau = new Object();
            MsgQueue = new Queue<Tuple<String, String>>();
        }

        public static void Input(Dysnomia.Domain.Tare.MSG A)
        {
            if(A.Priority > LogLevel)
            lock(Tau) MsgQueue.Enqueue(new Tuple<String, String>(Encoding.Default.GetString(A.From), Encoding.Default.GetString(A.Data)));
        }
    }
}
