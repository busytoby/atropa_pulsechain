using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Numerics;
using System.Text;
using System.Threading.Channels;
using System.Threading.Tasks;

namespace Dysnomia.Domain
{
    public class Fi
    {
        private TcpListener Mu;
        public Tare Rho;
        private Fa Upsilon;

        public Fi()
        {
            Rho = new Tare();
        }

        public void Listen(int port)
        {
            if (Mu != null) throw new Exception("Mu Non Null");
            Mu = new TcpListener(IPAddress.Any, port);
            Mu.Start();
            Mu.BeginAcceptTcpClient(Kappa, Mu);
        }

        private void Kappa(IAsyncResult result)
        {           
            new Thread(() => Phi(Mu.EndAcceptTcpClient(result))).Start();
            Mu.BeginAcceptTcpClient(Kappa, Mu);
        }

        private void Phi(TcpClient Beta)
        {
            NetworkStream Iota = Beta.GetStream();
            Iota.ReadTimeout = 100;
            byte[] bytes = new byte[32];
            Span<Byte> Omicron = new Span<Byte>(bytes);
            while (Beta.Connected)
            {
                if (Iota.DataAvailable)
                {
                    int size = Iota.Read(Omicron);
                    if (size > 0)
                    {
                        Tare.MSG M = new Tare.MSG(Encoding.Default.GetBytes("Fi"), Omicron.Slice(0, size).ToArray(), 1);
                        foreach (Tare.Gram G in Rho) G(M);
                    }
                }
                else
                    Thread.Sleep(400);

                try
                {
                    Iota.WriteByte(111);
                }
                catch (IOException E)
                {
                    break;
                }
            }
        }
    }
}
