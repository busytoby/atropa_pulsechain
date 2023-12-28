
using Apparition.Retaliation;
using Dysnomia.Domain;
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
        static public Queue<Tare.MSG> MsgQueue;
        static public short LogLevel = 1;

        static Apparition()
        {
            Tau = new Object();
            MsgQueue = new Queue<Tare.MSG>();
        }

        public static void Input(Tare.MSG A)
        {
            if(A.Priority > LogLevel) lock(Tau) MsgQueue.Enqueue(A);
        }
    }
}
