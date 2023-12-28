
using Apparition.Retaliation;
using System.Text;
using System.Windows;

namespace Dysnomia
{
    public static class Apparition
    {
        static public bool Stub = true;
        static public RetaliationWindow Window;
        static public Object Tau;
        static public Queue<Tuple<String, String>> MsgQueue;

        static Apparition()
        {
            Tau = new Object();
            MsgQueue = new Queue<Tuple<String, String>>();
        }

        public static void Input(Dysnomia.Domain.Tare.MSG A)
        {
            if(A.Priority > 0)
            lock(Tau) MsgQueue.Enqueue(new Tuple<String, String>(Encoding.Default.GetString(A.From), Encoding.Default.GetString(A.Data)));
        }
    }
}
