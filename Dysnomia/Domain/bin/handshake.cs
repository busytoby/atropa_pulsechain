using System;
using System.Text;
using Dysnomia.Lib;

namespace Dysnomia.Domain.bin
{
    internal class handshake : Command
    {
        new public static String Name = "cmd_Handshake";
        new public static String Description = "Handshake [peer_address] - Execute YI WinchesterMQ handshake";

        protected override async void Phi()
        {
            if (Theta == null) throw new Exception("Null Command Theta");
            if (Args == null) throw new Exception("Null Command Args");
            byte[] From = Encoding.Default.GetBytes(Name);

            if (Args.Length == 0)
            {
                Output(From, Encoding.Default.GetBytes("Handshake with who? Provide peer address."), 6);
                return;
            }

            string peerAddress = (string)Args[0];
            Output(From, Encoding.Default.GetBytes("Initiating YI handshake sequence for peer: " + peerAddress), 6);

            bool success = Controller.LocalNetwork.PerformYiHandshake(peerAddress);
            if (success)
            {
                Output(From, Encoding.Default.GetBytes("SUCCESS: YI Handshake established with " + peerAddress), 6);
            }
            else
            {
                Output(From, Encoding.Default.GetBytes("FAILURE: YI Handshake failed for " + peerAddress), 6);
            }
        }
    }
}
